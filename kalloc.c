// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define MAXENTRY 57334 

extern int PID[];
extern uint VPN[];
extern pte_t PTE_XV6[];

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    //kfree(p);
	PID[(int)(V2P(p))/4096] = -1; //If PID[i] is -1, the physical frame i is freespace.
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void kfree(int pid, char *v){

  uint kv, idx;

  panic("kfree");
  //TODO: Fill the code that supports kfree
  //1. Find the corresponding physical address for given pid and VA
  //2. Initialize the PID[idx], VPN[idx], and PTE_XV6[idx]
  //3. For memset, Convert the physical address for free to kernel's virtual address by using P2V macro
  // memset(kv, 1, PGSIZE); //TODO: You must perform memset for P2V(physical address);
}

#define USTART 1024 * PGSIZE

int find_k_free_space(){
    uint pa = PGROUNDUP((uint)V2P(end));
    int idx;

    for(; pa < USTART; pa += PGSIZE){
      idx = pa / PGSIZE;
      if(PID[idx] == -1){
        return idx;
      }
    }
    return -1;
}

int find_u_free_space(){
  uint pa = USTART;
  int idx;
  for(;pa < (uint)PHYSTOP; pa+= PGSIZE){
    idx = pa / PGSIZE;
    if(PID[idx] == -1){
      return idx;
    }
  }
  return -1;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

char*
kalloc(int pid, char *v)
{
  int idx;
  uint pa;
  if(kmem.use_lock)
    acquire(&kmem.lock);

  if (v == (char *)-1){
    idx = find_k_free_space();
  } else {
    idx = find_u_free_space();
  }
  
  if(idx != -1){
    if(kmem.use_lock)
      release(&kmem.lock);
    return 0;
  }
  pa = idx * PGSIZE;

  PID[idx] = pid;

  if(v == (char *)-1){
    VPN[idx] = (uint)P2V(pa);
  } else {
    VPN[idx] = (uint)v;
  }
  //TODO: Fill the code that supports kalloc
  //1. Find the freespace by hash function
  //2. Consider the case that v is -1, which means that the caller of kalloc is kernel so the virtual address is decided by the allocated physical address (P2V) 
  //3. Update the value of PID[idx] and VPN[idx] (Do not update the PTE_XV6[idx] in this code!)
  //4. Return (char*)P2V(physical address), if there is no free space, return 0
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char *)P2V(pa);
}

/*
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}
*/
