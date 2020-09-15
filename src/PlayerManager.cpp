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

#include "PlayerManager.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "MonitoringClients.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "main_help.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ClientCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaLogoutScript.hpp"

#include <chrono>
#include <memory>
#include <range/v3/all.hpp>

std::unique_ptr<PlayerManager> PlayerManager::instance = nullptr;
std::mutex PlayerManager::mut;
std::mutex PlayerManager::reloadmutex;

auto PlayerManager::get() -> PlayerManager & {
    if (!instance) {
        instance = std::make_unique<PlayerManager>();
    }

    return *instance;
}

void PlayerManager::activate() {
    running = true;

    login_thread = std::make_unique<std::thread>(loginLoop, this);
    save_thread = std::make_unique<std::thread>(playerSaveLoop, this);
}

void PlayerManager::stop() {
    running = false;

    Logger::info(LogFacility::Other) << "Waiting for login thread to terminate ..." << Log::end;
    login_thread->join();

    Logger::info(LogFacility::Other) << "Waiting for player save thread to terminate ..." << Log::end;
    save_thread->join();

    Logger::info(LogFacility::Other) << "Player manager terminated!" << Log::end;
}

auto PlayerManager::findPlayer(const std::string &name) const -> bool {
    std::lock_guard<std::mutex> lock(mut);

    using namespace ranges;
    auto hasThisName = [&name](const auto &player) { return player->getName() == name; };
    return any_of(loggedOutPlayers, hasThisName);
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
        auto &newplayers = pmanager->incon->getNewPlayers();
        pmanager->threadOk = true;

        while (pmanager->running) {
            // loop must be steered by counter so we parse every connection only one time bevor we getting to the other
            // loop
            int curconn = newplayers.size();
            unsigned short acceptVersion = Config::instance().clientversion;

            for (int i = 0; i < curconn; ++i) {
                auto Connection = newplayers.pop_front();

                if (Connection) {
                    try {
                        if (!Connection->online) {
                            throw Player::LogoutException(UNSTABLECONNECTION);
                        }

                        auto loginData = Connection->getLoginData();
                        if (loginData != nullptr) {
                            unsigned short int clientversion = loginData->getClientVersion();
                            if (clientversion == BBIWIClientVersion) {
                                // TODO handle login for BBIWI Clients...
                            } else if (clientversion != acceptVersion) {
                                Logger::error(LogFacility::Player)
                                        << loginData->getLoginName() << " tried to login with an old client (version "
                                        << clientversion << ") but version " << acceptVersion << " is required"
                                        << Log::end;
                                throw Player::LogoutException(OLDCLIENT);
                            }

                            // TODO is this check really necessary?
                            if (loginData->getLoginName().empty() || loginData->getPassword().empty()) {
                                throw Player::LogoutException(WRONGPWD);
                            }

                            // player already online?
                            if ((World::get()->Players.find(loginData->getLoginName()) != nullptr) ||
                                PlayerManager::get().findPlayer(loginData->getLoginName())) {
                                Logger::alert(LogFacility::Player)
                                        << loginData->getLoginName()
                                        << " tried to login twice from ip: " << Connection->getIPAdress() << Log::end;
                                throw Player::LogoutException(DOUBLEPLAYER);
                            }

                            Player *newPlayer = nullptr;
                            {
                                std::lock_guard<std::mutex> lock(reloadmutex);
                                newPlayer = new Player(Connection);
                            }

                            pmanager->loggedInPlayers.push_back(newPlayer);
                            World::get()->scheduler.signalNewPlayerAction();

                            Connection.reset();
                        } else {
                            if (Connection->nextInactive()) {
                                throw Player::LogoutException(UNSTABLECONNECTION);
                            }

                            newplayers.push_back(Connection);
                            Connection.reset();
                        }
                    } catch (Player::LogoutException &e) {
                        ServerCommandPointer cmd = std::make_shared<LogOutTC>(e.getReason());
                        Connection->shutdownSend(cmd);
                    }
                } else {
                    Connection.reset();
                }

            } // get new players

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
        }
    } catch (std::exception &e) {
    } catch (...) {
        throw;
    }
}

void PlayerManager::playerSaveLoop(PlayerManager *pmanager) {
    try {
        World *world = World::get();
        pmanager->threadOk = true;
        Player *tmpPl = nullptr;

        while (pmanager->running || !pmanager->loggedOutPlayers.empty()) {
            if (!pmanager->loggedOutPlayers.empty()) {
                while (!pmanager->loggedOutPlayers.empty()) {
                    tmpPl = pmanager->loggedOutPlayers.front();

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

                    std::lock_guard<std::mutex> lock(mut);
                    pmanager->loggedOutPlayers.pop_front();
                }

                Logger::debug(LogFacility::World) << "update player list [begin]" << Log::end;
                world->updatePlayerList();
                Logger::debug(LogFacility::World) << "update player list [end]" << Log::end;
            }

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
        }

    } catch (std::exception &e) {
    } catch (...) {
        throw;
    }
}
