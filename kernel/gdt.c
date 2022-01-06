/*
 * @Description: 
 * @Author: QIUFUYU
 * @Date: 2021-09-15 21:13:40
 * @LastEditTime: 2021-10-06 16:09:18
 */
#include"gdt.h"
#include"task/tss.h"
#include"qstring.h"
#include"kio.h"
// Lets us access our ASM functions from our C code.
extern void gdt_flush(uint32);
extern void tss_flush();
static void tss_write(int32,uint16,uint32);
extern void idt_flush(uint32);
static void gdt_set_gate(int32,uint32,uint32,uint8,uint8);
static void idt_set_gate(uint8,uint32,uint16,uint8);
gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;
tss_entry_t tss_entry;
static void gdt_set_gate(int32 num, uint32 base, uint32 limit, uint8 access, uint8 gran)
{
   gdt_entries[num].base_low    = (base & 0xFFFF);
   gdt_entries[num].base_middle = (base >> 16) & 0xFF;
   gdt_entries[num].base_high   = (base >> 24) & 0xFF;

   gdt_entries[num].limit_low   = (limit & 0xFFFF);
   gdt_entries[num].granularity = (limit >> 16) & 0x0F;

   gdt_entries[num].granularity |= gran & 0xF0;
   gdt_entries[num].access      = access;
}
static void idt_set_gate(uint8 num, uint32 base, uint16 sel, uint8 flags)
{
   idt_entries[num].base_lo = base & 0xFFFF;
   idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].always0 = 0;
   // We must uncomment the OR below when we get to using user-mode.
   // It sets the interrupt gate's privilege level to 3.
   idt_entries[num].flags   = flags /* | 0x60 */;
}
uint32 gdt_get_segment(uint8 id)
{
   return (uint32)&gdt_entries[id]- (uint32)gdt_entries;
}
void init_gdt()
{
    gdt_ptr.limit=(sizeof(gdt_entry_t)*6)-1;
    //gdt界限=gdt大小-1
    gdt_ptr.base=(uint32)&gdt_entries;

    gdt_set_gate(0,0,0,0,0);                //空段
    gdt_set_gate(1,0,0xFFFFFFFF,0x9A,0xCF); //代码段
    gdt_set_gate(2,0,0xFFFFFFFF,0x92,0xCF); //数据段
    gdt_set_gate(3,0,0xFFFFFFFF,0xFA,0xCF); //用户模式代码段
    gdt_set_gate(4,0,0xFFFFFFFF,0xF2,0xCF); //用户模式数据段
    tss_write(5,0x10,0x0);
    gdt_flush((uint32)&gdt_ptr);
    tss_flush();
}
void init_idt()
{
       idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
   idt_ptr.base  = (uint32)&idt_entries;

   memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
    //重新映射irq
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
   idt_set_gate( 0, (uint32)isr0 , 0x08, 0x8E);
   idt_set_gate( 1, (uint32)isr1 , 0x08, 0x8E);
   idt_set_gate( 2, (uint32)isr2 , 0x08, 0x8E);
   idt_set_gate( 3, (uint32)isr3 , 0x08, 0x8E);
   idt_set_gate( 4, (uint32)isr4 , 0x08, 0x8E);
   idt_set_gate( 5, (uint32)isr5 , 0x08, 0x8E);
   idt_set_gate( 6, (uint32)isr6 , 0x08, 0x8E);
   idt_set_gate( 7, (uint32)isr7 , 0x08, 0x8E);
   idt_set_gate( 8, (uint32)isr8 , 0x08, 0x8E);
   idt_set_gate( 9, (uint32)isr9 , 0x08, 0x8E);
   idt_set_gate( 10, (uint32)isr10 , 0x08, 0x8E);
   idt_set_gate( 11, (uint32)isr11 , 0x08, 0x8E);
   idt_set_gate( 12, (uint32)isr12 , 0x08, 0x8E);
   idt_set_gate( 13, (uint32)isr13 , 0x08, 0x8E);
   idt_set_gate( 14, (uint32)isr14 , 0x08, 0x8E);
   idt_set_gate( 15, (uint32)isr15 , 0x08, 0x8E);
   idt_set_gate( 16, (uint32)isr16 , 0x08, 0x8E);
   idt_set_gate( 17, (uint32)isr17 , 0x08, 0x8E);
   idt_set_gate( 18, (uint32)isr18 , 0x08, 0x8E);
   idt_set_gate( 19, (uint32)isr19 , 0x08, 0x8E);
   idt_set_gate( 20, (uint32)isr20 , 0x08, 0x8E);
   idt_set_gate( 21, (uint32)isr21 , 0x08, 0x8E);
   idt_set_gate( 22, (uint32)isr22 , 0x08, 0x8E);
   idt_set_gate( 23, (uint32)isr23 , 0x08, 0x8E);
   idt_set_gate( 24, (uint32)isr24 , 0x08, 0x8E);
   idt_set_gate( 25, (uint32)isr25 , 0x08, 0x8E);
   idt_set_gate( 26, (uint32)isr26 , 0x08, 0x8E);
   idt_set_gate( 27, (uint32)isr27 , 0x08, 0x8E);
   idt_set_gate( 28, (uint32)isr28 , 0x08, 0x8E);
   idt_set_gate( 29, (uint32)isr29 , 0x08, 0x8E);
   idt_set_gate( 30, (uint32)isr30 , 0x08, 0x8E);
   idt_set_gate(31, (uint32)isr31, 0x08, 0x8E);
   idt_set_gate(32, (uint32)irq0, 0x08, 0x8E);
   idt_set_gate(33, (uint32)irq1, 0x08, 0x8E);
   idt_set_gate(34, (uint32)irq2, 0x08, 0x8E);
   idt_set_gate(35, (uint32)irq3, 0x08, 0x8E);
   idt_set_gate(36, (uint32)irq4, 0x08, 0x8E);
   idt_set_gate(37, (uint32)irq5, 0x08, 0x8E);
   idt_set_gate(38, (uint32)irq6, 0x08, 0x8E);
   idt_set_gate(39, (uint32)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32)irq8, 0x08, 0x8E);
   idt_set_gate(41, (uint32)irq9, 0x08, 0x8E);
   idt_set_gate(42, (uint32)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32)irq11, 0x08, 0x8E);
   idt_set_gate(44, (uint32)irq12, 0x08, 0x8E);
   idt_set_gate(45, (uint32)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32)irq14, 0x08, 0x8E);
   idt_set_gate(47, (uint32)irq15, 0x08, 0x8E);
   idt_set_gate(128,(uint32)isr128,0x08,0x8e);
   idt_flush((uint32)&idt_ptr);
}
static void tss_write(int32 num, uint16 ss0, uint32 esp0)
{
   // Firstly, let's compute the base and limit of our entry into the GDT.
   uint32 base = (uint32) &tss_entry;
   uint32 limit = base + sizeof(tss_entry);

   // Now, add our TSS descriptor's address to the GDT.
   gdt_set_gate(num, base, limit, 0xE9, 0x00);

   // Ensure the descriptor is initially zero.
   memset(&tss_entry, 0, sizeof(tss_entry));

   tss_entry.ss0  = ss0;  // Set the kernel stack segment.
   tss_entry.esp0 = esp0; // Set the kernel stack pointer.

   // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
   // segments should be loaded when the processor switches to kernel mode. Therefore
   // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
   // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
   // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
   // to switch to kernel mode from ring 3.
   tss_entry.cs   = 0x0b;
   tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}
