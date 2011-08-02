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

#ifndef _LUA_LONG_TIME_EFFECT_SCRIPT_HPP_
#define _LUA_LONG_TIME_EFFECT_SCRIPT_HPP_

#include "LuaScript.hpp"
#include "TableStructs.hpp"

class World;
class Character;
class LongTimeEffect;
class Player;

class LuaLongTimeEffectScript : public LuaScript {
public:
    LuaLongTimeEffectScript(std::string filename, LongTimeEffectStruct effectStruct) throw(ScriptException);
    virtual ~LuaLongTimeEffectScript() throw();

    bool callEffect(LongTimeEffect *effect, Character *target);
    void doubleEffect(LongTimeEffect *effect, Character *target);
    void loadEffect(LongTimeEffect *effect, Character *target);
    void addEffect(LongTimeEffect *effect, Character *target);
    void removeEffect(LongTimeEffect *effect, Character *target);

private:
    LongTimeEffectStruct _effectStruct;
    LuaLongTimeEffectScript(const LuaItemScript &);
    LuaLongTimeEffectScript &operator=(const LuaLongTimeEffectScript &);
    void init_functions();
};

#endif

