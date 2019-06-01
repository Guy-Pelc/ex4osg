#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <iostream>
using namespace std;

//translates frame to base address of frame.
//assumes depth is 1
word_t frameToAddress(word_t frame){
	cout<<"frame "<<frame<<" to base page address ";
	word_t basePageAddress = frame*2;
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

word_t getMaxUsedFrame(){
	cout<<"getMaxUsedFrame..."<<endl;
	word_t curTableAddr = 0;
	word_t maxFrame = 0;
	word_t compFrame;
	for (int i=0;i<2;++i){
		PMread(curTableAddr+i,&compFrame);
		if(compFrame>maxFrame){maxFrame=compFrame;}
	}
	cout<<"..."<<maxFrame<<endl;
	return maxFrame;
}
/** BASE FOR MORE THAN 1 LVL DEPTH
word_t getMaxUsedFrame(){
	word_t maxFrame = 0;
	_getMaxUsedFrame(0,maxFrame);
	return maxFrame;
}

word_t _getMaxUsedFrame(word_t curFrame, $maxFrame){
	word_t nextFrame;
	for (int i=0;i<2;++i){
		PMread(curFrame,&val);	
		if (val!=0){
			_getMaxUsedFrame(val,)
		}
	}
	
	if val()

	return 0;
}
*/
int VMtranslateAddress(uint64_t virtualAddress,uint64_t *physicalAddress){
	cout<<"VMtranslateAddress "<<virtualAddress<<"..."<<endl;

	//assumes depth = 1, all tables and pages are in memory

	//split offset and page address:
	//later split to (tableAddres)*,pageAddress,offset
	word_t offset = virtualAddress%2;
	word_t pageNumber = virtualAddress/2;
	word_t pageFrameNumber;
	
	word_t emptyFrame;
	PMread(pageNumber,&pageFrameNumber); //reads frame number of page 0
	if (pageFrameNumber==0){
		cout<<"PAGE FAULT - PAGE NOT IN RAM. searching for empty frame"<<endl;
		emptyFrame =1 + getMaxUsedFrame();
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
