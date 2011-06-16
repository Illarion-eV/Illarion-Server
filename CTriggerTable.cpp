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
#include "CTriggerTable.hpp"
#include "CLogger.hpp"

template<class from>
const std::string toString(const from& convert) {
	std::stringstream stream;
	stream << convert;
	return stream.str();
}


CTriggerTable::CTriggerTable():  _dataOK(false) {
	reload();
}

void CTriggerTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout<<"CTriggerTable: reload!" <<std::endl;
#endif
	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
		std::vector<int16_t> posx,posy,posz;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);
		size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Varchar>(transaction,posx,posy,posz,n_scriptname,
					  "SELECT tgf_posx, tgf_posy, tgf_posz, tgf_script FROM triggerfields");
		for ( size_t i = 0; i < rows; ++i) {
			TriggerStruct Trigger; //new Trigger
			Trigger.pos = position(posx[i],posy[i],posz[i]);
			if (!n_scriptname.var[i]) {
				try {
					// we got a script... load it
					boost::shared_ptr<CLuaTriggerScript> script( new CLuaTriggerScript( scriptname[i], Trigger.pos ) );
					Trigger.script = script;
				} catch (ScriptException &e) {
                    CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
				}
			}
			Triggers.insert(std::pair<position, TriggerStruct>(Trigger.pos,Trigger)); //Zuweisen des Spells
		}
		std::cout << " loadet " << rows << " Triggerfields! " << std::endl;
		_dataOK = true;
	} catch (std::exception &e) {

		std::cerr << "exception: " << e.what() << std::endl;
		_dataOK = false;
	}
}

bool CTriggerTable::find(position pos, TriggerStruct &data) {
	TriggerMap::iterator iterator;
	iterator = Triggers.find(pos);
	if ( iterator == Triggers.end() ) {
		return false;
	} else {
		data = (*iterator).second;
		return true;
	}

}

void CTriggerTable::clearOldTable() {
	Triggers.clear();
}

CTriggerTable::~CTriggerTable() {
	clearOldTable();
}

