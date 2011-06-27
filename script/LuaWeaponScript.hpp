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


#ifndef _CLUAWEAPONSCRIPT_HPP
#define _CLUAWEAPONSCRIPT_HPP

#include "LuaScript.hpp"
#include "Item.hpp"
#include "TableStructs.hpp"

class CWorld;
class CCharacter;

class CLuaWeaponScript : public CLuaScript
{
    public:
        CLuaWeaponScript(std::string filename) throw(ScriptException);
        virtual ~CLuaWeaponScript() throw ();

        //A character attacks another person with this weapon.
        //\param: Attacker, the person who attacks.
        //\param: Defender, the person who is attacked.
        bool onAttack( CCharacter * Attacker, CCharacter * Defender );
        
    private:
	CLuaWeaponScript(const CLuaWeaponScript&);
	CLuaWeaponScript& operator=(const CLuaWeaponScript&);
};
#endif
