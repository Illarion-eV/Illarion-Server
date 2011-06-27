//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "db/ConnectionManager.hpp"

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
#include "CommonObjectTable.hpp"
#include "WeaponObjectTable.hpp"
#include "NamesObjectTable.hpp"
#include "ArmorObjectTable.hpp"
#include "ContainerObjectTable.hpp"
#include "TilesModificatorTable.hpp"
#include "TilesTable.hpp"
#include "Logger.hpp"
#include "main_help.hpp"
#include "Random.hpp"
#include "playersave.hpp"
#include "PlayerManager.hpp"
#include "ScriptVariablesTable.hpp"
#include "InitialConnection.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

extern boost::shared_ptr<LuaLoginScript>loginScript;
extern ScriptVariablesTable * scriptVariables;

extern bool importmaps;

//! Die Hauptfunktion zum Testen //
int main( int argc, char* argv[] ) {
    
    int res;
    rlimit rlp;
    res = getrlimit(RLIMIT_CORE, &rlp);
    std::cout<<"max core size: "<<rlp.rlim_cur<<std::endl;
    std::cout<<"current core size: "<<rlp.rlim_max<<std::endl;
    rlp.rlim_cur = 20000;
    res = getrlimit(RLIMIT_CORE, &rlp);
    if (res < 0) 
    {
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
    time( &starttime );

    std::stringstream ss;
    ss << starttime;
    configOptions["starttime"] = ss.str();
    //Initialize Logging Options
    InitLogOptions();
    
        Logger::writeMessage("basic", "\nStarte Illarion !" );

        // initialize randomizer
        initRandom();

        // initialize signalhandlers
        if ( ! init_sighandlers() )
               return 1;

        // deactivate savethread
        // unsigned long int thisonlinetime = 0;

        checkArguments( argc, argv );


        // set up logfiles etc. and check if everything works
        if (! setup_files() )
               return 1;
    
        Logger::writeMessage("basic", "main: server requires clientversion: " + configOptions["clientversion"], false);
        Logger::writeMessage("basic", "main: listen port: " + configOptions["port"], false );
        Logger::writeMessage("basic", "main: data directory: " + configOptions["datadir"], false );

        // initialise DB Manager
        dbmgr = ConnectionManager::CreateConnectionManager(configOptions["postgres_user"],configOptions["postgres_pwd"], configOptions["postgres_db"],configOptions["postgres_host"]);
        accdbmgr = ConnectionManager::CreateConnectionManager(configOptions["postgres_user"],configOptions["postgres_pwd"], configOptions["accountdb"],configOptions["postgres_host"]);
        //Welt anlegen
        World * world = World::create( configOptions["datadir"] , starttime);

        //Laden der Daten fr die Welt (Items, Scripte, Tabellen etc.)
        loadData();

        if ( !importmaps )
        {
            world->Load( "Illarion" );
        }
        else
        {
            configOptions["disable_login"] = "true";
            world->load_from_editor(configOptions["datadir"] + std::string("map/import/oberwelt_0"));
        }

	std::cout<<"Creation the PlayerManager"<<std::endl;
	PlayerManager::get()->activate();
	std::cout<<"PlayerManager activated"<<std::endl;
        PlayerManager::TPLAYERVECTOR & newplayers = PlayerManager::get()->getLogInPlayers();
        timespec stime;
        stime.tv_sec = 0;
        stime.tv_nsec = 25000000;    //25ms
        //NPC's erschaffen
        world->initNPC();

        //run both reload scripts to initialize semi-dynamic data
        try
        {
            boost::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript( "server.reload_defs" ));
            if (!tmpScript->onReload()) std::cerr << "server.reload_defs.onReload returned false" << std::endl;
        }
        catch (ScriptException &e)
        {
            std::cerr << "reload_defs: " << e.what() << std::endl;
        }
                
        try
        {
            boost::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript( "server.reload_tables" ));
            if (!tmpScript->onReload()) std::cerr << "server.reload_tables.onReload returned false" << std::endl;;
        }
        catch (ScriptException &e)
        {
            std::cerr << "reload_tables: " << e.what() << std::endl;
        }

        Logger::writeMessage("basic","Scheduler wird Initialisiert \n",false);
        //Scheduler Initialisieren
        world->initScheduler();

        int new_players_processed;
        
        running = true;
        // die OnlinePlayer-Liste aktualisieren (-> auf 0)
        world->saveAllPlayerNamesToFile( configOptions["datadir"] + std::string( ONLINEPLFILE ) );

        while ( running ) {
               // Ausgaben auf std::cout in die Datei schreiben
               std::cout.flush();
               // make sure we don't block the server with processing new players...
               new_players_processed = 0;

               // process new players from connection thread
               while (!newplayers.empty() && new_players_processed < MAXPLAYERSPROCESSED) 
               {
                   
                   new_players_processed++;
                   Player * newPlayer = newplayers.non_block_pop_front();
                   if ( newPlayer ) 
                   {
                       login_save(newPlayer);
                       if ( newPlayer->isMonitoringClient() )
                       {
                            world->monitoringClientList->clientConnect( newPlayer );
                       }
                       else
                       {
                           try
                           {
                               std::cout<<"login sucessully from: "<<newPlayer->name<<" "<<newPlayer->id<<std::endl;
                               world->Players.push_back(newPlayer);
                               newPlayer->login();
                               try
                               {
                                   std::cout<<"calling onlogin"<<std::endl;
                                   loginScript->onLogin(newPlayer);
                               }
                               catch (ScriptException &e)
                               {
                                   std::cerr<<"Login Script: Failed to load scripts/login.lua !"<<std::endl;
                               }                               
                               world->updatePlayerList();
                           }
                           catch ( Player::LogoutException &e )
                           {
                               std::cout<<"got logout Exception during login!"<<std::endl;
                               boost::shared_ptr<BasicServerCommand> cmd(new LogOutTC( e.getReason() ));
                               newPlayer->Connection->shutdownSend(cmd);
                               //newPlayer->Connection->closeConnection();
                               PlayerManager::get()->getLogOutPlayers().non_block_push_back( newPlayer );
                           }
                       }
                   }
                   else
                       std::cout<<"try to get new player but was NULL!"<<std::endl;

                } // get new players

               // Eingaben der Player abarbeiten und die Karte altern
               world->turntheworld();
               nanosleep( &stime, NULL );
        }
    

        Logger::writeMessage("basic","Beende Illarion!");
    
        std::cout<<"Server Shutdown:"<<std::endl;
        
        scriptVariables->save();
        std::cout<<"Scriptvariables saved!"<<std::endl;
        world->forceLogoutOfAllPlayers();

        //saving all players which where forced logged out.
        PlayerManager::get()->saveAll();

        world->takeMonsterAndNPCFromMap();
    

        Logger::writeMessage("basic","Statistik aktualisieren");
        Logger::writeMessage("basic","OnlinePlayer-Liste aktualisieren (-> auf 0)");
        world->saveAllPlayerNamesToFile( configOptions["datadir"] + std::string( ONLINEPLFILE ) );
        Logger::writeMessage("basic","Karten speichern");
        world->Save( "Illarion" );
        Logger::writeMessage("basic","InitialConnection beenden");
        Logger::writeMessage("basic", "Die in loadItems(..) angelegten Tabellen loeschen" );
        delete CommonItems;
        CommonItems = NULL;
        delete ItemNames;
        ItemNames = NULL;
        delete WeaponItems;
        WeaponItems = NULL;
        delete ArmorItems;
        ArmorItems = NULL;
        delete ContainerItems;
        ContainerItems = NULL;
        delete TilesModItems;
        TilesModItems = NULL;
        delete Tiles;
        Tiles = NULL;
        delete world;
        world = NULL;

        reset_sighandlers();

        time( &starttime );
        Logger::writeMessage("basic","main: Ende " );

        return EXIT_SUCCESS;
}
