/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LUA_MAGIC_SCRIPT_
#define _LUA_MAGIC_SCRIPT_

#include "LuaScript.hpp"
#include "TableStructs.hpp"

class World;
class Character;

class LuaMagicScript : public LuaScript {

public:
    LuaMagicScript(std::string filename, unsigned long int MagicFlag) throw(ScriptException);
    virtual ~LuaMagicScript() throw();

    void CastMagic(Character *caster, unsigned char ltastate);
    void CastMagicOnItem(Character *caster, ScriptItem TargetItem, unsigned char ltastate);
    void CastMagicOnCharacter(Character *caster, Character *target, unsigned char ltastate);
    void CastMagicOnField(Character *caster, position pos, unsigned char ltastate);
    bool actionDisturbed(Character *performer, Character *disturber);

private:

    unsigned long int _MagicFlag;
    LuaMagicScript(const LuaMagicScript &);
    LuaMagicScript &operator=(const LuaMagicScript &);

    void init_functions();
};

#endif

