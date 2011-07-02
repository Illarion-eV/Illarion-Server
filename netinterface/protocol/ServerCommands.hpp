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


#ifndef _SERVERCOMMANDS_HPP_
#define _SERVERCOMMANDS_HPP_

#include "NewClientView.hpp"
#include "netinterface/BasicServerCommand.hpp"
//#include "netinterface/NetInterface.hpp"
//#include "globals.hpp"
//#include "Item.hpp"
//#include "World.hpp"
//#include "data/ContainerObjectTable.hpp"
//#include "Logger.hpp"

struct WeatherStruct;
struct UserMenuStruct;
class Item;
typedef std::vector < Item > ITEMVECTOR;

/**
*@defgroup Servercommands Server Commands
*@ingroup Netinterface
*Commands which are sent by the server
*/

/**
*@ingroup Servercommands
*Defines the initialising bytes of the server commands
*/
enum ServerCommands {
    SC_ID_TC = 0xCA,
    SC_SETCOORDINATE_TC = 0xBD,
    SC_MAPSTRIPE_TC = 0xA1,
    SC_MAPCOMPLETE_TC = 0xA2,
    SC_PLAYERSPIN_TC = 0xE0,
    SC_UPDATEINVENTORYPOS_TC = 0xC1,
    SC_CLEARSHOWCASE_TC = 0xC4,
    SC_UPDATEWEATHER_TC = 0xB7,
    SC_UPDATESKILL_TC = 0xD1,
    SC_UPDATEMAGICFLAGS_TC = 0xB8,
    SC_UPDATEATTRIB_TC = 0xB9,
    SC_MUSIC_TC = 0xC8,
    SC_MUSICDEFAULT_TC = 0xCE,
    SC_SAY_TC = 0xD7,
    SC_WHISPER_TC = 0xD5,
    SC_SHOUT_TC = 0xD6,
    SC_INTRODUCE_TC = 0xD4,
    SC_MOVEACK_TC = 0xDF,
    SC_UPDATESHOWCASE_TC = 0xC5,
    SC_STARTPLAYERMENU_TC = 0xBC,
    SC_MAPITEMSWAP = 0xD9,
    SC_GRAPHICEFFECT_TC = 0xC9,
    SC_SOUND_TC = 0xC7,
    SC_ANIMATION_TC = 0xCB,
    SC_BOOK_TC = 0xCD,
    SC_AVIEWPLAYERS_TC = 0xC6,
    SC_ITEMREMOVE_TC = 0xC3,
    SC_ITEMPUT_TC = 0xC2,
    SC_NAMEOFMAPITEM_TC = 0xC0,
    SC_NAMEOFSHOWCASEITEM_TC = 0xBF,
    SC_NAMEOFINVENTORYITEM_TC = 0xBE,
    SC_ATTACKACKKNOWLEDGED_TC = 0xBB,
    SC_TARGETLOST_TC = 0xBA,
    SC_LOGOUT_TC = 0xCC,
    SC_UPDATETIME_TC = 0xB6,
    SC_APPEARANCE_TC = 0xE1,
    SC_REMOVECHAR_TC = 0xE2,
    SC_LOOKATCHARRESULT_TC = 0x18,
    SC_ITEMUPDATE_TC = 0x19

};

/**
*@ingroup Servercommands
*sends an update of all the items on a field (rotting items)
*/
class ItemUpdate_TC : public BasicServerCommand {
public:
    /**
    *sends the whole itemstack of an field
    *@param fieldpos the position of the field which stack is sended
    *@param items the reference to the items which are on the field
    */
    ItemUpdate_TC(position fieldpos, ITEMVECTOR &items);

    /**
    *sends the whole itemstack of an field
    *@param px the x position of the field which stack is sended
    *@param py the y position of the field which stack is sended
    *@param pz the z position of the field which stack is sended
    *@param items the reference to the items which are on the field
    */
    ItemUpdate_TC(int16_t px, int16_t py, int16_t pz, ITEMVECTOR &items);

};

/**
*@ingroup Servercommands
*command when something on a char has changed
*/
class CharDescription : public BasicServerCommand {
public:
    /**
    *sends the Appearance to the client
    *@param id of the char from which the appearance is sended
    *@param appearance the new appearance
    *@param size the sizemodificator of the character (calculated on normal size for this race)
    *@param vbonus bonus for visibility of that character
    *@param lightsource which lightsource item the player holds in his hand
    *@param attackmode 0 = unarmed, 1 = melee weapon, 2 = distance weapon, 3 = staff
    */
    CharDescription(TYPE_OF_CHARACTER_ID id, std::string description);

};

