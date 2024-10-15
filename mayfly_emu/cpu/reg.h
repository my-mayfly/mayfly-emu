#ifndef __RISCV64_REG_H__
#define __RISCV64_REG_H__
#include "../common.h"
static struct csr_map{
    const char *csr_name;
    int csr_addr;
    int csr_map_idx;
}csr_maps [] = {
	{"mstatus",   0x300, 0},
	{"mtvec",     0x305, 1},
	{"medeleg",   0x302, 2},
	{"mideleg",   0x303, 3},
	{"mip",       0x344, 4},
	{"mie",       0x304, 5},
	{"mscratch",  0x340, 6},
	{"mepc",      0x341, 7},
	{"mcause",    0x342, 8},
	{"mtval",     0x343, 9},

	{"stvec",     0x105, 10},
	{"scounteren",0x106, 11},
	{"sscratch",  0x140, 12},
	{"sepc",	  0x141, 13},
	{"scause",	  0x142, 14},
	{"stval",	  0x143, 15},
	{"satp",	  0x180, 16},
	{"sstatus",   0x100, 0},
	{"sip",		  0x144, 4},
	{"sie",		  0x104, 5},

	{"misa",      0x301, 17},

    {"mvendorid", 0xf11, 18},
    {"marchid",   0xf12, 19},
    {"mimpid",    0xf13, 20},
    {"mhartid",   0xf14, 21},
    {"minstret", 0x34A, 23},
	{"mcycle",    0xb00, 24}
};

#define CSR_NUMBER (int)(sizeof(csr_maps) / sizeof(csr_maps[0]))

static inline int check_reg_idx(int idx) {
    assert(idx >= 0 && idx < 32);
    return idx;
}

static inline int check_csr_idx(int idx) {
    int ret = -1;
    for( int i =0;i< CSR_NUMBER; i++ ){
        if(csr_maps[i].csr_addr == idx){ret = csr_maps[i].csr_map_idx;break;}
    }

	if((ret == -1)||((cpu.mode != MODE_M) && (idx > 0x200))){
		printf("mode error or bad csr addr 0x%x\n",idx);
		cpu.is_except = 1;
		if(ret == -1){cpu.is_diff_skip = 1;}
		return 0;
	}
	// if(ret == -1){
	// 	printf("no csr addr 0x%x\n",idx);
	// }
    // //assert(ret != -1);
    return ret;
}

static inline uint64_t csr_read(int idx){
	//printf("r idx %x,pc %lx\n",idx,cpu.pc);
	uint64_t temp = 0;
	switch(idx){
		case 0x300:temp = cpu.csr[0];break;
		case 0x305:temp = cpu.csr[1];break;
		case 0x302:temp = cpu.csr[2];break;
		case 0x303:temp = cpu.csr[3];break;
		case 0x344:temp = cpu.csr[4];break;
		case 0x304:temp = cpu.csr[5];break;
		case 0x340:temp = cpu.csr[6];break;
		case 0x341:temp = cpu.csr[7];break;
		case 0x342:temp = cpu.csr[8];break;
		case 0x343:temp = cpu.csr[9];break;

		case 0x105:temp = cpu.csr[10];break;
		case 0x106:temp = cpu.csr[11];break;
		case 0x140:temp = cpu.csr[12];break;
		case 0x141:temp = cpu.csr[13];break;
		case 0x142:temp = cpu.csr[14];break;
		case 0x143:temp = cpu.csr[15];break;
		case 0x180:temp = cpu.csr[16];break;
		case 0x100:temp = cpu.csr[0] & 0x80000003000de762;break;
		case 0x144:temp = cpu.csr[4] & 0x222;break;
		case 0x104:temp = cpu.csr[5] & 0x222;break;

		case 0x301:temp = 0x8000000000141101;break;
		case 0xf11:temp = 0;break;
		case 0xf12:temp = 0;break;
		case 0xf13:temp = 0;break;
		case 0xf14:temp = 0;break;
		case 0xb02:temp = cpu.csr[23];break; //case 0x34A:temp = cpu.csr[23];break;
		case 0xb00:temp = cpu.csr[24];printf("read mcycle\n");break;
		default:cpu.is_except = 1;cpu.is_diff_skip = 1;break;
	}
	if((idx>0x200)&& cpu.mode != MODE_M){cpu.is_except =1;}
	return temp;
}
static inline void csr_write(int idx,uint64_t wdata){
	//printf("w idx %x,wdata %lx,pc %lx\n",idx,wdata,cpu.pc);
	switch(idx){									
		case 0x300:cpu.csr[0] = (cpu.csr[0] & ~0x80000030007fffea)|(wdata&0x80000030007fffea);break;
		case 0x305:cpu.csr[1] = wdata;break;
		case 0x302:cpu.csr[2] = wdata;break;
		case 0x303:cpu.csr[3] = wdata;break;
		case 0x344:cpu.csr[4] = wdata;break;
		case 0x304:cpu.csr[5] = wdata;break;
		case 0x340:cpu.csr[6] = wdata;break;
		case 0x341:cpu.csr[7] = wdata;break;
		case 0x342:cpu.csr[8] = wdata;break;
		case 0x343:cpu.csr[9] = wdata;break;

		case 0x105:cpu.csr[10] = wdata;break;
		case 0x106:cpu.csr[11] = wdata;break;
		case 0x140:cpu.csr[12] = wdata;break;
		case 0x141:cpu.csr[13] = wdata;break;
		case 0x142:cpu.csr[14] = wdata;break;
		case 0x143:cpu.csr[15] = wdata;break;
		case 0x180:if((wdata>>60 ) == 8){cpu.csr[16] = (cpu.csr[0] & ~0xf0000fffffffffff)|(wdata & 0xf0000fffffffffff);};break;
		case 0x100:cpu.csr[0] = (cpu.csr[0] & ~0x80000000000de762)|(wdata & 0x80000000000de762);break;
		case 0x144:cpu.csr[4] = (cpu.csr[4] & ~0x222)|(wdata & 0x222);break;
		case 0x104:cpu.csr[5] = (cpu.csr[5] & ~0x222)|(wdata & 0x222);break;

		case 0x301:break;
		case 0xf11:break;
		case 0xf12:break;
		case 0xf13:break;
		case 0xf14:break;
		case 0xb02:break; //case 0x34A:break;
		case 0xb00:break;
		default:printf("w idx %d\n",idx);cpu.is_except = 1;cpu.is_diff_skip = 1;break;
	}
	if((idx>0x200)&& cpu.mode != MODE_M){cpu.is_except =1;}
}
#define gpr(idx) (cpu.gpr[idx])
//#define csr(idx) (cpu.csr[check_csr_idx(idx)])
static inline const char* reg_name(int idx, int width) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}
#endif