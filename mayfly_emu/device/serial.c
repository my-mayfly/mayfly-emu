#include "device.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define RX_FIFO_OFFSET 0x0
#define TX_FIFO_OFFSET 0x4 
#define STAT_REG_OFFSET 0x8
#define CTRL_REG_OFFSET 0xc


typedef union { 
    struct {
        uint32_t rdata : 8;
    }; 
    uint32_t val;
}rx_fifo_t;

typedef union { 
    struct {
        uint32_t tdata0 : 8;
		uint32_t tdata1 : 8;
		uint32_t tdata2 : 8;
		uint32_t tdata3 : 8;
    }; 
    uint32_t val;
}tx_fifo_t;

typedef union { 
    struct {
        uint32_t rst_tx_fifo : 1;
        uint32_t rst_rx_fifo : 1;
        uint32_t pad0 : 2;
        uint32_t en_intr : 1;
    }; 
    uint32_t val;
}ctrl_reg_t;

typedef union{
    struct {
        uint32_t rx_fifo_valid : 1;
        uint32_t rx_fifo_full : 1;
        uint32_t tx_fifo_empty : 1;
        uint32_t tx_fifo_full : 1;
        uint32_t Intr_enable : 1;
        uint32_t overrun_error : 1;
        uint32_t frame_error : 1;
        uint32_t parity_error : 1;
    };
    uint32_t val;
}stat_reg_t;

static rx_fifo_t rx_fifo;
static tx_fifo_t tx_fifo;
// static ctrl_reg_t ctrl_reg;
static stat_reg_t stat_reg;

uint64_t serail_read(int addr_offset,int len){
    uint64_t ret=0;
    switch(addr_offset){
        case RX_FIFO_OFFSET: ret = rx_fifo.val; stat_reg.rx_fifo_full = 0; stat_reg.rx_fifo_valid = 0;break;
        case TX_FIFO_OFFSET: ret = 0;break; 
        case STAT_REG_OFFSET: ret = stat_reg.val;break;
        case CTRL_REG_OFFSET: ret = 0;break;
    }
    return ret;
}
// static uint8_t temp_tx_fifo[16];
// static int temp_tx_point=0;
void serail_write(int addr_offset,int len,uint64_t wdata){
    switch(addr_offset){
        case RX_FIFO_OFFSET: break;
        case TX_FIFO_OFFSET: tx_fifo.val = wdata;printf("%c",tx_fifo.tdata0);stat_reg.tx_fifo_empty = 0;stat_reg.tx_fifo_full = 0;break;
        case STAT_REG_OFFSET: break;
        case CTRL_REG_OFFSET: if(wdata & 0x01){tx_fifo.val = 0;}if(wdata & 0x02)rx_fifo.val = 0;stat_reg.Intr_enable = ((wdata & 0x10) != 0);printf("set ctrl %lx\n",wdata);break;//ctrl_reg.val = wdata;printf("set ctrl %lx\n",wdata);break;
    }
}
static const char *fifo_rpath = "/tmp/message_rfifo";
static const char *fifo_path = "/tmp/message_fifo";
char temp[1];
char r_temp[1];
static int fd_pipe;
static int fd_rpipe;

int exteranl_irq2 = 0;
void serail_update(){
	if(!stat_reg.tx_fifo_empty){

		temp[0] = tx_fifo.tdata0 &0xff;
		int ret = write(fd_pipe, temp,1);
		if(ret == -1){printf("write failed \n");perror("write");}
		stat_reg.tx_fifo_empty = 1;
		stat_reg.tx_fifo_full = 0;
		exteranl_irq2 = 0x2;
	}else{
		exteranl_irq2 = 0x0;
	}
	if((!stat_reg.rx_fifo_valid)){
		int ret = read(fd_rpipe, r_temp, 1);
		if((ret != -1) && (ret != 0) && (r_temp[0] !='\0')){
			// putchar(r_temp[0]);
			// putchar('v');
			// putchar('\n');
			stat_reg.rx_fifo_full = 0;
			stat_reg.rx_fifo_valid = 1;
			rx_fifo.val = r_temp[0];
		}
	}

	if(stat_reg.Intr_enable & stat_reg.rx_fifo_valid){exteranl_irq = 0x2;}else{exteranl_irq = 0x0;}
	exteranl_irq = exteranl_irq | exteranl_irq2;
}

void serial_init(){


	mkfifo(fifo_path, 0666);

	stat_reg.tx_fifo_empty = 1;
	stat_reg.tx_fifo_full = 0;
	stat_reg.rx_fifo_full = 0;
	stat_reg.rx_fifo_valid = 0;
	fd_pipe = open(fifo_path, O_WRONLY);
	fd_rpipe = open(fifo_rpath, O_RDONLY | O_NONBLOCK);
}

void serial_exit(){

	unlink(fifo_path);
	close(fd_pipe);
}