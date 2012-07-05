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


#ifndef TUNINGCONSTANTS_HPP
#define TUNINGCONSTANTS_HPP

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

#define P_PUSH_COST 40
#define NP_PUSH_COST 40

#define P_CAST_COST 10
#define NP_CAST_COST 10

#define P_USE_COST 10
#define NP_USE_COST 10

#define P_LOOK_COST 5

#define P_SAY_COST 3
#define P_SHOUT_COST 9
#define P_WHISPER_COST 1

#define P_ITEMMOVE_COST 3

#define P_SPIN_COST 1

#define P_REGENERATE_COST 0
#define NP_REGENERATE_COST 10

#define STANDARD_MONSTER_WALKING_COST 8

#define P_MOVECOSTFORMULA_walkingCost_MULTIPLIER 10
#define P_MOVECOSTFORMULA_agility_ADD 30

#define NP_MOVECOSTFORMULA_walkingCost_MULTIPLIER 12
#define NP_MOVECOSTFORMULA_agility_ADD 30

#define P_STANDARDWALKCOST 30
#define NP_STANDARDWALKCOST 30

#define NP_STANDARDFLYCOST 20

#define MAXTHROWDISTANCE 10
#define MAXTHROWWEIGHT 99
#define MAXDROPDISTANCE 2

#endif
