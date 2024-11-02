#ifndef DEQUE_H
#define DEQUE_H

typedef struct scheduler scheduler; 
typedef void (*taskfunc)(void*, struct scheduler *);


typedef struct Task {
    taskfunc function;      
    void *parameters;       
} Task;


typedef struct Deque {
    int capacity;           
    int nb_tasks;           
    int top;                
    int bottom; 
    int was_oisif;            
    Task **tasks;              
} Deque;

Deque *create_deque(int capacity);
Task* create_task(taskfunc f, void *parameters);
int isEmpty(Deque *deque);
int isFull(Deque *deque);
void push_top(Deque *deque, taskfunc f, void *parameters);
void push_bottom(Deque *deque, taskfunc f, void *parameters);
Task *pop_top(Deque *deque);
Task *pop_bottom(Deque *deque);
void free_Deque(Deque *deque);

#endif 
