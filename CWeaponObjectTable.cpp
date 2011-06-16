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


#include "db/ConnectionManager.h"
#include <boost/shared_ptr.hpp>
#include "CWorld.hpp"
#include <iostream>
#include "CWeaponObjectTable.h"
#include "script/CLuaWeaponScript.hpp"
#include "CLogger.hpp"

CWeaponObjectTable::CWeaponObjectTable() : m_dataOK(false)
{
	reload();
}


void CWeaponObjectTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CWeaponObjectTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<TYPE_OF_ATTACK> attack;
		std::vector<TYPE_OF_DEFENCE> defence;
		std::vector<TYPE_OF_ACCURACY> accuracy;
		std::vector<TYPE_OF_RANGE> range;
		std::vector<TYPE_OF_WEAPONTYPE> weapontype;
		std::vector<TYPE_OF_AMMUNITIONTYPE> ammunitiontype;
		std::vector<TYPE_OF_ACTIONPOINTS> actionpoints;
		std::vector<TYPE_OF_MAGICDISTURBANCE> magicdisturbance;
		std::vector<TYPE_OF_POISONSTRENGTH> poison;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);

		size_t rows = di::select_all<
					  di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Varchar
					  >(transaction, ids, attack, defence, accuracy, range, weapontype,
						ammunitiontype, actionpoints, magicdisturbance, poison, n_scriptname,
						"SELECT wp_itemid, wp_attack, wp_defence, wp_accuracy, wp_range, wp_weapontype,"
						"wp_ammunitiontype, wp_actionpoints, wp_magicdisturbance, wp_poison, wp_fightingscript FROM weapon");

		if (rows > 0) {
			clearOldTable();
			for (size_t i = 0; i < rows; ++i) {
                WeaponStruct temprecord;
				temprecord.Attack = attack[i];
				temprecord.Defence = defence[i];
				temprecord.Accuracy = accuracy[i];
				temprecord.Range = range[i];
				temprecord.WeaponType = weapontype[i];
				temprecord.AmmunitionType = ammunitiontype[i];
				temprecord.ActionPoints = actionpoints[i];
				temprecord.MagicDisturbance = magicdisturbance[i];
				temprecord.PoisonStrength = poison[i];
				if (!n_scriptname.var[i]) 
                {
					try 
                    {
						boost::shared_ptr<CLuaWeaponScript> tmpScript(new CLuaWeaponScript( scriptname[i] ));
						temprecord.script = tmpScript;
					} 
                    catch(ScriptException &e) 
                    {
                        CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
					}
				}
				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;


#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CWeaponObjectTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}

}

bool CWeaponObjectTable::find( TYPE_OF_ITEM_ID Id, WeaponStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}



void CWeaponObjectTable::clearOldTable() {
	m_table.clear();
}


CWeaponObjectTable::~CWeaponObjectTable() {
	clearOldTable();
}


