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

using Coordinate = int_fast16_t; ///< Type for world coordinates (x, y, z).

// Item and character types
using TYPE_OF_ITEM_ID = uint16_t;      ///< Item type identifier.
using TYPE_OF_TILE_ID = uint16_t;      ///< Tile type identifier.
using TYPE_OF_CHARACTER_ID = uint32_t; ///< Character/Monster unique identifier.
using TYPE_OF_RACE_ID = uint16_t;      ///< Race identifier.
using TYPE_OF_RACE_TYPE_ID = uint16_t; ///< Race type identifier.

// Item properties
using TYPE_OF_VOLUME = uint16_t;    ///< Item volume.
using TYPE_OF_WEIGHT = uint16_t;    ///< Item weight.
using TYPE_OF_AGINGSPEED = uint8_t; ///< How quickly an item ages.
using TYPE_OF_BRIGHTNESS = uint8_t; ///< Light emitted by an item.
using TYPE_OF_WORTH = uint32_t;     ///< Monetary value of an item.
using TYPE_OF_MAX_STACK = uint16_t; ///< Maximum stack size for an item.
using TYPE_OF_BUY_STACK = uint16_t; ///< Stack size when buying.

using TYPE_OF_ITEMLEVEL = uint8_t; ///< Item level requirement.

// Weapon properties
using TYPE_OF_ATTACK = uint8_t;         ///< Attack value.
using TYPE_OF_DEFENCE = uint8_t;        ///< Defence value.
using TYPE_OF_ACCURACY = uint8_t;       ///< Accuracy value.
using TYPE_OF_RANGE = uint8_t;          ///< Weapon range.
using TYPE_OF_WEAPONTYPE = uint8_t;     ///< Weapon type identifier.
using TYPE_OF_POISONSTRENGTH = uint8_t; ///< Poison strength.

// Armor properties
using TYPE_OF_BODYPARTS = uint8_t;         ///< Body parts covered by armor.
using TYPE_OF_STROKEARMOR = uint8_t;       ///< Protection against slashing damage.
using TYPE_OF_PUNCTUREARMOR = uint8_t;     ///< Protection against piercing damage.
using TYPE_OF_THRUSTARMOR = uint8_t;       ///< Protection against bludgeoning damage.
using TYPE_OF_MAGICDISTURBANCE = uint16_t; ///< Magic interference from armor.
using TYPE_OF_ARMORTYPE = uint8_t;         ///< Armor type identifier.

using TYPE_OF_CONTAINERSLOTS = uint16_t; ///< Number of slots in a container.

using TYPE_OF_WALKINGCOST = uint16_t; ///< Movement cost for terrain.

using TYPE_OF_SPECIALITEM = uint8_t;    ///< Special item flag.
using TYPE_OF_AMMUNITIONTYPE = uint8_t; ///< Ammunition type identifier.
using TYPE_OF_ACTIONPOINTS = uint8_t;   ///< Action points required.

using TYPE_OF_SKILL_ID = uint8_t; ///< Skill identifier.

using TYPE_OF_QUEST_ID = uint16_t;   ///< Quest identifier.
using TYPE_OF_QUESTSTATUS = int32_t; ///< Quest status/progress value.

using TYPE_OF_GERMAN = std::string;  ///< German language string.
using TYPE_OF_ENGLISH = std::string; ///< English language string.

/**
 * @brief Cardinal and vertical movement directions.
 */
enum direction {
    dir_north = 0,     ///< North direction.
    dir_northeast = 1, ///< Northeast direction.
    dir_east = 2,      ///< East direction.
    dir_southeast = 3, ///< Southeast direction.
    dir_south = 4,     ///< South direction.
    dir_southwest = 5, ///< Southwest direction.
    dir_west = 6,      ///< West direction.
    dir_northwest = 7, ///< Northwest direction.
    dir_up = 8,        ///< Upward direction (z-axis).
    dir_down = 9,      ///< Downward direction (z-axis).
    dir_none = 10      ///< No direction/invalid.
};

constexpr auto minDirection = 0; ///< Minimum valid horizontal direction value.
constexpr auto maxDirection = 7; ///< Maximum valid horizontal direction value.

/**
 * @brief Types of character movement.
 */
enum class movement_type {
    walk = 0, ///< Walking on ground.
    fly = 1,  ///< Flying above ground.
    crawl = 2 ///< Crawling/swimming.
};

/**
 * @brief RGBA color representation.
 */
struct Colour {
    static constexpr uint8_t maxColourValue = 0xFF; ///< Maximum value for each color component (255).
    uint8_t red = maxColourValue;                   ///< Red component (0-255).
    uint8_t green = maxColourValue;                 ///< Green component (0-255).
    uint8_t blue = maxColourValue;                  ///< Blue component (0-255).
    uint8_t alpha = maxColourValue;                 ///< Alpha/transparency component (0-255).

    /**
     * @brief Default constructor creating white color (255, 255, 255, 255).
     */
    Colour() = default;

    /**
     * @brief Constructs an opaque color with specified RGB values.
     * @param red Red component (0-255).
     * @param green Green component (0-255).
     * @param blue Blue component (0-255).
     */
    Colour(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}

    /**
     * @brief Constructs a color with specified RGBA values.
     * @param red Red component (0-255).
     * @param green Green component (0-255).
     * @param blue Blue component (0-255).
     * @param alpha Alpha/transparency component (0-255).
     */
    Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
            : red(red), green(green), blue(blue), alpha(alpha) {}
};

#endif
