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

#include "LuaNPCScript.hpp"
#include "NPC.hpp"
#include "character_ptr.hpp"

LuaNPCScript::LuaNPCScript(const std::string &filename, NPC *thisnpc) throw(ScriptException)
    : LuaScript(filename), _thisnpc(thisnpc) {
}

LuaNPCScript::~LuaNPCScript() {}

void LuaNPCScript::nextCycle() {
    character_ptr fuse_thisnpc(_thisnpc);
    callEntrypoint("nextCycle", fuse_thisnpc);
}

void LuaNPCScript::lookAtNpc(Character *source, unsigned char mode) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_source(source);
    callEntrypoint("lookAtNpc", fuse_thisnpc, fuse_source, mode);
}

void LuaNPCScript::receiveText(Character::talk_type tt, const std::string &message, Character *cc) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_cc(cc);
    callEntrypoint("receiveText", fuse_thisnpc, (int)tt, message, fuse_cc);
}

void LuaNPCScript::useNPC(Character *user, unsigned char ltastate) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_user(user);
    callEntrypoint("useNPC", fuse_thisnpc, fuse_user, ltastate);
}

bool LuaNPCScript::actionDisturbed(Character *performer, Character *disturber) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_performer(performer);
    character_ptr fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_thisnpc, fuse_performer, fuse_disturber);
}

void LuaNPCScript::characterOnSight(Character *enemy) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_enemy(enemy);
    callEntrypoint("characterOnSight", fuse_thisnpc, fuse_enemy);
}

void LuaNPCScript::characterNear(Character *enemy) {
    character_ptr fuse_thisnpc(_thisnpc);
    character_ptr fuse_enemy(enemy);
    callEntrypoint("characterNear", fuse_thisnpc, fuse_enemy);
}

void LuaNPCScript::abortRoute() {
    character_ptr fuse_thisnpc(_thisnpc);
    callEntrypoint("abortRoute", fuse_thisnpc);
}

void LuaNPCScript::beforeReload() {
    character_ptr fuse_thisnpc(_thisnpc);
    callEntrypoint("beforeReload", fuse_thisnpc);
}

