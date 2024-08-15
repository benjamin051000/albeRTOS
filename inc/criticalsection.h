/*
 * G8RTOS_CriticalSection.h
 *
 *  Created on: Dec 11, 2016
 *      Author: Raz Aloni
 */
#pragma once

#include <albertOS.h>

#define START_CRIT_SECTION const int32_t criticalSectionStatus = StartCriticalSection();
#define END_CRIT_SECTION EndCriticalSection(criticalSectionStatus);

/*
 * Starts a critical section
 * 	- Saves the state of the current PRIMASK (I-bit)
 * 	- Disables interrupts
 * Returns: The current PRIMASK State
 */
extern "C" int32_t StartCriticalSection();

/*
 * Ends a critical Section
 * 	- Restores the state of the PRIMASK given an input
 * Param "IBit_State": PRIMASK State to update
 */
extern "C" void EndCriticalSection(int32_t IBit_State);
