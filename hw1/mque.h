#ifndef __MQUE__
#define __MQUE__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <linux/input.h>

/* message queue*/
#define INPUTQ_KEY 9258
#define OUTPUTQ_KEY 9914

/* input_buf */
#define FUNCTION_KEY 0
#define SWITCH_KEY 1

/* inut_event */
#define KEY_RELEASE 0
#define KEY_PRESS 1

typedef struct {
	long mtype;
	int type;
	int press;
	struct input_event ev;
	int sw_num;
	unsigned char sw_id1, sw_id2;
}input_buf;


#endif
