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


#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

const uint32_t DYNNPC_BASE = 0xFF800000;
const uint32_t NPC_BASE = 0xFF000000;
const uint32_t MONSTER_BASE = 0xFE000000;

const uint32_t MONSTERVIEWRANGE = 11;

#define MAXPOISONVALUE     400
#define MAXMANA            10000
#define MAXHPS 10000
#define MAXFOOD 60000
#define MAXATTRIB 255
#define MAXWEIGHT 30000

#define WAITINGVALIDATION 1
#define BANNED 30
#define BANNEDFORTIME 31

#define DEPOTITEM 321
#define DEPOTSIZE 100
#define BLOCKEDITEM 228

#define FLAG_WARPFIELD 1
#define FLAG_SPECIALITEM 2
#define FLAG_BLOCKPATH 4
#define FLAG_MAKEPASSABLE 8
#define FLAG_MONSTERONFIELD 16
#define FLAG_NPCONFIELD 32
#define FLAG_PLAYERONFIELD 64

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
#define MAPDIR          "map/"

//! das Verzeichnis der Skripte, relativ zum DEFAULTMUDDIR
#define SCRIPTSDIR          "scripts/"

//! Anzahl der maximal sichtbaren Ebenen nach Oben
#define RANGEUP       0x02

//! Anzahl der maximal sichtbaren Ebenen nach Unten
#define RANGEDOWN       0x02

//! Anzahl der Felder zwischen zwei Ebenen
#define LEVELDISTANCE 0x03

//! Typ der maximalen Anzahl von Item in einem Container
#define MAXCOUNTTYPE unsigned char

//! Die maximale Anzahl von Item auf einem Feld
#define MAXITEMS 250 // max 255 da oft als BYTE verwendet

//! die maximale Anzahl von Item am Grtel
#define MAX_BELT_SLOTS 6

//! Die maximale Anzahl von Item direkt am K�per
#define MAX_BODY_ITEMS 12

//! Rucksack
#define BACKPACK 0

//! Kopf
#define HEAD 1

//! Kopf-Flag
#define FLAG_HEAD 1

//! Hals
#define NECK 2

//! Hals-Flag
#define FLAG_NECK 2

//! Brustkorb
#define BREAST 3

//! Brustkorb-Flag
#define FLAG_BREAST 4

//! H�de (fr Handschuhe)
#define HANDS 4

//! H�de-Flag
#define FLAG_HANDS 8

//! Werkzeug / Waffe in der linken Hand
#define LEFT_TOOL 5

//! Werkzeug / Waffe in der rechten Hand
#define RIGHT_TOOL 6

//! Finger der linken Hand
#define FINGER_LEFT_HAND 7

//! Finger der rechten Hand
#define FINGER_RIGHT_HAND 8

//! Finger-Flag
#define FLAG_FINGER 32

//! Beine
#define LEGS 9

//! Beine-Flag
#define FLAG_LEGS 64

//! F�
#define FEET 10

//! F�-Flag
#define FLAG_FEET 128

//! Umhang
#define COAT 11

#define LAST_WEARABLE 11

//! Coat-Flag
#define FLAG_COAT 16

#define MAXSHOWCASES 100
#define MAX_DEPOT_SHOWCASE 9

//! Code fr "kein Feld"
#define NOFIELD 0xFFFF


//-------------- Client to Server ---------------------

//! folgender Wert ist relative x und y Koordinaten eines Items/Bodenplatte/Charakters
#define UID_KOORD 0x01

//! folgender Wert ist Showcasenummer+showcaseposition
#define UID_SHOWC 0x02

//! folgender Wert ist Inventory Position
#define UID_INV 0x03

//! Eine Person wird benutzt
#define UID_PERSON 0x05


#define UID_MAGICWAND 0x06

//-------------- Server to Client ---------------------

#define STILLMOVING 0x09
#define NOMOVE 0x0A
#define NORMALMOVE 0x0B
#define PUSH 0x0C
#define RUNNING 0x0D

//! Grund fr Verbindungsabbruch: Client logt aus
#define NORMALLOGOUT       0x00

//! Grund fr Verbindungsabbruch: zu alter Client
#define OLDCLIENT       0x01

//! Grund fr Verbindungsabbruch: Spieler ist schon online
#define DOUBLEPLAYER       0x02

//! Grund fr Verbindungsabbruch: Falsches Pa�ort
#define WRONGPWD         0x03

//! Grund fr Verbindungsabbruch: Servershutdown
#define SERVERSHUTDOWN      0x04

//! Grund fr Verbindungsabbruch: durch Gamemaster entfernt
#define BYGAMEMASTER       0x05

//! Grund fr Verbindungsabbruch: zum Erstellen eines neuen Player
#define FORCREATE            0x06

//! Grund fr Verbindungsabbruch: kein Platz fr den Player
#define NOPLACE               0x07

//! Grund fr Verbindungsabbruch: angegebener Spieler nicht gefunden
#define NOCHARACTERFOUND 0x08

//! Grund fr Verbindungsabbruch: Spieler wurde erstellt
#define PLAYERCREATED 0x09 // string name

//! Grund fr Verbindungsabbruch: UNSTABLECONNECTION
#define UNSTABLECONNECTION 0x0A // string name

//! Reason for Connection shutdown: player has no account
#define NOACCOUNT 0x0B

//! Grund fr Verbindungsabbruch: no skill package chosen
#define NOSKILLS 0x0C

//! Grund fuer Verbindungsabbruch: Spielerdaten korrupt
#define CORRUPTDATA 0x0D

#endif
