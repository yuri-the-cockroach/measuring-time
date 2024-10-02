#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

const long SAMPLE_SIZE = (long)1e9;
extern int THREADS;

void DisplayTime(const char *restrict text, long time) {
    printf("%s %lds:%.3ldms:%.3ldus:%.3ldns\n", text, time / (long)1e9,
           time / (long)1e6 % (long)1e3, time / (long)1e3 % (long)1e3,
           time % (long)1e3);
}

long GetTimeNS(void) {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * (long)1e9 + time.tv_nsec;
}


int stick_this_thread_to_core(int core_id) {
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   pthread_t current_thread = pthread_self();
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

void* TimeThread(void *thread) {
    long timesSum = 0;
    long thisTime = GetTimeNS();
    long lastTime = 0;
    long *toReturn = (long *)thread;
    stick_this_thread_to_core(toReturn[0]);

    toReturn[0] = 0;

    for (long i = 0; i < SAMPLE_SIZE; i++) {
        lastTime = thisTime;
        thisTime = GetTimeNS();
        timesSum += (thisTime - lastTime);
    }
    toReturn[0] = timesSum;
    return NULL;
}

int main() {

    int THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    cpu_set_t set;

    CPU_ZERO(&set);                                // clear cpu mask
    CPU_SET(0, &set);                              // set cpu 0
    sched_setaffinity(0, sizeof(cpu_set_t), &set); // 0 is the calling process

    long timesSum = 0;
    long lastTime = 0;

    long ** data = calloc(THREADS, sizeof(long*));

    for ( int i = 0; i < THREADS; i++ ) {
        data[i] = calloc(1, sizeof(long));
    }
    pthread_t threadsPtr[THREADS];

    long timeStart = GetTimeNS();
    for ( long i = 0; i < THREADS; i++ ) {
        data[i][0] = i;
        long result = pthread_create(&threadsPtr[i], NULL, TimeThread, (void *)data[i]);
    }

    for (int i = 0; i < THREADS; i++) {
        long result_code = pthread_join(threadsPtr[i], NULL);
        assert(!result_code);
        printf("In main: Thread %d has ended. Result: ", i);
        DisplayTime("", data[i][0]);
        timesSum += data[i][0];
        printf("average time = %ldns\n", data[i][0] / SAMPLE_SIZE);
    }
    long timeEnd = GetTimeNS();
    long average = timesSum / (SAMPLE_SIZE * THREADS);

    printf("<-------------- All threads have finished -------------->\n");

    printf("Mesured time %ld times. It took", SAMPLE_SIZE * THREADS);
    DisplayTime("", timeEnd - timeStart);

    DisplayTime("Time sum:", timesSum);
    printf("average time = %ldns\n", average);

    return 0;
}