class Character;
/**
*@ingroup Servercommands
*command when something on a char has changed
*/
class AppearanceTC : public BasicServerCommand {
public:
    /**
    *sends the Appearance to the client
    *@param id id of the char for which the appearance is sent
    *@param appearance the new appearance
    *@param size the sizemodificator of the character (calculated on normal size for this race)
    *@param vbonus bonus for visibility of that character
    *@param hair
    *@param beard
    *@param hairred
    *@param hairgreen
    *@param hairblue
    *@param skinred
    *@param skingreen
    *@param skinblue
    *@param head
    *@param breast
    *@param back
    *@param lefthanditem
    *@param righthanditem
    *@param legs
    *@param feet
    *@param attackmode 0 = unarmed, 1 = melee weapon, 2 = distance weapon, 3 = staff
    *@param deathflag
    */
    AppearanceTC(Character *cc);
};


/**
*@ingroup Servercommands
*command for performing character animation
*/
class AnimationTC : public BasicServerCommand {
public:
    AnimationTC(TYPE_OF_CHARACTER_ID id, uint8_t animID);
};


/**
*@ingroup Servercommands
*command for displaying books
*/
class BookTC : public BasicServerCommand {
public:
    BookTC(uint16_t bookID);
};


/**
*@ingroup Servercommands
*command to remove a character
*/
class RemoveCharTC : public BasicServerCommand {
public:
    /**
    *removes the character with given id
    *@param id of the char to be removed
    */
    RemoveCharTC(TYPE_OF_CHARACTER_ID id);
};


/**
*@ingroup Servercommands
*command when the client has to log out
*/
class UpdateTimeTC : public BasicServerCommand {
public:
    /**
    *sends the current ig time to the client
    *@param hour of the ig time
    *@param minute of the ig time
    *@param day of the ig time
    *@param month of the ig time
    *@param year of the ig time
    */
    UpdateTimeTC(unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, short int year);
};

/**
*@ingroup Servercommands
*command when the client has to log out
*/
class LogOutTC : public BasicServerCommand {
public:
    /**
    *send log out command
    *@param reason for the logout
    */
    LogOutTC(unsigned char reason);
};

/**
*@ingroup Servercommands
*command when the current target is lost ( died or out of sight )
*/
class TargetLostTC : public BasicServerCommand {
public:
    /**
    *send attack abbort
    */
    TargetLostTC();
};

/**
*@ingroup Servercommands
*command when an Attack is Ackknowledged
*/
class AttackAckknowledgedTC : public BasicServerCommand {
public:
    /**
    *send ok for the attack
    */
    AttackAckknowledgedTC();
};

/**
*@ingroup Servercommands
*command when a item in inventory is watched
*/
class NameOfInventoryItemTC : public BasicServerCommand {
public:
    /**
    *send name of a inventory tem
    *@param pos in which the name should be displayed
    *@param name which should be displayed
    */
    NameOfInventoryItemTC(unsigned char pos, std::string name);
};

/**
*@ingroup Servercommands
*command when a item in showcase or the menu is looked
*/
class NameOfShowCaseItemTC : public BasicServerCommand {
public:
    /**
    *send name of a showcase menu item
    *@param showcase in which the name should be displayed
    *@param pos in which the name should be displayed
    *@param name which should be displayed
    */
    NameOfShowCaseItemTC(unsigned char showcase, unsigned char pos, std::string name);
};


/**
*@ingroup Servercommands
*command when at an item on the map is looked and the name should be displayes
*/
class NameOfMapItemTC : public BasicServerCommand {
public:
    /**
    *send name of map item
    *@param x x coordinate of the item which was removed
    *@param y y coordinate of the item which was removed
    *@param z z coordinate of the item which was removed
    *@param name name which will be sent
    */
    NameOfMapItemTC(short int x, short int y, short int z, std::string name);
};

/**
*@ingroup Servercommands
*command for putting an item onto the map
*/
class ItemPutTC : public BasicServerCommand {
public:
    /**
    *item is put on the map
    *@param x x coordinate of the item which was put on the map
    *@param y y coordinate of the item which was put on the map
    *@param z z coordinate of the item which was put on the map
    *@param item the item which is putted on the map
    */
    ItemPutTC(short int x, short int y, short int z, Item &item);
};

/**
*@ingroup Servercommands
*command when an item is swapped on the map (simulates only a remove and a put in fast time
*/
class ItemSwapTC : public BasicServerCommand {
public:
    /**
    *item is putted the map
    *@param pos position of the item which should be swapped
    *@param id of the item which was removed
    *@param item the item which is putted on the map
    */
    ItemSwapTC(position pos, unsigned short int id, Item &item);

