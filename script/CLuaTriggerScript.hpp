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


#ifndef CLUATRIGGERSCRIPT_HPP
#define CLUATRIGGERSCRIPT_HPP

#include "CLuaScript.hpp"
#include "globals.h"
#include "Item.hpp"

class CWorld;

class CLuaTriggerScript: public CLuaScript {
	public:
		CLuaTriggerScript(std::string filename, position pos) throw(ScriptException);

		virtual ~CLuaTriggerScript() throw();

		//Es befindet sich ein Character auf den Feld womit ein Trigger ausgelöst wird, wird mit jeden Servertick
		//neu ausgeführt
		//\param Character der Character der sich auf den Feld befindet
		void CharacterOnField(CCharacter * Character);

		//Ein Character bewegt sich gerade auf das Feld
		//\param Character: der Character der sich auf das Feld bewegt
		void MoveToField(CCharacter * Character);

		//Ein Character bewegt sich von dem Feld herunter
		//\param Character: der Character der sich von dem Feld herunter bewegt
		void MoveFromField(CCharacter * Character);

		//Ein Character legt ein Item auf das Triggerfeld
		//\param item: Das Item welches auf das Feld gelegt wird
		//\param Character: der Character der das Item auf das Feld legt
		void PutItemOnField(ScriptItem item, CCharacter * Character);

		//Ein Character Zerstört ein Item auf einen Feld, Per Script oder indem er es wegnimmt
		//\param item: Das Item welches Zerstört oder weggenommen wird
		//\param Character: der Character der das Item zerstört
		void TakeItemFromField(ScriptItem item, CCharacter * Character);

		//Auf dem Triggerfeld Verrottet ein Gegenstand.
		//\param oldItem: Das item welches Verrottet bevor es verottet.
		//\param newItem: Das Item was nach dem Verotten entstanden ist.
		//ACHTUNG Scriptzugriffe nur über das newitem.
		void ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem);

	private:

		position _pos;
		CLuaTriggerScript(const CLuaTriggerScript&);
		CLuaTriggerScript& operator=(const CLuaTriggerScript&);
		void init_functions();
};

#endif
