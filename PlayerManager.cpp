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
#include "World.hpp"
#include "script/LuaLogoutScript.hpp"
#include "Player.hpp"
#include "Logger.hpp"
#include "main_help.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

PlayerManager *PlayerManager::instance;
boost::mutex PlayerManager::mut;
boost::shared_mutex PlayerManager::reloadmutex;

PlayerManager *PlayerManager::get() {
    if (!instance) {
        instance = new PlayerManager();
    }

    return instance;
}

void PlayerManager::activate() {
    std::cout<<"PlayerManager::activate called"<<std::endl;
    pthread_attr_t pattr;
    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);
    running = true;
    std::cout<<"creating login thread:";

    if (pthread_create(&login_thread, &pattr, (void*( *)(void *))&loginLoop, this) != 0) {
        std::cout<<"not sucessfull"<<std::endl;
    } else {
        std::cout<<"successfull"<<std::endl;
    }

    std::cout<<"creating save Loop:";

    if (pthread_create(&save_thread, &pattr, (void*( *)(void *))&playerSaveLoop, this) != 0) {
        std::cout<<"not sucessfull"<<std::endl;
    } else {
        std::cout<<"successfull"<<std::endl;
    }
}

PlayerManager::PlayerManager() {
    std::cout<<"==============creating Player manager==============="<<std::endl;
    running = false;
    threadOk = false;

    /* std::cout<<"creating deleteOldConnection Loop:";
    if ( pthread_create( &deleteOldConnection_thread, &pattr, (void*(*)(void*))&deleteOldConnectionsLoop, this ) != 0 )
         std::cout<<"not sucessfull"<<std::endl;
     else
         std::cout<<"successfull"<<std::endl; */
}

PlayerManager::~PlayerManager() {
    pthread_cancel(login_thread);
    pthread_cancel(save_thread);
    //pthread_cancel( deleteOldConnection_thread );
    //delete incon;
    //shutdownConnections.clear();
}

void PlayerManager::saveAll() {

    if (!loggedOutPlayers.empty()) {
        std::cout<<"forced save:"<<std::endl;

        for (TPLAYERVECTOR::iterator pIterator = loggedOutPlayers.begin(); pIterator != loggedOutPlayers.end(); ++pIterator) {
            // Logout - Vorgang protokollieren
            if (!(*pIterator)->isMonitoringClient()) {
                //( *pIterator )->closeAllShowcases();
                (*pIterator)->save();
                (*pIterator)->Connection->closeConnection();
            }
        }

        std::cout<<"saved all players"<<std::endl;
    }

    std::cout<<"clearing logged out player list"<<std::endl;
    loggedOutPlayers.clear();
    std::cout<<"cleared"<<std::endl;

}

bool PlayerManager::findPlayer(std::string name) {
    mut.lock();

    for (TPLAYERVECTOR::iterator pIterator = loggedOutPlayers.begin(); pIterator != loggedOutPlayers.end(); ++pIterator) {
        // Logout - Vorgang protokollieren
        if ((*pIterator)->name == name) {
            mut.unlock();
            return true;

        }
    }

    mut.unlock();
    return false;
}

void PlayerManager::setLoginLogout(bool val) {
    if (val) {
        //anmelden des exklusiven zugriffs
        reloadmutex.lock_upgrade();
        //bekommen des exklusiven zugriffs
        reloadmutex.unlock_upgrade_and_lock();
    } else {
        //exklusiver zugriff freigeben
        reloadmutex.unlock();
    }
}

