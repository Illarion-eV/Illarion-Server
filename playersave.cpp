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


#include "playersave.hpp"
#include "Player.hpp"
#include <unistd.h>
#include <signal.h>
#include "Connection.hpp"
#include "netinterface/NetInterface.hpp"

tvector<CPlayer*> players_to_save;
std::map<std::string, int> players_tosave_map;

void* player_save_loop(void*);

pthread_t save_thread;

bool keep_thread;

bool start_savethread() {
	pthread_attr_t pattr;
	pthread_attr_init( &pattr );
	pthread_attr_setdetachstate( &pattr, PTHREAD_CREATE_DETACHED );

	keep_thread=true;
	int temp = pthread_create( &save_thread, &pattr,(void*(*)(void*)) &player_save_loop, NULL);
	if ( temp != 0 ) {
		std::cerr << "Could not create Player save thread: "
		<< strerror( errno ) << std::endl;
		return false;
	}

	return true;
}

volatile bool save_done, save_thread_done;

void stop_savethread() {
	keep_thread = false;
	while (!save_thread_done) {
		std::cout << "waiting for save thread to exit..." << std::endl;
		sleep(1);
	}
}

pthread_t playersavethread;

void sig_chld(int) {
	// just exit the thread...
	pthread_exit(NULL);
}

void* save_this_player(void* ptr) {
	struct sigaction sigact;
	sigact.sa_handler = sig_chld;
	sigact.sa_flags = SA_RESTART;
	sigemptyset(&sigact.sa_mask);
	if ( sigaction( SIGCHLD, &sigact, NULL) < 0 ) {
		std::cerr << "failed to set sigaction for sig_chld!" << std::endl;
		pthread_exit(NULL);
	}
    try 
    {
        if (((CPlayer*)ptr)->save()) 
        {
            save_done=true;
        }
    }
    catch ( ... )
    {
        std::cerr<< "catched exception while trying to save player!"<<std::endl;
    }

	return NULL;
}

bool save_player(CPlayer* player) {
	players_tosave_map[player->name]++;
	pthread_attr_t pattr;
	pthread_attr_init( &pattr );
	pthread_attr_setdetachstate( &pattr, PTHREAD_CREATE_JOINABLE);
	save_done = false;
	int temp = pthread_create( &playersavethread, &pattr, &save_this_player, (void*)player );
	if (temp != 0)
		return false;
	for (int i=0; !save_done && i<4000; ++i)
		usleep(10000);
	void* ret;
	if (! save_done) {
		std::cerr << "killed save thread for player " << player->name << std::endl;
		// kill thread...
		pthread_kill(playersavethread, SIGCHLD);
		pthread_join(playersavethread, &ret);
		return false;
	}
	pthread_join(playersavethread, &ret);
	return true;
}

void* player_save_loop(void*) {
	struct sigaction sigact;
	sigact.sa_handler=  SIG_IGN;
	sigact.sa_flags = SA_NOMASK;
	sigemptyset(&sigact.sa_mask);
	if ( sigaction( SIGCHLD, &sigact, NULL) < 0 ) {
		std::cerr << "failed to ignore sig_chld!" << std::endl;
		exit(1);
	}

	save_thread_done = false;

	while (keep_thread) {
		while (!players_to_save.empty()) {
			CPlayer* temp = players_to_save.pop_front();
			if (! save_player(temp) ) {
				if (players_tosave_map[temp->name] > 10) {
					std::cerr << "*** player " << temp->name << " not saved!" << std::endl;
					delete temp;
				} else {
					std::cout << "could not save player " << temp->name << " trying again... " << std::endl;
					players_to_save.push_back(temp);
				}
			} else {
				players_tosave_map.erase(temp->name);
				std::cout << "saved player " << temp->name << std::endl;
				delete temp;
			}
		}
		usleep(100);
	}
	std::cout << "playersave thread done." << std::endl;
	save_thread_done = true;
	return NULL;
}
