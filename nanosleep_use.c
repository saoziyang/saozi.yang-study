#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>
#define COUNT 1000
#define MILLION 1000000L

int main(void)
{
    int i;
    struct timespec slptm;
    long   tdif;
    struct timeval tend, tstart;

    slptm.tv_sec = 0;
    slptm.tv_nsec = 1000;      //1000 ns = 1 us

    //struct sched_param param;   
    //param.sched_priority = 0;
    //sched_setscheduler(getpid(), SCHED_FIFO, &param);

    if (gettimeofday(&tstart, NULL) == -1) {
        fprintf(stderr, "Failed to get start time\n");
        return 1;
    }
    for (i = 0; i < COUNT; i++) {
        if (nanosleep(&slptm, NULL) == -1) {
            perror("Failed to nanosleep");
            return 1;
        }
    }
    if (gettimeofday(&tend, NULL) == -1) {
        fprintf(stderr, "Failed to get end time\n");
        return 1;
    }
    tdif = MILLION * (tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec);
    printf("nanosleep() time is %ld us\n", tdif/COUNT);
    return 0;
}
