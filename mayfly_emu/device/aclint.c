#include "device.h"
#include "../common.h"

static uint64_t mtimecmp = 0xffffffffffffffff;
static uint64_t mtime    = 0;
uint64_t aclint_read(int addr_offset,int len){
    uint64_t ret = 0;
    switch(addr_offset){
        case (CLINT_MTIMER_OFFSET + ACLINT_DEFAULT_MTIMECMP_OFFSET): ret = mtimecmp;break;
        case (CLINT_MTIMER_OFFSET + ACLINT_DEFAULT_MTIME_OFFSET):ret = mtime;break;
        case (CLINT_MSWI_OFFSET):ret = mip->msip;break;
        default:ret = 0;break;
    }
    return ret;
}

void aclint_write(int addr_offset,int len,uint64_t wdata){
    switch(addr_offset){
        case (CLINT_MTIMER_OFFSET + ACLINT_DEFAULT_MTIMECMP_OFFSET): mtimecmp = wdata;assert(len == 8);break;
        case (CLINT_MTIMER_OFFSET + ACLINT_DEFAULT_MTIME_OFFSET):mtime = wdata;assert(len == 8);break;
        case (CLINT_MSWI_OFFSET): mip->msip = wdata&0x1;break;
        default:break;
    }    
}
static int time_count = 0;
void aclint_update(){
	if(time_count %25== 0){
		mtime ++;
	}
	time_count++;
    //mtime = mtime + 100;

    if(mtime>=mtimecmp)mip->mtip = 1;
    else mip->mtip = 0;
}