#include "20131579.h"
/*
 * Initializing program
 */
int main(){

    pid_t pid_reader, pid_writer;

    //fork reader process
    if((pid_reader = fork()) == -1){
        perror("reader fork error:");
		exit(1);
    }
    //fork writer process
    if(pid_reader > 0){
        if((pid_writer = fork()) == -1){
            perror("writer fork error:");
		    exit(1);
        }
    }
    if(pid_reader == 0) input_main();
    else if(pid_reader > 0 && pid_writer > 0) proc_main();
    else if(pid_reader > 0 && pid_writer == 0) output_main();
    return 0;
}