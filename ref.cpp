
//old version for depths 1,2
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

//old version for depths 1,2 
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




//assumes lvl 1 depth - evicting page and not empty table.
int getPageToEvict(word_t &emptyFrame){
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

//working version for depth 1
int VMtranslateAddress1(uint64_t virtualAddress,uint64_t *physicalAddress){
	cout<<"VMtranslateAddress "<<virtualAddress<<"..."<<endl;

	//assumes depth = 1, all tables and pages are in memory

	//split offset and page address:
	//later split to (tableAddres)*,pageAddress,offset
	word_t offset = virtualAddress % PAGE_SIZE;
	word_t pageNumber = virtualAddress / PAGE_SIZE;
	
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

/** base for depth>1 pageToEvict
assumes lvl 1 depth - evicting page and not empty table.
int getPageToEvict(word_t $pageToEvict, $word_t pageFrameNumber){
	cout<<"getPageToEvict..."<<endl;
	bool pagesInMemory[NUM_PAGES]={false};

	pageToEvict = -1;
	
	//fill table with values.
	//ASSUMES ONLY ROOM FOR ONE PAGE IN MEMORY
	for (int i = 0; i<2; ++i){
		PMread(i,&pageFrameNumber);
		if (pageFrameNumber != 0) {
			pagesInMemory[pageFrameNumber] = true;
			pageToEvict = i;
			
		}
	}

	//print table:
	cout<<"pagesInMemory: ";
	for (int i=0;i<NUM_PAGES;++i){
		if (pagesInMemory[i]){
			cout<<i<<",";
		}
	}
	cout<<endl;
	if (pageToEvict==-1){
		cout<<"error! no page in ram"<<endl;
		return -1;
	}
	cout<<"..."<<pageToEvict<<endl;
	return pageToEvict;
}
*/