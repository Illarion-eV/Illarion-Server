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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iosfwd>
#include <sys/types.h>  // include this before any other sys headers
#include <sys/resource.h>
#include <sys/time.h>

#include <sstream>

#include "Field.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "MapException.hpp"
#include "constants.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "main_help.hpp"
#include "PlayerManager.hpp"
#include "InitialConnection.hpp"
#include "tuningConstants.hpp"
#include "MonitoringClients.hpp"

#include "data/Data.hpp"
#include "data/ScriptVariablesTable.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "db/SchemaHelper.hpp"
#include "db/ConnectionManager.hpp"
#include "db/SchemaHelper.hpp"

#include "script/LuaLoginScript.hpp"
#include "script/LuaReloadScript.hpp"

#include "Statistics.hpp"

extern std::shared_ptr<LuaLoginScript>loginScript;
extern ScriptVariablesTable *scriptVariables;

int main(int argc, char *argv[]) {

    rlimit rlp;
    getrlimit(RLIMIT_CORE, &rlp);
    std::cout<<"max core size: "<<rlp.rlim_cur<<std::endl;
    std::cout<<"current core size: "<<rlp.rlim_max<<std::endl;
    rlp.rlim_cur = 20000;
    int res = getrlimit(RLIMIT_CORE, &rlp);

    if (res < 0) {
        std::cout<<"err: "<<errno<<std::endl;
        perror("setrlimit: RLIMIT_CORE");
        exit(-2);
    }

    // get more info for unspecified exceptions
    std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    // save starting time
    time_t starttime;
    time(&starttime);

    Logger::info(LogFacility::Other) << "Starting Illarion!" << Log::end;

    // initialize signalhandlers
    if (! init_sighandlers()) {
        return 1;
    }

    checkArguments(argc, argv);

    Logger::info(LogFacility::Other) << "main: server requires clientversion: " << Config::instance().clientversion << Log::end;
    Logger::info(LogFacility::Other) << "main: listen port: " << Config::instance().port << Log::end;
    Logger::info(LogFacility::Other) << "main: data directory: " << Config::instance().datadir() << Log::end;
    Logger::notice(LogFacility::Script) << "Initialising script log ..." << Log::end;

    // initialise DB Manager
    Database::ConnectionManager::getInstance().setupManager();
    Database::SchemaHelper::setSchemata();

    World *world = World::create(Config::instance().datadir, starttime);

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
    //Scheduler Initialisieren
    world->initScheduler();

    running = true;

    using namespace Statistic;
    Statistics::getInstance().startTimer("cycle");

    while (running) {
        // make sure we don't block the server with processing new players...
        int new_players_processed = 0;

        // process new players from connection thread
        while (!newplayers.empty() && new_players_processed < MAXPLAYERSPROCESSED) {

            new_players_processed++;
            Player *newPlayer = newplayers.pop_front();

            if (newPlayer) {
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
        Statistics::getInstance().stopTimer("cycle");
    }


    Logger::info(LogFacility::Other) << "Stopping Illarion!" << Log::end;

    Data::ScriptVariables.save();
    Logger::info(LogFacility::Other) << "ScriptVariables saved!" << Log::end;

    world->forceLogoutOfAllPlayers();
    PlayerManager::get().stop();
    world->takeMonsterAndNPCFromMap();

    world->Save();
    delete world;
    world = nullptr;

    reset_sighandlers();

    Logger::info(LogFacility::Other) << "Illarion has been successfully terminated! " << Log::end;

    return EXIT_SUCCESS;
}
