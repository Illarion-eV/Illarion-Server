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


#ifndef _CSENDER_HPP_
#define _CSENDER_HPP_

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <sys/types.h>
#include "Connection.h"
#include "netinterface/CBasicServerCommand.hpp"
#include "netinterface/CNetInterface.hpp"
#include <boost/shared_ptr.hpp>


class CSender
{
    public:
    
        /**
        *constructor for the netinterface class
        *@param conn the connection to whichthe data should be send
        *@param sQueue the sendQueue from which the commands to send should be red
        */
        CSender( CNetInterface * conn,  boost::shared_ptr<CNetInterface::SERVERCOMMANDLIST> sQueue );
        ~CSender();
        
        /**
        *function which starts the send thread
        *@return false if an error occoured during starting the thread
        */
        bool start();
        
        /**
        * stops the current send thread
        */
        void stop(){ slrunning = false; }
        
        /**
        * checks if the thread is currently running or stopped
        * @return true if the thread was shutdown and is actually not running
        */
        bool threadsStopped(){ return !threadOK; }
        
        /**
        * starts a critical send which won't allow to shutdown the connection
        * until at least one critical send command is added to the sended commands
        */
        void criticalSendActive(){ cSendActive = true; }
        
	
    protected:
        
        CNetInterface * connection; /*< the connection to the player*/
        
    private:
    
        volatile bool cSendActive; /*<set to true if one critical command has to be sended before shutting down the connection*/
    	
        pthread_t send_thread; /*<the thread which sends the data*/
        boost::shared_ptr<CNetInterface::SERVERCOMMANDLIST> sendQueue; /*<pointer to the queue which holds the commands which should be sended*/
        volatile bool threadOK; /*<true if the thread works without problems*/
        volatile bool slrunning; /*<true if the thread is running*/
        
        /**
        *loop which runs in a thread 
        *proofes if new commands are in the to send queue if yes 
        *the buffer of the command is added to the connection and the data is send
        */
        static void* send_loop( CSender * sender );
   
};
#endif


