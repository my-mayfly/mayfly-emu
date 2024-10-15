#ifndef __MAYFLY_CPU_H
#define __MAYFLY_CPU_H
#include "decode.h"
int isa_exec_once(Decode *s);
void invalid_inst(uint64_t thispc,uint32_t inst);
void set_mayfly_state(int state, uint64_t pc, int halt_ret);
void cpu_exec(uint64_t n);
void show_iringbuf_info();
#define MAYFLYTRAP(thispc, code) set_mayfly_state(MAYFLY_END, thispc, code)
#endif