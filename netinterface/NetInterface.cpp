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


#include <boost/enable_shared_from_this.hpp>
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/protocol/ClientCommands.hpp"
#include "CommandFactory.hpp"


NetInterface::NetInterface(boost::asio::io_service &io_servicen) : online( false ), io_service(io_servicen), socket(io_servicen), inactive(0)
{
    cmd.reset();
}

std::string NetInterface::getIPAdress()
{
    return ipadress;
}


NetInterface::~NetInterface()
{
    try
    {
        //std::cout<<"destructing new netinterface"<<std::endl;
        online = false;
        sendQueue.clear();
        receiveQueue.clear();
		socket.close();   
        //std::cout<<"destruction done"<<std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
    }
    catch (...)
    {} 

}



void NetInterface::closeConnection()
{
    
    //std::cerr<<getIPAdress()<<"closeConnection called"<<std::endl;
    online = false;
}

bool NetInterface::activate()
{
    try
    {
        boost::asio::async_read(socket,boost::asio::buffer(headerBuffer,6), boost::bind(&NetInterface::handle_read_header, shared_from_this(),boost::asio::placeholders::error));
        ipadress = socket.remote_endpoint().address().to_string();
        online = true;
        return true;
    } 
    catch( std::exception &ex )
    {
        std::cerr<<"error during activate: "<<ex.what()<<std::endl;
        return false;
    }
}


void NetInterface::handle_read_data(const boost::system::error_code& error)
{
    if ( !error )
    {
        if ( online )
        {
            cmd->decodeData();
            //std::cout<<"data decoded"<<std::endl;
            if ( cmd->isDataOk() )
            {
                //std::cout<<"data was ok"<<std::endl;
                receiveQueueMutex.lock();
                receiveQueue.push_back( cmd );
                receiveQueueMutex.unlock();
            }
            else 
            {
                std::cout<<"error receiving command"<<std::endl;
            }
            //std::cout<<"cmd resetted!"<<std::endl;
            cmd.reset();
            boost::asio::async_read(socket,boost::asio::buffer(headerBuffer,6), boost::bind(&NetInterface::handle_read_header, shared_from_this(),boost::asio::placeholders::error));
        }
    }
    else
    {
        if (online)std::cerr<<"handle_read_data error during read "<<getIPAdress()<<" "<<error.message()<<" :"<< error<<std::endl;
        closeConnection();
        boost::asio::async_read(socket,boost::asio::buffer(headerBuffer,6), boost::bind(&NetInterface::handle_read_header, shared_from_this(),boost::asio::placeholders::error));
    }
}

bool NetInterface::nextInactive()
{
    inactive++;
    return ( inactive > 1000 );
}

void NetInterface::handle_read_header(const boost::system::error_code& error)
{
    if ( !error )
    {
        if ( (headerBuffer[0] xor 255) == headerBuffer[1])
        {
            //Correcter header decodieren und comand empfangen
            uint16_t length = headerBuffer[2] << 8;
            length = length | headerBuffer[3];
            uint16_t checkSum = headerBuffer[4] << 8;
            checkSum = checkSum | headerBuffer[5];
            cmd = commandFactory.getCommand(headerBuffer[0]);
            if ( cmd )
            {
                cmd->setHeaderData(length,checkSum);
                boost::asio::async_read(socket,boost::asio::buffer(cmd->msg_data(),cmd->getLength()), boost::bind(&NetInterface::handle_read_data, shared_from_this(),boost::asio::placeholders::error));
                return;
            }
            else
            {
                std::cout<<"No Command with id "<<headerBuffer[0]<<"found searching new Command"<<std::endl;
            }
        }
        //Kein Correcter Header
        std::cout<<"wrong header searching for command id"<<std::endl;
        //Suchen im Header nach einer Command ID
        for ( int i = 1 ; i < 5; ++i)
        {
            //Korrekte Command ID gefunden
            if ( (headerBuffer[i] xor 255) == headerBuffer[i+1] )
            {
                std::cout<<"found correct command id at pos "<<i<<std::endl;
                //copy the rest of the correct message to the start of the buffer
                int start = 0;
                while ( i < 6 )
                {
                    headerBuffer[start++] = headerBuffer[i++];
                }
                //restheader empfangen
                boost::asio::async_read(socket,boost::asio::buffer(&headerBuffer[start],6-start), boost::bind(&NetInterface::handle_read_header, shared_from_this(),boost::asio::placeholders::error));
                return;
            }
        }
        //Keine Command Signature gefunden wieder 6 Byte Header auslesen
        boost::asio::async_read(socket,boost::asio::buffer(headerBuffer,6), boost::bind(&NetInterface::handle_read_header, shared_from_this(),boost::asio::placeholders::error));

    }
    else
    {
        if (online)std::cerr<<"handle_read_header error during read "<<getIPAdress()<<" "<<error.message()<<" :"<< error<<std::endl;
        closeConnection();
        
    }
}


