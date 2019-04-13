#include "20131579.h"

struct timeval m5_timer, target_time, m5_tmp_time, m5_update_time;
struct timeval next_buz;
int counting_down, buz_cnt;

/*
 * input: long long
 * return: Time
 * Transform long long data to Time
 */
struct timeval ll2time(long long n){
	struct timeval ret;
    if(n < 0) n = 0;
	ret.tv_sec = n / 1000000LL;
	ret.tv_usec = n % 1000000LL;
	return ret;
}
/*
 * input: Time
 * return: long long
 * Tranform Time to long long
 */
long long time2ll(struct timeval t){
	long long ret = t.tv_sec * 1000000LL;
	return ret + t.tv_usec;
}
/*
 * input: Time
 * return: 4 digit data
 * Transform Time to 4 digis data
 */
int mode5_2data(struct timeval t){
	int res = 0;
	res = (int)t.tv_sec % 100;
	res *= 100;
	res += (int)t.tv_usec / 10000;
	return res;
}
/*
 * This func called periodically.
 * Update timer data And update fnd device
 * If timed out, Make beep with buzzer
 */
void mode5_period(){
	int i;
	long long t1, t2;
	long long c_tmp;
    long long target_tmp;
	gettimeofday(&m5_tmp_time, NULL);
	t1 = time2ll(m5_tmp_time);
	t2 = time2ll(m5_update_time);
	if(t1 >= t2){		
		c_tmp = time2ll(m5_tmp_time);
		c_tmp += 10000;
		m5_update_time = ll2time(c_tmp);
        target_tmp = time2ll(target_time);
        m5_timer = ll2time(target_tmp - c_tmp);
		send_fnd(mode5_2data(m5_timer));		
	}
	if((target_tmp - c_tmp) <= 0 
        && next_buz.tv_sec <= m5_tmp_time.tv_sec){ /* timed out, beep */
        buz_cnt++;
        unsigned char data = buz_cnt % 2;
        next_buz.tv_sec = m5_tmp_time.tv_sec + 1;
		if(buz_cnt % 2 == 0) counting_down = OFF;
		send_buz(data);
	}
}
/*
 * Start timer. Set target Time = Timer + Current Time
 */
void mode5_start(){
	long long tmp;
	gettimeofday(&m5_tmp_time, NULL);
	tmp = (long long)(m5_tmp_time.tv_sec + m5_timer.tv_sec) * 1000000LL;
	tmp += (long long)(m5_tmp_time.tv_usec + m5_timer.tv_usec);
	target_time.tv_sec = tmp / 1000000LL;
	target_time.tv_usec = tmp % 1000000LL;
    next_buz.tv_sec = 0;
}
/*
 * input: switch id
 * mode5 function
 */
void mode5_switch(int sw_id){
	if(sw_id == 1){ /* start or pause timer */
		if(counting_down == ON){
			counting_down = OFF;
		} else if(counting_down == OFF && time2ll(m5_timer)){
			counting_down = ON;
            buz_cnt = 0;
			mode5_start();
		}
	} else if(sw_id == 2 && counting_down == OFF){ /* +1 sec */
		m5_timer.tv_sec += 1;
	} else if(sw_id == 3){ /* reset timer */
		m5_timer.tv_sec = 0;
		m5_timer.tv_usec = 0;
		counting_down = OFF;
        buz_cnt = 0;
        send_buz(0);
	}
	send_fnd(mode5_2data(m5_timer));
}
