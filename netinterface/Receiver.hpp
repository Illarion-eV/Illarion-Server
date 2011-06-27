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


#ifndef _CRECEIVER_HPP_
#define _CRECEIVER_HPP_

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <sys/types.h>
#include "Connection.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/BasicClientCommand.hpp"
#include "CommandFactory.hpp"
#include "constants.hpp"
#include "netinterface/ByteBuffer.hpp"
#include <boost/shared_ptr.hpp>

/**
*@ingroup Netinterface
*class which holds the receive data thread and performs the action to receive 
*data from the client
*/
class Receiver
{
    public:
    
        /**
        *constructor
        *@param conn the connection from which the data is received
        *@param rQueue the receive queue to which the received commands should be stored
        */
        Receiver( NetInterface * conn , boost::shared_ptr<NetInterface::LIENTCOMMANDLIST> rQueue );
        ~Receiver();
        
        /**
        *starts the current threads
        */
        bool start();
        
        /**
        * stops the current threads
        */
        void stop()
        {
            rlrunning = false;
        }
        
        /**
        * checks if the thread is currently running
        * @return true if the thread is running otherwise false
        */
        bool threadsStopped(){ return ( !rlOK ); }
        
        
    protected:
    
        boost::shared_ptr<ByteBuffer> buffer; /*<local buffer to the received data*/
        //CByteBuffer * buffer;
        boost::shared_ptr<BasicClientCommand> cmd; /*<command which is actuelly received*/
        
        volatile void checkData(); /*<checkes the received and try to get the command data*/
        volatile uint8_t numtrys; /*< unsuccessful attemps to read data for a command*/
        
        volatile bool rlOK; /*<determines if the current receive loop isn't aborted*/
        volatile bool rlrunning; /*<checkes if the current receive Loop is running*/
        
        int lastReadedByte; /*<stores the last byte which was red from the buffer*/
        NetInterface * connection; /*<the connection to the player*/
        
        boost::shared_ptr<NetInterface::LIENTCOMMANDLIST> receivedQueue; /*<pointer to the queue which holds the received commands*/
        
        pthread_t receive_thread; /*<Thread which receives data and stores them in the buffer*/
	
        /**
        *function for the receive thread which stores data from the incoming transfer to a buffer
        *looks every x ms in the socket and stores the data in a buffer.
        */
        static void* receive_loop( Receiver * receiver );
        
        CommandFactory commandFactory;

};

#endif
