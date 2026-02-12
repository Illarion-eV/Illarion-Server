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

/**
 * @brief Contains all properties and metadata for an item type.
 *
 * This structure defines the characteristics of an item including physical properties,
 * value, aging behavior, and localized names/descriptions.
 */
struct ItemStruct {
    TYPE_OF_ITEM_ID id = 0;               ///< Unique item type identifier.
    TYPE_OF_VOLUME Volume = 0;            ///< Volume/size of the item.
    TYPE_OF_WEIGHT Weight = 0;            ///< Weight of the item.
    TYPE_OF_AGINGSPEED AgeingSpeed = 0;   ///< How quickly the item ages/decays.
    TYPE_OF_ITEM_ID ObjectAfterRot = 0;   ///< Item ID this becomes after aging once.
    TYPE_OF_ITEM_ID AfterInfiniteRot = 0; ///< Item ID this becomes after complete decay.
    TYPE_OF_BRIGHTNESS Brightness = 0;    ///< Light level emitted by this item.
    TYPE_OF_WORTH Worth = 0;              ///< Base monetary value.
    TYPE_OF_MAX_STACK MaxStack = 1;       ///< Maximum items that can stack together.
    TYPE_OF_BUY_STACK BuyStack = 1;       ///< Default purchase stack size.
    bool rotsInInventory = false;         ///< Whether item ages while in inventory.
    TYPE_OF_ENGLISH serverName;           ///< Internal server name.
    TYPE_OF_ENGLISH English;              ///< English display name.
    TYPE_OF_GERMAN German;                ///< German display name.
    TYPE_OF_ENGLISH EnglishDescription;   ///< English description text.
    TYPE_OF_GERMAN GermanDescription;     ///< German description text.
    int16_t Rareness = 1;                 ///< Rarity level of the item.
    TYPE_OF_ITEMLEVEL Level = 0;          ///< Required level to use this item.

    /**
     * @brief Checks if this item struct contains valid data.
     * @return true if id is non-zero, false otherwise.
     */
    [[nodiscard]] inline auto isValid() const -> bool { return id != 0; }
};

/**
 * @brief Tile modification parameters.
 */
struct TilesModificatorStruct {
    unsigned char Modificator; ///< Modification value for tile properties.
};

/**
 * @brief Defines a long-term effect that can be applied to characters.
 */
struct LongTimeEffectStruct {
    uint16_t effectid = 0;                           ///< Unique effect identifier.
    std::string effectname;                          ///< Human-readable effect name.
    std::string scriptname;                          ///< Script file name for this effect.
    std::shared_ptr<LuaLongTimeEffectScript> script; ///< Loaded script instance.
};

/**
 * @brief Contains weapon properties and combat statistics.
 *
 * Defines weapon types, damage characteristics, and combat mechanics.
 */
struct WeaponStruct {
    static constexpr auto slashing = 1;          ///< One-handed slashing weapon.
    static constexpr auto concussion = 2;        ///< One-handed bludgeoning weapon.
    static constexpr auto puncture = 3;          ///< One-handed piercing weapon.
    static constexpr auto slashingTwoHand = 4;   ///< Two-handed slashing weapon.
    static constexpr auto concussionTwoHand = 5; ///< Two-handed bludgeoning weapon.
    static constexpr auto punctureTwoHand = 6;   ///< Two-handed piercing weapon.
    static constexpr auto firearm = 7;           ///< Ranged firearm.
    static constexpr auto arrow = 10;            ///< Arrow ammunition.
    static constexpr auto bolt = 11;             ///< Crossbow bolt ammunition.
    static constexpr auto stone = 12;            ///< Stone/thrown weapon.
    static constexpr auto stave = 13;            ///< Two-handed stave.
    static constexpr auto shield = 14;           ///< Shield (defensive weapon).

