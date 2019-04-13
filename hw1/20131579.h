#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>

/* message queue key */
#define INPUTQ_KEY 	925
#define OUTPUTQ_KEY 991
/* input_buf */
#define BUFF_SIZE 64
#define FUNCTION_KEY 	0	/* == readkey */
#define SWITCH_KEY 		1
/* input_event */
#define KEY_RELEASE 0
#define KEY_PRESS 	1
/* output_buf */
#define FIX_DIE		0
#define FIX_FND 	1
#define FIX_LED 	2
#define FIX_LCD 	4
#define FIX_DOT 	8
#define FIX_BUZ		16
/* processor.c */
#define DEFAULT_MODE 1
#define MAX_MODE_NUM 5
/* function key */
#define BACK 158
#define VOL_UP 115
#define VOL_DOWN 114
/* mode */
#define MODE_CLOCK 1
#define MODE_COUNTER 2
#define MODE_TEXTEDITOR 3
#define MODE_DRAWBOARD 4
#define MODE_TIMER 5

/* reader.c */
#define FUNCTION_DEVICE "/dev/input/event0"
#define SWITCH_DEVICE "/dev/fpga_push_switch"
/* writer */
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"
#define DOT_DEVICE "/dev/fpga_dot"
#define LCD_DEVICE "/dev/fpga_text_lcd"
#define BUZ_DEVICE "/dev/fpga_buzzer"
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16				

/* MODE_COUNTER */
#define BASE_2 1
#define BASE_10 2
#define BASE_8 3
#define BASE_4 4
/* MODE_TEXTEDITOR */
#define ALPHA 1
#define NUMBER 2
#define DOT_A 10
#define DOT_CLEAR 11
#define MAX_STR_BUFF 32
#define LINE_BUFF 16
#define NO_OVERWRITE 0
#define OVERWRITE 1
/* MODE_DRAWBOARD */
#define HIDE_ON_BUSH 0
#define NO_HIDE 1
#define DOT_HEIGHT 10
#define DOT_WIDTH 7
#define DOT_NOSEL 0
#define DOT_SEL 1
#define DOT_XOR 2
/* MODE_TIMER */
#define OFF 0
#define ON 1

typedef struct { /* msg ds for reader-processor */
	long mtype;
	int type; 				/* function key or switch */
	struct input_event ev; 	/* function key ds */
	int sw_num; 			/* # of switch input */
	unsigned char sw_id1, sw_id2;
}input_buf;

typedef struct { /* msg ds for processor-writer */
	long mtype;
	int fix_bit;			/* which dev to work */
	unsigned char fnd[4];	/* 4 digits */
	unsigned char led;		/* Binary num */
	unsigned char text[MAX_STR_BUFF + 1]; 
	unsigned char hex_dot[10];
	unsigned char buz;
}output_buf;

/* processor function */
void num2array(int num, char* arr);
int num2led(int num);
void send_fnd(int data);
void send_led(int data);
void send_lcd(unsigned char str[MAX_STR_BUFF]);
void send_dot(unsigned char data[10]);
void send_buz(unsigned char data);
void send_kill();
void clear_mode(int mode);
void init_mode(int mode);
void init_devices();

/* writer function */
void write_fnd(unsigned char msg[4]);
void write_led(unsigned char n);
void write_dot(unsigned char data[10]);
void write_lcd(unsigned char str[MAX_STR_BUFF]);
void write_buz(unsigned char data);

/* mode 1(clock) function */
int sec2clock(int sec);
void mode1_period();
void mode1_change_time();
void mode1_reset_time();
void mode1_add_hour();
void mode1_add_min();

/* mode 2(counter) function */
int base_tran(int n, int from, int to);
void mode2_change_base();
void mode2_add(int n);

/* mode 3(text editor) function */
void mode3_change_input_mode();
void clear_text(unsigned char str[MAX_STR_BUFF + 1]);
void mode3_insert_char(char c, int overwrite);
void mode3_switch1(int sw_id);

/* mode 4(draw board) function */
int boundary_chk(int r, int c);
void select_hex_dot(unsigned char data[10], int r, int c, int cmd);
void mode4_blink();
void clear_dot();
void inverse_dot();
void mode4_hide_cursor();
void mode4_switch(int sw_id);

/* mode 5(timer) function */
struct timeval ll2time(long long n);
long long time2ll(struct timeval t);
int mode5_2data(struct timeval t);
void mode5_period();
void mode5_start();
void mode5_switch(int sw_id);


