#include "VirtualMemory.h"
#include "PhysicalMemory.h"

// #include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>
using namespace std;

#define DEBUG 1

void clearTable(uint64_t frameIndex) {
//	cout<<"clearTable: "<<frameIndex<<endl;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

//return true if p2 is more 'suitable' for eviction than p1
bool isReplacePageToEvict(	word_t &page_swapped_in,word_t &p1,
							word_t &p2){
//	cout<<"isReplacePageToEvict"<<endl;
	//in case there is not yet a candidate
	if (p1 == -1){
//		cout<<"set initial p="<<p2<<endl; 
		return true;
	}

	word_t p1val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p1),
						abs(page_swapped_in-p1)); 
	word_t p2val = min(	word_t(NUM_PAGES)-abs(page_swapped_in-p2),
						abs(page_swapped_in-p2)); 
//	cout<<"p1,p2,p,NUM_PAGES="<<p1<<","<<p2<<","<<page_swapped_in<<","<<NUM_PAGES<<endl;
//	cout<<"p1val,p2val="<<p1val<<","<<p2val<<endl;
	if (p1val<p2val){
//		cout<<"replacing..."<<endl;
	}
	else{
//		cout<<"not replacing"<<endl;
	}
	return p1val<p2val;
}
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
//	cout<<"getMaxFrame"<<endl;
	word_t m = _getMaxFrame(0, TABLES_DEPTH);
//	cout<<"max frame: "<<m<<endl;
	return m;
}
int splitAddress(uint64_t virtualAddress,uint64_t* pArr){
	for (int i = TABLES_DEPTH;i>=0;--i){
		pArr[i] = virtualAddress % PAGE_SIZE;
		virtualAddress = virtualAddress / PAGE_SIZE;
//		cout<<"pArr["<<i<<"]="<<pArr[i]<<endl;
	}
	return 0;
}

bool _evictFrame(	word_t pageToInsert,
					word_t &pToEvict,
					word_t &fToEvict,
					word_t &ptrToRemove,
					word_t ptrToT,
					word_t t,
					word_t pBase,
					int depth,
					word_t protectedTable){
	word_t f;
	word_t p;
	word_t ptrToF;
//	cout<<"_evictFrame, depth = "<<depth<<endl;
	bool isTEmpty = true;
	for (int i=0;i<PAGE_SIZE;++i){
		ptrToF = t*PAGE_SIZE+i; 
		PMread(ptrToF,&f);
		if (f!=0){
			// table/page exists at frame f
			isTEmpty = false;
			if (depth>1){
				//t points to tables
				//continue recursivly until t points to pages
				if (_evictFrame(pageToInsert,
							pToEvict,
							fToEvict,
							ptrToRemove,
							ptrToF,
							f,
							pBase+i*pow(PAGE_SIZE,depth-1),
							depth-1,
							protectedTable)==false){ return false;}
			}
			//else depth==1
			else{
				// t points to pages
				p = pBase + i;
				// page exists at frame f
				//check if to replace
				if (isReplacePageToEvict(pageToInsert,pToEvict,p)){
					pToEvict = p;
					fToEvict = f;
					ptrToRemove = t*PAGE_SIZE+i;
//					cout<<"replaced page to evict."<<endl;
//					cout<<"f,p,ptr="<<f<<","<<p<<","<<ptrToRemove<<endl;
				}
			}
		}
	}
	if (isTEmpty){
		//t is an empty table -
		// remove link to t and end search.

//		cout<<"found empty table!"<<endl;
		if (t== protectedTable){
//			cout<<"t is protected"<<endl;
			return true;
		}
		pToEvict = -1;
		fToEvict = t;
		ptrToRemove = ptrToT;
		return false;
	}	
	
	return true;
}
/** returns index of evicted frame*/
word_t evictFrame(word_t pageToInsert,word_t protectedTable){
	word_t pToEvict=-1;
	word_t fToEvict=-1;
	word_t ptrToRemove=-1;

	int isPageEvicted = _evictFrame	(
				pageToInsert,
				pToEvict,
				fToEvict,
				ptrToRemove,
				-1,
				0,
				0,
				TABLES_DEPTH,
				protectedTable
				);

	if (isPageEvicted){
		//evict page to disk
		PMevict(fToEvict,pToEvict);
	}

	//unlink from table
	PMwrite(ptrToRemove,0);
	return fToEvict;
}

// pArr[i] = offset of ith table (0..n-1)
// pArr[n] = offset of value in page
// pArr[n-1] = offset of page in final table
int VMtranslateAddress(uint64_t virtualAddress, uint64_t &physicalAddress){
//	cout<<"tablesDepth="<<TABLES_DEPTH<<endl;
	word_t n = TABLES_DEPTH;
	uint64_t pArr[n+1] = {0};
	word_t pageIndex = virtualAddress/PAGE_SIZE;
	splitAddress(virtualAddress, pArr);

	word_t t=0;
	word_t f;

	for (int i=0;i<n;++i){
//		cout<<"t["<<i<<"]="<<t<<endl;
		//(i=0..n-2) set f to frame of ith table 
		//(i=n-1) set f to frame of page
		PMread(t*PAGE_SIZE+pArr[i],&f);
		// if page/frame doesn't exist
		if (f==0){
//			cout<<"page/frame doesn't exist in RAM"<<endl;
			f = getMaxFrame()+1;
			// no empty frames
			if (f==NUM_FRAMES){
//				cout<<"need to evict!"<<endl;
				f = evictFrame(pageIndex,t);
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
			// printPhysical();
		}
		//(i=0..n-2) set next table to frame.
		//(i=n-1) t won't be used. 
		t=f;
	}

//	cout<<"t"<<TABLES_DEPTH<<"="<<t<<endl;

	physicalAddress = f*PAGE_SIZE+pArr[n]; 
//	cout<<"physicalAddress="<<physicalAddress<<endl;
	return 0;
}


/*
 * Initialize the virtual memory
 */
void VMinitialize() {
//	cout<<"VMinitialize"<<endl;
    clearTable(0);
//   cout<<"end init\n"<<endl;
}





/* reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value) {
//		cout<<"VMread "<<virtualAddress<<"..."<<endl;
		if (virtualAddress>=VIRTUAL_MEMORY_SIZE){
//			cout<<"error: virtual address must not exceed VIRTUAL_MEMORY_SIZE"<<endl;
			return 0;
		}

		uint64_t physicalAddress;
		VMtranslateAddress(virtualAddress,physicalAddress);

		PMread(physicalAddress,value); //writes value at page

//		cout<<"..."<<*value<<endl<<endl;
    return 1;
}

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value) {
//		cout<<"VMwrite "<<virtualAddress<<"..."<<endl;
		if (virtualAddress>=VIRTUAL_MEMORY_SIZE){
//			cout<<"error: virtual address must not exceed VIRTUAL_MEMORY_SIZE"<<endl;
			return 0;
		}

		uint64_t physicalAddress;
		VMtranslateAddress(virtualAddress,physicalAddress);

		PMwrite(physicalAddress,value); //writes value at page


//		cout<<"..."<<value<<endl<<endl;

		// printPhysical();

    return 1;
}
