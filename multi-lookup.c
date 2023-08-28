#include "multi-lookup.h"
#include "stack.h"
#define NOT_RESOLVED "NOT_RESOLVED"


//we want to know create our requestors function, where we pass in void* args as the paramter. What this will do is that it will puts files on the shared buffer using stack_put
//the stack implemented functions shoudl take care of the concurrent threads running. One we do that then we will write to our file, that is passed through the parameters.

void* requesters(void* args){
    Requesters *values = (Requesters *) args; //assign our requestors struct with the arguments value passed

    unsigned long tid = pthread_self(); //returns the Pthread handle of the calling thread
    int serviced_count = 0; //this is a variable we will increment the number of files that have been serviced, which will be independent for each requester that comes in the function call
    int local_index = -1; //every thread will need this index

    //need to make sure we are calculating time:
    //struct timeval start, end;
    //use get time of day from PA3 pdf:
    //gettimeofday(&start,NULL);

    //now we will create a a while loop that will run till termination conditons:
    while(1){
        //first termination condition, if we see all the files have been processed, that means we can return the print statement and the time assiociated with it.
        pthread_mutex_lock(&(values->file_position_lock));
        if(values->global_index >= values->file_count){
           
            //gettimeofday(&end, NULL); //get the end time
            //float time = (end.tv_sec - start.tv_sec) + 1e-6*(end.tv_usec - start.tv_usec); //calculate the time 
            fprintf(stdout,"Thread %lx serviced %d files\n", tid, serviced_count); //print the thread, serviced files number, and amount of time it took
            pthread_mutex_unlock(&(values->file_position_lock)); //unlock the file at the file index
            return NULL; //we return null since there are no error to be found
        }
        pthread_mutex_unlock(&(values->file_position_lock)); //unlock the file at the file index, just incase we don't hit the first termination condish

         //now we need to increment the global index, so we an assume that this is a critical section:
        pthread_mutex_lock(&(values->file_position_lock));
        local_index = values->global_index; //grab the global index and store it to our local index variable, which is just for our thread specfifc
        values->global_index++; //increment the global index
        pthread_mutex_unlock(&(values->file_position_lock)); // now we must unlock the mutex since we are chillen and are out of critical section

        char *filename; //now we create char point to the filename that is passed through the args

        filename = values->input_files[local_index]; //set the name of the file to the input_files local index that is set before

        FILE * fd = fopen(filename, "r"); //open the file, that is passed through the filename variable. Every thread will open a different file since its incremented var

        if(fd == NULL){ //if the filename for some reason is not openable, lets just skip that and not have a lock on that thread
            
            continue;
        }

        char hostname[MAX_NAME_LENGTH];
        while(fgets(hostname,sizeof(hostname), fd)){
            if((hostname[0]) != '\n'){
                hostname[strlen(hostname)-1] = '\0';

                //now we enter a critical section, where we will use our stack to implement into buffer:
                stack_put(values->shared_buffer,hostname);
                pthread_mutex_lock(&(values->serviced_lock)); //when in print we must make sure we are in critical section:
                fprintf(values->serviced_log,"%s\n", hostname);
                pthread_mutex_unlock(&(values->serviced_lock)); // now we must unlock the mutex since we are chillen and are out of critical section
            }

          }

        fclose(fd); //must close the file after we are done
        serviced_count++;
    }

    return NULL; //NO ERRORS!

}

//now we want to create a resolvers function that will also take in void* args, and will be the consumer. It will take off the array its IP address, the name will be mapped to an
//IP address out then we will write the output to a line in the resoler logfile.

void *resolvers(void* args){

    //create our resolver struct:
    Resolvers *values = (Resolvers *) args;
    unsigned long tid = pthread_self(); //returns the Pthread handle of the calling thread

    char * allocatename = malloc(MAX_NAME_LENGTH);
    char * allocateIP = malloc(MAX_IP_LENGTH);
    int number_of_resolved_cases = 0;

    //need to make sure we are calculating time:
    //struct timeval start, end;
    //use get time of day from PA3 pdf:
    //gettimeofday(&start,NULL);

    //in our while loop we will use posion pill termination 
    while(1){
        if(stack_get(values->shared_buffer, &allocatename) != 0){ //if we get an error from our stack_get function then we write the error
            pthread_mutex_lock(&(values->results_lock));
            fprintf(stderr,"Bad hostname");
            
            pthread_mutex_unlock(&(values->results_lock));
        }

        if(strcmp(allocatename, "The End") == 0){ //termination condition, if the buffer value we get out is the posion pill then now we know we are at the end:
            //gettimeofday(&end, NULL); //get the time 
            //float time = (end.tv_sec - start.tv_sec) + 1e-6*(end.tv_usec - start.tv_usec); //calculate time
            pthread_mutex_lock(&(values->results_lock));
            fprintf(stdout,"Thread %lx resolved %d hostnames\n", tid, number_of_resolved_cases); //print thread, #resolved, and time it took
            pthread_mutex_unlock(&(values->results_lock));
            free(allocatename); //free the malloc we did on name
            free(allocateIP); //free the malloc we did on IP
            return NULL; //no errors so return null
        }

        if(dnslookup(allocatename, allocateIP, MAX_IP_LENGTH) == 0){ //look up the name, IP, and maxnamelength and make sure we actully find it
            pthread_mutex_lock(&(values->results_lock)); //critical section, if we found it then we:
            fprintf(values->results_log, "%s, %s\n", allocatename, allocateIP); //write to the logfile 
            pthread_mutex_unlock(&(values->results_lock)); //unlock critical section
            number_of_resolved_cases++; //increase resolved count
        }
        else{ //if we didn't find it then:
            pthread_mutex_lock(&(values->results_lock)); //critical section
            fprintf(values->results_log, "%s, %s\n", allocatename, NOT_RESOLVED); //write the results log as NOT RESOLVED
            pthread_mutex_unlock(&(values->results_lock)); //unlock
        }
    }

    return NULL; //NO ERRORS Return nothing
}

