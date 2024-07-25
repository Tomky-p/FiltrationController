#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "utils.h"


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
    stops the intire programs

*/
config_t config = {0};
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

int processCommand(char *input){
    char *cmd_buffer = (char*)malloc(MAX_LENGHT * sizeof(char));
    char *param_buffer_first = (char*)malloc(MAX_LENGHT * sizeof(char));
    char *param_buffer_second = (char*)malloc(MAX_LENGHT * sizeof(char));
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

    if(strncmp(cmd_buffer, "mode", 5) == 0){
        pthread_mutex_lock(&config_mutex);
        if(strncmp(param_buffer_first, "-a", 3) == 0 && config.mode != AUTO){
            //switching mode something function           
        }
        else if (strncmp(param_buffer_first, "-a", 3) == 0 && config.mode == AUTO){
            printf("Already running in automatic mode.\n");
        }    
        else if (strncmp(param_buffer_first, "-m", 3) == 0 && config.mode != MANUAL){
            //switching mode something function   
        }
        else if (strncmp(param_buffer_first, "-m", 3) == 0 && config.mode == MANUAL){
            printf("Already running in manual mode.\n");
        }
        else{
            fprintf(stderr, "Unrecognized parameter, USAGE: -m for manual or -a for automatic mode.\n");
        }
        pthread_mutex_unlock(&config_mutex);
    }
    else if (strncmp(cmd_buffer, "run", 4) == 0)
    {   
        bool args_ok = true;
        pthread_mutex_lock(&config_mutex);
        if(config.mode == AUTO){
            fprintf(stderr, "Currently running in automatic mode, to use run command switch to manual mode.\n");
            args_ok = false;
        }
        pthread_mutex_unlock(&config_mutex);
 
        if(args_ok && !checkArgumentFloat(param_buffer_first)) args_ok = false;

        if(args_ok){
            float duration;
            duration = atof(param_buffer_first);
            
            int ret = recieveConfirmation(input);
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                //runFiltration(duration);
            }   
        }    
    }
    else if (strncmp(cmd_buffer, "config", 7) == 0)
    {
        bool args_ok = true;
        //TO DO: check args
        
        if(args_ok){
            float new_duration = atof(param_buffer_first);
            uint16_t new_time = atoi(param_buffer_second);
            printf("Filtration will run for %f minutes");
            int ret = recieveConfirmation(input);
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                pthread_mutex_lock(&config_mutex);
                config.duration = new_duration;
                config.time = new_time;
                pthread_mutex_unlock(&config_mutex);
            }
        } 
    }
    else if (strncmp(cmd_buffer, "stop", 5) == 0)
    {
        pthread_mutex_lock(&config_mutex);
        pthread_mutex_unlock(&config_mutex); 
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

    *command = (char*)malloc(capacity);
    if (*command == NULL) return ALLOCATION_ERR;

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
    while ((arg[index] != '\0' && arg[index] != '\n') && index < MAX_DIGITS)
    {
        if((arg[index] < '0' || arg[index] > '9') && arg[index] != '.') return false; 
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
            if(input[index] != '\0'){
                current_buffer++;
                index++;
            }
            cmd_buffer[index] = '\0';
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
            if(input[index] != '\0'){
                current_buffer++;
                index++;
            }
            param_buffer_first[buffer_index] = '\0';
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
