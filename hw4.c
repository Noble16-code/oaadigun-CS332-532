#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <fcntl.h>

/* Parameters */
#define NUM_PRODUCERS 10
#define NUM_CONSUMERS 20
#define PER_PRODUCER 500
#define PER_CONSUMER 250
#define RAND_MAX_VAL 1000

/* check at compile/run time */
#if NUM_PRODUCERS * PER_PRODUCER != NUM_CONSUMERS * PER_CONSUMER
#error "Total numbers produced must equal total numbers consumed"
#endif

/* Globals for pipe and synchronization (in parent) */
int pipefd[2]; /* pipefd[1] = write end, pipefd[0] = read end */

/* Mutexes for writing and printing */
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Utility: write exactly n bytes */
ssize_t write_full(int fd, const void *buf, size_t count) {
    size_t left = count;
    const uint8_t *p = (const uint8_t*)buf;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        left -= (size_t)w;
        p += w;
    }
    return (ssize_t)count;
}

/* read exactly n bytes */
ssize_t read_full(int fd, void *buf, size_t count) {
    size_t left = count;
    uint8_t *p = (uint8_t*)buf;
    while (left > 0) {
        ssize_t r = read(fd, p, left);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return (ssize_t)(count - left); /* EOF */
        left -= (size_t)r;
        p += r;
    }
    return (ssize_t)count;
}

/* Producer thread argument */
typedef struct {
    int tid;
    unsigned int seed;
} producer_arg_t;

void *producer_thread(void *arg) {
    producer_arg_t *parg = (producer_arg_t*)arg;
    int tid = parg->tid;
    unsigned int seed = parg->seed;

    /* Generate PER_PRODUCER unique numbers within this thread.
       We'll use a simple local boolean array of size RAND_MAX_VAL+1 to ensure uniqueness. */
    int chosen_count = 0;
    int *values = malloc(sizeof(int) * PER_PRODUCER);
    if (!values) {
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr, "Producer %d: malloc failed\n", tid);
        pthread_mutex_unlock(&print_mutex);
        return NULL;
    }

    /* If PER_PRODUCER > RAND_MAX_VAL+1 we'd have trouble; here PER_PRODUCER <= 500 and RAND_MAX_VAL=1000 */
    int available = RAND_MAX_VAL + 1;
    char *seen = calloc(available, 1);
    if (!seen) {
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr, "Producer %d: calloc failed\n", tid);
        pthread_mutex_unlock(&print_mutex);
        free(values);
        return NULL;
    }

    while (chosen_count < PER_PRODUCER) {
        int r = rand_r(&seed) % (RAND_MAX_VAL + 1);
        if (!seen[r]) {
            seen[r] = 1;
            values[chosen_count++] = r;
        }
    }

    /* Write values to the pipe; protect the actual write operation with a mutex to avoid interleaved calls */
    for (int i = 0; i < PER_PRODUCER; ++i) {
        int x = values[i];
        /* lock write to ensure atomicity of our write calls and consistent progress prints */
        pthread_mutex_lock(&write_mutex);
        if (write_full(pipefd[1], &x, sizeof(int)) != sizeof(int)) {
            pthread_mutex_lock(&print_mutex);
            fprintf(stderr, "Producer %d: write error: %s\n", tid, strerror(errno));
            pthread_mutex_unlock(&print_mutex);
            pthread_mutex_unlock(&write_mutex);
            break;
        }
        pthread_mutex_unlock(&write_mutex);

        /* Progress indicator: every 50 writes print a small update (protected by print_mutex) */
        if ((i + 1) % 50 == 0 || i == PER_PRODUCER - 1) {
            pthread_mutex_lock(&print_mutex);
            printf("Producer %d: wrote %d/%d numbers\n", tid, i + 1, PER_PRODUCER);
            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
        }
    }

    pthread_mutex_lock(&print_mutex);
    printf("Producer %d finished (thread id %lu)\n", tid, (unsigned long)pthread_self());
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);

    free(values);
    free(seen);
    return NULL;
}

/* Consumer arguments and result storage */
typedef struct {
    int cid;
    long long sum;
} consumer_result_t;

typedef struct {
    int cid;
} consumer_arg_t;

/* We need a shared read descriptor in the child; mutex is not strictly necessary because read operations on a pipe are already serialized,
   but we'll not use additional locks for reads because each read is for sizeof(int) and is atomic in practice; however,
   to be safe with partial reads across threads we implement read_full. */
