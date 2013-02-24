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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "db/ConnectionManager.hpp"
#include "db/SchemaHelper.hpp"

#include <sys/types.h>  // include this before any other sys headers
#include <sys/resource.h>

#include <stdlib.h>
#include <stdio.h>
#include <iosfwd>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>
#include "script/LuaLoginScript.hpp"
#include "script/LuaReloadScript.hpp"
#include "Field.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "MapException.hpp"
#include "constants.hpp"
#include "data/Data.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/ScriptVariablesTable.hpp"
#include "Logger.hpp"
#include "main_help.hpp"
#include "playersave.hpp"
#include "PlayerManager.hpp"
#include "InitialConnection.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "db/SchemaHelper.hpp"
#include "Config.hpp"

extern std::shared_ptr<LuaLoginScript>loginScript;
extern ScriptVariablesTable *scriptVariables;

extern bool importmaps;

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

    importmaps = false;
    //di::postgres::enable_trace_query = true;
    // get more info for unspecified exceptions
    std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    // save starting time
    time_t starttime;
    time(&starttime);

    Logger::info(LogFacility::Other) << "Starte Illarion!" << Log::end;

    // initialize signalhandlers
    if (! init_sighandlers()) {
        return 1;
    }

    checkArguments(argc, argv);

    // set up logfiles etc. and check if everything works
    if (! setup_files(starttime)) {
        return 1;
    }

    Logger::info(LogFacility::Other) << "main: server requires clientversion: " << Config::instance().clientversion << Log::end;
    Logger::info(LogFacility::Other) << "main: listen port: " << Config::instance().port << Log::end;
    Logger::info(LogFacility::Other) << "main: data directory: " << Config::instance().datadir << Log::end;

    // initialise DB Manager
    Database::ConnectionManager::getInstance().setupManager();
    Database::SchemaHelper::setSchemata();

    World *world = World::create(Config::instance().datadir, starttime);

    if (!Data::reloadTables()) {
        throw std::runtime_error("failed to initialise tables");
    }

    Data::activateTables();
    loadData();

    if (!importmaps) {
        world->Load("Illarion");
    } else {
	world->allowLogin(false);
        world->load_from_editor(Config::instance().datadir() + std::string("map/import/oberwelt_0"));
    }

    Logger::error(LogFacility::Script) << "Initialising script error log." << Log::end;
    Data::reloadScripts();

    std::cout<<"Creation the PlayerManager"<<std::endl;
    PlayerManager::get()->activate();
    std::cout<<"PlayerManager activated"<<std::endl;
    PlayerManager::TPLAYERVECTOR &newplayers = PlayerManager::get()->getLogInPlayers();
    timespec stime;
    stime.tv_sec = 0;
    stime.tv_nsec = 25000000;
    world->initNPC();

    try {
        std::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript("server.reload"));
        tmpScript->onReload();
    } catch (ScriptException &e) {
        std::cerr << "reload: " << e.what() << std::endl;
    }

    Logger::info(LogFacility::Other) << "Scheduler wird Initialisiert" << Log::end;
    //Scheduler Initialisieren
    world->initScheduler();

    int new_players_processed;

    running = true;
    // die OnlinePlayer-Liste aktualisieren (-> auf 0)
    world->saveAllPlayerNamesToFile(Config::instance().datadir() + std::string(ONLINEPLFILE));

    while (running) {
        // Ausgaben auf std::cout in die Datei schreiben
        std::cout.flush();
        // make sure we don't block the server with processing new players...
        new_players_processed = 0;

        // process new players from connection thread
        while (!newplayers.empty() && new_players_processed < MAXPLAYERSPROCESSED) {

            new_players_processed++;
            Player *newPlayer = newplayers.non_block_pop_front();

            if (newPlayer) {
                login_save(newPlayer);

                if (newPlayer->isMonitoringClient()) {
                    world->monitoringClientList->clientConnect(newPlayer);
                } else {
                    try {
                        std::cout<<"login successful by: "<<newPlayer->name<<" "<<newPlayer->id<<std::endl;
                        world->Players.push_back(newPlayer);
                        newPlayer->login();
                        loginScript->onLogin(newPlayer);
                        world->updatePlayerList();
                    } catch (Player::LogoutException &e) {
                        std::cout<<"got logout Exception during login!"<<std::endl;
                        boost::shared_ptr<BasicServerCommand> cmd(new LogOutTC(e.getReason()));
                        newPlayer->Connection->shutdownSend(cmd);
                        //newPlayer->Connection->closeConnection();
                        PlayerManager::get()->getLogOutPlayers().non_block_push_back(newPlayer);
                    }
                }
            } else {
                std::cout<<"try to get new player but was NULL!"<<std::endl;
            }

        } // get new players

        // Eingaben der Player abarbeiten und die Karte altern
        world->turntheworld();
        nanosleep(&stime, NULL);
    }


    Logger::info(LogFacility::Other) << "Beende Illarion!" << Log::end;

    std::cout<<"Server Shutdown:"<<std::endl;

    scriptVariables->save();
    std::cout<<"Scriptvariables saved!"<<std::endl;
    world->forceLogoutOfAllPlayers();

    //saving all players which where forced logged out.
    PlayerManager::get()->saveAll();

    world->takeMonsterAndNPCFromMap();


    Logger::info(LogFacility::Other) << "Statistik aktualisieren" << Log::end;
    Logger::info(LogFacility::Other) << "OnlinePlayer-Liste aktualisieren (-> auf 0)" << Log::end;
    world->saveAllPlayerNamesToFile(Config::instance().datadir() + std::string(ONLINEPLFILE));
    Logger::info(LogFacility::Other) << "Karten speichern" << Log::end;
    world->Save("Illarion");
    Logger::info(LogFacility::Other) << "InitialConnection beenden" << Log::end;
    Logger::info(LogFacility::Other) << "Die in loadItems(..) angelegten Tabellen loeschen" << Log::end;
    delete CommonItems;
    CommonItems = NULL;
    delete world;
    world = NULL;

    reset_sighandlers();

    time(&starttime);
    Logger::info(LogFacility::Other) << "main: Ende " << Log::end;

    return EXIT_SUCCESS;
}
