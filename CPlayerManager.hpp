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


#include "InitialConnection.h"
#include <pthread.h>
#include "tvector.hpp"
#include "boost/thread/shared_mutex.hpp"


class CPlayer;

class CPlayerManager
{
    public:
        static CPlayerManager * get();
        
        ~CPlayerManager();
        
        void saveAll();
        
        void activate();
        
        bool threadOK(){ return threadOk; }
        
        bool findPlayer(std::string name);
        
        void setLoginLogout(bool val);
        
        typedef tvector<CPlayer*> TPLAYERVECTOR;
        
        TPLAYERVECTOR &getLogOutPlayers(){ return loggedOutPlayers; }
        TPLAYERVECTOR &getLogInPlayers(){ return loggedInPlayers; }
        
        
    private:
        CPlayerManager();    
        
        static CPlayerManager * instance; /**< pointer to current local instance of the Player Manager*/
    
        /**
        * loop which is threaded to get new connections
        * create players, or storing data and deleting old connections
        */
        static void * loginLoop(CPlayerManager * pmanager);
        static void * playerSaveLoop(CPlayerManager * pmanager);
        //static void * deleteOldConnectionsLoop(CPlayerManager * pmanager);
        static boost::mutex mut;
        
        //Mutex der gesetzt wird beim reloaden. (Als multi read single write lock)
        static boost::shared_mutex reloadmutex;
                
        /**
        * true if the thread is running
        */
        volatile bool running;
        
        /**
        * if false the thread was exited correctly
        */
        volatile bool threadOk;
    
        /**
        * logged out players which connection wherent shutted down
        */
        //CInitialConnection::TVECTORPLAYER shutdownConnections;
        
        /**
        * player which are not on the main map anymore
        */
        TPLAYERVECTOR loggedOutPlayers;

        
        /**
        * players which are logged in and correctly loaded
        */
        TPLAYERVECTOR loggedInPlayers;
        
        /**
        * initial connection to get the new connections
        */
        CInitialConnection incon;
        
        pthread_t login_thread;
        pthread_t save_thread;
        //pthread_t deleteOldConnection_thread;
        
       
    
    
};
