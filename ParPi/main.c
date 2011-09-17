//
//  main.c
//  
//
//  Created by Alcides Fonseca on 9/14/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <signal.h>
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
#define RESOLUTION 100000000

#define BUFFER 100

int pid[NCORES];
int pipe_fd[NCORES][2];
fd_set pipeinfo;
int handle_signals = 0;


void workers_sigint_handler(int signum);
void sigint_handler(int signum);
void setupRand();
float randfloat();
int isInsideCircle(float x,float y);
void doWork(int experiments, int pipe);
void buildSelect();
void master();
int main(int argc, const char * argv[]) {
    int worker;
    long workToDo;
    if(argc == 2) {
      printf("Press <CTRL>-C to close the application\n");
      handle_signals = atoi(argv[1]);
    }
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
            if(handle_signals) {
              signal(SIGINT, workers_sigint_handler);
            }
            workToDo = RESOLUTION / NCORES;
            if (worker == NCORES-1) {
                workToDo += RESOLUTION % NCORES;
            }
            doWork(workToDo, pipe_fd[worker][1]);
            if(handle_signals) {
              pause();
            }
            else {
              exit(0);
            }
        }
        
    }
    master();
    for (worker = 0; worker < NCORES; worker++) {
      wait(NULL);
    }
    return 0;
}

void master_sigint_handler(int signum) {
  int i;
  printf("Received a sigint signal! Aborting all calculations!\n");
  for(i = 0; i < NCORES;i++){
    kill(pid[i], SIGINT);
  }
  exit(0);
}

void workers_sigint_handler(int signum) {
  printf("Received a sigint! Father shutting me down\n");
  exit(0);
}

void doWork(int experiments, int pipe) {
    setupRand();
    int i,c=0;
    char str[30];
    for (i=0; i<experiments; i++) {
        float x = randfloat();
        float y = randfloat();
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
float randfloat() {
    return rand() / (float) RAND_MAX;
}


void master() {
/* Master */
    
    if(handle_signals) {
      signal(SIGINT, workers_sigint_handler);
    }
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
}


