#ifndef _CRECEIVER_HPP_
#define _CRECEIVER_HPP_

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <sys/types.h>
#include "Connection.h"
#include "netinterface/CNetInterface.hpp"
#include "netinterface/CBasicClientCommand.hpp"
#include "CCommandFactory.hpp"
#include "constants.h"
#include "netinterface/CByteBuffer.hpp"
#include <boost/shared_ptr.hpp>

/**
*@ingroup Netinterface
*class which holds the receive data thread and performs the action to receive 
*data from the client
*/
class CReceiver
{
    public:
    
        /**
        *constructor
        *@param conn the connection from which the data is received
        *@param rQueue the receive queue to which the received commands should be stored
        */
        CReceiver( CNetInterface * conn , boost::shared_ptr<CNetInterface::CLIENTCOMMANDLIST> rQueue );
        ~CReceiver();
        
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
    
        boost::shared_ptr<CByteBuffer> buffer; /*<local buffer to the received data*/
        //CByteBuffer * buffer;
        boost::shared_ptr<CBasicClientCommand> cmd; /*<command which is actuelly received*/
        
        volatile void checkData(); /*<checkes the received and try to get the command data*/
        volatile uint8_t numtrys; /*< unsuccessful attemps to read data for a command*/
        
        volatile bool rlOK; /*<determines if the current receive loop isn't aborted*/
        volatile bool rlrunning; /*<checkes if the current receive Loop is running*/
        
        int lastReadedByte; /*<stores the last byte which was red from the buffer*/
        CNetInterface * connection; /*<the connection to the player*/
        
        boost::shared_ptr<CNetInterface::CLIENTCOMMANDLIST> receivedQueue; /*<pointer to the queue which holds the received commands*/
        
        pthread_t receive_thread; /*<Thread which receives data and stores them in the buffer*/
	
        /**
        *function for the receive thread which stores data from the incoming transfer to a buffer
        *looks every x ms in the socket and stores the data in a buffer.
        */
        static void* receive_loop( CReceiver * receiver );
        
        CCommandFactory commandFactory;

};

#endif
