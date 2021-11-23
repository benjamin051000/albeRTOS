/*
 * main.cpp
 *
 *  Created on: Nov 22, 2021
 *      Author: benja
 */
#include <albertOS.h>

void taskA() {
while(true) {
    // Do something
}
}

void taskB() {
while(true) {
    // Do something else
}
}

void per_event() {
    // Do something once
}

/**
 * A simple example of albertOS.
 */
int main() {

    albertOS::init();

    albertOS::addThread(taskA, 1, "Task A");
    albertOS::addThread(taskB, 3, "Task B");

    albertOS::addPeriodicEvent(per_event, 5000);

    if(albertOS::launch())
        while(true); // Something went wrong. Wait here.

    return 0;
}

