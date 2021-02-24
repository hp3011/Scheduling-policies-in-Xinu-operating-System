#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#define RAND_MAX 32767

double pow(double x, int y){
    double powval = 1;
    int i;
    for (i=0;i<y;i++){
        powval *= x;
    }
    return powval;
}
double log(double x){
    int n;
    double logval = 0;
    for (n=1;n<=20;n++){
        logval = logval -pow(x,n)/n;
    }
    return logval;
}
double expdev(double lambda){
    double dummy;
    do
        dummy= (double) rand() / RAND_MAX;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}