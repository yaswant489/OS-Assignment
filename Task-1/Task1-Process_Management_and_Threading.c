#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

long shared_counter = 0;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
int ITERATIONS = 500000;

void* unsafe_increment(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        long temp = shared_counter;
        temp = temp + 1;
        shared_counter = temp;
    }
    return NULL;
}

void* safe_increment(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&counter_lock);
        long temp = shared_counter;
        temp = temp + 1;
        shared_counter = temp;
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}

void demo_race_condition() {
    pthread_t threads[4];

    printf("\n 1: Race Condition (no lock)\n");
    shared_counter = 0;
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], NULL, unsafe_increment, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);
    printf("Expected: %ld, Got (unsafe): %ld\n",
           (long)(4 * ITERATIONS), shared_counter);

    printf("\n 2: Fixed with Mutex\n");
    shared_counter = 0;
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], NULL, safe_increment, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);
    printf("Expected: %ld, Got (safe): %ld\n",
           (long)(4 * ITERATIONS), shared_counter);
}

//Semaphore

sem_t resource_sem; // allows only 2 threads to access resource at once

void* access_resource(void* arg) {
    long id = (long)arg;
    printf("[Worker-%ld] waiting for resource...\n", id);
    sem_wait(&resource_sem);
    printf("[Worker-%ld] ACQUIRED resource\n", id);
    sleep(1); // simulate work
    printf("[Worker-%ld] RELEASING resource\n", id);
    sem_post(&resource_sem);
    return NULL;
}

void demo_semaphore() {
    printf("\n 3: Semaphore (max 2 concurrent workers)\n");
    sem_init(&resource_sem, 0, 2);

    pthread_t threads[5];
    for (long i = 0; i < 5; i++)
        pthread_create(&threads[i], NULL, access_resource, (void*)i);
    for (int i = 0; i < 5; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&resource_sem);
}

//deadlock prevention
pthread_mutex_t lock_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_b = PTHREAD_MUTEX_INITIALIZER;

void* task1_ordered(void* arg) {
    pthread_mutex_lock(&lock_a);
    usleep(100000);
    pthread_mutex_lock(&lock_b);
    printf("[Task-1] Acquired A then B safely\n");
    pthread_mutex_unlock(&lock_b);
    pthread_mutex_unlock(&lock_a);
    return NULL;
}

void* task2_ordered(void* arg) {
    pthread_mutex_lock(&lock_a);
    usleep(100000);
    pthread_mutex_lock(&lock_b);
    printf("[Task-2] Acquired A then B safely\n");
    pthread_mutex_unlock(&lock_b);
    pthread_mutex_unlock(&lock_a);
    return NULL;
}

void demo_deadlock_prevention() {
    printf("\n 4: Deadlock Prevention (consistent lock ordering)\n");
    pthread_t t1, t2;
    pthread_create(&t1, NULL, task1_ordered, NULL);
    pthread_create(&t2, NULL, task2_ordered, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("No deadlock both threads always request locks in the same order.\n");
}

//Round robin scheduel


typedef struct {
    int pid;
    int burst_time;
    int remaining_time;
    int waiting_time;
    int completion_time;
} Process;

void round_robin_scheduler(Process procs[], int n, int quantum) {
    printf("\n 5: Round-Robin Scheduler (quantum=%d)\n", quantum);

    int* queue = malloc(sizeof(int) * n * 20);
    int front = 0, back = 0;
    for (int i = 0; i < n; i++)
        queue[back++] = i;

    int time_elapsed = 0;
    printf("Execution order (PID, start, end):\n");

    while (front < back) {
        int idx = queue[front++];
        Process* p = &procs[idx];

        int run_time = (p->remaining_time < quantum) ? p->remaining_time : quantum;
        printf("  P%d: %d -> %d\n", p->pid, time_elapsed, time_elapsed + run_time);

        time_elapsed += run_time;
        p->remaining_time -= run_time;

        if (p->remaining_time > 0) {
            queue[back++] = idx;
        } else {
            p->completion_time = time_elapsed;
            p->waiting_time = p->completion_time - p->burst_time;
        }
    }

    int total_wait = 0;
    for (int i = 0; i < n; i++)
        total_wait += procs[i].waiting_time;

    printf("Average waiting time: %.2f\n", (float)total_wait / n);
    free(queue);
}


int main() {
    demo_race_condition();
    demo_semaphore();
    demo_deadlock_prevention();

    Process procs[3] = {
        {1, 10, 10, 0, 0},
        {2, 5, 5, 0, 0},
        {3, 8, 8, 0, 0}
    };
    round_robin_scheduler(procs, 3, 3);

    return 0;
}
