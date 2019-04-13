#include "20131579.h"

/* MODE_CLOCK */
int status_mode1_changing = 0;
struct timeval curtime, newtime, blinktime, nexttime, tmp_time;

/*
 * input : sec
 * return : 4 digits for (min:sec)
 * Tranform seconds to 4 digits time
 */
int sec2clock(int sec){
	int ret = 0;
	ret = (sec / 3600) % 24;
	ret *= 100;
	ret += (sec % 3600) / 60;
	return ret;
}
/*
 * if changing time mode, this func called periodically.
 */
void mode1_period(){
	if(status_mode1_changing){ 
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= blinktime.tv_sec){ /* send led message per 1 sec */
			blinktime.tv_sec = tmp_time.tv_sec + 1; /* set next blink time */
			if(tmp_time.tv_sec % 2 == 0){ /* blink */
				send_led(3);
			} else{
				send_led(4);
			}
		}
	} else{ 
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= nexttime.tv_sec) { /* send time per 1 min */
			nexttime.tv_sec = tmp_time.tv_sec + 60; /* set next Changing min time */
			curtime.tv_sec += 60;
			int data = sec2clock(curtime.tv_sec);
			send_fnd(data);
		}
	}
}
void mode1_change_time() {
	if (!status_mode1_changing) {
		status_mode1_changing = 1;
		newtime = curtime;
		blinktime.tv_sec = curtime.tv_sec + 1;
		nexttime.tv_sec = curtime.tv_sec + 60;
	}
	else {
		status_mode1_changing = 0;
		gettimeofday(&curtime, NULL);
		nexttime.tv_sec = curtime.tv_sec + 60;
		curtime = newtime;
		int data = sec2clock(curtime.tv_sec);
		send_fnd(data);
		send_led(1);
	}
}
void mode1_reset_time() {
	status_mode1_changing = 0;
	gettimeofday(&curtime, NULL);
	nexttime.tv_sec = curtime.tv_sec + 60;
	int data = sec2clock(curtime.tv_sec);
	send_fnd(data);
	send_led(1);
}
void mode1_add_hour() {
	newtime.tv_sec += 3600;
	int data = sec2clock(newtime.tv_sec);
	send_fnd(data);
}
void mode1_add_min() {
	newtime.tv_sec += 60;
	int data = sec2clock(newtime.tv_sec);
	send_fnd(data);
}