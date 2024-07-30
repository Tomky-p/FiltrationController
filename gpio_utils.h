#include <wiringPi.h>

#define RELAY_1_PIN_NUMBER 22
#define RELAY_2_PIN_NUMBER 5

#define GPIO_ERR 101

#ifndef DEVICE_PIN_NUMBER
#define DEVICE_PIN_NUMBER 22
#endif

int initGpioPinControl();

void runFilration(float duration);

void shutdownFiltration();