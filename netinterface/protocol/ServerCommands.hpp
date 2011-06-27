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
//#include "ContainerObjectTable.hpp"
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
enum ServerCommands
{
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
class CItemUpdate_TC : public CBasicServerCommand
{
    public:
        /**
        *sends the whole itemstack of an field
        *@param fieldpos the position of the field which stack is sended
        *@param items the reference to the items which are on the field
        */
        CItemUpdate_TC( position fieldpos, ITEMVECTOR & items );
        
        /**
        *sends the whole itemstack of an field
        *@param px the x position of the field which stack is sended
        *@param py the y position of the field which stack is sended
        *@param pz the z position of the field which stack is sended
        *@param items the reference to the items which are on the field
        */        
        CItemUpdate_TC( int16_t px, int16_t py, int16_t pz, ITEMVECTOR &items ); 

};

/**
*@ingroup Servercommands
*command when something on a char has changed
*/
class CCharDescription : public CBasicServerCommand
{
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
        CCharDescription(TYPE_OF_CHARACTER_ID id, std::string description); 

};

class CCharacter;
/**
*@ingroup Servercommands
*command when something on a char has changed
*/
class CAppearanceTC : public CBasicServerCommand
{
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
        CAppearanceTC( CCharacter* cc );
};


/**
*@ingroup Servercommands
*command for performing character animation
*/
class CAnimationTC : public CBasicServerCommand
{
    public:
        CAnimationTC( TYPE_OF_CHARACTER_ID id, uint8_t animID );
};


/**
*@ingroup Servercommands
*command for displaying books
*/
class CBookTC : public CBasicServerCommand
{
    public:
        CBookTC( uint16_t bookID );
};


/**
*@ingroup Servercommands
*command to remove a character
*/
class CRemoveCharTC : public CBasicServerCommand
{
    public:
        /**
        *removes the character with given id
        *@param id of the char to be removed
        */
        CRemoveCharTC(TYPE_OF_CHARACTER_ID id);
};


/**
*@ingroup Servercommands
*command when the client has to log out
*/
class CUpdateTimeTC : public CBasicServerCommand
{
    public:
        /**
        *sends the current ig time to the client
        *@param hour of the ig time
        *@param minute of the ig time
        *@param day of the ig time
        *@param month of the ig time
        *@param year of the ig time
        */
        CUpdateTimeTC(unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, short int year); 
};

/**
*@ingroup Servercommands
*command when the client has to log out
*/
class CLogOutTC : public CBasicServerCommand
{
    public:
        /**
        *send log out command
        *@param reason for the logout
        */
        CLogOutTC(unsigned char reason); 
};

/**
*@ingroup Servercommands
*command when the current target is lost ( died or out of sight )
*/
class CTargetLostTC : public CBasicServerCommand
{
    public:
        /**
        *send attack abbort
        */
        CTargetLostTC();
};

/**
*@ingroup Servercommands
*command when an Attack is Ackknowledged
*/
class CAttackAckknowledgedTC : public CBasicServerCommand
{
    public:
        /**
        *send ok for the attack
        */
        CAttackAckknowledgedTC();
};

/**
*@ingroup Servercommands
*command when a item in inventory is watched
*/
class CNameOfInventoryItemTC : public CBasicServerCommand
{
    public:
        /**
        *send name of a inventory tem
        *@param pos in which the name should be displayed
        *@param name which should be displayed
        */
        CNameOfInventoryItemTC( unsigned char pos, std::string name );
};

/**
*@ingroup Servercommands
*command when a item in showcase or the menu is looked
*/
class CNameOfShowCaseItemTC : public CBasicServerCommand
{
    public:
        /**
        *send name of a showcase menu item
        *@param showcase in which the name should be displayed
        *@param pos in which the name should be displayed
        *@param name which should be displayed
        */
        CNameOfShowCaseItemTC(unsigned char showcase, unsigned char pos, std::string name );
};


/**
*@ingroup Servercommands
*command when at an item on the map is looked and the name should be displayes
*/
class CNameOfMapItemTC : public CBasicServerCommand
{
    public:
        /**
        *send name of map item
        *@param x x coordinate of the item which was removed
        *@param y y coordinate of the item which was removed
        *@param z z coordinate of the item which was removed
        *@param name name which will be sent
        */
        CNameOfMapItemTC( short int x, short int y, short int z, std::string name );
};

/**
*@ingroup Servercommands
*command for putting an item onto the map
*/
class CItemPutTC : public CBasicServerCommand
{
    public:
        /**
        *item is put on the map
        *@param x x coordinate of the item which was put on the map
        *@param y y coordinate of the item which was put on the map
        *@param z z coordinate of the item which was put on the map
        *@param item the item which is putted on the map
        */
        CItemPutTC( short int x, short int y, short int z, Item &item );
};

/**
*@ingroup Servercommands
*command when an item is swapped on the map (simulates only a remove and a put in fast time
*/
class CItemSwapTC : public CBasicServerCommand
{
    public:
        /**
        *item is putted the map
        *@param pos position of the item which should be swapped
        *@param id of the item which was removed
        *@param item the item which is putted on the map
        */
        CItemSwapTC(position pos, unsigned short int id, Item &item);
        
