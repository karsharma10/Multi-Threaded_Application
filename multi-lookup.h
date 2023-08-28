#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H
//include statements:
#include "stack.h"
#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<stdlib.h>
#include <sys/time.h>
#include "util.h"

//lets define our preset limits here:

//Maximum number of hostname file arguments allowed:
#define MAX_INPUT_FILES 100

//Maximum number of concurrent requester threads allowed
#define MAX_REQUESTER_THREADS 10

//Maximum number of concurrent resolver threads allowed
#define MAX_RESOLVER_THREADS 10

//- INET6_ADDRSTRLEN is the maximum size IP address string util.c will return
#define MAX_IP_LENGTH INET6_ADDRSTRLEN 

//The pool of threads will service a set of text files as input, each name read should be placed on the shared array.
//This is our requesters struct, where it will service an input file, and check if there are any remaining input files left to service. If so, it requests one of the remaining files
//When writting to the service file, it should have a mutex to only allow one thread to write to the file at a time.
//If there are no more input files remaining, the thread write the number of files it serviced to stdout
typedef struct Requesters{ 
    
    
    pthread_mutex_t serviced_lock; // Now, we need a mutex in the requestors struct because we want to make sure that only one thread at a time can write to the file (requestor log file)
    pthread_mutex_t file_position_lock; //because we have a global index count, we do not want in inccur a racing scheme while incrementing global index, so we need another mutex.
    stack * shared_buffer; //use the stack class we created to add items to the shared_buffer which will handel the thread-safe cases for us, so we will intialize a pointer here
    //Where each threadname will be placed on the shared_buffer 
    int global_index; //this will be the index we use to see if all files have been serviced or not

    char** input_files; //this will be a list of input files
    FILE * serviced_log; //this is the argument #3, which is the requestor log file name, in which the requested hostnames are written. 
    int file_count; //this will be our argument 5, where it is the number of files that we are given through the command line.

}Requesters;

//A pool of threads will be comprised off resolver threads. A resolver thread consumes the shared array by taking a name off the array and resolving its IP address.
//After the name has been mapped to an IP address, the output is written to a line in the resolver logfile. 

typedef struct Resolvers{

    FILE * results_log; //argument #4, where this is the name of the file into which hostnames and resolved IP addresses are written.
    //We will need a mutex in the resolvers struct because we want to make sure that only one thread at a time can write to the resolver log, or else it will not be threadsafe
    pthread_mutex_t results_lock;

    stack * shared_buffer; //resolvers also must have access to the stack class because that is where we are going to access the shared_buffer, the resolver threads will consume the shared array by taking a name off the array and resolving its IP address.
    

}Resolvers;


void* requesters(void* args);
void* resolvers(void* args);

#endif