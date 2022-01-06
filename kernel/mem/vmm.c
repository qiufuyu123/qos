#include"mem/memory.h"
#include"types.h"

// Flush the table lookaside buffer
static inline void flush_tlb(void* address) {
    asm volatile("invlpg (%0)" ::"r" (address) : "memory");
}

static inline void enable_paging()
{
	uint32 cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}

static inline void load_page_dir(uint32 address) {
    asm volatile("mov %0, %%cr3":: "b"(address));
    enable_paging();
}


// Assuming the last PDE is mapped to page directory itself,
// the virtual address of the page directory is ALWAYS 0xFFFFF000 !
// Furthermore, the virtual address of a PDE (page table) at index 'i'
// is 0xFFC00000 + 0x400*i
// Reference: http://i159.photobucket.com/albums/t141/sovietweasel/mapping-31.png
//            http://www.rohitab.com/discuss/topic/31139-tutorial-paging-memory-mapping-with-a-recursive-page-directory/


uint32* get_page_directory() {
    return (uint32*)0xFFFFF000;
}
static inline uint32* get_page_table(uint32 index) {
    return ((uint32*)0xFFC00000) + (0x400 * index);
}


// Get physical address from virtual address
void* get_physical_addr(void* virtual_address) {

    uint32 pd_index = (uint32)virtual_address >> 22;
    uint32 pt_index = (uint32)virtual_address >> 12 & 0x03FF;
    
    uint32* page_dir = get_page_directory();

    if (!(page_dir[pd_index] & 1))  // PDE doesn't exist
        return 0;

    uint32* page_table = get_page_table(pd_index);
    if (!(page_table[pt_index] & 1)) // PTE doesn't exist
        return 0;

    return (void*)( (page_table[pt_index] & ~0xFFF) + ((uint32)virtual_address & 0xFFF) );
}

// Map a page for given virtual address to given physical address
// Return 0 if the virtual address is already taken or page table cannot be created
// and 1 if success
bool map_page(void* physical_address, void* virtual_address) {
    uint32 pd_index = (uint32)virtual_address >> 22;
    uint32 pt_index = (uint32)virtual_address >> 12 & 0x03FF;

    uint32* page_dir = get_page_directory();
    uint32* ptable = get_page_table(pd_index);

    if (page_dir[pd_index] & 1) {
        // PDE exists
        if (ptable[pt_index] & 1) {
            return 0;
        }

        // Map
        ptable[pt_index] = (uint32)physical_address | 3;
        flush_tlb(virtual_address);
        return 1;
    }

    // Create new PDE
    
    // First get physical memory to put the table
    uint32 ptable_phys = (uint32)pm_alloc_frame();
    if (!ptable_phys) {
        return 0;
    }

    // Then set the PDE and PTE respectively
    page_dir[pd_index] = ptable_phys | 3;

    // Empty the new page table
    for (int i=0; i<1024; ++i) {
        ptable[i] = 2;
    }
    // Then set the page at given index
    ptable[pt_index] = (uint32)physical_address | 3;
    flush_tlb(virtual_address);
    return 1;
}

// Unmap a virtual address
void unmap_page(void* virtual_address) {
    
    uint32 pd_index = (uint32)virtual_address >> 22;
    uint32 pt_index = (uint32)virtual_address >> 12 & 0x03FF;

    uint32* page_dir = get_page_directory();
    uint32* ptable = get_page_table(pd_index);

    if (page_dir[pd_index] & 1) {
        if (ptable[pt_index] & 1) {
            ptable[pt_index] = 2;
        }

        // Remove the PDE if the page table is now empty
        int i;
        for (i=0; i<1024; ++i)
            if (ptable[i] | 1)
                break;
        if (i == 1024) {
            page_dir[pd_index] = 2;
            pm_free_frame((void*)(page_dir[pd_index] & ~0xFFF));
        }

        flush_tlb(virtual_address);
    }
}



// Although a simple page directory was installed during booting
// we do it again, so that we have more control from C code

// It should be noted that for now, we only allocate one page table (4MB)
// for kernel space, which means we can only use up to 4MB until we start
// using memory allocation system later.

// Set up a new page directory for the kernel
void setup_kernel_page_dir() {

    uint32 dir_phys = (uint32)pm_alloc_frame();
    uint32 table_phys = (uint32)pm_alloc_frame();

    // Let's assume that the first 4MB was identity mapped
    // and the new page dir and table were allocated within that 4MB
    // Then for now, we can directly manipulate these two through the physical addresses
    uint32* dir_virt = (uint32*)dir_phys;
    uint32* table_virt = (uint32*)table_phys;

    uint32 phys_address = 0;
    for (int i=0; i<1024; ++i) {
        dir_virt[i] = 2;
        table_virt[i] = phys_address | 3;
        phys_address += 4096;
    }

    dir_virt[(VIRTUAL_KERNEL_BASE >> 22)] = table_phys | 3;    // Higher half kernel mapping
    dir_virt[1023] = dir_phys | 3;                             // Recursive Page Directory

    // Load the page directory
    load_page_dir(dir_phys);
}