#pragma once

#include "MemoryConstants.h"

bool isReplacePageToEvict(	word_t &page_swapped_in,word_t &p1,
							word_t &p2);

void fillPM(word_t* arr,int len);
word_t getMaxUsedFrame();
int getPageToEvict(word_t &pageToEvict, word_t &pageFrameNumber);
void printPhysical();
/*
 * Initialize the virtual memory
 */
void VMinitialize();

/* reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value);

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value);


