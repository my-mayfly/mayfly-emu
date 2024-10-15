#ifndef __COMMON_H
#define __COMMON_H
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "./cpu/mayfly_isa.h"
#include "./cpu/macro.h"
extern uint64_t cycle;
extern riscv64_CPU_state cpu;
extern MAYFLYState mayfly_state;
#define CPU_HZ 10000000 //假设CPU的频率为10MHz

#define CONFIG_MBASE 0x80000000
#define CONFIG_MSIZE 0x8000000

#define MISA_DATA       
#define MVENDORID_DATA
#define MARCHID_DATA 
#define MIMPID_DATA 
#define MHARTID_DATA 

#define mstatus    ((mstatus_t *)&cpu.csr[0])
#define mtvec      ((mtvec_t *)(&cpu.csr[1]))
#define medeleg    ((medeleg_t *)&cpu.csr[2])
#define mideleg    ((mideleg_t *)&cpu.csr[3])
#define mip        ((mip_t *)&cpu.csr[4])
#define m_mie      ((mie_t *)&cpu.csr[5])
#define mscratch   ((mscratch_t *)&cpu.csr[6])
#define mepc       ((mepc_t *)&cpu.csr[7])
#define mcause     ((mcause_t *)&cpu.csr[8])
#define mtval      ((mtval_t *)&cpu.csr[9])
#define mcycle     ((mcycle_t *)&cpu.csr[24])
#define minstret   ((minstret_t *)&cpu.csr[23])


//#define SSTATUS_DATA 
#define stvec      ((stvec_t *)&cpu.csr[10])
//#define SIP_DATA
//#define SIE_DATA 
#define scounteren  ((scounteren_t *)&cpu.csr[11])
#define sscratch    ((sscratch_t 8)&cpu.csr[12])
#define sepc       ((sepc_t *)&cpu.csr[13])
#define scause     ((scause_t *)&cpu.csr[14])
#define stval      ((stval_t *)&cpu.csr[15])
#define satp       ((satp_t *)&cpu.csr[16])

typedef uint64_t word_t;
typedef int64_t sword_t;

typedef word_t vaddr_t;
typedef uint64_t paddr_t;
enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

#define CONFIG_SHARE_SO 0
#define CONFIG_FLASH 0


//根据emu作为动态库，还是作为程序，判断是否需要开启DiffTest和指令追踪
#if CONFIG_SHARE_SO == 1

#else 
    #define CONFIG_ITRACE   // 是否开启指令追踪
//    #define CONFIG_DIFFTEST  // 是否开启DiffTest，这个的DiffTest是与spike对比的
#endif

//根据选择的启动方式，定义CPU的冷复位地址。
#if CONFIG_FLASH == 1
    #define RESET_VECTOR 0x30000000
#else 
    #define RESET_VECTOR 0x80000000
#endif 

#endif 