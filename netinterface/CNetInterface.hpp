#ifndef _CNETINTERFACE_HPP_
#define _CNETINTERFACE_HPP_

/**
*@ingroup Netinterface
*/


//include thread save vector
#include "tpvector.hpp"

#include "Connection.h"
#include "netinterface/CBasicClientCommand.hpp"
#include "netinterface/CBasicServerCommand.hpp"
#include "netinterface/CCommandFactory.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <deque>


/**
* @defgroup Netinterface Network Interface
* This is the interface for server client communication.
*/

/**
*@ingroup Netinterface
*class which holds the network interface and its thread for sending and receiving data
*/
class CNetInterface : public boost::enable_shared_from_this<CNetInterface>
{
    public:
        
        /**
        * standard constructor, creates the two thread class receiver and sender
        */
        CNetInterface(boost::asio::io_service &io_servicen);
        
        /**
        * destructor deletes all the classes which are constructed in the constructor
        * WARNING: shoudln't be called before the threads stopped correctly
        */
        ~CNetInterface();
        
        void closeConnection(); /*<closes the connection to the client*/
        bool activate(); /*<activates the connection starts the sending and receiving threads*/
        bool nextInactive();
        
        /**
        * adds a command to the send queue so it will be sended correctly to the connection
        * @param command the command which should be added
        */
        void addCommand( boost::shared_ptr<CBasicServerCommand> command );
        
        void shutdownSend( boost::shared_ptr<CBasicServerCommand> command );
        
        std::string getIPAdress();
        
        /**
        * returns a command from the receive Queue if on is available
        * @return the command which was in the receive Queue or an empty (NULL) Command if there wasn't something in the receive Queue
        */
        boost::shared_ptr<CBasicClientCommand> getCommand();
        
        
        volatile bool online; /*< if connection is active*/
        
        /**
        * gets the number of received Commands
        * @return the number of commands which are currently in the received queue
        */
        uint16_t receivedSize(){ return receiveQueue.size(); }
        
        typedef std::deque< boost::shared_ptr<CBasicClientCommand> >CLIENTCOMMANDLIST;
        typedef std::deque< boost::shared_ptr<CBasicServerCommand> >SERVERCOMMANDLIST;

	boost::asio::ip::tcp::socket& getSocket(){ return socket; }

    private:
        
        
        void handle_read_header(const boost::system::error_code& error);
        void handle_read_data(const boost::system::error_code& error);
        
        void handle_write(const boost::system::error_code& error);
        void handle_write_shutdown(const boost::system::error_code& error);
        
        //Buffer for the header of messages
        unsigned char headerBuffer[6];
       
        boost::shared_ptr<CBasicClientCommand> cmd;
        boost::shared_ptr<CBasicServerCommand> shutdownCmd;
        boost::shared_ptr<CBasicServerCommand> cmdToWrite;


        CLIENTCOMMANDLIST receiveQueue; /*<stores the commands which are received in a queue*/
        SERVERCOMMANDLIST sendQueue;
        
        std::string ipadress;

        boost::asio::io_service &io_service;
        boost::asio::ip::tcp::socket socket;
                
        //Factory für Commands vom Client
        CCommandFactory commandFactory;
        uint16_t inactive;
        boost::mutex sendQueueMutex;
        boost::mutex receiveQueueMutex;
        

       
};


#endif

