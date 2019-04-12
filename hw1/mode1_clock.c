#include "20131579.h"

/* MODE_CLOCK */
int status_mode1_changing = 0;
struct timeval curtime, newtime, blinktime, nexttime, tmp_time;

void mode1_period(){
	if(status_mode1_changing){
		// send led msg per 1 sec
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= blinktime.tv_sec){
			blinktime.tv_sec = tmp_time.tv_sec + 1;
			if(tmp_time.tv_sec % 2 == 0){
				send_led(3);
			} else{
				send_led(4);
			}
		}
	} else{
		// send time per 1 min
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= nexttime.tv_sec) {
			nexttime.tv_sec = tmp_time.tv_sec + 60;
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