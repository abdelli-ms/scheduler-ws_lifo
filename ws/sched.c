#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "sched.h"

int init_primitives_sync(scheduler *scheduler) {
    
    int erreur=0;
    erreur = pthread_mutex_init(&scheduler->lock_deque,NULL);
    if(erreur != 0) {
        perror("Failed to init mutex");
        return -1;
    }
    atomic_store_explicit(&scheduler->oisif, 0, memory_order_relaxed);
    return 0;
}




int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    
    scheduler *scheduler = malloc(sizeof(*scheduler));
    if (!scheduler) {
        perror("Failed to allocate memory for scheduler");
        return -1;
    }
    
    if (init_primitives_sync(scheduler) == -1) {
        free(scheduler);
        return -1;
    }

    scheduler->nbthreads = (nthreads == 0) ? sched_default_threads() : nthreads;
    
    scheduler->deques = malloc(sizeof(Deque*)*(scheduler->nbthreads));
    if (scheduler->deques == NULL) {
        perror("Failed to allocate memory for deques pointers");
        free(scheduler);
        return -1;
    }

    for (int i = 0; i < scheduler->nbthreads; i++) {
        scheduler->deques[i] = create_deque(qlen+1);
        if (scheduler->deques[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for deque at index %d\n", i);
            for (int j = 0; j < i; j++) free_Deque(scheduler->deques[j]);
            free(scheduler->deques);
            free(scheduler);
            return -1;
        }
    }
   
    scheduler->threads = malloc(scheduler->nbthreads * sizeof(pthread_t));
    if (scheduler->threads == NULL) {
        perror("Failed to allocate memory for threads");
        for (int i = 0; i < scheduler->nbthreads; i++) free_Deque(scheduler->deques[i]);
        free(scheduler->deques);
        free(scheduler);
        return -1;
    }

    push_bottom(scheduler->deques[0], f, closure);  

    srand(time(NULL));
    for (int i = 0; i < scheduler->nbthreads; i++) {
        if (pthread_create(&scheduler->threads[i], NULL, workStealingThread, scheduler)) {
            fprintf(stderr, "Failed to create thread at index %d\n", i);
            for (int j = 0; j < i; j++) 
            {
                pthread_join(scheduler->threads[j], NULL);
                int join_res = pthread_join(scheduler->threads[j], NULL);
                if (join_res != 0) {
                    fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(join_res));
                }
            }
            for (int k = 0; k < scheduler->nbthreads; k++) free_Deque(scheduler->deques[k]);
            free(scheduler->deques);
            free(scheduler->threads);
            free(scheduler);
            return -1;
        }
    }

    for (int i = 0; i < scheduler->nbthreads; i++) {
        int join_res = pthread_join(scheduler->threads[i], NULL);
        if (join_res != 0) {
            fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(join_res));
        }
    }

    freeScheduler(scheduler);  

    return 0;
}
void freeScheduler(struct scheduler *sched) {
    if (sched == NULL) return;
    pthread_mutex_destroy(&sched->lock_deque);
    for (int i = 0; i < sched->nbthreads; i++) {
        free_Deque(sched->deques[i]);
    }
    free(sched->deques);
    free(sched->threads);
    free(sched);
}

int findThreadIndex(struct scheduler *s){
	pthread_t t = pthread_self();
	for (int i = 0; i < s->nbthreads; i++)
		if(pthread_equal(t, s->threads[i])){
            return i;
        } 
	return -1;
}



void* workStealingThread(void* arg) {
    struct scheduler *sched = (scheduler *)arg;
    int thread_id = findThreadIndex(sched); 
    struct timespec ts = {0, 1000000};

    while (1) {

        Task *task = NULL;
      
        pthread_mutex_lock(&sched->lock_deque);
        task = pop_bottom(sched->deques[thread_id]);
        pthread_mutex_unlock(&sched->lock_deque);
        
            
        if (task == NULL) {
            
            int k = rand() % sched->nbthreads;
            int found_task = 0;

            for (int i = 0; i < sched->nbthreads; i++) {
                int idx = (k + i) % sched->nbthreads;
                if (idx != thread_id) {
                    pthread_mutex_lock(&sched->lock_deque);
                    task = pop_top(sched->deques[idx]);
                    pthread_mutex_unlock(&sched->lock_deque);
                    if (task != NULL) {
                        found_task = 1;                        
                        break;
                    }
                }
            }

            if (!found_task) {
                if(sched->deques[thread_id]->was_oisif==0){
                    atomic_fetch_add_explicit(&sched->oisif, 1, memory_order_relaxed);
                    sched->deques[thread_id]->was_oisif=1; 
                }
                if (atomic_load_explicit(&sched->oisif, memory_order_relaxed) == sched->nbthreads) {
                    break; 
                }
                nanosleep(&ts, NULL); 
                continue;
            }
        }

        if (task == NULL) {
            break;  
        }
        
        if(sched->deques[thread_id]->was_oisif==1){
            atomic_fetch_sub_explicit(&sched->oisif, 1, memory_order_relaxed);
            sched->deques[thread_id]->was_oisif=0;
            
        }
        task->function(task->parameters, sched);
        free(task);
    }

    return NULL;
}
int sched_spawn(taskfunc f, void *closure, scheduler *sched) {
    int thread_id = findThreadIndex(sched); 
    
    pthread_mutex_lock(&sched->lock_deque);
    push_bottom(sched->deques[thread_id], f, closure);
    pthread_mutex_unlock(&sched->lock_deque);
        
    
    return 0;
}