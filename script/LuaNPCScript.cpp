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
}

LuaNPCScript::~LuaNPCScript() throw() {}

void LuaNPCScript::nextCycle() {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    callEntrypoint("nextCycle", fuse_thisnpc);
}

void LuaNPCScript::lookAtNpc(Character *source, unsigned char mode) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_source(source);
    callEntrypoint("lookAtNpc", fuse_thisnpc, fuse_source, mode);
}

void LuaNPCScript::receiveText(Character::talk_type tt, std::string message, Character *cc) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_cc(cc);
    callEntrypoint("receiveText", fuse_thisnpc, (int)tt, message, fuse_cc);
}

void LuaNPCScript::useNPC(Character *user, unsigned char ltastate) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useNPC", fuse_thisnpc, fuse_user, ltastate);
}

bool LuaNPCScript::actionDisturbed(Character *performer, Character *disturber) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_performer(performer);
    fuse_ptr<Character> fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_thisnpc, fuse_performer, fuse_disturber);
}

void LuaNPCScript::characterOnSight(Character *enemy) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_enemy(enemy);
    callEntrypoint("characterOnSight", fuse_thisnpc, fuse_enemy);
}

void LuaNPCScript::characterNear(Character *enemy) {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    fuse_ptr<Character> fuse_enemy(enemy);
    callEntrypoint("characterNear", fuse_thisnpc, fuse_enemy);
}

void LuaNPCScript::abortRoute() {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    callEntrypoint("abortRoute", fuse_thisnpc);
}

void LuaNPCScript::beforeReload() {
    fuse_ptr<Character> fuse_thisnpc(_thisnpc);
    callEntrypoint("beforeReload", fuse_thisnpc);
}

