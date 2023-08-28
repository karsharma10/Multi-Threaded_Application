#ifndef STACK_H
#define STACK_H
//include statements:
#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<stdlib.h>


//define array size as 8 and max_name_length as 1025
#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 1025

typedef struct{ //now we define our stack features in this struct:

    char* array[ARRAY_SIZE]; //define our array be of size defined above
    int counter; //this will be our current pos in the buffer
   
    

//need semaphores and mutex to protect critical section
    sem_t emtpy;
    sem_t full;
    pthread_mutex_t mutex;

}stack;


//now we define the functions of our stack:

int stack_init(stack *s); //we must intialize our stack
int stack_put(stack *s, char *hostname); //this function will place the element on top of the stack
int stack_get(stack *s, char **hostname); //this function will remove the element on top of the stack.
void stack_free(stack *s); //we should free the stack


#endif