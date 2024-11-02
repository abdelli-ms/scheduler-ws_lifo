#ifndef PILE_H
#define PILE_H


#define EMPTY_PILE_ERROR "Empty pile error"
#define FULL_PILE_ERROR "Full pile error"


typedef struct scheduler scheduler;  // Déclaration avancée du type scheduler
typedef void (*taskfunc)(void*, struct scheduler *);  // Déclaration avancée de la fonction taskfunc
typedef struct Task{
    taskfunc function;
    void *parameters;
    struct Task *next;
}Task;

typedef struct Stack{
    int  capacity;
    int nb_tasks;
    Task *tasks;
    //sem_t mutex;
}Stack;

Stack* create_Stack(int capacity);
Task* create_task(taskfunc f, void *parameters);
int isEmpty(Stack *stack);
int  isFull(Stack *stack);
int push(Stack *stack,taskfunc f, void *parameters);
Task* pop(Stack *stack);
void freePile(Stack *stack);

#endif 

