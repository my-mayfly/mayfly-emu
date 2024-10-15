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
	// if(addr == 0xffffffe00040329c){
	// 	printf("soc_early_init  \n");
	// }else if(addr == 0xffffffe000c08440){
	// 	printf("init_task \n");
	// }else if(addr == 0xffffffe0000000b0){
	// 	printf("setup_trap_vector \n");
	// }else if(addr == 0xffffffe000400a18){
	// 	printf("start_kernel \n");
	// }else if(addr == 0xffffffe000007d2c){
	// 	printf("set_task_stack_end_magic \n");
	// }else if(addr == 0xffffffe0004009b8){
	// 	printf("smp_setup_processor_id\n");
	// }else if(addr == 0xffffffe000404c70){
	// 	printf("boot_cpu_init \n");
	// }else if(addr == 0xffffffe0002c4738){
	// 	printf("printk\n");
	// }else if(addr == 0xffffffe0004008f4){
	// 	printf("parse_early_param\n");
	// }else if(addr == 0xffffffe000417858){
	// 	printf("unflatten_device_tree\n");
	// }else if(addr == 0xffffffe0004140b4){ 
	// 	printf("early_uartlite_setup\n");
	// }else if(addr == 0xffffffe000413a08){
	// 	printf("setup_earlycon\n");
	// }else if(addr == 0xffffffe000416cdc){
	// 	printf("early_init_dt_scan_chosen_stdout\n");
	// }else if(addr == 0xffffffe000413cd8){
	// 	printf("param_setup_earlycon\n");
	// }else if(addr == 0xffffffe00040039c){
	// 	printf("unknown_bootoption \n");
	// }else if(addr == 0xffffffe000400124){
	// 	printf("do_early_param\n");
	// }else if(addr == 0xffffffe0002a9994){
	// 	printf("fdt_getprop\n");
	// }
	//--------------
	// if(addr == 0x00000000000fa598){
	// 	printf("run\n");
	// }else if(addr == 0xffffffff80601280){
	// 	printf("kernel_init_freeable\n");
	// }else if(addr == 0xffffffff806091b8){
	// 	printf("workqueue_init\n");
	// }else if(addr == 0xffffffff8060ea94){
	// 	printf("init_mm_internals\n");
	// }else if(addr == 0xffffffff803123d4){
	// 	printf("refcount_warn_saturate\n");
	// }else if(addr == 0xffffffff8056b494){
	// 	printf("mutex_lock\n");
	// }else if(addr == 0xffffffff806091f0){
	// 	printf("workqueue_init in mutex_lock after\n");
	// }else if(addr == 0xffffffff800246d8){
	// 	printf("create_worker\n");
	// }else if(addr == 0xffffffff80609340){
	// 	printf("workqueue_init is can end\n");
	// }else if(addr == 0xffffffff80609398){
	// 	printf("workqueue_init can ret\n");
	// }
	// else if(addr == 0x0000000000013c7c){
	// 	printf("full_write\n");
	// }else if(addr == 0x000000000011ba30){
	// 	printf("printf\n");
	// }else if(addr == 0x00000000000fa56c){
	// 	printf("xingk_test1\n");
	// }else if(addr == 0x00000000000fa540){
	// 	printf("xingk_test\n");
	//}
	// else if(addr == 0xffffffff801841d4){
	// 	printf("ksys_write\n");
	// }
	//else if(addr == 0xffffffff80181c6c){
	// 	printf("rw_verify_area\n");
	// }else if(addr == 0xffffffff803f28e0){
	// 	printf("ulite_console_write\n");
	// }else if(addr == 0xffffffff80183d74){
	// 	printf("vfs_write\n");
	// }else if(addr == 0xffffffff803f2ecc){
	// 	printf("ulite_start_tx\n");
	// }else if(addr == 0xffffffff803f25a4){
	// 	printf("ulite_request_port\n");
	// }else if(addr == 0xffffffff803f2380){
	// 	printf("ulite_tx_empty\n");
	// }else if(addr == 0xffffffff803f2360){
	// 	printf("uartlite_outle32\n");
	// }
	// else if(addr == 0xffffffff803f02d4){
	// 	printf("uart_write,a0 %lx\n",cpu.gpr[10]);
	// }else if(addr == 0xffffffff803cc9b4){
	// 	printf("file_tty_write\n");
	// }else if(addr == 0xffffffff803c89c4){
	// 	printf("xingk_signal_pending , a0 0x%lx\n",cpu.gpr[10]);
	// }else if(addr == 0xffffffff803c896c){
	// 	printf("xingk_iov_iter_count, a0 %lx\n",cpu.gpr[10]);
	// }else if(addr == 0xffffffff803cc6f0){
	// 	printf("do_tty_write_xingk\n");
	// }else if(addr == 0xffffffff803c8990){
	// 	printf("xingk_line, a0 %lx\n",cpu.gpr[10]);
	// }else if(addr == 0xffffffff803c8b20){
	// 	printf("tty_put_char\n");
	// }else if(addr == 0xffffffff803cef10){
	// 	printf("n_tty_write\n");
	// }else if(addr == 0xffffffff803f370c){
	// 	printf("ulite_isr\n");
	// }else if(addr == 0xffffffff803f2f0c){
	// 	printf("ulite_transmit\n");
	// }
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

