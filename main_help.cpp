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


#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

#include <sys/types.h>  // include this before any other sys headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "data/MonsterTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include <boost/shared_ptr.hpp>
#include "script/LuaWeaponScript.hpp" //For standard fighting script.
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "Connection.hpp"
#include "netinterface/NetInterface.hpp"
#include "Logger.hpp"
#include "World.hpp"
#include "Config.hpp"


/*
learn, bbiwi, basic, schedscripts, Spawn, World_Debug, World_Imports, World, World_Inits, Monster , Player_Moves, Casting, Use, Use_Scripts
*/

//! Die Initialisierung des Servers mit Daten aus einer Datei
bool Init(const std::string &initfile) {
    return Config::load(initfile);
#if 0
    // first we try to open the file
    std::ifstream configfile(initfile.c_str());

    // can't read config file
    if (!configfile.good()) {
        return false;
    }

    std::string temp;
    char buf[255];

    // read first token of a line while there are any tokens left...
    while (configfile >> temp && ! configfile.eof()) {

        if (temp[0] == '#') {
            // we found a comment... skip line
            configfile.ignore(255,'\n'); // ignore up to 255 chars until \n is found
            continue;
        }

        // store config options in map
        configfile.ignore(1); // ignore the blank char following the token
        configfile.getline(buf, 255, '\n');
        configOptions[temp] = buf;

        if (!configfile.good()) {
            return false;
        }
    }

    return true;
#endif
}

#include "Player.hpp"

#include "main_help.hpp"

// in diesen std::vector f�gen Fields die numbers der gel�schten containeritems ein,
//  damit die zugeh�rige Map die containerinhalte l�schen kann
std::vector<int> *erasedcontainers;

// Koordinaten von gel�schten Containern, ben�tigt zum Schlie�en offener Showcases
std::vector<position> *contpos;

//! a Table with Scheduled Scripts
ScheduledScriptsTable *ScheduledScripts;

//! a table containing monster descriptions
MonsterTable *MonsterDescriptions;

std::shared_ptr<LuaDepotScript>depotScript;
std::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
std::shared_ptr<LuaLookAtItemScript>lookAtItemScript;
std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
std::shared_ptr<LuaLoginScript>loginScript;
std::shared_ptr<LuaLogoutScript>logoutScript;
std::shared_ptr<LuaLearnScript>learnScript;
std::shared_ptr<LuaWeaponScript> standardFightingScript;

ScheduledScriptsTable *scheduledScripts;  //< table witch holds the scheduled scripts

void logout_save(Player *who, bool forced, unsigned long int thistime) {
    time_t acttime6;
    time(&acttime6);

    thistime = acttime6 - who->lastsavetime;
    who->onlinetime += thistime;

    unsigned int th = thistime / 3600;
    unsigned int tm = (thistime % 3600) / 60;
    unsigned int ts = (thistime % 3600) % 60;

    unsigned int oh = who->onlinetime / 3600;
    unsigned int om = (who->onlinetime % 3600) / 60;
    unsigned int os = (who->onlinetime % 3600) % 60;

    std::stringstream onlinetime;
    onlinetime << " after " << th << "h " << tm << "m " << ts << "s, onlinetime " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << (forced?"forced ":"") << "logout: " << who->Connection->getIPAdress() << *who << " on " << ctime(&acttime6) << onlinetime.str() << Log::end;
}

void login_save(Player *who) {
    time_t acttime7;
    time(&acttime7);

    unsigned int oh = who->onlinetime / 3600;
    unsigned int om = (who->onlinetime % 3600) / 60;
    unsigned int os = (who->onlinetime % 3600) % 60;

    std::stringstream onlinetime;
    onlinetime << " onlinetime till now: " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << "login: " << who->Connection->getIPAdress() << *who << " on " << ctime(&acttime7) << onlinetime.str() << Log::end;
}

