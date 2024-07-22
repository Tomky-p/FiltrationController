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
- run [amount] -(confirm)
    run in watering cycle in manual mode
    parameters:
    (int) amount in liters to be dispensed
- config [amount] [interval] -(confirm)
    change the config for automatic mode
    parameters:
    (int) new amount per cycle in liters
    (int) new interval per cycle in minutes
- weather (date)
    print the relevant weather data
    parameters:
    (date) date for which you want the weather data [optional]
    without parameter prints weather for the last 5 days and upcoming 5 days
- stop -(confirm)
    stops the intire programs

*/
void processCommand(char *input, config_t *config, bool *confirmation){
    char *cmd_buffer = (char*)malloc(MAX_LENGHT * sizeof(char));
    char *param_buffer_first = (char*)malloc(MAX_LENGHT * sizeof(char));
    char *param_buffer_second = (char*)malloc(MAX_LENGHT * sizeof(char));
    int i = 0;
    int current_buffer = 1;
    while (input[i] != '\0')
    {
        switch (current_buffer)
        {
        case 1:
            while (input[i] != ' ')
            {        
                cmd_buffer[i] = input[i];
                i++;
                if(i >= MAX_LENGHT){
                    fprintf(stderr, "Input parameter too long and not recongized.");
                    break;
                }
            }
            current_buffer++;
            break;
        case 2:
            int p = 0;
            while (input[i] != ' ')
            {        
                param_buffer_first[p] = input[i];
                i++;
                p++;
                if(p >= MAX_LENGHT){
                    fprintf(stderr, "Input parameter too long and not recongized.");
                    break;
                }
            }
            current_buffer++;
            break;
        case 3:
            int p = 0;
            while (input[i] != '\0' && input[i] != ' ')
            {        
                param_buffer_second[p] = input[i];
                i++;
                p++;
                if(p >= MAX_LENGHT){
                    fprintf(stderr, "Input parameter too long and not recongized.");
                    break;
                }
            }
            break;   
        default:
            fprintf(stderr, "Failed to parse command!");
            break;
        }
        if(i >= 50){
            fprintf(stderr, "Input command too long and not recongized.");
            break;
        }
    }
    if(strncmp(cmd_buffer, "mode", 5) == 0){
        
    }
    else if (strncmp(cmd_buffer, "run", 4) == 0)
    {
        /* code */
    }
    else if (strncmp(cmd_buffer, "config", 7) == 0)
    {
        /* code */
    }
    else if (strncmp(cmd_buffer, "stop", 5) == 0)
    {
        /* code */
    }
    else if (strncmp(cmd_buffer, "weather", 8) == 0){

    }
    else{
        fprintf(stderr, "Command not recognized.");
    }
    free(cmd_buffer);
    free(param_buffer_first);
    free(param_buffer_second);
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
    int i = 0;
    while ((arg[i] != '\0' && arg[i] != '\n') && i < MAX_DIGITS)
    {
        if(arg[i] < '0' || arg[i] > '9') return false; 
        i++;
    }
    if(arg[i] != '\0' && arg[i] != '\n'){
        return false;
    } 
    return true;
}
