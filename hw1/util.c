#include "20131579.h"

// util
int sec2clock(int sec){
	int ret = 0;
	ret = sec / 3600;
	ret *= 100;
	ret += (sec % 3600) / 60;
	return ret;
}
void num2array(int num, char* arr){
	// printf("util(num2array) called\n");
	int i;
	for(i=3; i>=0; i--){
		arr[i] = num % 10; 
		num /= 10;
	}
}
// return led id
int num2led(int num) {
	switch (num) 
	{
	case 0:
		return 0;
		break;
	case 1:
		return 128;
		break;
	case 2:
		return 64;
		break;
	case 3:
		return 32;
		break;
	case 4:
		return 16;
		break;
	case 5:
		return 8;
		break;
	case 6:
		return 4;
		break;
	case 7:
		return 2;
		break;
	case 8:
		return 1;
		break;
	}
}