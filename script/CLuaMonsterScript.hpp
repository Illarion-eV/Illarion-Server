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


#ifndef _CLUAMONSTERSCRIPT_HPP
#define _CLUAMONSTERSCRIPT_HPP

#include "CLuaScript.hpp"
#include "CCharacter.hpp"
#include "Item.hpp"

class CWorld;
class CCharacter;

class CLuaMonsterScript : public CLuaScript {
	public:
		CLuaMonsterScript(std::string filename) throw(ScriptException);

		virtual ~CLuaMonsterScript() throw();

		// we heard <cc> say <message>
		void receiveText(CCharacter * Monster, CCharacter::talk_type tt, std::string message, CCharacter* cc);

		//Monster Stirbt.
		//Monster: Das Monster welches Stirbt.
		//Killer: Der Char welcher das Monster getötet hat
		void onDeath(CCharacter * Monster);

		//! Monster wird angegriffen.
		//\param Monster: Monster welches angegriffen wird.
		//\param attacker: Der Char der das Monster angreift.
		void onAttacked(CCharacter * Monster, CCharacter * attacker);

		//! Monster wird bezaubert
		//\param Monster: Monster welches bezaubert wird.
		//\param caster: Char der Zaubert.
		void onCasted(CCharacter * Monster, CCharacter * caster);

		//! Ein Spieler benutzt ein Monster mit einen Item.
		//\param Monster: Das Monster welches benutzt wird.
		//\param user: Der Spieler welcher das Monster benutzt
		//\param item: Das Item welches mit dem Monster verwendet wird
		//\param counter: der eingestellte Counter Wert
		//\param param: Ein übergebener Param wert.
		void useMonsterWithItem(CCharacter * Monster, CCharacter * user, ScriptItem item, unsigned short int counter, unsigned short param, unsigned char ltastate);

		//! Ein Spieler benutzt ein Monster mit einen anderen Character
		//\param Monster: das Monster welches Benutzt wird.
		//\param user: Der Spieler welcher das Monster benutzt.
		//\param targetChar: Der Character der mit den Monster verwendet wird.
		//\param counter: der eingestellte Counter Wert
		//\param param: Ein übergebener Param wert.
		void useMonsterWithCharacter(CCharacter * Monster, CCharacter * user, CCharacter * targetChar, unsigned short int counter, unsigned short param, unsigned char ltastate);

		//! Ein Spieler benutzt ein Monster mit einem Feld
		//\param Monster: das Monster welches Benutzt wird.
		//\param user: der Spieler welcher das Monster benutzt.
		//\param pos: Die Position auf die das Monster verwendet wird.
		//\param counter: der eingestellte Counter Wert
		//\param param: Ein übergebener Param wert.
		void useMonsterWithField(CCharacter * Monster, CCharacter * user, position pos, unsigned short int counter, unsigned short param, unsigned char ltastate);

		//! Ein Spieler benutzt einfach nur ein Monster
		//\param Monster: Das Monster welches Benutzt wird.
		//\param user: der Spieler welcher das Monster benutzt
		//\param counter: Der eingestelle Counter Wert.
		//\param param: Ein übergebener param Wert.
		void useMonster(CCharacter * Monster, CCharacter * user, unsigned short int counter, unsigned short int param, unsigned char ltastate);
        
        /**
        * a player looks at a monster
        * @param source the source character which looks at the monster
        * @param target the target where source looks at
        * @param mode 0 for normal 1 for long
        */
        bool lookAtMonster(CCharacter * source, CCharacter * target, unsigned char mode);
        
        /**
        *a longtime action is disturbed by another person
        */
        bool actionDisturbed(CCharacter * performer, CCharacter * disturber);
		
		//! Ein Monster greift einen Spieler an
		//\param Monster: das Monster welches den Spieler angreift
		//\param target: der Spieler der vom Monster angegriffen wird.
		void onAttack(CCharacter * Monster, CCharacter * target);
		
		//! Ein Monster hat einen Feind entdeckt
		//\param Monster: Das Monster welches den Feind entdeckt hat
		//\param: enemy: der Spieler der als Feind fungiert
		//\ret: true wenn der Scriptaufruf true lieferte damit wird die weitere KI steuerung bis zur nächsten runde ausgesetzt.
		bool enemyOnSight(CCharacter * Monster, CCharacter * enemy);
		
		//! Ein Monster ist auf nahkampfreichweite an einen Feind 
		//\param Monster: das Monster welches auf nahkampfreichweite hinan ist.
		//\param enemy: Der Gegner.
		//\ret: true wenn der Scriptaufruf true lieferte damit wird die weitere KI steuerung bis zur nächsten runde ausgesetzt.
		bool enemyNear(CCharacter * Monster, CCharacter * enemy);
		
		//!ein Monster kann die Route nicht länger verfolgen
		void abortRoute(CCharacter * Monster);
		
		//!Ein Monster wurde gespawned
		void onSpawn(CCharacter * Monster);

          // Auswahl eines Angriffsziels
          bool setTarget( CCharacter * Monster, std::vector<CPlayer *> & CandidateList, CPlayer * &Target );
	private:

		CLuaMonsterScript(const CLuaMonsterScript&);
		CLuaMonsterScript& operator=(const CLuaMonsterScript&);
};
#endif
