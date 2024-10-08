#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#define STARTING_CAPACITY 4
#define MAX_LENGHT 25            
#define ALLOCATION_ERR -1
#define TIME_ERR -2
#define READING_SUCCESS 0
#define MAX_DIGITS 5
#define LENGHT_ERR 1

#ifndef MAX_DURATION
//The maximum duration of a filtration cycle
#define MAX_DURATION 18
#endif

//The maxium time to be inputted from 0:00 to 23:59
#define MAX_TIME 2359
                        
#define AUTO 1
#define MANUAL 0

#define YES 1
#define NO 0

typedef struct{
    uint8_t mode;
    float duration;
    uint16_t time;
    bool running;
    uint16_t run_until;
    bool filtration_running;
}config_t;

extern config_t config;
extern pthread_mutex_t config_mutex;

//Reads the user input commands from stdin
int readCmd(char **command);

//Check whether an string argument is an interger
bool checkArgument(char *arg);

//Proccess the commands from user, updates the config for irrigation thread and runs irrigation in manual mode
int processCommand(char *input);

//returns current clock time as an int in the 23:59 = 2359 format
int getCurrentTime();

//adds an int time value together with float duration value and returns a correct int time
int timeArithmeticAdd(int time, float duration);

//signals to the automatic thread to run and when to shutdown
int sendRunSignal(float duration);

//checks if a provided int is a valid time value
bool isIntTime(int time_val);
/*
* Separate input command into separate command and parameter buffers for processing of command
* RETURNS:
*   - true, when the command has proper format and lenght
*   - false, otherwise
*/
bool splitToBuffers(char *input, char *cmd_buffer, char *param_buffer_first, char *param_buffer_second);

//Get the confirmation for a command from user
int recieveConfirmation(char *command);

//Check whether an string argument is a float
bool checkArgumentFloat(char *arg);
