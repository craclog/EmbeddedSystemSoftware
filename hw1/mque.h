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
#include <string.h>

/* message queue*/
#define INPUTQ_KEY 	92
#define OUTPUTQ_KEY 99

/* input_buf */
#define FUNCTION_KEY 	0	//readkey
#define SWITCH_KEY 		1

/* input_event */
#define KEY_RELEASE 0
#define KEY_PRESS 	1

/* output_buf */
#define FIX_FND 	1
#define FIX_LED 	2
#define FIX_TEXT 	4
#define FIX_DOT 	8

typedef struct {
	long mtype;
	int type;
	int press;
	struct input_event ev;
	int sw_num;
	unsigned char sw_id1, sw_id2;
}input_buf;

typedef struct {
	long mtype;
	int fix_bit;
	unsigned char fnd[4];	//minute
	unsigned char led;	// Binary num
	unsigned char* text; 
	int text_size;
	int dot[10];
}output_buf;

#endif