    TYPE_OF_ATTACK Attack{0};                     ///< Base attack value.
    TYPE_OF_DEFENCE Defence{0};                   ///< Base defence value.
    TYPE_OF_ACCURACY Accuracy{0};                 ///< Accuracy/hit chance modifier.
    TYPE_OF_RANGE Range{0};                       ///< Attack range.
    TYPE_OF_WEAPONTYPE Type{0};                   ///< Weapon type constant.
    TYPE_OF_AMMUNITIONTYPE AmmunitionType{0};     ///< Required ammunition type for ranged weapons.
    TYPE_OF_ACTIONPOINTS ActionPoints{0};         ///< Action points required to use.
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance{0}; ///< Magic casting penalty.
    TYPE_OF_POISONSTRENGTH PoisonStrength{0};     ///< Poison damage on hit.

    WeaponStruct() = default;

    /**
     * @brief Checks if this weapon requires two hands to wield.
     * @return true if the weapon is two-handed, false otherwise.
     */
    [[nodiscard]] inline auto isTwoHanded() const -> bool {
        return Type == slashingTwoHand || Type == concussionTwoHand || Type == punctureTwoHand || Type == stave;
    };
};

/**
 * @brief Contains armor properties and protection statistics.
 *
 * Defines armor types, body coverage, and damage resistance values.
 */
struct ArmorStruct {
    static constexpr auto clothing = 0;  ///< Clothing (no armor).
    static constexpr auto general = 1;   ///< General/untyped armor.
    static constexpr auto light = 2;     ///< Light armor.
    static constexpr auto medium = 3;    ///< Medium armor.
    static constexpr auto heavy = 4;     ///< Heavy armor.
    static constexpr auto juwellery = 5; ///< Jewelry (accessories).

    TYPE_OF_BODYPARTS BodyParts{0};               ///< Body parts covered by this armor.
    TYPE_OF_PUNCTUREARMOR PunctureArmor{0};       ///< Protection against piercing damage.
    TYPE_OF_STROKEARMOR StrokeArmor{0};           ///< Protection against slashing damage.
    TYPE_OF_THRUSTARMOR ThrustArmor{0};           ///< Protection against bludgeoning damage.
    TYPE_OF_MAGICDISTURBANCE MagicDisturbance{0}; ///< Magic casting penalty.
    int16_t Absorb{0};                            ///< Damage absorption value.
    int16_t Stiffness{0};                         ///< Movement penalty/stiffness.
    TYPE_OF_ARMORTYPE Type{0};                    ///< Armor type constant.

    ArmorStruct() = default;
};

/**
 * @brief Defines a character skill with localized names.
 */
struct SkillStruct {
    std::string serverName;      ///< Internal server name for the skill.
    TYPE_OF_ENGLISH englishName; ///< English display name.
    TYPE_OF_GERMAN germanName;   ///< German display name.
};

/**
 * @brief Contains tile/terrain properties.
 */
struct TilesStruct {
    unsigned char flags{};             ///< Tile behavior flags.
    TYPE_OF_WALKINGCOST walkingCost{}; ///< Movement cost to traverse this tile.
    TYPE_OF_GERMAN German;             ///< German name for the tile.
    TYPE_OF_ENGLISH English;           ///< English name for the tile.
};

/**
 * @brief Defines player race creation parameters and attribute limits.
 */
struct PlayerraceStruct {
    std::string racename;          ///< Race name.
    short int points;              ///< Starting attribute points.
    short int minage;              ///< Minimum starting age.
    short int maxage;              ///< Maximum starting age.
    short int minweight;           ///< Minimum character weight.
    short int maxweight;           ///< Maximum character weight.
    unsigned char minbodyheight;   ///< Minimum body height.
    unsigned char maxbodyheight;   ///< Maximum body height.
    unsigned char minagility;      ///< Minimum agility value.
    unsigned char maxagility;      ///< Maximum agility value.
    unsigned char minconstitution; ///< Minimum constitution value.
    unsigned char maxconstitution; ///< Maximum constitution value.
    unsigned char mindexterity;    ///< Minimum dexterity value.
    unsigned char maxdexterity;    ///< Maximum dexterity value.
    unsigned char minessence;      ///< Minimum essence value.
    unsigned char maxessence;      ///< Maximum essence value.
    unsigned char minintelligence; ///< Minimum intelligence value.
    unsigned char maxintelligence; ///< Maximum intelligence value.
    unsigned char minperception;   ///< Minimum perception value.
    unsigned char maxperception;   ///< Maximum perception value.
    unsigned char minstrength;     ///< Minimum strength value.
    unsigned char maxstrength;     ///< Maximum strength value.
    unsigned char minwillpower;    ///< Minimum willpower value.
    unsigned char maxwillpower;    ///< Maximum willpower value.
};

