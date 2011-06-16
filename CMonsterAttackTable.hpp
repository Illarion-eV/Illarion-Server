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


#ifndef CMONSTERATTACKTABLE
#define CMONSTERATTACKTABLE

#include <string>
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

struct AttackBoni
{
    uint8_t attackType;
    int16_t attackValue;
    int16_t actionPointsLost;
    AttackBoni() : attackType(0), attackValue(0), actionPointsLost(0) {}
};

class CMonsterAttackTable
{
    
    public:
    
        //Constructor
        CMonsterAttackTable();
    
        //Destructor; 
        ~CMonsterAttackTable();
    
        //Sucht einen Eintrag und liefert die zugehörigen Boni
        bool find(CCharacter::race_type race, AttackBoni &ret);
    
        //lädt die Tabelle neu
        void reload();
    
        //get für m_dataOk (Zeigt an ob das Laden erfolgreich war)
        inline bool isDataOk() { return m_dataOk; }
    
    private:
    
        //Gibt an ob Laden der Daten erfolgreich war
        bool m_dataOk;
    
        //leert die alte Tabelle beim neuladen
        void clearOldTable();
    
        typedef hash_map<uint16_t, AttackBoni>TABLE;
        TABLE raceAttackBoni;

};
#endif

