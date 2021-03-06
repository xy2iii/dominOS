#ifndef __PAGING_H__
#define __PAGING_H__

#include "stdint.h"
#include "stdbool.h"

// A page is 4Kb (0x1000)
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_SHIFT 12 // 2^12 = 4Kb

// Flags
// Entry present in page table/directory
#define NONE 0x0
#define PRESENT 0x1
// Read write page
#define RW 0x2
// Page accessible in user mode if true, otherwhise only kernel mode page
#define US 0x4

/**
 * Map a zone of virtual adresses to a zone of physical adresses.
 * A zone is a range of memory (start-end).
 * @pre both zones must be the same size
 * @param pdir The page directory to map on
 * @param flags Flags to set on all pages
 */
void map_zone(uint32_t *pdir, uint64_t virt_start, uint64_t virt_end,
              uint64_t phy_start, uint64_t phy_end, uint32_t flags);

/**
 * Unmap a zone. The corresponding virtual adresses are no longer valid.
 */
void unmap_zone(uint32_t *pdir, uint64_t virt_start, uint64_t virt_end);

/** Create a page directory. */
uint32_t *page_directory_create();

/**
 * Free a page directory and any corresponding page tables, if they were allocated
 * in this page directory.
 */
void page_directory_destroy(uint32_t *dir);

void page_fault_handler();
/** Init the page fault handler, which will kill a process if it does a page fault. */
void init_page_fault_handler();

/**
 * Check if an virtual address is mapped with user privileges.
 * @param dir
 * @param virt_addr
 * @return Whether the address is mapped as user
 */
bool is_user_addr(uint32_t *dir, uint32_t virt_addr);

#endif