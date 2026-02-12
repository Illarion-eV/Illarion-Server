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

/**
 * @brief Magic school types available in the game
 */
enum magic_type { MAGE = 0, PRIEST = 1, BARD = 2, DRUID = 3 };

/**
 * @brief Exception thrown when no loot items are found on a character
 */
class NoLootFound : public std::exception {};

/**
 * @brief Represents a skill value with major and minor components
 * 
 * Skills are stored as major (main skill level) and minor (experience points
 * toward next level) values.
 */
struct SkillValue {
    int major = 0; ///< Main skill level (0-100)
    int minor = 0; ///< Experience points toward next major level
};

/**
 * @brief Abstract base class for all character types in the game
 * 
 * Character is the foundation for Players, NPCs, and Monsters. It manages:
 * - Character attributes (strength, dexterity, constitution, etc.)
 * - Inventory system (equipped items, belt, backpack, depots)
 * - Skills and magic abilities
 * - Position, movement, and pathfinding
 * - Combat and health management
 * - Long-term effects and status conditions
 * - Dialog system integration
 * - Action points and fight points for turn-based mechanics
 * 
 * @note This is an abstract class - use Player, Monster, or NPC subclasses
 * @note Character objects are non-copyable but movable
 */
class Character {
public:
    /**
     * @brief Character visual appearance settings
     */
    struct appearance {
        uint8_t hairtype = 0;  ///< Hair style ID
        uint8_t beardtype = 0; ///< Beard style ID (0 = no beard)
        Colour hair;           ///< Hair color
        Colour skin;           ///< Skin color

        appearance() = default;
    };

    /**
     * @brief Default constructor - creates character with default appearance
     */
    Character();

    /**
     * @brief Constructs character with specific appearance
     * @param appearance Initial appearance settings
     */
    explicit Character(const appearance &appearance);

    /**
     * @brief Virtual destructor - cleans up backpack and depot containers
     */
    virtual ~Character();

    Character(const Character &) = delete;
    auto operator=(const Character &) -> Character & = delete;
    Character(Character &&) = default;
    auto operator=(Character &&) -> Character & = default;

    /**
     * @brief Type of character entity
     */
    enum character_type { player = 0, monster = 1, npc = 2 };

    /**
     * @brief Character attribute indices
     * 
     * Defines all character attributes including stats, vitals, and descriptors.
     */
    enum attributeIndex {
        strength,      ///< Physical power
        dexterity,     ///< Fine motor control and precision
        constitution,  ///< Physical resilience and health
        agility,       ///< Speed and reflexes
        intelligence,  ///< Mental capacity and magic power
        perception,    ///< Awareness and sensory acuity
        willpower,     ///< Mental fortitude and resistance
        essence,       ///< Magical essence and mana regeneration
        hitpoints,     ///< Current health points
        mana,          ///< Current magic points
        foodlevel,     ///< Current satiation level
        sex,           ///< Character gender (0=male, 1=female)
        age,           ///< Character age in years
        weight,        ///< Character body weight
        height,        ///< Character body height
        attitude,      ///< Disposition toward other characters
        luck,          ///< Random event modifier
        ATTRIBUTECOUNT ///< Total number of attributes
    };

    using attribute_map_t = std::unordered_map<std::string, attributeIndex>;
    using attribute_string_map_t = std::unordered_map<attributeIndex, std::string, std::hash<int>>;
    static attribute_map_t attributeMap;          ///< Maps attribute names to indices
    static attribute_string_map_t attributeStringMap; ///< Maps attribute indices to names

    /**
     * @brief Speech range types
     */
    enum talk_type { tt_say = 0, tt_whisper = 1, tt_yell = 2 };

    /**
     * @brief Biological sex
     */
    enum sex_type { male = 0, female = 1 };

    /**
     * @brief Direction the character is facing
     */
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

    /**
     * @brief Message priority levels for inform() method
     */
    enum informType {
        informServer = 0,              ///< Server message
        informBroadcast = 1,           ///< Broadcast to all players
        informGM = 2,                  ///< GM-only message
        informScriptLowPriority = 100,    ///< Low priority script message
        informScriptMediumPriority = 101, ///< Medium priority script message
        informScriptHighPriority = 102    ///< High priority script message
    };

    using skillvalue = SkillValue;

    /**
     * @brief Magic school configuration and learned spells
     */
    struct s_magic {
        magic_type type;                       ///< Current active magic school
        std::array<unsigned long int, 4> flags; ///< Bitflags for learned spells per school
    };

    /**
     * @brief Gets the unique character ID
     * @return Character's ID
     */
    virtual auto getId() const -> TYPE_OF_CHARACTER_ID;

    /**
     * @brief Gets the character's name
     * @return Reference to character name string
     */
    auto getName() const -> const std::string &;

    /**
     * @brief Gets string representation of the character
     * @return String describing the character (pure virtual)
     */
    virtual auto to_string() const -> std::string = 0;

