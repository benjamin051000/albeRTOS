/*
 * IPC.h
 */
#pragma once

#include <albertOS.h>

namespace albertOS {

template<typename T, int MAXSIZE>
class FIFO {
    T buffer[MAXSIZE]; // Data buffer // TODO replace with std::array for nicer api
    T *head, *tail;
    unsigned lostData; // Counts amount of lost data
    Semaphore currentSize, mutex;
public:
    // Constructor
    FIFO();

    T read();
    bool write(T data);

    int32_t size() const {return currentSize;}
    bool full() const {return currentSize == MAXSIZE;}

};


template<typename T, int MAXSIZE>
FIFO<T, MAXSIZE>::FIFO() {
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
template<typename T, int MAXSIZE>
T FIFO<T, MAXSIZE>::read() {
    T data;

    albertOS::waitSemaphore(currentSize); // block if empty
    albertOS::waitSemaphore(mutex); //in case something else is reading
    data = *head;

    if(head == &buffer[MAXSIZE-1]) {
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
template<typename T, int MAXSIZE>
bool FIFO<T, MAXSIZE>::write(T data) {
    albertOS::waitSemaphore(mutex);

    if(currentSize == MAXSIZE) { // Out of room
        lostData++;
        albertOS::signalSemaphore(mutex);
        return false;
    }
    else {
        *tail = data; //write data // TODO verify this ref drop-in works since there's a weird deref

        if(tail == &buffer[MAXSIZE-1]) {

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