        /**
        *item is putted the map
        *@param x position of the item which was removed
        *@param y position of the item which was removed
        *@param z position of the item which was removed
        *@param id of the item which was removed
        *@param item the item which is putted on the map
        */
        CItemSwapTC(short int x, short int y, short int z, unsigned short int id, Item &item);
};

/**
*@ingroup Servercommands
*command when an item is removed from the map
*/
class CItemRemoveTC : public CBasicServerCommand
{
    public:
        /**
        *item was removed from the map
        *@param x x coordinate of the item which was removed
        *@param y y coordinate of the item which was removed
        *@param z z coordinate of the item which was removed
        */
        CItemRemoveTC( short int x, short int y, short int z );
};


/**
*@ingroup Servercommands
*command when a admin wants all playerdata
*/
class CAViewPlayersTC : public CBasicServerCommand
{
    public:
        /**
        *admin wants all playerdata
        */
        CAViewPlayersTC();
};


/**
*@ingroup Servercommands
*command when a sound should be played in the client
*/
class CSoundTC : public CBasicServerCommand
{
    public:
        /**
        *sends a sound Effect to the client
        *@param x - x coordinate of the effect
        *@param y - y coordinate of the effect
        *@param z - z coordinate of the effect
        *@param id - id of the effect which should be displayed
        */
        CSoundTC(short int x, short int y, short int z, unsigned short int id);
};

/**
*@ingroup Servercommands
*command when a graphiceffect should be displayed in the client
*/
class CGraphicEffectTC : public CBasicServerCommand
{
    public:
        /**
        *sends a graphic Effect to the client
        *@param x - x coordinate of the effect
        *@param y - y coordinate of the effect
        *@param z - z coordinate of the effect
        *@param id - id of the effect which should be displayed
        */
        CGraphicEffectTC(short int x, short int y, short int z, unsigned short int id);
};

/**
*@ingroup Servercommands
*command when a player is pushed
*/
class CPushPlayerTC : public CBasicServerCommand
{
    public:
        /**
        *sends a player pushed command to the client
        *@param xoffs - x offset of the push
        *@param yoffs - y offset of the push
        *@param id - of the character which is pushed
        *@param appearance - of the character which is pushed
        *@param direction - of the character which is pushed
        */
        CPushPlayerTC(char xoffs, char yoffs, uint32_t id, unsigned short int appearance, unsigned char direction);
};

/**
*@ingroup Servercommands
*command for starting a player menu
*/
class CStartPlayerMenuTC : public CBasicServerCommand
{
    public:
        /**
        *sends a new playermenu to the player
        *@param menu - the menu struct with the entrys for the menu
        */
        CStartPlayerMenuTC(UserMenuStruct menu);
};

/**
*@ingroup Servercommands
*command for looking int a showcase
*/
class CUpdateShowCaseTC : public CBasicServerCommand
{
    public:
        /**
        *sends the items inside a showcase to the client
        *@param showcase which showcase is ment
        *@param items inside the showcase
        */
        CUpdateShowCaseTC(unsigned char showcase, ITEMVECTOR &items);
};

/**
*@ingroup Servercommands
*send a map stripe to the client
*/
class CMapStripeTC : public CBasicServerCommand
{
    public:
        /**
        * send new mapstripe to the client 
        */
        CMapStripeTC( position pos, CNewClientView::stripedirection dir ); 
};


/**
*@ingroup Servercommands
*notify the client that the map is now complete
*/
class CMapCompleteTC : public CBasicServerCommand
{
    public:
        /**
        * notify the client that the map is complete 
        */
        CMapCompleteTC(); 
};


/**
*@ingroup Servercommands
*a movement was acknowledged send the infos to the client
*/
class CMoveAckTC : public CBasicServerCommand
{
    public:
        /**
        *send a move acknowledge to the client
        *@param id character id of the moving character
        *@param pos new position of the character
        *@param mode NOMOVE, NORMALMOVE, PUSH
        *@param waitpages waitpages of the move
        */
        CMoveAckTC( TYPE_OF_CHARACTER_ID id, position pos, unsigned char mode, unsigned char waitpages);
};


/**
*@ingroup Servercommands
*send a introduce command
*/
class CIntroduceTC : public CBasicServerCommand
{
    public:
        /**
        *send a new introduce command
        *@param id of the character to which i want to introduce
        *@param name of the character
        */
        CIntroduceTC(TYPE_OF_CHARACTER_ID id, std::string name);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class CShoutTC : public CBasicServerCommand
{
    public:
        /**
        *send a new talk command
        *@param x - coordinate of the talker
        *@param y - coordinate of the talker
        *@param z - coordinate of the talker
        *@param text - what was said
        */
        CShoutTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class CWhisperTC : public CBasicServerCommand
{
    public:
        /**
        *send a new talk command
        *@param x - coordinate of the talker
        *@param y - coordinate of the talker
        *@param z - coordinate of the talker
        *@param text - what was said
        */
        CWhisperTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*server should send a talk command
*/
class CSayTC : public CBasicServerCommand
{
    public:
        /**
        *send a new talk command
        *@param x - coordinate of the talker
        *@param y - coordinate of the talker
        *@param z - coordinate of the talker
        *@param text - what was said
        */
        CSayTC(int16_t x, int16_t y, int16_t z, std::string text);
};

/**
*@ingroup Servercommands
*client should play custom music
*/
class CMusicTC : public CBasicServerCommand
{
    public:
        /**
        *play new music
        *@param title the number of the title which should be played
        */
        CMusicTC(short int title); 
};

/**
*@ingroup Servercommands
*client should play default music
*/
class CMusicDefaultTC : public CBasicServerCommand
{
    public:
        /**
        *play default music
        */
        CMusicDefaultTC();
};

/**
*@ingroup Servercommands
*command which sends a attrib to the client
*/
class CUpdateAttribTC : public CBasicServerCommand
{
    public:
        /**
        *command for sending a attribute
        *@param name of the attribute
        *@param value of the attribute
        */
        CUpdateAttribTC(std::string name, short int value); 
};


/**
*@ingroup Servercommands
*command which sends the current magicflags to the client
*/
class CUpdateMagicFlagsTC : public CBasicServerCommand
{
    public:
        /**
        *command for sending the magic flags to the client.
        *@param type of magican
        *@param flags for the magic type
        */
        CUpdateMagicFlagsTC(unsigned char type, uint32_t flags); 
};



/**
*@ingroup Servercommands
*command that a showcase is closed 
*/
class CClearShowCaseTC : public CBasicServerCommand
{
    public:
        /**
        *a showcase was closed, send the id of it to the client, command
        *@param id of the showcase
        */
        CClearShowCaseTC(unsigned char id); 
};

/**
*@ingroup Servercommands
*command which sends a skill to the client
*/
class CUpdateSkillTC : public CBasicServerCommand
{
    public:
        /**
        *sends all skill information for one skill to the client
        *@param name of the skill
        *@param type of the skill
        *@param major value of the skill
        *@param minor value of the skill (currently not sended)
        */
        CUpdateSkillTC(std::string name, unsigned char type, unsigned short int major, unsigned short int minor); 
};

/**
*@ingroup Servercommands
*command that sends new weather informations to the client
*/
class CUpdateWeatherTC : public CBasicServerCommand
{
    public:
        /**
        *sends the current weather to the client
        *@param weather the weatherstruct with the informations
        */
        CUpdateWeatherTC( WeatherStruct weather );
        
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
        CUpdateWeatherTC( uint8_t cd, uint8_t fd, uint8_t wd, uint8_t gs, uint8_t ps, uint8_t pt, uint8_t ts, uint8_t tp );
};

/**
*@ingroup Servercommands
*command which sends the current id to the client
*normally sent after the login
*/
class CIdTC : public CBasicServerCommand
{
    public:
        /**
        *sends the id and the appearance to the client
        *@param id id of the char
        */
        CIdTC(int id);
};

/**
*@ingroup Servercommands
*command which sends one item in the inventory to the client
*/
class CUpdateInventoryPosTC : public CBasicServerCommand
{
    public:
        /**
        *sends one item in the inventory to a client
        *@param pos of the item in the inventory
        *@param id of the item in the inventory
        *@param number of the items in the inventory
        */
        CUpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, unsigned char number);
};

/**
*@ingroup Servercommands
*command which sends the current position of a char to the client
*/
class CSetCoordinateTC : public CBasicServerCommand
{
    public:
        /**
        *sends the position to the client
        *@param pos position of the char
        */
        CSetCoordinateTC( position pos );
        
        /**
        *sends the position to the client
        *@param x - coordinate
        *@param y - coordinate
        *@param z - coordinate
        */
        CSetCoordinateTC( short int x, short int y, short int z );
};


/**
*@ingroup Servercommands
*command which sends a player spin to the client
*/
class CPlayerSpinTC : public CBasicServerCommand
{
    public:
        /**
        *sends a player spin to the client
        *@param faceto the direction of the character
        *@param id the id of the character
        */
        CPlayerSpinTC( unsigned char faceto, TYPE_OF_CHARACTER_ID id );
};

#endif