    static constexpr auto actionPointUnit = 100; ///< Base unit for action point calculations

    // Action Points system
    /**
     * @brief Gets current action points
     * @return Current AP value
     */
    auto getActionPoints() const -> int;

    /**
     * @brief Gets minimum possible action points
     * @return Minimum AP value (can be overridden by subclasses)
     */
    virtual auto getMinActionPoints() const -> int;

    /**
     * @brief Gets maximum possible action points
     * @return Maximum AP value (can be overridden by subclasses)
     */
    virtual auto getMaxActionPoints() const -> int;

    /**
     * @brief Sets action points, clamping to maximum
     * @param ap New action point value
     */
    void setActionPoints(int ap);

    /**
     * @brief Increases action points by amount
     * @param ap Amount to add (can be negative)
     */
    void increaseActionPoints(int ap);

    /**
     * @brief Checks if character can perform actions
     * @return True if AP >= max AP
     */
    auto canAct() const -> bool;

    // Fight Points system
    /**
     * @brief Gets current fight points
     * @return Current FP value
     */
    auto getFightPoints() const -> int;

    /**
     * @brief Gets minimum fight points needed for combat
     * @return Minimum FP value (can be overridden)
     */
    virtual auto getMinFightPoints() const -> int;

    /**
     * @brief Gets maximum possible fight points
     * @return Maximum FP value (can be overridden)
     */
    virtual auto getMaxFightPoints() const -> int;

    /**
     * @brief Sets fight points, clamping to maximum
     * @param fp New fight point value
     */
    void setFightPoints(int fp);

    /**
     * @brief Increases fight points by amount
     * @param fp Amount to add (can be negative)
     */
    void increaseFightPoints(int fp);

    /**
     * @brief Checks if character can engage in combat
     * @return True if FP >= min FP
     */
    auto canFight() const -> bool;

    /**
     * @brief Gets character movement speed multiplier
     * @return Speed factor (1.0 = normal)
     */
    auto getSpeed() const -> double;

    /**
     * @brief Sets character movement speed multiplier
     * @param spd Speed factor
     */
    void setSpeed(double spd);

    /**
     * @brief Gets currently active language for speech
     * @return Language ID
     */
    auto getActiveLanguage() const -> short int;

    /**
     * @brief Sets active language for character speech
     * @param l Language ID
     */
    void setActiveLanguage(short int l);

    /**
     * @brief Gets current world position
     * @return Reference to position coordinates
     */
    virtual auto getPosition() const -> const position &;

    /**
     * @brief Checks if character is in attack mode
     * @return True if in attack mode
     */
    auto getAttackMode() const -> bool;

    /**
     * @brief Sets attack mode state
     * @param attack True to enable attack mode
     */
    void setAttackMode(bool attack);

    /**
     * @brief Gets the last text spoken by this character
     * @return Reference to last spoken message
     */
    auto getLastSpokenText() const -> const std::string &;

    /**
     * @brief Checks if character is invisible to others
     * @return True if invisible
     */
    auto isInvisible() const -> bool;

    /**
     * @brief Sets character invisibility
     * @param invisible True to make invisible
     */
    void setInvisible(bool invisible);

    LongTimeCharacterEffects effects; ///< Active long-term status effects
    WaypointList waypoints;          ///< Pathfinding waypoint queue

    /**
     * @brief Gets position directly in front of character
     * @return Position one tile ahead in facing direction
     */
    auto getFrontalPosition() const -> position;

    /**
     * @brief Checks if a long-term action is currently running
     * @return True if action active (overridden by Player)
     */
    virtual auto actionRunning() const -> bool { return false; }

    /**
     * @brief Gets active magic school type
     * @return Magic type (MAGE, PRIEST, BARD, DRUID)
     */
    inline auto getMagicType() const -> unsigned short { return magic.type; }

    /**
     * @brief Sets active magic school
     * @param newMagType New magic type
     */
    inline virtual void setMagicType(magic_type newMagType) { magic.type = newMagType; }

    /**
     * @brief Sets whether character is following a route
     * @param onr True if on route
     */
    void setOnRoute(bool onr) { _is_on_route = onr; }

    /**
     * @brief Checks if character is following a route
     * @return True if on route
     */
    auto getOnRoute() const -> bool { return _is_on_route; }

    /**
     * @brief Gets player's preferred language (overridden by Player)
     * @return Language enum (default: English for NPCs/Monsters)
     */
    virtual auto getPlayerLanguage() const -> Language { return Language::english; }

    /**
     * @brief Gets magic flags for a specific magic school
     * @param type Magic school (0-3: MAGE, PRIEST, BARD, DRUID)
     * @return Bitflags representing learned spells, or 0 if invalid type
     */
    inline auto getMagicFlags(unsigned char type) const -> unsigned long int {
        if (type < 4) {
            return magic.flags.at(type);
        }
        return 0;
    }

    /**
     * @brief Gets character type (pure virtual)
     * @return Character type identifier
     */
    virtual auto getType() const -> unsigned short = 0;

