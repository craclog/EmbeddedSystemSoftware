#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define DEBUG
#define FPGA_DEVICE "/dev/dev_driver"
#define IOM_FPGA_MAJOR 242

#define IOCTL_SET_DATA _IOW(IOM_FPGA_MAJOR, 0, char *)

int main(int argc, char **argv){
    char time_interval;
    char count;
    int data_stream;
    int i;
    int tmp;
    int ret;

    /* Argument validity check */
    if(argc != 4){
        printf("Usage : ./app [time interval] [count] [start_option]\n");
        return -1;
    }
    for(i=0; i<strlen(argv[1]); i++){
        if(!isdigit(argv[1][i])){
            printf("Argument error : time interval[1~100]\n");
            return -1;    
        }
    }
    for(i=0; i<strlen(argv[2]); i++){
        if(!isdigit(argv[2][i])){
            printf("Argument error : count[1~100]\n");
            return -1;    
        }
    }
    for(i=0; i<strlen(argv[3]); i++){
        if(argv[3][i] < '0' || argv[3][i] > '9') {
            printf("Argument error : start option[0001~8000]\n");
            return -1;    
        }
    }
    
    time_interval = atoi(argv[1]);
    count = atoi(argv[2]);
    
    /* Argument validity check */
    if(time_interval < 1 || time_interval > 100){
        printf("Range error : time interval[1~100]\n");
        return -1;
    }
    if(count < 1 || count > 100){
        printf("Range error : count[1~100]\n");
        return -1;
    }
    if(strlen(argv[3]) != 4){
        printf("Argument error : start option[0001~8000]\n");
        return -1;
    }
    tmp = 0;
    for(i=0; i<4; i++){
        if(argv[3][i] != '0'){
            tmp++;
            if(tmp != 1){
                printf("Argument error : start option[0001~8000]\n");
                return -1;
            }
        }
    }
    

    #ifdef DEBUG
    printf("Time interval : %d\n", time_interval);
    printf("count : %d\n", count);
    #endif

    
    /* Get data steam using syscall */
    data_stream = syscall(376, time_interval, count, argv[3]);
    printf("data stream : %d\n", data_stream);
    /* FPGA_DEVICE open */
    int dev = open(FPGA_DEVICE, O_WRONLY);
	if (dev < 0){
		printf("%s Open Failured!\n", FPGA_DEVICE);
		return -1;
	}

    // write(dev, &data_stream, 4);
    /* Control device using ioctl */
    
    ret = ioctl(dev, IOCTL_SET_DATA, &data_stream);
    if(ret < 0){
        printf("ioctl set data failed : %d\n", ret);
        exit(-1);
    }
    
    close(dev);
    return 0;
}