void *consumer_thread(void *arg) {
    consumer_arg_t *carg = (consumer_arg_t*)arg;
    int cid = carg->cid;
    long long local_sum = 0;

    for (int i = 0; i < PER_CONSUMER; ++i) {
        int x;
        ssize_t r = read_full(pipefd[0], &x, sizeof(int));
        if (r != sizeof(int)) {
            pthread_mutex_lock(&print_mutex);
            fprintf(stderr, "Consumer %d: read error or premature EOF (r=%zd)\n", cid, r);
            pthread_mutex_unlock(&print_mutex);
            pthread_exit((void*)NULL);
        }
        local_sum += x;

        /*small progress prints — print every 50 reads */
        if ((i + 1) % 50 == 0 || i == PER_CONSUMER - 1) {
            pthread_mutex_lock(&print_mutex);
            printf("Consumer %d: read %d/%d numbers\n", cid, i + 1, PER_CONSUMER);
            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
        }
    }

    consumer_result_t *cres = malloc(sizeof(consumer_result_t));
    if (!cres) pthread_exit((void*)NULL);
    cres->cid = cid;
    cres->sum = local_sum;

    pthread_mutex_lock(&print_mutex);
    printf("Consumer %d finished (thread id %lu) sum=%lld\n", cid, (unsigned long)pthread_self(), local_sum);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);

    return (void*)cres;
}

int main(int argc, char *argv[]) {
    /* Create pipe */
    if (pipe(pipefd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Fork child */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        /* Parent process: PRODUCERS */
        /* Parent will write to pipefd[1], close read end */
        close(pipefd[0]); /* close read end in parent */

        pthread_t producers[NUM_PRODUCERS];
        producer_arg_t pargs[NUM_PRODUCERS];

        /* Seed the random generator differently for each thread */
        unsigned int global_seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();

        for (int i = 0; i < NUM_PRODUCERS; ++i) {
            pargs[i].tid = i;
            pargs[i].seed = global_seed ^ (i * 101);
            if (pthread_create(&producers[i], NULL, producer_thread, &pargs[i]) != 0) {
                pthread_mutex_lock(&print_mutex);
                fprintf(stderr, "Failed to create producer %d\n", i);
                pthread_mutex_unlock(&print_mutex);
            }
        }

        /* Join producers */
        for (int i = 0; i < NUM_PRODUCERS; ++i) {
            pthread_join(producers[i], NULL);
        }

        /* All producers finished */
        pthread_mutex_lock(&print_mutex);
        printf("Parent: all producers finished, closing write end of pipe.\n");
        fflush(stdout);
        pthread_mutex_unlock(&print_mutex);

        close(pipefd[1]); /* signal EOF to child by closing write end */

        /* Wait for child to finish */
        int status;
        waitpid(pid, &status, 0);
        pthread_mutex_lock(&print_mutex);
        printf("Parent: child finished with status %d. Exiting.\n", status);
        fflush(stdout);
        pthread_mutex_unlock(&print_mutex);
        return 0;

    } else {
        /* Child process: CONSUMERS */
        /* Child will read from pipefd[0], close write end */
        close(pipefd[1]); /* close write end in child */

        pthread_t consumers[NUM_CONSUMERS];
        consumer_arg_t cargs[NUM_CONSUMERS];

        for (int i = 0; i < NUM_CONSUMERS; ++i) {
            cargs[i].cid = i;
            if (pthread_create(&consumers[i], NULL, consumer_thread, &cargs[i]) != 0) {
                pthread_mutex_lock(&print_mutex);
                fprintf(stderr, "Failed to create consumer %d\n", i);
                pthread_mutex_unlock(&print_mutex);
            }
        }

        long long sums[NUM_CONSUMERS];
        for (int i = 0; i < NUM_CONSUMERS; ++i) {
            void *res = NULL;
            pthread_join(consumers[i], &res);
            if (res) {
                consumer_result_t *cres = (consumer_result_t*)res;
                sums[i] = cres->sum;
                free(cres);
            } else {
                sums[i] = 0;
            }
        }

        /* Compute average of the sums (average per consumer) */
        long double total = 0.0L;
        for (int i = 0; i < NUM_CONSUMERS; ++i) total += (long double)sums[i];
        long double average = total / (long double)NUM_CONSUMERS;

        /* Print the average to stdout. Per assignment, the student should redirect stdout to a file if desired. */
        pthread_mutex_lock(&print_mutex);
        printf("Child: Average of consumer sums = %.6Lf\n", average);
        fflush(stdout);
        pthread_mutex_unlock(&print_mutex);

        /* close read end and exit */
        close(pipefd[0]);
        _exit(0);
    }
}
