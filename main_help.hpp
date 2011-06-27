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


#ifndef MAIN_HELP_H
#define MAIN_HELP_H

#include <signal.h>
#include <vector>

#include "globals.hpp"

class World;

//! falls false beendet sich das Programm
extern volatile bool running;

// in diesen std::vector fügen Fields die numbers der gelöschten containeritems ein,
// damit die zugehörige Map die containerinhalte löschen kann
extern std::vector<int>* erasedcontainers;

// Koordinaten von gelöschten Containern, benötigt zum Schließen offener Showcases
extern std::vector<position>* contpos;

// some forward declarations to not include unecessary headers
class Player;

void logout_save( Player* who, bool forced, unsigned long int thistime );

void login_save( Player* who );

//! Das Initialisieren von Logoptionen
void InitLogOptions();

//! Die Initialisierung des Servers mit Daten aus einer Datei
bool Init( const std::string& initfile);

//! zur Prüfung der Kommandozeilenargumente
void checkArguments( int argc, char* argv[]);

//! setup file positions
bool setup_files();

// Itemdefinitionen laden //
void loadData();

//! initialise signal handlers
bool init_sighandlers();

//! reset signal handlers
void reset_sighandlers();

#endif
