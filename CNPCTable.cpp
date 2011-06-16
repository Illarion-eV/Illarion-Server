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
#include "CNPCTable.h"
#include "CLogger.hpp"

template< typename To, typename From> To stream_convert( const From& from ) {
	std::stringstream stream;
	stream << from;
	To to;
	stream >> to;
	return to;
}

CNPCTable::CNPCTable() : _world(CWorld::get())
{
	//Constructor welche die Funktion Loaddata auslöst
	LoadData();
}

CNPCTable::~CNPCTable() {}

bool CNPCTable::LoadData() {
	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_CHARACTER_ID> ids;
		std::vector<int> pos[3];
		std::vector<short> direction;
		std::vector<short> sex;
		std::vector<int> type;
		std::vector<bool> healer;
		std::vector<std::string> name;
		std::vector<std::string> scriptname;
		di::isnull_vector<std::vector<std::string> > n_scriptname(scriptname);
        std::vector<uint8_t> hair;
        std::vector<uint8_t> beard;
        std::vector<uint8_t> hairred;
        std::vector<uint8_t> hairgreen;
        std::vector<uint8_t> hairblue;
        std::vector<uint8_t> skinred;
        std::vector<uint8_t> skingreen;
        std::vector<uint8_t> skinblue;

		size_t rows = di::select_all<
					  di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Varchar, di::Integer, di::Boolean, di::Varchar, di::Integer,
                      di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer
					  >(transaction, ids, type, pos[0], pos[1], pos[2], name, direction, healer, n_scriptname, sex,
                        hair, beard, hairred, hairgreen, hairblue, skinred, skingreen, skinblue,
						"SELECT npc_id, npc_type, npc_posx, npc_posy, npc_posz, npc_name,"
						"npc_faceto, npc_is_healer, npc_script, npc_sex, npc_hair, npc_beard,"
                        "npc_hairred, npc_hairgreen, npc_hairblue, npc_skinred, npc_skingreen, npc_skinblue FROM npc");

		std::vector<struct NPCTalk> texts;

		for (size_t i = 0; i < rows; ++i) {
			try {
				CNPC* newNPC = new CNPC(ids[i], name[i], (CCharacter::race_type)type[i], position(pos[0][i],pos[1][i], pos[2][i]),
										(CCharacter::face_to)direction[i], healer[i],(CCharacter::sex_type)sex[i],
                                        hair[i], beard[i], hairred[i], hairgreen[i], hairblue[i], skinred[i], skingreen[i], skinblue[i]);
				// add npc to npc list
				_world->Npc.push_back(newNPC);

				// set field to occupied
				CField* tempf;
				if ( _world->GetPToCFieldAt( tempf, pos[0][i], pos[1][i], pos[2][i] ) ) {
					tempf->setChar();
				}

				if (!n_scriptname.var[i]) {
					try {
						// we got a script... load it
						boost::shared_ptr<CLuaNPCScript> script(new CLuaNPCScript( scriptname[i], newNPC ));
						newNPC->setScript(script);
					} catch (ScriptException &e) {
                        CLogger::writeError( "scripts", "Error while loading script: " + scriptname[i] + ":\n" + e.what() + "\n" );
					}
				}
			} catch (NoSpace &s) {
				std::cout << "no space available for npc: " << ids[i] << " : " << s.what() << std::endl;
			}
		}

		return true;
	} catch (...) {
		std::cerr << "error while loading npcs" << std::endl;
		return false;
	}
}
