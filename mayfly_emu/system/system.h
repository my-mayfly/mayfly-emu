#ifndef __SYSTEM_H 
#define __SYSTEM_H
#include "../common.h"
void handle_difftest_irq();
uint64_t raise_intr(uint64_t NO, uint64_t epc);
bool intr_deleg_S(uint64_t exceptionNO);
void handle_irq_or_except();
int isa_mmu_check(uint64_t vaddr, int len, int type);
uint64_t isa_mmu_translate(uint64_t vaddr, int len, int type);
uint64_t raise_intr(uint64_t NO, uint64_t epc);
uint64_t isa_query_intr();
#define INTR_TVAL_REG(ex) (*((intr_deleg_S(ex)) ? (uint64_t *)stval : (uint64_t *)mtval))
#endif 