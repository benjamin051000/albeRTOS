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
    albertOS::sleep(1000); // let other tasks run
}
}

void taskB() {
while(true) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    albertOS::sleep(2000); // let other tasks run
}
}

void idleTask() {
    // empty task to ensure RTOS always has something to do.
    while(true) {
        albertOS::sleep(1);
    }
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

    albertOS::addThread(taskA, 1, "Task A");
//    albertOS::addThread(taskB, 3, "Task B");
    albertOS::addThread(idleTask, 4, "Idle Thread");

//    albertOS::addPeriodicEvent(per_event, 5000);

    if(albertOS::launch())
        while(true); // Something went wrong. Wait here.

    return 0;
}

