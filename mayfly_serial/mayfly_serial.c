#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

char rx_fifo[250];

const char *fifo_path = "/tmp/message_fifo";
static const char *fifo_rpath = "/tmp/message_rfifo";
void *get_info_from_terminal(void *arg){
	mkfifo(fifo_rpath, 0666);
	int fd_rpipe = open(fifo_rpath, O_WRONLY);
	while(1){

		char * p = fgets(rx_fifo, sizeof(rx_fifo), stdin);
		if(p == NULL){printf("get nothing\n");}
		int ret = write(fd_rpipe, rx_fifo,strlen(rx_fifo));
		if(ret == -1){printf("write failed \n");perror("write");}
	}
	unlink(fifo_rpath);
	close(fd_rpipe);
}

void *get_info_from_serial(void *arg){
	int fd_pipe = open(fifo_path, O_RDONLY);
	//char message[2000];
	//int i = 0;
	while(1){
		char message[2000] = {};
		int ret = read(fd_pipe, message, sizeof(message));
		if((ret != -1) && (ret != 0)){
			printf("%s",message);
			fflush(stdout);
		}
		
	}
	close(fd_pipe);
}
int main(){
	pthread_t thread_id;
	int result;
	// 创建子线程
	result = pthread_create(&thread_id, NULL, get_info_from_terminal, NULL);
	int result1;
	pthread_t thread_id1;
	result1 = pthread_create(&thread_id1, NULL, get_info_from_serial, NULL);

    // 等待子线程结束
    result = pthread_join(thread_id, NULL);
	result1 = pthread_join(thread_id1, NULL);
    if (result != 0) {
        perror("无法等待子线程");
        return 1;
    }
    if (result1 != 0) {
        perror("无法等待子线程");
        return 1;
    }
    printf("主线程等待子线程结束\n");

    return 0;
}
