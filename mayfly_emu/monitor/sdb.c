#include "sdb.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "../memory/memory.h"
#include "../cpu/mayfly_cpu.h"
void init_regex();
void init_wp_pool();

static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(mayfly) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  // -1 is the max number?? uint---- zhoutao
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
    return 255;
}

static int cmd_si(char *args){
    if(args == NULL){
       cpu_exec(1);
        return 0;
    }
   
    int num = atoi(args);
    if(num<=0){
        printf("you maybe input a illegal number!!!!\n");
        return 0;
    }
	if(mayfly_state.state != MAYFLY_RUNNING){
		mayfly_state.state = MAYFLY_RUNNING;
	}
    cpu_exec(num);
    return 0;
}

bool isa_difftest_checkregs(riscv64_CPU_state *ref_r,uint64_t next_pc,uint64_t current_pc){
	if(ref_r->pc != next_pc){
		printf("\033[01;40;31m-------------difftest pc false ---------- \033[0;0m\n");
        printf("\033[01;40;33m ref->next_pc 0x%lx, mayfly->next_pc :0x%lx,mayfly this pc :0x%lx \033[0;0m \n",ref_r->pc,next_pc,current_pc);
        return false;
	}
	if(ref_r->mode != cpu.mode){
		printf("\033[01;40;31m -------------difftest mode false ---------- \033[0;0m \n");
		printf("ref mode=%x,mayfly mode=%x\n",ref_r->mode,cpu.mode);
		return false;
	}
	for(int i=0;i<32;i++){
		if(ref_r->gpr[i] != cpu.gpr[i]){
			printf("\033[01;40;31m-------------difftest reg false ---------- \033[0;0m \n");
            printf("ref gpr[%d]=%lx,mayfly gpr[%d]=%lx\n",i,ref_r->gpr[i],i,cpu.gpr[i]);
            return false;
		}
	}
	for(int i=0;i<17;i++){
    if((i == 6) || (i == 12)) //跳过scratch的比较
    {
      continue;
    }
		if(ref_r->csr[i] != cpu.csr[i]){
			if(i == 4)continue;
			printf(" \033[01;40;31m -------------difftest csr false ----------  \033[0;0m \n");
            printf("ref csr[%d]=%lx,mayfly csr[%d]=%lx\n",i,ref_r->csr[i],i,cpu.csr[i]);
			//if(i == 4)continue;
            return false;
		}
	}
	return true;
}

void isa_reg_display(){
    printf("$0($0) = %lx, $1(ra) = %lx, $2(sp) = %lx, $3(gp) = %lx \n",cpu.gpr[0],cpu.gpr[1],cpu.gpr[2],cpu.gpr[3]);
    printf("$4(tp) = %lx, $5(t0) = %lx, $6(t1) = %lx, $7(t2) = %lx \n",cpu.gpr[4],cpu.gpr[5],cpu.gpr[6],cpu.gpr[7]);
    printf("$8(s0) = %lx, $9(s1) = %lx, $10(a0) = %lx, $11(a1) = %lx \n",cpu.gpr[8],cpu.gpr[9],cpu.gpr[10],cpu.gpr[11]);
    printf("$12(a2) = %lx, $13(a3) = %lx, $14(a4) = %lx, $15(a5) = %lx \n",cpu.gpr[12],cpu.gpr[13],cpu.gpr[14],cpu.gpr[15]);
    printf("$16(a6) = %lx, $17(a7) = %lx, $18(s2) = %lx, $19(s3) = %lx \n",cpu.gpr[16],cpu.gpr[17],cpu.gpr[18],cpu.gpr[19]);
    printf("$20(s4) = %lx, $21(s5) = %lx, $22(s6) = %lx, $23(s7) = %lx \n",cpu.gpr[20],cpu.gpr[21],cpu.gpr[22],cpu.gpr[23]);
    printf("$24(s8) = %lx, $25(s9) = %lx, $26(s10) = %lx, $27(s11) = %lx \n",cpu.gpr[24],cpu.gpr[25],cpu.gpr[26],cpu.gpr[27]);
    printf("$28(t3) = %lx, $29(t4) = %lx, $30(t5) = %lx, $31(t6) = %lx \n",cpu.gpr[28],cpu.gpr[29],cpu.gpr[30],cpu.gpr[31]);
	printf("current pc = %lx,next pc = %lx, mode = %x\n",cpu.pre_pc,cpu.pc,cpu.mode);
}

