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


#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <stdexcept>

class WaypointList;
class Field;
class InputDialog;
class MessageDialog;

/**
* thrown if a creature can't be set due to space problems
*/
class NoSpace : public std::runtime_error {
public:
    /**
    *a NoSpace error is thrown
    */
    NoSpace(std::string s = "no space") : std::runtime_error(s) {}
};

/**
* forward declarations
*/
class World;

/**
* enum which defines the magic type of an character
* @warning arrayaccess with theese values
*/
enum magic_type {
    MAGE=0, /**< character is a mage */
    PRIEST=1,  /**< character is a priest */
    BARD=2,  /**< character is a bard */
    DRUID=3  /**< character is a druid */
};

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/* #define Character_DEBUG */

#include <string>
#include <fstream>
#include <map>
#include <boost/noncopyable.hpp>
#include "globals.hpp"
#include "constants.hpp"
#include "TableStructs.hpp"
#include "Item.hpp"
#include "UserMenuStruct.hpp"
//hier ggf weitere includes
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "LongTimeCharacterEffects.hpp"

// some forward declarations for classes used exclusively as pointers to save compilation time
class Container;


/**
* @ingroup Scriptclasses
* the definition of an character
* is noncopyable
* cnpc, Monster, Player are from this type
* exported to lua
*/
class Character : private boost::noncopyable {

public:

    /**
    *@name ccharacter Lua Defintions:
    *definitions which are exported to lua like enums
    */

    //@{
    /**
    *======================start grouping Lua Definitions===================
    *@ingroup Scriptenums
    */

    /**
    *shows from which type the character is
    *<b>Lua: [character_type]</b>
    */
    enum character_type {
        player = 0, /**< the character is from type Player */
        monster = 1, /**< the character is from type Monster */
        npc = 2 /**< the character is from Type NPC */
    };


    /**
    * shows how loud the character talks
    *<b>Lua: [talk_type]</b>
    */
    enum talk_type {
        tt_say=0, /**< character talks normal */
        tt_whisper=1, /**< character whispers */
        tt_yell=2 /**< character shouts */
    };

    /**
    * sex type of the character
    *<b>Lua: [sex_type]</b>
    */
    enum sex_type {
        male = 0, /**< the character is male */
        female = 1, /**< the character is female */
        neuter = 2 /**< the character is nor male nor female*/
    };

    /**
    *shows in which direction the character is looking
    *<b>Lua: [face_to]</b>
    */
    enum face_to {
        north = 0,
        northeast = 1,
        east = 2,
        southeast = 3,
        south = 4,
        southwest = 5,
        west = 6,
        northwest = 7
    };

