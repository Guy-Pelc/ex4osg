
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include "MemoryConstants.h"

#include <iostream>
using namespace std;

#include <cstdio>
#include <cassert>

//test getMaxUsedFrame
void test11(){
	VMwrite(0,13);
	printPhysical();
}
void test8(){
	PMwrite(0,1);
	// PMwrite()
	PMwrite(3,2);
	PMwrite(5,13);
	cout<<getMaxUsedFrame()<<endl;

	printPhysical();
}

void test(){
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
void test5(){
	VMwrite(0,13);
	word_t pageToEvict;
	word_t frameOfPageToEvict;
	getPageToEvict(pageToEvict,frameOfPageToEvict);
}
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
