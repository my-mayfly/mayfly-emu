#include "memory.h"
#include "../device/device.h"
#include "../cpu/mayfly_cpu.h"
#include "../common.h"

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
static uint64_t imgfile_size = 0;
uint8_t* guest_to_host(uint64_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint64_t init_mem(char* img_file){
    if(img_file == NULL){
        printf("img_file is NULL \n");
        assert(0);
    }
    printf("img_file path : %s\n",img_file);
    FILE *fp = fopen(img_file, "rb");
    if(fp == NULL){
        printf("can not open %s !!!\n",img_file);
        assert(0);
    }
    fseek(fp,0,SEEK_END);
    imgfile_size = ftell(fp);
    if(imgfile_size > CONFIG_MSIZE){
        printf("!!!!!!!!!!! img size over the ram !!!!!!!!!!!!!!!!!\n");
    }
    fseek(fp,0,SEEK_SET);
    int ret = fread(pmem,imgfile_size,1,fp);    
    if(ret != 1){
        printf("error fread ret %d\n",ret);
        assert(0);
    }
    fclose(fp);
	printf(" see %x\n",*(uint32_t *)(pmem+0x200000));
	printf(" size %lx\n",imgfile_size);
    return imgfile_size;
}

void clear_mem(){
    memset(pmem,0,CONFIG_MSIZE);
}

void load_kernel(char* img_file){
    if(img_file == NULL){
        printf("img_file is NULL \n");
        assert(0);
    }
	printf("kernel_file path : %s\n",img_file);
    FILE *fp = fopen(img_file, "rb");
    if(fp == NULL){
        printf("can not open %s !!!\n",img_file);
        assert(0);
    }

    fseek(fp,0,SEEK_END);
    int size = ftell(fp);
    if(size > CONFIG_MSIZE){
        printf("!!!!!!!!!!! img size over the ram !!!!!!!!!!!!!!!!!\n");
    }
    fseek(fp,0,SEEK_SET);

    int ret = fread((pmem + 0x2000000),size,1,fp);    
    if(ret != 1){
        printf("error fread ret %d\n",ret);
        assert(0);
    }
    fclose(fp);
}

void load_fs(char* img_file){
    if(img_file == NULL){
        printf("img_file is NULL \n");
        assert(0);
    }
	printf("file-system path : %s\n",img_file);
    FILE *fp = fopen(img_file, "rb");
    if(fp == NULL){
        printf("can not open %s !!!\n",img_file);
        assert(0);
    }
    fseek(fp,0,SEEK_END);
    int size = ftell(fp);
    if(size > CONFIG_MSIZE){
        printf("!!!!!!!!!!! img size over the ram !!!!!!!!!!!!!!!!!\n");
    }
    fseek(fp,0,SEEK_SET);

    int ret = fread((pmem + 0x3000000),size,1,fp);    
    if(ret != 1){
        printf("error fread ret %d\n",ret);
        assert(0);
    }
    fclose(fp);
}

static inline bool in_pmem(uint64_t addr) {
  return (addr >= CONFIG_MBASE) && (addr < (uint64_t)CONFIG_MBASE + CONFIG_MSIZE);
}

static inline uint64_t host_read(void *addr, int len){
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    case 8: return *(uint64_t *)addr;
    default: return 0;
  }    
}

static inline void host_write(void *addr, int len, uint64_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    case 8: *(uint64_t *)addr = data; return;
    default: assert(0);
  }
}

static uint64_t pmem_read(uint64_t addr, int len){
  uint64_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(uint64_t addr, int len, uint64_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(uint64_t addr) {
  printf("address = 0x%lx,is out of bound of pmem at pc = 0x%lx\n",addr,cpu.pc);
  mayfly_state.state = MAYFLY_ABORT;
  show_iringbuf_info();
}

uint64_t paddr_read(uint64_t addr, int len){
    if (likely(in_pmem(addr))) return pmem_read(addr, len);
    cpu.is_mmio = 1;
  if (MAYFLY_UART_ADDR<=addr && addr <= (MAYFLY_UART_ADDR +0xffff)){
    return serail_read(addr - MAYFLY_UART_ADDR,len);
  }
  if(MAYFLY_CLINT_ADDR<=addr && addr <= (MAYFLY_CLINT_ADDR + 0xffff)){
    return aclint_read(addr - MAYFLY_CLINT_ADDR,len);
  }
  if(MAYFLY_PLIC_ADDR<=addr && addr <= (MAYFLY_PLIC_ADDR + 0x300000)){
    return plic_read(addr - MAYFLY_PLIC_ADDR,len);
  }
  if(MAYFLY_FLASH_ADDR<=addr && addr <= (MAYFLY_FLASH_ADDR + 0xfffffff)){
	return flash_read(addr - MAYFLY_FLASH_ADDR,len);
  }
  if(MAYFLY_GPIO_ADDR<=addr && addr <= (MAYFLY_GPIO_ADDR + 0xffffff)){
	return 0;
  }
  out_of_bound(addr);
  return 0;
}

void paddr_write(uint64_t addr, int len, uint64_t data){
	if (likely(in_pmem(addr))) {pmem_write(addr, len, data); return; }
	cpu.is_mmio = 1;
    if (MAYFLY_UART_ADDR<=addr && addr <= (MAYFLY_UART_ADDR +0xffff)){
        serail_write(addr - MAYFLY_UART_ADDR,len,data);
		return;
    }
    if(MAYFLY_CLINT_ADDR<=addr && addr <= (MAYFLY_CLINT_ADDR + 0xffff)){
        aclint_write(addr - MAYFLY_CLINT_ADDR,len,data);
		return;
    }
    if(MAYFLY_PLIC_ADDR<=addr && addr <= (MAYFLY_PLIC_ADDR + 0x300000)){
        plic_write(addr - MAYFLY_PLIC_ADDR,len,data);
		return;
    }
	if(MAYFLY_FLASH_ADDR<=addr && addr <= (MAYFLY_FLASH_ADDR + 0xfffffff)){
		flash_write(addr - MAYFLY_FLASH_ADDR,len,data);
		return;
	}
    if(MAYFLY_GPIO_ADDR<=addr && addr <= (MAYFLY_GPIO_ADDR + 0xffffff)){
	    return ;
    }
    out_of_bound(addr);
}