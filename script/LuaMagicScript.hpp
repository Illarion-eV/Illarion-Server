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


#ifndef C_LUA_MAGIC_SCRIPT
#define C_LUA_MAGIC_SCRIPT

#include "LuaScript.hpp"
#include "TableStructs.hpp"
#include "globals.hpp"

class World;
class Character;

class LuaMagicScript : public LuaScript {

	public:
		LuaMagicScript(std::string filename, unsigned long int MagicFlag) throw(ScriptException);
		virtual ~LuaMagicScript() throw();

		// Ein Character Zaubert einfach einen Spruch.
		// \param caster, Character welcher den Spruch Castet
		// \param counter, Eingestellter Counter Wert.
		void CastMagic( Character * caster, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf ein Item
		// \param caster, Character welcher den Spruch Castet.
		// \param TargetItem, Ziel des Spruches
		// \param counter, Eingestellter Counter wert.
		void CastMagicOnItem(Character * caster, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf einen anderen Character.
		// \param caster, Character welcher den Spruch Castet.
		// \param TargetCharacter, Ziel auf das gecastet wird
		// \param counter, Eingestellter Counter wert.
		void CastMagicOnCharacter(Character * caster, Character * target, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf ein Freies Feld.
		// \param caster, Character der den Spruch Castet.
		// \param position, Position auf die gecastet wird.
		// \param counter, Eingestellter Counter Wert.
		void CastMagicOnField(Character * caster, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate);
        
        /**
        *a longtime action is disturbed by another person
        */
        bool actionDisturbed(Character * performer, Character * disturber);

	private:

		unsigned long int _MagicFlag;
		LuaMagicScript(const LuaMagicScript&);
		LuaMagicScript& operator=(const LuaMagicScript&);

		void init_functions();
};

#endif
