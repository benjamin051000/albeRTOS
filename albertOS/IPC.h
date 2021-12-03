/*
 * IPC.h
 */
#pragma once

#include <albertOS.h>

namespace albertOS {

template<typename T>
class FIFO {
    const int FIFOSIZE = 16;

    T buffer[16]; // Data buffer // TODO won't let me use FIFOSIZE here even though it's const...
    T *head, *tail;
    unsigned lostData; // Counts amount of lost data
    Semaphore currentSize, mutex;
public:
    // Constructor
    FIFO();

    T read();
    bool write(T data);
};


template<typename T>
FIFO<T>::FIFO() {
    head = &buffer[0];
    tail = &buffer[0];
    lostData = 0;
    albertOS::initSemaphore(currentSize, 0);
    albertOS::initSemaphore(mutex, 1);
}


/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
template<typename T>
T FIFO<T>::read() {
    T data;

    albertOS::waitSemaphore(currentSize); // block if empty
    albertOS::waitSemaphore(mutex); //in case something else is reading
    data = *head;

    if(head == &buffer[FIFOSIZE-1]) {
        head = &buffer[0]; //overflow if necessary
    }
    else {
        head++;
    }

    albertOS::signalSemaphore(mutex);
    return data;
}

/*
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if necessary)
 *  Param "FIFOIndex": chooses which buffer to read from
 *        "Data": Data being put into FIFO
 *  Returns: false for full buffer (unable to write), true if no errors
 */
template<typename T>
bool FIFO<T>::write(T data) {
    albertOS::waitSemaphore(mutex);

    if(currentSize == FIFOSIZE) { // Out of room
        lostData++;
        albertOS::signalSemaphore(mutex);
        return false;
    }
    else {
        *tail = data; //write data // TODO verify this ref drop-in works since there's a weird deref

        if(tail == &buffer[FIFOSIZE-1]) {

            tail = &buffer[0];
        }
        else {
            tail++;
        }
    }

    albertOS::signalSemaphore(currentSize); // Increase the size! Also unblocks threads that were waiting for an element.

    albertOS::signalSemaphore(mutex);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Old FIFO stuff
////////////////////////////////////////////////////////////////////////////////
/*
 * Initializes One to One FIFO Struct
 */
int initFIFO(uint32_t FIFOIndex);

int32_t readFIFO(unsigned FIFOIndex);
int writeFIFO(unsigned FIFOIndex, int32_t data);

} // end of namespace albertOS
