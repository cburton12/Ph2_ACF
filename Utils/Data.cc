/*

    FileName :                     Data.cc
    Content :                      Data handling from DAQ
    Programmer :                   Nicolas PIERRE
    Version :                      1.0
    Date of creation :             10/07/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Data.h"
#include <iostream>

namespace Ph2_HwInterface
{
	//Data Class

	// copy constructor
        Data::Data( const Data& pD ) :

		// Initialise( pD.fNevents );
		fNevents( pD.fNevents ),
		fCurrentEvent(pD.fCurrentEvent ),
		fNCbc( pD.fNCbc ),
		fEventSize( pD.fEventSize )
	{
	}


        void Data::Set( const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents )
	{
	        Reset();

		std::vector<uint8_t> flist;
		for ( auto word: pData )
		{
		    flist.push_back((word >> 24) & 0xFF);
		    flist.push_back((word >> 16) & 0xFF);
		    flist.push_back((word >>  8) & 0xFF);
		    flist.push_back(word  & 0xFF);
		}

		// initialize the buffer data array and the buffer size (one 32 bit word is 4 char!)
		fNevents = static_cast<uint32_t>( pNevents );
		fEventSize = static_cast<uint32_t>( flist.size() / fNevents );
		fNCbc = ( fEventSize - ( EVENT_HEADER_TDC_SIZE_CHAR ) ) / ( CBC_EVENT_SIZE_CHAR );

#ifdef __CBCDAQ_DEV__
		std::cout << "Initializing list with " << flist.size() << ", i.e 4 * " << pData.size()
			  << " chars containing data from " 
                          << fNevents << "  Events with an eventbuffer size of " << fEventSize << " and " << fNCbc 
                          << " CBCs each! " << EVENT_HEADER_TDC_SIZE_CHAR << " " << CBC_EVENT_SIZE_CHAR << std::endl;
#endif

		// Fill fEventList
		std::vector<uint8_t> lvec;
                for (auto i = 0; i < flist.size(); ++i)
		{
                    lvec.push_back(flist[i]);
                    if ( i > 0 && ((i+1) % fEventSize) == 0 ) 
		    {
		      fEventList.push_back(new Event( pBoard, fNCbc, lvec ));
			//std::cout << "nEvent: " << fEventList.size() << ", lvec.size(): " << lvec.size() << ", fEventSize: " << fEventSize << std::endl;
			lvec.clear(); 
		    }
		}
	}

	void Data::Reset()
	{
	        for ( auto ievt: fEventList ) 
		      delete ievt;
                fEventList.clear();
		fCurrentEvent = 0;
	}
}