/**
 * @brief Monster armor values for different damage types.
 */
struct MonsterArmor {
    short int strokeArmor{0};   ///< Protection against slashing damage.
    short int punctureArmor{0}; ///< Protection against piercing damage.
    short int thrustArmor{0};   ///< Protection against bludgeoning damage.

    MonsterArmor() = default;
};

/**
 * @brief Defines an item with quantity and aging for inventory/equipment.
 */
struct itemdef_t {
    TYPE_OF_ITEM_ID itemid{};                         ///< Item type ID.
    std::pair<unsigned short, unsigned short> amount; ///< Min and max quantity range.
    TYPE_OF_AGINGSPEED AgeingSpeed{};                 ///< Aging speed for this item instance.
};

/**
 * @brief Character attribute ranges for generation or configuration.
 *
 * Each attribute is represented as a pair (min, max) for random generation.
 */
struct attributedef_t {
    std::pair<unsigned short, unsigned short> luck;         ///< Luck attribute range.
    std::pair<unsigned short, unsigned short> strength;     ///< Strength attribute range.
    std::pair<unsigned short, unsigned short> dexterity;    ///< Dexterity attribute range.
    std::pair<unsigned short, unsigned short> constitution; ///< Constitution attribute range.
    std::pair<unsigned short, unsigned short> agility;      ///< Agility attribute range.
    std::pair<unsigned short, unsigned short> intelligence; ///< Intelligence attribute range.
    std::pair<unsigned short, unsigned short> perception;   ///< Perception attribute range.
    std::pair<unsigned short, unsigned short> willpower;    ///< Willpower attribute range.
    std::pair<unsigned short, unsigned short> essence;      ///< Essence attribute range.

    /**
     * @brief Constructs attribute definition with default values.
     */
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
    static constexpr uint16_t defaultLuck = 10;         ///< Default luck value.
    static constexpr uint16_t defaultStrength = 15;     ///< Default strength value.
    static constexpr uint16_t defaultDexterity = 10;    ///< Default dexterity value.
    static constexpr uint16_t defaultConstitution = 8;  ///< Default constitution value.
    static constexpr uint16_t defaultAgility = 10;      ///< Default agility value.
    static constexpr uint16_t defaultIntelligence = 10; ///< Default intelligence value.
    static constexpr uint16_t defaultPerception = 10;   ///< Default perception value.
    static constexpr uint16_t defaultWillpower = 10;    ///< Default willpower value.
    static constexpr uint16_t defaultEssence = 10;      ///< Default essence value.
};

/**
 * @brief Complete monster definition including stats, loot, and behavior.
 */
struct MonsterStruct {
    /**
     * @brief Defines an item that can drop as loot with probability and variance.
     */
    struct LootStruct {
        TYPE_OF_ITEM_ID itemId{};                 ///< Item type that can drop.
        double probability{};                     ///< Drop chance (0.0 to 1.0).
        std::pair<uint16_t, uint16_t> amount;     ///< Min and max quantity range.
        std::pair<uint16_t, uint16_t> quality;    ///< Min and max quality range.
        std::pair<uint16_t, uint16_t> durability; ///< Min and max durability range.
        std::map<std::string, std::string> data;  ///< Additional item data key-value pairs.
    };

