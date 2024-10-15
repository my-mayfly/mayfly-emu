#include "system.h"

#define INTR_BIT (1ULL << 63)
#define INTR_EMPTY ((int64_t)-1)
enum {
  IRQ_USIP, IRQ_SSIP, IRQ_HSIP, IRQ_MSIP,
  IRQ_UTIP, IRQ_STIP, IRQ_HTIP, IRQ_MTIP,
  IRQ_UEIP, IRQ_SEIP, IRQ_HEIP, IRQ_MEIP
};

bool has_intr = false;
bool has_except = false;
bool intr_deleg_S(uint64_t exceptionNO) {
  uint64_t deleg = (exceptionNO & INTR_BIT ? mideleg->val : medeleg->val);
  bool delegS = ((deleg & (1 << (exceptionNO & 0xf))) != 0) && (cpu.mode < MODE_M);
  return delegS;
}

void handle_irq_or_except(){
#if CONFIG_SHARE_SO
    if(cpu.is_except == 1){
		cpu.pc = raise_intr(cpu.m_s_cause,cpu.pre_pc);
	}
#else 
	uint64_t intr;
    if(cpu.is_except == 1){
		cpu.pc = raise_intr(cpu.m_s_cause,cpu.pre_pc);
	}else{

		intr = isa_query_intr();
		if(intr != INTR_EMPTY){
			if(((intr&0xff) == IRQ_SEIP) |((intr&0xff) == IRQ_MEIP))printf("irq is externxx mepc %lx\n",cpu.pc);
			cpu.pc = raise_intr(intr,cpu.pc);
			cpu.is_irq = 1;
		}
	}
#endif
}

void handle_difftest_irq(){
	uint64_t intr;
		intr = isa_query_intr();
		if(intr != INTR_EMPTY){
			if(((intr&0xff) == IRQ_SEIP) |((intr&0xff) == IRQ_MEIP))printf("irq is externxx mepc %lx\n",cpu.pc);
			cpu.pc = raise_intr(intr,cpu.pc);
			cpu.is_irq = 1;
		}
}

static uint64_t get_trap_pc(uint64_t xtvec, uint64_t xcause){
  uint64_t base = (xtvec >> 2)<<2;
  uint64_t mode = (xtvec & 0x1); // bit 1 is reserved, dont care here.
  bool is_intr = (xcause >> (sizeof(uint64_t)*8 - 1)) == 1;
  uint64_t casue_no = xcause & 0xf;
  return (is_intr && mode==1) ? (base + (casue_no << 2)) : base;
}

uint64_t raise_intr(uint64_t NO, uint64_t epc){
  switch(NO){
    case EX_II:
    case EX_IPF:
    case EX_LPF:
    case EX_SPF: break;
  }
  bool delegS = intr_deleg_S(NO);

  if(delegS){
    scause->val = NO;
    sepc->val = epc;
    mstatus->spp = cpu.mode;
    mstatus->spie = mstatus->sie;
    mstatus->sie = 0;
	stval->val = cpu.m_s_tval;
    switch(NO){
      case EX_IPF: case EX_LPF: case EX_SPF:
      case EX_LAM: case EX_SAM:
      case EX_IAF: case EX_LAF: case EX_SAF:
        break;
      default: stval->val = 0;      
    }
    cpu.mode = MODE_S;
	//printf("S mstatus 0x%lx\n",mstatus->val);
    return get_trap_pc(stvec->val, scause->val);
  }else{
    mcause->val = NO;
    mepc->val = epc;
    mstatus->mpp = cpu.mode;
    mstatus->mpie = mstatus->mie;
    mstatus->mie = 0;
	mtval->val = cpu.m_s_tval;
	//printf("M mstatus 0x%lx\n",mstatus->val);
    switch (NO) {
      case EX_IPF: case EX_LPF: case EX_SPF:
      case EX_LAM: case EX_SAM:
      case EX_IAF: case EX_LAF: case EX_SAF:
        break;
      default: mtval->val = 0;
    }
    cpu.mode = MODE_M;
    return get_trap_pc(mtvec->val, mcause->val);
    // return mtvec->val;    
  }
}

uint64_t isa_query_intr(){
  uint64_t intr_vec = m_mie->val & mip->val;
  if(!intr_vec) return INTR_EMPTY;

  const int priority [] = {
    IRQ_MEIP, IRQ_MSIP, IRQ_MTIP,
    IRQ_SEIP, IRQ_SSIP, IRQ_STIP,
    IRQ_UEIP, IRQ_USIP, IRQ_UTIP    
  };
  int i;
  for (i = 0; i < 9; i++){
    int irq = priority[i];
    if (intr_vec & (1 << irq)){
      bool deleg = (mideleg->val & (1 << irq)) != 0;
      bool global_enable = (deleg ? ((cpu.mode == MODE_S) && mstatus->sie) || (cpu.mode < MODE_S) :
          ((cpu.mode == MODE_M) && mstatus->mie) || (cpu.mode < MODE_M));
      if (global_enable) return irq | INTR_BIT;
    }
  }
  return INTR_EMPTY;
}