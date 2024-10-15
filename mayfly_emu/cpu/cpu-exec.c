#include "mayfly_cpu.h"
#include "../common.h"
#include "../memory/memory.h"
#include "./utils.h"
#include "../device/device.h"
#include "../system/system.h"
#include "../utils/disasm.h"
#include "difftest.h"
#include <unistd.h>

uint64_t iringbuf_pc[16] = {0};
uint64_t iringbuf_inst[16] = {0};
uint64_t iringbuf_counter = 0;
void show_iringbuf_info(){
#ifdef CONFIG_ITRACE
  char instbuf[100]={0};
  char *p;
  for(int i = 0; i<16; i++){
	  p = instbuf;
      if((iringbuf_counter%16) == i){
		  disassemble(p,90,iringbuf_pc[i],(uint8_t *)&iringbuf_inst[i],4);

          printf("0x%lx, %s <------\n",iringbuf_pc[i],instbuf);
      }else{
		  disassemble(p,90,iringbuf_pc[i],(uint8_t *)&iringbuf_inst[i],4);
		  printf("0x%lx, %s\n",iringbuf_pc[i],instbuf);
      }
  }
#endif
}
void set_mayfly_state(int state, uint64_t pc, int halt_ret) {
	mayfly_state.state = state;
	mayfly_state.halt_pc = pc;
	mayfly_state.halt_ret = halt_ret;
};

__attribute__((noinline)) 
void invalid_inst(uint64_t thispc,uint32_t inst){
    printf(" invalid opcode(PC =  0x%lx, inst = 0x%x\n",thispc,inst);
    set_mayfly_state(MAYFLY_ABORT, thispc, -1);
}

extern int read_noblock_stdin_fd;
int get_stop(){
    int r;
    unsigned char c;
    
    if ((r = read(read_noblock_stdin_fd, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }  
}

static void exec_once(Decode *s, uint64_t pc){

    s->pc = pc;
    s->snpc = pc;

    isa_exec_once(s);

	iringbuf_pc[iringbuf_counter%16] = pc;
	iringbuf_inst[iringbuf_counter%16] = s->isa;

    cpu.pre_pc = pc;
    cpu.pc  = s->dnpc;

#if CONFIG_SHARE_SO == 1
	handle_irq_or_except();
#else 
    update_device();
    handle_irq_or_except();
    int ret = get_stop();
    if(ret == 27){
      mayfly_state.state = MAYFLY_STOP;
      //printf("ss,%d\n",ret);
    }else{
      //printf("ss1,%d\n",ret);
    }
#endif

#ifdef CONFIG_DIFFTEST 
	difftest_step(cpu.pre_pc,cpu.is_irq,cpu.is_mmio);
#endif
	iringbuf_counter ++;
	minstret->val = minstret->val +1;
	mcycle->val = mcycle->val + 1;
}

static void execute(uint64_t n){
    Decode s;
    for (;n > 0; n --){
		cpu.is_except 	= 0;
		cpu.is_irq 		= 0;
		cpu.is_mmio 	= 0;
		cpu.is_diff_skip = 0;
		cpu.amo 		= 0;

    exec_once(&s,cpu.pc);
    if (mayfly_state.state != MAYFLY_RUNNING){
      break;
    }
// #if CONFIG_SHARE_SO == 0
//         update_device();
//         handle_irq_or_except();
// #endif
    }
}

void cpu_exec(uint64_t n) {
  switch (mayfly_state.state) {
    case MAYFLY_END: case MAYFLY_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: mayfly_state.state = MAYFLY_RUNNING;
  }
  execute(n);
  switch (mayfly_state.state)
  {
    case MAYFLY_RUNNING: mayfly_state.state = MAYFLY_STOP; break;
    case MAYFLY_END: case MAYFLY_ABORT:
      printf("mayfly: %s at pc = " FMT_WORD,
          (mayfly_state.state == MAYFLY_ABORT ? ASNI_FMT("ABORT", ASNI_FG_RED) :
           (mayfly_state.halt_ret == 0 ? ASNI_FMT("HIT GOOD TRAP", ASNI_FG_GREEN) :
            ASNI_FMT("HIT BAD TRAP", ASNI_FG_RED))),
          mayfly_state.halt_pc);
    case MAYFLY_QUIT: printf("Finish running\n");
  }   
}