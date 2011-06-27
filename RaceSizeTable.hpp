//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _CRACESIZETABLE_HPP_
#define _CRACESIZETABLE_HPP_

#include <sys/types.h>
#include "Character.hpp"

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
