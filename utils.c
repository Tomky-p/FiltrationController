#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "utils.h"
#include "gpio_utils.h"
#include "cmd_proc.h"

//initialize global (thread shared) variables 
config_t config = {0};
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
//LIST OF ALL COMMANDS
- mode [mode] -(confirm)
    changes mode of the system
    parameters:
    (mode) -a automatic operation -m manual
- run [duration] -(confirm)
    run in watering cycle in manual mode
    parameters:
    (int) duration in liters to be dispensed
- config [duration] [time] -(confirm)
    change the config for automatic mode
    parameters:
    (int) new duration per cycle in liters
    (int) new time per cycle in minutes
- stop -(confirm)
    stops the current filtration cycle
- kill -(confirm)
    stops and quits the entire program

*/
int processCommand(char *input){
    char *cmd_buffer = (char*)calloc(MAX_LENGHT * sizeof(char), sizeof(char));
    char *param_buffer_first = (char*)calloc(MAX_LENGHT * sizeof(char), sizeof(char));
    char *param_buffer_second = (char*)calloc(MAX_LENGHT * sizeof(char), sizeof(char));
    //allocation check
    if(cmd_buffer == NULL || param_buffer_first == NULL || param_buffer_second == NULL){
        if(cmd_buffer != NULL) free(cmd_buffer);
        if(param_buffer_first != NULL) free(param_buffer_first);
        if(param_buffer_second !=  NULL) free(param_buffer_second);
        return ALLOCATION_ERR;
    }
    if(!splitToBuffers(input, cmd_buffer, param_buffer_first, param_buffer_second)){
        free(cmd_buffer);
        free(param_buffer_first);
        free(param_buffer_second);
        return EXIT_SUCCESS;
    }
    //printf("command: %s\nfirst parameter: %s\nsecond parameter %s\n", cmd_buffer, param_buffer_first, param_buffer_second);
    bool args_ok = true;
    if(strncmp(cmd_buffer, "mode", 5) == 0){
        processMode(param_buffer_first, param_buffer_second);
    }
    else if (strncmp(cmd_buffer, "run", 4) == 0)
    {   
        processRun(input, param_buffer_first, param_buffer_second);    
    }
    else if (strncmp(cmd_buffer, "config", 7) == 0)
    {
        processConfig(input, param_buffer_first, param_buffer_second); 
    }
    //TO DO implement the help command
    else if (strncmp(cmd_buffer, "kill", 5) == 0)
    {
        processKill(input, param_buffer_first, param_buffer_second);
    }
    else if (strncmp(cmd_buffer, "stop", 5) == 0)
    {
        processStop(input, param_buffer_first, param_buffer_second);
    }
    else{
        fprintf(stderr, "Command not recognized.\n");
    }
    free(cmd_buffer);
    free(param_buffer_first);
    free(param_buffer_second);
    return EXIT_SUCCESS;
}

int readCmd(char **command)
{
    unsigned capacity = STARTING_CAPACITY;
    unsigned curLength = 0;
    char c;

    while ((c = getchar()) != EOF && c != '\n' && c != '\0') {
        if(curLength >= MAX_LENGHT) return LENGHT_ERR;
        //increase capacity if length of message reaches capacity
        if (curLength >= capacity) {
            capacity = capacity * 2;
            char *tmp = realloc(*command, capacity);
            if (tmp == NULL){
                free(*command);
                return ALLOCATION_ERR;
            } 
            *command = tmp;
        }
        //write char into message and increase length
        (*command)[curLength] = c;
        curLength += 1;
    }
    if(curLength >= capacity){
        capacity = capacity * 2;
        char *tmp = realloc(*command, capacity);
        if (tmp == NULL){
            free(*command);
            return ALLOCATION_ERR;
        }
        *command = tmp;
    }
    (*command)[curLength] = '\0';

    return READING_SUCCESS;
}
 
