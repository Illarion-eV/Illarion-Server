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


#include "LuaItemScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "Field.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaItemScript::LuaItemScript(std::string filename, CommonStruct comstr) throw(ScriptException)
    : LuaScript(filename) , _comstr(comstr) {
    init_functions();
}

LuaItemScript::~LuaItemScript() throw() {}

void LuaItemScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisItem"] = _comstr;
}

void LuaItemScript::UseItem(Character *User, ScriptItem SourceItem, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","UseItem called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_User(User);
        call("UseItem")(fuse_User, SourceItem, TargetItem, counter, param, ltastate);

    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

bool LuaItemScript::NextCycle() {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","Nextcycle called for: " + Logger::toString(_comstr.id));
        call("NextCycle")();
        return true;
    } catch (luabind::error &e) {
        writeErrorMsg();
        return false;
    }

    return false;
}

void LuaItemScript::UseItemWithCharacter(Character *User, ScriptItem SourceItem, Character *character, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","UseItemWithCharacter called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_User(User);
        fuse_ptr<Character> fuse_character(character);
        call("UseItemWithCharacter")(fuse_User, SourceItem, fuse_character, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

void LuaItemScript::UseItemWithField(Character *User, ScriptItem SourceItem, position TargetPos, unsigned short counter, unsigned short param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","UseItemWithField called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_User(User);
        call("UseItemWithField")(fuse_User, SourceItem, TargetPos, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

bool LuaItemScript::actionDisturbed(Character *performer, Character *disturber) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","actionDisturbed called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_performer(performer);
        fuse_ptr<Character> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")(fuse_performer, fuse_disturber));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("actionDisturbed", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
    return true;
}

bool LuaItemScript::LookAtItem(Character *who, ScriptItem t_item) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","LookAtItem called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_who(who);
        std::pair<QuestScripts::iterator, QuestScripts::iterator> lookAtRange = questScripts.equal_range("LookAtItem");
        QuestScripts::iterator it;
        bool foundQuest = false;

        for (it = lookAtRange.first; it != lookAtRange.second; ++it) {
            foundQuest = foundQuest || luabind::object_cast<bool>(it->second->call("LookAtItem")(fuse_who, t_item));
        }

        if (!foundQuest) {
            call("LookAtItem")(fuse_who, t_item);
        }

        return true;
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("LookAtItem(quest)", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
        return false;
    }

    return false;
}

bool LuaItemScript::MoveItemBeforeMove(Character *who, ScriptItem sourceItem,ScriptItem targetItem) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","MoveItemBeforeMove called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_who(who);
        return luabind::object_cast<bool>(call("MoveItemBeforeMove")(fuse_who, sourceItem, targetItem));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("MoveItemBeforeMove", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }

    return true;
}

void LuaItemScript::MoveItemAfterMove(Character *who, ScriptItem sourceItem, ScriptItem targetItem) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","MoveItemAfterMove called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_who(who);
        call("MoveItemAfterMove")(fuse_who, sourceItem, targetItem);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

void LuaItemScript::CharacterOnField(Character *who) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","CharacterOnField called for: " + Logger::toString(_comstr.id));
        fuse_ptr<Character> fuse_who(who);
        call("CharacterOnField")(fuse_who);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

void LuaItemScript::addQuestScript(const std::string entrypoint, LuaScript *script) {
    boost::shared_ptr<LuaScript> script_ptr(script);
    questScripts.insert(std::pair<const std::string, boost::shared_ptr<LuaScript> >(entrypoint, script_ptr));
}
