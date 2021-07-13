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

#ifndef SCRIPT_SERVER_HPP
#define SCRIPT_SERVER_HPP

#include "script/LuaDepotScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaPlayerTalkScript.hpp"
#include "script/LuaWeaponScript.hpp"

namespace script::server {
auto depot() -> LuaDepotScript &;
auto lookAtPlayer() -> LuaLookAtPlayerScript &;
auto lookAtItem() -> LuaLookAtItemScript &;
auto playerDeath() -> LuaPlayerDeathScript &;
auto playerTalk() -> LuaPlayerTalkScript &;
auto login() -> LuaLoginScript &;
auto logout() -> LuaLogoutScript &;
auto learn() -> LuaLearnScript &;
auto fighting() -> LuaWeaponScript &;

auto reload() -> bool;
} // namespace script::server

#endif
