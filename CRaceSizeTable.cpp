#include "db/ConnectionManager.h"
#include "CRaceSizeTable.hpp"


CRaceSizeTable::CRaceSizeTable() : m_dataOk(false)
{
    reload();
}

CRaceSizeTable::~CRaceSizeTable()
{
    clearOldTable();
}

void CRaceSizeTable::reload()
{
#ifdef CDataConnect_DEBUG
    std::cout<<"Trying to reload CRaceSizeTable!"<<std::endl;
#endif
    
    try
    {
       std::vector<uint16_t>minheight; 
       std::vector<uint16_t>maxheight;
       std::vector<uint16_t>raceid;
       
       ConnectionManager::TransactionHolder transaction = accdbmgr->getTransaction();
       size_t rows = di::select_all<di::Integer, di::Integer, di::Integer>( transaction,raceid, minheight, maxheight,"SELECT id, minbodyheight, maxbodyheight FROM raceattr");
       
       //Zeilenweises laden der Daten
       for ( size_t i = 0; i < rows; ++i )
       {
            minsizes[ raceid[i] ] = minheight[i];
	    maxsizes[ raceid[i] ] = maxheight[i];
       }       
    
    
       m_dataOk = true;
                   
#ifdef CDataConnect_DEBUG
	   std::cout << "loaded " << rows << " rows into CRaceSizeTable" << std::endl;
#endif
    }
    catch ( ... )
    {
        m_dataOk = false;
    }
}

uint8_t CRaceSizeTable::getRelativeSize( CCharacter::race_type race, uint16_t size )
{
    // relative size is between 50 and 120 (in percent) and a linear interploation between min and max size
    uint16_t maxsize, minsize;
    TABLE::iterator iter;
    iter = minsizes.find( static_cast<uint16_t>(race) );
    if ( iter == minsizes.end() ) return 100;
    else
    {
         minsize = iter->second;
    }
    iter = maxsizes.find( static_cast<uint16_t>(race) );
    if ( iter == maxsizes.end() ) return 100;
    else
    {
         maxsize = iter->second;
    }
    if( size >= minsize && size <= maxsize )
        return (uint8_t)((70*(size-minsize))/(maxsize-minsize)+50);
    else
        return 100;
}

void CRaceSizeTable::clearOldTable()
{
    minsizes.clear();
    maxsizes.clear();
}
   
