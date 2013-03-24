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
#include <string>
#include <fstream>
#include <map>
#include <unordered_map>
#include "globals.hpp"
#include "constants.hpp"
#include "TableStructs.hpp"
#include "Item.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "Attribute.hpp"
#include "Language.hpp"
#include "WaypointList.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "types.hpp"

class World;
class Container;
class Field;
class InputDialog;
class MessageDialog;
class MerchantDialog;
class SelectionDialog;
class CraftingDialog;

struct NoSpace : public std::runtime_error {
    NoSpace(const std::string &s = "no space") : std::runtime_error(s) {}
};

enum magic_type {
    MAGE=0,
    PRIEST=1,
    BARD=2,
    DRUID=3
};

class Character {
    Character(const Character &) = delete;
    Character &operator=(const Character &) = delete;

public:
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

    Character(const appearance &appearance = {});
    virtual ~Character();

    enum character_type {
        player = 0,
        monster = 1,
        npc = 2
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

    typedef std::unordered_map<std::string, attributeIndex> attribute_map_t;
    typedef std::unordered_map<attributeIndex, std::string, std::hash<int>> attribute_string_map_t;
    static attribute_map_t attributeMap;
    static attribute_string_map_t attributeStringMap;

    enum talk_type {
        tt_say=0,
        tt_whisper=1,
        tt_yell=2
    };

    enum sex_type {
        male = 0,
        female = 1
    };

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

    enum informType {
        informServer = 0,
        informBroadcast = 1,
        informGM = 2,
        informScriptLowPriority = 100,
        informScriptMediumPriority = 101,
        informScriptHighPriority = 102
    };

    struct skillvalue {
        unsigned short int major = 0;
        unsigned short int minor = 0;
    };

    inline TYPE_OF_CHARACTER_ID getId() const {
        return id;
    }

    inline std::string getName() const {
        return name;
    }

    virtual std::string to_string() const = 0;

    short int actionPoints;
    short int fightPoints;
    short int activeLanguage;
    position pos;
    bool attackmode;

    std::string lastSpokenText;
    LongTimeCharacterEffects effects;
    WaypointList waypoints;

    position getFrontalPosition() const;

    virtual bool actionRunning() const {
        return false;
    }

    inline unsigned short get_magic_type() const {
        return magic.type;
    }

    inline virtual void set_magic_type(magic_type newMagType) {
        magic.type=newMagType;
    }

    void setOnRoute(bool onr) {
        _is_on_route = onr;
    }

    bool getOnRoute() const {
        return _is_on_route;
    }

    virtual Language getPlayerLanguage() const {
        return Language::english;
    }


    inline unsigned long int get_magic_flags(unsigned char type) const {
        return magic.flags[type];
    }

    inline unsigned short get_character() const {
        return character;
    }

    inline unsigned short get_race() const {
        return race;
    }

    inline unsigned short get_face_to() const {
        return faceto;
    }

    virtual bool isAdmin() const {
        return false;
    }

    virtual TYPE_OF_CHARACTER_ID getType() const {
        return 0;
    }

    virtual void changeQualityAt(unsigned char pos, short int amount);
    virtual void changeQualityItem(TYPE_OF_ITEM_ID id, short int amount);
    virtual void increasePoisonValue(short int value);

    inline short int getPoisonValue() const {
        return poisonvalue;
    }

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
    inline virtual void startAction(unsigned short int wait,
                                    unsigned short int ani = 0,
                                    unsigned short int redoani = 0,
                                    unsigned short int sound = 0,
                                    unsigned short int redosound = 0) {
    }

    inline virtual void abortAction() {
    }

    inline virtual void successAction() {
    }

    inline virtual void actionDisturbed(Character *disturber) {
    }

    inline virtual void changeSource(Character *cc) {
    }

    inline virtual void changeSource(const ScriptItem &sI) {
    }

    inline virtual void changeSource(const position &pos) {
    }

    inline virtual void changeSource() {
    }

    inline virtual void changeTarget(Character *cc) {
    }

    inline virtual void changeTarget(const ScriptItem &sI) {
    }

    inline virtual void changeTarget(const position &pos) {
    }

    inline virtual void changeTarget() {
    }

    inline int getMentalCapacity() const {
        return mental_capacity;
    }

    inline void setMentalCapacity(int value) {
        mental_capacity = value;
    }

    virtual void increaseMentalCapacity(int value);

