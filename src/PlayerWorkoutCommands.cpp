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


#include "World.hpp"

#include "Player.hpp"
#include "tuningConstants.hpp"

#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/ClientCommands.hpp"


void Player::workoutCommands() {
    std::unique_lock<std::mutex> lock(commandMutex);
    while (!immediateCommands.empty()) {
	    ClientCommandPointer cmd = immediateCommands.front();
	    immediateCommands.pop();
	    lock.unlock();
	    cmd->performAction(this);
	    lock.lock();
    }

    while (!queuedCommands.empty() && queuedCommands.front()->getMinAP() <= getActionPoints()) {
	    ClientCommandPointer cmd = queuedCommands.front();
	    immediateCommands.pop();
	    lock.unlock();
	    cmd->performAction(this);
	    lock.lock();
    }
}

void Player::checkFightMode() {
    if (getAttackMode() && canFight()) {
	    //cp->ltAction->abortAction();
	    World::get()->characterAttacks(this);
    }
}

void Player::receiveCommand(ClientCommandPointer cmd) {
	bool notify = false;
	{
		std::unique_lock<std::mutex> lock(commandMutex);
		if (cmd->getMinAP() == 0) {
			immediateCommands.push(cmd);
			notify = true;
		} else {
			if (getActionPoints() > cmd->getMinAP() && queuedCommands.empty())
				notify = true;
			queuedCommands.push(cmd);
		}
	}

	if (notify) {
		World::get()->addPlayerImmediateActionQueue(this);
		World::get()->scheduler.signalNewPlayerAction();
	}
}

