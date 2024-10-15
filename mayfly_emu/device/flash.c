#include "device.h"
#include <assert.h>
#define FLASH_SIZE (64 * 1024 * 1024)
static uint8_t flash_mem[FLASH_SIZE];
void flash_init(char* img_file){
    if(img_file == NULL){
        printf("img_file is NULL \n");
        assert(0);
    }
	printf("img_file path : %s\n",img_file);
    FILE *fp = fopen(img_file, "rb");
    if(fp == NULL){
        printf("can not open %s !!!\n",img_file);
        assert(0);
    }
    fseek(fp,0,SEEK_END);
    int size = ftell(fp);
    if(size > FLASH_SIZE){
        printf("!!!!!!!!!!! img size over the ram !!!!!!!!!!!!!!!!!\n");
    }
    fseek(fp,0,SEEK_SET);

    int ret = fread(flash_mem,size,1,fp);    
    if(ret != 1){
        printf("error fread ret %d\n",ret);
        assert(0);
    }
    fclose(fp);
}
uint64_t flash_read(int addr_offset,int len){
	//printf("falsh 0x%x,data 0x%x\n",addr_offset,*(uint32_t *)(flash_mem + addr_offset));
	switch(len){
    case 1: return *(uint8_t  *)(flash_mem + addr_offset);
    case 2: return *(uint16_t *)(flash_mem + addr_offset);
    case 4: return *(uint32_t *)(flash_mem + addr_offset);
	default: return 0;
	}
}
void flash_write(int addr_offset,int len,uint32_t wdata){
	switch(len){
		case 1: *(uint8_t *)(flash_mem + addr_offset) = wdata;return;
		case 2: *(uint16_t *)(flash_mem + addr_offset) = wdata;return;
		case 4: *(uint32_t *)(flash_mem + addr_offset) = wdata;return;
		default:;
	}
}