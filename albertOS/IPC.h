/*
 * G8RTOS_IPC.h
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */
#pragma once

#include <albertOS.h>

namespace albertOS {

// Private namespace
namespace {

const int FIFOSIZE = 16;
const int MAX_NUMBER_OF_FIFOS = 4;

} // end of private namespace

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

struct FIFO {
    int32_t buffer[FIFOSIZE]; //where data is held
    int32_t* head;
    int32_t* tail;
    int32_t lostData;
    Semaphore currentSize;
    Semaphore mutex;
};

/*
 * Initializes One to One FIFO Struct
 */
int initFIFO(uint32_t FIFOIndex);

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFO);

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFO, int32_t data);

} // end of namespace albertOS
