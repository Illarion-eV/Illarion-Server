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


#include "LuaNPCScript.hpp"
#include "NPC.hpp"
#include "World.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaNPCScript::LuaNPCScript(std::string filename, NPC *thisnpc) throw(ScriptException)
    : LuaScript(filename), _thisnpc(thisnpc) {
    init_functions();
}

LuaNPCScript::~LuaNPCScript() throw() {}

void LuaNPCScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    fuse_ptr<Character> fuse__thisnpc(_thisnpc);
    globals["thisNPC"] = fuse__thisnpc;
}

void LuaNPCScript::nextCycle() {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        call("nextCycle")();
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

bool LuaNPCScript::lookAtNpc(Character *source, unsigned char mode) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        fuse_ptr<Character> fuse_source(source);
        call("lookAtNpc")(fuse_source, mode);
        return true;
    } catch (luabind::error &e) {
        writeErrorMsg();
        return false;
    }
}

// we heard <cc> say <message>
void LuaNPCScript::receiveText(Character::talk_type tt, std::string message, Character *cc) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::receiveText called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::receiveText called for: " + _thisnpc->name);
#endif
        fuse_ptr<Character> fuse_cc(cc);
        call("receiveText")((int)tt, message, fuse_cc);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaNPCScript::receiveText called for: " + _thisnpc->name + " " + e.what(),3));
    }
}

void LuaNPCScript::useNPC(Character *user, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::useNPC called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::useNPC called for: " + _thisnpc->name);
        fuse_ptr<Character> fuse_user(user);
        call("useNPC")(fuse_user, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaNPCScript::useNPC called for: " + _thisnpc->name + " " + e.what(),3));
    }
}

void LuaNPCScript::useNPCWithCharacter(Character *user, Character *targetChar, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name);
        fuse_ptr<Character> fuse_user(user);
        fuse_ptr<Character> fuse_targetChar(targetChar);
        call("useNPCWithCharacter")(fuse_user, fuse_targetChar, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name + " " + e.what(),3));
    }
}

void LuaNPCScript::useNPCWithField(Character *user, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::useNPCWithField called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::useNPCWithField called for: " + _thisnpc->name);
        fuse_ptr<Character> fuse_user(user);
        call("useNPCWithField")(fuse_user, pos, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaNPCScript::useNPCWithField called for: " + _thisnpc->name + " " + e.what(),3));
    }
}

void LuaNPCScript::useNPCWithItem(Character *user, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::useNPCWithItem called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::useNPCWithItem called for: " + _thisnpc->name);
        fuse_ptr<Character> fuse_user(user);
        call("useNPCWithItem")(fuse_user, TargetItem, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaNPCScript::useNPCWithItem called for: " + _thisnpc->name + " " + e.what(),3));
    }
}

bool LuaNPCScript::actionDisturbed(Character *performer, Character *disturber) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaNPCScript::actionDisturbed called for: " + _thisnpc->name,3));
        Logger::writeMessage("scripts","LuaNPCScript::actionDisturbed called for: " + _thisnpc->name);
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

void LuaNPCScript::characterOnSight(Character *npc, Character *enemy) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyOnSight called for: " + Monster->name,3));
        Logger::writeMessage("scripts","characterOnSight called for: " + npc->name);
#endif
        fuse_ptr<Character> fuse_npc(npc);
        fuse_ptr<Character> fuse_enemy(enemy);
        call("characterOnSight")(fuse_npc, fuse_enemy);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyOnSight called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaNPCScript::characterNear(Character *npc, Character *enemy) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyNear called for: " + Monster->name,3));
        Logger::writeMessage("scripts","characterNear called for: " + npc->name);
#endif
        fuse_ptr<Character> fuse_npc(npc);
        fuse_ptr<Character> fuse_enemy(enemy);
        call("characterNear")(fuse_npc, fuse_enemy);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaNPCScript::abortRoute(Character *npc) {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyNear called for: " + Monster->name,3));
        Logger::writeMessage("scripts","abortRoute called for: " + npc->name);
#endif
        fuse_ptr<Character> fuse_npc(npc);
        call("abortRoute")(fuse_npc);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaNPCScript::beforeReload() {
    init_functions();

    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","LuaNPCScript::beforeReload called for: " + _thisnpc->name);
        call("beforeReload")();
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}
