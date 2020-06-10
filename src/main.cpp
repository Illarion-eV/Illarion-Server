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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Config.hpp"
#include "InitialConnection.hpp"
#include "Logger.hpp"
#include "MonitoringClients.hpp"
#include "Player.hpp"
#include "PlayerManager.hpp"
#include "World.hpp"
#include "constants.hpp"
#include "data/Data.hpp"
#include "data/ScriptVariablesTable.hpp"
#include "db/ConnectionManager.hpp"
#include "db/SchemaHelper.hpp"
#include "main_help.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaReloadScript.hpp"
#include "tuningConstants.hpp"

#include <memory>
#include <sstream>

extern std::shared_ptr<LuaLoginScript> loginScript;
extern ScriptVariablesTable *scriptVariables;

auto main(int argc, char *argv[]) -> int {
    // get more info for unspecified exceptions
    std::set_terminate(__gnu_cxx::__verbose_terminate_handler);

    Logger::info(LogFacility::Other) << "Starting Illarion!" << Log::end;

    // initialize signalhandlers
    if (!init_sighandlers()) {
        throw std::runtime_error("failed to initialise signal handlers");
    }

    // load configfile
    if (!checkArguments(argc, argv)) {
        throw std::runtime_error("failed to process commandline arguments");
    }

    Logger::info(LogFacility::Other) << "main: server requires clientversion: " << Config::instance().clientversion
                                     << Log::end;
    Logger::info(LogFacility::Other) << "main: listen port: " << Config::instance().port << Log::end;
    Logger::info(LogFacility::Other) << "main: data directory: " << Config::instance().datadir() << Log::end;
    Logger::notice(LogFacility::Script) << "Initialising script log ..." << Log::end;

    // initialise DB Manager
    Database::ConnectionManager::getInstance().setupManager();
    Database::SchemaHelper::setSchemata();

    std::unique_ptr<World> world(World::create());

    Data::preReload();

    if (!Data::Skills.reloadBuffer()) {
        throw std::runtime_error("failed to initialise skills");
    }

    Data::Skills.activateBuffer();

    if (!Data::reloadTables()) {
        throw std::runtime_error("failed to initialise tables");
    }

    Data::activateTables();
    loadData();

    world->Load();

    Data::reloadScripts();

    Logger::info(LogFacility::Other) << "create PlayerManager" << Log::end;
    PlayerManager::get().activate();
    Logger::info(LogFacility::Other) << "PlayerManager activated" << Log::end;
    PlayerManager::TPLAYERVECTOR &newplayers = PlayerManager::get().getLogInPlayers();
    world->initNPC();

    try {
        std::shared_ptr<LuaReloadScript> tmpScript = std::make_shared<LuaReloadScript>("server.reload");
        tmpScript->onReload();
    } catch (ScriptException &e) {
        Logger::critical(LogFacility::Script) << "server.reload failed: " << e.what() << Log::end;
    }

    Logger::info(LogFacility::Other) << "scheduler is being initialised" << Log::end;
    world->initScheduler();

    running = true;

    while (running) {
        // make sure we don't block the server with processing new players...
        int new_players_processed = 0;

        // process new players from connection thread
        while (!newplayers.empty() && new_players_processed < MAXPLAYERSPROCESSED) {
            new_players_processed++;
            Player *newPlayer = newplayers.pop_front();

            if (newPlayer != nullptr) {
                login_save(newPlayer);

                if (newPlayer->isMonitoringClient()) {
                    world->monitoringClientList->clientConnect(newPlayer);
                } else {
                    try {
                        world->Players.insert(newPlayer);
                        newPlayer->login();
                        loginScript->onLogin(newPlayer);
                        world->updatePlayerList();
                    } catch (Player::LogoutException &e) {
                        ServerCommandPointer cmd = std::make_shared<LogOutTC>(e.getReason());
                        newPlayer->Connection->shutdownSend(cmd);
                        PlayerManager::get().getLogOutPlayers().push_back(newPlayer);
                    }
                }
            }
        }

        // run scheduler until next task or for 25ms
        world->scheduler.run_once(std::chrono::seconds(1));
        world->checkPlayerImmediateCommands();
    }

    Logger::info(LogFacility::Other) << "Stopping Illarion!" << Log::end;

    Data::ScriptVariables.save();
    Logger::info(LogFacility::Other) << "ScriptVariables saved!" << Log::end;

    world->forceLogoutOfAllPlayers();
    PlayerManager::get().stop();
    world->takeMonsterAndNPCFromMap();

    world->Save();

    reset_sighandlers();

    Logger::info(LogFacility::Other) << "Illarion has been successfully terminated! " << Log::end;

    return 0;
}