//now for our main funtions that will call our helper functions above:

int main(int argc, char* argv[]){
    struct timeval start,end;
    gettimeofday(&start,NULL);

    pthread_mutex_t lock;
    //pthread_mutex_t calculLock;

    if(argc > MAX_INPUT_FILES +5){ //if we have too many arguments
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Too many arguments\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }

    if(argc < 6){ //if we have less arguments than what we need we print an error
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Not enough arguments\n");
        pthread_mutex_unlock(&lock);
        return -1; //error
    }

    int num_requesters = atoi(argv[1]);
    int num_resolvers = atoi(argv[2]);
    
    if(num_requesters<1){
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Invalid thread request\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }

    if(num_resolvers<1){
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Invalid thread request\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if(num_requesters>MAX_REQUESTER_THREADS){
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Invalid thread request\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }
    if(num_resolvers>MAX_RESOLVER_THREADS){
        pthread_mutex_lock(&lock);
        fprintf(stdout,"Error: Invalid thread request\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }

    char *file_list[argc - 5]; //the length of the file array is in the input
    for(int i=0; i<argc-5; i++){
        //create a list of file names
        file_list[i] = malloc (sizeof(char) * MAX_NAME_LENGTH );
    }
    
    stack buffer; //intialize our stack, which controls the buffer
    stack_init(&buffer); //intialize our stack
    Requesters reqester; //initlaize our requester struct
    Resolvers resolver; //intialize our resolver struct
    pthread_t id_of_requesters[num_requesters]; //intialize
    pthread_t id_of_resolvers[num_resolvers]; //intialize

    for(int i=5; i<argc; i++){
        if(access(argv[i], F_OK) !=0){
            continue;
        }else{
            strcpy(file_list[i-5], argv[i]);

        }

    }
    
    reqester.input_files = file_list; //asign input file
    reqester.file_count = argc-5; //assign file count
    reqester.global_index = 0; //asign global index
    pthread_mutex_init((&reqester.file_position_lock), 0); //create mutix
    pthread_mutex_init((&reqester.serviced_lock), 0); //create mutex
    reqester.shared_buffer = &buffer; //assign buffer
    reqester.serviced_log = fopen(argv[3], "w+"); //assign log to write


    //resolver intialization:
    pthread_mutex_init((&resolver.results_lock), 0); 
    resolver.shared_buffer = &buffer;
    resolver.results_log = fopen(argv[4], "w+");


    //now its time to create the threads with is given from the command line:
    for(int k=0; k<num_requesters; k++){
        pthread_create(&id_of_requesters[k], NULL, requesters, &reqester); //creates a new thread and runs the function that is passed and a generic pointer to memory

    }
    for(int k=0; k<num_resolvers; k++){
        pthread_create(&id_of_resolvers[k], NULL, resolvers, &resolver);
    }   
    for(int k=0; k<num_requesters; k++){
        pthread_join(id_of_requesters[k], NULL); //wait till all requester threads are done.
    }

    //we must wait until the buffer it complety empty before we can inject the poison pill which is our termination condish:
    //while(buffer.counter != 0);
    for(int k =0; k<num_resolvers; k++){ //iterate over each resolver
        stack_put(&buffer, "The End"); //add the pill into the buffer
    }

    for(int k =0; k < num_resolvers; k++){
        pthread_join(id_of_resolvers[k], NULL);
    }
    //destory all mutex created!
    pthread_mutex_destroy(&(reqester.file_position_lock));
    pthread_mutex_destroy(&(reqester.serviced_lock));
    pthread_mutex_destroy(&(resolver.results_lock));
    pthread_mutex_destroy(&(lock)); //destory lock mutex
    //free filelist var
    for(int k=0; k<argc-5; k++){
        free(file_list[k]);
    }

    //free our stack!
    stack_free(&buffer);
    //close files:
    fclose(reqester.serviced_log);
    fclose(resolver.results_log);

    gettimeofday(&end, NULL);
    
    float time = (end.tv_sec - start.tv_sec) + 1e-6*(end.tv_usec - start.tv_usec);
    //pthread_mutex_lock(&calculLock);
    fprintf(stdout,"./multi-lookup: total time is %f seconds\n", time);
    //pthread_mutex_unlock(&calculLock);
    

   // pthread_mutex_destroy(&(calculLock)); //destory lock mutex
    return 0;
}