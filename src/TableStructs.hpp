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

#ifndef TABLESTRUCTS_HPP
#define TABLESTRUCTS_HPP

#include "types.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class LuaTileScript;
class LuaItemScript;
class LuaMonsterScript;
class LuaWeaponScript;
class LuaLongTimeEffectScript;

struct ItemStruct {
    TYPE_OF_ITEM_ID id = 0;
    TYPE_OF_VOLUME Volume = 0;
    TYPE_OF_WEIGHT Weight = 0;
    TYPE_OF_AGINGSPEED AgeingSpeed = 0;
    TYPE_OF_ITEM_ID ObjectAfterRot = 0;
    TYPE_OF_ITEM_ID AfterInfiniteRot = 0;
    TYPE_OF_BRIGHTNESS Brightness = 0;
    TYPE_OF_WORTH Worth = 0;
    TYPE_OF_MAX_STACK MaxStack = 1;
    TYPE_OF_BUY_STACK BuyStack = 1;
    bool rotsInInventory = false;
    TYPE_OF_ENGLISH serverName;
    TYPE_OF_ENGLISH English;
    TYPE_OF_GERMAN German;
    TYPE_OF_ENGLISH EnglishDescription;
    TYPE_OF_GERMAN GermanDescription;
    int16_t Rareness = 1;
    TYPE_OF_ITEMLEVEL Level = 0;

    [[nodiscard]] auto isValid() const -> bool { return id != 0; }
};

struct TilesModificatorStruct {
    unsigned char Modificator;
};

struct LongTimeEffectStruct {
    uint16_t effectid;
    std::string effectname;
    std::string scriptname;
    std::shared_ptr<LuaLongTimeEffectScript> script;
    LongTimeEffectStruct() { effectid = 0; }
};

struct WeaponStruct {
    TYPE_OF_ATTACK Attack{0};
    TYPE_OF_DEFENCE Defence{0};
    TYPE_OF_ACCURACY Accuracy{0};
    TYPE_OF_RANGE Range{0};
    TYPE_OF_WEAPONTYPE Type{0};
    TYPE_OF_AMMUNITIONTYPE AmmunitionType{0};
    TYPE_OF_ACTIONPOINTS ActionPoints{0};
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance{0};
    TYPE_OF_POISONSTRENGTH PoisonStrength{0};
    WeaponStruct() = default;
};

struct ArmorStruct {
    TYPE_OF_BODYPARTS BodyParts{0};
    TYPE_OF_PUNCTUREARMOR PunctureArmor{0};
    TYPE_OF_STROKEARMOR StrokeArmor{0};
    TYPE_OF_THRUSTARMOR ThrustArmor{0};
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance{0};
    int16_t Absorb{0};
    int16_t Stiffness{0};
    TYPE_OF_ARMORTYPE Type{0};
    ArmorStruct() = default;
};

struct SkillStruct {
    std::string serverName;
    TYPE_OF_ENGLISH englishName;
    TYPE_OF_GERMAN germanName;
};

struct TilesStruct {
    unsigned char flags{};
    TYPE_OF_WALKINGCOST walkingCost{};
    TYPE_OF_GERMAN German;
    TYPE_OF_ENGLISH English;
};

struct PlayerraceStruct {
    std::string racename;
    short int points;
    short int minage;
    short int maxage;
    short int minweight;
    short int maxweight;
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

struct MonsterArmor {
    short int strokeArmor{0};
    short int punctureArmor{0};
    short int thrustArmor{0};
    MonsterArmor() = default;
};

struct itemdef_t {
    TYPE_OF_ITEM_ID itemid{};
    std::pair<unsigned short, unsigned short> amount;
    TYPE_OF_AGINGSPEED AgeingSpeed{};
};

struct attributedef_t {
    std::pair<unsigned short, unsigned short> luck;
    std::pair<unsigned short, unsigned short> strength;
    std::pair<unsigned short, unsigned short> dexterity;
    std::pair<unsigned short, unsigned short> constitution;
    std::pair<unsigned short, unsigned short> agility;
    std::pair<unsigned short, unsigned short> intelligence;
    std::pair<unsigned short, unsigned short> perception;
    std::pair<unsigned short, unsigned short> willpower;
    std::pair<unsigned short, unsigned short> essence;

