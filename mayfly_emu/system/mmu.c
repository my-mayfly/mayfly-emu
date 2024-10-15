#include "system.h"
#include "../memory/memory.h"
typedef union PageTableEntry {
  struct {
    uint32_t v   : 1;
    uint32_t r   : 1;
    uint32_t w   : 1;
    uint32_t x   : 1;
    uint32_t u   : 1;
    uint32_t g   : 1;
    uint32_t a   : 1;
    uint32_t d   : 1;
    uint32_t rsw : 2;
    uint64_t ppn :44;
    uint32_t pad :10;
  };
  uint64_t val;
} PTE;

#define PgSHIFT 12 
#define PgMASK ((1ull << PgSHIFT) - 1)
#define PgBASE(pn) (pn << PgSHIFT)

// Sv39 page walk
#define PTW_LEVEL 3
#define PTE_SIZE 8
#define VPNMASK 0x1ff

static inline uint64_t VPN_i(uint64_t va, int i){
	return (va >> (12 + 9 *i)) &0x1ff;
}

static inline bool check_permission(PTE *pte, bool ok, uint64_t vaddr, int type){
  bool ifetch = (type == MEM_TYPE_IFETCH);
  uint32_t mode = (mstatus->mprv && !ifetch ? mstatus->mpp : cpu.mode);
  assert(mode == MODE_U || mode == MODE_S);
  ok = ok && pte->v;
  ok = ok && !(mode == MODE_U && !pte->u);
  ok = ok && !(pte->u && ((mode == MODE_S) && ((!mstatus->sum) || ifetch)));
  if(ifetch){
    bool update_ad = (!pte->a);
    // printf("mmu access dirty bit is bad\n");
    //bool update_ad = false;
    if(!(ok && pte->x) || update_ad){
      assert(!cpu.amo);
	    cpu.m_s_cause = EX_IPF;
	    cpu.m_s_tval  = vaddr;
      cpu.is_except = true;
      return false;      
    }
  }else if(type == MEM_TYPE_READ){
    bool can_load = pte->r || (mstatus->mxr && pte->x);
    bool update_ad = (!pte->a);
    if(!(ok && can_load) || update_ad){
      if(cpu.amo)printf("redirect to AMO page fault exception at pc = 0x%lx\n",cpu.pc);
      int ex = (cpu.amo ? EX_SPF : EX_LPF);
	    cpu.m_s_cause = ex;
	    cpu.m_s_tval  = vaddr;
      cpu.amo = false;
      cpu.is_except = true;
      return false;      
    }
  }else{
    bool update_ad = !pte->a || !pte->d;
    if (!(ok && pte->w) || update_ad) {
	    cpu.m_s_cause = EX_SPF;
	    cpu.m_s_tval  = vaddr;
      cpu.amo = false;
      cpu.is_except = true;
      return false;
    }
  }
  return true;
}

int isa_mmu_check(uint64_t vaddr, int len, int type){
    //printf("MMU checking addr %lx\n", vaddr);
    bool is_ifetch = type == MEM_TYPE_IFETCH; 
    bool vm_enable = (((mstatus->mprv && (!is_ifetch)) ? mstatus->mpp : cpu.mode) < MODE_M) && (satp->mode == 8);
    uint64_t va_mask = ((((uint64_t)1) << (63 - 38 + 1)) - 1);
    uint64_t va_msbs = vaddr >> 38;
    bool va_msbs_ok = (va_msbs == va_mask) || va_msbs == 0 || !vm_enable;    
    if(!va_msbs_ok){
      if(is_ifetch){
			  cpu.m_s_cause = EX_IPF;
			  cpu.m_s_tval  = vaddr;
        cpu.is_except = true;
      }else if(type == MEM_TYPE_READ){
        int ex = (cpu.amo != 0) ? EX_SPF : EX_LPF;
			  cpu.m_s_cause = ex;
			  cpu.m_s_tval  = vaddr;
        cpu.is_except = true;
      }else{

			  cpu.m_s_cause = EX_SPF;
			  cpu.m_s_tval  = vaddr;
        cpu.is_except = true;
      }
      return MMU_FAIL;
    }

    return vm_enable ? MMU_TRANSLATE : MMU_DIRECT;
}

uint64_t isa_mmu_translate(uint64_t vaddr, int len, int type){
  //printf("MMU translate addr %lx \n", vaddr);
  if(cpu.pc == 0xffffffff8000a23c){
	  printf("0xffffffff8000a23c MMU translate addr %lx \n", vaddr);
  }
  uint64_t pg_base = PgBASE(satp->ppn);
  uint64_t p_pte;
  PTE pte;
  int level;
  // update a/d by hardware
  int64_t vaddr39 = vaddr << (64 - 39);
  vaddr39 >>= (64 - 39);
  if ((uint64_t)vaddr39 != vaddr) goto bad;
  for (level = PTW_LEVEL - 1; level >=0;){
    p_pte = pg_base + VPN_i(vaddr, level) * PTE_SIZE; //地址
	
    pte.val = paddr_read(p_pte,PTE_SIZE); //对应地址上的值
    if(cpu.pc == 0xffffffff8000a23c){printf("paddr %lx,data %lx\n",p_pte,pte.val);}
    pg_base = PgBASE(pte.ppn);
    if (!pte.v || (!pte.r && pte.w)) {
		  goto bad;
	  }
    if (pte.r || pte.x){break;}
    else {
      level --;
      if(level < 0){printf("level 0x%x\n",level);goto bad;}
    }
  }
  //printf("pte 0x%lx \n",pte.val);
  if (!check_permission(&pte, true, vaddr, type)) return MEM_RET_FAIL; //检测该结果的权限

  if(level > 0){
    // superpage
    uint64_t pg_mask = ((1ull << ( PgSHIFT+ 9*level)) - 1); //如果是大页查看是否对齐
    if ((pg_base & pg_mask) != 0) {
      // missaligned superpage
	    printf("missaligned superpage pg_base 0x%lx, pg_mask %lx\n",pg_base,pg_mask);
      goto bad;
    }
    pg_base = (pg_base & ~pg_mask) | (vaddr & pg_mask & ~PgMASK);
  }

  return pg_base | MEM_RET_OK;
bad:
  //printf("Memory translation bad\n");
  check_permission(&pte, false, vaddr, type);
  return MEM_RET_FAIL;
}