bool checkArgument(char *arg){
    if(arg == NULL) return false;
    int index = 0;
    while ((arg[index] != '\0' && arg[index] != '\n') && index < MAX_DIGITS)
    {
        if(arg[index] < '0' || arg[index] > '9') return false; 
        index++;
    }
    if(arg[index] != '\0' && arg[index] != '\n') return false; 
    return true;
}

bool checkArgumentFloat(char *arg){
    if(arg == NULL) return false;
    int index = 0;
    bool hasDecimalPoint = false;
    while ((arg[index] != '\0' && arg[index] != '\n') && index < MAX_DIGITS)
    {
        if(arg[index] == '.' && hasDecimalPoint) return false;
        if((arg[index] < '0' || arg[index] > '9') && arg[index] != '.') return false;
        if(arg[index] == '.') hasDecimalPoint = true; 
        index++;
    }
    if(arg[index] != '\0' && arg[index] != '\n') return false; 
    return true;
}

bool splitToBuffers(char *input, char *cmd_buffer, char *param_buffer_first, char *param_buffer_second){
    int index = 0;
    int buffer_index = 0;
    int offset = 0;
    int current_buffer = 1;
    while (input[index] != '\0')
    {
        switch (current_buffer)
        {
        case 1:
            while (input[index] != ' ' && input[index] != '\0')
            {        
                cmd_buffer[index] = input[index];
                index++;
                if(index >= MAX_LENGHT){
                    fprintf(stderr, "Input parameter too long and not recongized.\n");
                    return false;
                }
            }
            cmd_buffer[index] = '\0';
            if(input[index] != '\0'){
                current_buffer++;
                index++;
            }
            break;
        case 2:
            buffer_index = 0;
            offset = index;
            while (input[index] != ' ' && input[index] != '\0')
            {        
                param_buffer_first[buffer_index] = input[index];
                index++;
                buffer_index++;
                if(buffer_index >= MAX_LENGHT - offset){
                    fprintf(stderr, "Input parameter too long and not recongized.\n");
                    return false;
                }
            }
            param_buffer_first[buffer_index] = '\0';
            if(input[index] != '\0'){
                current_buffer++;
                index++;
            }
            break;
        case 3:
            buffer_index = 0;
            offset = index;
            while (input[index] != '\0' && input[index] != ' ')
            {        
                param_buffer_second[buffer_index] = input[index];
                index++;
                buffer_index++;
                if(buffer_index >= MAX_LENGHT - offset){
                    fprintf(stderr, "Input parameter too long and not recongized.\n");
                    return false;
                }
            }
            if(input[index] != '\0'){
                fprintf(stderr, "Too many parameters, there are commands that accept 2 parameters at most.\n");
                return false;
            }
            param_buffer_second[buffer_index] = '\0';
            break;   
        default:
            fprintf(stderr, "Failed to parse command!\n");
            return false;
        }
        if(index >= MAX_LENGHT){
            fprintf(stderr, "Input command too long and not recongized.\n");
            return false;
        }
    }
    return true;
}

int recieveConfirmation(char *command){
    int ret = readCmd(&command);
    if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
    if(ret != READING_SUCCESS || strncmp(command, "y", 2) != 0) return NO;
    else return YES;
}

int getCurrentTime(){
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    if (currentTime == NULL) return TIME_ERR;
    return (currentTime->tm_hour*100 + currentTime->tm_min);
}

int sendRunSignal(float duration){
    int curtime = getCurrentTime();
    if(curtime == TIME_ERR) return TIME_ERR;

    pthread_mutex_lock(&config_mutex);
    config.run_until = timeArithmeticAdd(curtime, duration);
    pthread_mutex_unlock(&config_mutex);
    return READING_SUCCESS;
    
}
int timeArithmeticAdd(int time, float duration){
    int minutes = duration * 60;
    int hours = minutes/60;
    minutes = minutes % 60;
    return (time + hours*100 + minutes);
}

bool isIntTime(int time_val){
    if(time_val > MAX_TIME || time_val < 0) return false;
    int minutes = time_val % 100;
    if(minutes > 59 || minutes < 0) return false;
    return true;
}
