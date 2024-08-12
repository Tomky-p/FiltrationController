#include <stdio.h>
#include "utils.h"
#include "gpio_utils.h"
#include "cmd_proc.h"

void processRun(char *input, char *param_buffer_first, char * param_buffer_second){
    bool args_ok = true;
    pthread_mutex_lock(&config_mutex);
    if(config.mode == AUTO){
        fprintf(stderr, "Currently running in automatic mode, to use run command switch to manual mode.\n");
        args_ok = false;
    }
    if(config.filtration_running){
        fprintf(stderr, "Filtration is already running, wait for the cycle to finish or terminate it to run again.\n");
        args_ok = false;
    }
    pthread_mutex_unlock(&config_mutex);
 
    if((args_ok && !checkArgumentFloat(param_buffer_first)) || (atof(param_buffer_first) <= 0 && args_ok) || (atof(param_buffer_first) > MAX_DURATION && args_ok)){
        args_ok = false;
        fprintf(stderr, "Invalid argument, provide a decimal number as duration in hours. Between 0 and 18 hours.\n");
    }
    if(args_ok && strncmp(param_buffer_second, "" , MAX_LENGHT) != 0){
        args_ok = false;
        fprintf(stderr, "Invalid arguments, provide only one argument.\n");
    }
    if(args_ok){
        float duration = atof(param_buffer_first);
        printf("Filtration will run for %0.f minutes\nAre you sure you want proceed?\n[y/n]", duration*60);
        int ret = recieveConfirmation(input);
        if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
        if(ret == YES){
            printf("Proceeding...\nLaunching filtration for %0.f minutes.\n", duration*60);
            ret = sendRunSignal(duration);
            if(ret == TIME_ERR) return TIME_ERR;
        }
        else{
            printf("Aborted.\n");
        }   
    }
}

void processStop(char *input, char *param_buffer_first, char * param_buffer_second){
    bool args_ok = true;
    if(strncmp(param_buffer_first, "", MAX_LENGHT) != 0 || strncmp(param_buffer_second, "" , MAX_LENGHT) != 0){
        fprintf(stderr, "This command does not take any parameters.\n");
        args_ok = false;
    }
    pthread_mutex_lock(&config_mutex);
    if(!config.filtration_running){
        fprintf(stderr, "The filtration is currently not running.\n");
        args_ok = false;
    }
    pthread_mutex_unlock(&config_mutex);
    if(args_ok){
        printf("The filtration currently running.\nAre you sure you want stop the current filtration cycle?\n[y/n]");
        int ret = recieveConfirmation(input);
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

void processKill(char *input, char *param_buffer_first, char * param_buffer_second){
    bool args_ok = true;
    if(strncmp(param_buffer_first, "", MAX_LENGHT) != 0 || strncmp(param_buffer_second, "" , MAX_LENGHT) != 0){
        args_ok = false;
    }
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

void processConfig(char *input, char *param_buffer_first, char * param_buffer_second){
    bool args_ok = true;
    if(strncmp(param_buffer_first, "", MAX_LENGHT) == 0 && strncmp(param_buffer_second, "" , MAX_LENGHT) == 0){
        args_ok = false;
        pthread_mutex_lock(&config_mutex);
        char *mode = config.mode == AUTO? "automatic" : "manual";
        printf("Current configuration:\nMode: %s\nDuration: %0.f minutes\nTime: %d:%d\n", mode, config.duration*60, config.time/100, config.time-((config.time/100)*100));
        pthread_mutex_unlock(&config_mutex);
    }
    pthread_mutex_lock(&config_mutex);
    if(args_ok && config.filtration_running){
        fprintf(stderr, "Filtration is currently running. Cannot change config now.\n");
        args_ok = false;
    }
    pthread_mutex_unlock(&config_mutex);
    if(!checkArgumentFloat(param_buffer_first) || !checkArgument(param_buffer_second)) args_ok = false;

    if((args_ok && atof(param_buffer_first) <= 0) || (args_ok && atoi(param_buffer_second) < 0)
     || (args_ok && atof(param_buffer_first) > MAX_DURATION) || (args_ok && atoi(param_buffer_second) > MAX_TIME)){
        fprintf(stderr, "Provided parameters are invalid! USAGE: config [mode](-a/-m) [duration](float between 0 and 18) [time](int where 11:30 = 1130 for example)\n");
        args_ok = false;
    } 
    if(args_ok && !isIntTime(atoi(param_buffer_second))){
        fprintf(stderr, "Provided time is invalid. Provide a int corresponding to a time of day in format: 1135 (= 11:35)\n");
        args_ok = false;
    }
    if(args_ok){
        float new_duration = atof(param_buffer_first);
        uint16_t new_time = atoi(param_buffer_second);
        printf("Set new configuration? Duration: %0.f minutes\nTime: %d:%d\nAre you sure you want proceed?\n[y/n]", new_duration*60, new_time/100, new_time-((new_time/100)*100));
        int ret = recieveConfirmation(input);
        if(ret == ALLOCATION_ERR) return ALLOCATION_ERR;
        if(ret == YES){
            pthread_mutex_lock(&config_mutex);
            config.duration = new_duration;
            config.time = new_time;
            printf("Configuration set to:\nDuration: %0.f minutes\nTime: %d:%d\n", config.duration*60, config.time/100, config.time-((config.time/100)*100));
            pthread_mutex_unlock(&config_mutex);
        }
        else{
            printf("Aborted.\n");
        }
    }
}

void processMode(char *param_buffer_first, char * param_buffer_second){
    pthread_mutex_lock(&config_mutex);
    if(config.filtration_running){
        fprintf(stderr, "Cannot switch the operating mode while the filtration is running.\n");
    }
    else if (strncmp(param_buffer_second, "", MAX_LENGHT) != 0){
        fprintf(stderr, "Too many parameters, USAGE: -m for manual or -a for automatic mode.\n");
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
    else if (strncmp(param_buffer_first, "", MAX_LENGHT) == 0){
        fprintf(stderr, "Too few parameters, USAGE: -m for manual or -a for automatic mode.\n");        
    }
    else{
        fprintf(stderr, "Unrecognized parameter, USAGE: -m for manual or -a for automatic mode.\n");
    }
    pthread_mutex_unlock(&config_mutex);
}


void processTime(char *param_buffer_first, char * param_buffer_second){

}

void processHelp(char *param_buffer_first, char * param_buffer_second){

}