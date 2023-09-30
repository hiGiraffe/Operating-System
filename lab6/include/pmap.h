#ifndef _PMAP_H_
#define _PMAP_H_

#include <mmu.h>
#include <printk.h>
#include <queue.h>
#include <types.h>

extern Pde *cur_pgdir;

LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

struct Page {
	Page_LIST_entry_t pp_link; /* free list link */

	// Ref is the count of pointers (usually in page table entries)
	// to this page.  This only holds for pages allocated using
	// page_alloc.  Pages allocated at boot time using pmap.c's "alloc"
	// do not have valid reference count fields.

	u_short pp_ref;
};

extern struct Page *pages;
extern struct Page_list page_free_list;

//将物理页帧号（Page Frame Number，PFN）转换成页表项（Page Table Entry，PTE）中存储的PFN
static inline u_long page2ppn(struct Page *pp) {
	return pp - pages;
}

//将物理页帧号（PFN）转换成物理地址（PA）
static inline u_long page2pa(struct Page *pp) {
	return page2ppn(pp) << PGSHIFT;
}

//将物理地址（PA）转换成物理页帧号（PFN）
static inline struct Page *pa2page(u_long pa) {
	if (PPN(pa) >= npage) {
		panic("pa2page called with invalid pa: %x", pa);
	}
	return &pages[PPN(pa)];
}

//将页表项（PTE）中存储的物理页帧号（PFN）转换成内核虚拟地址（KVA）
static inline u_long page2kva(struct Page *pp) {
	return KADDR(page2pa(pp));
}

//将虚拟地址（VA）转换成物理地址（PA）
static inline u_long va2pa(Pde *pgdir, u_long va) {
	Pte *p;

	pgdir = &pgdir[PDX(va)];
	if (!(*pgdir & PTE_V)) {
		return ~0;
	}
	p = (Pte *)KADDR(PTE_ADDR(*pgdir));
	if (!(p[PTX(va)] & PTE_V)) {
		return ~0;
	}
	return PTE_ADDR(p[PTX(va)]);
}

void mips_detect_memory(void);
void mips_vm_init(void);
void mips_init(void);
void page_init(void);
void *alloc(u_int n, u_int align, int clear);

int page_alloc(struct Page **pp);
void page_free(struct Page *pp);
void page_decref(struct Page *pp);
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm);
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte);
void page_remove(Pde *pgdir, u_int asid, u_long va);
void tlb_invalidate(u_int asid, u_long va);

extern struct Page *pages;

void physical_memory_manage_check(void);
void page_check(void);

#endif /* _PMAP_H_ */
