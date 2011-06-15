#include "CPlayerManager.hpp"
#include "CWorld.hpp"
#include "script/CLuaLogoutScript.hpp"
#include "CPlayer.hpp"
#include "CLogger.hpp"
#include "main_help.h"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

CPlayerManager * CPlayerManager::instance;
boost::mutex CPlayerManager::mut;
boost::shared_mutex CPlayerManager::reloadmutex;

CPlayerManager * CPlayerManager::get()
{
    if ( !instance )instance = new CPlayerManager();
    return instance;
}
    
void CPlayerManager::activate()
{
    std::cout<<"CPlayerManager::activate called"<<std::endl;	
    pthread_attr_t pattr;
    pthread_attr_init( &pattr );
    pthread_attr_setdetachstate( &pattr, PTHREAD_CREATE_DETACHED );
    running = true;
    std::cout<<"creating login thread:";
    if ( pthread_create( &login_thread, &pattr, (void*(*)(void*))&loginLoop, this ) != 0 )
        std::cout<<"not sucessfull"<<std::endl;
    else
        std::cout<<"successfull"<<std::endl;
    std::cout<<"creating save Loop:";
    if ( pthread_create( &save_thread, &pattr, (void*(*)(void*))&playerSaveLoop, this ) != 0 )
        std::cout<<"not sucessfull"<<std::endl;
    else
        std::cout<<"successfull"<<std::endl;  
}
    
CPlayerManager::CPlayerManager()
{
    std::cout<<"==============creating Player manager==============="<<std::endl;
    running = false;
    threadOk = false;
    
   /* std::cout<<"creating deleteOldConnection Loop:";
   if ( pthread_create( &deleteOldConnection_thread, &pattr, (void*(*)(void*))&deleteOldConnectionsLoop, this ) != 0 )
        std::cout<<"not sucessfull"<<std::endl;
    else
        std::cout<<"successfull"<<std::endl; */
}

CPlayerManager::~CPlayerManager()
{
    pthread_cancel( login_thread );
    pthread_cancel( save_thread );
    //pthread_cancel( deleteOldConnection_thread );
    //delete incon;
    //shutdownConnections.clear();
}

