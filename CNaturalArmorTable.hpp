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


#ifndef CNATURALARMORTABLE
#define CNATURALARMORTABLE

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

class CNaturalArmorTable
{
    public:
        
        //sucht einen Eintrag aus der Tabelle nach Rassentyp
        //liefert true oder false je nachdem ob ein eintrag gefunden wurde.
        bool find(CCharacter::race_type race, MonsterArmor &ret);

        //Constructor        
        CNaturalArmorTable();
        
        //Destructor
        ~CNaturalArmorTable();
        
        //Lädt die Tabelle neu
        void reload();
        
        //liefert den Wert der Privat Variable m_dataOK zurück
        inline bool isDataOk(){ return m_dataOK; }
        
     protected:
     
        //Tabelle zum aufnehmen der eigentlichen Daten
        typedef hash_map<uint16_t, MonsterArmor> TABLE;
        TABLE m_ArmorTable;
        
        //Bool Wert der enthält ob das Laden der Daten geklappt hat bzw alles IO mit der Tabelle ist
        bool m_dataOK;
        
        //leer die alte Tabelle (löscht deren Inhalt)
        void clearOldTable();
        
};
#endif
