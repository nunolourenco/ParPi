//
//  main.c
//  
//
//  Created by Alcides Fonseca on 9/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <limits.h>
#include <string.h>




#define NCORES 4
#define RESOLUTION 100000000

#define BUFFER 100

int pid[NCORES];
int pipe_fd[NCORES];
fd_set pipeinfo;

void setupRand();
float randint();
int isInsideCircle(float x,float y);
void doWork(int experiments, int pipe);
void buildSelect();


int main(int argc, const char * argv[]) {
    
    if (pipe(pipe_fd) < 0) {
        perror ("Failed to create pipes.");
        exit(1);
    }
    int worker;
    int workToDo;
    for (worker = 0; worker < NCORES; worker++) {
        if ((pid[worker]=fork()) < 0) {
            perror ("Fork failed.");
            exit(1);
        }
        
        if (pid[worker] == 0) {
            workToDo = RESOLUTION / NCORES;
            if (worker == NCORES-1) {
                workToDo += RESOLUTION % NCORES;
            }
            doWork(workToDo, pipe_fd[worker]);
            exit(0);
        }
    }
    
    /* Master */
    printf("Master\n");
    
    
    int ready_fd;
    while (1) {
        buildSelect();
        ready_fd = select(pipe_fd[NCORES-1]+1, &pipeinfo, (fd_set *) 0, (fd_set *) 0, NULL);
        printf("%d pipes ready\n", ready_fd);
        if (ready_fd < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}
    
    }
    
}

void doWork(int experiments, int pipe) {
    printf("Im in worker %d\n", getpid());
    setupRand();
    int i,c=0;
    char str[30];
    for (i=0; i<experiments; i++) {
        float x = randint();
        float y = randint();
        c += isInsideCircle(x, y);
    }
    sprintf(str,"%d",c);
    write(pipe, str, strlen(str)+1);
    printf("done\n");
}

void buildSelect() {
    int i;
    FD_ZERO(&pipeinfo);
    for (i=0; i < NCORES; i++) {
        FD_SET(pipe_fd[i], &pipeinfo);
    }
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