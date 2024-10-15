#ifndef __DEVICE_H
#define __DEVICE_H 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAYFLY_UART_ADDR			0x10000000
#define MAYFLY_UART_FREQ			50000000   //这个需要稍后进行判断一下，可能需要修改
#define MAYFLY_UART_BAUDRATE		115200
#define MAYFLY_UART_REG_SHIFT		2
#define MAYFLY_UART_REG_WIDTH		4
#define MAYFLY_UART_REG_OFFSET		0

#define MAYFLY_PLIC_ADDR			0xc000000
#define MAYFLY_PLIC_NUM_SOURCES		3
#define MAYFLY_HART_COUNT			1
#define MAYFLY_CLINT_ADDR			0x2000000
#define MAYFLY_ACLINT_MTIMER_FREQ	1000000		//这个需要稍后进行判断一下，可能需要修改
#define CLINT_MSWI_OFFSET		0x0000
#define CLINT_MTIMER_OFFSET		0x4000
#define MAYFLY_ACLINT_MSWI_ADDR			(MAYFLY_CLINT_ADDR + CLINT_MSWI_OFFSET)
#define MAYFLY_ACLINT_MTIMER_ADDR		(MAYFLY_CLINT_ADDR + CLINT_MTIMER_OFFSET)
#define ACLINT_DEFAULT_MTIME_OFFSET	0x7ff8
#define ACLINT_DEFAULT_MTIMECMP_OFFSET	0x0000
#define MAYFLY_ACLINT_DEFAULT_MTIME_ADDR        (MAYFLY_ACLINT_MTIMER_ADDR + ACLINT_DEFAULT_MTIME_OFFSET)
#define MAYFLY_ACLINT_DEFAULT_MTIMECMP_ADDR     (MAYFLY_ACLINT_MTIMER_ADDR + ACLINT_DEFAULT_MTIMECMP_OFFSET)

#define MAYFLY_FLASH_ADDR 			0x30000000
#define MAYFLY_GPIO_ADDR 			0x40000000

uint64_t serail_read(int addr_offset,int len);
void serail_write(int addr_offset,int len,uint64_t wdata);
uint64_t plic_read(int addr_offset,int len);
void plic_write(int addr_offset,int len,uint32_t wdata);
uint64_t aclint_read(int addr_offset,int len);
void aclint_write(int addr_offset,int len,uint64_t wdata);

void flash_write(int addr_offset,int len,uint32_t wdata);
uint64_t flash_read(int addr_offset,int len);
void flash_init(char* img_file);

void init_device();
void serial_init();
void device_exit();
void serial_exit();

void update_device();
void aclint_update();
void plic_update();
void serail_update();

extern uint32_t exteranl_irq;
#endif