void NetInterface::addCommand( boost::shared_ptr<BasicServerCommand> command )
{
    if ( online )
    {
        command->addHeader();
        sendQueueMutex.lock();
        bool write_in_progress = !sendQueue.empty();
        sendQueue.push_back(command);
        try
        {
            if (!write_in_progress && online)
            {
                boost::asio::async_write(socket,boost::asio::buffer(sendQueue.front()->cmdData(),sendQueue.front()->getLength()),
                    boost::bind(&NetInterface::handle_write, shared_from_this(),boost::asio::placeholders::error));
            }
        }
        catch ( std::exception &ex )
        {
            std::cerr<<"addCommand error during write: "<<ex.what()<<std::endl;
            closeConnection();
        }
        sendQueueMutex.unlock();
    }

}

void NetInterface::shutdownSend( boost::shared_ptr<BasicServerCommand> command )
{
	try
	{
		command->addHeader();
		shutdownCmd = command;
		boost::asio::async_write(socket,boost::asio::buffer(shutdownCmd->cmdData(),shutdownCmd->getLength()),
		boost::bind(&NetInterface::handle_write_shutdown, shared_from_this(),boost::asio::placeholders::error));
	}
	catch ( std::exception &ex )
	{
		std::cerr<<"Exception beim Schreiben von Daten:"<<ex.what()<<std::endl;
		closeConnection();
	}
}

void NetInterface::handle_write(const boost::system::error_code& error)
{
	try
	{
		if ( !error )
		{
            if ( online )
            {
                sendQueueMutex.lock();
                sendQueue.pop_front();
                if ( !sendQueue.empty() && online)
                {
                    boost::asio::async_write(socket,boost::asio::buffer(sendQueue.front()->cmdData(),sendQueue.front()->getLength()),
                        boost::bind(&NetInterface::handle_write, shared_from_this(),boost::asio::placeholders::error));
                }
                sendQueueMutex.unlock();
                
            }
		}
		else
		{    
            std::cerr<<"handle_write error during write: "<<error.message()<<" :"<<error<<std::endl;
			closeConnection();
		}
	}
	catch ( std::exception &ex )
	{
        std::cout<<"caugth Exception on handle_write"<<ex.what()<<std::endl;
		closeConnection();
	}
}

void NetInterface::handle_write_shutdown(const boost::system::error_code& error)
{
    if ( !error )
    {
        closeConnection();
        shutdownCmd.reset();
    }
    else
    {
        if (online)std::cerr<<"handle_write_shutdown error during write: "<<error.message()<<" :"<<error<<std::endl;
        closeConnection();
    }
}

boost::shared_ptr<BasicClientCommand> NetInterface::getCommand()
{
    if ( online )
    {
        boost::shared_ptr<BasicClientCommand> ret;
        receiveQueueMutex.lock();
        if ( !receiveQueue.empty() )
        {
            ret = receiveQueue.front();
            receiveQueue.pop_front();
        }
        receiveQueueMutex.unlock();
        return ret;
    }
    return boost::shared_ptr<BasicClientCommand>();
}

