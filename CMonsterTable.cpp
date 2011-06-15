#include "db/ConnectionManager.h"
#include "CMonsterTable.hpp"
#include <iostream>
#include <sstream>
#include "CCommonObjectTable.h"
#include "CWorld.hpp"
#include "CLogger.hpp"

//! table with item attributes
extern CCommonObjectTable* CommonItems;

//! wird von verschiedenen Funktionen als Zwischenvariable genutzt
extern CommonStruct tempCommon;
/*
template<class from>
const std::string toString(const from& convert) {
	std::stringstream stream;
	stream << convert;
	return stream.str();
}*/

CMonsterTable::CMonsterTable() : m_dataOK(false), world(CWorld::get()) 
{
	reload();
}


void CMonsterTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CMonsterTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<std::string> names;
		std::vector<uint16_t> races;
		std::vector<uint16_t> hitpoints;
		std::vector<std::string> movementtype;
		std::vector<bool> canattack;
		std::vector<bool> canheal;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);
		std::vector<uint16_t> minsizes;
		std::vector<uint16_t> maxsizes;

		size_t rows = di::select_all<
					  di::Integer, di::Varchar, di::Integer, di::Integer, di::Varchar, di::Boolean, di::Boolean, di::Varchar, di::Integer, di::Integer
					  >(transaction, ids, names, races, hitpoints, movementtype, canattack, canheal, n_scriptname, minsizes, maxsizes, 
						"SELECT mob_monsterid, mob_name, mob_race, mob_hitpoints, mob_movementtype,"
						"mob_canattack, mob_canhealself, script, mob_minsize, mob_maxsize FROM monster");

		// load data for each of the monsters
		for (size_t i = 0; i < rows; ++i) {
			MonsterStruct temprecord;
			temprecord.name = names[i];
			temprecord.race = (CCharacter::race_type)races[i];
			temprecord.hitpoints = hitpoints[i];
			temprecord.canselfheal = canheal[i];
			temprecord.canattack = canattack[i];
			temprecord.minsize = minsizes[i];
			temprecord.maxsize = maxsizes[i];
			temprecord.movement = CCharacter::walk; // don't need to check for walk since it's default
			if (movementtype[i] == "fly")
				temprecord.movement = CCharacter::fly;
			if (movementtype[i] == "crawl")
				temprecord.movement = CCharacter::crawl;

			if (!n_scriptname.var[i] ) {
				try {
					boost::shared_ptr<CLuaMonsterScript> script(new CLuaMonsterScript( scriptname[i] ) );
					temprecord.script = script;
				} catch (ScriptException &e) {
                    CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
				}

			}

			// load attributes
			std::string query = "SELECT mobattr_name, mobattr_min, mobattr_max FROM monster_attributes WHERE mobattr_monsterid=";
			query += toString(ids[i]);

			std::vector<std::string> attrname;
			std::vector<uint16_t> minmax[2];

			size_t rows2 = di::select_all<
						   di::Varchar, di::Integer, di::Integer
						   >(transaction, attrname, minmax[0], minmax[1], query);

			for (size_t j = 0; j < rows2; ++j) {

				if (attrname[j] == "luck")
					temprecord.attributes.luck = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "strength")
					temprecord.attributes.strength = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "dexterity")
					temprecord.attributes.dexterity = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "constitution")
					temprecord.attributes.constitution = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "agility")
					temprecord.attributes.agility = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "intelligence")
					temprecord.attributes.intelligence = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "perception")
					temprecord.attributes.perception = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "willpower")
					temprecord.attributes.willpower = std::make_pair(minmax[0][j], minmax[1][j]);
				else if (attrname[j] == "essence")
					temprecord.attributes.essence = std::make_pair(minmax[0][j], minmax[1][j]);
				else
					std::cerr << "unknown attribute type: "<< attrname[j] << std::endl;
			}

			// load skills
			std::vector<std::string> skillname;
			minmax[0].clear();
			minmax[1].clear();

			query = "SELECT mobsk_name, mobsk_minvalue, mobsk_maxvalue FROM monster_skills WHERE mobsk_monsterid=";
			query += toString(ids[i]);

			rows2 = di::select_all<
					di::Varchar, di::Integer, di::Integer
					>(transaction, skillname, minmax[0], minmax[1], query);

			for (size_t j = 0; j < rows2; ++j)
				temprecord.skills[skillname[j]] = std::make_pair(minmax[0][j], minmax[1][j]);

			// load items
			query = "SELECT mobit_position, mobit_itemid, mobit_mincount, mobit_maxcount, mobit_propability ";
			query += "FROM monster_items WHERE mobit_monsterid=" + toString(ids[i]);

			std::vector<std::string> positions;
			std::vector<TYPE_OF_ITEM_ID> itemids;
			std::vector<uint16_t> propability;
			minmax[0].clear();
			minmax[1].clear();

			rows2 = di::select_all<
					di::Varchar, di::Integer, di::Integer, di::Integer, di::Integer
					>(transaction, positions, itemids, minmax[0], minmax[1], propability, query);

			itemdef_t tempitem;
			uint16_t temp;
			for (size_t j = 0; j < rows2; ++j) {
				tempitem.itemid = itemids[j];
				tempitem.propability = propability[j];
				tempitem.amount = std::make_pair(minmax[0][j], minmax[1][j]);
				// get position
				if (positions[j] == "head")
					temp = 1;
				else if (positions[j] == "neck")
					temp = 2;
				else if (positions[j] == "breast")
					temp = 3;
				else if (positions[j] == "hands")
					temp = 4;
				else if (positions[j] == "left hand")
					temp = 5;
				else if (positions[j] == "right hand")
					temp = 6;
				else if (positions[j] == "left finger")
					temp = 7;
				else if (positions[j] == "right finger")
					temp = 8;
				else if (positions[j] == "legs")
					temp = 9;
				else if (positions[j] == "feet")
					temp = 10;
				else if (positions[j] == "coat")
					temp = 11;
				else if (positions[j] == "belt1")
					temp = 12;
				else if (positions[j] == "belt2")
					temp = 13;
				else if (positions[j] == "belt3")
					temp = 14;
				else if (positions[j] == "belt4")
					temp = 15;
				else if (positions[j] == "belt5")
					temp = 16;
				else if (positions[j] == "belt6")
					temp = 17;
				else {
					std::cerr << "specified invalid itemslot: " <<  temp << " for monster " << temprecord.name << std::endl;
					temp = 99;
				}

				if (temp < 99 && CommonItems->find( tempitem.itemid, tempCommon)) {
					tempitem.AgeingSpeed = tempCommon.AgeingSpeed;
					temprecord.items[temp].push_back(tempitem);
				} else if (temp < 99) {
					std::cerr << "couldn't find item: " <<  tempitem.itemid << " for monster " << temprecord.name << std::endl;
				}

			}

			m_table[ ids[i] ] = temprecord;
			m_dataOK = true;
		}

#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << m_table.size() << " monsters!" << std::endl;
#endif

	} catch (std::exception e) {
		std::cerr << "exception: " << e.what() << std::endl;
		m_dataOK = false;
	}

}

bool CMonsterTable::find( TYPE_OF_CHARACTER_ID Id, MonsterStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}

void CMonsterTable::clearOldTable() {
	m_table.clear();
}

CMonsterTable::~CMonsterTable() {
	clearOldTable();
}
