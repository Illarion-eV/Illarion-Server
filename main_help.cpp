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

#include "data/NamesObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "data/ScriptVariablesTable.hpp"
#include <boost/shared_ptr.hpp>
#include "script/LuaWeaponScript.hpp" //For standard fighting script.
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaLookAtItemScript.hpp"
#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "script/LuaLoginScript.hpp"
#include "script/LuaLogoutScript.hpp"
#include "script/LuaLearnScript.hpp"
#include "data/LongTimeEffectTable.hpp"
#include "Connection.hpp"
#include "netinterface/NetInterface.hpp"
#include "data/RaceSizeTable.hpp"
#include "Logger.hpp"
#include "World.hpp"


// a map storing configuration options from a config file...
std::map<std::string, std::string> configOptions;
bool importmaps;

/*
learn, bbiwi, basic, schedscripts, Spawn, World_Debug, World_Imports, World, World_Inits, Monster , Player_Moves, Casting, Use, Use_Scripts
*/
void InitLogOptions() {
    Logger::activateLog("basic");
    Logger::activateLog("World_Inits");
    Logger::activateLog("World");
    //LogOptions["World_Debug"] = true;
}

//! file containing kill logs
std::ofstream kill_log;

//! Die Initialisierung des Servers mit Daten aus einer Datei
bool Init(const std::string &initfile) {
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

        std::stringstream stream;

    }

    return true;
}

#include "Player.hpp"

#include "main_help.hpp"

// in diesen std::vector f�gen Fields die numbers der gel�schten containeritems ein,
//  damit die zugeh�rige Map die containerinhalte l�schen kann
std::vector<int>* erasedcontainers;

// Koordinaten von gel�schten Containern, ben�tigt zum Schlie�en offener Showcases
std::vector<position>* contpos;

//! eine Tabelle mit den Rassenspezifischen Angriffswerten
MonsterAttackTable *MonsterAttacks;

//! eine Tabelle mit Rassenspezifischen R�stwerten
NaturalArmorTable *NaturalArmors;

//! eine Tabelle mit den allgemeinen Attributen der Item
CommonObjectTable *CommonItems;

//! eine Tabelle mit den Namen der Item
NamesObjectTable *ItemNames;

//! eine Tabelle f�r Waffen - Item Daten
WeaponObjectTable *WeaponItems;

//! eine Tabelle f�r Schutzkleidungs - Item Daten
ArmorObjectTable *ArmorItems;

//! eine Tabelle f�r Beh�lter - Item Daten
ContainerObjectTable *ContainerItems;

//! eine Tabelle f�r die Zauberspr�che - Spells
SpellTable *Spells;

//! a Table with Scheduled Scripts
ScheduledScriptsTable *ScheduledScripts;

//! Eine Tabelle mit Triggerfeldern
TriggerTable *Triggers;

//! eine Tabelle mit Item welche die Eigenschaften des Feldes auf dem sie liegen modifizieren
TilesModificatorTable *TilesModItems;

//! eine Tabelle mit allen Arten von Bodenplatten
TilesTable *Tiles;

//! a table containing monster descriptions
MonsterTable *MonsterDescriptions;

//! ein struct f�r die Allgemeinen Attribute eines Item
CommonStruct tempCommon;

//! ein struct f�r die Namen eines Item
NamesStruct tempNames;

//! ein struct f�r Daten einer Waffe
WeaponStruct tempWeapon;

//! ein struct f�r Daten einer Schutzkleidung
ArmorStruct tempArmor;

//! ein struct f�r Daten von Item
TilesModificatorStruct tempModificator;

//! ein struct f�r Daten von Bodenplatten
TilesStruct tempTile;

ScriptVariablesTable *scriptVariables;

LongTimeEffectTable *LongTimeEffects;

RaceSizeTable *RaceSizes;

boost::shared_ptr<LuaDepotScript>depotScript;
boost::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
boost::shared_ptr<LuaLookAtItemScript>lookAtItemScript;
boost::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
boost::shared_ptr<LuaLoginScript>loginScript;
boost::shared_ptr<LuaLogoutScript>logoutScript;
boost::shared_ptr<LuaLearnScript>learnScript;
boost::shared_ptr<LuaWeaponScript> standardFightingScript;

ScheduledScriptsTable *scheduledScripts;  //< table witch holds the scheduled scripts