    /**
    * shows a direction
    *<b>Lua: [direction]</b>
    */
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
        dir_down = 9
    };

    /**
    * @ingroup Scriptclasses
    * one skill entry in the skillmap
    *<b>Lua: [skillvalue]</b>
    */
    struct skillvalue {
        /**
        *@name skillvalue Lua Variables:
        *to Lua exported Variables
        */

        //@{
        /**
        *=========================Lua variables==================
        */

        /**
        *type of the skill (working, fighting, magic ... )
        * @see constants.h
        *<b>Lua: [type]</b>
        */
        unsigned char type;

        /**
        * larger value of this skill
        * <b>Lua: [major]</b>
        */
        unsigned short int major;

        /**
        * minor value of this skill
        * <b>Lua: [minor]</b>
        */
        unsigned short int minor;

        /**
        * number of unsuccessfull trys increasing this skil
        * <b>Lua: [firsttry]</b>
        */
        uint16_t firsttry; /**< number of unsuccesful first trys*/

        /**
        * constructor initializes all values with 0
        *<b>Lua</b>
        */
        skillvalue() {
            type = 0;
            major = 0;
            minor = 0;
            firsttry = 0;
        }

        /**
        *======================end of group lua variables============
        */
        //@}

    };



    /**
    *======================end of group Lua Definitions===================
    */
    //@}

    /**
    * @ingroup Scriptvariables
    * @name ccharacter Lua Variables:
    * the Variables which are exported to Lua
    */

    //@{
    /**
    *======================start grouping Lua Variables===================
    *@ingroup Scriptvariables
    */

    /**
    * name of the character
    *<b>Lua: (ro) [name]</b>
    */
    std::string name;

    /**
    * current action points of the character
    * <b>Lua: (r/w) [movepoints]</b>
    */
    short int actionPoints;

    /**
    * current fight points of the character
    * <b>Lua: (r/w) [fightpoints]</b>
    */
    short int fightPoints;

    /**
    * current active language of the character
    * 0 = common Language ...
    *<b>Lua: (r/w) [activeLanguage]</b>
    */
    short int activeLanguage;

    /**
    * id of the Character
    *<b>Lua: (ro) [name]</b>
    */
    TYPE_OF_CHARACTER_ID id;

    /**
    * position on the map
    *<b>Lua: (ro) [pos]</b>
    */
    position pos;

    /**
    * is true if the character attacks someone
    * <b>Lua: (ro) [attackmode]</b>
    */
    bool attackmode;

    uint8_t hair;
    uint8_t beard;
    uint8_t hairred;
    uint8_t hairgreen;
    uint8_t hairblue;
    uint8_t skinred;
    uint8_t skingreen;
    uint8_t skinblue;

    /**
    * stores the last message the character has talked
    * used in scripts
    *<b>Lua: (ro) [lastSpokenText]</b>
    */
    std::string lastSpokenText;

    /**
    *the effects which are currently working on the character
    */
    LongTimeCharacterEffects *effects;

    WaypointList *waypoints;

    /**
    *======================end of group Lua Variables===================
    */
    //@}

    /**
    *@ingroup Scriptfunctions
    *@name ccharacter Lua Functions:
    *functions which are exported to lua
    */

    //@{
    /**
    *======================start grouping Lua Functions===================
    *@ingroup Scriptfunctions
    */

    position getFrontalPosition();

    /**
    * checks if there is a action currently running
    * <b>Lua: [:actionRunning]</b>
    * @return true if an action is running otherwise false
    */
    inline virtual bool actionRunning() {
        return false;
    }

    /**
    * returns the magic type of a character
    * <b>Lua: [:getMagicType]</b>
    * @return a number which represents the current magic type
    */
    inline unsigned short get_magic_type() {
        return magic.type;
    }

    /**
    * sets a new magic type to the character
    * <b>Lua: [:setMagicType]</b>
    * @param newMagType the new magic type of the character
    */
    inline virtual void set_magic_type(magic_type newMagType) {
        magic.type=newMagType;
    }

    void setOnRoute(bool onr) {
        _is_on_route = onr;
    }

    bool getOnRoute() {
        return _is_on_route;
    }

    /**
    * gets the ooc language of the player
    * <b>Lua: [:getPlayerLanguage()]</b>
    * @return the number which determines the player language 0 german, 1 english
    */
    inline virtual const short unsigned int getPlayerLanguage() {
        return 1;   //nothing to do at all
    } //mother_tongue}

    /**
    * returns the magic flags of the specified magic type of the char
    * <b>Lua: [:getMagicFlags]</b>
    * @param type magictype from which we want to know the runes
    * @return the flags of this magic type
    */
    inline unsigned long int get_magic_flags(unsigned char type) {
        return magic.flags[type];
    }

    /**
    * returns the character type of this char
    * <b>Lua: [getType]</b>
    * @return the number of the character type
    * @see character_type
    */
    inline unsigned short get_character() {
        return character;
    }

    /**
    * returns the race type number of this character
    * <b>Lua: [:get_race]</b>
    * @return the number of the racetype
    */
    inline unsigned short get_race() {
        return race;
    }

    /**
    * returns the current direction of the character
    * <b>Lua: [:get_face_to]</b>
    * @return the number of the current direction
    */
    inline unsigned short get_face_to() {
        return faceto;
    }

    /**
    * determines if the current char is a admin or not (use only in Player)
    * <b>Lua: [:isAdmin]</b>
    * @return true if the char is an admin otherwise false
    */
    inline virtual bool isAdmin() {
        return false;
    }

    /**
    * <b>Lua: [:getType]</b>
    * returns the monstertype
    * @return the monster type, < 0 for no monster
    */
    inline virtual TYPE_OF_CHARACTER_ID getType() {
        return 0;
    }

    /**
    * changes the quality of an item at a specific position
    * <b>Lua: [:changeQualityAt]</b>
    * @param pos the position of this item
    * @param amount how much should the quality beeing changed
    */
    virtual void changeQualityAt(unsigned char pos, short int amount);

    /**
    * <b>Lua: [:changeQualityItem]</b>
    * changes the Quality of an Item with a specific ID
    * @param id of the item which should be changed
    * @param amount how much the quality should be changed
    */
    virtual void changeQualityItem(TYPE_OF_ITEM_ID id, short int amount);

    /**
    * <b>Lua: [:increasePoisonValue]</b>
    * increase or decreases the poison value of this character
    * @param value the amount how much the poision value should be increased relative
    */
    virtual void increasePoisonValue(short int value);

    /**
    * returns the current poison value
    * @return the poison value of this character
    */
    inline short int getPoisonValue() {
        return poisonvalue;
    }

    /**
    * sets the poison value absolutely
    * <b>Lua: [:setPoisonValue]</b>
    * @param value the new value of poison
    */
    inline void setPoisonValue(short int value) {
        poisonvalue = value;
    }

    /**
    * starts a new longtime action for this character (overloaded in Player)
    * <b>Lua: [:startAction]</b>
    * @param wait time to wait until the action is successfull
    * @param ani the animation which should be shown for this action, if not set this value is 0
    * @param redoani after how much 1/10s the animation should be shown again, if not set this value is 0 (0 never)
    * @param sound the sound which should be played for this action, if not set this value is 0
    * @param redosound after hoch much 1/10s the sound is played again, if not set this value is 0, 0 means never
    */
    inline virtual void startAction(unsigned short int wait, unsigned short int ani=0, unsigned short int redoani=0, unsigned short int sound=0, unsigned short int redosound=0) {
        /**nothing to do here**/
    }

    /**
    * aborts the last long time action (overloaded in Player)
    * <b>Lua: [:abortAction]</b>
    */
    inline virtual void abortAction() {
        /**nothing to do only for players**/
    }

    /**
    * lets sucessfull complete a long time action for this character
    * overloaded in Player
    * <b>Lua: [:successAction]</b>
    */
    inline virtual void successAction() {
        /**nothing to do only for player**/
    }

    /**
    * a long time action was disturbed by someone
    * overloaded in Player
    * <b>Lua: [:actionDisturbed]</b>
    * @param disturber the character which has dirsturbed this action
    */
    inline virtual void actionDisturbed(Character *disturber) {
        /**nothing to do only for player**/
    }

    /**
    *changes the source of the last action of this character
    *<b>Lua: [:changeSource]</b>
    *@param cc source is a character the pointer to this character
    */
    inline virtual void changeSource(Character *cc) {
        /**nothing to do only for player**/
    }

    /**
    *changes the source of the last action for this character
    *<b>Lua: [:changeSource]</b>
    *@param sI source is a item the new item
    */
    inline virtual void changeSource(ScriptItem sI) {
        /**nothing to do only for player**/
    }

    /**
    *changes the Source of the last action for this character.
    *<b>Lua: [:changeSource]</b>
    *@param pos source is a position the new position
    */
    inline virtual void changeSource(position pos) {
        /**nothing to do only for player**/
    }

    /**
    *changes the Source of the last action to nothing for this character
    *<b>Lua: [:changeSource]</b>
    */
    inline virtual void changeSource() {
        /**nothing to do only for player**/
    }

    /**
    *changes the Target of the last action for this character.
    *<b>Lua: [:changeTarget]</b>
    *@param cc target is a character the pointer to this character
    */
    inline virtual void changeTarget(Character *cc) {
        /**nothing to do only for player**/
    }

    /**
    *changes the Target of the last action for this character.
    *<b>Lua: [:changeTarget]</b>
    *@param sI target is a item the new item
    */
    inline virtual void changeTarget(ScriptItem sI) {
        /**nothing to do only for player**/
    }

    /**
    *changes the target of the last action for this character.
    *<b>Lua: [:changeTarget]</b>
    *@param pos Target is a position the new position
    */
    inline virtual void changeTarget(position pos) {
        /**nothing to do only for player**/
    }

    /**
    *changes the Target of the last action to nothing for this character
    *<b>Lua: [:changeTarget]</b>
    */
    inline virtual void changeTarget() {
        /**nothing to do only for player**/
    }

    /**
    * returns the current mental capacity
    * <b>Lua: [:getMentalCapacity]</b>
    * @return the current mental capacity of this character
    */
    inline int getMentalCapacity() {
        return mental_capacity;
    }

    /**
    * sets the abolut value of the mental capacity
    * <b>Lua: [:setMentalCapacity]</b>
    * @param value the new value of mental capacity
    */
    inline void setMentalCapacity(int value) {
        mental_capacity = value;
    }

    /**
    * increases the amount of the mental capacity relativ
    * <b>Lua: [:increaseMentalCapacity]</b>
    * @param value how much the mental capacity should be increased or decreased
    */
    virtual void increaseMentalCapacity(int value);

    /**
     * count how much items with a specific id the character has in his inventory or backpack
     * <b>Lua: [:countItem]</b>
     * @param itemid the id of the item which should be counted
     * @return the number of items with itemid which the character is carring
     */
    int countItem(TYPE_OF_ITEM_ID itemid);


    /**
    * counts the number of items with a specific id at a special place of the character
    * <b>Lua: [:countItemAt]</b>
    * @param where where should the items counted ("all","belt","body","backpack")
    * @param itemid id of the items which should be counted
    * @return number of the items with itemid the character carries at the specific place
    */
    int countItemAt(std::string where, TYPE_OF_ITEM_ID itemid);

    /**
    * counts the number of items with a specific id and a specific data at a special place of the character
    * <b>Lua: [:countItemAt]</b>
    * @param where where should the items counted ("all","belt","body","backpack")
    * @param itemid id of the items which should be counted
    * @param data the date of the item
    * @return number of the items with itemid the character carries at the specific place
    */
    int countItemAt(std::string where, TYPE_OF_ITEM_ID itemid, uint32_t data);

    /**
    * deletes x elements of an Item with a specific id
    * <b>Lua: [:eraseItem]</b>
    * @param itemid the id of the items which should be deleted
    * @param count the number of items which should be deleted
    * @return the number of item which where sucessfully deleted (can be smaller than count)
    */
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count);

    /**
    * deletes x elements of an Item with a specific id and data
    * <b>Lua: [:eraseItem]</b>
    * @param itemid the id of the items which should be deleted
    * @param count the number of items which should be deleted
    * @param data the data value of items which should be deleted
    * @return the number of item which where sucessfully deleted (can be smaller than count)
    */
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count, uint32_t data);

    /**
    * creates count items with a specific id in the inventory of the character
    * <b>Lua: [:createItem]</b>
    * @param itemid the id of the item which should be created
    * @param count how much of the items should be created
    * @param quali the quality of the new created items
    * @param data the data value of the new created items
    * @return the number of items which where created sucessfully
    */
    virtual int createItem(TYPE_OF_ITEM_ID itemid, uint8_t count, uint16_t quali, uint32_t data);

    /**
    * changes the amount of items at a specific position
    * <b>Lua: [:increaseAtPos]</b>
    * @param pos the position where the number of items should be increased or decreased
    * @param count the number of increasage or decreasage
    * @return the number of items which where deleted/added
    */
    virtual int increaseAtPos(unsigned char pos, int count);

    virtual void setDeathConsequences(bool dc) {
        death_consequences = dc;
    }
    virtual bool getDeathConsequences() const {
        return death_consequences;
    }

    /**
    * creates a amount of items at a specific pos on the character
    * <b>Lua: [:increaseAtPos]</b>
    * @param pos where the items should be created
    * @param newid id of the item which should be created
    * @param count how much items should be created
    * @return the number of items which where created
    */
    virtual int createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count);

    /**
    * changes a item at the specific pos
    * <b>Lua: [:swapAtPos]</b>
    * @param pos where the item should be changed
    * @param newid the new id of this item
    * @param newQuality new Quality of the items (if not set it is set to 0)
    */
    virtual bool swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);

    /**
    * returns a scriptItem at the specific bodypos
    * <b>Lua: [:getItemAt]</b>
    * @param itempos the position from which we want to get the scriptitem struct
    * @return the scriptitem struct of the item at this position
    * @see ScriptItem
    */
    ScriptItem GetItemAt(unsigned char itempos);

    /**
    * returns a pointer to the backpack
    * <b>Lua: [:getBackPack]</b>
    * @return pointer to the current backpack of the character
    */
    Container *GetBackPack();

    /**
    * returns a pointer to a depot of the character
    * <b>Lua: [:getDepot]</b>
    * @param depotid the id of the depot from which we want the pointer
    * @return pointer to this depot
    */
    Container *GetDepot(uint32_t depotid);

    /**
    * returns a list of all items which the char has with a specific id
    * <b>Lua: [:getItemList]</b>
    * @param id the id of the item which is searched
    */
    luabind::object getItemList(TYPE_OF_ITEM_ID id);

    /**
    *returns a list with all items the char is wearing
    * <b>Lua: [:getItemList]</b>
    *@param depots, if true all depots where also searched
    *@return a lua list with all items as scriptitems
    */
    //luabind::object getItemList(bool depots = false);

    /**
    *returns a list with all items inside a specific depot
    * <b>Lua: [:getItemListDepot]</b>
    *@param depotid id of the depot which should be searched
    *@return a list with all the items as scriptitems
    */
    //luabind::object getItemListDepot(uint16_t depotid);

    /**
    *returns a list with all items inside all depots
    * <b>Lua: [:getItemListDepot]</b>
    *@return a list with all items as scriptitems
    */
    //luabind::object getItemListDepot();

    /**
    *returns a list with all the items from a specific container
    * <b>Lua: [:getItemListDepot]</b>
    *@param itempos, the pos of the container
    *@return a list with all items inside the container
    */
    //luabind::object getItemListContainer(unsigned char itempos);

    /**
    *returns a list with all the items from a specific container
    * <b>Lua: [:getItemListDepot]</b>
    *@param item, the item which is the container
    *@return a list with all items inside the container
    */
    //luabind::object getItemListContainer(ScriptItem item);

    /**
    * returns the major skill value of a specific skill
    * <b>Lua: [:getSkill]</b>
    * @param s the name of the skill where the value should be returned
    * @return 0 if there exist no skill with name s otherwise the skillvalue of this skill
    */
    unsigned short int getSkill(std::string s);

    /**
    * returns the skillvalue of a specific skill
    * <b>Lua: [:getSkillValue]</b>
    * @param s the name of the skill where the value should be returned
    * @return nil if there exist no skill with name s otherwise the skillvalue of this skill
    */
    skillvalue *getSkillValue(std::string s);

    /**
    * returns the minor skill value of a specific skill
    * <b>Lua: [:getMinorSkill]</b>
    * @param s the name of the skill where the value should be returned
    * @return 0 if there exist no skill with name s otherwise the skillvalue of this skill
    */
    unsigned short int getMinorSkill(std::string s);

    void setSkinColor(uint8_t red, uint8_t green, uint8_t blue);


    void getSkinColor(uint8_t &red, uint8_t &green, uint8_t &blue);


    void setHairColor(uint8_t red, uint8_t green, uint8_t blue);


    void getHairColor(uint8_t &red, uint8_t &green, uint8_t &blue);


    void setHair(uint8_t hairID);


    uint8_t getHair();

    void setBeard(uint8_t beardID);

    uint8_t getBeard();



    /**
    * changes the value of an attribute
    * <b>Lua: [:increaseAttrib]</b>
    * @param name of the attribute which should be changed
    * @param amount how much this attribute should be changed
    * @return the attribute value after the change
    */
    virtual unsigned short int increaseAttrib(std::string name, short int amount);


    /**
    * changes one attrib temporary
    * <b>Lua: [:tempChangeAttrib]</b>
    *@param name name of the attrib which should be changed
    *@param amount how much this attrib should be changed
    *@param time how much seconds the temporary changing should hold
    */
    virtual void tempChangeAttrib(std::string name, short int amount, uint16_t time);


    /**
    * changes a attribute aboslute
    * <b>Lua: [:setAttrib]</b>
    * @param name of the attribute which should be changed
    * @param wert the new value of this attribute
    */
    void setAttrib(std::string name, short int wert);

    /**
    * changes the major value of a skill or adds this skill to the skillmap of the character
    * <b>Lua: [:increaseSkill]</b>
    * @param typ the type of the skill which should be added/increased/decreased
    * @param name of the skill which should be added/increased/decreased
    * @param amount how much should be added/decreased to the skill
    * @return the major value after the changes
    */
    virtual unsigned short int increaseSkill(unsigned char typ, std::string name, short int amount);

    /**
    * changes the minor value of a skill or adds this skill to the skillmap of the character
    * <b>Lua: [:increaseMinorSkill]</b>
    * @param typ the type of the skill which should be added/increased/decreased
    * @param name of the skill which should be added/increased/decreased
    * @param amount how much should be added/decreased to the skill
    * @return the major value after the changes
    */
    virtual unsigned short int increaseMinorSkill(unsigned char typ, std::string name, short int amount);


    /**
    * try to learn a skill
    * <b>Lua: [:learn]</b>
    * @param skill name of the skill the user tries to learn
    * @param skillGroup group of the given skill
    * @param actionPoints cost of learning
    * @param opponent gives information until which skill level learning may be successful
    * @param leadAttrib value of the lead attribute responsible for how successful learning the given skill is
    */
    virtual void learn(std::string skill, uint8_t skillGroup, uint32_t actionPoints, uint8_t opponent, uint8_t leadAttrib);

    /**
    * the character learns a new magical rune
    * <b>Lua: [:teachMagic]</b>
    * @param type of magic he learns (bard, priest, druidic, magic)
    * @param flag the rune he wants to learn
    */
    virtual void teachMagic(unsigned char type, unsigned char flag);

    /**
    * checks if the character is in range to another character
    * <b>Lua: [:isInRange]</b>
    * @param cc character from which we want to check if he is in range
    * @param distancemetric the number of fields around the character which we want to check
    * @return true if cc is < distancemetric fields away from this character otherwise false
    */
    bool isInRange(Character *cc, unsigned short int distancemetric);

    /**
    * returns the distance of this character and another character in fields
    * <b>Lua: [:distanceMetric]</b>
    * @param cc the character to which we want to get the distance
    * @return the number of fields between cc and this character
    */
    unsigned short int distanceMetric(Character *cc);

    /**
    * checks if the character is in range to a position
    * <b>Lua: [:isInRangeToField]</b>
    * @param m_pos the position to which we want to check if we are in range
    * @param distancemetric the number of fields around the character which we want to check
    * @return true if this character < distancemetrif fields away from m_pos
    */
    bool isInRangeToField(position m_pos, unsigned short int distancemetric);

    /**
    * returns the distance to a position in fields
    * <b>Lua: [:distanceMetricToPosition]</b>
    * @param m_pos the field to which we want to get the distance
    * @return the number of fields between m_pos and this character
    */
    unsigned short int distanceMetricToPosition(position m_pos);

    /**
    * alters a message for a specific language
    * <b>Lua: [:alterSpokenMessage]</b>
    * @param message the orginal message
    * @param languageSkill the language skill in which the message is spoken
    * @return a changed message depending how good the char understands the spoken language
    */
    std::string alterSpokenMessage(std::string message, int languageSkill);

    /**
    * gets the language skill of a specific language
    * <b>Lua: [:getLanguageSkill]</b>
    * @param languageSkillNumber number of the language from which we want to get the skill
    * 0 = common...
    * @return the value of the language skill
    */
    int getLanguageSkill(int languageSkillNumber);

    /**
     * character says something
     * <b>Lua: [:talk]</b>
     * @param tt how loudely the character talks
     * @param message the message the character talks
     */
    virtual void talk(talk_type tt, std::string message);

    /**
    * the character talks something in a specific language
    * <b>Lua: [:talkLanguage]</b>
    * @param tt how loudely the character talks
    * @param lang in which language the character talkes
    * @param message the message the character talkes
    */
    virtual void talkLanguage(talk_type tt, unsigned char lang, std::string message);

    /**
    * informs only this character about something
    * <b>Lua: [:inform]</b>
    * @param message which should be sended to this character
    */
    virtual void inform(std::string message);

    /**
    * moves this character in a direction
    * <b>Lua: [:move]</b>
    * @param dir the direction in which the character moves
    * @param active (if the move is active or if he is pushed)
    */
    virtual bool move(direction dir, bool active=true);

    /**
    * sends a message only to this character
    * <b>Lua: [:sendMessage]</b>
    * @param message which is sended
    */
    virtual void sendMessage(std::string message);

    /**
    * gets the direction of a step toward a possible position
    * @param dir byref returns the direction in which the step must be to reach tpos
    * @param tpos position which should be reached.
    * @param checkrange the range which should be checked (large numbers will decrease server speed dramatically)
    * @return true if there ist a possible step otherwise false.
    */
    bool getNextStepDir(position tpos, int checkrange, Character::direction &dir);

    std::list<Character::direction> getStepList(position tpos, int checkrange);

    luabind::object getLuaStepList(position tpos, int checkrange);

    /**
    * starts a menu for this character (overloaded in Player)
    * <b>Lua: [:startPlayerMenu]</b>
    * @param menu a menu struct which holds all the menu entrys
    */
    virtual void startPlayerMenu(UserMenuStruct menu);

    /**
    * warps the character to another position
    * <b>Lua: [:warp]</b>
    * @param newPos the position to which the character should be warped
    * @return true if the warp was sucessfull
    */
    virtual bool Warp(position newPos);

    /**
    * Warps a Character to a different position regardless of the state of that position
    * <b>Lua: [:forceWarp]</b>
    * @param newPos the position to which the character should be warped
    * @return true if warp was succesful otherwise false
    */
    virtual bool forceWarp(position newPos);

    /**
    * increases the hp over a longer time period
    * <b>Lua: [:LTIncreaseHP]</b>
    * @param value how much the hp should be increased in every cycle
    * @param count how much cycles the value should be increased
    * @param time hom much sec before the next cycle
    */
    virtual void LTIncreaseHP(unsigned short int value, unsigned short int count, unsigned short int time);

    /**
    * increases the mana over a longer time period
    * <b>Lua: [:LTIncreaseMana]</b>
    * @param value how much the hp should be increased in every cycle
    * @param count how much cycles the value should be increased
    * @param time hom much sec before the next cycle
    */
    virtual void LTIncreaseMana(unsigned short int value, unsigned short int count, unsigned short int time);

    /**
    * starts new music for the character
    * <b>Lua: [:startMusic]</b>
    * @param title the new id of the music
    */
    virtual void startMusic(short int title);

    /**
    * starts default music for the character
    * <b>Lua: [:defaultMusic]</b>
    */
    virtual void defaultMusic();

    /**
    * sets the questprogress of a specific quest
    * <b>Lua: [:setQuestProgress]</b>
    * @param questid id of the quest
    * @param progress the current questprograss as flags
    */
    virtual void setQuestProgress(uint16_t questid, uint32_t progress) throw();

    /**
    * gets the questprogress of a specific quest
    * <b>Lua: [:getQuestProgress]</b>
    * @param questid id of the quest
    * @return progress the current questprograss as flags
    */
    virtual uint32_t getQuestProgress(uint16_t questid) throw();

    /**
    * send a character description to the player if the  char is a player
    * <b>Lua: [:sendCharDescription]</b>
    * @param id of the character from which the description is sended
    * @param desc the current descpription
    */
    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id,const std::string &desc);

    /**
    *======================end of grouping Lua Functions===================
    */
    //@}

    /**
    * shows the depot of the character
    */
    virtual void Depot();

    /**
         * calculates the weapon mode
         * @return 0: unarmed; 1: melee; 2: distance; 3: wand
         */
    uint8_t getWeaponMode();

    virtual uint32_t idleTime();

    /**
    * asks the client to display a specific book
    * @param bookID ID of the book the client has to display
    */
    virtual void sendBook(uint16_t bookID);

    /**
    * sends an update of the appearance to all characters
    * @param always if false only send the appearance if the char is not known to the client
    */
    void updateAppearanceForAll(bool always);

    /**
    * sends an update of the appearance to all characters
    */
    void forceUpdateAppearanceForAll();

    /**
    * sends an update of the appearance only to one player
    * @param target the player to which the appreance should be sended
    * @param always if false only send the appearance if the char is not known to the client
    */
    void updateAppearanceForPlayer(Player *target, bool always);

    void performAnimation(uint8_t animID);

    /**
    * an equalation funktion for the skillmap
    * @see skillmap
    */
    struct ltstr {
        /**
        * overloading of the () operator to see if two strings have the same structure
        */
        bool operator()(const char *s1, const char *s2) const {
            return strcmp(s1, s2) < 0;
        }
    };

    /**
    * defines the type of the skills for the character
    */
    typedef std::map < const char *, skillvalue, ltstr > SKILLMAP;

    /**
    * the type of movement which this char uses
    */
    enum movement_type {
        walk = 0, /**< the character walks */
        fly = 1, /**< the character swims */
        crawl = 2 /**< the character crawls */
        //swim = 3
    };

    /**
    * returns the movement type of the character
    * @return the movement type
    */
    movement_type GetMovement();

    /**
    * sets the movement type of this character
    * @param tmovement the new movement type for this character
    */
    void SetMovement(movement_type tmovement);

    void AddWeight();
    void SubWeight();

    void ReduceSkills();
    void ReduceAllSkills();

    /**
    * magic abilitys
    */
    struct s_magic {
        magic_type type; /**< which type fo magican is the character*/
        unsigned long int flags[ 4 ]; /**< which rune has the character in the different magic fields (stored as flags) */
    };

    /**
    *the race type of this character
    */

    enum race_type {  human = 0,
                      dwarf = 1,
                      halfling = 2,
                      elf = 3,
                      orc = 4,
                      lizardman = 5,
                      gnome = 6,
                      healer = 7,
                      troll = 9,
                      mumie = 10,
                      skeleton = 11,
                      beholder = 12,
                      blackbeholder = 13,
                      transparentbeholder = 14,
                      brownmummy = 15,
                      bluemummy = 17,
                      sheep = 18,
                      spider = 19,
                      demonskeleton = 20,
                      redspider = 21,
                      greenspider = 22,
                      bluespider = 23,
                      pig = 24,
                      boar = 25,
                      transparentspider = 26,
                      wasp = 27,
                      redwasp = 28,
                      stonegolem = 30,
                      brownstonegolem = 31,
                      redstonegolem = 32,
                      silverstonegolem = 33,
                      transparentstonegolem = 34,
                      cow = 37,
                      bull = 38,
                      wolf = 39,
                      transparentwolf = 40,
                      blackwolf = 41,
                      greywolf = 42,
                      redwolf = 43,
                      redraptor = 48,
                      silverbear = 49,
                      blackbear = 50,
                      bear = 51,
                      raptor = 52,
                      zombie = 53,
                      hellhound = 54,
                      imp = 55,
                      iron_golem = 56,
                      ratman = 57,
                      dog = 58,
                      beetle = 59,
                      fox = 60,
                      slime = 61,
                      chicken = 62,
                      bonedragon = 63,
                      blackbonedragon = 64,
                      redbonedragon = 65,
                      transparentbonedragon = 66,
                      greenbonedragon = 67,
                      bluebonedragon = 68,
                      goldbonedragon = 69,
                      redmummy = 70,
                      greymummy = 71,
                      blackmummy = 72,
                      goldmummy = 73,
                      transparentskeleton = 74,
                      blueskeleton = 75,
                      greenskeleton = 76,
                      goldgolem = 77,
                      goldskeleton = 78,
                      bluetroll = 79,
                      blacktroll = 80,
                      redtroll = 81,
                      blackzombie = 82,
                      transparentzombie = 83,
                      redzombie = 84,
                      blackhellhound = 85,
                      transparenthellhound = 86,
                      greenhellhound = 87,
                      redhellhound = 88,
                      redimp = 89,
                      blackimp = 90,
                      blueirongolem = 91,
                      redratman = 92,
                      greenratman = 93,
                      blueratman = 94,
                      reddog = 95,
                      greydog = 96,
                      blackdog = 97,
                      greenbeetle = 98,
                      copperbeetle = 99,
                      redbeetle = 100,
                      goldbeetle = 101,
                      greyfox = 102,
                      redslime = 103,
                      blackslime = 104,
                      transparentslime = 105,
                      brownchicken = 106,
                      redchicken = 107,
                      blackchicken = 108,
                   };

    race_type race;

    character_type character; /**< the type of the character*/

    std::string prefix; /**< prefix of the character */
    std::string suffix; /**< suffix of the character */

    s_magic magic; /**< magic posibilities of the character */

    /**
    * direction in which the character looks
    */
    face_to faceto;


    /**
    * stores the basic attributes of the character
    */
    struct base_attributes {
        Character::sex_type truesex; /**< true sex of the character */
        Character::sex_type sex; /**< current (temporary) sex of the character */
        uint16_t time_sex; /**< time value in seconds before sex is changed back to the true value */

        unsigned short int trueage; /**< true age of the character */
        unsigned short int age; /**<  (temporary) age of the character */
        uint16_t time_age; /**< time value in seconds before age is changed back to the true value */

        unsigned short int trueweight; /**< true weight of the character */
        unsigned short int weight; /**< (temporary) weight of the character */
        uint16_t time_weight; /**< time value in seconds before weight is changed back to the true value */

        uint16_t truebody_height; /**< true height of the character */
        uint16_t body_height; /**< (temporary) height of the character */
        uint16_t time_body_height; /**< time value in seconds before sex is changed back to the true value */

        unsigned short int truehitpoints; /**< true hitpoints of the character */
        unsigned short int hitpoints; /**< (temporary) hitpoint of the character */
        uint16_t time_hitpoints; /**< time value in seconds before hitpoints is changed back to the true value */

        unsigned short int truemana; /**< true mana of the character */
        unsigned short int mana; /**< (temporary) mana of the character */
        uint16_t time_mana; /**< time value in seconds before mana is changed back to the true value */

        unsigned char attitude; /**<  (temporary) attitude of the character */
        unsigned char trueattitude; /**< true attitude of the character*/
        uint16_t time_attitude; /**< time value in seconds before attitude is changed back to the true value */

        unsigned char luck; /**<  (temporary) luck of the character */
        unsigned char trueluck; /**< true luck of the character */
        uint16_t time_luck; /**< time value in seconds before luck is changed back to the true value */

        unsigned char strength; /**<  (temporary) strength of the character */
        unsigned char truestrength; /**< true strength of the character */
        uint16_t time_strength; /**< time value in seconds before strength is changed back to the true value */

        unsigned char dexterity; /**< (temporary) dexterity of the character */
        unsigned char truedexterity; /**< true dexterity of the character */
        uint16_t time_dexterity; /**< time value in seconds before dexterity is changed back to the true value */

        unsigned char constitution; /**< (temporary) constitution of the character */
        unsigned char trueconstitution; /**< true constitution of the character */
        uint16_t time_constitution; /**< time value in seconds before dexterity is changed back to the true value */

        unsigned char agility; /**< (temporary) agility of the character */
        unsigned char trueagility; /**< true aglitiy of the character */
        uint16_t time_agility; /**< time value in seconds before agility is changed back to the true value */

        unsigned char intelligence; /**< (temporary)intelligence of the character */
        unsigned char trueintelligence; /**< true intelligence of the character */
        uint16_t time_intelligence; /**< time value in seconds before intelligence is changed back to the true value */

        unsigned char perception; /**< (temporary) perception of the character */
        unsigned char trueperception; /**< true perception of the character */
        uint16_t time_perception; /**< time value in seconds before sex is changed back to the true value */

        unsigned char willpower; /**< (temporary) willpower of the character */
        unsigned char truewillpower; /**< true willpower of the character */
        uint16_t time_willpower; /**< time value in seconds before willpower is changed back to the true value */

        unsigned char essence; /**< (temporary) essence of the character */
        unsigned char trueessence; /**< true essencee of the character */
        uint16_t time_essence; /**< time value in seconds before essence is changed back to the true value */

        unsigned short int foodlevel; /**< (temporary) foodlevel of the character */
        unsigned short int truefoodlevel; /**< true foodlevel of the character */
        uint16_t time_foodlevel; /**< time value in seconds before foodlevel is changed back to true values*/

    };

    base_attributes battrib; /**< the basic attributes of the character*/

    bool informCharacter; /**< if true the character gets more debug informations*/

    inline virtual void setClippingActive(bool tclippingActive) {}
    inline virtual bool getClippingActive() {
        return true;
    }

    /**
    * sets the inform state of a character
    * @param newInformChar the new inform state of this character
    */
    inline virtual void setInformChar(bool newInformChar) {
        informCharacter=newInformChar;
    }


    /**
    * stores the skills of a character
    * @see SKILLMAP
    */
    SKILLMAP skills;

    //! Zust�de des Character
    //std::map < unsigned short int, unsigned short int > state;

    /**
    * array for the items of the character
    * 0 = backpack, 1 - MAX_BODY_ITEMS - 1 = items at the body
    * (MAX_BODY_ITEMS + MAX_BELT_SLOTS -1) = items in the belt
    * @see Item
    */
    Item characterItems[ MAX_BODY_ITEMS + MAX_BELT_SLOTS ];

    /**
    * pointer to the items in the backpack of the character
    */
    Container *backPackContents;

    /**
    * map to the different depots
    * first param is the depot id
    * second param is the pointer to the container which stores the items in this depot
    */
    std::map<uint32_t, Container *> depotContents;

    /**
    *standard constructor which initializes all lokal variables
    */
    Character();

    /**
    * virtual destructor
    */
    virtual ~Character();

    bool isinvisible; /**< shows if a character is invisible*/

    int appearance_alive(); /**< returns the id of graphic if the character is alive*/


    int appearance_dead(); /**< returns the id of the graphic if this character is dead*/

    virtual void ageInventory();

    /**
    * current lifestate of the character (alive or death)
    */
    unsigned short int lifestate;

    unsigned short int appearance; /**< appearance of this character */

    /**
    * checks if a character is alive
    * @return true if the character is alive otherwise false
    */
    inline bool IsAlive() {
        return ((lifestate & 1) != 0);
    }