    /**
    *item is putted the map
    *@param x position of the item which was removed
    *@param y position of the item which was removed
    *@param z position of the item which was removed
    *@param id of the item which was removed
    *@param item the item which is putted on the map
    */
    ItemSwapTC(short int x, short int y, short int z, unsigned short int id, Item &item);
};

/**
*@ingroup Servercommands
*command when an item is removed from the map
*/
class ItemRemoveTC : public BasicServerCommand {
public:
    /**
    *item was removed from the map
    *@param x x coordinate of the item which was removed
    *@param y y coordinate of the item which was removed
    *@param z z coordinate of the item which was removed
    */
    ItemRemoveTC(short int x, short int y, short int z);
};


/**
*@ingroup Servercommands
*command when a admin wants all playerdata
*/
class AViewPlayersTC : public BasicServerCommand {
public:
    /**
    *admin wants all playerdata
    */
    AViewPlayersTC();
};


/**
*@ingroup Servercommands
*command when a sound should be played in the client
*/
class SoundTC : public BasicServerCommand {
public:
    /**
    *sends a sound Effect to the client
    *@param x - x coordinate of the effect
    *@param y - y coordinate of the effect
    *@param z - z coordinate of the effect
    *@param id - id of the effect which should be displayed
    */
    SoundTC(short int x, short int y, short int z, unsigned short int id);
};

/**
*@ingroup Servercommands
*command when a graphiceffect should be displayed in the client
*/
class GraphicEffectTC : public BasicServerCommand {
public:
    /**
    *sends a graphic Effect to the client
    *@param x - x coordinate of the effect
    *@param y - y coordinate of the effect
    *@param z - z coordinate of the effect
    *@param id - id of the effect which should be displayed
    */
    GraphicEffectTC(short int x, short int y, short int z, unsigned short int id);
};

/**
*@ingroup Servercommands
*command when a player is pushed
*/
class PushPlayerTC : public BasicServerCommand {
public:
    /**
    *sends a player pushed command to the client
    *@param xoffs - x offset of the push
    *@param yoffs - y offset of the push
    *@param id - of the character which is pushed
    *@param appearance - of the character which is pushed
    *@param direction - of the character which is pushed
    */
    PushPlayerTC(char xoffs, char yoffs, uint32_t id, unsigned short int appearance, unsigned char direction);
};

/**
*@ingroup Servercommands
*command for starting a player menu
*/
class StartPlayerMenuTC : public BasicServerCommand {
public:
    /**
    *sends a new playermenu to the player
    *@param menu - the menu struct with the entrys for the menu
    */
    StartPlayerMenuTC(UserMenuStruct menu);
};

/**
*@ingroup Servercommands
*command for looking int a showcase
*/
class UpdateShowCaseTC : public BasicServerCommand {
public:
    /**
    *sends the items inside a showcase to the client
    *@param showcase which showcase is ment
    *@param items inside the showcase
    */
    UpdateShowCaseTC(unsigned char showcase, ITEMVECTOR &items);
};

/**
*@ingroup Servercommands
*send a map stripe to the client
*/
class MapStripeTC : public BasicServerCommand {
public:
    /**
    * send new mapstripe to the client
    */
    MapStripeTC(position pos, NewClientView::stripedirection dir);
};


/**
*@ingroup Servercommands
*notify the client that the map is now complete
*/
class MapCompleteTC : public BasicServerCommand {
public:
    /**
    * notify the client that the map is complete
    */
    MapCompleteTC();
};


/**
*@ingroup Servercommands
*a movement was acknowledged send the infos to the client
*/
class MoveAckTC : public BasicServerCommand {
public:
    /**
    *send a move acknowledge to the client
    *@param id character id of the moving character
    *@param pos new position of the character
    *@param mode NOMOVE, NORMALMOVE, PUSH
    *@param waitpages waitpages of the move
    */
    MoveAckTC(TYPE_OF_CHARACTER_ID id, position pos, unsigned char mode, unsigned char waitpages);
};


