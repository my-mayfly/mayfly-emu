#ifndef __SDB_H__
#define __SDB_H__
#include "../common.h"
uint64_t expr(char *e, bool *success);
void init_regex();
void create_watchpoint(char *str);
void delete_watchpoint(int No);
void display_watchpoint();
bool calculate_watchpoint();
void init_sdb();
void sdb_mainloop();

bool isa_difftest_checkregs(riscv64_CPU_state *ref_r,uint64_t next_pc,uint64_t current_pc);
void isa_reg_display();
void isa_csr_reg_display();
#endif