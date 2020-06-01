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

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "types.hpp"

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

    [[nodiscard]] auto isValid() const -> bool {
        return id != 0;
    }
};

struct TilesModificatorStruct {
    unsigned char Modificator;
};

struct LongTimeEffectStruct {
    uint16_t effectid;
    std::string effectname;
    std::string scriptname;
    std::shared_ptr<LuaLongTimeEffectScript> script;
    LongTimeEffectStruct() {
        effectid = 0;
    }
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
    WeaponStruct()  = default;
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
    ArmorStruct()  = default;
};


struct SkillStruct {
    std::string serverName;
    TYPE_OF_ENGLISH englishName;
    TYPE_OF_GERMAN germanName;
};


struct TilesStruct {
    unsigned char flags;
    TYPE_OF_WALKINGCOST walkingCost;
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
    MonsterArmor()  = default;
};

struct itemdef_t {
    TYPE_OF_ITEM_ID itemid;
    std::pair<unsigned short, unsigned short> amount;
    TYPE_OF_AGINGSPEED AgeingSpeed;
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

    attributedef_t():
            luck(std::make_pair(10, 10)),
            strength(std::make_pair(15, 15)),
            dexterity(std::make_pair(10, 10)),
            constitution(std::make_pair(8, 8)),
            agility(std::make_pair(10, 10)),
            intelligence(std::make_pair(10, 10)),
            perception(std::make_pair(10, 10)),
            willpower(std::make_pair(10, 10)),
            essence(std::make_pair(10, 10))
    {}

};

struct MonsterStruct {
    struct LootStruct {
        TYPE_OF_ITEM_ID itemId;
        double probability;
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
    TYPE_OF_RACE_ID race;
    unsigned short hitpoints;
    bool canselfheal;
    movement_type movement;
    bool canattack;
    attributedef_t  attributes;
    skilltype skills;
    itemtype items;
    loottype loot;
    std::shared_ptr<LuaMonsterScript> script;
    uint16_t minsize;
    uint16_t maxsize;
};

struct RaceStruct {
    std::string serverName;
    uint16_t minSize = 140;
    uint16_t maxSize = 220;
    uint8_t minAgility = 2;
    uint8_t maxAgility = 20;
    uint8_t minConstitution = 2;
    uint8_t maxConstitution = 20;
    uint8_t minDexterity = 2;
    uint8_t maxDexterity = 20;
    uint8_t minEssence = 2;
    uint8_t maxEssence = 20;
    uint8_t minIntelligence = 2;
    uint8_t maxIntelligence = 20;
    uint8_t minPerception = 2;
    uint8_t maxPerception = 20;
    uint8_t minStrength = 2;
    uint8_t maxStrength = 20;
    uint8_t minWillpower = 2;
    uint8_t maxWillpower = 20;
    uint8_t maxAttribs = 84;
};

struct RaceConfiguration {
    uint32_t subType = 0;
    uint16_t hair = 0;
    uint16_t beard = 0;
    Colour hairColour;
    Colour skinColour;
};

#endif
