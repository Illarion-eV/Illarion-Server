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

#ifndef NPC_HPP
#define NPC_HPP

#include "Character.hpp"

#include <memory>
#include <utility>

class LuaNPCScript;

/**
 * @brief Holds localized text for NPC dialogue.
 * 
 * Contains German and English versions of spoken text for bilingual NPC interactions.
 */
struct NPCTalk {
    std::string SpeechText_ger; ///< German version of the spoken text
    std::string SpeechText_eng; ///< English version of the spoken text
};

class World;

/**
 * @brief Represents a non-player character with scripted behavior.
 * 
 * NPCs are stationary or semi-stationary characters that interact with players through
 * Lua scripts. Unlike Monsters, NPCs typically:
 * - Have fixed spawn positions and don't respawn from SpawnPoints
 * - Use dialogue scripts for player interaction
 * - Can act as healers to resurrect dead players
 * - Maintain persistent IDs based on database records
 * - Are loaded from the NPCTable during world initialization
 * 
 * Each NPC is assigned a unique ID from either NPC_BASE (for persistent NPCs) or
 * DYNNPC_BASE (for dynamically spawned NPCs).
 * 
 * @see Character
 * @see LuaNPCScript
 * @see NPCTable
 * @note NPCs are movable but not copyable
 */
class NPC : public Character {
public:
    /**
     * @brief Creates a new NPC at a specific position.
     * 
     * Initializes the NPC with full hitpoints (MAXHPS) and spawns it at the specified
     * position. The NPC is immediately visible to all nearby players.
     * 
     * @param id Database ID for the NPC (use DYNNPC_BASE for dynamically generated NPCs)
     * @param name Display name of the NPC
     * @param race Race type ID determining appearance and racial traits
     * @param pos World position where the NPC spawns
     * @param dir Direction the NPC initially faces
     * @param ishealer If true, NPC can resurrect dead players
     * @param sex Biological sex of the NPC
     * @param appearance Visual appearance settings (hair, beard, colors)
     * 
     * @note Persistent NPCs get ID = NPC_BASE + id; dynamic NPCs get ID = DYNNPC_BASE + counter
     */
    NPC(TYPE_OF_CHARACTER_ID id, const std::string &name, TYPE_OF_RACE_ID race, const position &pos,
        Character::face_to dir, bool ishealer, Character::sex_type sex, const appearance &appearance);

    /**
     * @brief Default constructor for testing purposes.
     * 
     * Creates an uninitialized NPC. Should only be used in test scenarios.
     */
    NPC() = default;

    ~NPC() override;
    NPC(const NPC &) = delete;
    auto operator=(const NPC &) -> NPC & = delete;
    NPC(NPC &&) = default;
    auto operator=(NPC &&) -> NPC & = default;

    /**
     * @brief Returns the character type identifier.
     * 
     * @return Always returns 'npc' constant
     */
    auto getType() const -> unsigned short override { return npc; }

    /**
     * @brief Checks if this NPC can resurrect dead players.
     * 
     * @return true if the NPC is a healer, false otherwise
     */
    auto getHealer() const -> bool { return _ishealer; }

    /**
     * @brief Retrieves the NPC's original spawn position.
     * 
     * @return The position where the NPC was created
     */
    auto getStartPos() const -> position { return _startpos; }

    /**
     * @brief Retrieves the Lua script controlling this NPC's behavior.
     * 
     * @return Shared pointer to the NPC's script, or nullptr if no script is assigned
     */
    auto getScript() const -> std::shared_ptr<LuaNPCScript> { return _script; }

    /**
     * @brief Assigns a Lua script to control this NPC's behavior.
     * 
     * @param script Shared pointer to a LuaNPCScript instance
     */
    void setScript(std::shared_ptr<LuaNPCScript> script) { _script = std::move(script); }

    /**
     * @brief Processes speech received from nearby characters.
     * 
     * If the NPC has a Lua script with a receiveText entry point, this triggers
     * the script callback for dialogue processing. NPCs cannot receive text from themselves.
     * 
     * @param tt The speech volume/type (whisper, say, shout, etc.)
     * @param message The spoken text
     * @param cc The character who spoke (nullptr if system message)
     */
    void receiveText(talk_type tt, const std::string &message, Character *cc) override;

    /**
     * @brief Returns a debug string representation of the NPC.
     * 
     * @return String in format "NPC Name(ID)"
     */
    auto to_string() const -> std::string override;

protected:
    static uint32_t counter; ///< Global counter for generating unique dynamic NPC IDs
    bool _ishealer{}; ///< If true, this NPC can resurrect dead players
    position _startpos{}; ///< Original spawn position of the NPC
    std::shared_ptr<LuaNPCScript> _script; ///< Lua script controlling NPC behavior and dialogue
};

#endif
