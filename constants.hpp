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


#ifndef ONSTANTS_HH
#define ONSTANTS_HH

#include <stdint.h>

const uint32_t DYNNPC_BASE = 0xFF800000;
const uint32_t NPC_BASE = 0xFF000000;
const uint32_t MONSTER_BASE = 0xFE000000;

#define MAXPOISONVALUE     400
#define MAXMANA            10000
#ifdef DO_UNCONSCIOUS
#define MAXHPS             15000
#define UNCONSCIOUS        5000
#define DYING              2500
#define UNCONSCIOUSHPGAIN  100
#else
#define MAXHPS 10000
#endif

#define WAITINGVALIDATION 1
#define JAILED 20
#define JAILEDFORTIME 21
#define BANNED 30
#define BANNEDFORTIME 31

#define LEFTHANDMODIFIER 2

#define FIGHTMUSIC 1
#define SHOPMUSIC 2
#define SURFACEMUSIC 3
#define AVEMUSIC 4

#define SOUNDFIELD 1
#define MUSICFIELD 2

#define DEPOTITEM 321
#define BLOCKEDITEM 228

#define LANGUAGECOUNT 3

#define FLAG_GROUNDLEVEL 3 //!< 0,1,2,3

#define FLAG_SPECIALITEM 8 //!< 1 == true

#define FLAG_PENETRATEABLE 16 //!< 0 == true

#define FLAG_TRANSPARENT 32 //!< 0 == true

#define FLAG_PASSABLE 64 //!< 0 == true

#define FLAG_MAKEPASSABLE 128 //!< 1 == true

#define FLAG_MONSTERONFIELD 4 //!< 1 == true

#define FLAG_NPCONFIELD 8 //!< 1 == true

#define FLAG_MAPLEVEL 112 //!< 0 .. 7

#define FLAG_PLAYERONFIELD 128 //!< 1 == true

#define FLAG_SPECIALTILE 128  //!< 1 == true

#define FLAG_WARPFIELD 1 //!< 1 == true

// Verwendung siehe Tabelle:
// WERT|      tiles        |   tilesmoditems   |    clientflags     |     extraflags    |
// ----+-------------------+-------------------+--------------------+-------------------+
// 001 |FLAG_GROUNDLEVEL   |FLAG_GROUNDLEVEL   |FLAG_GROUNDLEVEL    |FLAG_WARPFIELD     |
// ----+-------------------+-------------------+--------------------+-------------------+
// 002 |FLAG_GROUNDLEVEL   |FLAG_GROUNDLEVEL   |FLAG_GROUNDLEVEL    |                   |
// ----+-------------------+-------------------+--------------------+-------------------+
// 004 |                   |                   |FLAG_MONSTERONFIELD |                   |
// ----+-------------------+-------------------+--------------------+-------------------+
// 008 |                   |FLAG_SPECIALITEM   |FLAG_NPCONFIELD     |FLAG_SPECIALITEM   |
// ----+-------------------+-------------------+--------------------+-------------------+
// 016 |FLAG_PENETRATEABLE |FLAG_PENETRATEABLE |FLAG_MAPLEVEL       |FLAG_PENETRATEABLE |
// ----+-------------------+-------------------+--------------------+-------------------+
// 032 |FLAG_TRANSPARENT   |FLAG_TRANSPARENT   |FLAG_MAPLEVEL       |FLAG_TRANSPARENT   |
// ----+-------------------+-------------------+--------------------+-------------------+
// 064 |FLAG_PASSABLE      |FLAG_PASSABLE      |FLAG_MAPLEVEL       |FLAG_PASSABLE      |
// ----+-------------------+-------------------+--------------------+-------------------+
// 128 |FLAG_SPECIALTILE   |                   |FLAG_PLAYERONFIELD  |FLAG_SPECIALTILE   |
// ----+-------------------+-------------------+--------------------+-------------------+

////////////////////////
//! das root - Verzeichnis des Servers
#define DEFAULTMUDDIR "/home/ftp/pub/testserver/" // testserver

//! das Verzeichnis mit den Gespr�hen der Player, relativ zum DEFAULTMUDDIR
#define TALKDIR          "talk/"

//! das Verzeichnis der Karte, relativ zum DEFAULTMUDDIR
#define MAPDIR          "map/"

