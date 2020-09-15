//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "main_help.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/MonsterTable.hpp"
#include "data/RaceTypeTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "netinterface/NetInterface.hpp"
#include "script/server.hpp"

#include <csignal>
#include <memory>
#include <sstream>
#include <string>

std::unique_ptr<ScheduledScriptsTable> scheduledScripts;
std::unique_ptr<MonsterTable> monsterDescriptions;
std::unique_ptr<RaceTypeTable> raceTypes;

// break out of the main loop if false
std::atomic_bool running;

void logout_save(Player *who, bool forced, unsigned long int thistime) {
    time_t acttime = 0;
    time(&acttime);

    thistime = acttime - who->lastsavetime;
    who->onlinetime += thistime;

    static constexpr auto secondsInMinute = 60;
    static constexpr auto secondsInHour = 60 * 60;

    unsigned int th = thistime / secondsInHour;
    unsigned int tm = (thistime % secondsInHour) / secondsInMinute;
    unsigned int ts = (thistime % secondsInHour) % secondsInMinute;

    unsigned int oh = who->onlinetime / secondsInHour;
    unsigned int om = (who->onlinetime % secondsInHour) / secondsInMinute;
    unsigned int os = (who->onlinetime % secondsInHour) % secondsInMinute;

    std::stringstream onlinetime;
    onlinetime << " after " << th << "h " << tm << "m " << ts << "s, onlinetime " << oh << "h " << om << "m " << os
               << "s";

    Logger::info(LogFacility::Player) << (forced ? "forced " : "") << "logout: " << who->Connection->getIPAdress()
                                      << *who << " on " << ctime(&acttime) << onlinetime.str() << Log::end;
}

void login_save(Player *who) {
    time_t acttime = 0;
    time(&acttime);
    static constexpr auto secondsInMinute = 60;
    static constexpr auto secondsInHour = 60 * 60;

    unsigned int oh = who->onlinetime / secondsInHour;
    unsigned int om = (who->onlinetime % secondsInHour) / secondsInMinute;
    unsigned int os = (who->onlinetime % secondsInHour) % secondsInMinute;

    std::stringstream onlinetime;
    onlinetime << " onlinetime till now: " << oh << "h " << om << "m " << os << "s";

    Logger::info(LogFacility::Player) << "login of " << *who << " from " << who->Connection->getIPAdress() << " on "
                                      << ctime(&acttime) << onlinetime.str() << Log::end;
}

// process commandline arguments
auto checkArguments(const std::vector<std::string> &args) -> bool {
    if (args.size() == 2) {
        // config file specified on command line
        if (Config::load(args.at(1))) {
            Logger::info(LogFacility::Other) << "main: using configfile: " << args.at(1) << Log::end;
            return true;
        }

        Logger::error(LogFacility::Other) << "main: error reading configfile: " << args.at(1) << Log::end;

        Logger::error(LogFacility::Other) << "main: USAGE: " << args.at(0) << " configfile" << Log::end;

        return false;
    }

    Logger::error(LogFacility::Other) << "main: invalid commandline arguments" << Log::end;

    Logger::error(LogFacility::Other) << "main: USAGE: " << args.at(0) << " configfile" << Log::end;

    return false;
}

// load item definitions
void loadData() {
    scheduledScripts = std::make_unique<ScheduledScriptsTable>();
    monsterDescriptions = std::make_unique<MonsterTable>();
    raceTypes = std::make_unique<RaceTypeTable>();
    script::server::reload();
}

void sig_term(int /*unused*/) {
    Logger::info(LogFacility::Other) << "SIGTERM received!" << Log::end;

    std::signal(SIGTERM, SIG_IGN); // NOLINT

    World::get()->allowLogin(false);
    running = false;
}

// signal handler for SIGSEGV
void sig_segv(int /*unused*/) {
    Logger::error(LogFacility::Other) << "SIGSEGV received! Last Script: " << World::get()->currentScript->getFileName()
                                      << Log::end;

    std::signal(SIGSEGV, SIG_IGN); // NOLINT
}

// signal handler for SIGUSR1 - Used to reload maps
void sig_usr(int /*unused*/) {
    std::signal(SIGUSR1, SIG_IGN); // NOLINT

    Logger::info(LogFacility::World) << "SIGUSR received! Importing new maps." << Log::end;
    Logger::info(LogFacility::World) << "Disable login and force log out of all players." << Log::end;
    World *world = World::get();
    world->allowLogin(false);
    world->forceLogoutOfAllPlayers();
    world->import();
    world->allowLogin(true);
    Logger::info(LogFacility::World) << "Map import finished." << Log::end;

    std::signal(SIGUSR1, sig_usr);
}

void init_sighandlers() {
    std::signal(SIGPIPE, SIG_IGN); // NOLINT
    std::signal(SIGCHLD, SIG_IGN); // NOLINT
    std::signal(SIGINT, SIG_IGN);  // NOLINT
    std::signal(SIGQUIT, SIG_IGN); // NOLINT

    std::signal(SIGTERM, sig_term);
    std::signal(SIGSEGV, sig_segv);
    std::signal(SIGUSR1, sig_usr);
}

void reset_sighandlers() {
    std::signal(SIGPIPE, SIG_DFL);
    std::signal(SIGTERM, SIG_DFL);
    std::signal(SIGSEGV, SIG_DFL);
}
