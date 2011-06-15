#ifndef _CRACESIZETABLE_HPP_
#define _CRACESIZETABLE_HPP_

#include <sys/types.h>
#include "CCharacter.hpp"

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
#endif

#endif

class CRaceSizeTable
{
    public:
        CRaceSizeTable();
        ~CRaceSizeTable();
        
        /**
        * gets the relative size of a race 
        * @param race the race for which i want to get the relative size
	* @param size the absolute size
        */
        uint8_t getRelativeSize(CCharacter::race_type race, uint16_t size);
        
        /**
        * reloads the data
        */
        void reload();
        
        /**
        * checks if all loading works well
        * @return true if loading successfully otherwise false
        */
        inline bool isDataOk(){ return m_dataOk; }
        
    private:
        bool m_dataOk;
        void clearOldTable();
        
        typedef hash_map<uint16_t, uint16_t>TABLE;
        TABLE minsizes;
	TABLE maxsizes;

};

#endif