void isa_csr_reg_display(){
	printf("mstatus = %lx, mtvec = %lx, medeleg = %lx, mideleg = %lx\n",cpu.csr[0],cpu.csr[1],cpu.csr[2],cpu.csr[3]);
	printf("mip = %lx, mie = %lx, mscratch = %lx, mepc = %lx \n",cpu.csr[4],cpu.csr[5],cpu.csr[6],cpu.csr[7]);
	printf("mcause = %lx, mtval = %lx, stvec = %lx,scounteren = %lx \n",cpu.csr[8],cpu.csr[9],cpu.csr[10],cpu.csr[11]);
	printf("sscratch = %lx, sepc = %lx, scause = %lx, stval = %lx\n",cpu.csr[12],cpu.csr[13],cpu.csr[14],cpu.csr[15]);
	printf("satp = %lx, mcycle = %lx,minstret = %lx\n",cpu.csr[16],cpu.csr[24],cpu.csr[23]);
}

extern void isa_ref_csr_reg_display();
static int cmd_info(char *args){
    if(args == NULL ){
        printf("You must follow the format strictly!!!! not have more space and .etc !!!!\n");
        return 0;
    }
    if(strncmp("r",args,1)==0){
        isa_reg_display(); 
        return 0;
    }
    if(strncmp("w",args,1)==0){
        display_watchpoint();
        return 0;
    }
    if(strncmp("csr",args,3)==0){
        isa_csr_reg_display();
        return 0;
    }
    if(strncmp("fr",args,7)==0){
        isa_ref_csr_reg_display();
        return 0;
    }
	if(strncmp("inst",args,4)==0){
		printf("ll\n");
        show_iringbuf_info();
        return 0;
	}
    printf("You must follow the format strictly!!!! not have more space and .etc !!!!\n");
    return 0;
}

static int cmd_x(char *args){
    if(args == NULL ){
        printf("You must follow the format strictly!!!!\n");
        return 0;
    }
    char *str_num = strtok(args, " ");
    int num = atoi(str_num);
    if(num <= 0){
        printf("You must follow the format strictly !!! error input!!! \n");
        return 0;
	}
    char *str_addr = strtok(NULL," ");
    uint64_t paddr = strtoul(str_addr,NULL,16); 
    if(paddr == 0){
        if(strncmp(str_addr,"0x00000000",10)!=0){
            printf("You must follow the format strictly !!! error input!!! \n");
            return 0;
         }
    }
    // It maybe read the virtual memory not the physical memory ????
    printf("The physical memory scan ~~~~~\n");
    for(int i =0; i < num; i++){

        uint64_t value =  paddr_read(paddr + i*4, 4);
       printf("paddr: 0x%lx, value: 0x%lx \n",paddr + i*4, value);
    }
    return 0;
}

static int cmd_p(char *args){
    bool success = true;
    uint64_t value =  expr(args, &success);
    if(success == true){
        printf("expr: %s , value: 0x%lx \n",args,value);
    }else {
        printf("0x%lx \n",value);
        printf("error expr !!!! \n");
    }

    return 0;
}

static int cmd_w(char *args){
    create_watchpoint(args);    
    return 0;
}

static int cmd_d(char *args){
    int num = atoi(args);
    // exits risks when num = 0; really!
    if(num >=32){
        printf("error input \n");
        return 0;
    }
    delete_watchpoint(num);
    return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "si [N],Step through n instructions and stop. defalt N = 1", cmd_si},
  { "info", "info SUBCMD,printf the registers' status info r, printf the watchpoint  info w, printf csr info csr", cmd_info},
  { "x", "x N EXPR,scan memory. Evaluate the expression EXPR, using the result as the starting memory address, output \
      N consecutive 4-bytes in hexadecimal", cmd_x },
  { "p", "p EXPR, Evaluate the expression EXPR", cmd_p },
  { "w", "w EXPR, set watchpoint , It means stopping the program when the expression EXPR has changed!",cmd_w },
  { "d", "d N, delete the watchpoint whoes number is N !", cmd_d },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_mainloop() {
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
          int temp = cmd_table[i].handler(args);
          if(temp<0){return;}
          else if(temp == 255){printf("mayfly quit!!!\n");exit(0);}
       // if (cmd_table[i].handler(args) < 0) { return; }
       // if (cmd_table[i].handler(args)== 255){printf("nemu quit!!!\n"); exit(0);};
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}