    attributedef_t()
            : luck(std::make_pair(defaultLuck, defaultLuck)),
              strength(std::make_pair(defaultStrength, defaultStrength)),
              dexterity(std::make_pair(defaultDexterity, defaultDexterity)),
              constitution(std::make_pair(defaultConstitution, defaultConstitution)),
              agility(std::make_pair(defaultAgility, defaultAgility)),
              intelligence(std::make_pair(defaultIntelligence, defaultIntelligence)),
              perception(std::make_pair(defaultPerception, defaultPerception)),
              willpower(std::make_pair(defaultWillpower, defaultWillpower)),
              essence(std::make_pair(defaultEssence, defaultEssence)) {}

private:
    static constexpr uint16_t defaultLuck = 10;
    static constexpr uint16_t defaultStrength = 15;
    static constexpr uint16_t defaultDexterity = 10;
    static constexpr uint16_t defaultConstitution = 8;
    static constexpr uint16_t defaultAgility = 10;
    static constexpr uint16_t defaultIntelligence = 10;
    static constexpr uint16_t defaultPerception = 10;
    static constexpr uint16_t defaultWillpower = 10;
    static constexpr uint16_t defaultEssence = 10;
};

struct MonsterStruct {
    struct LootStruct {
        TYPE_OF_ITEM_ID itemId{};
        double probability{};
        std::pair<uint16_t, uint16_t> amount;
        std::pair<uint16_t, uint16_t> quality;
        std::pair<uint16_t, uint16_t> durability;
        std::map<std::string, std::string> data;
    };

    using skilltype = std::map<TYPE_OF_SKILL_ID, std::pair<unsigned short, unsigned short>>;
    using itemtype = std::map<unsigned short, std::vector<itemdef_t>>;
    using loottype = std::map<uint16_t, std::map<uint16_t, LootStruct>>;
    std::string nameDe;
    std::string nameEn;
    TYPE_OF_RACE_ID race{};
    unsigned short hitpoints{};
    bool canselfheal{};
    movement_type movement{};
    bool canattack{};
    attributedef_t attributes;
    skilltype skills;
    itemtype items;
    loottype loot;
    std::shared_ptr<LuaMonsterScript> script;
    uint16_t minsize{};
    uint16_t maxsize{};
};

struct RaceStruct {
    std::string serverName;
    uint16_t minSize = defaultMinHeight;
    uint16_t maxSize = defaultMaxHeight;
    uint8_t minAgility = defaultMinAttribute;
    uint8_t maxAgility = defaultMaxAttribute;
    uint8_t minConstitution = defaultMinAttribute;
    uint8_t maxConstitution = defaultMaxAttribute;
    uint8_t minDexterity = defaultMinAttribute;
    uint8_t maxDexterity = defaultMaxAttribute;
    uint8_t minEssence = defaultMinAttribute;
    uint8_t maxEssence = defaultMaxAttribute;
    uint8_t minIntelligence = defaultMinAttribute;
    uint8_t maxIntelligence = defaultMaxAttribute;
    uint8_t minPerception = defaultMinAttribute;
    uint8_t maxPerception = defaultMaxAttribute;
    uint8_t minStrength = defaultMinAttribute;
    uint8_t maxStrength = defaultMaxAttribute;
    uint8_t minWillpower = defaultMinAttribute;
    uint8_t maxWillpower = defaultMaxAttribute;
    uint8_t maxAttribs = defaultMaxAttributePoints;

    static constexpr uint16_t defaultMinHeight = 100;
    static constexpr uint16_t defaultMaxHeight = 100;
    static constexpr uint8_t defaultMinAttribute = 2;
    static constexpr uint8_t defaultMaxAttribute = 20;
    static constexpr uint8_t defaultMaxAttributePoints = 84;
};

struct RaceConfiguration {
    uint32_t subType = 0;
    uint16_t hair = 0;
    uint16_t beard = 0;
    Colour hairColour;
    Colour skinColour;
};

#endif