    int countItem(TYPE_OF_ITEM_ID itemid) const ;
    // where determines where the items will be counted ("all", "belt", "body", "backpack")
    int countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid, script_data_exchangemap const *data = nullptr) const;
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr);
    virtual int createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, script_data_exchangemap const *data);
    virtual int increaseAtPos(unsigned char pos, int count);
    virtual int createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count);
    virtual bool swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);
    ScriptItem GetItemAt(unsigned char itempos);
    Container *GetBackPack() const;
    Container *GetDepot(uint32_t depotid) const;
    std::vector<ScriptItem> getItemList(TYPE_OF_ITEM_ID id) const;

    virtual std::string getSkillName(TYPE_OF_SKILL_ID s) const;
    unsigned short int getSkill(TYPE_OF_SKILL_ID s) const;
    const skillvalue *getSkillValue(TYPE_OF_SKILL_ID s) const;
    unsigned short int getMinorSkill(TYPE_OF_SKILL_ID s) const;

    void setSkinColor(uint8_t red, uint8_t green, uint8_t blue);
    void getSkinColor(uint8_t &red, uint8_t &green, uint8_t &blue) const;
    void setHairColor(uint8_t red, uint8_t green, uint8_t blue);
    void getHairColor(uint8_t &red, uint8_t &green, uint8_t &blue) const;
    void setHair(uint8_t hairID);
    uint8_t getHair() const;
    void setBeard(uint8_t beardID);
    uint8_t getBeard() const;

    appearance getAppearance() const {
        return _appearance;
    }

    void setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);
    Attribute::attribute_t getAttribute(Character::attributeIndex attribute) const;
    Attribute::attribute_t increaseAttribute(Character::attributeIndex attribute, int amount);
    virtual void handleAttributeChange(Character::attributeIndex attribute);
    Attribute::attribute_t increaseAttrib(const std::string &name, int amount);
    void setAttrib(const std::string &name, Attribute::attribute_t value);

    virtual unsigned short int increaseSkill(TYPE_OF_SKILL_ID skill, short int amount);
    virtual unsigned short int increaseMinorSkill(TYPE_OF_SKILL_ID skill, short int amount);
    virtual unsigned short int setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor);
    virtual void deleteAllSkills();
    virtual void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent);
    virtual void teachMagic(unsigned char type, unsigned char flag);

    bool isInRange(Character *cc, unsigned short int distancemetric) const;
    virtual unsigned short int getScreenRange() const;
    unsigned short int distanceMetric(Character *cc) const;
    bool isInRangeToField(const position &m_pos, unsigned short int distancemetric) const;
    unsigned short int distanceMetricToPosition(const position &m_pos) const;

    std::string alterSpokenMessage(const std::string &message, int languageSkill) const;
    int getLanguageSkill(int languageSkillNumber) const;

    virtual void talk(talk_type tt, const std::string &message);
    virtual void talk(talk_type tt, const std::string &german, const std::string &english);

    virtual void inform(const std::string &message, informType type = informServer) const;
    virtual void inform(const std::string &german, const std::string &english, informType type = informServer) const;

    virtual bool move(direction dir, bool active=true);

    bool getNextStepDir(const position &goal, direction &dir) const;
    bool getStepList(const position &goal, std::list<direction> &steps) const;

    virtual bool Warp(const position &newPos);
    virtual bool forceWarp(const position &newPos);

    virtual void startMusic(short int title);
    virtual void defaultMusic();

    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress);
    virtual TYPE_OF_QUESTSTATUS getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const;

    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc);

    virtual bool pageGM(const std::string &ticket);

    virtual uint32_t idleTime() const;

    virtual void sendBook(uint16_t bookID);

    void updateAppearanceForAll(bool always);
    void forceUpdateAppearanceForAll();
    void updateAppearanceForPlayer(Player *target, bool always);

    void performAnimation(uint8_t animID);

    typedef std::map<TYPE_OF_SKILL_ID, skillvalue> SKILLMAP;

    enum movement_type {
        walk = 0,
        fly = 1,
        crawl = 2
    };

    movement_type GetMovement() const;
    void SetMovement(movement_type tmovement);

    void AddWeight();
    void SubWeight();

    struct s_magic {
        magic_type type;
        unsigned long int flags[ 4 ];
    };

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

    character_type character;

    s_magic magic;

    face_to faceto;

    inline virtual void setClippingActive(bool tclippingActive) {}
    inline virtual bool getClippingActive() const {
        return true;
    }

    SKILLMAP skills;

    /**
    * array for the items of the character
    * 0 = backpack, 1 - MAX_BODY_ITEMS - 1 = items at the body
    * (MAX_BODY_ITEMS + MAX_BELT_SLOTS -1) = items in the belt
    */
    Item characterItems[ MAX_BODY_ITEMS + MAX_BELT_SLOTS ];

    Container *backPackContents;

    /**
    * map to the different depots
    * first param is the depot id
    * second param is the pointer to the container which stores the items in this depot
    */
    std::map<uint32_t, Container *> depotContents;

    bool isinvisible;

    virtual void ageInventory();

    inline bool IsAlive() const {
        return alive;
    }

    virtual void SetAlive(bool t);

    character_type enemytype;
    TYPE_OF_CHARACTER_ID enemyid;
    virtual bool attack(Character *target);

    unsigned short int maxLoadWeight() const;
    int LoadWeight() const;

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

    virtual void turn(direction dir);
    virtual void turn(const position &posi);

    virtual void receiveText(talk_type tt, const std::string &message, Character *cc);

    virtual void introducePlayer(Player *player);

    void callAttackScript(Character *Attacker, Character *Defender);

    virtual void requestInputDialog(InputDialog *inputDialog);
    virtual void requestMessageDialog(MessageDialog *messageDialog);
    virtual void requestMerchantDialog(MerchantDialog *merchantDialog);
    virtual void requestSelectionDialog(SelectionDialog *selectionDialog);
    virtual void requestCraftingDialog(CraftingDialog *craftingDIalog);
    virtual void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt);
    virtual void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt);

protected:
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

    void setId(TYPE_OF_CHARACTER_ID id);
    void setName(const std::string &name);

    bool _is_on_route;
    short int poisonvalue;
    int mental_capacity;
    World *_world;

    virtual bool moveToPossible(const Field *field) const;
    virtual void updatePos(const position &newpos); /**< sets the character to a new position*/

    /**
    * calculates movement costs for this character
    * @param sourcefield from which we want to get the movement cost
    * @return the movement costs
    */
    virtual uint16_t getMovementCost(const Field *sourcefield) const;

    appearance _appearance;

private:
    TYPE_OF_CHARACTER_ID id;
    std::string name;
    movement_type _movement;
    std::vector<Attribute> attributes;
    bool alive;
};

std::ostream &operator<<(std::ostream &os, const Character &character);


#endif
