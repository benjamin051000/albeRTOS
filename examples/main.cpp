#include <albeRTOS/scheduler.h>

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
void main() {

    albertOS::init();

    // Setup GPIO
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    // Add threads
    albertOS::addThread(taskA, 1, (char*)"Task A");
    albertOS::addThread(taskB, 3, (char*)"Task B");
    albertOS::addThread(idleTask, 4, (char*)"Idle Thread");

    // Add events
    albertOS::addPeriodicEvent(per_event, 100);

    // Start the kernel
    albertOS::launch();

    while(true); // If we end up here, something went wrong with the launch().
}

