#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void){
	int fd;
	int retn;
	char buf[2] = {0,};
	
	/* Open stopwatch device */
	fd = open("/dev/stopwatch", O_RDWR);
	if(fd < 0) {
		perror("/dev/stopwatch error");
		exit(-1);
	}
	else { 
		printf("< stopwatch Device has been detected > \n"); 
	}
	/* ************************************************
	 * Wrtie data to stopwatch 
	 * Process sleeps until press VOL- button 3 seconds
	 * ************************************************/
	retn = write(fd, buf, 2);
	close(fd);
	printf("user application exit\n");
	return 0;
}
