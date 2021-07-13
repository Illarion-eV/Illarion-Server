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

#include "Attribute.hpp"
#include "Item.hpp"
#include "ItemLookAt.hpp"
#include "Language.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "TableStructs.hpp"
#include "WaypointList.hpp"
#include "constants.hpp"
#include "dialog/CraftingDialog.hpp"
#include "dialog/InputDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "tuningConstants.hpp"

#include <array>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>

class World;
class Container;
namespace map {
class Field;
}
class Player;

enum magic_type { MAGE = 0, PRIEST = 1, BARD = 2, DRUID = 3 };

class NoLootFound : public std::exception {};

struct SkillValue {
    int major = 0;
    int minor = 0;
};

class Character {
public:
    struct appearance {
        uint8_t hairtype = 0;
        uint8_t beardtype = 0;
        Colour hair;
        Colour skin;

        appearance() = default;
    };

    Character();
    explicit Character(const appearance &appearance);
    virtual ~Character();
    Character(const Character &) = delete;
    auto operator=(const Character &) -> Character & = delete;
    Character(Character &&) = default;
    auto operator=(Character &&) -> Character & = default;

    enum character_type { player = 0, monster = 1, npc = 2 };

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

    using attribute_map_t = std::unordered_map<std::string, attributeIndex>;
    using attribute_string_map_t = std::unordered_map<attributeIndex, std::string, std::hash<int>>;
    static attribute_map_t attributeMap;
    static attribute_string_map_t attributeStringMap;

    enum talk_type { tt_say = 0, tt_whisper = 1, tt_yell = 2 };

    enum sex_type { male = 0, female = 1 };

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

    using skillvalue = SkillValue;

    struct s_magic {
        magic_type type;
        std::array<unsigned long int, 4> flags;
    };

    virtual auto getId() const -> TYPE_OF_CHARACTER_ID;
    auto getName() const -> const std::string &;
    virtual auto to_string() const -> std::string = 0;

    static constexpr auto actionPointUnit = 100;

    auto getActionPoints() const -> int;
    virtual auto getMinActionPoints() const -> int;
    virtual auto getMaxActionPoints() const -> int;
    void setActionPoints(int ap);
    void increaseActionPoints(int ap);
    auto canAct() const -> bool;

    auto getFightPoints() const -> int;
    virtual auto getMinFightPoints() const -> int;
    virtual auto getMaxFightPoints() const -> int;
    void setFightPoints(int fp);
    void increaseFightPoints(int fp);
    auto canFight() const -> bool;

    auto getSpeed() const -> double;
    void setSpeed(double spd);

    auto getActiveLanguage() const -> short int;
    void setActiveLanguage(short int l);

    virtual auto getPosition() const -> const position &;

    auto getAttackMode() const -> bool;
    void setAttackMode(bool attack);

    auto getLastSpokenText() const -> const std::string &;

    auto isInvisible() const -> bool;
    void setInvisible(bool invisible);

    LongTimeCharacterEffects effects;
    WaypointList waypoints;

    auto getFrontalPosition() const -> position;

    virtual auto actionRunning() const -> bool { return false; }

    inline auto getMagicType() const -> unsigned short { return magic.type; }

    inline virtual void setMagicType(magic_type newMagType) { magic.type = newMagType; }

    void setOnRoute(bool onr) { _is_on_route = onr; }

    auto getOnRoute() const -> bool { return _is_on_route; }

    virtual auto getPlayerLanguage() const -> Language { return Language::english; }

    inline auto getMagicFlags(unsigned char type) const -> unsigned long int {
        if (type < 4) {
            return magic.flags.at(type);
        }
        return 0;
    }

    virtual auto getType() const -> unsigned short = 0;

    virtual void changeRace(TYPE_OF_RACE_ID race) {
        this->race = race;
        updateAppearanceForAll(true);
    }

    virtual auto getRace() const -> TYPE_OF_RACE_ID { return race; }

    virtual auto getFaceTo() const -> face_to { return faceto; }

    virtual auto isAdmin() const -> bool { return false; }

    virtual auto getMonsterType() const -> TYPE_OF_CHARACTER_ID { return 0; }

    virtual void changeQualityAt(unsigned char pos, int amount);
    virtual void increasePoisonValue(int value);

    virtual auto getPoisonValue() const -> int { return poisonvalue; }

    virtual void setPoisonValue(int value) { poisonvalue = value; }

    /**
     * starts a new longtime action for this character (overloaded in Player)
     * <b>Lua: [:startAction]</b>
     * @param wait time to wait until the action is successfull
     * @param ani the animation which should be shown for this action, if not set this value is 0
     * @param redoani after how much 1/10s the animation should be shown again, if not set this value is 0 (0 never)
     * @param sound the sound which should be played for this action, if not set this value is 0
     * @param redosound after hoch much 1/10s the sound is played again, if not set this value is 0, 0 means never
     */
    inline virtual void startAction(unsigned short int wait, unsigned short int ani = 0, unsigned short int redoani = 0,
                                    unsigned short int sound = 0, unsigned short int redosound = 0) {}

