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
#include "CSpellTable.hpp"
#include "CLogger.hpp"

template<class from>
const std::string toString(const from& convert) {
	std::stringstream stream;
	stream << convert;
	return stream.str();
}

CSpellTable::CSpellTable() : _dataOK(false) {
	//im Constructor Daten Reloaden
	reload();
}

void CSpellTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout<<"CSpellTable: reload!" <<std::endl;
#endif
	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
		std::vector<uint32_t> spellid;
		std::vector<uint8_t> magictype;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);
		size_t rows = di::select_all<di::Integer, di::Integer, di::Varchar>(transaction,spellid,magictype,n_scriptname,
					  "SELECT spl_spellid, spl_magictype, spl_scriptname FROM spells");
		for ( size_t i = 0; i < rows; ++i) {
			SpellStruct spell; //new Spell
			spell.magictype = magictype[i];
			if (!n_scriptname.var[i]) {
				try {
					// we got a script... load it
					boost::shared_ptr<CLuaMagicScript> script(new CLuaMagicScript( scriptname[i], spellid[i] ) );
					spell.script = script;
				} catch (ScriptException &e) {
                    CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
				}
			}
			Spells.insert(std::pair<unsigned long int, SpellStruct>(spellid[i],spell)); //Zuweisen des Spells

		}
		std::cout << " loadet " << rows << " magic Scripts! " << std::endl;
		_dataOK = true;
	} catch (std::exception &e) {

		std::cerr << "exception: " << e.what() << std::endl;
		_dataOK = false;
	}
}

bool CSpellTable::find(unsigned long int magicFlag,unsigned short int magic_type, SpellStruct &magicSpell) {
	std::pair<SpellMap::iterator, SpellMap::iterator> range = Spells.equal_range(magicFlag);
	SpellMap::iterator iter;
	//Liste durchlaufen und prüfen ob der Magictype stimmt.
	for ( iter = range.first; iter != range.second; ++iter) {
		if ( (*iter).second.magictype == magic_type) {
			magicSpell = (*iter).second;
			return true;
		}
	}
	return false;
}

void CSpellTable::clearOldTable() {
	Spells.clear();
}

CSpellTable::~CSpellTable() {
	clearOldTable();
}


