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

namespace script::server {

namespace {
std::unique_ptr<LuaDepotScript> depotScript;
std::unique_ptr<LuaLookAtPlayerScript> lookAtPlayerScript;
std::unique_ptr<LuaLookAtItemScript> lookAtItemScript;
std::unique_ptr<LuaPlayerDeathScript> playerDeathScript;
std::unique_ptr<LuaLoginScript> loginScript;
std::unique_ptr<LuaLogoutScript> logoutScript;
std::unique_ptr<LuaLearnScript> learnScript;
std::unique_ptr<LuaWeaponScript> standardFightingScript;
} // namespace

auto depot() -> LuaDepotScript & { return *depotScript; }
auto lookAtPlayer() -> LuaLookAtPlayerScript & { return *lookAtPlayerScript; }
auto lookAtItem() -> LuaLookAtItemScript & { return *lookAtItemScript; }
auto playerDeath() -> LuaPlayerDeathScript & { return *playerDeathScript; }
auto login() -> LuaLoginScript & { return *loginScript; }
auto logout() -> LuaLogoutScript & { return *logoutScript; }
auto learn() -> LuaLearnScript & { return *learnScript; }
auto fighting() -> LuaWeaponScript & { return *standardFightingScript; }

auto reload() -> bool {
    bool success = true;

    try {
        standardFightingScript = std::make_unique<LuaWeaponScript>("server.standardfighting");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.standardfighting: " << e.what() << Log::end;
    }

    try {
        lookAtPlayerScript = std::make_unique<LuaLookAtPlayerScript>("server.playerlookat");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.playerlookat: " << e.what() << Log::end;
    }

    try {
        lookAtItemScript = std::make_unique<LuaLookAtItemScript>("server.itemlookat");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: server.itemlookat: " << e.what() << Log::end;
    }

    try {
        playerDeathScript = std::make_unique<LuaPlayerDeathScript>("server.playerdeath");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script)
                << "Error while loading script: server.playerdeath: " << e.what() << Log::end;
    }

    try {
        depotScript = std::make_unique<LuaDepotScript>("server.depot");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: server.depot: " << e.what() << Log::end;
    }

    try {
        loginScript = std::make_unique<LuaLoginScript>("server.login");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: server.login: " << e.what() << Log::end;
    }

    try {
        logoutScript = std::make_unique<LuaLogoutScript>("server.logout");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: server.logout: " << e.what() << Log::end;
    }

    try {
        learnScript = std::make_unique<LuaLearnScript>("server.learn");
    } catch (ScriptException &e) {
        success = false;
        Logger::error(LogFacility::Script) << "Error while loading script: server.learn: " << e.what() << Log::end;
    }

    return success;
}
} // namespace script::server
