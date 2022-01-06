/*
 * @Description: GDT
 * @Author: QIUFUYU
 * @Date: 2021-09-15 21:10:10
 * @LastEditTime: 2021-10-04 13:54:52
 */
#ifndef _H_GDT
#define _H_GDT
#include"types.h"
#include"task/tss.h"
//gdt全局描述表
//注意，在启动分页后我们会重新加载一次gdt
// gdt入口(entry)表
// packed告诉编译器不要更改结构体排列方式
//gdt entry的结构如下
struct gdt_entry_struct
{
   uint16 limit_low;           // The lower 16 bits of the limit.
   uint16 base_low;            // The lower 16 bits of the base.
   uint8  base_middle;         // The next 8 bits of the base.
   uint8  access;              // Access flags, determine what ring this segment can be used in.
   uint8  granularity;
   uint8  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
//gdt指针结构
//汇编中lgdt就是这个结构
struct gdt_ptr_struct
{
   uint16 limit;               // The upper 16 bits of all selector limits.
   uint32 base;                // The address of the first gdt_entry_t struct.
}
 __attribute__((packed));

// A struct describing an interrupt gate.
struct idt_entry_struct
{
   uint16 base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
   uint16 sel;                 // Kernel segment selector.
   uint8  always0;             // This must always be zero.
   uint8  flags;               // More flags. See documentation.
   uint16 base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
   uint16 limit;
   uint32 base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

// These extern directives let us access the addresses of our ASM ISR handlers.
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10 ();
extern void isr11 ();
extern void isr12();
extern void isr13 ();
extern void isr14 ();
extern void isr15 ();
extern void isr16 ();
extern void isr17 ();
extern void isr18 ();
extern void isr19 ();
extern void isr20 ();
extern void isr21 ();
extern void isr22 ();
extern void isr23 ();
extern void isr24 ();
extern void isr25();
extern void isr26 ();
extern void isr27 ();
extern void isr28 ();
extern void isr29 ();
extern void isr30();
extern void isr31();
extern void isr32();
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void isr128();
//extern void irq48();
typedef struct gdt_ptr_struct gdt_ptr_t;
typedef struct gdt_entry_struct gdt_entry_t;
extern tss_entry_t tss_entry;
void init_gdt();
void init_idt();
uint32 gdt_get_segment(uint8 id);
#endif