#include <ipc.h>

// Private namespace
namespace {
const unsigned FIFOSIZE = 16;
const unsigned MAX_NUMBER_OF_FIFOS = 4;


struct FIFO_old {
    int32_t buffer[FIFOSIZE]; // Data buffer
    int32_t *head, *tail;
    int32_t lostData; // Counts amount of lost data
	albeRTOS::Semaphore currentSize, mutex;
};

/* Array of FIFOS */
FIFO_old FIFOs[MAX_NUMBER_OF_FIFOS];

} // end of private namespace


/*
 * Initializes FIFO Struct
 */
int albeRTOS::initFIFO(unsigned FIFOIndex)
{
    if(FIFOIndex >= MAX_NUMBER_OF_FIFOS) {
        return -1; // TODO return error codes
    }

    FIFO_old& newFIFO = FIFOs[FIFOIndex];

    newFIFO.head = &newFIFO.buffer[0];
    newFIFO.tail = &newFIFO.buffer[0];
    newFIFO.lostData = 0;
    albeRTOS::initSemaphore(newFIFO.currentSize, 0);
    albeRTOS::initSemaphore(newFIFO.mutex, 1);

    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t albeRTOS::readFIFO(unsigned FIFOIndex) {
    // Obtain ref to FIFO struct
    FIFO_old& fifo = FIFOs[FIFOIndex];

    int32_t data = 0;
    albeRTOS::waitSemaphore(fifo.mutex); //in case something else is reading
    albeRTOS::waitSemaphore(fifo.currentSize); //block if its empty
    data = *fifo.head;

    if(fifo.head == &fifo.buffer[FIFOSIZE-1]) {
        fifo.head = &fifo.buffer[0]; //overflow if necessary
    }
    else {
        fifo.head++;
    }

    albeRTOS::signalSemaphore(fifo.mutex);
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
int albeRTOS::writeFIFO(unsigned FIFOIndex, int32_t data) {
    //G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);

    // Obtain ref to FIFO struct
    FIFO_old& fifo = FIFOs[FIFOIndex];

    if(fifo.currentSize == FIFOSIZE) { // Out of room
        fifo.lostData++;
        albeRTOS::signalSemaphore(fifo.mutex);
        return -1;
    }
    else {
        *fifo.tail = data; //write data // TODO verify this ref drop-in works since there's a weird deref

        if(fifo.tail == &fifo.buffer[FIFOSIZE-1]) {

            fifo.tail = &fifo.buffer[0];
        }
        else {
            fifo.tail++;
        }
    }

    albeRTOS::signalSemaphore(fifo.currentSize);
   // G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
    return 0;
}

