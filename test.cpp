
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include "MemoryConstants.h"

#include <iostream>
using namespace std;

#include <cstdio>
#include <cassert>

#include <cmath>
#include <algorithm>




void test(){
	// depth n
	// tests creating new pages.
	// assume tables are in place and infinite PM
	// VW = 2
	// PW = 3
	// OFF = 1
	// word_t arr[4] = {1,0,10,11};
	// fillPM(arr,4);
	// getMaxFrame();
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,100+i);
	}

}
void test31(){
	// test for any depth code, 
	//assume all pages and tables are in PM
	//VW = 4
	//PW = 4
	//OFF = 1
	//-> depth = 1
	word_t arr[26] ={1,8,
					2,5,
					3,4,
					0,0,
					0,0,
					6,7,
					0,0,
					0,0,
					9,12,
					10,11,
					0,0,
					0,0,
					13,14};
					
	fillPM(arr,26);


	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,i);
	}
	getMaxFrame();
	// cout<<"max frame: "<<getMaxFrame()<<endl;
}

void test23(){
	//depth 2 irregular table sizes, infinite #frames
	//VW = 5
	//offset = 2
	//PW = 6 (only 10 needed)
	
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,i);
	}
}

int main(){
	VMinitialize();
	test();

	return 0;
	}


int main1(int argc, char **argv) {
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}
