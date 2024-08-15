#pragma once

#include <cstdint>
#include <semaphores.h>

namespace albeRTOS {

/**
 * A FIFO.
 * Underlying implementation is a circular buffer of size MAXSIZE.
 */
template<typename Type, int32_t MAXSIZE> // TODO what type should MAXSIZE be?
class fifo {
    Type buffer[MAXSIZE]; // Data buffer // TODO replace with std::array for nicer api
    Type *head, *tail;
    int32_t lostData; // Counts amount of lost data // TODO how large should this type be?
	Semaphore currentSize, mutex;
public:
    // Constructor
	fifo() : head(&buffer[0]), tail(&buffer[0]), lostData(0) {
		initSemaphore(currentSize, 0);
		initSemaphore(mutex, 1);
	}

	/*
	 * Reads fifo
	 *  - Waits until CurrentSize semaphore is greater than zero
	 *  - Gets data and increments the head ptr (wraps if necessary)
	 * Param: "FIFOChoice": chooses which buffer we want to read from
	 * Returns: uint32_t Data from fifo
	 */
	Type read() {
		waitSemaphore(currentSize); // block if empty
		waitSemaphore(mutex); //in case something else is reading
		const Type data = *head;

		if(head == &buffer[MAXSIZE-1]) {
			head = &buffer[0]; //overflow if necessary
		}
		else {
			head++;
		}

		signalSemaphore(mutex);
		return data;
	}

	/*
	 *  Writes data to Tail of the buffer if the buffer is not full
	 *  Increments tail (wraps if necessary)
	 *  Param "FIFOIndex": chooses which buffer to read from
	 *        "Data": Data being put into fifo
	 *  Returns: false for full buffer (unable to write), true if no errors
	 */
	bool write(Type data) {
		waitSemaphore(mutex);

		if(currentSize == MAXSIZE) { // Out of room
			lostData++;
			signalSemaphore(mutex);
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

		signalSemaphore(currentSize); // Increase the size! Also unblocks threads that were waiting for an element.

		signalSemaphore(mutex);
		return true;
	}

    [[nodiscard]] int32_t size() const {return currentSize;}
    [[nodiscard]] bool full() const {return currentSize == MAXSIZE;}
};

} // end of namespace albeRTOS
