#ifndef __MAYFLY_ISA
#define __MAYFLY_ISA
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "macro.h"
// --------------- state --------------------
enum { MAYFLY_RUNNING, MAYFLY_STOP, MAYFLY_END, MAYFLY_ABORT, MAYFLY_QUIT};
typedef struct{
	int 		state;
	uint64_t	halt_pc;
	uint32_t 	halt_ret;
} MAYFLYState;

typedef struct {
  uint64_t gpr[32];
  uint64_t pc;
  uint64_t pre_pc;
  uint8_t mode;//M mode,S mode,U mode
  uint64_t csr[32]; //difftest中结构体指针强制转换，不应当改变成员变量的顺序。
  
  uint64_t m_s_tval;
  uint8_t  m_s_cause;

  uint8_t amo;
  uint8_t is_irq;
  uint8_t is_except;
  uint8_t is_mmio;
  uint8_t is_diff_skip;
} riscv64_CPU_state;

#define CSR_STRUCT_START(name) \
  typedef union { \
    struct {

#define CSR_STRUCT_END(name) \
    }; \
    uint64_t val; \
  } concat(name, _t);

CSR_STRUCT_START(mtvec)
CSR_STRUCT_END(mtvec)

CSR_STRUCT_START(mstatus)
  uint64_t uie : 1;
  uint64_t sie : 1;
  uint64_t pad0: 1;
  uint64_t mie : 1;
  uint64_t upie: 1;
  uint64_t spie: 1;
  uint64_t pad1: 1;
  uint64_t mpie: 1;
  uint64_t spp : 1;
  uint64_t vs: 2;
  uint64_t mpp : 2;
  uint64_t fs  : 2;
  uint64_t xs  : 2;
  uint64_t mprv: 1;
  uint64_t sum : 1;
  uint64_t mxr : 1;
  uint64_t tvm : 1;
  uint64_t tw  : 1;
  uint64_t tsr : 1;
  uint64_t pad3: 9;
  uint64_t uxl : 2;
  uint64_t sxl : 2;
  uint64_t sbe : 1;
  uint64_t mbe : 1;
  uint64_t pad4:25;
  uint64_t sd  : 1;
CSR_STRUCT_END(mstatus)

CSR_STRUCT_START(mcause)
  uint64_t code:63;
  uint64_t intr: 1;
CSR_STRUCT_END(mcause)

CSR_STRUCT_START(mepc)
CSR_STRUCT_END(mepc)

CSR_STRUCT_START(medeleg)
CSR_STRUCT_END(medeleg)

CSR_STRUCT_START(mideleg)
CSR_STRUCT_END(mideleg)

CSR_STRUCT_START(mscratch)
CSR_STRUCT_END(mscratch)

CSR_STRUCT_START(mtval)
CSR_STRUCT_END(mtval)

CSR_STRUCT_START(mie)
  uint64_t usie : 1;
  uint64_t ssie : 1;
  uint64_t hsie : 1;
  uint64_t msie : 1;
  uint64_t utie : 1;
  uint64_t stie : 1;
  uint64_t htie : 1;
  uint64_t mtie : 1;
  uint64_t ueie : 1;
  uint64_t seie : 1;
  uint64_t heie : 1;
  uint64_t meie : 1;
CSR_STRUCT_END(mie)

CSR_STRUCT_START(mip)
  uint64_t usip : 1;  //0
  uint64_t ssip : 1;  //1
  uint64_t hsip : 1;  //2
  uint64_t msip : 1;  //3
  uint64_t utip : 1;  //4
  uint64_t stip : 1;  //5
  uint64_t htip : 1;  //6
  uint64_t mtip : 1;  //7
  uint64_t ueip : 1;  //8
  uint64_t seip : 1;  //9
  uint64_t heip : 1;  //10
  uint64_t meip : 1;  //11
CSR_STRUCT_END(mip)

CSR_STRUCT_START(mcycle)
CSR_STRUCT_END(mcycle)

CSR_STRUCT_START(minstret)
CSR_STRUCT_END(minstret)

CSR_STRUCT_START(sstatus)
  uint64_t uie : 1;
  uint64_t sie : 1;
  uint64_t pad0: 2;
  uint64_t upie: 1;
  uint64_t spie: 1;
  uint64_t pad1: 2;
  uint64_t spp : 1;
  uint64_t pad2: 4;
CSR_STRUCT_END(sstatus)

CSR_STRUCT_START(stvec)
CSR_STRUCT_END(stvec)

CSR_STRUCT_START(scounteren)
CSR_STRUCT_END(scounteren)

CSR_STRUCT_START(sie)
  uint64_t usie : 1;
  uint64_t ssie : 1;
  uint64_t pad0 : 2;
  uint64_t utie : 1;
  uint64_t stie : 1;
  uint64_t pad1 : 2;
  uint64_t ueie : 1;
  uint64_t seie : 1;
  uint64_t pad2 : 2;
CSR_STRUCT_END(sie)

CSR_STRUCT_START(sip)
  uint64_t usip : 1;
  uint64_t ssip : 1;
  uint64_t pad0 : 2;
  uint64_t utip : 1;
  uint64_t stip : 1;
  uint64_t pad1 : 2;
  uint64_t ueip : 1;
  uint64_t seip : 1;
  uint64_t pad2 : 2;
CSR_STRUCT_END(sip)

CSR_STRUCT_START(satp)
  uint64_t ppn  : 44;
  uint64_t asid : 16;
  uint64_t mode : 4;
CSR_STRUCT_END(satp)

CSR_STRUCT_START(scause)
  uint64_t code:63;
  uint64_t intr: 1;
CSR_STRUCT_END(scause)

CSR_STRUCT_START(sepc)
CSR_STRUCT_END(sepc)

CSR_STRUCT_START(stval)
CSR_STRUCT_END(stval)

CSR_STRUCT_START(sscratch)
CSR_STRUCT_END(sscratch)

enum {
  EX_IAM, // instruction address misaligned
  EX_IAF, // instruction address fault
  EX_II,  // illegal instruction
  EX_BP,  // breakpoint
  EX_LAM, // load address misaligned
  EX_LAF, // load address fault
  EX_SAM, // store/amo address misaligned
  EX_SAF, // store/amo address fault
  EX_ECU, // ecall from U-mode
  EX_ECS, // ecall from S-mode
  EX_RS0, // reserved
  EX_ECM, // ecall from M-mode
  EX_IPF, // instruction page fault
  EX_LPF, // load page fault
  EX_RS1, // reserved
  EX_SPF, // store/amo page fault
};
// memory
enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };
enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE, MEM_TYPE_IFETCH_READ, MEM_TYPE_WRITE_READ };
enum { MODE_U = 0, MODE_S, MODE_H, MODE_M };
enum { MEM_RET_OK, MEM_RET_FAIL, MEM_RET_CROSS_PAGE };
#endif 