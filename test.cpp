
#include "VirtualMemory.h"

#include "MemoryConstants.h"

#include <iostream>
using namespace std;

#include <cstdio>
#include <cassert>

#include <cmath>
#include <algorithm>
void test(){
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,100+i);
	}
	cout<<"yes"<<endl;
}
int test6() {
	//make sure VW >= 1+PW
	//make sure PW-OFF >= depth
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5* i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5* i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}
void test5(){
	// depth n
	// tests all with no assumptions
	// VW = 3
	// PW = 3
	// OFF = 1

	// word_t arr[8] =
	// 		{1,0,
	// 		2,3,
	// 		100,101,
	// 		102,103};
					
	// fillPM(arr,8);

	// VMwrite(4,104);
	// VMwrite(7,107);
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,100+i);
	}

	word_t val;
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMread(i,&val);
		assert(val==100+i);
	}
	cout<<"success!"<<endl;
}
void test4(){
	// depth n
	// tests evicting pages (not frames)
	// assume single candidate
	// VW = 3
	// PW = 3
	// OFF = 1

	word_t arr[8] =
				{1,2,
				0,0,
				3,0,
				104,105};
					
	fillPM(arr,8);
	// for (int i=0;i<VIRTUAL_MEMORY_SIZE/2;++i){
	// 	VMwrite(i,100+i);
	// }
	// VMwrite(4,104);
	VMwrite(7,107);

}
void test11(){
	// depth n
	// tests creating new pages.
	// assume tables are in place and infinite PM
	// VW = n
	// PW = n+1
	// OFF = 1

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
