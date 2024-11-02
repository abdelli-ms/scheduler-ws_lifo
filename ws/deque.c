#include "deque.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

Deque *create_deque(int capacity ) {
    Deque *deque =  malloc(sizeof(*deque));
    if (!deque) return NULL;

    deque->capacity = capacity;
    deque->nb_tasks = 0;
    deque->top = 0;
    deque->bottom = -1;
    deque->was_oisif =0; 
    deque->tasks = malloc(sizeof(Task*) * (capacity));
    if (!deque->tasks) { 
        free(deque);
        return NULL;
    }

    return deque;
}

Task* create_task(taskfunc f, void *parameters) {
    Task *new_task = malloc(sizeof(Task));
    if (new_task == NULL) {
        return NULL;
    }
    new_task->function = f;
    new_task->parameters = parameters;
    return new_task;
}

void free_Deque(Deque *deque) {
    if (deque != NULL) {
        free(deque->tasks); 
        free(deque); 
    }
}


int isEmpty(Deque *deque) {
    return deque->nb_tasks == 0;
}

int isFull(Deque *deque) {
    return deque->nb_tasks == deque->capacity;
}

void push_top(Deque *deque, taskfunc f, void *parameters) {
    if (isFull(deque)) {
        errno = EAGAIN;
        return;
    }
    
    int new_top = (deque->top - 1 + deque->capacity) % deque->capacity;

    deque->tasks[new_top] = create_task(f, parameters);;
    deque->top = new_top;
    deque->nb_tasks++;
    return;
}

void  push_bottom(Deque *deque, taskfunc f, void *parameters) {
    if (isFull(deque)) {        
        errno = EAGAIN;
        return ;
    }
    int new_bottom = (deque->bottom + 1) % deque->capacity;
    deque->tasks[new_bottom]=create_task(f, parameters);
    deque->bottom = new_bottom;
    deque->nb_tasks++;
    return ;
}

Task *pop_top(Deque *deque) {
    if (isEmpty(deque)) {
        return NULL;
    }
    Task *task = deque->tasks[deque->top];
    deque->top = (deque->top + 1) % deque->capacity;
    deque->nb_tasks--;
    return task;
}

Task *pop_bottom(Deque *deque) {
    if (isEmpty(deque)) {
        return NULL;
    }
    Task *task = deque->tasks[deque->bottom];
    deque->bottom = (deque->bottom - 1 + deque->capacity) % deque->capacity;
    deque->nb_tasks--;
    return task;
}