#ifdef DO_UNCONSCIOUS
    inline bool IsConscious() {
        return (battrib.hitpoints > UNCONSCIOUS || ! IsAlive());
    }

    inline bool IsDying() {
        return (battrib.hitpoints <= DYING && IsAlive());
    }
#endif

    /**
    * changes the lifestate of this character
    * @param t true if the character should be alive again otherwise false
    */
    virtual void SetAlive(bool t);

    /**
    * stores the type of the enemy of this character
    * @see character_type
    */
    character_type enemytype;

    /**
    * id of the attacked player
    */
    TYPE_OF_CHARACTER_ID enemyid;

    /**
    * the character attacks another character
    * @param target pointer to the character which is attacked
    * @param sound reference to a sound which should be played
    * @param updateInv returns true if the inventory should be updated (currently evertime true)
    */
    virtual bool attack(Character *target, int &sound, bool &updateInv);

    /**
    *checks all attribs if the time base is smaller 0 and sets them back to the default value
    */
    virtual void tempAttribCheck();

    /**
    * changes the values of a skill comepletly
    * @param typ the tpye of the skill which should be changed
    * @param sname the name of the skill which should be changed
    * @param major the major skill value
    * @param minor the minor skill value
    * @param firsttry the number of unsuccesfull tries to increase this skill
    * @return the major value after the changes
    */
    virtual unsigned short int setSkill(unsigned char typ, std::string sname, short int major, short int minor, uint16_t firsttry);


    /**
    *deletes all skill of a character
    */
    virtual void deleteAllSkills();

    /**
    * returns the maximal weight the character can lift
    * @return the number of (pounds??) how much the character can lift
    */
    unsigned short int maxLiftWeigt();

    /**
    * returns the maximal weight the character can carry
    * @return the number of (pounds??) the character can carry in hin inventory
    */
    unsigned short int maxLoadWeight();

    /**
    * the weight amount the character is carrieng
    * @return the number of (pounds??) the character carries
    */
    int LoadWeight();

    /**
    * checks if the character is under a map
    */
    bool under[ RANGEUP ];

    /**
    * checks if a character is under a Map
    */
    void *roofmap[ RANGEUP ];

    /**
    * returns the absolut value of value
    * @param value from which we want the absolute value
    * @return the absolute value of value
    */
    int Abso(int value);

    /**
    * returns the weight of count items with a specific id
    * @param id the id from which we want to get the weight
    * @param count how much items from which we want to get the weight
    * @return the weight of count items with id
    */
    int weightItem(TYPE_OF_ITEM_ID id, int count);

    /**
    * returns the weight of a container
    * @param id of the container from which we want to get the weight
    * @param count if the weight should be positive or negative
    * @param tcont the container from which we want to get the weight
    * @return the +/- weight of the container
    */
    int weightContainer(unsigned short int id, int count, Container *tcont);

    /**
    * checks if in tcont is enough place for count items with id
    * @param id the item which should be placed in the container
    * @param count how much items should be added to the container
    * @param tcont pointer to the container (items inside the container)
    * @return true if count items of id can be added otherwise false
    */
    bool weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont);


    /**
    * turns a character to a specific direction
    * @param dir in which the character should be turned
    */
    virtual void turn(direction dir);

    /**
    * turns the character to a specific position
    * @param posi in which direction the character should be turned
    */
    virtual void turn(position posi);

    /**
    * the character receives a text which was spoken from another character
    * @param tt how loudely the message is spoken
    * @param message which was spoken
    * @param cc the character which has spoken the message
    */
    virtual void receiveText(talk_type tt, std::string message, Character *cc);

    /**
    * introduces the character to another one
    * @param cc the character to which we want to introduce
    */
    virtual void introducePerson(Character *cc);

    void callAttackScript(Character *Attacker, Character *Defender);

    virtual void requestInputDialog(InputDialog *inputDialog);
    virtual void requestMessageDialog(MessageDialog *messageDialog);

    //===========================================In attack.cc==================================================

    void destroyWeapons(int strHit);

    void destroyArmorpart(int strHit);

    //! addiert zu slashA und thrustA den Schutzwert des Item an der Inventory-Position pos
    // \param slashA nach dem Funktionsaufruf um den Schutzwert des Item an der Inventory-Position pos erh�t
    // \param thrustA nach dem Funktionsaufruf um den Schutzwert des Item an der Inventory-Position pos erh�t
    void addArmorFromPos(unsigned char pos, int &slashA, int &thrustA, int &punctureA);

    //! addiert zu slashA und thrustA die natrliche Rstungsklasse des Characters
    // \param slashA nach dem Funktionsaufruf um den natrlichen Schutzwert erh�t
    // \param thrustA nach dem Funktionsaufruf um den natrlichen Schutzwert erh�t
    void addNaturalArmor(int &slashA, int &thrustA, int &punctureA);



