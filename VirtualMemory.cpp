#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>
using namespace std;



//general depth version
//assumes all tables and pages are loaded in ram.
word_t _getMaxFrame(word_t t, int depth){
	word_t m=t;
	word_t f;

	for (int i=0;i<PAGE_SIZE;++i){
		PMread(t*PAGE_SIZE+i,&f);
		m = max(m,f);
		// cout<<"m["<<i<<"]="<<m<<endl;
			if (f!=0 && depth>1){
				// cout<<"here"<<endl;
				m = max(m,_getMaxFrame(f, depth-1));
			}
	}
	// cout<<"m="<<m<<endl;
	return m;
}
word_t getMaxFrame(){
	cout<<"getMaxFrame"<<endl;
	word_t m = _getMaxFrame(0, TABLES_DEPTH);
	cout<<"max frame: "<<m<<endl;
	return m;
}
int splitAddress(uint64_t virtualAddress,uint64_t* pArr){
	for (int i = TABLES_DEPTH;i>=0;--i){
		pArr[i] = virtualAddress % PAGE_SIZE;
		virtualAddress = virtualAddress / PAGE_SIZE;
		cout<<"pArr["<<i<<"]="<<pArr[i]<<endl;
	}
	return 0;
}

word_t _evictFrame(	word_t pageToInsert,
					word_t &pToEvict,
					word_t &fToEvict,
					word_t &ptrToRemove,
					word_t t,
					word_t pBase,
					int depth){
	word_t f;
	word_t p;

	for (int i=0;i<PAGE_SIZE;++i){
		PMread(t*PAGE_SIZE+i,&f);
		if (depth>1){
			//t points to tables
			if (f!=0){
				// table exists at frame f
				_evictFrame(pageToInsert,
							pToEvict,
							fToEvict,
							ptrToRemove,
							f,
							i*pow(PAGE_SIZE,depth-1),
							depth-1);
			}
		}
		//else depth==1
		else{
			// t points to pages
			p = pBase + i;
			if (f!=0){
				// page exists at frame f
				//check if to replace
				if (isReplacePageToEvict(pageToInsert,pToEvict,p)){
					pToEvict = p;
					fToEvict = f;
					ptrToRemove = t*PAGE_SIZE+i;
					cout<<"replaced page to evict."<<endl;
					cout<<"f,p,ptr="<<f<<","<<p<<","<<ptrToRemove<<endl;
				}
			}
		}


	}
	return 0;
}
/** returns index of evicted frame*/
word_t evictFrame(word_t pageToInsert){
	word_t pToEvict=-1;
	word_t fToEvict=-1;
	word_t ptrToRemove=-1;

	_evictFrame	(
				pageToInsert,
				pToEvict,
				fToEvict,
				ptrToRemove,
				0,
				0,
				TABLES_DEPTH
				);
	// assumes evicted is page and not table!
	PMevict(fToEvict,pToEvict);
	
	//unlink from table
	PMwrite(ptrToRemove,0);
	return fToEvict;
}

// pArr[i] = offset of ith table (0..n-1)
// pArr[n] = offset of value in page
// pArr[n-1] = offset of page in final table
int VMtranslateAddress(uint64_t virtualAddress, uint64_t &physicalAddress){
	cout<<"tablesDepth="<<TABLES_DEPTH<<endl;
	word_t n = TABLES_DEPTH;
	uint64_t pArr[n+1] = {0};
	word_t pageIndex = virtualAddress/PAGE_SIZE;
	splitAddress(virtualAddress, pArr);

	word_t t=0;
	word_t f;

	for (int i=0;i<n;++i){
		cout<<"t["<<i<<"]="<<t<<endl;
		//(i=0..n-2) set f to frame of ith table 
		//(i=n-1) set f to frame of page
		PMread(t*PAGE_SIZE+pArr[i],&f);
		// if page/frame doesn't exist
		if (f==0){
			cout<<"page/frame doesn't exist in RAM"<<endl;
			f = getMaxFrame()+1;
			// no empty frames
			if (f==NUM_FRAMES){
				cout<<"need to evict!"<<endl;
				f = evictFrame(pageIndex);
			}
			// link frame to previous table
			PMwrite(t*PAGE_SIZE+pArr[i],f);
			//(i=0..n-2) t is frame of table: clear table
			if (i < n-1){
				clearTable(f);
			}
			// (i=n-1) t is frame of page:  restore page from disk 
			else{
				PMrestore(f,pageIndex);	
			}
		}
		//(i=0..n-2) set next table to frame.
		//(i=n-1) t won't be used. 
		t=f;
	}

	cout<<"t"<<TABLES_DEPTH<<"="<<t<<endl;

	physicalAddress = f*PAGE_SIZE+pArr[n]; 
	cout<<"physicalAddress="<<physicalAddress<<endl;
	return 0;
}

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
	if (p1 == -1){cout<<"set initial p="<<p2<<endl; return true;}

	word_t p1val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p1),
						abs(page_swapped_in-p1)); 
	word_t p2val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p2),
						abs(page_swapped_in-p2)); 
	cout<<"p1,p2,p,NUM_PAGES="<<p1<<","<<p2<<","<<page_swapped_in<<","<<NUM_PAGES<<endl;
	cout<<"p1val,p2val="<<p1val<<","<<p2val<<endl;
	if (p1val<p2val){cout<<"replacing..."<<endl;}
	else{cout<<"not replacing"<<endl;}
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
		VMtranslateAddress(virtualAddress,physicalAddress);

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
		VMtranslateAddress(virtualAddress,physicalAddress);

		PMwrite(physicalAddress,value); //writes value at page


		cout<<"..."<<value<<endl<<endl;

		printPhysical();

    return 1;
}
