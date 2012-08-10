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


#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <stdint.h>

typedef uint16_t TYPE_OF_ITEM_ID;
typedef uint32_t TYPE_OF_CHARACTER_ID;

typedef uint16_t TYPE_OF_WEIGHT;
typedef uint8_t  TYPE_OF_AGINGSPEED;
typedef uint8_t  TYPE_OF_BRIGHTNESS;
typedef uint32_t TYPE_OF_WORTH;
typedef uint16_t TYPE_OF_BUY_STACK;

typedef uint8_t  TYPE_OF_ATTACK;
typedef uint8_t  TYPE_OF_DEFENCE;
typedef uint8_t  TYPE_OF_ACCURACY;
typedef uint8_t  TYPE_OF_RANGE;
typedef uint8_t  TYPE_OF_WEAPONTYPE;
typedef uint8_t  TYPE_OF_POISONSTRENGTH;

typedef uint8_t  TYPE_OF_BODYPARTS;
typedef uint8_t  TYPE_OF_STROKEARMOR;
typedef uint8_t  TYPE_OF_PUNCTUREARMOR;
typedef uint8_t  TYPE_OF_THRUSTARMOR;
typedef uint16_t TYPE_OF_MAGICDISTURBANCE;

typedef uint16_t TYPE_OF_CONTAINERSLOTS;

typedef uint8_t  TYPE_OF_WALKINGCOST;

typedef uint8_t  TYPE_OF_GROUNDLEVEL;
typedef uint8_t  TYPE_OF_SPECIALTILE;
typedef uint8_t  TYPE_OF_SPECIALITEM;
typedef uint16_t TYPE_OF_CATEGORY;
typedef uint8_t  TYPE_OF_AMMUNITIONTYPE;
typedef uint8_t  TYPE_OF_ACTIONPOINTS;

typedef std::string TYPE_OF_GERMAN;
typedef std::string TYPE_OF_ENGLISH;
typedef std::string TYPE_OF_FRENCH;

#endif
