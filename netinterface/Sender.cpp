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


#include "netinterface/Sender.hpp"
#include <boost/shared_ptr.hpp>

CSender::CSender( CNetInterface * conn, boost::shared_ptr<CNetInterface::SERVERCOMMANDLIST> sQueue ) : connection( conn ) , sendQueue(sQueue)
{
    cSendActive = false;
    threadOK = false;
    slrunning = false;
    sendQueue->clear();
}

CSender::~CSender()
{
    sendQueue->clear();
    try
    {
        if ( threadOK )
        {
            pthread_cancel( send_thread );
            std::cerr<<"send Thread cancelled!"<<std::endl;
        }
    }
    catch (std::exception const& e)
    {
        //std::cerr << e.what () << std::endl;
    }
    catch ( ... )
    {
        throw;
    }
}


bool CSender::start()
{
    pthread_attr_t pattr;
    pthread_attr_init( &pattr);
    pthread_attr_setdetachstate( &pattr, PTHREAD_CREATE_DETACHED );
    if ( pthread_create( &send_thread, &pattr, (void*(*)(void*))&send_loop, this ) != 0 )
    {
        std::cerr << "Read-S-Number: " << connection->m_read_socket << " error while creating send thread!" << std::endl;
        return false;
    }
    threadOK = true;
    //pthread_join( send_thread, NULL ); 
    return true;
}

void* CSender::send_loop( CSender * sender )
{
    try
    {
         //create waittime struct so the thread waits for 20 ms 
         timespec waittime; 
         waittime.tv_sec = 0;
         waittime.tv_nsec = 10000000; //1 ms;    
         sender->slrunning = true;
         timespec lwaittime;
         lwaittime.tv_sec = 0;
         lwaittime.tv_nsec = 100000000; //50 ms
         boost::shared_ptr<CBasicServerCommand> cmd;
         while (  (sender->cSendActive && sender->sendQueue) || (sender->slrunning && sender->sendQueue) )
         {
             cmd.reset();
             if ( sender->cSendActive ) std::cout<<"critical Send"<<std::endl;
             if ( !sender->sendQueue->empty()  )
             {
                  cmd = sender->sendQueue->pop_front();
                  if ( cmd )
                  {
                      cmd->cmdSend( sender->connection );
                      if ( cmd->isCriticalSendCommand )
                      {
                          sender->cSendActive = false;
                          std::cout<<"critical send Active = false"<<std::endl;
                      }
                  }
                  else
                  {
                     std::cout<<"got not referenced command in sendQueue!"<<std::endl;
                     
                  }
                  nanosleep( &waittime, NULL );                  
             }
             else
             {
                 nanosleep( &lwaittime, NULL );
             }
         }
         std::cout<<"send Thread ended!"<<std::endl;
         sender->threadOK = false;
         pthread_exit( 0 );
         return NULL;
    }
    catch ( std::exception e)
    {
        //std::cerr<<e.what()<<std::endl;
    }
    catch ( ... )
    {
        throw;
    }
    return NULL;
}