    /**
     * @brief Changes character's race and updates appearance
     * @param race New race ID
     */
    virtual void changeRace(TYPE_OF_RACE_ID race) {
        this->race = race;
        updateAppearanceForAll(true);
    }

    /**
     * @brief Gets character's race
     * @return Race ID
     */
    virtual auto getRace() const -> TYPE_OF_RACE_ID { return race; }

    /**
     * @brief Gets direction character is facing
     * @return Face direction enum value
     */
    virtual auto getFaceTo() const -> face_to { return faceto; }

    /**
     * @brief Checks if character has admin privileges
     * @return True if admin (overridden by Player)
     */
    virtual auto isAdmin() const -> bool { return false; }

    /**
     * @brief Gets monster type ID
     * @return Monster type (0 for non-monsters, overridden by Monster)
     */
    virtual auto getMonsterType() const -> TYPE_OF_CHARACTER_ID { return 0; }

    /**
     * @brief Changes quality of item at specific position
     * @param pos Item position index
     * @param amount Amount to change quality by (can be negative)
     */
    virtual void changeQualityAt(unsigned char pos, int amount);

    /**
     * @brief Increases poison value on character
     * @param value Amount to increase poison by
     */
    virtual void increasePoisonValue(int value);

    /**
     * @brief Gets current poison level
     * @return Poison value
     */
    virtual auto getPoisonValue() const -> int { return poisonvalue; }

    /**
     * @brief Sets poison level
     * @param value New poison value
     */
    virtual void setPoisonValue(int value) { poisonvalue = value; }

    /**
     * @brief Starts a long-time action for this character
     * 
     * Overloaded in Player to display action UI.
     * 
     * @param wait Time to wait until action succeeds (in 1/10 seconds)
     * @param ani Animation ID to display (0 = none)
     * @param redoani How often to repeat animation in 1/10s (0 = once)
     * @param sound Sound ID to play (0 = none)
     * @param redosound How often to repeat sound in 1/10s (0 = once)
     * @note Lua binding: [:startAction]
     */
    inline virtual void startAction(unsigned short int wait, unsigned short int ani = 0, unsigned short int redoani = 0,
                                    unsigned short int sound = 0, unsigned short int redosound = 0) {}

    /**
     * @brief Aborts current long-time action
     */
    inline virtual void abortAction() {}

    /**
     * @brief Marks current action as successful
     */
    inline virtual void successAction() {}

    /**
     * @brief Called when action is disturbed by another character
     * @param disturber Character that caused disturbance
     */
    inline virtual void actionDisturbed(Character *disturber) {}

    /**
     * @brief Changes source context to another character
     * @param cc Source character
     */
    inline virtual void changeSource(Character *cc) {}

    /**
     * @brief Changes source context to a script item
     * @param sI Source script item
     */
    inline virtual void changeSource(const ScriptItem &sI) {}

    /**
     * @brief Changes source context to a position
     * @param pos Source position
     */
    inline virtual void changeSource(const position &pos) {}

    /**
     * @brief Changes source context to speech
     * @param text Speech text
     * @param talkType Type of speech
     */
    inline virtual void changeSource(const std::string &text, talk_type talkType) {}

    /**
     * @brief Clears source context
     */
    inline virtual void changeSource() {}

    /**
     * @brief Gets current mental capacity
     * @return Mental capacity value
     */
    virtual auto getMentalCapacity() const -> int { return mental_capacity; }

    /**
     * @brief Sets mental capacity
     * @param value New mental capacity
     */
    virtual void setMentalCapacity(int value) { mental_capacity = value; }

    /**
     * @brief Increases mental capacity by amount
     * @param value Amount to increase
     */
    virtual void increaseMentalCapacity(int value);

    // Inventory management
    /**
     * @brief Counts total items of specific type
     * @param itemid Item ID to count
     * @return Total count across all inventory slots and backpack
     */
    virtual auto countItem(TYPE_OF_ITEM_ID itemid) const -> int;

    /**
     * @brief Counts items in specific inventory location
     * @param where Location ("all", "belt", "body", "backpack")
     * @param itemid Item ID to count
     * @param data Optional script data to match (nullptr = any)
     * @return Count in specified location
     */
    auto countItemAt(const std::string &where, TYPE_OF_ITEM_ID itemid,
                     script_data_exchangemap const *data = nullptr) const -> int;

