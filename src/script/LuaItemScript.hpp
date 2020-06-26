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

#ifndef LUA_ITEM_SCRIPT_HPP
#define LUA_ITEM_SCRIPT_HPP

#include "ItemLookAt.hpp"
#include "LuaScript.hpp"
#include "TableStructs.hpp"

#include <map>
#include <string>

class World;
namespace map {
class Field;
}
class Character;
class ScriptItem;

class LuaItemScript : public LuaScript {
public:
    LuaItemScript();
    LuaItemScript(const std::string &filename, ItemStruct comstr);
    LuaItemScript(const std::string &code, const std::string &codename, ItemStruct comstr);
    ~LuaItemScript() override;
    LuaItemScript(const LuaItemScript &) = delete;
    auto operator=(const LuaItemScript &) -> LuaItemScript & = delete;
    LuaItemScript(LuaItemScript &&) = default;
    auto operator=(LuaItemScript &&) -> LuaItemScript & = default;

    void UseItem(Character *User, const ScriptItem &SourceItem, unsigned char ltastate);
    auto actionDisturbed(Character *performer, Character *disturber) -> bool;
    auto LookAtItem(Character *who, const ScriptItem &t_item) -> ItemLookAt;
    auto MoveItemBeforeMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem) -> bool;
    void MoveItemAfterMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem);
    void CharacterOnField(Character *who);

private:
    ItemStruct _comstr;
    void init_functions() const;
};

#endif
