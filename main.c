#include <stdio.h>
#include <pthread.h>
#include "utils.h"
#include <string.h>

#ifndef MAX_DURATION
//The maximum duration of water to be realeased in a cycle (in liters)
#define MAX_DURATION 6
#endif

//
#define MAX_HOURS 23
#define MAX_MINUTES 59

//thread function manages the irrigation process in automatic mode
void* automaticController();

void* cmdManager();

//USAGE: ./irrigationManager [Mode][Amount (l)][Interval (min)]
int main(int argc, char *argv[]){
    //argument check
    if(argc < 4){
        fprintf(stderr, "Not enough arguments, USAGE: ./FiltrationController [Mode of(-a/-m)] [duration of filtration cycle in hours] [Time between cycles in minutes]\n");
        return EXIT_FAILURE;
    }
    //process variables 
    //config_t init_config;
    float arg_value = 0;
    //argument parsing
    if(strncmp(argv[1], "-m", 2) == 0) config.mode = MANUAL;
    else if (strncmp(argv[1], "-a", 2) == 0) config.mode = AUTO;
    else{
        fprintf(stderr, "Invalid argument, USAGE: -a for Automatic irrigation, -m for manual irrigation.\n");
        return EXIT_FAILURE;
    }

    if(checkArgumentFloat(argv[2]) == false){
        fprintf(stderr, "Invalid argument, provided duration is not a number or is too large.\n");
        return EXIT_FAILURE;
    }
    arg_value = atof(argv[2]);
    if(arg_value <= 0 || arg_value > MAX_DURATION){
        fprintf(stderr, "Invalid argument, duration must be in between .\n");
        return EXIT_FAILURE;
    }
    config.duration = arg_value;

    if(checkArgument(argv[3]) == false){
        fprintf(stderr, "Invalid argument, provided time is not an interger or is too large.\n");
        return EXIT_FAILURE;
    }
    arg_value = (float)atoi(argv[3]);
    if(arg_value > (MAX_HOURS*10)+MAX_MINUTES || arg_value < 0){
        fprintf(stderr, "Invalid argument, provided invalid time USAGE: provide int that = (desired hour) * 10 + desired minute i.e. 22:15 = 22*10 + 15.\n");
        return EXIT_FAILURE;
    }
    config.time = (int)arg_value;
    config.running = true;

    printf("Starting with following configuration:\nmode: %d\nduration: %f\ntime: %d\n", config.mode, config.duration, config.time);

    int *AF_thread_result = NULL;
    int *CMD_thread_result = NULL;

    //command line monitoring and handling thread
    pthread_t cmdMonitor;
    //irrigation executor thread
    pthread_t automaticFiltration;

    if(pthread_mutex_init(&config_mutex, NULL) != 0){
        fprintf(stderr, "FATAL ERR! Failed to initialize configuration mutex.");
        return EXIT_FAILURE;
    }
    //create threads
    if(pthread_create(&cmdMonitor, NULL, cmdManager, NULL) != 0){
        fprintf(stderr, "FATAL ERR! Failed to create command line monitor thread.\n");
        pthread_mutex_destroy(&config_mutex);
        return EXIT_FAILURE;
    }
    if(pthread_create(&automaticFiltration, NULL, automaticController, NULL) != 0){
        fprintf(stderr, "FATAL ERR! Failed to create automatic filtration thread.\n");
        pthread_mutex_destroy(&config_mutex);
        return EXIT_FAILURE;
    }
    
    if(pthread_join(automaticFiltration, (void**)&AF_thread_result) != 0 &&
    pthread_join(cmdMonitor, (void**)&CMD_thread_result) != 0){
        fprintf(stderr, "FATAL ERR! Failed to join threads.");
        pthread_mutex_destroy(&config_mutex);
        return EXIT_FAILURE;
    }

    pthread_mutex_destroy(&config_mutex);

    int AF_ret = *AF_thread_result;
    int CMD_ret = *CMD_thread_result;
    free(AF_thread_result);
    free(CMD_thread_result);

    if(AF_ret != EXIT_SUCCESS) return AF_ret;
    if(CMD_ret != EXIT_SUCCESS) return CMD_ret;
    return EXIT_SUCCESS;
}


void* automaticController(){   
    int *ret = malloc(sizeof(int));
    *ret = EXIT_SUCCESS;
    while (config.running)
    {
        
        if(config.mode == AUTO){

        }
    }
    return (void*)ret;
}

void* cmdManager(){
    int *ret = malloc(sizeof(int));
    char *command = NULL;
    printf("Launching command line listener thread.\n");

    //command line control thread
    pthread_mutex_lock(&config_mutex);
    while (config.running)
    {
        pthread_mutex_unlock(&config_mutex);
        *ret = readCmd(&command);
        if(*ret == READING_SUCCESS){
            printf("Executing: %s\n", command);
            *ret = processCommand(command);
        }
        if(*ret == ALLOCATION_ERR){
            fprintf(stderr, "FATAL ERR! Memory allocation failure.\n");
            break;
        }
        pthread_mutex_lock(&config_mutex);
    }
    free(command);
    return (void*)ret;
}
