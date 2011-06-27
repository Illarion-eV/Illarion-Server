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


#include "InitialConnection.hpp"
#include <boost/thread.hpp>
#include <sstream>
#define InitialConnection_DEBUG

InitialConnection::InitialConnection() 
{
#ifdef InitialConnection_DEBUG
	std::cout << "InternetConnection Konstruktor Start\n";
#endif
        boost::thread servicethread(boost::bind(&InitialConnection::run_service,this));
#ifdef InitialConnection_DEBUG
	std::cout << "InternetConnection Konstruktor Ende\n";
#endif
}


InitialConnection::TVECTORPLAYER& InitialConnection::get_Player_Vector()
{
	return playerVector;
}

void InitialConnection::run_service()
{
    int port;
    std::stringstream ss;
    ss << configOptions["port"];
    ss >> port;

    boost::asio::ip::tcp::endpoint endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor = new boost::asio::ip::tcp::acceptor(io_service,endpoint);
    boost::shared_ptr<NetInterface> newConnection(new NetInterface(io_service));
    acceptor->async_accept(newConnection->getSocket(),boost::bind(&InitialConnection::accept_connection, this, newConnection, boost::asio::placeholders::error));
    std::cout<<"Starting the IO Service!"<<std::endl;
    io_service.run();
}

//CInternetConnection* InitialConnection::accept_connection() {
void InitialConnection::accept_connection(boost::shared_ptr<NetInterface> connection, const boost::system::error_code& error)
{
    if (!error)
    {
        if (connection->activate())
        {
            //Verbindung in die Liste aufnehmen
            playerVector.push_back(connection);
        }
        else
        {
            std::cerr<<"Fehler bei Aktivierung der Connection"<<std::endl;
        }
        boost::shared_ptr<NetInterface> newConnection(new NetInterface(io_service));
        acceptor->async_accept(newConnection->getSocket(),boost::bind(&InitialConnection::accept_connection, this, newConnection, boost::asio::placeholders::error));
    }
    else
    {
        std::cerr<<"Fehler im Accept:" << error.message() << ": " <<error.value() <<std::endl;
    }
        
}



InitialConnection::~InitialConnection() {
#ifdef InitialConnection_DEBUG
	std::cout << "InternetConnection Destruktor Start" << std::endl;
#endif
        io_service.stop();
        delete acceptor;
        acceptor = NULL;
#ifdef InitialConnection_DEBUG
	std::cout << "InternetConnection Destruktor Ende" << std::endl;
#endif
}