protected:

    bool death_consequences;

    //! ein Datentyp in dem einige Werte fr die verschiedenen Rassen gespeichert werden k�nen
    struct RaceStruct {
        std::string racename;
        unsigned short int points;
        unsigned short int minage;
        unsigned short int maxage;
        unsigned short int minweight;
        unsigned short int maxweight;
        unsigned char minbodyheight;
        unsigned char maxbodyheight;
        unsigned char minagility;
        unsigned char maxagility;
        unsigned char minconstitution;
        unsigned char maxconstitution;
        unsigned char mindexterity;
        unsigned char maxdexterity;
        unsigned char minessence;
        unsigned char maxessence;
        unsigned char minintelligence;
        unsigned char maxintelligence;
        unsigned char minperception;
        unsigned char maxperception;
        unsigned char minstrength;
        unsigned char maxstrength;
        unsigned char minwillpower;
        unsigned char maxwillpower;
    };

protected:

    bool _is_on_route; /**< if the character on a route*/

    short int poisonvalue; /**< current poison value of the character*/
    int mental_capacity; /**< current mental capacity of the character */
    World *_world; /**< pointer to the game world */

    virtual bool moveToPossible(const Field *field); /**< checks if moving to a field is possible*/

    virtual void updatePos(position newpos); /**< sets the character to a new position*/

    /**
    * calculates movement costs for this character
    * @param sourcefield from which we want to get the movement cost
    * @return the movement costs
    */
    virtual uint16_t getMovementCost(Field *sourcefield);

    // shared code for public eraseItem methods
    virtual int _eraseItem(TYPE_OF_ITEM_ID itemid, int count, uint32_t data, bool useData);

private:

    movement_type _movement; /**< the movement type of the character*/

};

#endif // CHARACTER_HHP
