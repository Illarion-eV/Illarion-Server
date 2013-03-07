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
#include <boost/unordered_map.hpp>
#include "Attribute.hpp"
#include "Language.hpp"
#include "WaypointList.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "types.hpp"

class Field;
class InputDialog;
class MessageDialog;
class MerchantDialog;
class SelectionDialog;
class CraftingDialog;

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

    enum attributeIndex {
        strength,
        dexterity,
        constitution,
        agility,
        intelligence,
        perception,
        willpower,
        essence,
        hitpoints,
        mana,
        foodlevel,
        sex,
        age,
        weight,
        height,
        attitude,
        luck,
        ATTRIBUTECOUNT
    };

    static boost::unordered_map<std::string, attributeIndex> attributeMap;
    static boost::unordered_map<attributeIndex, std::string> attributeStringMap;

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
    enum informType {
        informServer = 0,
        informBroadcast = 1,
        informGM = 2,
        informScriptLowPriority = 100,
        informScriptMediumPriority = 101,
        informScriptHighPriority = 102
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
        * constructor initializes all values with 0
        *<b>Lua</b>
        */
        skillvalue() {
            major = 0;
            minor = 0;
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

    std::string nameAndId() const {
        return name + "(" + boost::lexical_cast<std::string>(id) + ")";
    }

    virtual std::string to_string() const = 0;

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

    struct appearance {

        struct color {
            uint8_t red, green, blue;
        };

        uint8_t hairtype = 0;
        uint8_t beardtype = 0;
        color hair = { 255, 255, 255 };
        color skin = { 255, 255, 255 };

        appearance() {};
    };

    /**
    * stores the last message the character has talked
    * used in scripts
    *<b>Lua: (ro) [lastSpokenText]</b>
    */
    std::string lastSpokenText;

    /**
    *the effects which are currently working on the character
    */
    LongTimeCharacterEffects effects;

    WaypointList waypoints;

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
    virtual Language getPlayerLanguage() const {
        return Language::english;   //nothing to do at all
    } //mother_tongue}

    uint32_t getPlayerLanguageLua() const {
        return static_cast<uint32_t>(getPlayerLanguage());
    }

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
    virtual bool isAdmin() const {
        return false;
    }

    /**
    * <b>Lua: [:getType]</b>
    * returns the monstertype
    * @return the monster type, < 0 for no monster
    */
    virtual TYPE_OF_CHARACTER_ID getType() {
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
    int countItemAt(std::string where, TYPE_OF_ITEM_ID itemid, const luabind::object &data);

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
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count, const luabind::object &data);

    /**
    * creates count items with a specific id in the inventory of the character
    * <b>Lua: [:createItem]</b>
    * @param itemid the id of the item which should be created
    * @param count how much of the items should be created
    * @param quali the quality of the new created items
    * @param data the data value of the new created items
    * @return the number of items which where created sucessfully
    */
    virtual int createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object &data);

    /**
    * changes the amount of items at a specific position
    * <b>Lua: [:increaseAtPos]</b>
    * @param pos the position where the number of items should be increased or decreased
    * @param count the number of increasage or decreasage
    * @return the number of items which where deleted/added
    */
    virtual int increaseAtPos(unsigned char pos, int count);

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

    virtual std::string getSkillName(TYPE_OF_SKILL_ID s);
    unsigned short int getSkill(TYPE_OF_SKILL_ID s);
    skillvalue *getSkillValue(TYPE_OF_SKILL_ID s);
    unsigned short int getMinorSkill(TYPE_OF_SKILL_ID s);

    void setSkinColor(uint8_t red, uint8_t green, uint8_t blue);


    void getSkinColor(uint8_t &red, uint8_t &green, uint8_t &blue);


    void setHairColor(uint8_t red, uint8_t green, uint8_t blue);


    void getHairColor(uint8_t &red, uint8_t &green, uint8_t &blue);


    void setHair(uint8_t hairID);


    uint8_t getHair();

    void setBeard(uint8_t beardID);

    uint8_t getBeard();

    appearance getAppearance() const {
        return _appearance;
    }

    void setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);
    Attribute::attribute_t getAttribute(Character::attributeIndex attribute) const;
    Attribute::attribute_t increaseAttribute(Character::attributeIndex attribute, int amount);
    virtual void handleAttributeChange(Character::attributeIndex attribute);

    /**
    * changes the value of an attribute
    * <b>Lua: [:increaseAttrib]</b>
    * @param name of the attribute which should be changed
    * @param amount how much this attribute should be changed
    * @return the attribute value after the change
    */
    Attribute::attribute_t increaseAttrib(std::string name, int amount);

    /**
    * changes a attribute aboslute
    * <b>Lua: [:setAttrib]</b>
    * @param name of the attribute which should be changed
    * @param value the new value of this attribute
    */
    void setAttrib(std::string name, Attribute::attribute_t value);

    virtual unsigned short int increaseSkill(TYPE_OF_SKILL_ID skill, short int amount);
    virtual unsigned short int increaseMinorSkill(TYPE_OF_SKILL_ID skill, short int amount);

    virtual void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent);

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

    virtual unsigned short int getScreenRange() const;

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
    std::string alterSpokenMessage(const std::string &message, int languageSkill) const;

    int getLanguageSkill(int languageSkillNumber);

    /**
     * character says something
     * <b>Lua: [:talk]</b>
     * @param tt how loudely the character talks
     * @param message the message the character talks
     */
    virtual void talk(talk_type tt, const std::string &message);
    virtual void talk(talk_type tt, const std::string &german, const std::string &english);

    /**
    * informs only this character about something
    * <b>Lua: [:inform]</b>
    * @param message which should be sended to this character
    */
    virtual void inform(const std::string &message, informType type = informServer) const;
    virtual void informLua(const std::string &message) const;
    virtual void informLua(const std::string &german, const std::string &english) const;
    virtual void informLua(const std::string &message, informType type) const;
    virtual void informLua(const std::string &german, const std::string &english, informType type) const;

    /**
    * moves this character in a direction
    * <b>Lua: [:move]</b>
    * @param dir the direction in which the character moves
    * @param active (if the move is active or if he is pushed)
    */
    virtual bool move(direction dir, bool active=true);

    bool getNextStepDir(position goal, direction &dir);
    bool getStepList(position goal, std::list<direction> &steps);
    luabind::object getLuaStepList(position goal);

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
    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress);

    /**
    * gets the questprogress of a specific quest
    * <b>Lua: [:getQuestProgress]</b>
    * @param questid id of the quest
    * @return progress the current questprograss as flags
    */
    virtual TYPE_OF_QUESTSTATUS getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) throw();

    /**
    * send a character description to the player if the  char is a player
    * <b>Lua: [:sendCharDescription]</b>
    * @param id of the character from which the description is sended
    * @param desc the current descpription
    */
    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id,const std::string &desc);

    virtual bool pageGM(std::string ticket);

    /**
    *======================end of grouping Lua Functions===================
    */
    //@}

    /**
    * shows the depot of the character
    */
    virtual void Depot();

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
    typedef std::map<TYPE_OF_SKILL_ID, skillvalue> SKILLMAP;

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

