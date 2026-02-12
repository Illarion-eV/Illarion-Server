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

#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "Character.hpp"
#include "data/MonsterTable.hpp"

class SpawnPoint;

/**
 * @brief Represents a computer-controlled hostile or neutral creature in the game world.
 * 
 * Monsters are AI-controlled characters that spawn from SpawnPoints and can engage in combat.
 * They have randomized attributes based on their monster type definition and can:
 * - Attack players and other characters
 * - Drop loot when killed
 * - Execute custom Lua scripts for behavior (attack, death, speech reception)
 * - Navigate toward targets using A* pathfinding
 * - Heal themselves periodically
 * - Remember last seen target positions for pursuit
 * 
 * Each monster is assigned a unique ID from the MONSTER_BASE range and maintains a reference
 * to its SpawnPoint for respawn management.
 * 
 * @ingroup Scriptclasses
 * @see Character
 * @see SpawnPoint
 * @see MonsterTable
 * @note Monsters are non-copyable and non-movable
 */
class Monster : public Character {
public:
    /**
     * @brief Exception thrown when attempting to set an invalid monster type ID.
     * 
     * @see setMonsterType()
     */
    class unknownIDException {};

    /**
     * @brief Creates a new monster at the specified position.
     * 
     * Initializes the monster with randomized attributes based on its type definition from
     * MonsterTable. The monster is assigned a unique ID from the MONSTER_BASE range.
     * 
     * @param type The monster type ID to create (must exist in MonsterTable)
     * @param newpos The world position where the monster spawns
     * @param spawnpoint Optional pointer to the SpawnPoint that created this monster (nullptr if spawned manually)
     * 
     * @throws unknownIDException if the monster type does not exist in MonsterTable
     * @see setMonsterType()
     */
    Monster(const TYPE_OF_CHARACTER_ID &type, const position &newpos, SpawnPoint *spawnpoint = nullptr);

    /**
     * @brief Retrieves the loot table for this monster type.
     * 
     * @return Reference to the monster's loot configuration
     * @throws NoLootFound if the monster type has no loot defined
     */
    auto getLoot() const -> const MonsterStruct::loottype & override;

    /**
     * @brief Returns the character type identifier.
     * 
     * @return Always returns 'monster' constant
     */
    auto getType() const -> unsigned short override { return monster; }

    /**
     * @brief Changes the monster's type and resets all attributes accordingly.
     * 
     * Completely reinitializes the monster with new randomized attributes, skills, items,
     * appearance, and behavior from the new monster type definition. Clears existing skills
     * before applying new ones.
     * 
     * @param type The new monster type ID
     * @throws unknownIDException if the type does not exist in MonsterTable
     */
    void setMonsterType(TYPE_OF_CHARACTER_ID type);

    /**
     * @brief Associates this monster with a different spawn point.
     * 
     * @param sp Pointer to the new SpawnPoint (can be nullptr to detach from spawn point)
     */
    void setSpawn(SpawnPoint *sp);

    /**
     * @brief Sets the alive/dead state and triggers death events.
     * 
     * When a monster dies (t = false), this calls the monster's onDeath Lua script
     * if one exists. The associated SpawnPoint is notified through the destructor.
     * 
     * @param t The new life state (true = alive, false = dead)
     * @note Overrides Character::setAlive() to add script callbacks
     */
    void setAlive(bool t) override;

    /**
     * @brief Removes the monster from the game world without triggering spawn point notifications.
     * 
     * This directly sets alive to false using Character::setAlive(), bypassing the death
     * script callbacks and spawn point cleanup that would occur with Monster::setAlive().
     */
    void remove();

    /**
     * @brief Returns a debug string representation of the monster.
     * 
     * @return String in format "Monster of race X(ID)"
     */
    auto to_string() const -> std::string override;

    /**
     * @brief Processes speech received from nearby characters.
     * 
     * If the monster has a Lua script with a receiveText entry point, this triggers
     * the script callback. Monsters cannot receive text from themselves.
     * 
     * @param tt The speech volume/type (whisper, say, shout, etc.)
     * @param message The spoken text
     * @param cc The character who spoke (nullptr if system message)
     */
    void receiveText(talk_type tt, const std::string &message, Character *cc) override;

    /**
     * @brief Retrieves the SpawnPoint that created this monster.
     * 
     * @return Pointer to the spawn point, or nullptr if spawned manually
     */
    inline auto getSpawn() const -> SpawnPoint * { return spawn; }

    /**
     * @brief Returns the monster's type identifier.
     * 
     * @return The monster type ID used to look up definition in MonsterTable
     */
    auto getMonsterType() const -> TYPE_OF_CHARACTER_ID override { return monstertype; }

    /**
     * @brief Initiates an attack on a target character.
     * 
     * Calls the monster's onAttack Lua script before delegating to Character::attack().
     * 
     * @param target The character to attack
     * @return Result of the attack action (currently unused)
     */
    auto attack(Character *target) -> bool override;

    /**
     * @brief Restores hitpoints and mana by a fixed amount.
     * 
     * Increases both hitpoints and mana by monsterSelfHealAmount (defined in tuningConstants.hpp).
     */
    void heal();

    /**
     * @brief Moves the monster one step toward a target position using pathfinding.
     * 
     * Uses A* pathfinding to calculate a route to targetpos. If the current waypoint doesn't
     * match the target or no valid path exists, recalculates the route. Falls back to random
     * movement if pathfinding fails.
     * 
     * @param targetpos The destination position to move toward
     */
    void performStep(position targetpos);

    /**
     * @brief Destructor that notifies the spawn point of monster death.
     * 
     * If this monster was created by a SpawnPoint, informs it that this monster type
     * has died so it can schedule a respawn.
     */
    ~Monster() override;
    Monster(const Monster &) = delete;
    auto operator=(const Monster &) -> Monster & = delete;
    Monster(Monster &&) = delete;
    auto operator=(Monster &&) -> Monster & = delete;

    position lastTargetPosition{}; ///< Last known position of the most recent enemy target
    bool lastTargetSeen = false; ///< If true, monster will pursue lastTargetPosition when no enemies are visible

    /**
     * @brief Checks if this monster can initiate attacks.
     * 
     * @return true if the monster is aggressive and can attack, false for peaceful monsters
     */
    inline auto canAttack() const -> bool { return _canAttack; }

    std::string nameDe; ///< German localized name of the monster

protected:
    /**
     * @brief Default constructor for subclass use only.
     */
    Monster() = default;

private:
    static uint32_t counter; ///< Global counter for generating unique monster IDs
    SpawnPoint *spawn = nullptr; ///< The spawn point that created this monster (nullptr if spawned manually)
    TYPE_OF_CHARACTER_ID monstertype = 0; ///< Monster type ID for looking up definition in MonsterTable
    bool _canAttack = true; ///< Whether this monster can initiate combat
};

#endif
