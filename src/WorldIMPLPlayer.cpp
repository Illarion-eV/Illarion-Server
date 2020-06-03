/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MonitoringClients.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "db/InsertQuery.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "version.hpp"

#include <regex>

// register any Player commands here...
void World::InitPlayerCommands() {
    PlayerCommands["gm"] = [](World *world, Player *player, const std::string &text) -> bool {
        return world->gmpage_command(player, text);
    };
    PlayerCommands["language"] = [](World *world, Player *player, const std::string &text) -> bool {
        return active_language_command(player, text);
    };
    PlayerCommands["l"] = PlayerCommands["language"];
    PlayerCommands["version"] = [](World *world, Player *player, const std::string & /*unused*/) -> bool {
        version_command(player);
        return true;
    };
    PlayerCommands["v"] = PlayerCommands["version"];
}

//! parse PlayerCommands of the form !<string1> <string2> and process them
auto World::parsePlayerCommands(Player *player, const std::string &text) -> bool {
    return executeUserCommand(player, text, PlayerCommands);
}

// GM page (!gm <text>)
auto World::gmpage_command(Player *player, const std::string &ticket) const -> bool {
    try {
        logGMTicket(player, ticket, false);
        player->inform("--- Die Nachricht wurde an das GM-Team gesendet. ---",
                       "--- The message has been delivered to the GM team. ---");
        return true;
    } catch (...) {
    }

    return false;
}

void World::logGMTicket(Player *player, const std::string &ticket, bool automatic) const {
    using namespace Database;

    InsertQuery insQuery;
    insQuery.setServerTable("gmpager");
    const InsertQuery::columnIndex userColumn = insQuery.addColumn("pager_user");
    const InsertQuery::columnIndex textColumn = insQuery.addColumn("pager_text");
    insQuery.addValue(userColumn, player->getId());
    insQuery.addValue(textColumn, ticket);

    insQuery.execute();

    std::string message;

    if (automatic) {
        message = "Automatic page about " + player->to_string() + ": " + ticket;
    } else {
        message = "Page from " + player->to_string() + ": " + ticket;
    }

    sendMessageToAdmin(message);
    ServerCommandPointer cmd = std::make_shared<BBMessageTC>(message, 2);
    monitoringClientList->sendCommand(cmd);
}

// !language <language>, language=common, human, dwarfen, elven, lizard, orc, ...
auto World::active_language_command(Player *cp, const std::string &language) -> bool {
    if (language == "common") {
        cp->setActiveLanguage(0);
    }

    if (language == "human") {
        cp->setActiveLanguage(1);
    }

    if (language == "dwarf") {
        cp->setActiveLanguage(2);
    }

    if (language == "elf") {
        cp->setActiveLanguage(3);
    }

    if (language == "lizard") {
        cp->setActiveLanguage(4);
    }

    if (language == "orc") {
        cp->setActiveLanguage(5);
    }

    if (language == "halfling") {
        cp->setActiveLanguage(6);
    }

    if (language == "fairy") {
        cp->setActiveLanguage(7);
    }

    if (language == "gnome") {
        cp->setActiveLanguage(8);
    }

    if (language == "goblin") {
        cp->setActiveLanguage(9);
    }

    if (language == "ancient") {
        cp->setActiveLanguage(10);
    }

    return true;
}

void World::version_command(Player *player) { player->inform("Version: " SERVER_VERSION); }
