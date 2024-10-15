#ifndef __MEMORY_H 
#define __MEMORY_H 
#include "../common.h"
uint64_t vaddr_ifetch(uint64_t addr, int len);
uint64_t vaddr_read(uint64_t addr, int len);
void vaddr_write(uint64_t addr, int len, uint64_t data);
uint64_t vaddr_to_paddr(uint64_t addr,int len,int type);

/* convert the guest physical address in the guest program to host virtual address in NEMU */
uint8_t* guest_to_host(uint64_t paddr);
uint64_t paddr_read(uint64_t addr, int len);
void paddr_write(uint64_t addr, int len, uint64_t data);
uint64_t init_mem(char* img_file);
void clear_mem();
void load_kernel(char* img_file);
void load_fs(char* img_file);
#endif