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


#ifndef CLUAITEMSCRIPT_H
#define CLUAITEMSCRIPT_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "LuaScript.hpp"
#include "Item.hpp"
#include "TableStructs.hpp"

class CWorld;
class CField;
class CCharacter;

class CLuaItemScript : public CLuaScript {
	public:
		CLuaItemScript(std::string filename, CommonStruct comstr) throw(ScriptException);
		virtual ~CLuaItemScript() throw();
		
		//Ausführen eines StandardCycles für das Item. (Hauptsächlich für ein RessourcenSystem Um die Ressourcen zu erhöhen)
		bool NextCycle();

		//! Player uses an item or an item with another item.
		//\ param cp player welcher das Item benutzt
		//\ param SourceItem, das Quellitem
		//\ param TargetItem, das Zielitem
		void UseItem(CCharacter * User, ScriptItem SourceItem, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate );

		//! Character benutzt ein Item mit einen leeren Feld.
		//\param user Character der das Item benutzt
		//\SourceItem Item welches benutzt wird
		//\TargetPos position auf die das Item benutzt wird
		//\counter Counter der bei CCharacter eingestellt ist.
		//\param Param Menu des Items
		void UseItemWithField(CCharacter * User, ScriptItem SouceItem, position TargetPos, unsigned short counter, unsigned short int param, unsigned char ltastate );


		//! Spieler benutzt ein Item mit einen anderen Character
		//\ param cp Spieler welcher das Item benutzt
		//\ param SourceItem das Quellitem
		//\ character der Character mit dem das Item verwendet wird.
		void UseItemWithCharacter(CCharacter * User, ScriptItem SourceItem, CCharacter * character, unsigned short counter, unsigned short int param, unsigned char ltastate );
        
        bool actionDisturbed(CCharacter * performer, CCharacter * disturber);

		//! Character schaut sich ein Item an
		//\ param who welcher Character schaut sich ein Item an
		//\ t_item, das entsprechende Item
		//\ pos die Position des Items entweder auf der Karte oder ansonsten die pos von who wenn es eines seiner Items ist
		//\ itempos, wo sich genau das Item befindet. Körper, Rucksack, Container oder Karte.
		bool LookAtItem(CCharacter * who,ScriptItem t_item);

		//! Character verschiebt ein Item wird ausgeführt bevor Item Verschoben wird. Ggf kann verschieben verhindert werden wenn false zurück gegeben wird.
		//\ param who, character der Item verschiebt.
		//\ sourceItem, welches Item verschoben wird.
		//\ tagetpos ggf koordinaten wohin das Item geworfen wird.
		//\ sourceItemPos von wo das Item verschoben wird. Körper, Rucksack, Container, Karte.
		//\ targetItemPos wohin das Item verschoben wird. Körper, Rucksack, Conatiner, Karte.
		bool MoveItemBeforeMove(CCharacter * who, ScriptItem sourceItem, ScriptItem targetItem);

		//! Character verschiebt ein Item wird ausgeführt nachdem Item Verschoben wurde.
		//\ param who, character der Item verschiebt.
		//\ sourceItem, welches Item verschoben wird.
		//\ tagetpos ggf koordinaten wohin das Item geworfen wird.
		//\ sourceItemPos von wo das Item verschoben wird. Körper, Rucksack, Container, Karte.
		//\ targetItemPos wohin das Item verschoben wird. Körper, Rucksack, Conatiner, Karte.
		void MoveItemAfterMove(CCharacter * who, ScriptItem sourceItem, ScriptItem targetItem);

		//! Ein Character befindet  sich auf einen Feld auf dem ein Item liegt.
		//Wird nur ausgeführt wenn das Item den SpecialItem Flag hat. Um unnötige Rechenzeit zu sparen und nicht bei jeden Item auf jeden Feld zu schauen ob die funktion
		//aktiv wird
		//\ param who, Character der auf dem gleichen Feld wie das Item steht.
		void CharacterOnField(CCharacter * who);

        void addQuestScript(const std::string entrypoint, CLuaScript *script);

	private:
		CommonStruct _comstr;
        typedef std::multimap<const std::string, boost::shared_ptr<CLuaScript> > QuestScripts;
        QuestScripts questScripts;
		CLuaItemScript(const CLuaItemScript&);
		CLuaItemScript& operator=(const CLuaItemScript&);
		void init_functions();
};
#endif
