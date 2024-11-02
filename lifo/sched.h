#ifndef SCHED_H
#define SCHED_H
#include "pile.h" 
#include <stdatomic.h>

typedef struct scheduler{
    Stack *stack;
    pthread_t *threads;
    int nbthreads;
    atomic_int count;
    int stop;
    pthread_mutex_t stack_mutex; 
    pthread_cond_t cv; 

}scheduler;

typedef void (*taskfunc)(void*, struct scheduler *);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}
void* start_thread(void *arg);
int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);

#endif 