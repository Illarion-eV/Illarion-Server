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


#include "PlayerManager.hpp"

#include "make_unique.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Logger.hpp"
#include "main_help.hpp"
#include "MonitoringClients.hpp"
#include "LongTimeAction.hpp"

#include "script/LuaLogoutScript.hpp"

#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

std::unique_ptr<PlayerManager> PlayerManager::instance = nullptr;
std::mutex PlayerManager::mut;
std::mutex PlayerManager::reloadmutex;

PlayerManager &PlayerManager::get() {
    if (!instance) {
        instance = std::make_unique<PlayerManager>();
    }

    return *instance;
}

void PlayerManager::activate() {
    running = true;

    login_thread = std::make_unique<std::thread>(loginLoop, this);
    login_thread->detach();
    save_thread = std::make_unique<std::thread>(playerSaveLoop, this);
    save_thread->detach();
}

void PlayerManager::saveAll() {
    if (!loggedOutPlayers.empty()) {
        for (auto const &player : loggedOutPlayers) {
            if (!player->isMonitoringClient()) {
                player->save();
                player->Connection->closeConnection();
            }
        }
    }

    loggedOutPlayers.clear();
}

bool PlayerManager::findPlayer(const std::string &name) const {
    std::lock_guard<std::mutex> lock(mut);

    for (auto const &player : loggedOutPlayers) {
        if (player->getName() == name) {
            return true;
        }
    }

    return false;
}

void PlayerManager::setLoginLogout(bool val) {
    if (val) {
        reloadmutex.lock();
    } else {
        reloadmutex.unlock();
    }
}

void PlayerManager::loginLoop(PlayerManager *pmanager) {
    try {
        auto &newplayers = pmanager->incon.get_Player_Vector();
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;

        while (pmanager->running) {
            //loop must be steered by counter so we parse every connection only one time bevor we getting to the other loop
            int curconn = newplayers.non_block_size();

            for (int i = 0; i < curconn; ++i) {
                auto Connection = newplayers.non_block_pop_front();

                if (Connection) {
                    try {
                        if (!Connection->online) {
                            throw Player::LogoutException(UNSTABLECONNECTION);
                        }

                        if (Connection->receivedSize() > 0) {
                            Player *newPlayer = nullptr;
                            {
                                std::lock_guard<std::mutex> lock(reloadmutex);
                                newPlayer = new Player(Connection);
                            }
                            
                            if (newPlayer) {
                                pmanager->loggedInPlayers.non_block_push_back(newPlayer);
                            }

                            Connection.reset();
                        } else {
                            if (Connection->nextInactive()) {
                                std::cout<<Connection->getIPAdress()<<" logged out because inactive count!"<<std::endl;
                                throw Player::LogoutException(UNSTABLECONNECTION);
                            }

                            newplayers.non_block_push_back(Connection);
                            Connection.reset();
                        }
                    } catch (Player::LogoutException &e) {
                        std::cout<<"got logoutException durin loading! "<<std::endl;
                        ServerCommandPointer cmd = std::make_shared<LogOutTC>(e.getReason());
                        Connection->shutdownSend(cmd);
                    }
                } else {
                    std::cout<<"try to get connection but was nullptr!"<<std::endl;
                    Connection.reset();
                }

            } // get new players

            nanosleep(&waittime, nullptr);
        }
    } catch (std::exception &e) {

    } catch (...) {
        throw;
    }
}

void PlayerManager::playerSaveLoop(PlayerManager *pmanager) {
    try {
        World *world = World::get();
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;
        Player *tmpPl = nullptr;

        while (pmanager->running) {
            if (!pmanager->loggedOutPlayers.empty()) {
                Logger::debug(LogFacility::World) << "Ausgelogte Spieler bearbeiten: " << pmanager->loggedOutPlayers.size() << " players gone" << Log::end;

                while (!pmanager->loggedOutPlayers.empty()) {
                    {
                        std::lock_guard<std::mutex> lock(mut);
                        tmpPl = pmanager->loggedOutPlayers.non_block_pop_front();
                    }

                    if (!tmpPl->isMonitoringClient()) {
                        {
                            std::lock_guard<std::mutex> lock(reloadmutex);
                            tmpPl->save();
                        }
                        tmpPl->Connection->closeConnection();
                        ServerCommandPointer cmd = std::make_shared<BBLogOutTC>(tmpPl->getId());
                        world->monitoringClientList->sendCommand(cmd);
                        delete tmpPl;
                        tmpPl = nullptr;
                    } else {
                        tmpPl->Connection->closeConnection();
                        delete tmpPl;
                        tmpPl = nullptr;
                    }
                }

                Logger::debug(LogFacility::World) << "update player list [begin]" << Log::end;
                world->updatePlayerList();
                Logger::debug(LogFacility::World) << "update player list [end]" << Log::end;
            }

            nanosleep(&waittime, nullptr);
        }

    } catch (std::exception &e) {

    } catch (...) {
        throw;
    }
}