void logout_save(Player *who, bool forced, unsigned long int thistime) {
    FILE *f;
    f = fopen(configOptions["login_logfile"].c_str(), "at");
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

    if (f != NULL) {
        if (forced) {
            if (who->isAdmin()) {
            } else {
                fprintf(f, "forced logout: %-15s Player %-20s on %s", who->Connection->getIPAdress().c_str(), who->name.c_str(), ctime(&acttime6));
            }
        } else {
            if (who->isAdmin()) {
                fprintf(f, "logout: %-15s Admin  %-20s on %s", who->Connection->getIPAdress().c_str(), who->name.c_str(), ctime(&acttime6));
            } else {
                fprintf(f, "logout: %-15s Player %-20s on %s", who->Connection->getIPAdress().c_str(), who->name.c_str(), ctime(&acttime6));
            }
        }

        fprintf(f, "----- after %5uh %2um %2us, onlinetime %5uh %2um %2us -----\n\n", th, tm, ts, oh, om, os);
        fclose(f);
    } else {
        std::cerr << "main: Konnte nicht in das " << configOptions["login_logfile"] << " File schreiben" << std::endl;
    }
}

void login_save(Player *who) {
    FILE *f;
    f = fopen(configOptions["login_logfile"].c_str(), "at");
    time_t acttime7;
    time(&acttime7);

    unsigned int oh = who->onlinetime / 3600;
    unsigned int om = (who->onlinetime % 3600) / 60;
    unsigned int os = (who->onlinetime % 3600) % 60;

    if (f != NULL) {
        if (who->isAdmin()) {
            std::cout << "main: login Admin, IP: " << who->Connection->getIPAdress() << " Name: " << who->name << " erfolgreich\n" << std::endl;
            fprintf(f, "login:  %-15s Admin  %-20s on %s", who->Connection->getIPAdress().c_str(), who->name.c_str(), ctime(&acttime7));
        } else {
            std::cout << "main: login Player, IP: " << who->Connection->getIPAdress() << " Name: " << who->name << " erfolgreich\n" << std::endl;
            fprintf(f, "login:  %-15s Player %-20s on %s", who->Connection->getIPAdress().c_str(), who->name.c_str(), ctime(&acttime7));
        }

        fprintf(f, "----- onlinetime till now: %5uh %2um %2us -----\n\n", oh, om, os);
        fclose(f);
    } else {
        std::cerr << "main: Konnte nicht in das " << configOptions["login_logfile"] << " File schreiben" << std::endl;
    }
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
    } else if (argc == 3) {
        std::cout<<"main: 3 args for mapimport"<<std::endl;

        if (Init(std::string(argv[ 1 ]))) {
            std::cout << "main: USING CONFIGFILE " << argv[ 1 ] << "\n";
            std::cout << "main: LOADING..." << std::endl;
        } else {
            std::cout << "main: ERROR READING CONFIGFILE " << argv[ 1 ] << " ! " << std::endl;
            std::cout << "main: USAGE: " << argv[0] << " configfile" << std::endl;
            exit(-1);
        }

        if (std::string(argv[ 2 ]) == "-MAPIMPORT") {
            std::cout << "main: IMPORTING MAPS NEW "<<std::endl;
            importmaps = true;
        }
    } else {
        std::cout << "main: USAGE: " << argv[0] << " configfile" << std::endl;
        exit(-1);
    }
}

