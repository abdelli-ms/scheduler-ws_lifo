#ifndef SCHED_H
#define SCHED_H
#include "deque.h" 
#include <stdatomic.h>



typedef struct scheduler{
    Deque **deques;
    pthread_t *threads;
    int nbthreads;
    atomic_int oisif;
    pthread_mutex_t lock_deque;
    pthread_mutex_t lock_var_oisif;
}scheduler;

typedef void (*taskfunc)(void*, struct scheduler *);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int init_primitives_sync(scheduler *scheduler);
int findThreadIndex(struct scheduler *s);
void* workStealingThread(void *arg);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);
void freeScheduler(struct scheduler *sched);

#endif 