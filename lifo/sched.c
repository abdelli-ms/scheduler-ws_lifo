#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "sched.h"

void clean_mem(scheduler *scheduler){
    freePile(scheduler->stack);
    free(scheduler->threads);
    free(scheduler);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure)
{
    int  er;

    scheduler *scheduler = malloc(sizeof(*scheduler));
    if (!scheduler) {
        perror("Failed to allocate memory for scheduler");
        return -1;
    }
    
    scheduler->stack = create_Stack(qlen + 1);
    scheduler->nbthreads = (nthreads == 0) ? sched_default_threads() : nthreads;
    atomic_store_explicit(&scheduler->count, 0, memory_order_relaxed);
    push(scheduler->stack, f, closure);
    scheduler->threads = malloc(scheduler->nbthreads * sizeof(pthread_t));
    if(scheduler->threads==NULL){
        perror("malloc");
        clean_mem(scheduler);
        return -1;
    }
    er = pthread_mutex_init(&scheduler->stack_mutex, NULL);
    if (er != 0)
    {
        fprintf(stderr, "mutex_init %s", strerror(er));
        clean_mem(scheduler);
    }
    er = pthread_cond_init(&scheduler->cv, NULL);
    if (er != 0)
    {
        fprintf(stderr, "cond_init %s", strerror(er));
        er = pthread_mutex_destroy(&scheduler->stack_mutex);
        if(er){
            fprintf(stderr,"pthread_mutex_destroy %s",strerror(er));
        }
        clean_mem(scheduler);
    }
    scheduler->stop = 0;
    int rc;
    for (int i = 0; i < scheduler->nbthreads; i++)
    {
        rc = pthread_create(&scheduler->threads[i], NULL, start_thread, scheduler);
        if (rc != 0)
        {
            fprintf(stderr,"pthread_create %s",strerror(er));
            return -1;
        }
    }
    for (int i = 0; i < scheduler->nbthreads; i++)
    {
        pthread_join(scheduler->threads[i], NULL);
    }
    er = pthread_mutex_destroy(&scheduler->stack_mutex);
    if(er){
        fprintf(stderr,"pthread_mutex_destroy");
    }
    er = pthread_cond_destroy(&scheduler->cv);
    if(er){
        fprintf(stderr,"pthread_cond_destroy");
    }
    clean_mem(scheduler);
    return 0;
}
void *start_thread(void *arg)
{
    int er;
    struct Task *task;
    struct scheduler *s = (struct scheduler *)arg;
    while (1)
    {
        er = pthread_mutex_lock(&s->stack_mutex);
        if (er != 0)
        {
            fprintf(stderr, "pthread_lock_mutex %s", strerror(er));
            return NULL;
        }
        if (s->stop != 1)
        {
            if (isEmpty(s->stack))
            {
                atomic_fetch_add_explicit(&s->count, 1, memory_order_relaxed);
                if (s->count >= s->nbthreads)
                {
                    s->stop = 1;
                    er = pthread_mutex_unlock(&s->stack_mutex);
                    if (er)
                    {
                        fprintf(stderr, "pthread_mutex_unlock %s", strerror(er));
                    }
                    er = pthread_cond_broadcast(&s->cv);
                    if (er != 0)
                    {
                        fprintf(stderr, "pthread_cond_broadcast %s", strerror(er));
                    }
                    return NULL;
                }
                er = pthread_cond_wait(&s->cv, &s->stack_mutex);
                if (er != 0)
                {
                    fprintf(stderr, "error pthread_cond_wait %s", strerror(er));
                    return NULL;
                }
                atomic_fetch_sub_explicit(&s->count, 1, memory_order_relaxed);
            }
            if (isEmpty(s->stack))
            {
                er = pthread_mutex_unlock(&s->stack_mutex);
                if (er != 0)
                {
                    fprintf(stderr, "error pthread_mutex_task NULL %s", strerror(er));
                    return NULL;
                }
                continue;
            }
            task = pop(s->stack);
            er = pthread_mutex_unlock(&s->stack_mutex);
            if (er != 0)
            {
                fprintf(stderr, "pthread_unlock_mutex %s", strerror(er));
                return NULL;
            }
            task->function(task->parameters, s);
            free(task);
            er = pthread_mutex_lock(&s->stack_mutex);
            if (er != 0)
            {
                fprintf(stderr, "mutex_lock check end %s", strerror(er));
                return NULL;
            }
            if (isEmpty(s->stack) && s->count >= s->nbthreads - 1)
            {
                s->stop = 1;
                er = pthread_mutex_unlock(&s->stack_mutex);
                if (er)
                {
                    fprintf(stderr, "pthread_mutex_unlock %s", strerror(er));
                    return NULL;
                }
                er = pthread_cond_broadcast(&s->cv);
                if (er != 0)
                {
                    fprintf(stderr, "pthread_cond_broadcast %s", strerror(er));
                    return NULL;
                }
                return NULL;
            }
        }
        else if (s->stop == 1)
        {
            er = pthread_mutex_unlock(&s->stack_mutex);
            if (er != 0)
            {
                fprintf(stderr, "pthread_unlock_mutex %s", strerror(er));
                return NULL;
            }
            return NULL;
        }
        er = pthread_mutex_unlock(&s->stack_mutex);
        if (er != 0)
        {
            fprintf(stderr, "pthread_mutex_unlock %s", strerror(er));
        }
    }
    return NULL;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s)
{
    int er;
    er = pthread_mutex_lock(&s->stack_mutex);
    if (er != 0)
    {
        fprintf(stderr, "pthread_mutex_lock <> spawn: %s", strerror(er));
        return -1;
    }

    int isf;
    isf = isFull(s->stack);
    if (isf)
    {
        er = pthread_mutex_unlock(&s->stack_mutex); //
        if (er != 0)
            fprintf(stderr, "spawn mutex_unlock %s", strerror(er));
        return -1;
    }
    push(s->stack, f, closure);
    er = pthread_mutex_unlock(&s->stack_mutex); //
    if (er != 0)
    {
        fprintf(stderr, "spawn pthread_unlock %s", strerror(er));
        return -1;
    }
    er = pthread_cond_signal(&s->cv);
    if (er != 0)
    {
        fprintf(stderr, "spawn cond_signal %s", strerror(er));
        return -1;
    }
    return 0;
}
