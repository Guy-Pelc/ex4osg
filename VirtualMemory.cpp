#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <iostream>
using namespace std;

void printPhysical(){
	word_t val;
	for (uint64_t i=0; i<RAM_SIZE; ++i){
			PMread(i,&val);
			cout<<"RAM["<<i<<"]="<<val<<endl;}
	cout<<endl;

	}

//translates frame to base address of frame.
//assumes depth is 1
word_t frameToAddress(word_t frame){
	// cout<<"frame "<<frame<<" to base page address ";
	word_t basePageAddress = frame*PAGE_SIZE;
	// cout<<basePageAddress<<endl;
	return basePageAddress;
}
void clearTable(uint64_t frameIndex) {
	cout<<"clearTable: "<<frameIndex<<endl;
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
//assumes no empty tables.
int getPageToEvict(	word_t &emptyFrame,word_t &protectedTableFrameNumber){
	cout<<"getPageToEvict..."<<endl;
	printPhysical();

	bool isEmptyTable;

	word_t pageToEvictFrameNumber;
	word_t addressToPtrOfPageToEvict;
	word_t pageToEvict = -1;
	
	word_t table2FrameNumber;
	word_t pageFrameNumber;
	word_t table2Address;
	//ASSUMES ONLY ROOM FOR ONE PAGE IN MEMORY
	//first level
	for (int i = 0; i<PAGE_SIZE; ++i){
		PMread(i,&table2FrameNumber);
		if (table2FrameNumber != 0) {
			isEmptyTable = true;

			//second level:
			for (int j =0; j<PAGE_SIZE;++j){

				table2Address = frameToAddress(table2FrameNumber);
				PMread(j+table2Address,&pageFrameNumber);
				//exists frame with page:
				if (pageFrameNumber!=0){
					cout<<"found page to evict:"<<pageFrameNumber<<endl;
					isEmptyTable = false;
					addressToPtrOfPageToEvict = j+table2Address;
					pageToEvictFrameNumber = pageFrameNumber;
					pageToEvict = i*PAGE_SIZE+j;

				}
			}
			//evict table if empty (prefer over evict page)
			if (isEmptyTable==true){
				if (table2FrameNumber == protectedTableFrameNumber){
					cout<<"move on"<<endl;
					continue;
				}
				cout<<"protectedTableFrameNumber:"<<protectedTableFrameNumber<<endl;
				cout<<"table2FrameNumber:"<<table2FrameNumber<<endl;
				// cout<<"here"<<endl;
				//unlink table:
				PMwrite(i,0);
				// set return value
				emptyFrame = table2FrameNumber;
				cout<<"found empty table"<<endl;
				return 0;
			}
		}
	}
	PMevict(pageToEvictFrameNumber,pageToEvict);
	//unlink from table
	PMwrite(addressToPtrOfPageToEvict,0);

	emptyFrame = pageToEvictFrameNumber;
	if (pageToEvict==-1){
		cout<<"error! no page in ram"<<endl;
		return -1;
	}
				// cout<<"pageFrameNumber:"<<pageFrameNumber<<endl;			

	cout<<"..."<<pageToEvict<<" at frame "<<pageToEvictFrameNumber<<endl;
	return 0;
}

//fix for depth 2!!
word_t getMaxUsedFrame(){
	cout<<"getMaxUsedFrame..."<<endl;
	word_t curTableAddr = 0;
	word_t maxFrame = 0;
	word_t compFrame;
	word_t compFramelvl2;
	//first depth table
	for (int i=0;i<PAGE_SIZE;++i){
		PMread(curTableAddr+i,&compFrame);
		if (compFrame!=0){
			if(compFrame>maxFrame){maxFrame=compFrame;}	
			// second depth table	
			for (int j = 0;j<PAGE_SIZE;++j){
				PMread(frameToAddress(compFrame)+j,&compFramelvl2);
				if(compFramelvl2>maxFrame){maxFrame=compFramelvl2;}		
			}
		}
		
	}
	cout<<"..."<<maxFrame<<endl;
	return maxFrame;
}


int VMtranslateAddress(uint64_t virtualAddress,uint64_t *physicalAddress){
	cout<<"VMtranslateAddress "<<virtualAddress<<"..."<<endl;

	//assumes depth = 1, all tables and pages are in memory

	//split offset and page address:
	//later split to (tableAddres)*,pageAddress,offset
	word_t offset = virtualAddress % PAGE_SIZE;
	word_t pageNumber = virtualAddress / PAGE_SIZE;
	
	word_t pageFrameNumber;
	word_t emptyFrame;

	word_t table1to2Index = pageNumber / PAGE_SIZE;
	word_t table2toPageIndex = pageNumber % PAGE_SIZE;


	word_t table2FrameNumber;

	//get table1to2FrameNumber:

	PMread(0+table1to2Index,&table2FrameNumber); // base of table1 is always zero
	cout<<table1to2Index<<","<<table2FrameNumber<<endl;
	if (table2FrameNumber == 0){
		cout<<"PAGE FAULT - TABLE NOT IN RAM. searching for empty frame"<<endl;
		emptyFrame =1 + getMaxUsedFrame();
		// clear frame - only for tables.
		clearTable(emptyFrame);

		table2FrameNumber = emptyFrame;
		PMwrite(table1to2Index,table2FrameNumber);
	}

	word_t table2Address = frameToAddress(table2FrameNumber);


	//get pageFrameNumber:
	PMread(table2Address+table2toPageIndex,&pageFrameNumber); 
	cout<<table2Address+table2toPageIndex<<","<<pageFrameNumber<<endl;

	if (pageFrameNumber==0){
		cout<<"PAGE FAULT - PAGE NOT IN RAM. searching for empty frame"<<endl;

		emptyFrame =1 + getMaxUsedFrame();


		if (emptyFrame==NUM_FRAMES){
			cout<<"NO EMPTY FRAME. choosing victim and releasing frame"<<endl;

			getPageToEvict(emptyFrame,table2FrameNumber);

		}
		//restore page and link to table
		PMrestore(emptyFrame, pageNumber);

		pageFrameNumber = emptyFrame;

		PMwrite(table2Address+table2toPageIndex,pageFrameNumber);
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

		printPhysical();

    return 1;
}
