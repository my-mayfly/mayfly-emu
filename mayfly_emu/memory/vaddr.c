#include "memory.h"
#include "../system/system.h"
#define PgSHIFT 12 
#define PgMASK ((1ull << PgSHIFT) - 1)

__attribute__((noinline))
static uint64_t vaddr_mmu_read(uint64_t addr,int len,int type){
	uint64_t page_base = isa_mmu_translate(addr,len,type);
	//uint64_t paddr = page_base | (addr & (PgMASK));
	uint64_t rdata = 0;
	if(cpu.pc == 0xffffffff8056995c)printf("vaddr %lx,paddr %lx\n",addr,page_base);
	//printf("vaddr_mmu_read paddr 1 %lx, %llx\n",paddr,(paddr & ((1ull<<12)-1)));
	if((page_base & ((1ull<<12)-1)) == MEM_RET_OK){
		uint64_t paddr = page_base | (addr & (PgMASK));
		//printf("vaddr_mmu_read paddr 2%lx\n",paddr);
		rdata = paddr_read(paddr, len);
	}
	return rdata;
}

__attribute__((noinline))
static void vaddr_mmu_write(uint64_t addr,int len, uint64_t data){
	if((addr & (len - 1)) != 0){
		cpu.is_except = true;
		cpu.m_s_tval  = addr;
		cpu.m_s_cause = EX_SAM;
		return ;
	};
	uint64_t page_base = isa_mmu_translate(addr,len,MEM_TYPE_WRITE);
	//uint64_t paddr = page_base | (addr & (PgMASK));
	if((page_base &((1ull<<12)-1)) == MEM_RET_OK){
		uint64_t paddr = page_base | (addr & (PgMASK));
		paddr_write(paddr, len, data);
	}
}

uint64_t vaddr_ifetch(uint64_t addr, int len){
    int check_type = isa_mmu_check(addr,len,MEM_TYPE_IFETCH);
    uint64_t rdata=0;
    switch(check_type){
        case MMU_DIRECT:rdata = paddr_read(addr, len);break;
        case MMU_TRANSLATE:rdata = vaddr_mmu_read(addr,len,MEM_TYPE_IFETCH);break;
        case MMU_FAIL:printf("vaddr_ifetch vaddr 0x%lx,this situation should not happen!\n",addr);assert(0);break;
        default:break;
    }
	//printf("vaddr_ifetch %lx \n",rdata);
    return rdata;
}

uint64_t vaddr_read(uint64_t addr, int len){
	if((addr & (len - 1)) != 0){
		//cpu.gpr[2]	= 1; ////difftest 以错误的用法强制报错，单作为nemu，要去除这一句
		cpu.is_except = true;
		cpu.m_s_tval  = addr;
		cpu.m_s_cause = EX_LAM;
		return 0;
	};
	int check_type = isa_mmu_check(addr,len,MEM_TYPE_READ);
	uint64_t rdata=0;
	switch(check_type){
		case MMU_DIRECT:rdata = paddr_read(addr, len);break;
		case MMU_TRANSLATE:rdata = vaddr_mmu_read(addr,len,MEM_TYPE_READ);break;
		case MMU_FAIL:printf("vaddr_read vaddr 0x%lx,this situation should not happen!\n",addr);break;
		default:break;
	}
  	return rdata;
}

void vaddr_write(uint64_t addr, int len, uint64_t data){
	if((addr & (len - 1)) != 0){
		//cpu.gpr[2]	= 1; ////difftest 以错误的用法强制报错，单作为nemu，要去除这一句
		cpu.is_except = true;
		cpu.m_s_tval  = addr;
		cpu.m_s_cause = EX_SAM;
	}; //

	int check_type = isa_mmu_check(addr,len,MEM_TYPE_WRITE);
	switch(check_type){
		case MMU_DIRECT:paddr_write(addr, len, data);break;
		case MMU_TRANSLATE:vaddr_mmu_write(addr,len,data);break;
		case MMU_FAIL:printf("vaddr_write vaddr 0x%lx,this situation should not happen!\n",addr);break;
		default:break;
	}    
}

uint64_t vaddr_to_paddr(uint64_t addr,int len,int type){
	int check_type = isa_mmu_check(addr,len,type);
	//printf("gpr[14] %lx\n",cpu.gpr[14]);
	uint64_t paddr = 0;
	switch(check_type){
		case MMU_DIRECT:paddr = addr;break;
		case MMU_TRANSLATE:paddr = (isa_mmu_translate(addr,len,type) | (addr & (PgMASK)));break;
		case MMU_FAIL:printf("vaddr_write vaddr 0x%lx,this situation should not happen!\n",addr);break;
		default:break;
	}
	if(cpu.pc == 0xffffffff8056995c){
		printf("pc %lx,lr/sc vaddr %lx,paddr %lx\n",cpu.pc,addr,paddr);
	}
	
	return paddr;
}