    /**
     * @brief Removes items from inventory
     * @param itemid Item ID to remove
     * @param count Number to remove
     * @param data Optional script data to match
     * @return Number that couldn't be removed (0 = all removed)
     */
    virtual auto eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr) -> int;

    /**
     * @brief Creates new items in inventory
     * @param id Item ID
     * @param number Quantity to create
     * @param quality Item quality
     * @param data Script data for items
     * @return Number that couldn't be created (0 = all created)
     */
    virtual auto createItem(Item::id_type id, Item::number_type number, Item::quality_type quality,
                            script_data_exchangemap const *data) -> int;

    /**
     * @brief Increases stack count at specific position
     * @param pos Item position
     * @param count Amount to increase
     * @return Amount that couldn't be added
     */
    virtual auto increaseAtPos(unsigned char pos, int count) -> int;

    /**
     * @brief Creates items at specific position
     * @param pos Item position
     * @param newid Item ID
     * @param count Quantity
     * @return Amount that couldn't be created
     */
    virtual auto createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) -> int;

    /**
     * @brief Swaps item at position with new item
     * @param pos Item position
     * @param newid New item ID
     * @param newQuality Quality for new item
     * @return True if swap succeeded
     */
    virtual auto swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int newQuality = 0) -> bool;

    /**
     * @brief Gets item at specific position as ScriptItem
     * @param itempos Item position index
     * @return ScriptItem wrapper for the item
     */
    virtual auto GetItemAt(unsigned char itempos) -> ScriptItem;

    /**
     * @brief Gets backpack container
     * @return Pointer to backpack container (nullptr if none)
     */
    virtual auto GetBackPack() const -> Container *;

    /**
     * @brief Gets depot container by ID
     * @param depotid Depot ID
     * @return Pointer to depot container
     */
    auto GetDepot(uint32_t depotid) const -> Container *;

    /**
     * @brief Gets list of all items of specific type
     * @param id Item ID to find
     * @return Vector of ScriptItem wrappers
     */
    auto getItemList(TYPE_OF_ITEM_ID id) -> std::vector<ScriptItem>;

    // Skill system
    /**
     * @brief Gets localized skill name
     * @param s Skill ID
     * @return Skill name string
     */
    virtual auto getSkillName(TYPE_OF_SKILL_ID s) const -> std::string;

    /**
     * @brief Gets major skill level
     * @param s Skill ID
     * @return Skill level (0-100)
     */
    virtual auto getSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int;

    /**
     * @brief Gets complete skill value structure
     * @param s Skill ID
     * @return Pointer to skillvalue (nullptr if not learned)
     */
    virtual auto getSkillValue(TYPE_OF_SKILL_ID s) const -> const skillvalue *;

    /**
     * @brief Gets minor skill points (experience)
     * @param s Skill ID
     * @return Minor skill points
     */
    virtual auto getMinorSkill(TYPE_OF_SKILL_ID s) const -> unsigned short int;

    // Appearance
    /**
     * @brief Sets skin color
     * @param c Color value
     */
    void setSkinColour(const Colour &c);

    /**
     * @brief Gets skin color
     * @return Skin color
     */
    auto getSkinColour() const -> Colour;

    /**
     * @brief Sets hair color
     * @param c Color value
     */
    void setHairColour(const Colour &c);

    /**
     * @brief Gets hair color
     * @return Hair color
     */
    auto getHairColour() const -> Colour;

    /**
     * @brief Sets hair style
     * @param hairID Hair style ID
     */
    void setHair(uint8_t hairID);

    /**
     * @brief Gets hair style
     * @return Hair style ID
     */
    auto getHair() const -> uint8_t;

    /**
     * @brief Sets beard style
     * @param beardID Beard style ID
     */
    void setBeard(uint8_t beardID);

    /**
     * @brief Gets beard style
     * @return Beard style ID
     */
    auto getBeard() const -> uint8_t;

    /**
     * @brief Gets complete appearance structure
     * @return Appearance data
     */
    auto getAppearance() const -> appearance { return _appearance; }

    // Attribute system
    /**
     * @brief Sets base attribute value with validation
     * @param attribute Attribute to set
     * @param value New base value
     * @return True if value was valid and set
     */
    auto setBaseAttribute(Character::attributeIndex attribute, Attribute::attribute_t value) -> bool;

    /**
     * @brief Sets attribute value directly (includes offsets)
     * @param attribute Attribute to set
     * @param value New value
     */
    void setAttribute(Character::attributeIndex attribute, Attribute::attribute_t value);

    /**
     * @brief Gets base attribute value (without offsets)
     * @param attribute Attribute to get
     * @return Base attribute value
     */
    auto getBaseAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t;

    /**
     * @brief Gets total attribute value (base + offsets)
     * @param attribute Attribute to get
     * @return Total attribute value
     */
    auto getAttribute(Character::attributeIndex attribute) const -> Attribute::attribute_t;

    /**
     * @brief Increases base attribute with validation
     * @param attribute Attribute to increase
     * @param amount Amount to increase by
     * @return True if increase was valid
     */
    auto increaseBaseAttribute(Character::attributeIndex attribute, int amount) -> bool;

    /**
     * @brief Increases attribute value (modifies offsets)
     * @param attribute Attribute to increase
     * @param amount Amount to increase
     * @return New total value
     */
    auto increaseAttribute(Character::attributeIndex attribute, int amount) -> Attribute::attribute_t;

    /**
     * @brief Checks if base attribute value is valid for race
     * @param attribute Attribute to check
     * @param value Value to validate
     * @return True if value is within race limits
     */
    auto isBaseAttributeValid(Character::attributeIndex attribute, Attribute::attribute_t value) const -> bool;

    /**
     * @brief Gets sum of all base attributes
     * @return Total base attribute points
     */
    auto getBaseAttributeSum() const -> uint16_t;

    /**
     * @brief Gets maximum attribute points allowed for race
     * @return Max attribute points
     */
    auto getMaxAttributePoints() const -> uint16_t;

    /**
     * @brief Saves base attributes to database (overridden by Player)
     * @return True if saved successfully
     */
    virtual auto saveBaseAttributes() -> bool;

    /**
     * @brief Handles side effects of attribute changes
     * @param attribute Attribute that changed
     * @note Called automatically when attributes change
     */
    virtual void handleAttributeChange(Character::attributeIndex attribute);

    // String-based attribute access (for Lua)
    /**
     * @brief Validates base attribute by name
     * @param name Attribute name
     * @param value Value to validate
     * @return True if valid
     */
    auto isBaseAttribValid(const std::string &name, Attribute::attribute_t value) const -> bool;

    /**
     * @brief Sets base attribute by name
     * @param name Attribute name
     * @param value New value
     * @return True if successful
     */
    auto setBaseAttrib(const std::string &name, Attribute::attribute_t value) -> bool;

    /**
     * @brief Sets attribute by name
     * @param name Attribute name
     * @param value New value
     */
    void setAttrib(const std::string &name, Attribute::attribute_t value);

    /**
     * @brief Gets base attribute by name
     * @param name Attribute name
     * @return Base value (0 if invalid name)
     */
    auto getBaseAttrib(const std::string &name) const -> Attribute::attribute_t;

    /**
     * @brief Increases base attribute by name
     * @param name Attribute name
     * @param amount Amount to increase
     * @return True if successful
     */
    auto increaseBaseAttrib(const std::string &name, int amount) -> bool;

    /**
     * @brief Increases attribute by name
     * @param name Attribute name
     * @param amount Amount to increase
     * @return New value (0 if invalid name)
     */
    auto increaseAttrib(const std::string &name, int amount) -> Attribute::attribute_t;

    // Skill training
    /**
     * @brief Increases major skill level
     * @param skill Skill ID
     * @param amount Amount to increase
     * @return New skill level
     */
    virtual auto increaseSkill(TYPE_OF_SKILL_ID skill, int amount) -> int;

    /**
     * @brief Increases minor skill points
     * @param skill Skill ID
     * @param amount Amount to increase
     * @return New minor skill value
     */
    virtual auto increaseMinorSkill(TYPE_OF_SKILL_ID skill, int amount) -> int;

    /**
     * @brief Sets skill levels explicitly
     * @param skill Skill ID
     * @param major Major skill level
     * @param minor Minor skill points
     * @return New major skill level
     */
    virtual auto setSkill(TYPE_OF_SKILL_ID skill, int major, int minor) -> int;

    /**
     * @brief Removes all learned skills
     */
    virtual void deleteAllSkills();

    /**
     * @brief Processes skill learning from action
     * @param skill Skill being trained
     * @param actionPoints AP cost of action
     * @param opponent Difficulty rating (0-255)
     */
    virtual void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent);

    /**
     * @brief Teaches magic spell to character
     * @param type Magic school (0-3)
     * @param flag Spell bit flag
     */
    virtual void teachMagic(unsigned char type, unsigned char flag);

    // Distance and range checks
    /**
     * @brief Checks if another character is in range
     * @param cc Target character
     * @param distancemetric Maximum distance
     * @return True if within range
     */
    auto isInRange(Character *cc, Coordinate distancemetric) const -> bool;

    /**
     * @brief Checks if position is within screen range
     * @param pos Position to check
     * @return True if visible on character's screen
     */
    auto isInScreen(const position &pos) const -> bool;

    /**
     * @brief Gets screen/view range distance
     * @return View distance in tiles
     */
    virtual auto getScreenRange() const -> Coordinate;

    /**
     * @brief Calculates distance to another character
     * @param cc Target character
     * @return Distance value
     */
    auto distanceMetric(Character *cc) const -> Coordinate;

    /**
     * @brief Checks if field position is in range
     * @param m_pos Field position
     * @param distancemetric Maximum distance
     * @return True if within range
     */
    auto isInRangeToField(const position &m_pos, Coordinate distancemetric) const -> bool;

    /**
     * @brief Calculates distance to position
     * @param m_pos Target position
     * @return Distance value
     */
    auto distanceMetricToPosition(const position &m_pos) const -> Coordinate;

    // Communication
    /**
     * @brief Makes character speak
     * @param tt Talk type (say, whisper, yell)
     * @param message Message to speak
     */
    virtual void talk(talk_type tt, const std::string &message);

    /**
     * @brief Makes character speak with language-specific text
     * @param tt Talk type
     * @param german German text
     * @param english English text
     */
    virtual void talk(talk_type tt, const std::string &german, const std::string &english);

    /**
     * @brief Sends informational message to character
     * @param message Message text
     * @param type Message priority/type
     */
    virtual void inform(const std::string &message, informType type = informServer) const;

    /**
     * @brief Sends language-specific informational message
     * @param german German text
     * @param english English text
     * @param type Message priority/type
     */
    virtual void inform(const std::string &german, const std::string &english, informType type = informServer) const;

    // Movement and positioning
    /**
     * @brief Moves character in direction
     * @param dir Direction to move
     * @param active True if active player movement
     * @return True if move succeeded
     */
    virtual auto move(direction dir, bool active = true) -> bool;

    /**
     * @brief Gets next step direction toward goal
     * @param goal Target position
     * @param dir Output: direction to move
     * @return True if path found
     */
    virtual auto getNextStepDir(const position &goal, direction &dir) const -> bool;

    /**
     * @brief Calculates full path to goal
     * @param goal Target position
     * @param steps Output: list of directions
     * @return True if path found
     */
    auto getStepList(const position &goal, std::list<direction> &steps) const -> bool;

    /**
     * @brief Warps character to new position (checks validity)
     * @param newPos Target position
     * @return True if warp succeeded
     */
    virtual auto Warp(const position &newPos) -> bool;

    /**
     * @brief Forces warp regardless of validity
     * @param newPos Target position
     * @return True if warp succeeded
     */
    virtual auto forceWarp(const position &newPos) -> bool;

    // Audio
    /**
     * @brief Starts music for character (Players only)
     * @param title Music track ID
     */
    virtual void startMusic(short int title);

    /**
     * @brief Resets to default ambient music (Players only)
     */
    virtual void defaultMusic();

    // Quest system
    /**
     * @brief Sets quest progress status
     * @param questid Quest ID
     * @param progress New progress value
     */
    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress);

    /**
     * @brief Gets quest progress status
     * @param questid Quest ID
     * @param time Output: time since quest update
     * @return Current quest status
     */
    virtual auto getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const -> TYPE_OF_QUESTSTATUS;

    // Client-specific features (Players only)
    /**
     * @brief Sends character description text
     * @param id Character ID being described
     * @param desc Description text
     */
    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc);

    /**
     * @brief Checks if player is new (Players only)
     * @return True if new player
     */
    virtual auto isNewPlayer() const -> bool;

    /**
     * @brief Sends GM page request (Players only)
     * @param ticket Ticket message
     * @return True if page sent
     */
    virtual auto pageGM(const std::string &ticket) -> bool;

    /**
     * @brief Gets idle time in seconds (Players only)
     * @return Seconds idle
     */
    virtual auto idleTime() const -> uint32_t;

    /**
     * @brief Sends book content to player (Players only)
     * @param bookID Book ID to display
     */
    virtual void sendBook(uint16_t bookID);

    // Appearance updates
    /**
     * @brief Updates character appearance for all nearby players
     * @param always True to force update even if unchanged
     */
    void updateAppearanceForAll(bool always);

    /**
     * @brief Forces appearance update for all players
     */
    virtual void forceUpdateAppearanceForAll();

    /**
     * @brief Updates appearance for specific player
     * @param target Target player
     * @param always True to force update
     */
    void updateAppearanceForPlayer(Player *target, bool always);

    /**
     * @brief Performs animation
     * @param animID Animation ID
     */
    virtual void performAnimation(uint8_t animID);

    /**
     * @brief Skill ID to skill value mapping
     */
    using SKILLMAP = std::map<TYPE_OF_SKILL_ID, skillvalue>;

    /**
     * @brief Gets character movement type
     * @return Movement type (walk, run, crawl, etc.)
     */
    auto GetMovement() const -> movement_type;

    /**
     * @brief Sets character movement type
     * @param tmovement New movement type
     */
    void SetMovement(movement_type tmovement);

    /**
     * @brief Recalculates and adds inventory weight
     */
    void AddWeight();

    /**
     * @brief Recalculates and subtracts inventory weight
     */
    void SubWeight();

    /**
     * @brief Sets whether clipping is active (Players only)
     * @param tclippingActive True to enable clipping
     */
    inline virtual void setClippingActive(bool tclippingActive) {}

    /**
     * @brief Gets clipping state (Players only)
     * @return True if clipping enabled
     */
    inline virtual auto getClippingActive() const -> bool { return true; }

    SKILLMAP skills; ///< Map of learned skills

    /**
     * @brief Character inventory array
     * 
     * Layout:
     * - Index 0: backpack
     * - Indices 1 to MAX_BODY_ITEMS-1: equipped items (armor, weapons, etc.)
     * - Indices MAX_BODY_ITEMS to MAX_BODY_ITEMS+MAX_BELT_SLOTS-1: belt items
     */
    std::array<Item, MAX_BODY_ITEMS + MAX_BELT_SLOTS> items = {};

    Container *backPackContents{nullptr}; ///< Container holding backpack items

    /**
     * @brief Map of depot containers
     * 
     * Key: depot ID
     * Value: pointer to container storing depot items
     */
    std::map<uint32_t, Container *> depotContents;

    /**
     * @brief Ages all inventory items (durability decay)
     */
    virtual void ageInventory();

    /**
     * @brief Checks if character is alive
     * @return True if alive (hitpoints > 0)
     */
    inline auto isAlive() const -> bool { return alive; }

    /**
     * @brief Sets alive state
     * @param t True if alive
     */
    virtual void setAlive(bool t);

    // Combat system
    character_type enemytype = player; ///< Type of current enemy
    TYPE_OF_CHARACTER_ID enemyid = 0;  ///< ID of current enemy

    /**
     * @brief Initiates attack on target
     * @param target Character to attack
     * @return True if attack started
     */
    virtual auto attack(Character *target) -> bool;

    /**
     * @brief Stops current attack
     */
    virtual void stopAttack();

    /**
     * @brief Gets current attack target
     * @return Smart pointer to target (nullptr if none)
     */
    auto getAttackTarget() const -> character_ptr;

    // Weight and carrying capacity
    /**
     * @brief Calculates maximum carrying weight
     * @return Max weight in unit (based on strength)
     */
    auto maxLoadWeight() const -> unsigned short int;

    /**
     * @brief Calculates current inventory weight
     * @return Current weight
     */
    auto LoadWeight() const -> int;

    /**
     * @brief Calculates relative load (current/max)
     * @return Load ratio (0.0 to 1.0+)
     */
    auto relativeLoad() const -> double;

    /**
     * @brief Load burden level affecting movement
     */
    enum class LoadLevel { unburdened, burdened, overtaxed };

    /**
     * @brief Gets current load burden level
     * @return Load level enum
     */
    auto loadFactor() const -> LoadLevel;

    /**
     * @brief Calculates weight of a container
     * @param id Item ID of container
     * @param count Stack count (positive or negative)
     * @param tcont Pointer to container data
     * @return Total weight (positive or negative)
     */
    static auto weightContainer(unsigned short int id, int count, Container *tcont) -> int;

    /**
     * @brief Checks if adding items won't exceed weight limit
     * @param id Item ID to add
     * @param count Number to add
     * @param tcont Container to add to
     * @return True if weight would be acceptable
     */
    auto weightOK(TYPE_OF_ITEM_ID id, int count, Container *tcont) const -> bool;

    /**
     * @brief Turns character to face direction
     * @param dir Direction to face
     */
    virtual void turn(direction dir);

    /**
     * @brief Turns character to face position
     * @param posi Position to face toward
     */
    virtual void turn(const position &posi);

    /**
     * @brief Receives text from another character
     * @param tt Talk type
     * @param message Message text
     * @param cc Speaking character
     */
    virtual void receiveText(talk_type tt, const std::string &message, Character *cc);

    /**
     * @brief Introduces a player to this character (overridden by Player)
     * @param player Player being introduced
     */
    virtual void introducePlayer(Player *player);

    /**
     * @brief Calls attack script handlers
     * @param Attacker Attacking character
     * @param Defender Defending character
     */
    void callAttackScript(Character *Attacker, Character *Defender);

    // Dialog system
    /**
     * @brief Requests text input from player
     * @param inputDialog Input dialog to display
     */
    virtual void requestInputDialog(InputDialog *inputDialog);

    /**
     * @brief Requests message display
     * @param messageDialog Message dialog to display
     */
    virtual void requestMessageDialog(MessageDialog *messageDialog);

    /**
     * @brief Requests merchant trade dialog
     * @param merchantDialog Merchant dialog to display
     */
    virtual void requestMerchantDialog(MerchantDialog *merchantDialog);

    /**
     * @brief Requests selection dialog
     * @param selectionDialog Selection dialog to display
     */
    virtual void requestSelectionDialog(SelectionDialog *selectionDialog);

    /**
     * @brief Requests crafting dialog
     * @param craftingDialog Crafting dialog to display
     */
    virtual void requestCraftingDialog(CraftingDialog *craftingDialog);

    /**
     * @brief Requests look-at for crafting result
     * @param dialogId Dialog ID
     * @param lookAt Look-at data to populate
     */
    virtual void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt);

    /**
     * @brief Requests look-at for crafting ingredient
     * @param dialogId Dialog ID
     * @param lookAt Look-at data to populate
     */
    virtual void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt);

    /**
     * @brief Logs admin action (Players only)
     * @param message Log message
     */
    virtual void logAdmin(const std::string &message);

    /**
     * @brief Gets monster loot table (Monsters only)
     * @return Reference to loot structure
     * @throws NoLootFound if no loot available
     */
    virtual auto getLoot() const -> const MonsterStruct::loottype &;

