#include "stack.h"
#include "multi-lookup.h"

int stack_init(stack *s){ //intitalizing our stack, and setting our variables,semaphores,and mutex
    s->counter = 0;
    

    //malloc our array we defined in stack.h:
    for(int k =0; k < ARRAY_SIZE; k++){
        s->array[k] = (char *) malloc(MAX_NAME_LENGTH * sizeof(char));
    }

    //got help from office hours to intialize mutex,semaphores:

    pthread_mutex_init(&(s->mutex),0);
    sem_init(&(s->emtpy),0, ARRAY_SIZE); //intialize our semaphore to the array size
    sem_init(&(s->full),0, 0); //intialize our semaphore to 0

    return 0; //since this is an int function, returning 0 is making us known that everything is working properly.
}

int stack_put(stack *s, char *hostname){ //producer

    //need to check if the length of hostname is greater than our predefined max_length:
    if(strlen(hostname) > MAX_NAME_LENGTH){
        //printf("This is an error, we cannot handle hostnames that are greater than max_name_length\n");
        return -1; //we turn -1 because its an error
    }

    //so now we call semaphore wait, which will decrement the value of empty by 1, if the value of empty is zero then we know we should block this
    //process since there are no empty slots to add an item too since its full.
    sem_wait(&(s->emtpy)); //decrement value of empty by 1

    //now we are entering our critical section we so must use mutex to lock this critical section since if a producer is writting, a consumer can not be consuming.
    pthread_mutex_lock(&(s->mutex));

    //critical section:
    if(strncpy(s->array[s->counter], hostname, MAX_NAME_LENGTH) == NULL){
        //printf("The string you are trying to access cannot be found");
        return 0;
    }

    s->counter++; //increment our current position in the stack.

    pthread_mutex_unlock(&(s->mutex)); //now that we are done with the critical section we can use our mutex to unlock so that another thread may enter
    //unlocks the sempahore full variable if consumer is waiting
    //If the semaphore value is positive from this, then no threads were blocked waiting for this semaphore to be unlocked, so we simply just increment its value by 1. 
    //Because we added a element to the buffer. 
    sem_post(&(s->full)); 
    return 0; //return 0 has operation has been complete.

}

int stack_get(stack *s, char **hostname){ //consumer

    sem_wait(&(s->full)); //decrement the value of full by 1, this will check to see if there is anything even in the stack to remove.
    //if there is nothing in the stack to remove then this will be blocked here and wait till it is incremented in the stack_put

     //from office hours

    //now we are entering critical section so we must lock our mutex variable:

    pthread_mutex_lock(&(s->mutex));

    //just in case our counter is zero, we should exit, however this should never happen because we are waiting on full semaphore, and it goes to zero or less than zero
    //then we wait.
    //now we are in the critical section:

    if(strncpy(*hostname, s->array[s->counter-1], MAX_NAME_LENGTH) == NULL){
        //printf("Could not find that hostname that was provided\n");
        //
        return -1;
    }

    //decrement our counter by 1:
    s->counter--;
    

    //unlock:
    pthread_mutex_unlock(&(s->mutex));

    //unlocks the sempahore emtpy variable if producer is waiting
    //If the semaphore value is positive from this, then no threads were blocked waiting for this semaphore to be unlocked, so we simply just increment its value by 1. 
    
    sem_post(&(s->emtpy));

    return 0; //sucess status
}

void stack_free(stack *s){
    //destory sempahores:
    sem_destroy(&(s->emtpy));
    sem_destroy(&(s->full));
    pthread_mutex_destroy(&(s->mutex));

    //now free buffer:
    for(int k =0; k < ARRAY_SIZE; k++){
        free(s->array[k]);
    }
    
}