//! zur Pr�fung der Kommandozeilenargumente
void checkArguments(int argc, char *argv[]) {
    if (argc == 2) {
        // config file specified on command line
        if (Init(std::string(argv[ 1 ]))) {
            std::cout << "main: USING CONFIGFILE " << argv[ 1 ] << "\n";
            std::cout << "main: LOADING..." << std::endl;
        } else {
            std::cout << "main: ERROR READING CONFIGFILE " << argv[ 1 ] << " ! " << std::endl;
            std::cout << "main: USAGE: " << argv[0] << " configfile" << std::endl;
            exit(-1);
        }
    } else {
        std::cout << "main: USAGE: " << argv[0] << " configfile" << std::endl;
        exit(-1);
    }
}

// Itemdefinitionen laden
void loadData() {
    bool ok = true;

    ScheduledScripts = new ScheduledScriptsTable();
    scheduledScripts = ScheduledScripts;

    if (!ScheduledScripts->dataOK()) {
        ok = false;
    }

    MonsterDescriptions = new MonsterTable();

    if (!MonsterDescriptions->dataOK()) {
        ok=false;
    }

    erasedcontainers = new std::vector<int>;

    contpos= new std::vector<position>;

    try {
        std::shared_ptr<LuaWeaponScript> tmpScript(new LuaWeaponScript("server.standardfighting"));
        standardFightingScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.standardfighting: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaLookAtPlayerScript>tmpScript(new LuaLookAtPlayerScript("server.playerlookat"));
        lookAtPlayerScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.playerlookat: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaLookAtItemScript>tmpScript(new LuaLookAtItemScript("server.itemlookat"));
        lookAtItemScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.itemlookat: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaPlayerDeathScript>tmpScript(new LuaPlayerDeathScript("server.playerdeath"));
        playerDeathScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.playerdeath: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaDepotScript>tmpScript(new LuaDepotScript("server.depot"));
        depotScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.depot: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaLoginScript>tmpScript(new LuaLoginScript("server.login"));
        loginScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.login: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaLogoutScript>tmpScript(new LuaLogoutScript("server.logout"));
        logoutScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.logout: " << e.what() << Log::end;
    }

    try {
        std::shared_ptr<LuaLearnScript>tmpScript(new LuaLearnScript("server.learn"));
        learnScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::error(LogFacility::Script) << "Error while loading script: server.learn: " << e.what() << Log::end;
    }


    if (!ok) {
        std::cerr << "could not load tables from database!" << std::endl;
    }
}


////////////////////////////////////////
// signal handling functions
////////////////////////////////////////

//! falls false beendet sich das Programm
volatile bool running;

struct sigaction act_segv, act_segv_o , act_pipe, act_pipe_o, act_term, act_term_o, act_usr;

//! die signal handler for SIGTERM
void sig_term(int) {
    std::cout << "\nSIGTERM received !" << std::endl;
    //  ignore signal
    act_term.sa_handler = SIG_IGN;

    if (sigaction(SIGTERM, &act_term, NULL) < 0) {
        std::cerr << "SIGTERM: sigaction failed" << std::endl;
    }

    World::get()->allowLogin(false);
    running = false;
}

//! die Signalbehandlung f�r SIGSEGV
void sig_segv(int) {
    std::cout << "\nSIGSEGV received !" << std::endl;
    std::cerr <<"SEGV received! last Script: "<<World::get()->currentScript->getFileName()<<std::endl;
    // ignore signal
    act_segv.sa_handler = SIG_IGN;

    if (sigaction(SIGSEGV, &act_segv, NULL) < 0) {
        std::cerr << "SIGSEGV: sigaction failed" << std::endl;
    }

}

void sig_usr(int) {
    Logger::info(LogFacility::World) << "SIGUSR received Importing maps new!" << Log::end;
    act_usr.sa_handler = sig_usr;

    Logger::info(LogFacility::World) << "disable login and force log out of all Players" << Log::end;
    World *world = World::get();
    world->allowLogin(false);
    world->forceLogoutOfAllPlayers(); //Alle spieler ausloggen
    world->maps.clear(); //alte Karten l�schen
    std::cout<<"loading maps"<<std::endl;
    world->load_maps();
    //alles importiert also noch ein save machen
    Logger::info(LogFacility::World) << "Saving World..." << Log::end;
    world->Save("Illarion");
    world->allowLogin(true);
    Logger::info(LogFacility::World) << "Map import finished" << Log::end;

    if (sigaction(SIGUSR1, &act_usr, NULL) < 0) {
        Logger::error(LogFacility::Other) << "SIGUSR1: sigaction failed" << Log::end;
    }

}

