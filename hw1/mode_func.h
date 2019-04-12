#ifndef __MODE_FUNC__
#define __MODE_FUNC__

#include <sys/time.h>
#include <stdio.h>

struct timeval curtime, newtime, blinktime;
int status_mode1_changing = 0;

void mode1_periodic();


#endif