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


#ifndef _CMONITORINGCLIENTS_
#define _CMONITORINGCLIENTS_

#include <list>

#include "netinterface/BasicServerCommand.hpp"

class World;
class Player;

/**
 * class which holds all the monitoring clients on the gameserver
 * and sends all the important server informations to them
 */
class MonitoringClients {
public:
    MonitoringClients();
    ~MonitoringClients();

    /**
     * a new Monitoring client connects to the server
     * @param player a pointer to the player which connects with the monitoring client
     */
    void clientConnect(Player *player);

    /**
     * sends a new Command to all the connected clients
     * @param command the command which should be sended
     */
    void sendCommand(const ServerCommandPointer &command);

    /**
     * function which checks if new commands from clients are arrived and handels them
     */
    void CheckClients();

private:
    std::list<Player *> client_list;
    World *_world;  /*< pointer to the gameworld*/
};
#endif //_CMONITORINGCLIENTS_
