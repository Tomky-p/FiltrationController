#include "utils.h"
#include "gpio_utils.h"
#include <time.h>
#include <wiringPi.h>
#include <stdio.h>

int initGpioPinControl(){
    //int ret = wiringPiSetupGpio();
    //pinMode(DEVICE_PIN_NUMBER, OUTPUT);
    //return ret;
    return EXIT_SUCCESS;
}

bool checkDeviceState(){
    //if(digitalRead(DEVICE_PIN_NUMBER) == HIGH) return true;
    //else return false;
    if(config.filtration_running) return true;
    else return false;
}

void runFilration(float duration){
    double in_seconds = ((duration*60)*60);
    launchFiltration();
    pthread_mutex_lock(&config_mutex);
    while (config.filtration_running && in_seconds > 0)
    {
        pthread_mutex_unlock(&config_mutex);
        delay(1000);
        in_seconds = in_seconds - 1;
        pthread_mutex_lock(&config_mutex);
    }
    pthread_mutex_unlock(&config_mutex);
    if(checkDeviceState()){
        shutdownFiltration();
    }
}

void shutdownFiltration(){
    pthread_mutex_lock(&config_mutex);
    //digitalWrite(DEVICE_PIN_NUMBER, LOW);
    printf("Turned off.\n");
    config.filtration_running = false;
    pthread_mutex_unlock(&config_mutex);
}

void launchFiltration(){
    pthread_mutex_lock(&config_mutex);
    //digitalWrite(DEVICE_PIN_NUMBER, HIGH);
    printf("Turned on.\n");
    config.filtration_running = true;
    pthread_mutex_unlock(&config_mutex);
}
