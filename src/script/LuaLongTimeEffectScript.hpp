/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUA_LONG_TIME_EFFECT_SCRIPT_HPP
#define LUA_LONG_TIME_EFFECT_SCRIPT_HPP

#include "LuaScript.hpp"
#include "TableStructs.hpp"

class World;
class Character;
class LongTimeEffect;
class Player;

class LuaLongTimeEffectScript : public LuaScript {
public:
    LuaLongTimeEffectScript(const std::string &filename, LongTimeEffectStruct effectStruct);
    ~LuaLongTimeEffectScript() override;

    auto callEffect(LongTimeEffect *effect, Character *target) -> bool;
    void doubleEffect(LongTimeEffect *effect, Character *target);
    void loadEffect(LongTimeEffect *effect, Character *target);
    void addEffect(LongTimeEffect *effect, Character *target);
    void removeEffect(LongTimeEffect *effect, Character *target);

private:
    LongTimeEffectStruct _effectStruct;
    explicit LuaLongTimeEffectScript(const LuaItemScript &);
    auto operator=(const LuaLongTimeEffectScript &) -> LuaLongTimeEffectScript &;
    void init_functions() const;
};

#endif

