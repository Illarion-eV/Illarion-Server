#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "db/ConnectionManager.h"

#include <sys/types.h>  // include this before any other sys headers
#include <sys/resource.h>

#include <stdlib.h>
#include <stdio.h>
#include <iosfwd>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>
#include "script/CLuaLoginScript.hpp"
#include "script/CLuaReloadScript.hpp"
#include "CField.hpp"
#include "CPlayer.hpp"
#include "CWorld.hpp"
#include "CMapException.hpp"
#include "constants.h"
#include "CCommonObjectTable.h"
#include "CWeaponObjectTable.h"
#include "CNamesObjectTable.h"
#include "CArmorObjectTable.h"
#include "CContainerObjectTable.h"
#include "CTilesModificatorTable.h"
#include "CTilesTable.h"
#include "CLogger.hpp"
#include "main_help.h"
#include "Random.h"
#include "playersave.hh"
#include "CPlayerManager.hpp"
#include "CScriptVariablesTable.hpp"
#include "InitialConnection.h"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

extern boost::shared_ptr<CLuaLoginScript>loginScript;
extern CScriptVariablesTable * scriptVariables;

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
    
        CLogger::writeMessage("basic", "\nStarte Illarion !" );

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
    
        CLogger::writeMessage("basic", "main: server requires clientversion: " + configOptions["clientversion"], false);
        CLogger::writeMessage("basic", "main: listen port: " + configOptions["port"], false );
        CLogger::writeMessage("basic", "main: data directory: " + configOptions["datadir"], false );

        // initialise DB Manager
        dbmgr = ConnectionManager::CreateConnectionManager(configOptions["postgres_user"],configOptions["postgres_pwd"], configOptions["postgres_db"],configOptions["postgres_host"]);
        accdbmgr = ConnectionManager::CreateConnectionManager(configOptions["postgres_user"],configOptions["postgres_pwd"], configOptions["accountdb"],configOptions["postgres_host"]);
        //Welt anlegen
        CWorld * world = CWorld::create( configOptions["datadir"] , starttime);

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
	CPlayerManager::get()->activate();
	std::cout<<"PlayerManager activated"<<std::endl;
        CPlayerManager::TPLAYERVECTOR & newplayers = CPlayerManager::get()->getLogInPlayers();
        timespec stime;
        stime.tv_sec = 0;
        stime.tv_nsec = 25000000;    //25ms
        //NPC's erschaffen
        world->initNPC();

        //run both reload scripts to initialize semi-dynamic data
        try
        {
            boost::shared_ptr<CLuaReloadScript> tmpScript(new CLuaReloadScript( "server.reload_defs" ));
            if (!tmpScript->onReload()) std::cerr << "server.reload_defs.onReload returned false" << std::endl;
        }
        catch (ScriptException &e)
        {
            std::cerr << "reload_defs: " << e.what() << std::endl;
        }
                
        try
        {
            boost::shared_ptr<CLuaReloadScript> tmpScript(new CLuaReloadScript( "server.reload_tables" ));
            if (!tmpScript->onReload()) std::cerr << "server.reload_tables.onReload returned false" << std::endl;;
        }
        catch (ScriptException &e)
        {
            std::cerr << "reload_tables: " << e.what() << std::endl;
        }

        CLogger::writeMessage("basic","Scheduler wird Initialisiert \n",false);
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
                   CPlayer * newPlayer = newplayers.non_block_pop_front();
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
                           catch ( CPlayer::LogoutException &e )
                           {
                               std::cout<<"got logout Exception during login!"<<std::endl;
                               boost::shared_ptr<CBasicServerCommand> cmd(new CLogOutTC( e.getReason() ));
                               newPlayer->Connection->shutdownSend(cmd);
                               //newPlayer->Connection->closeConnection();
                               CPlayerManager::get()->getLogOutPlayers().non_block_push_back( newPlayer );
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
    

        CLogger::writeMessage("basic","Beende Illarion!");
    
        std::cout<<"Server Shutdown:"<<std::endl;
        
        scriptVariables->save();
        std::cout<<"Scriptvariables saved!"<<std::endl;
        world->forceLogoutOfAllPlayers();

        //saving all players which where forced logged out.
        CPlayerManager::get()->saveAll();

        world->takeMonsterAndNPCFromMap();
    

        CLogger::writeMessage("basic","Statistik aktualisieren");
        CLogger::writeMessage("basic","OnlinePlayer-Liste aktualisieren (-> auf 0)");
        world->saveAllPlayerNamesToFile( configOptions["datadir"] + std::string( ONLINEPLFILE ) );
        CLogger::writeMessage("basic","Karten speichern");
        world->Save( "Illarion" );
        CLogger::writeMessage("basic","InitialConnection beenden");
        CLogger::writeMessage("basic", "Die in loadItems(..) angelegten Tabellen loeschen" );
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
        CLogger::writeMessage("basic","main: Ende " );

        return EXIT_SUCCESS;
}
