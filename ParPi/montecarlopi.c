//
//  main.c
//  MonteCarloPI
//
//  Created by Alcides Fonseca on 9/9/11.
//  Modified by Bruno Cabral on 10/09/2011
//  Copyright 2011 SO 2011/2012 - DEI - FTCUC All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#define RESOLUTION 1000000000

void setupRand();
float randint();
int isInsideCircle(float x,float y);

int main (int argc, const char * argv[])
{
	clock_t start, stop;
    double c = 0, pi, duration;
    long i;
    // seed the random number generator (must be done once
    // in each different process)
    setupRand();
	// start counting the time it will take to get the desired 
	// approximation
    start = clock();    
    // generate the exact number (RESOLUTION) of random coordinates 
    // and count how many are inside the circle
    for (i=0; i<RESOLUTION; i++) {
        float x = randint();
        float y = randint();
        c += isInsideCircle(x, y);
    }
    // calculate the number of PI based on the number of points
    // that fall in the circle
    pi = 4 * c/RESOLUTION;
    // stop countinf time
    stop = clock();
    // show the result
    printf("PI is: %f\n", pi);
    
    // calculate time taken for the operation
	duration = ( double ) ( stop - start ) / CLOCKS_PER_SEC;
	// show the time spent in the operation
	printf("Time taken to process log file: %.3lf seconds\n", duration);
    return 0;
}


// check if a coordinate is below or above the circle line
int isInsideCircle(float x,float y) {
    return (sqrt( pow(x,2) + pow(y,2)) <= 1) ? 1 : 0;
}

// seed the random numbers generator
void setupRand() {
    time_t seconds;
    time(&seconds);
    srand(((unsigned int) seconds) + getpid());
}

// set a limit for the random number generator
float randint() {
    return rand() / (float) RAND_MAX;
}
