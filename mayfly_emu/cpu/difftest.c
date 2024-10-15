#include "difftest.h"
#include <dlfcn.h>
#include "../memory/memory.h"
#include "../monitor/sdb.h"
#include "../system/system.h"
#include "mayfly_cpu.h"
//--------------------------------------DUT Difftest------------------------------------------------
void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(void *dut, bool direction) = NULL;

void init_difftest(char *ref_so_file,long img_size){
	assert(ref_so_file != NULL);

	void *handle;
	handle = dlopen(ref_so_file, RTLD_LAZY | RTLD_DEEPBIND);
	assert(handle);

  	ref_difftest_memcpy = (void (*)(paddr_t, void*, size_t, bool))dlsym(handle, "difftest_memcpy");
  	assert(ref_difftest_memcpy);

  	ref_difftest_regcpy = (void (*)(void *dut, bool directio))dlsym(handle, "difftest_regcpy");
  	assert(ref_difftest_regcpy);

  	ref_difftest_exec = (void (*)(uint64_t n))dlsym(handle, "difftest_exec");
  	assert(ref_difftest_exec);

  	ref_difftest_raise_intr = (void (*)(void *dut, bool direction))dlsym(handle, "difftest_raise_intr");
  	assert(ref_difftest_raise_intr);

  	void (*ref_difftest_init)(int) =(void (*)(int)) dlsym(handle, "difftest_init");
  	assert(ref_difftest_init);

	printf("Differential testing: %s",ASNI_FMT("ON", ASNI_FG_GREEN));
	printf("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);
	
	ref_difftest_init(0);
	ref_difftest_memcpy(0x80000000,guest_to_host(0x80000000), img_size, DIFFTEST_TO_REF);
	ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
};

void ref_csr_reg_display(riscv64_CPU_state *ref_r){
	printf("mstatus = %lx, mtvec = %lx, medeleg = %lx, mideleg = %lx\n",ref_r->csr[0],ref_r->csr[1],ref_r->csr[2],ref_r->csr[3]);
	printf("mip = %lx, mie = %lx, mscratch = %lx, mepc = %lx \n",ref_r->csr[4],ref_r->csr[5],ref_r->csr[6],ref_r->csr[7]);
	printf("mcause = %lx, mtval = %lx, stvec = %lx,scounteren = %lx \n",ref_r->csr[8],ref_r->csr[9],ref_r->csr[10],ref_r->csr[11]);
	printf("sscratch = %lx, sepc = %lx, scause = %lx, stval = %lx\n",ref_r->csr[12],ref_r->csr[13],ref_r->csr[14],ref_r->csr[15]);
	printf("satp = %lx, mcycle = %lx,minstret = %lx\n",ref_r->csr[16],ref_r->csr[24],ref_r->csr[23]);
}

void isa_ref_csr_reg_display(){
	riscv64_CPU_state ref_r;
	ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
	ref_csr_reg_display(&ref_r);
}

static void checkregs(riscv64_CPU_state *ref,uint64_t pc){
	if(!isa_difftest_checkregs(ref,cpu.pc,pc)){
		mayfly_state.state	= MAYFLY_ABORT;
		mayfly_state.halt_pc	= pc;
		printf("-----reg-------\n");
		isa_reg_display();
		printf("-----dut csr-----\n");
		isa_csr_reg_display();
		printf("-----ref csr------\n");
		ref_csr_reg_display(ref);
		show_iringbuf_info();
	}
}

void difftest_step(uint64_t pc, bool is_irq, bool is_mmio){
	if(is_mmio | cpu.is_diff_skip){
		ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
		//printf("skip one inst\n");
		return;
	}
	if(is_irq){
		ref_difftest_exec(1);
		ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
		return;
	}
	riscv64_CPU_state ref_r;
	ref_difftest_exec(1);
	ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
	checkregs(&ref_r,pc);
}

//------------------------------------------REF Difftest--------------------------------------------
//extern CPU_state cpu;
void difftest_memcpy(uint64_t addr, void *buf, int n, bool direction)
{	
	//printf("ref difftest_memcpy %lx\n",addr);
	if(direction == true){
		for(int i = 0; i < n; i++){
			paddr_write(addr + i,1,*((uint8_t*)buf + i));
		}
	}else{
		assert(0);
	}
}

void difftest_regcpy(void *dut, bool direction)
{
	riscv64_CPU_state* temp = (riscv64_CPU_state*)dut;
	if(direction == false){
		memcpy(temp->gpr,cpu.gpr,sizeof(cpu.gpr));
		temp->pc = cpu.pc;
		temp->pre_pc = cpu.pre_pc;
		temp->mode = cpu.mode;
		memcpy(temp->csr,cpu.csr,sizeof(cpu.csr));
	}else{	// to ref 
		memcpy(cpu.gpr,temp->gpr,sizeof(cpu.gpr));

		uint64_t temp_s = cpu.csr[4];
		memcpy(cpu.csr,temp->csr,sizeof(cpu.csr));
		cpu.csr[4] = temp_s;

		cpu.pc = temp->pc;
		cpu.pre_pc = temp->pre_pc;
		//cpu.mode = temp->mode;
	}
}

void difftest_only_gprcpy(void *dut){
	riscv64_CPU_state* temp = (riscv64_CPU_state*)dut;
	memcpy(cpu.gpr,temp->gpr,sizeof(cpu.gpr));
	cpu.pc = temp->pc;
	cpu.pre_pc = temp->pre_pc;
}

void difftest_exec(uint64_t n){
	cpu_exec(n);
}

void difftest_raise_intr(void *dut, bool direction){
	riscv64_CPU_state* temp = (riscv64_CPU_state*)dut;
	if(direction == true){
		cpu.csr[4]	= temp->csr[4];
		handle_difftest_irq();
	}
}

void difftest_init(){
  /* Set the initial program counter. */
  cpu.mode = 3;//"b11" M Mode, "b01" S Mode, "b00" U Mode
  cpu.pc = RESET_VECTOR;//RESET_VECTOR;
  cpu.amo = 0;
  cpu.is_irq = 0;
  cpu.is_except = 0;
  //cpu.csr[17]= 0x8000000000141101;
  minstret->val = 0;
  mcycle->val = 0;
  //mstatus->spp = 1;
  mstatus->mpp = 3;
  //mstatus->uxl = 2;
  //mstatus->sxl = 2;
  /* The zero register is always 0. */
  cpu.gpr[0] = 0;
  //clear_mem();
  //paddr_read(0x80000000,4);
  //printf("ref1 r0x81200004  data 0x%lx, pc 0x%lx\n",pmem_read(0x81200004, 4),cpu.pc);
  //printf("ref init_difftest 68\n");
}
