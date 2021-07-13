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

#include "script/server.hpp"

#include "Logger.hpp"

#include <memory>
#include <string>

namespace script::server {

namespace {
std::unique_ptr<LuaDepotScript> depotScript;
std::unique_ptr<LuaLookAtPlayerScript> lookAtPlayerScript;
std::unique_ptr<LuaLookAtItemScript> lookAtItemScript;
std::unique_ptr<LuaPlayerDeathScript> playerDeathScript;
std::unique_ptr<LuaPlayerTalkScript> playerTalkScript;
std::unique_ptr<LuaLoginScript> loginScript;
std::unique_ptr<LuaLogoutScript> logoutScript;
std::unique_ptr<LuaLearnScript> learnScript;
std::unique_ptr<LuaWeaponScript> standardFightingScript;
} // namespace

auto depot() -> LuaDepotScript & { return *depotScript; }
auto lookAtPlayer() -> LuaLookAtPlayerScript & { return *lookAtPlayerScript; }
auto lookAtItem() -> LuaLookAtItemScript & { return *lookAtItemScript; }
auto playerDeath() -> LuaPlayerDeathScript & { return *playerDeathScript; }
auto playerTalk() -> LuaPlayerTalkScript & { return *playerTalkScript; }
auto login() -> LuaLoginScript & { return *loginScript; }
auto logout() -> LuaLogoutScript & { return *logoutScript; }
auto learn() -> LuaLearnScript & { return *learnScript; }
auto fighting() -> LuaWeaponScript & { return *standardFightingScript; }

template <typename T> auto loadScript(std::unique_ptr<T> &script, const std::string &file) -> bool {
    bool success = true;

    try {
        script = std::make_unique<T>(file);
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: " << file << ": " << e.what() << Log::end;
    }

    return success;
}

auto reload() -> bool {
    bool success = true;

    success = success && loadScript(standardFightingScript, "server.standardfighting");
    success = success && loadScript(lookAtPlayerScript, "server.playerlookat");
    success = success && loadScript(lookAtItemScript, "server.itemlookat");
    success = success && loadScript(playerDeathScript, "server.playerdeath");
    success = success && loadScript(playerTalkScript, "server.playertalk");
    success = success && loadScript(depotScript, "server.depot");
    success = success && loadScript(loginScript, "server.login");
    success = success && loadScript(logoutScript, "server.logout");
    success = success && loadScript(learnScript, "server.learn");

    return success;
}
} // namespace script::server
