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
#include "luabind/luabind.hpp"
#include "TableStructs.hpp"
#include "World.hpp"
#include "TilesTable.hpp"
#include "script/LuaTileScript.hpp"
#include "constants.hpp"
#include <iostream>
#include "Logger.hpp"

TilesTable::TilesTable() : m_dataOK(false) {
	reload();
}


void TilesTable::reload() {
#ifdef DataConnect_DEBUG
	std::cout << "TilesTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<uint8_t> notpassable;
		std::vector<uint8_t> nottransparent;
		std::vector<uint8_t> notpenetrateable;
		std::vector<uint8_t> specialtile;
		std::vector<unsigned char> groundlevel;
		std::vector<std::string> name[3];
		std::vector<TYPE_OF_WALKINGCOST> walkingcost;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);

		size_t rows = di::select_all<
					  di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Varchar, di::Varchar, di::Varchar, di::Integer, di::Varchar
					  >(transaction, ids, notpassable, nottransparent, notpenetrateable, specialtile, groundlevel, name[0], name[1], name[2], walkingcost, n_scriptname,
						"SELECT til_id, til_isnotpassable, til_isnottransparent, til_isnotpenetrateable,"
						"til_specialtile, til_groundlevel, til_german, til_english, til_french,"
						"til_walkingcost, til_script FROM tiles");

		if (rows > 0) {
			clearOldTable();
			for (size_t i = 0; i < rows; ++i) {
                TilesStruct temprecord;
				temprecord.flags = 0;
				temprecord.flags = groundlevel[i];
				temprecord.flags |= notpassable[i] ? FLAG_PASSABLE : 0;
				temprecord.flags |= nottransparent[i] ? FLAG_TRANSPARENT : 0;
				temprecord.flags |= notpenetrateable[i] ? FLAG_PENETRATEABLE : 0;
				temprecord.flags |= specialtile[i]? FLAG_SPECIALTILE : 0;
				temprecord.German = name[0][i];
				temprecord.English = name[1][i];
				temprecord.French = name[2][i];
				temprecord.walkingCost = walkingcost[i];
				if (!n_scriptname.var[i] && scriptname[i] != "") 
                {
					try 
                    {
						boost::shared_ptr<LuaTileScript> script( new LuaTileScript( scriptname[i], temprecord ) );
						temprecord.script = script;
					} 
                    catch (ScriptException &e) 
                    {
                        Logger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
					}
				}
				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;


#ifdef DataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into TilesTable" << std::endl;
#endif

	} catch (std::exception &e) {
		std::cout << "exception in tiles loading: " << e.what() << std::endl;
		m_dataOK = false;
	}


}

bool TilesTable::find( TYPE_OF_ITEM_ID Id, TilesStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}

void TilesTable::clearOldTable() {
	m_table.clear();
}

TilesTable::~TilesTable() {
	clearOldTable();
}
