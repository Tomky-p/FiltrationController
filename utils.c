#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "utils.h"
#include "gpio_utils.h"

//initialize global (thread shared) variables 
config_t config = {0};
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    bool args_ok = true;
    if(strncmp(cmd_buffer, "mode", 5) == 0)
    {
        pthread_mutex_lock(&config_mutex);
        if(config.filtration_running){
            fprintf(stderr, "Cannot switch the operating mode while the filtration is running.\n");
        }
        else if (strncmp(param_buffer_second, "", MAX_LENGHT) != 0){
            fprintf(stderr, "Too many parameters, USAGE: -m for manual or -a for automatic mode.\n");
        }
        else if (strncmp(param_buffer_first, "", MAX_LENGHT) == 0){
            const char *response = config.mode == AUTO ? "Currently running in automatic mode.\n" : "Currently running in manual mode.\n";
            printf("%s",response);        
        }     
        else if (strncmp(param_buffer_first, "-a", 3) == 0 && config.mode == AUTO){
            printf("Already running in automatic mode.\n");
        }
        else if (strncmp(param_buffer_first, "-a", 3) == 0 && config.mode != AUTO){
            printf("Switching to automatic mode...\n");
            config.mode = AUTO;           
        }    
        else if (strncmp(param_buffer_first, "-m", 3) == 0 && config.mode != MANUAL){
            printf("Switching to manual mode...\n");
            config.mode = MANUAL;
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
        pthread_mutex_lock(&config_mutex);
        if(config.mode == AUTO){
            fprintf(stderr, "Currently running in automatic mode, to use run command switch to manual mode.\n");
            args_ok = false;
        }
        pthread_mutex_unlock(&config_mutex);
        verifyState(&args_ok, false);
        verifyArguments(&args_ok, param_buffer_first, param_buffer_second, 1);

        if(args_ok){
            float duration = atof(param_buffer_first);
            printf("Filtration will run for %0.f minutes\nAre you sure you want proceed?\n[y/n]", duration*60);
            int ret = recieveConfirmation();
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                printf("Proceeding...\nLaunching filtration for %0.f minutes.\n", duration*60);
                pthread_mutex_lock(&config_mutex);
                config.manual_duration = duration;
                pthread_mutex_unlock(&config_mutex);
            }
            else{
                printf("Aborted.\n");
            }   
        }   
    }
    else if (strncmp(cmd_buffer, "config", 7) == 0)
    {
        if(strncmp(param_buffer_first, "", MAX_LENGHT) == 0 && strncmp(param_buffer_second, "" , MAX_LENGHT) == 0){
            args_ok = false;
            printConfig();
        }
        verifyState(&args_ok, false);
        countArguments(2, &args_ok, param_buffer_first, param_buffer_second);
        verifyArguments(&args_ok, param_buffer_first, param_buffer_second, 2);
        
        if(args_ok){
            float new_duration = atof(param_buffer_first);
            uint16_t new_time = atoi(param_buffer_second);
            char *conf_string = (new_time%100 < 10) ? "Set new configuration?\nDuration: %0.f minutes\nTime: %d:0%d\nAre you sure you want proceed?\n[y/n]" : "Set new configuration?\nDuration: %0.f minutes\nTime: %d:%d\nAre you sure you want proceed?\n[y/n]";
            printf(conf_string, new_duration*60, new_time/100, new_time%100);
            int ret = recieveConfirmation();
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                pthread_mutex_lock(&config_mutex);
                config.duration = new_duration;
                config.time = new_time;
                pthread_mutex_unlock(&config_mutex);
                printConfig();
            }
            else{
                printf("Aborted.\n");
            }
        }
    }
    //TO DO implement the help command
    else if (strncmp(cmd_buffer, "kill", 5) == 0)
    {
        countArguments(0, &args_ok, param_buffer_first, param_buffer_second);
        if(args_ok){
            printf("WARNING! The filtration controler system will be terminated.\nAre you sure you want proceed?\n[y/n]");
            int ret = recieveConfirmation(input);
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                pthread_mutex_lock(&config_mutex);
                printf("Quitting program...\n");
                config.running = false;
                pthread_mutex_unlock(&config_mutex);
                if(checkDeviceState()){
                    shutdownFiltration();
                }
            }   
            else{
                printf("Aborted.\n");
            }
        }
    }
    else if (strncmp(cmd_buffer, "stop", 5) == 0)
    {
        countArguments(0, &args_ok, param_buffer_first, param_buffer_second);
        verifyState(&args_ok, true);
        if(args_ok){
            printf("The filtration currently running.\nAre you sure you want stop the current filtration cycle?\n[y/n]");
            int ret = recieveConfirmation();
            if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
            if(ret == YES){
                printf("Stopping filtration...\n");
                pthread_mutex_lock(&config_mutex);
                config.filtration_running = false;
                pthread_mutex_unlock(&config_mutex);
            }
            else{
                printf("Aborted.\n");
            }
        }   
    }
    else if (strncmp(cmd_buffer, "help", 5) == 0)
    {
        printf("%s", HELP_MESSAGE);   
    }
    else if (strncmp(cmd_buffer, "state", 6) == 0)
    {
        countArguments(0, &args_ok, param_buffer_first, param_buffer_second);
         if(args_ok){
            char *state_string = checkDeviceState() ? "The filtration is currently running.\n" : "The filtration is not running.\n";
            printf("%s", state_string);
        }  
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

int recieveConfirmation(){
    char *buffer = (char*)malloc(STARTING_CAPACITY);
    if(buffer == NULL) return ALLOCATION_ERR;
    int ret = readCmd(&buffer);
    if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
    if(ret != READING_SUCCESS || strncmp(buffer, "y", 2) != 0) {
        free(buffer);
        return NO;
    }
    else {
        free(buffer);
        return YES;
    }
}

int getCurrentTime(){
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    if (currentTime == NULL) return TIME_ERR;
    return (currentTime->tm_hour*100 + currentTime->tm_min);
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

void printConfig(){
    pthread_mutex_lock(&config_mutex);
    const char *mode_string = (config.mode == AUTO) ? "CONFIGURATION:\nMode: AUTOMATIC\n" : "CONFIGURATION:\nMode: MANUAL\n";
    const char *conf_string = (config.time % 100 < 10) ? "Time: %d:0%d\nDuration: %.0f minutes.\n": "Time: %d:%d\nDuration: %.0f minutes.\n";
    printf("%s", mode_string);
    printf(conf_string, config.time/100, config.time%100, config.duration*60);
    pthread_mutex_unlock(&config_mutex);
}

void verifyState(bool *args_ok, bool desiredOn){
    if((*args_ok) == false) return;
    if(checkDeviceState()){
        if(desiredOn) return;
        *args_ok = false;
        fprintf(stderr, SYSTEM_RUNNING_MSG);
    }
    else{
        if(!desiredOn) return;
        *args_ok = false;
        fprintf(stderr, SYSTEM_NOT_RUNNING_MSG); 
    } 
}

void countArguments(int desired_count, bool *args_ok, char *first_param, char *second_param){
    if((*args_ok) == false) return;
    switch (desired_count)
    {
    case 0:
        if(strncmp(first_param, "", MAX_LENGHT) != 0 || strncmp(second_param, "" , MAX_LENGHT) != 0){
            fprintf(stderr, "This command does not take any parameters.\n");
            *args_ok = false;
        }
        break;
    case 1:
        if(strncmp(second_param, "" , MAX_LENGHT) != 0){
            fprintf(stderr, "This command takes only one parameter\n");
            *args_ok = false;
        }
        if(strncmp(first_param, "", MAX_LENGHT) == 0){
            fprintf(stderr, "Too few parameters.\n");
            *args_ok = false;
        }
        break;
    case 2:
        if(strncmp(first_param, "", MAX_LENGHT) == 0 || strncmp(second_param, "" , MAX_LENGHT) == 0){
            fprintf(stderr, "Too few parameters.\n");
            *args_ok = false;
        }
        break;
    }
}

void verifyArguments(bool *args_ok, char *first_arg, char *second_arg, int desired_count){
    if((*args_ok) == false) return;
    switch (desired_count){
    case 1:
        if(!checkArgumentFloat(first_arg)){
            *args_ok = false;
            fprintf(stderr, NOT_FLOAT_MSG);
            break;
        }
        if(atof(first_arg) > MAX_DURATION || atof(first_arg) <= 0){
            *args_ok = false;
            fprintf(stderr, INVALID_DURATION_MSG, MAX_DURATION);
        }
        break;
    
    case 2:
        if(!checkArgumentFloat(first_arg) || !checkArgument(second_arg)){
            *args_ok = false;
            fprintf(stderr, INVALID_ARGS_MSG);
            break;    
        } 
        if(atof(first_arg) > MAX_DURATION || atof(first_arg) <= 0){
            *args_ok = false;
            fprintf(stderr, INVALID_DURATION_MSG, MAX_DURATION);
            break;
        }
        if(!isIntTime(atoi(second_arg))){
            *args_ok = false;
            fprintf(stderr, INVALID_TIME_INPUT);
        }
        break;
    }
}
