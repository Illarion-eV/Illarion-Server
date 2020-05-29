//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "LuaItemScript.hpp"
#include "character_ptr.hpp"
#include "Item.hpp"
#include "Character.hpp"

LuaItemScript::LuaItemScript() : LuaScript() {
    init_functions();
}

LuaItemScript::LuaItemScript(const std::string &filename, const ItemStruct &comstr)
    : LuaScript(filename) , _comstr(comstr) {
    init_functions();
}

LuaItemScript::LuaItemScript(const std::string &code, const std::string &codename, const ItemStruct &comstr)
    : LuaScript(code, codename), _comstr(comstr) {
    init_functions();
}

LuaItemScript::~LuaItemScript() = default;

void LuaItemScript::init_functions() const {
    const luabind::object &globals = luabind::globals(_luaState);
    globals["thisItem"] = _comstr;
}

void LuaItemScript::UseItem(Character *User, const ScriptItem &SourceItem, unsigned char ltastate) {
    character_ptr fuse_User(User);
    callEntrypoint("UseItem", fuse_User, SourceItem, ltastate);
}

auto LuaItemScript::actionDisturbed(Character *performer, Character *disturber) -> bool {
    character_ptr fuse_performer(performer);
    character_ptr fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

auto LuaItemScript::LookAtItem(Character *who, const ScriptItem &t_item) -> ItemLookAt {
    character_ptr fuse_who(who);
    return callEntrypoint<ItemLookAt>("LookAtItem", fuse_who, t_item);
}

auto LuaItemScript::MoveItemBeforeMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem) -> bool {
    character_ptr fuse_who(who);
    return callEntrypoint<bool>("MoveItemBeforeMove", fuse_who, sourceItem, targetItem);
}

void LuaItemScript::MoveItemAfterMove(Character *who, const ScriptItem &sourceItem, const ScriptItem &targetItem) {
    character_ptr fuse_who(who);
    callEntrypoint("MoveItemAfterMove", fuse_who, sourceItem, targetItem);
}

void LuaItemScript::CharacterOnField(Character *who) {
    character_ptr fuse_who(who);
    callEntrypoint("CharacterOnField", fuse_who);
}

