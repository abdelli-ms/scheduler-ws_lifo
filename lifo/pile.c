#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pile.h"

Stack *create_Stack(int capacity)
{
    Stack *stack =  malloc(sizeof(*stack));
    stack->capacity = capacity;
    stack->nb_tasks = 0;
    stack->tasks = NULL;
    return stack;
}

Task *create_task(taskfunc f, void *parameters)
{
    Task *new_task = (Task *)malloc(sizeof(Task));
    if (new_task == NULL)
    {
        perror("Memory allocation error");
        return NULL;
    }
    new_task->function = f;
    new_task->parameters = parameters;
    new_task->next = NULL;
    return new_task;
}

int isEmpty(Stack *stack)
{
    return (stack->nb_tasks <= 0);
}

int isFull(Stack *stack)
{
    return (stack->nb_tasks == stack->capacity);
}

int push(Stack *stack, taskfunc f, void *parameters)
{
    if (stack->nb_tasks >= stack->capacity)
    {
        errno = EAGAIN;
        return -1;
    }

    Task *new_task = create_task(f, parameters);
    new_task->next = stack->tasks;
    stack->tasks = new_task;
    stack->nb_tasks++;
    return 0;
}

Task *pop(Stack *stack)
{
    if (isEmpty(stack))
    {
        return NULL;
    }
    Task *task = stack->tasks;
    stack->tasks = task->next;
    stack->nb_tasks--;
    return task;
}

void freePile(Stack *stack)
{
    while (!isEmpty(stack))
    {
        Task *temp = pop(stack);
        free(temp);
    }
    free(stack);
}

