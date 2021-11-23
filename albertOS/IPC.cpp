/*
 * IPC.cpp
 */
#include <albertOS.h>

namespace albertOS {

// private namespace
namespace {

/* Array of FIFOS */
FIFO FIFOs[4];

} // end of private namespace


/*
 * Initializes FIFO Struct
 */
int initFIFO(uint32_t FIFOIndex)
{
    if(FIFOIndex >= MAX_NUMBER_OF_FIFOS) {
        return -1; // TODO error codes
    }

    FIFO& newFIFO = FIFOs[FIFOIndex];

    newFIFO.head = &newFIFO.buffer[0];
    newFIFO.tail = &newFIFO.buffer[0];
    newFIFO.lostData = 0;

    albertOS::initSemaphore(newFIFO.currentSize, 0);
    albertOS::initSemaphore(newFIFO.mutex, 1);

    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice) {
    FIFO& fifo = FIFOs[FIFOChoice];

    // Ensure no other threads are currently interacting with this FIFO.
    albertOS::waitSemaphore(fifo.mutex);
    // If FIFO is empty, block until it has data.
    albertOS::waitSemaphore(fifo.currentSize);

    int32_t data = *(fifo.head);

    // Update FIFO head/tail pointers
    if(fifo.head == &fifo.buffer[FIFOSIZE-1]) {
        fifo.head = &fifo.buffer[0]; // Overflow if necessary
    }
    else {
        fifo.head++; // Advance head to next data
    }

    albertOS::signalSemaphore(fifo.mutex);

    return data;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, int32_t data)
{
    FIFO& fifo = FIFOs[FIFOChoice];

//    albertOS::waitSemaphore(&fifo.mutex);

    if(fifo.currentSize == FIFOSIZE) { // Out of room
        fifo.lostData++;
        albertOS::signalSemaphore(fifo.mutex); // TODO BUG don't signal if you didn't previously acquire, right?
        return -1;
    }
    else {
        *fifo.tail = data;

        // Update tail pointer
        if(fifo.tail == &fifo.buffer[FIFOSIZE-1]) {
            fifo.tail = &fifo.buffer[0];
        }
        else {
            fifo.tail++;
        }
    }

    // This will automatically increase the current size
    albertOS::signalSemaphore(fifo.currentSize);
   // G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
    return 0;
}

} // end of namespace albertOS
