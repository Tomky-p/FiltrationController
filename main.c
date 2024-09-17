#include <stdio.h>
#include <pthread.h>
#include "utils.h"
#include "gpio_utils.h"
#include <string.h>
#include <time.h>

//thread function manages the filtration process
void* automaticController();

//thread function reads and processes input from the user through the command line
void* cmdManager();

//USAGE: ./FiltrationManager [Mode][Duration (hours)][Time of day]
int main(int argc, char *argv[]){
    //argument check
    if(argc < 4){
        fprintf(stderr, "Not enough arguments, USAGE: ./FiltrationController [Mode of(-a/-m)] [Duration of filtration cycle in hours] [Time of day]\n");
        return EXIT_FAILURE;
    }
    //argument parsing
    float arg_value = 0;
    if(strncmp(argv[1], "-m", 3) == 0) config.mode = MANUAL;
    else if (strncmp(argv[1], "-a", 3) == 0) config.mode = AUTO;
    else{
        fprintf(stderr, "Invalid argument, USAGE: -a for Automatic filtration, -m for manual filtration.\n");
        return EXIT_FAILURE;
    }

    if(checkArgumentFloat(argv[2]) == false){
        fprintf(stderr, NOT_FLOAT_MSG);
        return EXIT_FAILURE;
    }
    arg_value = atof(argv[2]);
    if(arg_value <= 0 || arg_value > MAX_DURATION){
        fprintf(stderr, INVALID_DURATION_MSG, MAX_DURATION);
        return EXIT_FAILURE;
    }
    config.duration = arg_value;

    if(checkArgument(argv[3]) == false){
        fprintf(stderr, NOT_INT_MSG);
        return EXIT_FAILURE;
    }
    arg_value = (float)atoi(argv[3]);
    if(isIntTime(arg_value) == false){
        fprintf(stderr, INVALID_TIME_INPUT);
        return EXIT_FAILURE;
    }
    config.time = (int)arg_value;
    config.running = true;
    config.filtration_running = false;
    config.manual_duration = 0;

    printConfig();

    int *AF_thread_result;
    int *CMD_thread_result;

    //command line monitoring and handling thread
    pthread_t cmdMonitor;
    //filtration executor thread
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
    //join threads
    if(pthread_join(automaticFiltration, (void**)&AF_thread_result) != 0 ||
    pthread_join(cmdMonitor, (void**)&CMD_thread_result) != 0){
        fprintf(stderr, "FATAL ERR! Failed to join threads.");
        pthread_mutex_destroy(&config_mutex);
        return EXIT_FAILURE;
    }
    //shutdown if not already shutdown
    if(checkDeviceState()){
        shutdownFiltration();
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
    printf("Launching automatic filtration thread.\n");
    
    //initialize gpio pin interface
    if(initGpioPinControl() == ALLOCATION_ERR){
        config.running = false;
        *ret = GPIO_ERR;
        return (void*)ret;
    }
    pthread_mutex_lock(&config_mutex);
    while (config.running)
    {    
        int curtime = getCurrentTime();
        if(curtime == TIME_ERR){
            *ret = TIME_ERR;
            config.running = false;
            return (void*)ret;
        }

        if(config.mode == AUTO && curtime == config.time && !config.filtration_running){        
            float duration = config.duration;
            pthread_mutex_unlock(&config_mutex);
            runFilration(duration);
        }
        else if(config.mode == MANUAL && config.manual_duration > 0){
            float duration = config.manual_duration;
            //invalidate the manual duration
            config.manual_duration = 0; 
            pthread_mutex_unlock(&config_mutex);
            runFilration(duration);
        }
        else{
            pthread_mutex_unlock(&config_mutex);
        }
        delay(100); //wait 100ms NOTE: cannot be over a minute
        pthread_mutex_lock(&config_mutex);
    }
    pthread_mutex_unlock(&config_mutex);
    return (void*)ret;
}

void* cmdManager(){
    int *ret = malloc(sizeof(int));
    char *command = (char*)malloc(STARTING_CAPACITY);
    if (command == NULL){
        *ret = ALLOCATION_ERR;
        config.running = false;
        return (void*)ret;
    } 
    printf("Launching command line listener thread.\n");

    pthread_mutex_lock(&config_mutex);
    while (config.running)
    {
        pthread_mutex_unlock(&config_mutex);
        *ret = readCmd(&command);
        if(*ret == READING_SUCCESS){
            *ret = processCommand(command);
        }
        if(*ret == ALLOCATION_ERR){
            fprintf(stderr, "FATAL ERR! Memory allocation failure.\n");
            break;
        }
        if(*ret == TIME_ERR){
            fprintf(stderr, "FATAL ERR! Failed to get current time.");
            break;
        }
        pthread_mutex_lock(&config_mutex);
    }
    pthread_mutex_unlock(&config_mutex);
    config.running = false;
    free(command);
    return (void*)ret;
}
