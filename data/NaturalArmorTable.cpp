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


#include "db/ConnectionManager.hpp"
#include "NaturalArmorTable.hpp"
#include <iostream>

NaturalArmorTable::NaturalArmorTable() : m_dataOK(false) {
    reload();
}

NaturalArmorTable::~NaturalArmorTable() {
    clearOldTable();
}

void NaturalArmorTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"Trying to reload NaturalArmorTable!"<<std::endl;
#endif

    try {
        //LAden einer Connection
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

        std::vector<uint16_t> raceTyp;
        std::vector<int16_t> strokeArmor;
        std::vector<int16_t> punctureArmor;
        std::vector<int16_t> thrustArmor;

        //Laden der Daten in die Vectoren
        size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer>
                      (transaction, raceTyp, strokeArmor, punctureArmor, thrustArmor, "SELECT nar_race, nar_strokeArmor, nar_punctureArmor, nar_thrustArmor FROM NaturalArmor");

        //Löschen der alten Daten
        clearOldTable();

        //Schleife durch Tuples
        for (size_t i = 0; i < rows; ++i) {
            MonsterArmor Armor;
            Armor.strokeArmor = strokeArmor[i];
            Armor.punctureArmor = punctureArmor[i];
            Armor.thrustArmor = thrustArmor[i];
            m_ArmorTable[ raceTyp[i] ] = Armor;
        }

        m_dataOK = true;

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into NaturalArmorTable" << std::endl;
#endif
    } catch (...) {
        m_dataOK = false;
    }

}

bool NaturalArmorTable::find(Character::race_type race, MonsterArmor &ret) {
    TABLE::iterator iterat;
    iterat = m_ArmorTable.find(static_cast<uint16_t>(race));

    if (iterat == m_ArmorTable.end()) {
        return false;
    } else {
        ret = (*iterat).second;
        return true;
    }

    return false;
}

void NaturalArmorTable::clearOldTable() {
    m_ArmorTable.clear();
}


