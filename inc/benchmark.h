#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
void timeval_print(struct timeval *tv);
