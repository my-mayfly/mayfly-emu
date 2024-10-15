#include "common.h"
#include "./cpu/mayfly_isa.h"
#include "./memory/memory.h"
#include "./monitor/sdb.h"
#include "./device/device.h"
#include "./utils/disasm.h"
#include "./cpu/difftest.h"
#include <getopt.h>
// #include <pthread.h>
// #include <termio.h>
#include <unistd.h>
#include <fcntl.h>
uint64_t cycle = 0; 
riscv64_CPU_state cpu = { };
MAYFLYState mayfly_state = {};

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x0002b823,  // sd  zero,16(t0)
  0x0102b503,  // ld  a0,16(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};

static void restart(){
  /* Set the initial program counter. */
  cpu.mode = 3;//"b11" M Mode, "b01" S Mode, "b00" U Mode
  cpu.pc = RESET_VECTOR;//RESET_VECTOR;//RESET_VECTOR;
  cpu.amo = 0;
  cpu.is_irq = 0;
  cpu.is_except = 0;
  cpu.csr[17]= 0x8000000000141101;
  minstret->val = 0;
  mcycle->val = 0;
  mstatus->spp = 1;
  mstatus->mpp = 3;
  mstatus->uxl = 2;
  mstatus->sxl = 2;
  /* The zero register is always 0. */
  cpu.gpr[0] = 0;
}

int read_noblock_stdin_fd;
void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(0x80000000), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}

void welcome(){
    printf("^_^ ***welcome to start debug in mayfly ***  O(∩_∩)O~\n");
}

static char *img_file = NULL;
static char *kernel_file = NULL;
static char *file_system = NULL;

#ifdef CONFIG_DIFFTEST
static char *ref_so_file = "./tools/spike-diff/build/riscv64-spike-so";
#endif

static int parse_args(int argc, char *argv[]){
  const struct option table[] = {
    {"img"      , required_argument, NULL, 'i'},
    {"kernel"   , required_argument, NULL, 'k'},
    {"file-system"  ,required_argument, NULL, 'f'},
    {NULL,      0,   NULL,0},
  };
  int o;
  for(int i = 0; i < argc; i++){
    printf("i= %d, %s\n",i,argv[i]);
  }
  while((o = getopt_long(argc, argv, "-i:k::f::", table, NULL)) != -1){
    switch(o){
      case 'i': img_file    = optarg; printf("%s\n",optarg);break;
      case 'k': kernel_file = optarg; printf("%s\n",optarg);break;
      case 'f': file_system = optarg; printf("%s\n",optarg);break;
      default:
        printf("error args \n");break;
    }
  }
  optind = 0;
  return 0;
}

int main(int argc, char *argv[]){
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  init_device();
  init_isa();
  //assert(img_file != NULL);
if(img_file != NULL){
#if CONFIG_FLASH == 1
  flash_init(img_file);
#else 
  init_mem(img_file);
#endif
}

  if(kernel_file != NULL){
    load_kernel(kernel_file);
  }

  if(file_system  != NULL){
    load_fs(file_system);
  }

#ifdef CONFIG_DIFFTEST
  assert(ref_so_file!=NULL);
  init_difftest(ref_so_file,0x8000000);
#endif 

  init_sdb();
#ifdef CONFIG_ITRACE
	init_disasm("riscv64""-pc-linux-gnu");
#endif 
  welcome();

#if CONFIG_SHARE == 0
  read_noblock_stdin_fd = open("/dev/tty", O_RDONLY|O_NONBLOCK);
#endif 
  sdb_mainloop();
  close(read_noblock_stdin_fd);
}