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

#include <chrono>

using namespace std::chrono_literals;

constexpr auto reduceMentalCapacityInterval = 10s;
constexpr auto checkMonitoringClientsInterval = 250ms;
constexpr auto scheduledScriptsInterval = 1s;
constexpr auto wearReductionInterval = 3min;
constexpr auto gameLoopInterval = 100ms;
constexpr auto ingameTimeUpdateInterval = 8h;

constexpr auto CLIENT_TIMEOUT = 50;

// how many players to process each turn (maximum)
constexpr auto MAXPLAYERSPROCESSED = 5;

constexpr auto MIN_AP_UPDATE = 100;

constexpr auto P_MIN_AP = 7;
constexpr auto P_MAX_AP = 21;

constexpr auto NP_MIN_AP = 7;
constexpr auto NP_MAX_AP = 21;

constexpr auto P_MIN_FP = 0;
constexpr auto P_MAX_FP = 0;

constexpr auto NP_MIN_FP = 0;
constexpr auto NP_MAX_FP = 0;

constexpr auto P_ATTACK_COST = 15;
constexpr auto NP_ATTACK_COST = 20;

constexpr auto P_PUSH_COST = 1;
constexpr auto NP_PUSH_COST = 1;

constexpr auto P_CAST_COST = 1;
constexpr auto NP_CAST_COST = 1;

constexpr auto P_USE_COST = 1;
constexpr auto NP_USE_COST = 1;

constexpr auto P_LOOK_COST = 0;

constexpr auto P_SAY_COST = 1;
constexpr auto P_SHOUT_COST = 1;
constexpr auto P_WHISPER_COST = 1;

constexpr auto P_ITEMMOVE_COST = 1;

constexpr auto P_SPIN_COST = 1;

constexpr auto P_REGENERATE_COST = 0;
constexpr auto NP_REGENERATE_COST = 10;

constexpr auto NP_WALK_COST = 20;

constexpr unsigned short MAX_WALK_AGI = 20;
constexpr TYPE_OF_WALKINGCOST ADDITIONAL_MONSTER_WALKING_COST = 1;
constexpr TYPE_OF_WALKINGCOST MIN_WALK_COST = 300;
constexpr TYPE_OF_WALKINGCOST MAX_WALK_COST = 800;
constexpr auto NP_STANDARDFLYCOST = 5;
constexpr float overtaxedRelativeLoad = 1.0;
constexpr float burdenedRelativeLoad = 0.75;
constexpr int walkNeutralAgility = 10;
constexpr float walkAgilityWeight = 0.01;
constexpr float walkLoadWeight = 0.3;
constexpr float runningMoveTimeMultiplier = 0.6;

constexpr double randomMonsterMoveProbability = 0.2;
constexpr auto monsterSelfHealAmount = 150;
constexpr uint8_t monsterViewRange = 9;

constexpr auto screenRange = 14;
constexpr auto whisperRange = 2;
constexpr auto talkRange = screenRange;
constexpr auto yellRange = 30;

constexpr auto nearbyFieldRange = 5;

constexpr auto MAXTHROWDISTANCE = 10;
constexpr auto MAXTHROWWEIGHT = 99;
constexpr auto MAXDROPDISTANCE = 2;
constexpr auto minimumCarryWeight = 5000;
constexpr auto carryWeightPerStrength = 500;
#endif
