/*
 * main.cpp
 *
 *  Created on: Nov 22, 2021
 *      Author: benja
 */
#include <albertOS.h>


void taskA() {
while(true) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    albertOS::sleep(500);
}
}

void taskB() {
while(true) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    albertOS::sleep(1000);
}
}

void idleTask() {
    // empty task to ensure RTOS always has something to do.
    while(true);
}

void per_event() {
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/**
 * A simple example of albertOS.
 */
int main() {

    albertOS::init();

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    albertOS::addThread(taskA, 1, (char*)"Task A");
    albertOS::addThread(taskB, 3, (char*)"Task B");
    albertOS::addThread(idleTask, 4, (char*)"Idle Thread");

    albertOS::addPeriodicEvent(per_event, 500);

    albertOS::launch();

    return 0;
}