void CPlayerManager::saveAll()
{
    
    if ( !loggedOutPlayers.empty() )
    {
        std::cout<<"forced save:"<<std::endl;
        for ( TPLAYERVECTOR::iterator pIterator = loggedOutPlayers.begin(); pIterator != loggedOutPlayers.end(); ++pIterator )
        {
              // Logout - Vorgang protokollieren
              if ( !(*pIterator)->isMonitoringClient() )
              {
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

bool CPlayerManager::findPlayer(std::string name)
{
    mut.lock();
    for ( TPLAYERVECTOR::iterator pIterator = loggedOutPlayers.begin(); pIterator != loggedOutPlayers.end(); ++pIterator )
    {
        // Logout - Vorgang protokollieren
        if ( (*pIterator)->name == name )
        {
            mut.unlock();
            return true;
            
        }
    }
    mut.unlock();
    return false;
}

void CPlayerManager::setLoginLogout(bool val)
{
    if ( val ) 
    {
        //anmelden des exklusiven zugriffs
        reloadmutex.lock_upgrade();
        //bekommen des exklusiven zugriffs
        reloadmutex.unlock_upgrade_and_lock();
    }
    else
    {
        //exklusiver zugriff freigeben
        reloadmutex.unlock();
    }
}

void * CPlayerManager::loginLoop(CPlayerManager * pmanager)
{
    try
    {
        CInitialConnection::TVECTORPLAYER& newplayers = pmanager->incon.get_Player_Vector();
        TPLAYERVECTOR::iterator pIterator;
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;
        int curconn = 0;
        boost::shared_ptr<CNetInterface> Connection;
        while ( pmanager->running )
        {
             //loop must be steered by counter so we parse every connection only one time bevor we getting to the other loop
             curconn = newplayers.non_block_size();
             for ( int i = 0; i < curconn; ++i)
             {
                      std::cout<<"read connection"<<std::endl;
                      Connection = newplayers.non_block_pop_front();
                      if ( Connection )
                      {
                          bool isLocked = false;
                          try
                          {
                              if ( !Connection->online)throw CPlayer::LogoutException(UNSTABLECONNECTION);
                              if ( Connection->receivedSize() > 0 )
                              {
                                 std::cout<<"try to login player"<<std::endl;
                                 //get shared lock
                                 reloadmutex.lock_shared();
                                 isLocked = true; 
                                 CPlayer * newPlayer = new CPlayer(Connection);
                                 //release shared lock
                                 reloadmutex.unlock_shared();
                                 isLocked = false;
                                 pmanager->loggedInPlayers.non_block_push_back( newPlayer );
                                 std::cout<<"added player to list"<<std::endl;
                                 Connection.reset();
                              }
                              else
                              {
                                  if ( Connection->nextInactive() )
                                  {
                                      std::cout<<Connection->getIPAdress()<<" logged out because inactive count!"<<std::endl;
                                      throw CPlayer::LogoutException(UNSTABLECONNECTION);
                                  }
                                  newplayers.non_block_push_back( Connection );
                                  Connection.reset();
                              }
                         } 
                         catch (CPlayer::LogoutException &e) 
                         {   // Only unlock reloadmutex if it has actually been locked
                             if( isLocked ) reloadmutex.unlock_shared();
                             std::cout<<"got logoutException durin loading! "<<std::endl;
                             boost::shared_ptr<CBasicServerCommand>cmd(new CLogOutTC( e.getReason() ));
                             Connection->shutdownSend(cmd);
                             //Connection->closeConnection();
                             //Connection.reset();
                             //pmanager->shutdownConnections.non_block_push_back( Connection );
                         }
                      }
                      else
                      {
                          std::cout<<"try to get connection but was NULL!"<<std::endl;
                          Connection.reset();
                      }

                } // get new players
                nanosleep(&waittime, NULL );
        }
    }
    catch ( std::exception &e )
    {
        
    }
    catch ( ... )
    {
        throw;
    }
    return NULL;
}        
    
void * CPlayerManager::playerSaveLoop(CPlayerManager * pmanager)
{
    try
    {
        CWorld * world = CWorld::get();
        TPLAYERVECTOR::iterator pIterator;
        timespec waittime;
        waittime.tv_sec = 0;
        waittime.tv_nsec = 100000000;
        pmanager->threadOk = true;
        CPlayer * tmpPl=NULL;
        while ( pmanager->running )
        {
                if ( !pmanager->loggedOutPlayers.empty() )
                {
                    CLogger::writeMessage("basic", "Ausgelogte Spieler bearbeiten: " + CLogger::toString(pmanager->loggedOutPlayers.size()) + " players gone");
                    std::cout<<"ausgeloggte Spieler bearbeiten:"<<std::endl;
                    while ( !pmanager->loggedOutPlayers.empty() )
                    {
                        mut.lock();
                        tmpPl = pmanager->loggedOutPlayers.non_block_pop_front();
                        mut.unlock();
                        // Logout - Vorgang protokollieren
                        if ( !tmpPl->isMonitoringClient() )
                        {
                            reloadmutex.lock_shared(); 
                            tmpPl->save();
                            reloadmutex.unlock_shared(); 
                            tmpPl->Connection->closeConnection();
                            std::cout<<"removed player: "<<tmpPl->name<<std::endl;
                            boost::shared_ptr<CBasicServerCommand> cmd(new CBBLogOutTC( tmpPl->id, tmpPl->name ));
                            world->monitoringClientList->sendCommand( cmd );
                            //std::cout<<"deleting player!"<<std::endl;
                            //pmanager->shutdownConnections.non_block_push_back( tmpPl->Connection);
                            //(*pIterator)->Connection.reset();
                            delete tmpPl;
                            tmpPl=NULL;
                            std::cout<<"deleting end!"<<std::endl;
                        }
                        else
                        {
                            tmpPl->Connection->closeConnection();
                            //std::cout<<"deleting Monitoring client!"<<std::endl;
                            //pmanager->shutdownConnections.non_block_push_back( tmpPl->Connection);
                            //(*pIterator)->Connection.reset();
                            delete tmpPl;
                            tmpPl=NULL;
                            std::cout<<"player deleted"<<std::endl;
                        }
                  }
                  CLogger::writeMessage("basic","Ausgelogte Spieler bearbeiten fertig [Begin]" );
                  std::cout<<"Ausgelogte Spieler bearbeiten fertig [Begin]"<<std::endl;
                  // Statistik aktualisieren
                  //loadlog.update( world->Players.size() );
                  //world->saveAllPlayerNamesToFile( configOptions["datadir"] + std::string( ONLINEPLFILE ) );
                  world->updatePlayerList();
                  CLogger::writeMessage("basic", "Ausgelogte Spieler bearbeiten fertig [End]" );
                  std::cout<<"Ausgelogte Spieler bearbeiten fertig [End]"<<std::endl;
               }
               nanosleep(&waittime, NULL);
        }

    }
    catch ( std::exception &e )
    {
        
    }
    catch ( ... )
    {
        throw;
    }
    return NULL;
}                 

//void * CPlayerManager::deleteOldConnectionsLoop(CPlayerManager * pmanager)
//{
//    try
//    {
//        TPLAYERVECTOR::iterator pIterator;
//        timespec waittime;
//        waittime.tv_sec = 0;
//        waittime.tv_nsec = 100000000;
//        pmanager->threadOk = true;
//        boost::shared_ptr<CNetInterface> sd;
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
