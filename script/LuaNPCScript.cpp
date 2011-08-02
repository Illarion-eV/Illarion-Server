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

#include "LuaNPCScript.hpp"
#include "NPC.hpp"
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
    callEntrypoint("nextCycle");
}

void LuaNPCScript::lookAtNpc(Character *source, unsigned char mode) {
    init_functions();
    fuse_ptr<Character> fuse_source(source);
    callEntrypoint("lookAtNpc", fuse_source, mode);
}

void LuaNPCScript::receiveText(Character::talk_type tt, std::string message, Character *cc) {
    init_functions();
    fuse_ptr<Character> fuse_cc(cc);
    callEntrypoint("receiveText", (int)tt, message, fuse_cc);
}

void LuaNPCScript::useNPC(Character *user, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useNPC", fuse_user, counter, param, ltastate);
}

void LuaNPCScript::useNPCWithCharacter(Character *user, Character *targetChar, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();
    fuse_ptr<Character> fuse_user(user);
    fuse_ptr<Character> fuse_targetChar(targetChar);
    callEntrypoint("useNPCWithCharacter", fuse_user, fuse_targetChar, counter, param, ltastate);
}

void LuaNPCScript::useNPCWithField(Character *user, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useNPCWithField", fuse_user, pos, counter, param, ltastate);
}

void LuaNPCScript::useNPCWithItem(Character *user, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate) {
    init_functions();
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useNPCWithItem", fuse_user, TargetItem, counter, param, ltastate);
}

bool LuaNPCScript::actionDisturbed(Character *performer, Character *disturber) {
    init_functions();
    fuse_ptr<Character> fuse_performer(performer);
    fuse_ptr<Character> fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

void LuaNPCScript::characterOnSight(Character *npc, Character *enemy) {
    init_functions();
    fuse_ptr<Character> fuse_npc(npc);
    fuse_ptr<Character> fuse_enemy(enemy);
    callEntrypoint("characterOnSight", fuse_npc, fuse_enemy);
}

void LuaNPCScript::characterNear(Character *npc, Character *enemy) {
    init_functions();
    fuse_ptr<Character> fuse_npc(npc);
    fuse_ptr<Character> fuse_enemy(enemy);
    callEntrypoint("characterNear", fuse_npc, fuse_enemy);
}

void LuaNPCScript::abortRoute(Character *npc) {
    init_functions();
    fuse_ptr<Character> fuse_npc(npc);
    callEntrypoint("abortRoute", fuse_npc);
}

void LuaNPCScript::beforeReload() {
    init_functions();
    callEntrypoint("beforeReload");
}

