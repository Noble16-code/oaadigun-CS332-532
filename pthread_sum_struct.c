/* pthread_sum_struct.c
   Modified pthread_sum.c to use a per-thread structure passed to each thread.
   Removes global variables a, sum, N, size.
   Compile: gcc -O -Wall pthread_sum_struct.c -lpthread -o pthread_sum_struct
   Run: ./pthread_sum_struct <# elements> <# threads>
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct thread_info {
    double *a;               // pointer to array elements
    double *global_sum;      // pointer to shared sum
    int N;                   // total number of elements
    int size;                // number of threads
    long tid;                // thread index
    pthread_mutex_t *mutex;  // pointer to shared mutex
} thread_info_t;

void *compute(void *arg) {
    thread_info_t *tinfo = (thread_info_t *)arg;
    int myStart, myEnd, myN, i;
    long tid = tinfo->tid;

    // determine start and end of the work for this thread
    myN = tinfo->N / tinfo->size;
    myStart = tid * myN;
    myEnd = myStart + myN;

    if (tid == tinfo->size - 1)
        myEnd = tinfo->N; // last thread gets leftovers

    // compute partial sum
    double mysum = 0.0;
    for (i = myStart; i < myEnd; i++)
        mysum += tinfo->a[i];

    // update global sum with mutex
    pthread_mutex_lock(tinfo->mutex);
    *(tinfo->global_sum) += mysum;
    pthread_mutex_unlock(tinfo->mutex);

    // free the per-thread struct
    free(tinfo);

    return NULL;
}

int main(int argc, char **argv) {
    long i;
    pthread_t *tid;
    double *a;
    double sum = 0.0;
    int N, size;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    if (argc != 3) {
        printf("Usage: %s <# elements> <# threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    N = atoi(argv[1]);
    size = atoi(argv[2]);

    if (N <= 0 || size <= 0) {
        printf("Both arguments must be positive integers.\n");
        exit(EXIT_FAILURE);
    }

    // allocate array and initialize
    tid = (pthread_t *)malloc(sizeof(pthread_t) * size);
    a = (double *)malloc(sizeof(double) * N);

    for (i = 0; i < N; i++)
        a[i] = (double)(i + 1);

    // create threads
    for (i = 0; i < size; i++) {
        thread_info_t *tinfo = malloc(sizeof(thread_info_t));
        tinfo->a = a;
        tinfo->global_sum = &sum;
        tinfo->N = N;
        tinfo->size = size;
        tinfo->tid = i;
        tinfo->mutex = &mutex;

        pthread_create(&tid[i], NULL, compute, (void *)tinfo);
    }

    // join threads
    for (i = 0; i < size; i++)
        pthread_join(tid[i], NULL);

    printf("The total is %g, it should be equal to %g\n",
        sum, ((double)N * (N + 1)) / 2);

    free(a);
    free(tid);
    pthread_mutex_destroy(&mutex);

    return 0;
}