private:
    std::vector<Attribute> attributes;

public:
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
    Character(const appearance &appearance = {});

    /**
    * virtual destructor
    */
    virtual ~Character();

    bool isinvisible; /**< shows if a character is invisible*/

    virtual void ageInventory();

private:
    /**
    * current lifestate of the character (alive or death)
    */
    bool alive;

public:
    /**
    * checks if a character is alive
    * @return true if the character is alive otherwise false
    */
    inline bool IsAlive() const {
        return alive;
    }

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
    */
    virtual bool attack(Character *target);

    /**
    * changes the values of a skill comepletly
    * @param typ the tpye of the skill which should be changed
    * @param sname the name of the skill which should be changed
    * @param major the major skill value
    * @param minor the minor skill value
    * @return the major value after the changes
    */
    virtual unsigned short int setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor);


    /**
    *deletes all skill of a character
    */
    virtual void deleteAllSkills();

    /**
    * returns the maximal weight the character can carry
    * @return the number of (pounds??) the character can carry in hin inventory
    */
    unsigned short int maxLoadWeight() const;

    /**
    * the weight amount the character is carrieng
    * @return the number of (pounds??) the character carries
    */
    int LoadWeight() const;

    /**
    * returns the absolut value of value
    * @param value from which we want the absolute value
    * @return the absolute value of value
    */
    int Abso(int value);

    /**
    * returns the weight of a container
    * @param id of the container from which we want to get the weight
    * @param count if the weight should be positive or negative
    * @param tcont the container from which we want to get the weight
    * @return the +/- weight of the container
    */
    int weightContainer(unsigned short int id, int count, Container *tcont) const;

    /**
    * checks if in tcont is enough place for count items with id
    * @param id the item which should be placed in the container
    * @param count how much items should be added to the container
    * @param tcont pointer to the container (items inside the container)
    * @return true if count items of id can be added otherwise false
    */
    bool weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const;


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

    virtual void introducePlayer(Player *player);

    void callAttackScript(Character *Attacker, Character *Defender);

    virtual void requestInputDialog(InputDialog *inputDialog);
    virtual void requestMessageDialog(MessageDialog *messageDialog);
    virtual void requestMerchantDialog(MerchantDialog *merchantDialog);
    virtual void requestSelectionDialog(SelectionDialog *selectionDialog);
    virtual void requestCraftingDialog(CraftingDialog *craftingDIalog);
    virtual void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt);
    virtual void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt);

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
    virtual int _eraseItem(TYPE_OF_ITEM_ID itemid, int count, const luabind::object &data, bool useData);

    appearance _appearance;

private:

    movement_type _movement; /**< the movement type of the character*/

};

std::ostream &operator<<(std::ostream &os, const Character &character);

#endif // CHARACTER_HHP
