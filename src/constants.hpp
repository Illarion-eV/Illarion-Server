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

/**
 * @file constants.hpp
 * @brief Global game constants and configuration values.
 *
 * Defines fundamental game parameters including:
 * - Character attribute limits (HP, mana, food, stats)
 * - Map and visibility ranges
 * - Equipment slot indices and body part flags
 * - Character ID ranges for monsters, NPCs, and dynamic NPCs
 * - Tile and field flags for pathfinding and special effects
 * - Network protocol constants
 * - Disconnect reason codes
 * - Language identifiers
 * - Time conversion factors for in-game calendar
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>

#include "types.hpp"

#include <cstdint>
#include <string>

#ifdef SYSLOG
constexpr bool useSysLog = true; ///< Use system logging if compiled with -DSYSLOG
#else
constexpr bool useSysLog = false; ///< Use file-based logging
#endif

constexpr auto illarionBirthTime = 950742000; ///< Unix timestamp of Illarion epoch (Feb 17, 2000, UTC+1)
constexpr auto illarionTimeFactor = 3; ///< In-game time passes 3x faster than real time

constexpr auto maxDataKeyLength = 255; ///< Maximum length for item data map keys
constexpr auto maxDataValueLength = 255; ///< Maximum length for item data map values

constexpr uint32_t DYNNPC_BASE = 0xFF800000; ///< Base ID for dynamically created NPCs
constexpr uint32_t NPC_BASE = 0xFF000000; ///< Base ID for static NPCs
constexpr uint32_t MONSTER_BASE = 0xFE000000; ///< Base ID for monsters
static_assert(MONSTER_BASE < NPC_BASE);
static_assert(NPC_BASE < DYNNPC_BASE);

constexpr auto MONSTERVIEWRANGE = 11; ///< Tiles a monster can see in any direction
constexpr auto MAX_SCREEN_RANGE = 30; ///< Maximum client screen range
constexpr auto MAX_ACT_RANGE = 60; ///< Maximum range for actions/interactions

constexpr auto MAXPOISONVALUE = 400; ///< Maximum poison intensity value
constexpr auto MAXMANA = 10000; ///< Maximum mana points
constexpr auto MAXHPS = 10000; ///< Maximum hit points
constexpr auto MAXFOOD = 60000; ///< Maximum food/satiation value
constexpr auto MAXATTRIB = 255; ///< Maximum value for any character attribute
constexpr auto MAXWEIGHT = 30000; ///< Maximum carry weight in mass units

constexpr auto WAITINGVALIDATION = 1; ///< Account status: awaiting email validation
constexpr auto BANNED = 30; ///< Account status: permanently banned
constexpr auto BANNEDFORTIME = 31; ///< Account status: temporarily banned

constexpr std::array<int, 2> DEPOTITEMS = {321, 4817}; ///< Item IDs that function as depot chests
constexpr auto DEPOTSIZE = 100; ///< Maximum items per depot
constexpr auto BLOCKEDITEM = 228; ///< Item ID used to block map tiles

constexpr auto FLAG_WARPFIELD = 1; ///< Tile flag: triggers teleportation
constexpr auto FLAG_SPECIALITEM = 2; ///< Item/tile flag: has special scripted behavior
constexpr auto FLAG_BLOCKPATH = 4; ///< Tile/item flag: blocks character movement
constexpr auto FLAG_MAKEPASSABLE = 8; ///< Item flag: allows movement through blocking tiles
constexpr auto FLAG_MONSTERONFIELD = 16; ///< Field flag: monster occupies this tile
constexpr auto FLAG_NPCONFIELD = 32; ///< Field flag: NPC occupies this tile
constexpr auto FLAG_PLAYERONFIELD = 64; ///< Field flag: player occupies this tile

// Verwendung siehe Tabelle:
// WERT|      tiles        |   tilesmoditems   |       flags        |
// ----+-------------------+-------------------+--------------------+
// 001 |                   |                   |FLAG_WARPFIELD      |
// ----+-------------------+-------------------+--------------------+
// 002 |                   |FLAG_SPECIALITEM   |FLAG_SPECIALITEM    |
// ----+-------------------+-------------------+--------------------+
// 004 |FLAG_BLOCKPATH     |FLAG_BLOCKPATH     |FLAG_BLOCKPATH      |
// ----+-------------------+-------------------+--------------------+
// 008 |                   |FLAG_MAKEPASSABLE  |FLAG_MAKEPASSABLE   |
// ----+-------------------+-------------------+--------------------+
// 016 |                   |                   |FLAG_MONSTERONFIELD |
// ----+-------------------+-------------------+--------------------+
// 032 |                   |                   |FLAG_NPCONFIELD     |
// ----+-------------------+-------------------+--------------------+
// 064 |                   |                   |FLAG_PLAYERONFIELD  |
// ----+-------------------+-------------------+--------------------+
// 128 |                   |                   |                    |
// ----+-------------------+-------------------+--------------------+

//! das Verzeichnis der Karte, relativ zum DEFAULTMUDDIR
const std::string MAPDIR("map/"); ///< Map files directory path relative to server root

//! das Verzeichnis der Skripte, relativ zum DEFAULTMUDDIR
const std::string SCRIPTSDIR("scripts/"); ///< Lua scripts directory path relative to server root

//! Anzahl der maximal sichtbaren Ebenen nach Oben
constexpr Coordinate RANGEUP = 2; ///< Number of Z-levels visible above player

//! Anzahl der maximal sichtbaren Ebenen nach Unten
constexpr Coordinate RANGEDOWN = 2; ///< Number of Z-levels visible below player

//! Anzahl der Felder zwischen zwei Ebenen
constexpr Coordinate LEVELDISTANCE = 3; ///< Vertical tile distance between Z-levels

//! Typ der maximalen Anzahl von Item in einem Container
using MAXCOUNTTYPE = unsigned char; ///< Type for item count in containers (max 255)

//! Die maximale Anzahl von Item auf einem Feld
constexpr auto MAXITEMS = 250; ///< Maximum items on a single map tile (limited by byte representation)

//! die maximale Anzahl von Item am Grtel
constexpr auto MAX_BELT_SLOTS = 6; ///< Number of quick-access belt slots

//! Die maximale Anzahl von Item direkt am K�per
constexpr auto MAX_BODY_ITEMS = 12; ///< Number of equipment slots on character body

constexpr auto BACKPACK = 0; ///< Equipment slot: backpack/bag
constexpr auto HEAD = 1; ///< Equipment slot: head (helmet)
constexpr auto NECK = 2; ///< Equipment slot: neck (amulet)
constexpr auto BREAST = 3; ///< Equipment slot: chest (armor)
constexpr auto HANDS = 4; ///< Equipment slot: hands (gloves)
constexpr auto LEFT_TOOL = 5; ///< Equipment slot: left hand tool/weapon
constexpr auto RIGHT_TOOL = 6; ///< Equipment slot: right hand tool/weapon
constexpr auto FINGER_LEFT_HAND = 7; ///< Equipment slot: left ring finger
constexpr auto FINGER_RIGHT_HAND = 8; ///< Equipment slot: right ring finger
constexpr auto LEGS = 9; ///< Equipment slot: legs (pants/leggings)
constexpr auto FEET = 10; ///< Equipment slot: feet (boots)
constexpr auto COAT = 11; ///< Equipment slot: coat/cloak
constexpr auto LAST_WEARABLE = 11; ///< Index of last wearable equipment slot
constexpr auto BELT1 = 12; ///< Belt slot 1
constexpr auto BELT2 = 13; ///< Belt slot 2
constexpr auto BELT3 = 14; ///< Belt slot 3
constexpr auto BELT4 = 15; ///< Belt slot 4
constexpr auto BELT5 = 16; ///< Belt slot 5
constexpr auto BELT6 = 17; ///< Belt slot 6

constexpr uint8_t FLAG_HEAD = 0b0000'0001; ///< Body part flag: head slot
constexpr uint8_t FLAG_NECK = 0b0000'0010; ///< Body part flag: neck slot
constexpr uint8_t FLAG_BREAST = 0b0000'0100; ///< Body part flag: chest slot
constexpr uint8_t FLAG_HANDS = 0b0000'1000; ///< Body part flag: hands slot
constexpr uint8_t FLAG_COAT = 0b0001'0000; ///< Body part flag: coat slot
constexpr uint8_t FLAG_FINGER = 0b0010'0000; ///< Body part flag: finger slots
constexpr uint8_t FLAG_LEGS = 0b0100'0000; ///< Body part flag: legs slot
constexpr uint8_t FLAG_FEET = 0b1000'0000; ///< Body part flag: feet slot
constexpr uint8_t FLAG_ALL_SLOTS = 0b1111'1111; ///< Body part flag: all equipment slots

constexpr auto MAXSHOWCASES = 100; ///< Maximum number of open containers per player
constexpr auto MAX_DEPOT_SHOWCASE = 9; ///< Maximum depot containers that can be open simultaneously

//! Code fr "kein Feld"
constexpr auto NOFIELD = 0xFFFF; ///< Invalid/no field marker value

//-------------- Client to Server ---------------------

//! folgender Wert ist relative x und y Koordinaten eines Items/Bodenplatte/Charakters
constexpr auto UID_KOORD = 0x01; ///< Network protocol: item reference by map coordinates

//! folgender Wert ist Showcasenummer+showcaseposition
constexpr auto UID_SHOWC = 0x02; ///< Network protocol: item reference by container ID and position

//! folgender Wert ist Inventory Position
constexpr auto UID_INV = 0x03; ///< Network protocol: item reference by inventory slot

//! Eine Person wird benutzt
constexpr auto UID_PERSON = 0x05; ///< Network protocol: character interaction

constexpr auto UID_MAGICWAND = 0x06; ///< Network protocol: magic wand usage

//-------------- Server to Client ---------------------

constexpr auto STILLMOVING = 0x09; ///< Movement type: character still in motion
constexpr auto NOMOVE = 0x0A; ///< Movement type: no movement occurred
constexpr auto NORMALMOVE = 0x0B; ///< Movement type: standard walking
constexpr auto PUSH = 0x0C; ///< Movement type: pushed by another character
constexpr auto RUNNING = 0x0D; ///< Movement type: running

//! Grund fr Verbindungsabbruch: Client logt aus
constexpr auto NORMALLOGOUT = 0x00; ///< Disconnect reason: normal logout

//! Grund fr Verbindungsabbruch: zu alter Client
constexpr auto OLDCLIENT = 0x01; ///< Disconnect reason: client version too old

//! Grund fr Verbindungsabbruch: Spieler ist schon online
constexpr auto DOUBLEPLAYER = 0x02; ///< Disconnect reason: character already logged in

//! Grund fr Verbindungsabbruch: Falsches Pa�ort
constexpr auto WRONGPWD = 0x03; ///< Disconnect reason: incorrect password

//! Grund fr Verbindungsabbruch: Servershutdown
constexpr auto SERVERSHUTDOWN = 0x04; ///< Disconnect reason: server shutting down

//! Grund fr Verbindungsabbruch: durch Gamemaster entfernt
constexpr auto BYGAMEMASTER = 0x05; ///< Disconnect reason: kicked by GM

//! Grund fr Verbindungsabbruch: zum Erstellen eines neuen Player
constexpr auto FORCREATE = 0x06; ///< Disconnect reason: redirect to character creation

//! Grund fr Verbindungsabbruch: kein Platz fr den Player
constexpr auto NOPLACE = 0x07; ///< Disconnect reason: no spawn position available

//! Grund fr Verbindungsabbruch: angegebener Spieler nicht gefunden
constexpr auto NOCHARACTERFOUND = 0x08; ///< Disconnect reason: character not found in database

//! Grund fr Verbindungsabbruch: Spieler wurde erstellt
constexpr auto PLAYERCREATED = 0x09; ///< Disconnect reason: character successfully created

//! Grund fr Verbindungsabbruch: UNSTABLECONNECTION
constexpr auto UNSTABLECONNECTION = 0x0A; ///< Disconnect reason: connection quality issues

//! Reason for Connection shutdown: player has no account
constexpr auto NOACCOUNT = 0x0B; ///< Disconnect reason: account does not exist

//! Grund fr Verbindungsabbruch: no skill package chosen
constexpr auto NOSKILLS = 0x0C; ///< Disconnect reason: no skill package selected during creation

//! Grund fuer Verbindungsabbruch: Spielerdaten korrupt
constexpr auto CORRUPTDATA = 0x0D; ///< Disconnect reason: corrupted character data

constexpr auto dbStatusNoSkills = 7; ///< Database status code: character has no skills assigned

constexpr auto languageCommon = 0; ///< Language ID: common tongue (understood by all)
constexpr auto languageHuman = 1; ///< Language ID: human language
constexpr auto languageDwarf = 2; ///< Language ID: dwarven language
constexpr auto languageElf = 3; ///< Language ID: elven language
constexpr auto languageLizard = 4; ///< Language ID: lizardman language
constexpr auto languageOrc = 5; ///< Language ID: orcish language
constexpr auto languageHalfling = 6; ///< Language ID: halfling language
constexpr auto languageFairy = 7; ///< Language ID: fairy language
constexpr auto languageGnome = 8; ///< Language ID: gnomish language
constexpr auto languageGoblin = 9; ///< Language ID: goblin language
constexpr auto languageAncient = 10; ///< Language ID: ancient/magical language

constexpr auto maximumMajorSkill = 100; ///< Maximum skill level for major skills
constexpr auto maximumMinorSkill = 10000; ///< Maximum skill value for minor skills

constexpr auto BBIWIClientVersion = 200; ///< Required BBIWI (admin client) protocol version
#endif
