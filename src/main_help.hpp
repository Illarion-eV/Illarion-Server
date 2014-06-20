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


#ifndef MAIN_HELP_H
#define MAIN_HELP_H

#include <vector>

// break out of the main loop if false
extern volatile bool running;

// fields write numbers of deleted containeritems, the maps deletes them
extern std::vector<int> erasedcontainers;

// some forward declarations to not include unnecessary headers
class Player;

void logout_save(Player *who, bool forced, unsigned long int thistime);

void login_save(Player *who);

// process commandline arguments
bool checkArguments(int argc, char *argv[]);

// load item definitions
void loadData();

// initialise signal handlers
bool init_sighandlers();

// reset signal handlers
void reset_sighandlers();

#endif
