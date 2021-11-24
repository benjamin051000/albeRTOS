/*
 * main.cpp
 *
 *  Created on: Nov 22, 2021
 *      Author: benja
 */
#include <G8RTOS.h>
#include <G8RTOS_Scheduler.h>

void taskA() {
while(true) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
//    albertOS::sleep(1000); // let other tasks run
    sleep(500);
}
}

void taskB() {
while(true) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
//    albertOS::sleep(2000); // let other tasks run
    sleep(1000);
}
}

void idleTask() {
    // empty task to ensure RTOS always has something to do.
    while(true) {
//        albertOS::sleep(1);
    }
}

void per_event() {
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/**
 * A simple example of albertOS.
 */
int main() {

//    albertOS::init();
    G8RTOS_Init();

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    G8RTOS_AddThread(taskA, 1, "Task A");
    G8RTOS_AddThread(taskB, 3, "Task B");
    G8RTOS_AddThread(idleTask, 4, "Idle Thread");

    G8RTOS_AddPeriodicEvent(per_event, 500);

    G8RTOS_Launch();

//    if(albertOS::launch())
//        while(true); // Something went wrong. Wait here.

    return 0;
}

