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
#include "ArmorObjectTable.hpp"
#include <iostream>

CArmorObjectTable::CArmorObjectTable() : m_dataOK(false) {
	reload();
}

void CArmorObjectTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "ArmorObjectTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<TYPE_OF_BODYPARTS> bodyparts;
		std::vector<TYPE_OF_PUNCTUREARMOR> puncture;
		std::vector<TYPE_OF_STROKEARMOR> stroke;
		std::vector<TYPE_OF_THRUSTARMOR> thrust;
		std::vector<TYPE_OF_MAGICDISTURBANCE> magicdisturbance;
        std::vector<int16_t> absorb;
        std::vector<int16_t> stiffness;
		size_t rows = di::select_all<
        di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer
					  >(transaction, ids, bodyparts, puncture, stroke, thrust, magicdisturbance, absorb, stiffness,
						"SELECT arm_itemid, arm_bodyparts, arm_puncture, arm_stroke, arm_thrust, arm_magicdisturbance, arm_absorb, arm_stiffness FROM armor");

		if (rows > 0) {
			clearOldTable();
			ArmorStruct temprecord;
			for (size_t i = 0; i < rows; ++i) {
				temprecord.BodyParts = bodyparts[i];
				temprecord.PunctureArmor = puncture[i];
				temprecord.StrokeArmor = stroke[i];
				temprecord.ThrustArmor = thrust[i];
				temprecord.MagicDisturbance = magicdisturbance[i];
                temprecord.Absorb = absorb[i];
                temprecord.Stiffness = stiffness[i];
				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;

#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CArmorObjectTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}
}

bool CArmorObjectTable::find( TYPE_OF_ITEM_ID Id, ArmorStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}



void CArmorObjectTable::clearOldTable() {
	m_table.clear();
}


CArmorObjectTable::~CArmorObjectTable() {
	clearOldTable();
}