bool init_sighandlers() {

    // ignore all signals while installing signal handlers
    if (sigfillset(&act_pipe.sa_mask) < 0) {
        std::cerr << "main: sig..set failed" << std::endl;
        return false;
    }

    // ignore signals
    act_pipe.sa_handler = SIG_IGN;
    act_pipe.sa_flags = SA_RESTART;

    // install signal handlers
    if (sigaction(SIGPIPE, &act_pipe, &act_pipe_o) < 0) {
        std::cerr << "main: sigaction failed" << std::endl;
        return false;
    }

    if (sigaction(SIGCHLD, &act_pipe, NULL) < 0) {
        std::cerr << "main: sigaction failed" << std::endl;
        return false;
    }


    if (sigaction(SIGINT, &act_pipe, NULL) < 0) {
        std::cerr << "main: sigaction failed" << std::endl;
        return false;
    }


    if (sigaction(SIGQUIT, &act_pipe, NULL) < 0) {
        std::cerr << "main: sigaction failed" << std::endl;
        return false;
    }

    // ignore all signals while installing signal handlers
    if (sigfillset(&act_term.sa_mask) < 0) {
        std::cerr << "main: sig..set failed" << std::endl;
        return false;
    }

    act_term.sa_handler = sig_term;
    act_term.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &act_term, &act_term_o) < 0) {
        std::cerr << "main: sigaction SIGTERM failed" << std::endl;
        return false;
    }

    if (sigfillset(&act_segv.sa_mask) < 0) {
        std::cerr << "main: sig..set failed" << std::endl;
        return false;
    }

    act_segv.sa_handler = sig_segv;
    act_segv.sa_flags = SA_RESTART;

    if (sigaction(SIGSEGV, &act_segv, &act_segv_o) < 0) {
        std::cerr << "main: sigaction SIGSEGV failed" << std::endl;
        return false;
    }

    if (sigfillset(&act_usr.sa_mask) < 0) {
        std::cerr << "main: sig..set failed" <<std::endl;
        return false;
    }

    act_usr.sa_handler = sig_usr;
    act_usr.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &act_usr, NULL) < 0) {
        std::cerr << "main: sigaction SIGUSR1 failed" << std::endl;
        return false;
    }


    return true;
}

void reset_sighandlers() {
    std::cout << "reset of signal handlers...";

    sigaction(SIGPIPE, &act_pipe_o, NULL);

    sigaction(SIGTERM, &act_term_o, NULL);

    sigaction(SIGSEGV, &act_segv_o, NULL);

    std::cout << " done.";
}

////////////////////////////////////////
// configuration reading funktions
////////////////////////////////////////

// cout/cerr logfiles
std::ofstream coutfile, cerrfile;

bool setup_files(time_t starttime) {

    std::string cout_logfile = Config::instance().coutdir() + std::to_string(starttime) + std::string(".out");
    std::string cerr_logfile =  Config::instance().coutdir() + std::to_string(starttime) + std::string(".err");

    std::cout << "redirecting all further output to file: " << cout_logfile << std::endl;
    std::cout << "redirecting error output to file: " << cerr_logfile << std::endl;

    // open files for redirectings stdout/stderr
    coutfile.open(cout_logfile.c_str(), std::ios::out | std::ios::trunc);
    cerrfile.open(cerr_logfile.c_str(), std::ios::out | std::ios::trunc);

    // check if files are ok
    if (!coutfile.good()) {
        std::cerr << "Could not open stdout log file for writing: " << cout_logfile << std::endl;
        return false;
    }

    if (!cerrfile.good()) {
        std::cerr << "Could not open stderr log file for writing: " << cerr_logfile << std::endl;
        return false;
    }

    // redirect stdout/stderr
    std::cout.rdbuf(coutfile.rdbuf());
    std::cerr.rdbuf(cerrfile.rdbuf());

    return true;

}
