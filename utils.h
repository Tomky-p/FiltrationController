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

//ERR messages
#define ALLOC_ERR_MSG "FATAL ERR! Memory allocation failed.\n"
#define TIME_ERR_MSG "FATAL ERR! Failed to get current time.\n"
#define GPIO_ERR_MSG "FATAL ERR! Failed to initialize access to GPIO periferies.\n"

//USER ERR messages
#define INVALID_TIME_INPUT "Provided time is invalid. Provide a number corresponding to a time of day in format: 1135 = 11:35\n"
#define SYSTEM_RUNNING_MSG "Cannot execute command the filtration is currently running.\n"
#define SYSTEM_NOT_RUNNING_MSG "Cannot execute command the filtration is currently not running.\n"
#define INVALID_ARGS_MSG "Provided parameters are invalid! PROVIDE: [duration] (in hours) [time of day]\n"
#define INVALID_DURATION_MSG "Provided duration is invalid, provide a duration between 0.1 and %d.\n"
#define NOT_FLOAT_MSG "Provided duration is not a float.\n"
#define NOT_INT_MSG "Provided time is not an integer\n"
#define TOO_FEW_ARGS_MSG "Too few arguments provided.\n";

//INFO messages
#define HELP_MESSAGE "LIST OF ALL COMMANDS\n- mode [mode]\n  changes mode of the system\n    parameters:\n    (mode) -a automatic operation -m manual\n   - without parameters: show the current mode\n- run [duration] -(confirm)\n    run the filtration in manual mode\n    parameters:\n    (float) duration in hours.\n- config [duration] [time] -(confirm)\n    change the configuration for automatic mode\n    parameters:\n    (float) new duration in hours\n    (int) new time at which the filtration cycle begins\n    - without paramater: shows current configuration\n- stop -(confirm)\n    stops the filtration\n- kill -(confirm)\n    stops and quits the entire program\n- state\n    shows whether the filtration is running or not\n"

                        
#define AUTO 1
#define MANUAL 0

#define YES 1
#define NO 0

typedef struct{
    uint8_t mode;
    float duration;
    uint16_t time;
    bool running;
    float manual_duration;
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
//int sendRunSignal(float duration);

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
int recieveConfirmation();

//Check whether an string argument is a float
bool checkArgumentFloat(char *arg);

//prints current configuration
void printConfig();

//invalides the arg flag based the device state and desired state
void verifyState(bool *args_ok, bool desiredOn);

//checks if the desired amount of arguments was provided and invalidates the flag if not
void countArguments(int desired_count, bool *args_ok, char *first_param, char *second_param);

//verifies if the arguments are valid and invalidates the flag if they are not
void verifyArguments(bool *args_ok, char *first_arg, char *second_arg, int desired_count);