    inline virtual void abortAction() {}

    inline virtual void successAction() {}

    inline virtual void actionDisturbed(Character *disturber) {}

    inline virtual void changeSource(Character *cc) {}

    inline virtual void changeSource(const ScriptItem &sI) {}

    inline virtual void changeSource(const position &pos) {}

    inline virtual void changeSource() {}

    virtual auto getMentalCapacity() const -> int { return mental_capacity; }

    virtual void setMentalCapacity(int value) { mental_capacity = value; }

    virtual void increaseMentalCapacity(int value);

    virtual auto countItem(TYPE_OF_ITEM_ID itemid) const -> int;
    // where determines where the items will be counted ("all", "belt", "body", "backpack")
    auto countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid,
                     script_data_exchangemap const *data = nullptr) const -> int;
    virtual auto eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr) -> int;
    virtual auto createItem(Item::id_type id, Item::number_type number, Item::quality_type quality,
                            script_data_exchangemap const *data) -> int;
    virtual auto increaseAtPos(unsigned char pos, int count) -> int;
    virtual auto createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) -> int;
    virtual auto swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int newQuality = 0) -> bool;
    virtual auto GetItemAt(unsigned char itempos) -> ScriptItem;
    virtual auto GetBackPack() const -> Container *;
    auto GetDepot(uint32_t depotid) const -> Container *;
    auto getItemList(TYPE_OF_ITEM_ID id) -> std::vector<ScriptItem>;

    virtual auto getSkillName(TYPE_OF_SKILL_ID s) const -> std::string;
    virtual auto getSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int;
    virtual auto getSkillValue(TYPE_OF_SKILL_ID s) const -> const skillvalue *;
    virtual auto getMinorSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int;

    void setSkinColour(const Colour &c);
    auto getSkinColour() const -> Colour;
    void setHairColour(const Colour &c);
    auto getHairColour() const -> Colour;
    void setHair(uint8_t hairID);
    auto getHair() const -> uint8_t;
    void setBeard(uint8_t beardID);
    auto getBeard() const -> uint8_t;

    auto getAppearance() const -> appearance { return _appearance; }

    auto setBaseAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) -> bool;
    void setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);
    auto getBaseAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t;
    auto getAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t;
    auto increaseBaseAttribute(Character::attributeIndex attribute, int amount) -> bool;
    auto increaseAttribute(Character::attributeIndex attribute, int amount) -> Attribute::attribute_t;
    auto isBaseAttributeValid(Character::attributeIndex attribute, Attribute::attribute_t value) const -> bool;
    auto getBaseAttributeSum() const -> uint16_t;
    auto getMaxAttributePoints() const -> uint16_t;
    virtual auto saveBaseAttributes() -> bool;
    virtual void handleAttributeChange(Character::attributeIndex attribute);
    auto isBaseAttribValid(const std::string &name, Attribute::attribute_t value) const -> bool;
    auto setBaseAttrib(const std::string &name, Attribute::attribute_t value) -> bool;
    void setAttrib(const std::string &name, Attribute::attribute_t value);
    auto getBaseAttrib(const std::string &name) const -> Attribute::attribute_t;
    auto increaseBaseAttrib(const std::string &name, int amount) -> bool;
    auto increaseAttrib(const std::string &name, int amount) -> Attribute::attribute_t;

    virtual auto increaseSkill(TYPE_OF_SKILL_ID skill, int amount) -> int;
    virtual auto increaseMinorSkill(TYPE_OF_SKILL_ID skill, int amount) -> int;
    virtual auto setSkill(TYPE_OF_SKILL_ID skill, int major, int minor) -> int;
    virtual void deleteAllSkills();
    virtual void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent);
    virtual void teachMagic(unsigned char type, unsigned char flag);

    auto isInRange(Character *cc, Coordinate distancemetric) const -> bool;
    auto isInScreen(const position &pos) const -> bool;
    virtual auto getScreenRange() const -> Coordinate;
    auto distanceMetric(Character *cc) const -> Coordinate;
    auto isInRangeToField(const position &m_pos, Coordinate distancemetric) const -> bool;
    auto distanceMetricToPosition(const position &m_pos) const -> Coordinate;

    virtual void talk(talk_type tt, const std::string &message);
    virtual void talk(talk_type tt, const std::string &german, const std::string &english);

    virtual void inform(const std::string &message, informType type = informServer) const;
    virtual void inform(const std::string &german, const std::string &english, informType type = informServer) const;

    virtual auto move(direction dir, bool active = true) -> bool;

    virtual auto getNextStepDir(const position &goal, direction &dir) const -> bool;
    auto getStepList(const position &goal, std::list<direction> &steps) const -> bool;

    virtual auto Warp(const position &newPos) -> bool;
    virtual auto forceWarp(const position &newPos) -> bool;

    virtual void startMusic(short int title);
    virtual void defaultMusic();

    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress);
    virtual auto getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const -> TYPE_OF_QUESTSTATUS;

    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc);

    virtual auto isNewPlayer() const -> bool;
    virtual auto pageGM(const std::string &ticket) -> bool;

    virtual auto idleTime() const -> uint32_t;

    virtual void sendBook(uint16_t bookID);

    void updateAppearanceForAll(bool always);
    virtual void forceUpdateAppearanceForAll();
    void updateAppearanceForPlayer(Player *target, bool always);

    virtual void performAnimation(uint8_t animID);

    using SKILLMAP = std::map<TYPE_OF_SKILL_ID, skillvalue>;

    auto GetMovement() const -> movement_type;
    void SetMovement(movement_type tmovement);

    void AddWeight();
    void SubWeight();

    inline virtual void setClippingActive(bool tclippingActive) {}
    inline virtual auto getClippingActive() const -> bool { return true; }

    SKILLMAP skills;

    /**
     * array for the items of the character
     * 0 = backpack, 1 to MAX_BODY_ITEMS - 1: equipped items
     * MAX_BODY_ITEMS - 1 to MAX_BODY_ITEMS + MAX_BELT_SLOTS - 1: items in the belt
     */
    std::array<Item, MAX_BODY_ITEMS + MAX_BELT_SLOTS> items = {};

    Container *backPackContents{nullptr};

    /**
     * map to the different depots
     * first param is the depot id
     * second param is the pointer to the container which stores the items in this depot
     */
    std::map<uint32_t, Container *> depotContents;

    virtual void ageInventory();

    inline auto isAlive() const -> bool { return alive; }

    virtual void setAlive(bool t);

    character_type enemytype = player;
    TYPE_OF_CHARACTER_ID enemyid = 0;
    virtual auto attack(Character *target) -> bool;
    virtual void stopAttack();
    auto getAttackTarget() const -> character_ptr;

    auto maxLoadWeight() const -> unsigned short int;
    auto LoadWeight() const -> int;
    auto relativeLoad() const -> double;

    enum class LoadLevel { unburdened, burdened, overtaxed };

    auto loadFactor() const -> LoadLevel;

    /**
     * returns the weight of a container
     * @param id of the container from which we want to get the weight
     * @param count if the weight should be positive or negative
     * @param tcont the container from which we want to get the weight
     * @return the +/- weight of the container
     */
    static auto weightContainer(unsigned short int id, int count, Container *tcont) -> int;

    /**
     * checks if in tcont is enough place for count items with id
     * @param id the item which should be placed in the container
     * @param count how much items should be added to the container
     * @param tcont pointer to the container (items inside the container)
     * @return true if count items of id can be added otherwise false
     */
    auto weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const -> bool;

    virtual void turn(direction dir);
    virtual void turn(const position &posi);

    virtual void receiveText(talk_type tt, const std::string &message, Character *cc);

    virtual void introducePlayer(Player *player);

    void callAttackScript(Character *Attacker, Character *Defender);

    virtual void requestInputDialog(InputDialog *inputDialog);
    virtual void requestMessageDialog(MessageDialog *messageDialog);
    virtual void requestMerchantDialog(MerchantDialog *merchantDialog);
    virtual void requestSelectionDialog(SelectionDialog *selectionDialog);
    virtual void requestCraftingDialog(CraftingDialog *craftingDialog);
    virtual void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt);
    virtual void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt);

    virtual void logAdmin(const std::string &message);

    virtual auto getLoot() const -> const MonsterStruct::loottype &;

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
    int poisonvalue = 0;
    int mental_capacity = 0;
    World *_world;

    virtual auto moveToPossible(const map::Field &field) const -> bool;

    // returns time of a move in ms
    virtual auto getMoveTime(const map::Field &targetField, bool diagonalMove, bool running) const
            -> TYPE_OF_WALKINGCOST;

    auto canTalk(talk_type tt) const -> bool;
    static /*consteval*/ auto talkCost(talk_type tt) -> int;
    void logTalk(talk_type tt, const std::string &message) const;
    auto talkScript(talk_type tt, const std::string &message) -> std::string;

    appearance _appearance;

private:
    TYPE_OF_CHARACTER_ID id = 0;
    std::string name;
    movement_type _movement = movement_type::walk;
    std::vector<Attribute> attributes;
    bool alive = true;
    int actionPoints = NP_MAX_AP;
    int fightPoints = NP_MAX_FP;
    short int activeLanguage = 0;
    position pos = {0, 0, 0};
    bool attackmode = false;
    std::string lastSpokenText = {};
    bool isinvisible = false;
    TYPE_OF_RACE_ID race = 0;
    face_to faceto = north;
    s_magic magic{};
    double speed = 1.0;
};

auto operator<<(std::ostream &os, const Character &character) -> std::ostream &;

#endif
