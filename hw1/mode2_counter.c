#include "20131579.h"

/* MODE_COUNTER */
int cnt2;
int base;

int base_tran(int n, int from, int to){
	int tmp = 0, res = 0;
	int mul = 1;
	int t_n = n;
	while(t_n){
		tmp += (t_n%10) * mul;
		t_n /= 10;
		mul *= from;
	}
	mul = 1;
	while(tmp){
		if(mul > 1000) break;
		res += (tmp%to) * mul;
		tmp = tmp / to;
		mul *= 10;
	}
	return res;
}
void mode2_change_base(){
	int new_base;
	if(base == 10) {new_base = 8; send_led(BASE_8);}
	else if(base == 8) {new_base = 4; send_led(BASE_4);}
	else if(base == 4) {new_base = 2; send_led(BASE_2);}
	else if(base == 2) {new_base = 10; send_led(BASE_10);}
	cnt2 = base_tran(cnt2, base, new_base) % 1000;
	base = new_base;
	send_fnd(cnt2);
}
void mode2_add(int n){
	cnt2 = base_tran(cnt2, base, 10) + base_tran(n, base, 10);
	cnt2 = base_tran(cnt2, 10, base) % 1000;
	send_fnd(cnt2);
}