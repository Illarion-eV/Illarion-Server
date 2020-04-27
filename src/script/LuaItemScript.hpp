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

#ifndef _LUA_ITEM_SCRIPT_HPP_
#define _LUA_ITEM_SCRIPT_HPP_

#include <map>
#include <string>
#include "LuaScript.hpp"
#include "ItemLookAt.hpp"
#include "TableStructs.hpp"

class World;
class Field;
class Character;
class ScriptItem;

class LuaItemScript : public LuaScript {
public:
    LuaItemScript();
    LuaItemScript(const std::string &filename, const ItemStruct &comstr);
    LuaItemScript(const std::string &code, const std::string &codename, const ItemStruct &comstr);
    virtual ~LuaItemScript();

    void UseItem(Character *User, const ScriptItem &SourceItem, unsigned char ltastate);
    bool actionDisturbed(Character *performer, Character *disturber);
    ItemLookAt LookAtItem(Character *who, const ScriptItem &t_item);
    bool MoveItemBeforeMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem);
    void MoveItemAfterMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem);
    void CharacterOnField(Character *who);

private:
    ItemStruct _comstr;
    LuaItemScript(const LuaItemScript &);
    LuaItemScript &operator=(const LuaItemScript &);
    void init_functions() const;
};

#endif

