#include "device.h"
#include "../common.h"
uint32_t exteranl_irq = 0;  //目前计划只对一种情况进行模拟，就是串口的读取时，是否需要中断的问题
static uint32_t source_priority[31];
static uint32_t source_pending;
static uint32_t target_enables;
static uint32_t target_threshold;
static uint32_t target_claim;

uint64_t plic_read(int addr_offset,int len){
    uint64_t ret = 0;

    if(addr_offset <= 0x7c){
        ret = source_priority[addr_offset >>2];
    }
    switch(addr_offset){
        case 0x1000: ret = source_pending;break;
        case 0x2080: ret = target_enables;break;
        case 0x201000: ret = target_threshold;break;
        case 0x201004: ret = target_claim;break;
        default: break;
    }
    return ret;
}

void plic_write(int addr_offset,int len,uint32_t wdata){
    if(addr_offset <= 0x7c){
        assert(len == 4);
        source_priority[addr_offset >>2] = wdata;
		printf("plic source_priority[%d]:0x%x\n",addr_offset >>2,wdata);
    }
    switch(addr_offset){
        case 0x1000: assert(0);break;
        case 0x2080: target_enables = wdata;printf("plic enable:0x%x\n",target_enables);break;
        case 0x201000: target_threshold = wdata;printf("plic threshold:0x%x\n",target_enables);break;
        case 0x201004: source_pending = source_pending &(~(1<<wdata));break;
        default: printf("plic isproblem 0x%x,0x%x\n",addr_offset,wdata);break;
    }
}

void plic_update(){
    source_pending = source_pending | exteranl_irq;
    uint32_t max_id = 0;
    uint32_t max_priority = 0;
    for(int i =0 ; i<=31;i++){
        if(((source_pending & target_enables)>>i)&0x1){
            if(source_priority[i]>max_priority){max_id = i;max_priority= source_priority[i];}
        }
    }
    target_claim = max_id;
    if(max_priority > target_threshold){
        //mip->meip=1; //需要判断当前CPU模式
		mip->seip=1;
    }else{
        //mip->meip=0; //需要判断当前CPU模式
		mip->seip=0;
	}
}