    using skilltype =
            std::map<TYPE_OF_SKILL_ID, std::pair<unsigned short, unsigned short>>; ///< Skill map with min/max values.
    using itemtype = std::map<unsigned short, std::vector<itemdef_t>>;   ///< Equipment slots mapped to possible items.
    using loottype = std::map<uint16_t, std::map<uint16_t, LootStruct>>; ///< Categorized loot tables.

    std::string nameDe;                       ///< German monster name.
    std::string nameEn;                       ///< English monster name.
    TYPE_OF_RACE_ID race{};                   ///< Monster race identifier.
    unsigned short hitpoints{};               ///< Maximum hit points.
    bool canselfheal{};                       ///< Whether monster can regenerate health.
    movement_type movement{};                 ///< Movement type (walk/fly/crawl).
    bool canattack{};                         ///< Whether monster can initiate combat.
    attributedef_t attributes;                ///< Monster attribute ranges.
    skilltype skills;                         ///< Monster skill ranges.
    itemtype items;                           ///< Starting equipment by slot.
    loottype loot;                            ///< Loot drop tables.
    std::shared_ptr<LuaMonsterScript> script; ///< AI/behavior script.
    uint16_t minsize{};                       ///< Minimum monster size.
    uint16_t maxsize{};                       ///< Maximum monster size.
};

/**
 * @brief Race-specific constraints and defaults for character creation.
 */
struct RaceStruct {
    std::string serverName;                         ///< Internal race identifier.
    uint16_t minSize = defaultMinHeight;            ///< Minimum character height.
    uint16_t maxSize = defaultMaxHeight;            ///< Maximum character height.
    uint8_t minAgility = defaultMinAttribute;       ///< Minimum agility attribute.
    uint8_t maxAgility = defaultMaxAttribute;       ///< Maximum agility attribute.
    uint8_t minConstitution = defaultMinAttribute;  ///< Minimum constitution attribute.
    uint8_t maxConstitution = defaultMaxAttribute;  ///< Maximum constitution attribute.
    uint8_t minDexterity = defaultMinAttribute;     ///< Minimum dexterity attribute.
    uint8_t maxDexterity = defaultMaxAttribute;     ///< Maximum dexterity attribute.
    uint8_t minEssence = defaultMinAttribute;       ///< Minimum essence attribute.
    uint8_t maxEssence = defaultMaxAttribute;       ///< Maximum essence attribute.
    uint8_t minIntelligence = defaultMinAttribute;  ///< Minimum intelligence attribute.
    uint8_t maxIntelligence = defaultMaxAttribute;  ///< Maximum intelligence attribute.
    uint8_t minPerception = defaultMinAttribute;    ///< Minimum perception attribute.
    uint8_t maxPerception = defaultMaxAttribute;    ///< Maximum perception attribute.
    uint8_t minStrength = defaultMinAttribute;      ///< Minimum strength attribute.
    uint8_t maxStrength = defaultMaxAttribute;      ///< Maximum strength attribute.
    uint8_t minWillpower = defaultMinAttribute;     ///< Minimum willpower attribute.
    uint8_t maxWillpower = defaultMaxAttribute;     ///< Maximum willpower attribute.
    uint8_t maxAttribs = defaultMaxAttributePoints; ///< Total attribute points allowed.

    static constexpr uint16_t defaultMinHeight = 100;        ///< Default minimum height.
    static constexpr uint16_t defaultMaxHeight = 100;        ///< Default maximum height.
    static constexpr uint8_t defaultMinAttribute = 2;        ///< Default minimum attribute value.
    static constexpr uint8_t defaultMaxAttribute = 20;       ///< Default maximum attribute value.
    static constexpr uint8_t defaultMaxAttributePoints = 84; ///< Default total attribute points.
};

/**
 * @brief Character appearance configuration for a specific race.
 */
struct RaceConfiguration {
    uint32_t subType = 0; ///< Race subtype/variant identifier.
    uint16_t hair = 0;    ///< Hair style identifier.
    uint16_t beard = 0;   ///< Beard style identifier.
    Colour hairColour;    ///< Hair color.
    Colour skinColour;    ///< Skin color.
};

#endif
