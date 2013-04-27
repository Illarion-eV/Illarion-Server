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


#include "Player.hpp"
#include "World.hpp"
#include "Character.hpp"
#include "Logger.hpp"
#include "PlayerManager.hpp"
#include "MonitoringClients.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/BBIWIClientCommands.hpp"


MonitoringClients::MonitoringClients() : _world(World::get()) {
}

MonitoringClients::~MonitoringClients() {
    client_list.clear();
}

void MonitoringClients::clientConnect(Player *player) {
    Logger::info(LogFacility::Admin) << "New BBIWI Client connects: " << *player << "; active clients online: " << client_list.size() << Log::end;
    //create new Monitoring Client
    client_list.push_back(player); /*<add a new client to the list*/
    //setup the keepalive
    time(&(player->lastkeepalive));
    //Send all player infos to the new connected client

    _world->Players.for_each([&](Player *p) {
        ServerCommandPointer cmd = std::make_shared<BBPlayerTC>(p->getId(), p->getName(), p->getPosition());
        player->Connection->addCommand(cmd);
        cmd = std::make_shared<BBSendAttribTC>(p->getId(), "hitpoints", p->increaseAttrib("hitpoints",0));
        player->Connection->addCommand(cmd);
        cmd = std::make_shared<BBSendAttribTC>(p->getId(), "mana", p->increaseAttrib("mana",0));
        player->Connection->addCommand(cmd);
        cmd = std::make_shared<BBSendAttribTC>(p->getId(), "foodlevel", p->increaseAttrib("foodlevel",0));
        player->Connection->addCommand(cmd);
    });
}

void MonitoringClients::sendCommand(const ServerCommandPointer &command) {
    for (const auto &client : client_list) {
        client->Connection->addCommand(command);
    }
}


void MonitoringClients::CheckClients() {
    for (auto it = client_list.begin(); it != client_list.end(); ++it) {
        time_t thetime;
        time(&thetime);

        if ((*it)->Connection->online) {
            time_t tempkeepalive;
            time(&tempkeepalive);
            int temptime;
            temptime = tempkeepalive - (*it)->lastkeepalive;

            //check if we have a timeout
            if ((temptime >= 0) && (temptime < 20)) {
                (*it)->workoutCommands();
            } else {
                //timeout so we have to disconnect
                Logger::info(LogFacility::Admin) << "BBIWI Client timed out: " << (*it)->to_string() << Log::end;
                (*it)->Connection->closeConnection();
            }
        } else {
            PlayerManager::get().getLogOutPlayers().non_block_push_back(*it);
            it = client_list.erase(it);
            --it;
        }

        time(&thetime);
    }
}

