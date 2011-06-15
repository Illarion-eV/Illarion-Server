#include "db/ConnectionManager.h"
#include "luabind/luabind.hpp"
#include "TableStructs.hpp"
#include "CWorld.hpp"
#include "CTilesTable.h"
#include "script/CLuaTileScript.hpp"
#include "constants.h"
#include <iostream>
#include "CLogger.hpp"

CTilesTable::CTilesTable() : m_dataOK(false) {
	reload();
}


void CTilesTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CTilesTable: reload" << std::endl;
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
						boost::shared_ptr<CLuaTileScript> script( new CLuaTileScript( scriptname[i], temprecord ) );
						temprecord.script = script;
					} 
                    catch (ScriptException &e) 
                    {
                        CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
					}
				}
				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;


#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CTilesTable" << std::endl;
#endif

	} catch (std::exception e) {
		std::cout << "exception in tiles loading: " << e.what() << std::endl;
		m_dataOK = false;
	}


}

bool CTilesTable::find( TYPE_OF_ITEM_ID Id, TilesStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}

void CTilesTable::clearOldTable() {
	m_table.clear();
}

CTilesTable::~CTilesTable() {
	clearOldTable();
}
