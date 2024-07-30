#include "utils.h"
#include "gpio_utils.h"
#include <wiringPi.h>
#include <stdio.h>

int initGpioPinControl(){
    int ret = wiringPiSetupGpio();
    pinMode(DEVICE_PIN_NUMBER, OUTPUT);
    return ret;
}

void runFilration(float duration){
    //TO DO: figure out how run all time delay functions in the automatic thread
}

void shutdownFiltration(){

}