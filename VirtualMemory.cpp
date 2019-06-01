#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <iostream>
using namespace std;

void printPhysical(){
	word_t val;
	for (uint64_t i=0; i<RAM_SIZE; ++i){
			PMread(i,&val);
			cout<<"RAM["<<i<<"]="<<val<<endl;}
	}

//translates frame to base address of frame.
//assumes depth is 1
word_t frameToAddress(word_t frame){
	cout<<"frame "<<frame<<" to base page address ";
	word_t basePageAddress = frame*PAGE_SIZE;
	cout<<basePageAddress<<endl;
	return basePageAddress;
}
void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}
/*
 * Initialize the virtual memory
 */
void VMinitialize() {
	cout<<"VMinitialize"<<endl;
    clearTable(0);
    cout<<"end init\n"<<endl;
}
//assumes lvl 1 depth - evicting page and not empty table.
int getPageToEvict(word_t &pageToEvict, word_t &pageFrameNumber){
	cout<<"getPageToEvict..."<<endl;

	pageToEvict = -1;
	
	word_t val;
	//fill table with values.
	//ASSUMES ONLY ROOM FOR ONE PAGE IN MEMORY
	for (int i = 0; i<PAGE_SIZE; ++i){
		PMread(i,&val);
		if (val != 0) {
			pageFrameNumber = val;
			pageToEvict = i;
			// cout<<"pageFrameNumber:"<<pageFrameNumber<<endl;			
		}
	}

	if (pageToEvict==-1){
		cout<<"error! no page in ram"<<endl;
		return -1;
	}
				// cout<<"pageFrameNumber:"<<pageFrameNumber<<endl;			

	cout<<"..."<<pageToEvict<<" at frame "<<pageFrameNumber<<endl;
	return 0;
}

word_t getMaxUsedFrame(){
	cout<<"getMaxUsedFrame..."<<endl;
	word_t curTableAddr = 0;
	word_t maxFrame = 0;
	word_t compFrame;
	for (int i=0;i<PAGE_SIZE;++i){
		PMread(curTableAddr+i,&compFrame);
		if(compFrame>maxFrame){maxFrame=compFrame;}
	}
	cout<<"..."<<maxFrame<<endl;
	return maxFrame;
}


int VMtranslateAddress(uint64_t virtualAddress,uint64_t *physicalAddress){
	cout<<"VMtranslateAddress "<<virtualAddress<<"..."<<endl;

	//assumes depth = 1, all tables and pages are in memory

	//split offset and page address:
	//later split to (tableAddres)*,pageAddress,offset
	word_t offset = virtualAddress%PAGE_SIZE;
	word_t pageNumber = virtualAddress/PAGE_SIZE;
	word_t pageFrameNumber;
	
	word_t emptyFrame;


	PMread(pageNumber,&pageFrameNumber); //reads frame number of page 0
	if (pageFrameNumber==0){
		cout<<"PAGE FAULT - PAGE NOT IN RAM. searching for empty frame"<<endl;

		emptyFrame =1 + getMaxUsedFrame();
		if (emptyFrame==NUM_FRAMES){
			cout<<"NO EMPTY FRAME. choosing victim and releasing frame"<<endl;
			word_t pageToEvict;
			word_t frameOfPageToEvict;
			getPageToEvict(pageToEvict,frameOfPageToEvict);
			PMevict(frameOfPageToEvict,pageToEvict);
			//unlink from table
			PMwrite(pageToEvict,0);

			emptyFrame = frameOfPageToEvict;
		}
		//restore page and link to table
		PMrestore(emptyFrame, pageNumber);
		pageFrameNumber = emptyFrame;

		PMwrite(pageNumber,pageFrameNumber);
	}

	word_t pageAddress = frameToAddress(pageFrameNumber);
	
	*physicalAddress = pageAddress+offset;


	cout<<"..."<<*physicalAddress<<endl;
	return 1;
}

/* reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
		cout<<"VMread "<<virtualAddress<<"..."<<endl;

		uint64_t physicalAddress;
		VMtranslateAddress(virtualAddress,&physicalAddress);

		PMread(physicalAddress,value); //writes value at page

		cout<<"..."<<*value<<endl<<endl;
    return 1;
}

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value) {
		cout<<"VMwrite "<<virtualAddress<<"..."<<endl;

		uint64_t physicalAddress;
		VMtranslateAddress(virtualAddress,&physicalAddress);

		PMwrite(physicalAddress,value); //writes value at page


		cout<<"..."<<value<<endl<<endl;
    return 1;
}
