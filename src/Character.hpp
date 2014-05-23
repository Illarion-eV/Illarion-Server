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


#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <stdexcept>
#include <string>
#include <fstream>
#include <map>
#include <unordered_map>
#include "constants.hpp"
#include "tuningConstants.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "WaypointList.hpp"
#include "Language.hpp"
#include "Attribute.hpp"
#include "Item.hpp"

class World;
class Container;
class Field;
class InputDialog;
class MessageDialog;
class MerchantDialog;
class SelectionDialog;
class CraftingDialog;
class Player;

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
        north = dir_north,
        northeast = dir_northeast,
        east = dir_east,
        southeast = dir_southeast,
        south = dir_south,
        southwest = dir_southwest,
        west = dir_west,
        northwest = dir_northwest
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

    enum movement_type {
        walk = 0,
        fly = 1,
        crawl = 2
    };

    struct s_magic {
        magic_type type;
        unsigned long int flags[ 4 ];
    };

    virtual TYPE_OF_CHARACTER_ID getId() const;
    const std::string &getName() const;
    virtual std::string to_string() const = 0;

    short int getActionPoints() const;
    virtual short int getMinActionPoints() const;
    virtual short int getMaxActionPoints() const;
    void setActionPoints(short int ap);
    void increaseActionPoints(short int ap);
    bool canAct() const;

    short int getFightPoints() const;
    virtual short int getMinFightPoints() const;
    virtual short int getMaxFightPoints() const;
    void setFightPoints(short int fp);
    void increaseFightPoints(short int fp);
    bool canFight() const;

    short int getActiveLanguage() const;
    void setActiveLanguage(short int l);

    virtual const position &getPosition() const;

    bool getAttackMode() const;
    void setAttackMode(bool attack);

    const std::string &getLastSpokenText() const;

    bool isInvisible() const;
    void setInvisible(bool invisible);

    LongTimeCharacterEffects effects;
    WaypointList waypoints;

    position getFrontalPosition() const;

    virtual bool actionRunning() const {
        return false;
    }

    inline unsigned short getMagicType() const {
        return magic.type;
    }

    inline virtual void setMagicType(magic_type newMagType) {
        magic.type = newMagType;
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


    inline unsigned long int getMagicFlags(unsigned char type) const {
        if (type < 4) {
            return magic.flags[type];
        } else {
            return 0;
        }
    }

    virtual unsigned short getType() const = 0;

    virtual void changeRace(TYPE_OF_RACE_ID race) {
        this->race = race;
        updateAppearanceForAll(true);
    }

    virtual TYPE_OF_RACE_ID getRace() const {
        return race;
    }

    virtual face_to getFaceTo() const {
        return faceto;
    }

    virtual bool isAdmin() const {
        return false;
    }

    virtual TYPE_OF_CHARACTER_ID getMonsterType() const {
        return 0;
    }

    virtual void changeQualityAt(unsigned char pos, short int amount);
    virtual void increasePoisonValue(short int value);

    virtual short int getPoisonValue() const {
        return poisonvalue;
    }

    virtual void setPoisonValue(short int value) {
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

    virtual int getMentalCapacity() const {
        return mental_capacity;
    }

    virtual void setMentalCapacity(int value) {
        mental_capacity = value;
    }

    virtual void increaseMentalCapacity(int value);

    virtual int countItem(TYPE_OF_ITEM_ID itemid) const ;
    // where determines where the items will be counted ("all", "belt", "body", "backpack")
    int countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid, script_data_exchangemap const *data = nullptr) const;
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr);
    virtual int createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, script_data_exchangemap const *data);
    virtual int increaseAtPos(unsigned char pos, int count);
    virtual int createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count);
    virtual bool swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);
    virtual ScriptItem GetItemAt(unsigned char itempos);
    virtual Container *GetBackPack() const;
    Container *GetDepot(uint32_t depotid) const;
    std::vector<ScriptItem> getItemList(TYPE_OF_ITEM_ID id) const;

    virtual std::string getSkillName(TYPE_OF_SKILL_ID s) const;
    virtual unsigned short int getSkill(TYPE_OF_SKILL_ID s) const;
    virtual const skillvalue *getSkillValue(TYPE_OF_SKILL_ID s) const;
    virtual unsigned short int getMinorSkill(TYPE_OF_SKILL_ID s) const;

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

    bool setBaseAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);
    void setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);
    Attribute::attribute_t getBaseAttribute(Character::attributeIndex attribute) const;
    Attribute::attribute_t getAttribute(Character::attributeIndex attribute) const;
    bool increaseBaseAttribute(Character::attributeIndex attribute, int amount);
    Attribute::attribute_t increaseAttribute(Character::attributeIndex attribute, int amount);
    bool isBaseAttributeValid(Character::attributeIndex attribute, Attribute::attribute_t value) const;
    uint16_t getBaseAttributeSum() const;
    uint16_t getMaxAttributePoints() const;
    virtual bool saveBaseAttributes();
    virtual void handleAttributeChange(Character::attributeIndex attribute);
    bool isBaseAttribValid(const std::string &name, Attribute::attribute_t value) const;
    bool setBaseAttrib(const std::string &name, Attribute::attribute_t value);
    void setAttrib(const std::string &name, Attribute::attribute_t value);
    Attribute::attribute_t getBaseAttrib(const std::string &name);
    bool increaseBaseAttrib(const std::string &name, int amount);
    Attribute::attribute_t increaseAttrib(const std::string &name, int amount);

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

    virtual std::string alterSpokenMessage(const std::string &message, int languageSkill) const;
    int getLanguageSkill(int languageSkillNumber) const;

    virtual void talk(talk_type tt, const std::string &message);
    virtual void talk(talk_type tt, const std::string &german, const std::string &english);

    virtual void inform(const std::string &message, informType type = informServer) const;
    virtual void inform(const std::string &german, const std::string &english, informType type = informServer) const;

    virtual bool move(direction dir, bool active=true);

    virtual bool getNextStepDir(const position &goal, direction &dir) const;
    bool getStepList(const position &goal, std::list<direction> &steps) const;

    virtual bool Warp(const position &newPos);
    virtual bool forceWarp(const position &newPos);

    virtual void startMusic(short int title);
    virtual void defaultMusic();

    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress);
    virtual TYPE_OF_QUESTSTATUS getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const;

    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc);

    virtual bool isNewPlayer() const;
    virtual bool pageGM(const std::string &ticket);

    virtual uint32_t idleTime() const;

    virtual void sendBook(uint16_t bookID);

    void updateAppearanceForAll(bool always);
    virtual void forceUpdateAppearanceForAll();
    void updateAppearanceForPlayer(Player *target, bool always);

    virtual void performAnimation(uint8_t animID);

    typedef std::map<TYPE_OF_SKILL_ID, skillvalue> SKILLMAP;

    movement_type GetMovement() const;
    void SetMovement(movement_type tmovement);

    void AddWeight();
    void SubWeight();

    inline virtual void setClippingActive(bool tclippingActive) {}
    inline virtual bool getClippingActive() const {
        return true;
    }

    SKILLMAP skills;

    /**
    * array for the items of the character
    * 0 = backpack, 1 to MAX_BODY_ITEMS - 1: equipped items
    * MAX_BODY_ITEMS - 1 to MAX_BODY_ITEMS + MAX_BELT_SLOTS - 1: items in the belt
    */
    Item characterItems[ MAX_BODY_ITEMS + MAX_BELT_SLOTS ];

    Container *backPackContents;

    /**
    * map to the different depots
    * first param is the depot id
    * second param is the pointer to the container which stores the items in this depot
    */
    std::map<uint32_t, Container *> depotContents;

    virtual void ageInventory();

    inline bool isAlive() const {
        return alive;
    }

    virtual void setAlive(bool t);

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

    virtual void logAdmin(const std::string &message);

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
    void setPosition(const position &pos);
    void setRace(TYPE_OF_RACE_ID race);
    void setFaceTo(face_to faceTo);
    void setMagicFlags(magic_type type, uint64_t flags);

    bool _is_on_route = false;
    short int poisonvalue = 0;
    int mental_capacity = 0;
    World *_world;

    virtual bool moveToPossible(const Field *field) const;

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
    movement_type _movement = walk;
    std::vector<Attribute> attributes;
    bool alive = true;
    short int actionPoints = NP_MAX_AP;
    short int fightPoints = NP_MAX_FP;
    short int activeLanguage = 0;
    position pos = {0, 0, 0};
    bool attackmode = false;
    std::string lastSpokenText = {};
    bool isinvisible = false;
    TYPE_OF_RACE_ID race = 0;
    face_to faceto = north;    
    s_magic magic;
};

std::ostream &operator<<(std::ostream &os, const Character &character);

#endif
