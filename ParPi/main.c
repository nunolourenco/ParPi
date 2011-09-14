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
#include <errno.h>



#define NCORES 2
#define RESOLUTION 1000000000

#define BUFFER 100

int pid[NCORES];
int pipe_fd[NCORES][2];
fd_set pipeinfo;

void setupRand();
float randint();
int isInsideCircle(float x,float y);
void doWork(int experiments, int pipe);
void buildSelect();
void init_time();
void master();
int get_time();
int main(int argc, const char * argv[]) {

    clock_t start, stop;
    double duration;
    start = clock();
/*    init_time();*/
    
    int worker;
    long workToDo;
    for (worker = 0; worker < NCORES; worker++) {
        if (pipe(pipe_fd[worker]) < 0) {
            perror ("Failed to create pipes.");
            exit(1);
        }
        
        if ((pid[worker]=fork()) < 0) {
            perror ("Fork failed.");
            exit(1);
        }
        
        if (pid[worker] == 0) {
            workToDo = RESOLUTION / NCORES;
            if (worker == NCORES-1) {
                workToDo += RESOLUTION % NCORES;
            }
            doWork(workToDo, pipe_fd[worker][1]);
            exit(0);
        }
        
    }
    master();
    for (worker = 0; worker < NCORES; worker++) {

      wait(NULL);
    }
    stop = clock();
    duration = ( double ) ( stop - start ) / (CLOCKS_PER_SEC);
    // show the time spent in the operation
    printf("Time taken to process log file: %.3lf seconds\n", duration);

    
        return 0;
}

void doWork(int experiments, int pipe) {
    setupRand();
    int i,c=0;
    char str[30];
    for (i=0; i<experiments; i++) {
        float x = randint();
        float y = randint();
        c += isInsideCircle(x, y);
    }
    sprintf(str,"%d\n",c);
    write(pipe, str, strlen(str)+1);
}

void buildSelect() {
    int i;
    FD_ZERO(&pipeinfo);
    for (i=0; i < NCORES; i++) {
        FD_SET(pipe_fd[i][0], &pipeinfo);
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


void master() {
/* Master */
    
    int points_inside, ready_fd, received_workers = 0;
    ssize_t nbytes;
    char readbuffer[BUFFER];
    
    while (received_workers < NCORES) {
        buildSelect();
        ready_fd = select(pipe_fd[NCORES-1][0]+1, &pipeinfo, (fd_set *) 0, (fd_set *) 0, NULL);
        if (ready_fd < 0 && errno != 4) {
			perror("select");
			exit(errno);
		}
        
        int listnum;
        for (listnum = 0; listnum < NCORES; listnum++) {
            if (FD_ISSET(pipe_fd[listnum][0],&pipeinfo)) {
                nbytes = read(pipe_fd[listnum][0], readbuffer, sizeof(readbuffer));
                int value;
                sscanf(readbuffer,"%d\n", &value);
                points_inside += value;
                received_workers++;
            }    
        }
    }
    printf("PI=%f\n", 4 * (points_inside/(float) RESOLUTION));
    printf("Done in %d\n", get_time());
}


static struct timeval start_time;

void init_time() {
    gettimeofday(&start_time, NULL);
}

int get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (int) (t.tv_sec - start_time.tv_sec) * 1000000
    + (t.tv_usec - start_time.tv_usec);
}
