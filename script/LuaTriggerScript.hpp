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


#ifndef LUATRIGGERSCRIPT_HPP
#define LUATRIGGERSCRIPT_HPP

#include "LuaScript.hpp"
#include "globals.hpp"
#include "Item.hpp"

class World;

class LuaTriggerScript: public LuaScript {
	public:
		LuaTriggerScript(std::string filename, position pos) throw(ScriptException);

		virtual ~LuaTriggerScript() throw();

		//Es befindet sich ein Character auf den Feld womit ein Trigger ausgelöst wird, wird mit jeden Servertick
		//neu ausgeführt
		//\param Character der Character der sich auf den Feld befindet
		void CharacterOnField(Character * Character);

		//Ein Character bewegt sich gerade auf das Feld
		//\param Character: der Character der sich auf das Feld bewegt
		void MoveToField(Character * Character);

		//Ein Character bewegt sich von dem Feld herunter
		//\param Character: der Character der sich von dem Feld herunter bewegt
		void MoveFromField(Character * Character);

		//Ein Character legt ein Item auf das Triggerfeld
		//\param item: Das Item welches auf das Feld gelegt wird
		//\param Character: der Character der das Item auf das Feld legt
		void PutItemOnField(ScriptItem item, Character * Character);

		//Ein Character Zerstört ein Item auf einen Feld, Per Script oder indem er es wegnimmt
		//\param item: Das Item welches Zerstört oder weggenommen wird
		//\param Character: der Character der das Item zerstört
		void TakeItemFromField(ScriptItem item, Character * Character);

		//Auf dem Triggerfeld Verrottet ein Gegenstand.
		//\param oldItem: Das item welches Verrottet bevor es verottet.
		//\param newItem: Das Item was nach dem Verotten entstanden ist.
		//ACHTUNG Scriptzugriffe nur über das newitem.
		void ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem);

	private:

		position _pos;
		LuaTriggerScript(const LuaTriggerScript&);
		LuaTriggerScript& operator=(const LuaTriggerScript&);
		void init_functions();
};

#endif
