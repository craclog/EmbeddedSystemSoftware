#include "mode_func.h"

void mode1_periodic(){
    if(status_mode1_changing){
		// send led msg per 1 sec
		struct timeval tmp_time;
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
		struct timeval tmp_time;
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec % 60 == 0) {
			curtime.tv_sec += 60;
			int data = sec2clock(curtime.tv_sec);
			send_fnd(data);
		}
	}
}