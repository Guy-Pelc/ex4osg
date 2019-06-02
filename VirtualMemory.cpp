#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;

void fillPM(word_t* arr,int len){
	for (int i=0;i<len;++i){
		PMwrite(i,arr[i]); 
	}
	cout<<endl;
	printPhysical();
}

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

//return true if p2 is more 'suitable' for eviction than p1
bool isReplacePageToEvict(	word_t &page_swapped_in,word_t &p1,
							word_t &p2){
	cout<<"isReplacePageToEvict"<<endl;
	//in case there is not yet a candidate
	if (p1 == -1){cout<<"true"<<endl; return true;}

	word_t p1val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p1),
						abs(page_swapped_in-p1)); 
	word_t p2val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p2),
						abs(page_swapped_in-p2)); 
	cout<<"p1,p2,p,NUM_PAGES="<<p1<<","<<p2<<","<<page_swapped_in<<","<<NUM_PAGES<<endl;
	cout<<"p1val,p2val="<<p1val<<","<<p2val<<endl;
	return p1val<p2val;
}


int getPageToEvict(	word_t &emptyFrame,word_t &protectedTableFrameNumber,word_t &pageToInsertNumber){
	cout<<"getPageToEvict..."<<endl;
	printPhysical();

	bool isEmptyTable;

	word_t pageToEvictFrameNumber;
	word_t addressToPtrOfPageToEvict;
	word_t pageToEvict = -1;
	
	word_t table2FrameNumber;
	word_t pageFrameNumber;
	word_t table2Address;
	
	if (TABLES_DEPTH==2){
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
					word_t candidatePageToEvict = i*PAGE_SIZE+j;
					cout<<"found page to evict:"<<candidatePageToEvict<<endl;
					isEmptyTable = false;
					cout<<"comparing with current page to evict..."<<endl;

					if (isReplacePageToEvict(pageToInsertNumber,pageToEvict,candidatePageToEvict)){
						cout<<"switching page to evict"<<endl;
						addressToPtrOfPageToEvict = j+table2Address;
						pageToEvictFrameNumber = pageFrameNumber;
						pageToEvict = candidatePageToEvict;
					}
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
					cout<<"found empty table"<<endl;
					PMwrite(i,0);
					// set return value
					emptyFrame = table2FrameNumber;
					
					return 0;
				}
			}
		}
	}
	//depth = 1
	else{
		table2FrameNumber = 0;
		int i = 0;
	//second level:
			for (int j =0; j<PAGE_SIZE;++j){

				table2Address = frameToAddress(table2FrameNumber);
				PMread(j+table2Address,&pageFrameNumber);
				//exists frame with page:
				if (pageFrameNumber!=0){
					word_t candidatePageToEvict = i*PAGE_SIZE+j;
					cout<<"found page to evict:"<<candidatePageToEvict<<endl;
					isEmptyTable = false;
					cout<<"comparing with current page to evict..."<<endl;

					if (isReplacePageToEvict(pageToInsertNumber,pageToEvict,candidatePageToEvict)){
						cout<<"switching page to evict"<<endl;
						addressToPtrOfPageToEvict = j+table2Address;
						pageToEvictFrameNumber = pageFrameNumber;
						pageToEvict = candidatePageToEvict;
					}
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
	if (TABLES_DEPTH == 2){
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
	}
	// tables depth == 1
	else {
		compFrame = 0;
		for (int j = 0;j<PAGE_SIZE;++j){
					PMread(frameToAddress(compFrame)+j,&compFramelvl2);
					if(compFramelvl2>maxFrame){maxFrame=compFramelvl2;}		
				}
	}
	
	cout<<"..."<<maxFrame<<endl;
	return maxFrame;
}


int VMtranslateAddress(uint64_t virtualAddress,uint64_t *physicalAddress){
	cout<<"VMtranslateAddress "<<virtualAddress<<"..."<<endl;


	//split offset and page address:
	//later split to (tableAddres)*,pageAddress,offset
	word_t offset = virtualAddress % PAGE_SIZE;
	word_t pageNumber = virtualAddress / PAGE_SIZE;
	
	word_t pageFrameNumber;
	word_t emptyFrame;

	word_t table2toPageIndex = pageNumber % PAGE_SIZE;
	word_t table1to2Index = pageNumber / PAGE_SIZE;


	word_t table2FrameNumber;
	word_t table2Address;
	//get table1to2FrameNumber:
	if (TABLES_DEPTH == 2){
		PMread(0+table1to2Index,&table2FrameNumber); // base of table1 is always zero
		cout<<table1to2Index<<","<<table2FrameNumber<<endl;
		if (table2FrameNumber == 0){
			cout<<"PAGE FAULT - TABLE NOT IN RAM. searching for empty frame"<<endl;
			emptyFrame =1 + getMaxUsedFrame();

			if (emptyFrame==NUM_FRAMES){
			cout<<"NO EMPTY FRAME. choosing victim and releasing frame"<<endl;

			getPageToEvict(emptyFrame,table2FrameNumber, pageNumber);

		}
			// clear frame - only for tables.
			clearTable(emptyFrame);

			table2FrameNumber = emptyFrame;
			PMwrite(table1to2Index,table2FrameNumber);
		}

		table2Address = frameToAddress(table2FrameNumber);

	}
	//depth == 1
	else{
		table2FrameNumber = 0;
		table2Address = 0;
		table2toPageIndex = pageNumber;
	}
	//get pageFrameNumber:
	PMread(table2Address+table2toPageIndex,&pageFrameNumber); 
	cout<<table2Address+table2toPageIndex<<","<<pageFrameNumber<<endl;

	if (pageFrameNumber==0){
		cout<<"PAGE FAULT - PAGE NOT IN RAM. searching for empty frame"<<endl;

		emptyFrame =1 + getMaxUsedFrame();


		if (emptyFrame==NUM_FRAMES){
			cout<<"NO EMPTY FRAME. choosing victim and releasing frame"<<endl;

			getPageToEvict(emptyFrame,table2FrameNumber, pageNumber);

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
