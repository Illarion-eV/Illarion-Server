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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <string>

using TYPE_OF_ITEM_ID = uint16_t;
using TYPE_OF_TILE_ID = uint16_t;
using TYPE_OF_CHARACTER_ID = uint32_t;
using TYPE_OF_RACE_ID = uint16_t;
using TYPE_OF_RACE_TYPE_ID = uint16_t;

using TYPE_OF_VOLUME = uint16_t;
using TYPE_OF_WEIGHT = uint16_t;
using TYPE_OF_AGINGSPEED = uint8_t;
using TYPE_OF_BRIGHTNESS = uint8_t;
using TYPE_OF_WORTH = uint32_t;
using TYPE_OF_MAX_STACK = uint16_t;
using TYPE_OF_BUY_STACK = uint16_t;

using TYPE_OF_ITEMLEVEL = uint8_t;

using TYPE_OF_ATTACK = uint8_t;
using TYPE_OF_DEFENCE = uint8_t;
using TYPE_OF_ACCURACY = uint8_t;
using TYPE_OF_RANGE = uint8_t;
using TYPE_OF_WEAPONTYPE = uint8_t;
using TYPE_OF_POISONSTRENGTH = uint8_t;

using TYPE_OF_BODYPARTS = uint8_t;
using TYPE_OF_STROKEARMOR = uint8_t;
using TYPE_OF_PUNCTUREARMOR = uint8_t;
using TYPE_OF_THRUSTARMOR = uint8_t;
using TYPE_OF_MAGICDISTURBANCE = uint16_t;
using TYPE_OF_ARMORTYPE = uint8_t;

using TYPE_OF_CONTAINERSLOTS = uint16_t;

using TYPE_OF_WALKINGCOST = uint16_t;

using TYPE_OF_SPECIALITEM = uint8_t;
using TYPE_OF_AMMUNITIONTYPE = uint8_t;
using TYPE_OF_ACTIONPOINTS = uint8_t;

using TYPE_OF_SKILL_ID = uint8_t;

using TYPE_OF_QUEST_ID = uint16_t;
using TYPE_OF_QUESTSTATUS = int32_t;

using TYPE_OF_GERMAN = std::string;
using TYPE_OF_ENGLISH = std::string;

enum direction {
    dir_north = 0,
    dir_northeast = 1,
    dir_east = 2,
    dir_southeast = 3,
    dir_south = 4,
    dir_southwest = 5,
    dir_west = 6,
    dir_northwest = 7,
    dir_up = 8,
    dir_down = 9,
    dir_none = 10
};

constexpr auto minDirection = 0;
constexpr auto maxDirection = 7;

enum class movement_type { walk = 0, fly = 1, crawl = 2 };

struct Colour {
    static constexpr uint8_t maxColourValue = 0xFF;
    uint8_t red = maxColourValue;
    uint8_t green = maxColourValue;
    uint8_t blue = maxColourValue;
    uint8_t alpha = maxColourValue;

    Colour() = default;
    Colour(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
    Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
            : red(red), green(green), blue(blue), alpha(alpha) {}
};

#endif
