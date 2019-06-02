
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
/*
void test22(){
	//complete test depth 2

	//VW = 3
	//OFFSET = 1
	//PW = 3

	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
	VMwrite(i,i);
	}

	word_t val;
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
	VMread(i,&val);
	cout<<"VM["<<i<<"]="<<val<<endl;
	}

}
*/
void test21(){
	//testing page to evict empty frame
	//depth 2

	//VW = 3
	//OFFSET = 1
	//PW = 3

	word_t pm[8] = {1,0,2,3,0,1,2,3};
	fillPM(pm,8);
	VMwrite(4,4); //now table 0 in frame 1 has 0,0 values.

	VMwrite(7,7); //should clear 1st frame that had table 0
}

void test20(){
	//testing page to evict for multiple pages in frames 
	//in depth 2

	//VW = 3
	//OFFSET = 1
	//PW = 3
	word_t pm[8] = {1,0,2,3,0,1,2,3};
	fillPM(pm,8);

	VMwrite(4,4);
}

void test15(){
	//testing page to evict for multiple pages in frames 
	//in depth 1

	//VW = 4
	//offset = 2
	//PW = 4
	//=>depth = 2

	VMwrite(0,10);
	VMwrite(4,22);
	VMwrite(8,33);
	
	// printPhysical();
	VMwrite(12,44);
	// printPhysical();
}

void test18(){
	word_t a = 1;
	word_t b = 2;
	cout<<min(word_t(NUM_PAGES)-abs((a-b)),a-b)<<endl;
}//testing isReplacePageToEvict
void test16(){
	word_t pageNumber = 3;
	word_t p1 = 1;
	word_t p2 = 2;
	cout<<isReplacePageToEvict(pageNumber,p1,p2)<<endl;
}


void test14(){

	PMwrite(0,1);
	PMwrite(1,0);
	PMwrite(2,2);
	PMwrite(3,3);
	PMwrite(4,0);
	PMwrite(5,1);
	PMwrite(6,2);
	PMwrite(7,3);

	printPhysical();

	VMwrite(4,4);
}
void test13(){
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
	VMwrite(i,i);
	}
}

//test getMaxUsedFrame
void test12(){
	VMwrite(0,10);
	VMwrite(5,5);
	VMwrite(0,10);
	VMwrite(3,33);

}
void test11(){
	VMwrite(0,13);
	printPhysical();
}
void test8(){
	PMwrite(0,1);
	// PMwrite()
	PMwrite(3,2);
	PMwrite(5,13);
	// cout<<getMaxUsedFrame()<<endl;

	printPhysical();
}

void test112(){
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
	VMwrite(i,i);
	}
	printPhysical();
}
void test7(){
	word_t val;
	// VMwrite(7,7);
	for (int i=0;i<VIRTUAL_MEMORY_SIZE;++i){
		VMwrite(i,i);
	}
	
	printPhysical();
	
	VMread(0,&val);
	printPhysical();
	
}
void test6(){
	VMwrite(0,13);
	VMwrite(3,17);

	printPhysical();

	word_t val;
	VMread(0,&val);
	printPhysical();

	VMread(2,&val);
	printPhysical();
}


//tests getPageToEvict
// void test5(){
// 	VMwrite(0,13);
// 	word_t addrToPtr;
// 	word_t pageToEvict;
// 	word_t frameOfPageToEvict;
// 	getPageToEvict(	pageToEvict,frameOfPageToEvict,
// 					addrToPtr);
// }
//tests getMaxUsedFrame
void test4(){
	VMwrite(0,13);
	VMwrite(3,17);
	// PMwrite(1,2);
	cout<<endl;
	
	word_t val;
	VMread(0,&val);
	// cout<<getMaxUsedFrame()<<endl;;
	
	// word_t ret;
	// VMread(0,&ret);
	printPhysical();
}
void test2(){
	VMinitialize();
	//set table links
	PMwrite(0,1);

	//write via vm to page:
	VMwrite(0,13);

	word_t ret;
	// PMread(2,&ret);
	// cout<<"Pmread 3, "<<ret<<endl;
	//read page via vm:
	VMread(0,&ret);
}
void test3(){
	
	//link 0th entry in table to 1st frame
	word_t ret;
	//set table links
	PMwrite(0,1);
	PMwrite(1,2);
	
	//write directly to page
	PMwrite(2,13);
	PMwrite(3,15);
	PMwrite(4,17);
	PMwrite(5,19);
	cout<<endl;

	//read from page via virtual addr
	
	VMread(0,&ret);
	VMread(1,&ret);
	VMread(2,&ret);
	VMread(3,&ret);
}
void test1(){
	PMwrite(uint64_t(0),13);
	word_t ret;
	PMread(uint64_t(0),&ret);
	printf("ret: %d",ret);
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
