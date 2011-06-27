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


#ifndef _CLUANPCSCRIPT_HPP
#define _CLUANPCSCRIPT_HPP

#include "LuaScript.hpp"
#include "Character.hpp"
#include "Item.hpp"

class CNPC;
class CWorld;

class CLuaNPCScript : public CLuaScript {
	public:
		CLuaNPCScript(std::string filename, CNPC* thisnpc) throw(ScriptException);

		virtual ~CLuaNPCScript() throw();

		// call script function to see if we make any actions
		void nextCycle();

		// we heard <cc> say <message>
		void receiveText(CCharacter::talk_type tt, std::string message, CCharacter* cc);

		//! NPC is used with anoter Item
		//\param user: User who uses the NPC
		//\ScriptItem: the Item which is used
		//\counter: the counter Value
		//\param: a Param Value for Menus
		void useNPCWithItem(CCharacter * user, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate);

		//! NPC is used with anoter Character
		//\param user: The Person who used the NPC
		//\targetChar: the Character with which the NPC is used
		//\counter: the counter value
		//\param: a param for menus
		void useNPCWithCharacter(CCharacter * user, CCharacter * targetChar, unsigned short counter, unsigned short int param, unsigned char ltastate);

		//! NPC is used with an empty Field
		//\param user: The person who used the NPC
		//\pos: The position of the field which is used
		//\counter: the counter value
		//\param: a param for menus
		void useNPCWithField(CCharacter * user, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate);

		//! NPC is used without any other action
		//\param user: The person who used the NPC
		//\counter: the counter value
		//\param: a param for menus
		void useNPC(CCharacter * user, unsigned short counter, unsigned short int param, unsigned char ltastate);
        
                bool lookAtNpc(CCharacter * source, unsigned char mode);
        
                /**
                 *a longtime action is disturbed by another person
                 */
                bool actionDisturbed(CCharacter * performer, CCharacter * disturber);

                // An NPC loses all its data during a !fr, so we will look if there is anything left to clean up
	        void beforeReload();
			
		//! Ein NPC hat einen Feind entdeckt
		//\param Monster: Das Monster welches den Feind entdeckt hat
		//\param: enemy: der Spieler der als Feind fungiert
		//\ret: true wenn der Scriptaufruf true lieferte damit wird die weitere KI steuerung bis zur nächsten runde ausgesetzt.
		void characterOnSight(CCharacter * npc, CCharacter * enemy);
		
		//! Ein NPC ist auf nahkampfreichweite an einen Feind 
		//\param Monster: das Monster welches auf nahkampfreichweite hinan ist.
		//\param enemy: Der Gegner.
		//\ret: true wenn der Scriptaufruf true lieferte damit wird die weitere KI steuerung bis zur nächsten runde ausgesetzt.
		void characterNear(CCharacter * npc, CCharacter * enemy);
		
		void abortRoute(CCharacter * npc);

	private:
		CNPC* _thisnpc;

		CLuaNPCScript(const CLuaNPCScript&);
		CLuaNPCScript& operator=(const CLuaNPCScript&);
		void init_functions();

};

#endif
