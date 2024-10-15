/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "mmu.h"
#include "sim.h"
//#include "../../include/common.h"
//#include <difftest-def.h>
//#include "../../common.h"
enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };
typedef uint64_t word_t;
typedef int64_t sword_t;

typedef word_t vaddr_t;
typedef uint64_t paddr_t;
#define __EXPORT __attribute__((visibility("default")))
#define CONFIG_MBASE 0x80000000
#define CONFIG_MSIZE 0x8000000

#define CONFIG_RV64 //xingk
#define NR_GPR  32

static std::vector<std::pair<reg_t, abstract_device_t*>> difftest_plugin_devices;
static std::vector<std::string> difftest_htif_args;
static std::vector<std::pair<reg_t, mem_t*>> difftest_mem(
    1, std::make_pair(reg_t(DRAM_BASE), new mem_t(CONFIG_MSIZE)));
static debug_module_config_t difftest_dm_config = {
  .progbufsize = 2,
  .max_sba_data_width = 0,
  .require_authentication = false,
  .abstract_rti = 0,
  .support_hasel = true,
  .support_abstract_csr_access = true,
  .support_abstract_fpr_access = true,
  .support_haltgroups = true,
  .support_impebreak = true
};

struct diff_context_t {
  word_t gpr[32];
  word_t pc;
  word_t pre_pc;
  uint8_t mode;
  word_t mstatus;
  word_t mtvec;
  word_t medeleg;
  word_t mideleg;
  word_t mip;
  word_t mie;
  word_t mscratch;
  word_t mepc;
  word_t mcause;
  word_t mtval;

  word_t stvec;
  word_t scounteren;
  word_t sscratch;
  word_t sepc;
  word_t scause;
  word_t stval;
  word_t satp;
};

static sim_t* s = NULL;
static processor_t *p = NULL;
static state_t *state = NULL;

void sim_t::diff_init(int port) {
  p = get_core("0");
  state = p->get_state();
}

void sim_t::diff_step(uint64_t n) {
  step(n);
}

void sim_t::diff_get_regs(void* diff_context) {
  struct diff_context_t* ctx = (struct diff_context_t*)diff_context;
  for (int i = 0; i < NR_GPR; i++) {
    ctx->gpr[i] = state->XPR[i];
  }
  ctx->mode = state->prv;
  ctx->mstatus 	= state->mstatus->read();
  ctx->mtvec	 	= state->mtvec->read();
  ctx->medeleg 	= state->medeleg->read();
  ctx->mideleg 	= state->mideleg->read();
  ctx->mip	 	= state->mip->read();
  ctx->mie	 	= state->mie->read();
  ctx->mscratch 	= 0;//state->mscratch;
  ctx->mepc	  	= state->mepc->read();
  ctx->mcause	  	= state->mcause->read();
  ctx->mtval 	  	= state->mtval->read();

  ctx->stvec		= state->stvec->read();
  ctx->scounteren = state->scounteren->read();
  ctx->sscratch	= 0;//state->sscratch;
  ctx->sepc		= state->sepc->read();
  ctx->scause		= state->scause->read();
  ctx->stval		= state->stval->read();
  ctx->satp		= state->satp->read();
  //ctx->pc = state->pc;
  ctx->pc = state->pc;
}

void sim_t::diff_set_regs(void* diff_context) {
  struct diff_context_t* ctx = (struct diff_context_t*)diff_context;
  for (int i = 0; i < NR_GPR; i++) {
    state->XPR.write(i, (sword_t)ctx->gpr[i]);
  }
	state->prv 		= ctx->mode;
	//state->mstatus	= ctx->mstatus;
  state->mstatus->write((reg_t)(ctx->mstatus));
	state->mtvec->write((reg_t)(ctx->mtvec));	//= ctx->mtvec;
	state->medeleg->write((reg_t)(ctx->medeleg));	//= ctx->medeleg;
	state->mideleg->write((reg_t)(ctx->mideleg));	//= ctx->mideleg;
	state->mip->write((reg_t)(ctx->mip));		//= ctx->mip;
	state->mie->write((reg_t)(ctx->mie));		//= ctx->mie;
	//state->mscratch->write((reg_t)(ctx->mstatus));	= ctx->mscratch;
	state->mepc->write((reg_t)(ctx->mepc));		//= ctx->mepc;
	state->mcause->write((reg_t)(ctx->mcause));	//= ctx->mcause;
	state->mtval->write((reg_t)(ctx->mtval));	//= ctx->mtval;

	state->stvec->write((reg_t)(ctx->stvec));		//= ctx->stvec;
	state->scounteren->write((reg_t)(ctx->scounteren)); 	//= ctx->scounteren;
	//state->sscratch->write((reg_t)(ctx->mstatus));		= ctx->sscratch;
	state->sepc->write((reg_t)(ctx->sepc));			//= ctx->sepc;
	state->scause->write((reg_t)(ctx->scause));		//= ctx->scause;
	state->stval->write((reg_t)(ctx->stval));		//= ctx->stval;
	state->satp->write((reg_t)(ctx->satp));			//= ctx->satp;

  state->pc = ctx->pc;
}

void sim_t::diff_memcpy(reg_t dest, void* src, size_t n) {
  mmu_t* mmu = p->get_mmu();
  for (size_t i = 0; i < n; i++) {
    mmu->store<uint8_t>(dest+i, *((uint8_t*)src+i));
  }
}

extern "C" {

__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    s->diff_memcpy(addr, buf, n);
  } else {
    assert(0);
  }
}

__EXPORT void difftest_regcpy(void* dut, bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    s->diff_set_regs(dut);
  } else {
    s->diff_get_regs(dut);
  }
}

__EXPORT void difftest_exec(uint64_t n) {
  s->diff_step(n);
}

__EXPORT void difftest_init(int port) {
  difftest_htif_args.push_back("");
  const char *isa = "RV" "64" "I" "MAFDC";
  cfg_t cfg(/*default_initrd_bounds=*/std::make_pair((reg_t)0, (reg_t)0),
            /*default_bootargs=*/nullptr,
            /*default_isa=*/isa,
            /*default_priv=*/DEFAULT_PRIV,
            /*default_varch=*/DEFAULT_VARCH,
            /*default_misaligned=*/false,
            /*default_endianness*/endianness_little,
            /*default_pmpregions=*/16,
            /*default_mem_layout=*/std::vector<mem_cfg_t>(),
            /*default_hartids=*/std::vector<size_t>(1),
            /*default_real_time_clint=*/false,
            /*default_trigger_count=*/4);
  s = new sim_t(&cfg, false,
      difftest_mem, difftest_plugin_devices, difftest_htif_args,
      difftest_dm_config, nullptr, false, NULL,
      false,
      NULL,
      true);
  s->diff_init(port);
}

__EXPORT void difftest_raise_intr(uint64_t NO) {
  trap_t t(NO);
  p->take_trap_public(t, state->pc);
}

}