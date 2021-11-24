/*
 * G8RTOS_IPC.c
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */
#include <G8RTOS.h>

/*********************************************** Defines ******************************************************************************/

#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

typedef struct FIFO {
    int32_t buffer[FIFOSIZE]; //where data is held
    int32_t* head;
    int32_t* tail;
    int32_t lostData;
    Semaphore currentSize;
    Semaphore mutex;
} FIFO_t;

/* Array of FIFOS */
static FIFO_t FIFOs[4];

/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    if(FIFOIndex > 3)
    {
        return -1;
    }

    FIFOs[FIFOIndex].head = &FIFOs[FIFOIndex].buffer[0];
    FIFOs[FIFOIndex].tail = &FIFOs[FIFOIndex].buffer[0];
    FIFOs[FIFOIndex].lostData = 0;
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].currentSize, 0);
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].mutex, 1);

    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice)
{
    int32_t data = 0;
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex); //in case something else is reading
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].currentSize); //block if its empty
    data = *FIFOs[FIFOChoice].head;

    if(FIFOs[FIFOChoice].head == &FIFOs[FIFOChoice].buffer[FIFOSIZE-1])
    {
        FIFOs[FIFOChoice].head = &FIFOs[FIFOChoice].buffer[0]; //overflow if necessary
    }
    else
    {
        FIFOs[FIFOChoice].head += 1; //increment head pointer
    }
    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
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
int writeFIFO(uint32_t FIFOChoice, int32_t Data)
{
    //G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].mutex);

    if(FIFOs[FIFOChoice].currentSize == FIFOSIZE) //out of room
    {
        FIFOs[FIFOChoice].lostData += 1;
        G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
        return -1;
    }
    else
    {
        *FIFOs[FIFOChoice].tail = Data; //write data

        if(FIFOs[FIFOChoice].tail == &FIFOs[FIFOChoice].buffer[FIFOSIZE-1]) //wrap?
        {
            FIFOs[FIFOChoice].tail = &FIFOs[FIFOChoice].buffer[0];
        }
        else
        {
            FIFOs[FIFOChoice].tail += 1;
        }
    }

    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].currentSize);
   // G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].mutex);
    return 0;
}

