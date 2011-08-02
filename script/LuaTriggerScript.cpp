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

#include "LuaTriggerScript.hpp"
#include "Character.hpp"
#include "fuse_ptr.hpp"

LuaTriggerScript::LuaTriggerScript(std::string filename, position pos) throw(ScriptException)
    : LuaScript(filename), _pos(pos) {
    init_functions();
}

LuaTriggerScript::~LuaTriggerScript() throw() {}

void LuaTriggerScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisField"] = _pos;
}

void LuaTriggerScript::CharacterOnField(Character *character) {
    fuse_ptr<Character> fuse_character(character);
    callEntrypoint("CharacterOnField", fuse_character);
}

void LuaTriggerScript::ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem) {
    callEntrypoint("ItemRotsOnField", oldItem, newItem);
}

void LuaTriggerScript::MoveFromField(Character *character) {
    fuse_ptr<Character> fuse_character(character);
    callEntrypoint("MoveFromField", fuse_character);
}

void LuaTriggerScript::MoveToField(Character *character) {
    fuse_ptr<Character> fuse_character(character);
    callEntrypoint("MoveToField", fuse_character);
}

void LuaTriggerScript::PutItemOnField(ScriptItem item, Character *character) {
    fuse_ptr<Character> fuse_character(character);
    callEntrypoint("PutItemOnField", item, fuse_character);
}

void LuaTriggerScript::TakeItemFromField(ScriptItem item, Character *character) {
    fuse_ptr<Character> fuse_Character(character);
    callEntrypoint("TakeItemFromField", item, fuse_Character);
}

