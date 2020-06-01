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

#ifndef TUNINGCONSTANTS_HPP
#define TUNINGCONSTANTS_HPP

#include "types.hpp"

#define CLIENT_TIMEOUT 50

// how many players to process each turn (maximum)
#define MAXPLAYERSPROCESSED 5

#define MIN_AP_UPDATE 100

#define P_MIN_AP 7
#define P_MAX_AP 21

#define NP_MIN_AP 7
#define NP_MAX_AP 21

#define P_MIN_FP 0
#define P_MAX_FP 0

#define NP_MIN_FP 0
#define NP_MAX_FP 0

#define P_ATTACK_COST 15
#define NP_ATTACK_COST 20

#define P_PUSH_COST 1
#define NP_PUSH_COST 1

#define P_CAST_COST 1
#define NP_CAST_COST 1

#define P_USE_COST 1
#define NP_USE_COST 1

#define P_LOOK_COST 0

#define P_SAY_COST 1
#define P_SHOUT_COST 1
#define P_WHISPER_COST 1

#define P_ITEMMOVE_COST 1

#define P_SPIN_COST 1

#define P_REGENERATE_COST 0
#define NP_REGENERATE_COST 10

const unsigned short MAX_WALK_AGI = 20;
const TYPE_OF_WALKINGCOST ADDITIONAL_MONSTER_WALKING_COST = 1;
const TYPE_OF_WALKINGCOST MIN_WALK_COST = 300;
const TYPE_OF_WALKINGCOST MAX_WALK_COST = 800;

#define NP_STANDARDFLYCOST 5

#define MAXTHROWDISTANCE 10
#define MAXTHROWWEIGHT 99
#define MAXDROPDISTANCE 2

#endif