// Itemdefinitionen laden
void loadData() {
    bool ok = true;

    scriptVariables = new ScriptVariablesTable();

    if (!scriptVariables->isDataOk()) {
        ok = false;
    }

    RaceSizes = new RaceSizeTable();

    if (!RaceSizes->isDataOk()) {
        ok = false;
    }

    NaturalArmors = new NaturalArmorTable();

    if (!NaturalArmors->isDataOk()) {
        ok = false;
    }

    MonsterAttacks = new MonsterAttackTable();

    if (!MonsterAttacks->isDataOk()) {
        ok = false;
    }

    CommonItems = new CommonObjectTable();

    if (!CommonItems->dataOK()) {
        ok = false;
    }

    ItemNames = new NamesObjectTable();

    if (!ItemNames->dataOK()) {
        ok = false;
    }

    WeaponItems = new WeaponObjectTable();

    if (!WeaponItems->dataOK()) {
        ok = false;
    }

    ArmorItems = new ArmorObjectTable();

    if (!ArmorItems->dataOK()) {
        ok = false;
    }

    ContainerItems = new ContainerObjectTable();

    if (!ContainerItems->dataOK()) {
        ok = false;
    }

    TilesModItems = new TilesModificatorTable();

    if (!TilesModItems->dataOK()) {
        ok = false;
    }

    Tiles = new TilesTable();

    if (!Tiles->dataOK()) {
        ok = false;
    }

    Spells = new SpellTable();

    if (!Spells->isDataOK()) {
        ok = false;
    }

    ScheduledScripts = new ScheduledScriptsTable();
    scheduledScripts = ScheduledScripts;

    if (!ScheduledScripts->dataOK()) {
        ok = false;
    }

    Triggers = new TriggerTable();

    if (!Triggers->isDataOK()) {
        ok = false;
    }

    MonsterDescriptions = new MonsterTable();

    if (!MonsterDescriptions->dataOK()) {
        ok=false;
    }

    LongTimeEffects = new LongTimeEffectTable();

    if (!LongTimeEffects->dataOK()) {
        ok = false;
    }

    erasedcontainers = new std::vector<int>;

    contpos= new std::vector<position>;

    try {
        boost::shared_ptr<LuaWeaponScript> tmpScript(new LuaWeaponScript("server.standardfighting"));
        standardFightingScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.standardfighting:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaLookAtPlayerScript>tmpScript(new LuaLookAtPlayerScript("server.playerlookat"));
        lookAtPlayerScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.playerlookat:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaLookAtItemScript>tmpScript(new LuaLookAtItemScript("server.itemlookat"));
        lookAtItemScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.itemlookat:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaPlayerDeathScript>tmpScript(new LuaPlayerDeathScript("server.playerdeath"));
        playerDeathScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.playerdeath:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaDepotScript>tmpScript(new LuaDepotScript("server.depot"));
        depotScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.depot:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaLoginScript>tmpScript(new LuaLoginScript("server.login"));
        loginScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.login:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaLogoutScript>tmpScript(new LuaLogoutScript("server.logout"));
        logoutScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.logout:\n" + std::string(e.what()) + "\n");
    }

    try {
        boost::shared_ptr<LuaLearnScript>tmpScript(new LuaLearnScript("server.learn"));
        learnScript = tmpScript;
    } catch (ScriptException &e) {
        Logger::writeError("scripts", "Error while loading script: server.learn:\n" + std::string(e.what()) + "\n");
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

    configOptions["disable_login"] = "false";
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
    std::cout<<"SIGUSR received Importing maps new !" <<std::endl;
    act_usr.sa_handler = sig_usr;

    std::cout<<"disable login and log out"<<std::endl;
    configOptions["disable_login"] = "true";
    World *world = World::get();
    world->forceLogoutOfAllPlayers(); //Alle spieler ausloggen
    world->maps.clear(); //alte Karten l�schen
    std::cout<<"importing mainland"<<std::endl;
    world->load_from_editor(configOptions["datadir"] + std::string("map/import/oberwelt_0"));
    std::cout<<"loading maps"<<std::endl;
    world->load_maps();
    //alles importiert also noch ein save machen
    std::cout<<"save world"<<std::endl;
    world->Save("Illarion");
    configOptions["disable_login"] = "false";
    std::cout<<"...all done"<<std::endl;

    if (sigaction(SIGUSR1, &act_usr, NULL) < 0) {
        std::cerr << "SIGUSR1: sigaction failed" << std::endl;
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

bool setup_files() {

    configOptions["login_logfile"] = configOptions["logdir"] + configOptions["starttime"] + std::string(".log");
    configOptions["kill_logfile"] = configOptions["logdir"] + configOptions["starttime"] + std::string(".kills");
    configOptions["cout_logfile"] =  configOptions["coutdir"] + configOptions["starttime"] + std::string(".out");
    configOptions["cerr_logfile"] =  configOptions["coutdir"] + configOptions["starttime"] + std::string(".err");

    std::cout << "redirecting all further output to file: " << configOptions["cout_logfile"] << std::endl;
    std::cout << "redirecting error output to file: " << configOptions["cerr_logfile"] << std::endl;

    // open files for redirectings stdout/stderr
    coutfile.open(configOptions["cout_logfile"].c_str(), std::ios::out | std::ios::trunc);
    cerrfile.open(configOptions["cerr_logfile"].c_str(), std::ios::out | std::ios::trunc);

    // check if files are ok
    if (!coutfile.good()) {
        std::cerr << "Could not open stdout log file for writing: " << configOptions["cout_logfile"] << std::endl;
        return false;
    }

    if (!cerrfile.good()) {
        std::cerr << "Could not open stderr log file for writing: " << configOptions["cerr_logfile"] << std::endl;
        return false;
    }

    // redirect stdout/stderr
    std::cout.rdbuf(coutfile.rdbuf());
    std::cerr.rdbuf(cerrfile.rdbuf());

    kill_log.open(configOptions["kill_logfile"].c_str());

    if (!kill_log.good()) {
        std::cerr << "Could not open kill log file for writing: " << configOptions["kill_logfile"] << std::endl;
        return false;
    }

    return true;

}
