#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(){
	pid_t reader_pid, writer_pid;

	if((reader_pid = fork()) == -1){
		perror("reader fork error:");
		exit(1);
	}
	if(reader_pid == 0){
		char *argv[] = {"./reader"};
		execv(argv[0], argv);
	}
	if((writer_pid = fork()) == -1){
		perror("writer fork error:");
		exit(1);
	}
	if(writer_pid == 0){
		char *argv[] = {"./writer"};
		execv(argv[0], argv);
	}
	printf("Am i main? : %d\n", getpid());

	printf("Am i main???? : %d\n", getpid());



	return 0;
}