//! das Verzeichnis der Monster, relativ zum DEFAULTMUDDIR
#define MONSTERDIR       "monster/"

//! das Verzeichnis der NPC, relativ zum DEFAULTMUDDIR
#define NPCDIR          "npc/"

//! das Verzeichnis der Tabellen, relativ zum DEFAULTMUDDIR
#define TABLESDIR          "tables/"

//! das Verzeichnis der Skripte, relativ zum DEFAULTMUDDIR
#define SCRIPTSDIR          "scripts/"

//! das Verzeichnis fr Statistiken, relativ zum DEFAULTMUDDIR
#define STATISTICSDIR          "statistics/"

//! Datei in welche die aktuell aktiven Spieler eingetragen werden
#define ONLINEPLFILE      "onlineplayer.txt"

//! Datei in welche die Anzahl der aktiven Player zu bestimmten Zeitpunkten gespeichert wird
#define HISTORYFILE      "onlinehistory.txt"

//! Datei in der die gr�te vergebene Player-ID steht
#define PLAYERCOUNTERNAME      "playercount.bin"

//! Datei in der die gr�te vergebene Monster-ID steht
#define MONSTERCOUNTERNAME      "monstercount.bin"

//! Datei in der die gr�te vergebene NPC-ID steht
#define NPCCOUNTERNAME      "npccount.bin"

//! Datei in der die Namen der Admins stehen
#define ADMINFILE       "admins.txt"

#define APPROVEDFILE    "approved.txt"

//! Datei mit allen bisher vergebenen Namen
#define NAMEFILE      "names.txt"

//! der Standard Lese-Port fr Verbindungen (aus Serversicht)
#define READPORT_PL      3012  // testserver

//! die aktuell ben�igte Clientversion
#define LIENTVERSION      0x0019

//! die aktuelle Serverversion (wichtig fr die Konvertierung gespeicherter Daten)
#define SERVERVERSION      0x0010

//! Entfernung des Players zum n�dlichen Rand seines Sichtbereiches
//#define RANGENORTH       0x0E

//! Entfernung des Players zum sdlichen Rand seines Sichtbereiches
//#define RANGESOUTH       0x0E

//! Entfernung des Players zum �tlichen Rand seines Sichtbereiches
//#define RANGEEAST       0x0E

//! Entfernung des Players zum westlichen Rand seines Sichtbereiches
//#define RANGEWEST       0x0E

//! Anzahl der maximal sichtbaren Ebenen nach Oben
#define RANGEUP       0x02

//! Anzahl der maximal sichtbaren Ebenen nach Unten
#define RANGEDOWN       0x02

//! Anzahl der Felder zwischen zwei Ebenen
#define LEVELDISTANCE 0x03

//! genutzt zum �erprfen, ob der Spieler unter einem Dach steht
#define NOTHING 0x7FFF

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
#define OAT 11

#define LAST_WEARABLE 11

//! Coat-Flag
#define FLAG_COAT 16

//! Die maximale Anzahl von dargestellten showcases
#define MAXSHOWCASES 2

//! Code fr "kein Feld"
#define NOFIELD 0xFFFF

//! Code fr eine transparente Bodenplatte, die auch eine Ebene darunterliegende Tr/Fenster anzeigt
#define TRANSPARENTDISAPPEAR 0xFFFE

//! transparente Bodenplatte
#define TRANSPARENT 0x0000



//-------------- Client to Server ---------------------

//ParameterID's for use
//! folgender Wert ist Skill (ID oder String?) (im moment noch nicht implementiert)
#define UID_SKILL 0x00

//! folgender Wert ist relative x und y Koordinaten eines Items/Bodenplatte/Charakters
#define UID_KOORD 0x01

//! folgender Wert ist Showcasenummer+showcaseposition
#define UID_SHOWC 0x02

//! folgender Wert ist Inventory Position
#define UID_INV 0x03

//! Variabler Parameter (wird ans Skript durchgereicht)
#define UID_VAR 0x04

//! Eine Person wird benutzt
#define UID_PERSON 0x05


#define UID_MAGICWAND 0x06

//-------------- Server to Client ---------------------

//! Code fr "keine Bewegung" (anstatt der Richtung zu senden)
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
#define ORRUPTDATA 0x0D

const unsigned char MAXTRY = 5;
#endif