protected:
    /**
     * @brief Race definition structure with attribute limits
     * 
     * Contains min/max values for all attributes based on race.
     */
    struct RaceStruct {
        std::string racename;       ///< Race name
        unsigned short int points;  ///< Starting attribute points
        unsigned short int minage;  ///< Minimum age
        unsigned short int maxage;  ///< Maximum age
        unsigned short int minweight;  ///< Minimum weight
        unsigned short int maxweight;  ///< Maximum weight
        unsigned char minbodyheight;   ///< Minimum body height
        unsigned char maxbodyheight;   ///< Maximum body height
        unsigned char minagility;      ///< Minimum agility
        unsigned char maxagility;      ///< Maximum agility
        unsigned char minconstitution; ///< Minimum constitution
        unsigned char maxconstitution; ///< Maximum constitution
        unsigned char mindexterity;    ///< Minimum dexterity
        unsigned char maxdexterity;    ///< Maximum dexterity
        unsigned char minessence;      ///< Minimum essence
        unsigned char maxessence;      ///< Maximum essence
        unsigned char minintelligence; ///< Minimum intelligence
        unsigned char maxintelligence; ///< Maximum intelligence
        unsigned char minperception;   ///< Minimum perception
        unsigned char maxperception;   ///< Maximum perception
        unsigned char minstrength;     ///< Minimum strength
        unsigned char maxstrength;     ///< Maximum strength
        unsigned char minwillpower;    ///< Minimum willpower
        unsigned char maxwillpower;    ///< Maximum willpower
    };

    /**
     * @brief Sets character ID
     * @param id New character ID
     */
    void setId(TYPE_OF_CHARACTER_ID id);

    /**
     * @brief Sets character name
     * @param name New name
     */
    void setName(const std::string &name);

    /**
     * @brief Sets position directly
     * @param pos New position
     */
    void setPosition(const position &pos);

    /**
     * @brief Sets race directly
     * @param race New race ID
     */
    void setRace(TYPE_OF_RACE_ID race);

    /**
     * @brief Sets facing direction directly
     * @param faceTo New direction
     */
    void setFaceTo(face_to faceTo);

    /**
     * @brief Sets magic flags for school
     * @param type Magic school
     * @param flags New spell bitflags
     */
    void setMagicFlags(magic_type type, uint64_t flags);

    bool _is_on_route = false;      ///< True if following automatic route
    int poisonvalue = 0;            ///< Current poison damage level
    int mental_capacity = 0;        ///< Mental capacity for effects
    World *_world;                  ///< Pointer to game world

    /**
     * @brief Checks if character can move to field
     * @param field Target field
     * @return True if movement allowed
     */
    virtual auto moveToPossible(const map::Field &field) const -> bool;

    /**
     * @brief Calculates time cost of movement
     * @param targetField Destination field
     * @param diagonalMove True if diagonal movement
     * @param running True if running
     * @return Move time in milliseconds
     */
    virtual auto getMoveTime(const map::Field &targetField, bool diagonalMove, bool running) const
            -> TYPE_OF_WALKINGCOST;

    /**
     * @brief Checks if character can speak (has enough AP)
     * @param tt Talk type
     * @return True if can afford speech
     */
    auto canTalk(talk_type tt) const -> bool;

    /**
     * @brief Gets AP cost of speech type
     * @param tt Talk type
     * @return Action point cost
     */
    static /*consteval*/ auto talkCost(talk_type tt) -> int;

    /**
     * @brief Logs speech to server log
     * @param tt Talk type
     * @param message Message text
     */
    void logTalk(talk_type tt, const std::string &message) const;

    /**
     * @brief Processes speech through scripts
     * @param tt Talk type
     * @param message Original message
     * @return Processed message
     */
    auto talkScript(talk_type tt, const std::string &message) -> std::string;

    appearance _appearance; ///< Character visual appearance

private:
    TYPE_OF_CHARACTER_ID id = 0;       ///< Unique character identifier
    std::string name;                  ///< Character name
    movement_type _movement = movement_type::walk; ///< Current movement mode
    std::vector<Attribute> attributes; ///< All character attributes
    bool alive = true;                 ///< Alive state (hitpoints > 0)
    int actionPoints = NP_MAX_AP;      ///< Current action points
    int fightPoints = NP_MAX_FP;       ///< Current fight points
    short int activeLanguage = 0;      ///< Active speech language
    position pos = {0, 0, 0};          ///< Current world position
    bool attackmode = false;           ///< True if in combat mode
    std::string lastSpokenText = {};   ///< Last message spoken
    bool isinvisible = false;          ///< Invisibility state
    TYPE_OF_RACE_ID race = 0;          ///< Character race ID
    face_to faceto = north;            ///< Current facing direction
    s_magic magic{};                   ///< Magic school and spell data
    double speed = 1.0;                ///< Movement speed multiplier
};

/**
 * @brief Stream output operator for Character
 * @param os Output stream
 * @param character Character to output
 * @return Modified stream
 */
auto operator<<(std::ostream &os, const Character &character) -> std::ostream &;

#endif
