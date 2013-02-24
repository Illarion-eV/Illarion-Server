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

#ifndef _LUA_TRIGGER_SCRIPT_HPP_
#define _LUA_TRIGGER_SCRIPT_HPP_

#include "LuaScript.hpp"
#include "Item.hpp"

class World;

class LuaTriggerScript: public LuaScript {
public:
    LuaTriggerScript() throw(ScriptException);
    LuaTriggerScript(std::string filename, position pos) throw(ScriptException);
    virtual ~LuaTriggerScript() throw();

    void CharacterOnField(Character *Character);
    void MoveToField(Character *Character);
    void MoveFromField(Character *Character);
    void PutItemOnField(ScriptItem item, Character *Character);
    void TakeItemFromField(ScriptItem item, Character *Character);
    void ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem);

private:

    position _pos;
    LuaTriggerScript(const LuaTriggerScript &);
    LuaTriggerScript &operator=(const LuaTriggerScript &);
    void init_functions();
};

#endif

