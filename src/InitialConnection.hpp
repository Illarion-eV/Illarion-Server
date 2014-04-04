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


#ifndef InitialConnection_HH
#define InitialConnection_HH

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
//#define InitialConnection_DEBUG

#include <memory>

#include <boost/asio.hpp>

#include "thread_safe_vector.hpp"
#include "Connection.hpp"

class NetInterface;

//! die maximal Anzahl von noch nicht bearbeiteten Verbindungen in der Warteschlange
#define BACKLOG 10

//! erstellt Verbindungen zu neuen Clients
// Eine Instanz dieser Klasse wartet auf Verbindungsanfragen
// von Clients �ber einen speziellen Port und erstellt die ben�tigten
// Kommunikationsendpunkte (Socket).
class InitialConnection : public Connection {

public:


    typedef thread_safe_vector<std::shared_ptr<NetInterface>> TVECTORPLAYER;

    //! Konstruktor
    InitialConnection();

    //! Destruktor
    ~InitialConnection();

    //! liefert alle neu erstellten Player zur�ck
    // \return f�r die angemeldeten Clients mit richtiger Clientversion
    // einen Player mit player.name und player.pw, entsprechend der gesendeten Daten
    // besetzt
    TVECTORPLAYER &get_Player_Vector();

private:

    void run_service();

    boost::asio::io_service io_service;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;


    //! wartet auf eine neue Verbindung (blockierend)
    // \return eine neue Verbindung oder nullptr
    //CInternetConnection* accept_connection();
    void accept_connection(std::shared_ptr<NetInterface> connection, const boost::system::error_code &error);

    //! std::vector for new players...
    TVECTORPLAYER playerVector;
};

#endif