void *PlayerManager::loginLoop(PlayerManager *pmanager) {
    try {
        InitialConnection::TVECTORPLAYER &newplayers = pmanager->incon.get_Player_Vector();
        TPLAYERVECTOR::iterator pIterator;
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;
        int curconn = 0;
        boost::shared_ptr<NetInterface> Connection;

        while (pmanager->running) {
            //loop must be steered by counter so we parse every connection only one time bevor we getting to the other loop
            curconn = newplayers.non_block_size();

            for (int i = 0; i < curconn; ++i) {
                std::cout<<"read connection"<<std::endl;
                Connection = newplayers.non_block_pop_front();

                if (Connection) {
                    bool isLocked = false;

                    try {
                        if (!Connection->online) {
                            throw Player::LogoutException(UNSTABLECONNECTION);
                        }

                        if (Connection->receivedSize() > 0) {
                            std::cout<<"try to login player"<<std::endl;
                            //get shared lock
                            reloadmutex.lock_shared();
                            isLocked = true;
                            Player *newPlayer = new Player(Connection);
                            //release shared lock
                            reloadmutex.unlock_shared();
                            isLocked = false;
                            pmanager->loggedInPlayers.non_block_push_back(newPlayer);
                            std::cout<<"added player to list"<<std::endl;
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
                        // Only unlock reloadmutex if it has actually been locked
                        if (isLocked) {
                            reloadmutex.unlock_shared();
                        }

                        std::cout<<"got logoutException durin loading! "<<std::endl;
                        boost::shared_ptr<BasicServerCommand>cmd(new LogOutTC(e.getReason()));
                        Connection->shutdownSend(cmd);
                        //Connection->closeConnection();
                        //Connection.reset();
                        //pmanager->shutdownConnections.non_block_push_back( Connection );
                    }
                } else {
                    std::cout<<"try to get connection but was NULL!"<<std::endl;
                    Connection.reset();
                }

            } // get new players

            nanosleep(&waittime, NULL);
        }
    } catch (std::exception &e) {

    } catch (...) {
        throw;
    }

    return NULL;
}

void *PlayerManager::playerSaveLoop(PlayerManager *pmanager) {
    try {
        World *world = World::get();
        TPLAYERVECTOR::iterator pIterator;
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;
        Player *tmpPl=NULL;

        while (pmanager->running) {
            if (!pmanager->loggedOutPlayers.empty()) {
                Logger::debug(LogFacility::World) << "Ausgelogte Spieler bearbeiten: " << pmanager->loggedOutPlayers.size() << " players gone" << Log::end;

                while (!pmanager->loggedOutPlayers.empty()) {
                    mut.lock();
                    tmpPl = pmanager->loggedOutPlayers.non_block_pop_front();
                    mut.unlock();

                    // Logout - Vorgang protokollieren
                    if (!tmpPl->isMonitoringClient()) {
                        reloadmutex.lock_shared();
                        tmpPl->save();
                        reloadmutex.unlock_shared();
                        tmpPl->Connection->closeConnection();
                        std::cout<<"removed player: "<<tmpPl->name<<std::endl;
                        boost::shared_ptr<BasicServerCommand> cmd(new BBLogOutTC(tmpPl->id, tmpPl->name));
                        world->monitoringClientList->sendCommand(cmd);
                        //std::cout<<"deleting player!"<<std::endl;
                        //pmanager->shutdownConnections.non_block_push_back( tmpPl->Connection);
                        //(*pIterator)->Connection.reset();
                        delete tmpPl;
                        tmpPl=NULL;
                        std::cout<<"deleting end!"<<std::endl;
                    } else {
                        tmpPl->Connection->closeConnection();
                        //std::cout<<"deleting Monitoring client!"<<std::endl;
                        //pmanager->shutdownConnections.non_block_push_back( tmpPl->Connection);
                        //(*pIterator)->Connection.reset();
                        delete tmpPl;
                        tmpPl=NULL;
                        std::cout<<"player deleted"<<std::endl;
                    }
                }

                Logger::debug(LogFacility::World) << "update player list [begin]" << Log::end;
                world->updatePlayerList();
                Logger::debug(LogFacility::World) << "update player list [end]" << Log::end;
            }

            nanosleep(&waittime, NULL);
        }

    } catch (std::exception &e) {

    } catch (...) {
        throw;
    }

    return NULL;
}

//void * PlayerManager::deleteOldConnectionsLoop(PlayerManager * pmanager)
//{
//    try
//    {
//        TPLAYERVECTOR::iterator pIterator;
//        timespec waittime;
//        waittime.tv_sec = 0;
//        waittime.tv_nsec = 100000000;
//        pmanager->threadOk = true;
//        boost::shared_ptr<NetInterface> sd;
//        int size = 0;
//        while ( pmanager->running )
//        {
//               if ( !pmanager->shutdownConnections.non_block_empty() )
//               {
//                   size = 0;
//                   size = pmanager->shutdownConnections.size();
//                   for ( int i = 0; i < size ; ++i )
//                   {
//                        sd = pmanager->shutdownConnections.non_block_pop_front();
//                        if ( sd )
//                        {
//                            if ( sd->shutdownOK() )
//                            {
//                                std::cout<<"try to delete 1 connection to proceed: "<<size<<std::endl;
//                                sd.reset();
//                            }
//                            else
//                            {
//                                pmanager->shutdownConnections.non_block_push_back( sd );
//                            }
//                        }
//                   }
//              }
//             nanosleep(&waittime, NULL);
//        }
//        pmanager->threadOk = false;
//    }
//    catch ( std::exception &e )
//    {
//
//    }
//    catch ( ... )
//    {
//        throw;
//    }
//    return NULL;
//}
