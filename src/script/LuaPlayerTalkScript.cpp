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

#include "LuaPlayerTalkScript.hpp"

#include "Character.hpp"
#include "LongTimeAction.hpp"
#include "character_ptr.hpp"

LuaPlayerTalkScript::LuaPlayerTalkScript(const std::string &filename) : LuaScript(filename) {}

auto LuaPlayerTalkScript::talk(Character *player, int talkType, const std::string &message, LtaState actionState)
        -> std::string {
    character_ptr fusePlayer(player);
    return callEntrypoint<std::string>("talk", fusePlayer, talkType, message, static_cast<unsigned char>(actionState));
}

auto LuaPlayerTalkScript::actionDisturbed(Character *performer, Character *disturber) -> bool {
    character_ptr fuse_performer(performer);
    character_ptr fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}