/**
*@ingroup Servercommands
*send a introduce command
*/
class IntroduceTC : public BasicServerCommand {
public:
    /**
    *send a new introduce command
    *@param id of the character to which i want to introduce
    *@param name of the character
    */
    IntroduceTC(TYPE_OF_CHARACTER_ID id, std::string name);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class ShoutTC : public BasicServerCommand {
public:
    /**
    *send a new talk command
    *@param x - coordinate of the talker
    *@param y - coordinate of the talker
    *@param z - coordinate of the talker
    *@param text - what was said
    */
    ShoutTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class WhisperTC : public BasicServerCommand {
public:
    /**
    *send a new talk command
    *@param x - coordinate of the talker
    *@param y - coordinate of the talker
    *@param z - coordinate of the talker
    *@param text - what was said
    */
    WhisperTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class SayTC : public BasicServerCommand {
public:
    /**
    *send a new talk command
    *@param x - coordinate of the talker
    *@param y - coordinate of the talker
    *@param z - coordinate of the talker
    *@param text - what was said
    */
    SayTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*client should play custom music
*/
class MusicTC : public BasicServerCommand {
public:
    /**
    *play new music
    *@param title the number of the title which should be played
    */
    MusicTC(short int title);
};

/**
*@ingroup Servercommands
*client should play default music
*/
class MusicDefaultTC : public BasicServerCommand {
public:
    /**
    *play default music
    */
    MusicDefaultTC();
};

/**
*@ingroup Servercommands
*command which sends a attrib to the client
*/
class UpdateAttribTC : public BasicServerCommand {
public:
    /**
    *command for sending a attribute
    *@param name of the attribute
    *@param value of the attribute
    */
    UpdateAttribTC(std::string name, short int value);
};


/**
*@ingroup Servercommands
*command which sends the current magicflags to the client
*/
class UpdateMagicFlagsTC : public BasicServerCommand {
public:
    /**
    *command for sending the magic flags to the client.
    *@param type of magican
    *@param flags for the magic type
    */
    UpdateMagicFlagsTC(unsigned char type, uint32_t flags);
};



/**
*@ingroup Servercommands
*command that a showcase is closed
*/
class ClearShowCaseTC : public BasicServerCommand {
public:
    /**
    *a showcase was closed, send the id of it to the client, command
    *@param id of the showcase
    */
    ClearShowCaseTC(unsigned char id);
};

/**
*@ingroup Servercommands
*command which sends a skill to the client
*/
class UpdateSkillTC : public BasicServerCommand {
public:
    /**
    *sends all skill information for one skill to the client
    *@param name of the skill
    *@param type of the skill
    *@param major value of the skill
    *@param minor value of the skill (currently not sended)
    */
    UpdateSkillTC(std::string name, unsigned char type, unsigned short int major, unsigned short int minor);
};

/**
*@ingroup Servercommands
*command that sends new weather informations to the client
*/
class UpdateWeatherTC : public BasicServerCommand {
public:
    /**
    *sends the current weather to the client
    *@param weather the weatherstruct with the informations
    */
    UpdateWeatherTC(WeatherStruct weather);

    /**
    *sends the current weather to the client
    *@param cd - cloud density
    *@param fd - fog density
    *@param wd - wind dir
    *@param gs - gust strength
    *@param ps - percipitation strength
    *@param pt - percipitation type
    *@param ts - thunderstorm
    *@param tp - temperature
    */
    UpdateWeatherTC(uint8_t cd, uint8_t fd, uint8_t wd, uint8_t gs, uint8_t ps, uint8_t pt, uint8_t ts, uint8_t tp);
};

/**
*@ingroup Servercommands
*command which sends the current id to the client
*normally sent after the login
*/
class IdTC : public BasicServerCommand {
public:
    /**
    *sends the id and the appearance to the client
    *@param id id of the char
    */
    IdTC(int id);
};

/**
*@ingroup Servercommands
*command which sends one item in the inventory to the client
*/
class UpdateInventoryPosTC : public BasicServerCommand {
public:
    /**
    *sends one item in the inventory to a client
    *@param pos of the item in the inventory
    *@param id of the item in the inventory
    *@param number of the items in the inventory
    */
    UpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, unsigned char number);
};

/**
*@ingroup Servercommands
*command which sends the current position of a char to the client
*/
class SetCoordinateTC : public BasicServerCommand {
public:
    /**
    *sends the position to the client
    *@param pos position of the char
    */
    SetCoordinateTC(position pos);

    /**
    *sends the position to the client
    *@param x - coordinate
    *@param y - coordinate
    *@param z - coordinate
    */
    SetCoordinateTC(short int x, short int y, short int z);
};


/**
*@ingroup Servercommands
*command which sends a player spin to the client
*/
class PlayerSpinTC : public BasicServerCommand {
public:
    /**
    *sends a player spin to the client
    *@param faceto the direction of the character
    *@param id the id of the character
    */
    PlayerSpinTC(unsigned char faceto, TYPE_OF_CHARACTER_ID id);
};

#endif

