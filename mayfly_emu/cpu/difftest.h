#ifndef __CPU_DIFFTEST_H__
#define __CPU_DIFFTEST_H__
#include "../common.h"
void init_difftest(char *ref_so_file, long img_size);
void difftest_step(uint64_t pc, bool is_irq, bool is_mmio);
void isa_ref_csr_reg_display();

#endif

