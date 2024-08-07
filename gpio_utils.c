#include "utils.h"
#include "gpio_utils.h"
#include <time.h>
#include <wiringPi.h>
#include <stdio.h>

int initGpioPinControl(){
    int ret = wiringPiSetupGpio();
    pinMode(DEVICE_PIN_NUMBER, OUTPUT);
    return ret;
}

void runFilration(float duration){
    float in_miliseconds = ((duration*60)*60)*1000;
    launchFiltration();
    delay(in_miliseconds);
    shutdownFiltration();
}

void shutdownFiltration(){
    pthread_mutex_lock(&config_mutex);
    digitalWrite(DEVICE_PIN_NUMBER, LOW);
    printf("Turned off.\n");
    config.filtration_running = false;
    pthread_mutex_unlock(&config_mutex);
}

void launchFiltration(){
    pthread_mutex_lock(&config_mutex);
    digitalWrite(DEVICE_PIN_NUMBER, HIGH);
    printf("Turned on.\n");
    config.filtration_running = true;
    pthread_mutex_unlock(&config_mutex);
}