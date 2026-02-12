/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WORLD_HPP
#define WORLD_HPP

#define LOG_TALK

#include "Character.hpp"
#include "CharacterContainer.hpp"
#include "Language.hpp"
#include "MonitoringClients.hpp"
#include "NewClientView.hpp"
#include "Scheduler.hpp"
#include "SpawnPoint.hpp"
#include "TableStructs.hpp"
#include "Timer.hpp"
#include "WorldScriptInterface.hpp"
#include "character_ptr.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/MonsterTable.hpp"
#include "map/WorldMap.hpp"

#include <chrono>
#include <list>
#include <memory>
#include <unordered_map>

class Player;
class Monster;
class NPC;
class LuaScript;

/**
 * @brief Function signature for GM and player command handlers
 * 
 * Command handlers receive the world instance, the player executing the command,
 * and the command text string. They return true if the command was successfully
 * executed, false otherwise.
 */
using CommandType = std::function<bool(World *, Player *, const std::string &)>;

/**
 * @brief Environmental weather conditions for the game world
 * 
 * Defines all weather parameters that affect the visual atmosphere of the world.
 * These values are synchronized to all online players when changed.
 * 
 * @see World::setWeather()
 * @see World::sendWeatherToAllPlayers()
 */
struct WeatherStruct {
    char cloud_density = defaultCloudDensity;                   ///< Cloud density in percent (0-100)
    char fog_density = defaultFogDensity;                       ///< Fog density in percent (0-100)
    char wind_dir = defaultWindDirection;                       ///< Wind direction (-100 to 100, -100=from east, 0=north, 100=west)
    char gust_strength = defaultGustStrength;                   ///< Wind strength in percent (0-100)
    char percipitation_strength = defaultPercipitationStrength; ///< Precipitation strength in percent (0-100)
    char per_type = defaultPercipitationType;                   ///< Precipitation type: 0=rain, 1=snow
    char thunderstorm = defaultThunderstormIntensity;           ///< Thunderstorm intensity in percent (0-100)
    char temperature = defaultTemperature;                      ///< Current temperature in °C

    static constexpr char defaultCloudDensity = 20;
    static constexpr char defaultFogDensity = 0;
    static constexpr char defaultWindDirection = 50;
    static constexpr char defaultGustStrength = 10;
    static constexpr char defaultPercipitationStrength = 0;
    static constexpr char defaultPercipitationType = 0;
    static constexpr char defaultThunderstormIntensity = 0;
    static constexpr char defaultTemperature = 20;
};

/**
 * @brief Describes an object or character blocking a field or line of sight
 * 
 * Used to identify what is preventing movement or vision at a specific location.
 * This can be either a character, an item, or nothing at all.
 */
struct BlockingObject {
    enum BlockingType { BT_ITEM = 0, BT_CHARACTER = 1, BT_NONE = 2 };
    BlockingType blockingType = BT_NONE;   ///< Type of blocking object
    Character *blockingChar = nullptr;     ///< Pointer to blocking character (if BT_CHARACTER)
    ScriptItem blockingItem;               ///< Blocking item (if BT_ITEM)
};

/**
 * @brief Central singleton managing the game world and all entities within it
 * 
 * World is the core manager for the Illarion game server, responsible for:
 * - Managing all characters (players, monsters, NPCs) and their containers
 * - Maintaining and updating the world map system
 * - Processing game loop (turntheworld) with action points
 * - Handling character movement, item manipulation, and field interactions
 * - Managing spawn points and monster spawning
 * - Executing GM and player commands
 * - Broadcasting messages, weather, and time to clients
 * - Managing warp fields and teleportation
 * - Scheduling and executing timed scripts
 * - Handling combat, line-of-sight, and targeting systems
 * - Persisting world state to database
 * 
 * The World class follows a singleton pattern and must be created via World::create()
 * before accessing via World::get(). It processes the game in time slices based on
 * action points (AP), which are distributed to all entities for turn-based mechanics.
 * 
 * Implementation is split across multiple files:
 * - World.cpp: Core loop, initialization, and general functionality
 * - WorldIMPLCharacterMoves.cpp: Character movement and positioning
 * - WorldIMPLItemMoves.cpp: Item creation, manipulation, and transfers
 * - WorldIMPLTools.cpp: Utility functions and field/character lookups
 * - WorldIMPLTalk.cpp: Message broadcasting and communication
 * - WorldIMPLAdmin.cpp: GM commands and administrative functions
 * - WorldIMPLScriptHelp.cpp: Lua script interface implementations
 * - WorldIMPLPlayer.cpp: Player-specific functionality
 * 
 * @note This class is non-copyable and non-movable (singleton pattern)
 * @see WorldScriptInterface
 * @see Character
 * @see map::WorldMap
 */
class World : public WorldScriptInterface {
public:
    auto operator=(const World &) -> World & = delete;
    World(const World &) = delete;
    World(World &&) = delete;
    auto operator=(World &&) -> World & = delete;

    /////////////////////////
    NewClientView clientview; ///< View manager for client field-of-view calculations
    /////////////////////////

    /**
     * @brief Container type for all active players
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    using PLAYERVECTOR = CharacterContainer<Player>;

    /**
     * @brief Container type for all monsters
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    using MONSTERVECTOR = CharacterContainer<Monster>;

    /**
     * @brief Container type for all NPCs
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    using NPCVECTOR = CharacterContainer<NPC>;

    /**
     * @brief All currently logged-in players
     * 
     * Players are added when they successfully log in and removed when they disconnect
     * or time out. Access is thread-safe through the CharacterContainer interface.
     * 
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    PLAYERVECTOR Players;

    /**
     * @brief Changes the ground tile at a player's next step position
     * 
     * Used for tile drawing/painting by GMs ("turtle" mode). Updates the tile
     * where the player will move next.
     * 
     * @param cp Pointer to the player whose next tile should be changed
     * @param tilenumber ID of the tile to set
     */
    void setNextTile(Player *cp, unsigned char tilenumber);

    /**
     * @brief All active monsters in the world
     * 
     * Monsters are spawned from SpawnPoints and removed when killed. Each monster
     * maintains a reference to its spawn point for respawn management.
     * 
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    MONSTERVECTOR Monsters;

    /**
     * @brief Queue of monsters waiting to be spawned
     * 
     * Monsters scheduled for spawning are placed here and added to the world
     * in a deferred manner to prevent crashes when monsters create other monsters.
     */
    std::vector<Monster *> newMonsters;

    /**
     * @brief All active NPCs in the world
     * 
     * NPCs are loaded from the database at server start and persist until
     * explicitly deleted or the server shuts down.
     * 
     * @todo Change the three vectors (PLAYERVECTOR, MONSTERVECTOR, NPCVECTOR) to use a single unified container
     */
    NPCVECTOR Npc;

    /**
     * @brief Character IDs of NPCs pending deletion
     * 
     * NPCs marked for deletion are added here and removed in deleteAllLostNPC().
     * This is a workaround to prevent segfaults when NPCs are sold or removed.
     * 
     * @todo Replace with better character lifecycle management using smart pointers and unified container
     */
    std::vector<TYPE_OF_CHARACTER_ID> LostNpcs;

    /**
     * @brief Manages monitoring/admin client connections
     * 
     * Monitoring clients can connect to observe server state and receive diagnostic
     * messages without being actual players.
     */
    std::unique_ptr<MonitoringClients> monitoringClientList = nullptr;

    std::chrono::steady_clock::time_point startTime; ///< Server start timestamp for uptime calculations
    long usedAP{0}; ///< Total action points consumed since server start

    int ap{}; ///< Action points available for current game loop iteration

    ClockBasedScheduler<std::chrono::steady_clock> scheduler; ///< Manages scheduled script execution

    WeatherStruct weather; ///< Current weather conditions for the world

    /**
     * @brief Sets new weather conditions and broadcasts them to all players
     * 
     * @param nWeather New weather configuration to apply
     * @see WeatherStruct
     * @see sendWeatherToAllPlayers()
     */
    inline void setWeather(WeatherStruct nWeather) {
        weather = nWeather;
        sendWeatherToAllPlayers();
    }

    /**
     * @brief Parses and executes GM commands
     * 
     * GM commands are prefixed with '!' and provide administrative functions like
     * teleportation, spawning, reloading, etc. The user must have appropriate
     * GM rights to execute the command.
     * 
     * @param user Player attempting to execute the command
     * @param text Full command text including the '!' prefix
     * @return true if a valid command was found and executed, false otherwise
     * @see parsePlayerCommands()
     * @see gm_rights
     */
    auto parseGMCommands(Player *user, const std::string &text) -> bool;

    /**
     * @brief Parses and executes player commands
     * 
     * Player commands are prefixed with '!' and provide utility functions available
     * to all players, such as language switching or help information.
     * 
     * @param player Player executing the command
     * @param text Full command text including the '!' prefix
     * @return true if a valid command was found and executed, false otherwise
     * @see parseGMCommands()
     */
    auto parsePlayerCommands(Player *player, const std::string &text) -> bool;

    /**
     * @brief Currently executing Lua script
     * 
     * Set during script execution to track context for script callbacks.
     * nullptr when no script is running.
     */
    LuaScript *currentScript{nullptr};

    ~World() override = default;

    /**
     * @brief Executes one iteration of the main game loop
     * 
     * Called repeatedly by the server's main loop to advance game state. Calculates
     * elapsed time since server start and converts it to action points (AP) based on
     * MIN_AP_UPDATE constant. Only processes game state if at least 1 AP has accumulated.
     * 
     * Each call:
     * - Calculates elapsed milliseconds and converts to AP
     * - If AP > 0, calls checkPlayers(), checkMonsters(), and checkNPC() in sequence
     * - Updates usedAP counter to track total AP consumed
     * 
     * The AP system provides a time-sliced, turn-based mechanic where all entities
     * receive AP proportional to elapsed time, ensuring fair action distribution
     * regardless of server frame rate.
     * 
     * @see checkPlayers()
     * @see checkMonsters()
     * @see checkNPC()
     */
    void turntheworld();

    /**
     * @brief Processes all active players and their pending actions
     * 
     * For each online player, this method:
     * - Checks connection timeout and disconnects idle players
     * - Distributes action points and fight points
     * - Processes queued client commands
     * - Updates fight mode state
     * - Checks long-term actions and effects
     * - Periodically saves player data
     * - Removes disconnected players and notifies visible characters
     * 
     * @see Player::workoutCommands()
     * @see Player::checkFightMode()
     */
    void checkPlayers();

    /**
     * @brief Invalidates all open dialogs for all players
     * 
     * Called when server state changes require closing all dialogs
     * (e.g., during reload operations).
     */
    void invalidatePlayerDialogs() const;

    /**
     * @brief Processes all active monsters and their AI behavior
     * 
     * For each living monster, this method:
     * - Checks monstertimer (1-minute interval) and spawns new monsters if enabled
     * - Consumes 1 AP from the current turn (if ap > 1, decrements ap)
     * - Distributes action points and fight points to each monster
     * - Checks and applies long-term effects
     * - Executes monster AI only if:
     *   - Monster can act (has sufficient AP)
     *   - A player is nearby (within MAX_ACT_RANGE) OR monster is on a route
     * 
     * Monster AI behavior:
     * - Target selection using weapon range and visibility
     * - Calls monster scripts (onAttacked, enemyNear, enemyOnSight)
     * - Performs attacks if target is in range and monster can fight
     * - Pathfinding toward last known target position
     * - Random movement and self-healing when no targets present
     * - Stays within spawn point range boundaries
     * 
     * Dead monsters are collected and removed after iteration completes.
     * 
     * @note Monsters are performance-optimized: only monsters near players are processed
     * @see SpawnPoint
     * @see Monster
     * @see isPlayerNearby()
     */
    void checkMonsters();

    /**
     * @brief Processes all active NPCs and their scripted behavior
     * 
     * For each NPC, this method:
     * - Distributes action points and fight points
     * - Checks and applies long-term effects
     * - Executes NPC scripts if a player is nearby
     * 
     * @see NPC
     * @see LuaNPCScript
     */
    void checkNPC();

    /**
     * @brief Loads all NPCs from database and places them in the world
     * 
     * Called during server initialization to spawn all persistent NPCs
     * defined in the database. Each NPC is created with its stored
     * position, appearance, and associated script.
     */
    void initNPC();

    /**
     * @brief Initializes and starts the scheduled script system
     * 
     * Sets up recurring script execution based on the scheduled scripts
     * table in the database.
     */
    void initScheduler();

    /**
     * @brief Gets the current in-game time value
     * 
     * Returns various components of the Illarion time system, which has its own
     * calendar and time progression separate from real-world time.
     * 
     * @param timeType One of: "year", "month", "day", "hour", "minute", "second"
     * @return The requested time component value
     * @see sendIGTime()
     * @see sendIGTimeToAllPlayers()
     */
    auto getTime(const std::string &timeType) const -> long override;

    /**
     * @brief Enables or disables player login
     * 
     * When disabled, only GMs with gmr_allowlogin can connect.
     * Useful for maintenance or testing.
     * 
     * @param allow true to allow logins, false to restrict them
     * @see isLoginAllowed()
     */
    void allowLogin(bool allow) { _is_login_allowed = allow; }

    /**
     * @brief Checks if player login is currently allowed
     * 
     * @return true if regular players can log in, false if restricted to GMs
     * @see allowLogin()
     */
    auto isLoginAllowed() const -> bool { return _is_login_allowed; }

    /**
     * @brief Enables or disables monster spawning
     * 
     * When disabled, spawn points will not create new monsters. Useful
     * for events or testing scenarios.
     * 
     * @param enable true to enable spawning, false to disable
     * @see isSpawnEnabled()
     */
    void enableSpawn(bool enable) { _is_spawn_enabled = enable; }

    /**
     * @brief Checks if monster spawning is currently enabled
     * 
     * @return true if monsters can spawn, false otherwise
     * @see enableSpawn()
     */
    auto isSpawnEnabled() const -> bool { return _is_spawn_enabled; }

    /**
     * @brief Creates the singleton World instance
     * 
     * Initializes the world with spawn points, GM/player commands, and
     * monitoring clients. Must be called before World::get().
     * 
     * @return Pointer to the created or existing World instance
     * @see get()
     */
    static auto create() -> World *;

    /**
     * @brief Gets the singleton World instance
     * 
     * @return Pointer to the World instance
     * @throws std::runtime_error if World::create() has not been called
     * @see create()
     */
    static auto get() -> World *;

    /**============ WorldIMPLTools.cpp ==================*/

    /**
     * @brief Removes all NPCs marked for deletion from the world
     * 
     * Processes the LostNpcs list, removing each NPC from its field and
     * notifying visible players. This is a workaround to prevent segfaults
     * when NPCs are sold or otherwise removed during gameplay.
     * 
     * @todo Replace with better character lifecycle management using smart pointers
     * @see LostNpcs
     */
    void deleteAllLostNPC();

    /**
     * @brief Gets the currently executing Lua script
     * 
     * @return Pointer to current script, or nullptr if no script is running
     * @see setCurrentScript()
     */
    inline auto getCurrentScript() const -> LuaScript * { return currentScript; }

    /**
     * @brief Sets the currently executing Lua script
     * 
     * Used to track script context during execution for error reporting
     * and script callbacks.
     * 
     * @param script Pointer to the script being executed
     * @see getCurrentScript()
     */
    inline void setCurrentScript(LuaScript *script) { currentScript = script; }

    /**
     * @brief Updates the onlineplayer database table with current players
     * 
     * Clears and rebuilds the database table containing all currently
     * logged-in player IDs. Used for web interfaces and server monitoring.
     */
    void updatePlayerList() const;

    /**
     * @brief Finds all warp fields within a specified range of a position
     * 
     * Searches the map for warp fields (teleportation points) near the given
     * position, returning their locations.
     * 
     * @param pos Center position to search from
     * @param range Search radius in coordinates
     * @param warppositions Output vector filled with found warp field positions
     * @return true if any warp fields were found, false otherwise
     */
    auto findWarpFieldsInRange(const position &pos, Coordinate range, std::vector<position> &warppositions) -> bool;

    /**
     * @brief Determines all objects blocking line of sight between two positions
     * 
     * Uses a line-drawing algorithm to trace from start to end, checking each
     * field for blocking characters (players) or large items. Returns a list
     * of all blocking objects encountered along the line.
     * 
     * @param startingpos Starting position of the line
     * @param endingpos Ending position of the line
     * @return List of BlockingObjects along the line, ordered from start to end
     * @see BlockingObject
     */
    auto blockingLineOfSight(const position &startingpos, const position &endingpos) const
            -> std::list<BlockingObject> override;

    /**
     * @brief Finds all characters in sight within a directional cone
     * 
     * Searches for characters in range that are positioned in the specified
     * facing direction (e.g., all characters north of pos when direction is north).
     * 
     * @param pos Origin position
     * @param range Search radius
     * @param ret Output vector filled with found characters
     * @param direction Facing direction to search in
     * @return true if any characters were found, false otherwise
     */
    auto findTargetsInSight(const position &pos, Coordinate range, std::vector<Character *> &ret,
                            Character::face_to direction) const -> bool;

    /**
     * @brief Finds any character standing on the specified field
     * 
     * Searches all three character containers (players, monsters, NPCs) for
     * a character at the given position.
     * 
     * @param pos Position to check
     * @return Pointer to the character found, or nullptr if field is empty
     * @see findPlayerOnField()
     */
    auto findCharacterOnField(const position &pos) const -> Character *;

    /**
     * @brief Finds a player standing on the specified field
     * 
     * Only searches the Players container, more efficient than findCharacterOnField
     * when you know the character type.
     * 
     * @param pos Position to check
     * @return Pointer to the player found, or nullptr if no player present
     * @see findCharacterOnField()
     */
    auto findPlayerOnField(const position &pos) const -> Player *;

    /**
     * @brief Searches for a character by their unique ID
     * 
     * Searches all three character containers based on ID ranges:
     * - IDs < MONSTER_BASE: Players
     * - IDs < NPC_BASE: Monsters (including newMonsters queue)
     * - IDs >= NPC_BASE: NPCs
     * 
     * @param id Character ID to find
     * @return Pointer to the character, or nullptr if not found
     * @todo Should be unified when character containers are merged
     */
    virtual auto findCharacter(TYPE_OF_CHARACTER_ID id) -> Character *;

    /**
     * @brief Removes all monsters and NPCs from the map and deletes them
     * 
     * Clears the Monsters and Npc containers, removing each entity from
     * its field and freeing memory. Used during server shutdown or reload.
     * Players are not affected.
     * 
     * @see Monsters
     * @see Npc
     */
    void takeMonsterAndNPCFromMap();

    /**
     * @brief Executes an attack from one character to their current target
     * 
     * Resolves a combat action where the character attacks their enemyid target.
     * Handles attack resolution, death, script callbacks, and target loss notifications.
     * The character must have an enemy set and the target must be in range.
     * 
     * @param cp Attacking character
     * @return true if attack was executed, false if target not found/out of range
     * @see Character::attack()
     */
    auto characterAttacks(Character *cp) const -> bool;

    /**
     * @brief Immediately kills a monster and removes it from the world
     * 
     * Removes the monster from its field, notifies visible players, and
     * deletes the monster object. Does not generate loot or call death scripts.
     * 
     * @param id Monster's character ID
     * @return true if monster was found and killed, false otherwise
     */
    auto killMonster(TYPE_OF_CHARACTER_ID id) -> bool;

    /**
     * @brief Gets the field at a specific position (mutable)
     * 
     * Delegates directly to the WorldMap's at() method.
     * 
     * @param pos Position to retrieve
     * @return Reference to the field
     * @throws FieldNotFound if position is not loaded or outside any map boundary
     */
    auto fieldAt(const position &pos) -> map::Field & override;

    /**
     * @brief Gets the field at a specific position (const)
     * 
     * Delegates directly to the WorldMap's at() method.
     * 
     * @param pos Position to retrieve
     * @return Const reference to the field
     * @throws FieldNotFound if position is not loaded or outside any map boundary
     */
    auto fieldAt(const position &pos) const -> const map::Field & override;

    /**
     * @brief Finds the first non-transparent field at or below a position
     * 
     * Iterates downward from the given position for up to RANGEDOWN+1 levels (typically 2),
     * checking each z-level until a solid (non-transparent) field is found.
     * 
     * The position parameter is modified to the z-level where the solid field was found.
     * This is useful for determining where items/characters actually rest when placed
     * on transparent tiles like air.
     * 
     * @param pos Starting position (modified to the found field's position)
     * @return Reference to the first non-transparent field
     * @throws FieldNotFound if all checked levels are transparent or out of range
     */
    auto fieldAtOrBelow(position &pos) -> map::Field &;

    /**
     * @brief Finds a walkable field near the specified position
     * 
     * Delegates to walkableNear() utility function which searches adjacent fields
     * for one that is not blocked by terrain, items, or characters.
     * 
     * @param pos Target position
     * @return Reference to a nearby walkable field
     * @throws FieldNotFound if no walkable field is nearby
     */
    auto walkableFieldNear(const position &pos) -> map::Field &;

    /**
     * @brief Marks a field as persistent (should be saved to disk)
     * 
     * Persistent fields have their tile and item data saved when the map
     * is written to disk, preserving player/GM modifications across server restarts.
     * Delegates to WorldMap::makePersistentAt().
     * 
     * @param pos Position to mark as persistent
     */
    void makePersistentAt(const position &pos) override;

    /**
     * @brief Removes persistence marking from a field
     * 
     * The field will revert to its default state on next server restart,
     * discarding any runtime modifications. Delegates to WorldMap::removePersistenceAt().
     * 
     * @param pos Position to unmark
     */
    void removePersistenceAt(const position &pos) override;

    /**
     * @brief Checks if a field is marked as persistent
     * 
     * Delegates to WorldMap::isPersistentAt().
     * 
     * @param pos Position to check
     * @return true if field is persistent, false otherwise
     */
    auto isPersistentAt(const position &pos) const -> bool override;

    /**
     * @brief Retrieves a specific attribute value for an item type
     * 
     * Queries various item data tables (armor, weapon, tiles modificator, common items)
     * for specific attributes. Used primarily for scripting and item analysis.
     * 
     * Supported attributes:
     * - Armor: "bodyparts", "strokearmor", "thrustarmor", "armormagicdisturbance"
     * - Weapon: "accuracy", "attack", "defence", "range", "weapontype", "weaponmagicdisturbance"
     * - Item: "agingspeed", "objectafterrot", "weight"
     * - Tiles: "modificator"
     * 
     * @param s Attribute name to query
     * @param ItemID Item type ID
     * @return Attribute value, or 0 if attribute/item not found
     */
    static auto getItemAttrib(const std::string &s, TYPE_OF_ITEM_ID ItemID) -> int;

    /**
     * @brief Loads all world maps from disk
     * 
     * Attempts to load saved map files. If loading fails, falls back to
     * importing from the map editor format.
     * 
     * @see Save()
     * @see import()
     */
    void Load();

    /**
     * @brief Saves all world maps to disk
     * 
     * Persists modified map data to disk files for preservation across
     * server restarts.
     * 
     * @see Load()
     */
    void Save() const;

    /**
     * @brief Imports maps from map editor format
     * 
     * Forces import of maps from editor files, overwriting any existing
     * runtime or saved data. Used for deploying new map versions.
     * 
     * @see Load()
     */
    void import();

    /**
     * @brief Creates a new map area in the world
     * 
     * @param name Map identifier/name
     * @param origin Top-left corner position
     * @param width Map width in tiles
     * @param height Map height in tiles
     * @param tile Default tile ID to fill the map with
     * @return true if map was created successfully, false otherwise
     */
    auto createMap(const std::string &name, const position &origin, uint16_t width, uint16_t height, uint16_t tile)
            -> bool {
        return maps.createMap(name, origin, width, height, tile);
    }

    /**
     * @brief Notifies all visible players that a character was removed
     * 
     * Sends character removal messages to all players who can see the
     * specified position. Used when characters die, disconnect, or teleport.
     * 
     * @param id Character ID that was removed
     * @param pos Position where the character was removed from
     */
    void sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, const position &pos) const;

    /**
     * @brief Notifies all visible players of a character's health change
     * 
     * Updates health bars displayed by player clients for visible characters.
     * 
     * @param cc Character whose health changed
     * @param health New health value
     */
    void sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health) const;

    /**============in WorldIMPLCharacterMoves.cpp==================*/

    /**
     * @brief Sends all visible characters to a player's client
     * 
     * Used when a player logs in or their view changes significantly.
     * Populates the player's client with all characters currently visible
     * within their screen range.
     * 
     * @param cp Player who should receive character data
     * @param sendSpin If true, also sends character facing directions
     */
    void sendAllVisibleCharactersToPlayer(Player *cp, bool sendSpin);

    /**
     * @brief Creates a one-way warp field with optional tile/item changes
     * 
     * Establishes a teleportation point that transports characters from
     * 'where' to 'target'. Optionally modifies the starting tile and/or
     * places an item marker at the warp location.
     * 
     * @param where Starting position of the warp field
     * @param target Destination position where characters are teleported
     * @param starttilenr Tile ID to set at warp location (0 = no change)
     * @param startitemnr Item ID to place at warp location (0 = no change)
     * @return true if warp field was successfully created, false otherwise
     * @see removeWarpField()
     */
    auto addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                      Item::id_type startitemnr) -> bool;

    /**
     * @brief Creates a bidirectional warp field pair
     * 
     * Establishes two linked teleportation points that transport characters
     * between 'where' and 'target' in both directions. Optionally modifies
     * tiles and places items at both locations.
     * 
     * @param where First warp field position
     * @param target Second warp field position
     * @param starttilenr Tile ID at first position (0 = no change)
     * @param startitemnr Item ID at first position (0 = no change)
     * @param targettilenr Tile ID at second position (0 = no change)
     * @param targetitemnr Item ID at second position (0 = no change)
     * @return true if both warp fields were successfully created, false otherwise
     * @see addWarpField()
     * @see removeWarpField()
     */
    auto addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                      Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr) -> bool;

    /**
     * @brief Removes a warp field at the specified position
     * 
     * @param pos Position of the warp field to remove
     * @return true if warp field was found and removed, false otherwise
     * @see addWarpField()
     */
    auto removeWarpField(const position &pos) -> bool;

    /**
     * @brief Triggers field scripts when a character moves to/from a field
     * 
     * Calls Lua trigger scripts associated with the character's current
     * field position, notifying the script of entrance or exit.
     * 
     * @param cc Character that is moving
     * @param moveto true if moving onto the field, false if moving away
     * @see checkFieldAfterMove()
     */
    static void triggerFieldMove(Character *cc, bool moveto);

    /**
     * @brief Updates character container indices after position change
     * 
     * Must be called after a character's position is modified to maintain
     * the spatial indexing used by character containers for efficient lookups.
     * 
     * @param cc Character that moved
     * @param to New position of the character
     */
    void moveTo(Character *cc, const position &to);

    /**
     * @brief Moves a player in a specific direction
     * 
     * Handles player movement including collision detection, walking cost
     * calculation, and animation. Updates the player's position and facing
     * direction if movement is successful.
     * 
     * @param cp Player to move
     * @param d Direction to move (0=North, 2=West, 4=South, 6=East, 8=up, 9=down)
     * @param walkcost Output parameter receiving the movement cost in tenths of a second
     * @return true if movement was successful, false if blocked or invalid
     * @todo Can be deprecated in favor of Player::move()
     */
    auto pushPlayer(Player *cp, unsigned char d, short int &walkcost) -> bool;

    /**
     * @brief Triggers field effects after a character moves onto a field
     * 
     * Checks for special items and trigger fields at the character's new
     * position, calling associated Lua scripts. Used for traps, teleporters,
     * and other field-based mechanics.
     * 
     * @param character Character that just moved
     * @param field Field the character moved onto
     * @see triggerFieldMove()
     */
    static void checkFieldAfterMove(Character *character, const map::Field &field);

    /**
     * @brief Notifies visible players of a character's passive (pushed) movement
     * 
     * Used when a character is pushed, knocked back, or otherwise moved
     * without walking normally.
     * 
     * @param ccp Character that was moved passively
     */
    void sendPassiveMoveToAllVisiblePlayers(Character *ccp) const;

    /**
     * @brief Notifies visible players of a character's facing direction change
     * 
     * @param cc Character that turned/spun
     */
    void sendSpinToAllVisiblePlayers(Character *cc) const;

    /**
     * @brief Notifies visible players of a character's movement
     * 
     * Sends movement animation commands to all players who can see the
     * character's position.
     * 
     * @param cc Character that moved
     * @param moveType Type of movement (NORMALMOVE, RUNNING, etc.)
     * @param duration Movement duration in tenths of a second
     */
    void sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char moveType,
                                              TYPE_OF_WALKINGCOST duration) const;

    /**
     * @brief Notifies visible characters of a character's movement
     * 
     * Similar to sendCharacterMoveToAllVisiblePlayers but only sends to players.
     * Uses NORMALMOVE type.
     * 
     * @param cc Character that moved
     * @param duration Movement duration in tenths of a second
     */
    void sendCharacterMoveToAllVisibleChars(Character *cc, TYPE_OF_WALKINGCOST duration) const;

    /**
     * @brief Notifies visible players of a character's teleportation/warp
     * 
     * Handles the network updates when a character instantly moves from one
     * position to another (not walking). Removes character from old position
     * and adds at new position for affected players.
     * 
     * @param cc Character that warped
     * @param oldpos Position before teleportation
     * @param moveType Type of warp movement
     */
    void sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char moveType) const;

    /**
     * @brief Template helper for sending character data to a player
     * 
     * @tparam T Character type (Player, Monster, or NPC)
     * @param vec Vector of characters to send
     * @param cp Player who should receive the data
     * @param sendSpin If true, also send facing directions
     */
    template <class T> void sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin);

    /**
     * @brief Sends item information when a player looks at a map item
     * 
     * Generates and sends detailed item information including name, description,
     * and custom script data when a player examines an item on the ground.
     * 
     * @param player Player performing the look action
     * @param pos Position of the item on the map
     * @param stackPos Index of the item in the field's item stack
     */
    void lookAtMapItem(Player *player, const position &pos, uint8_t stackPos);

private:
    /**
     * @brief Sends tile information when a player looks at a map tile
     * 
     * @param cp Player performing the look action
     * @param tile Tile ID being examined
     * @param pos Position of the tile
     */
    static void lookAtTile(Player *cp, unsigned short int tile, const position &pos);

public:
    /**
     * @brief Sends item information when a player looks at a showcase item
     * 
     * Showcases are opened containers displayed to the player. This sends
     * detailed information about an item in an open showcase.
     * 
     * @param cp Player performing the look action
     * @param showcase Showcase ID (which open container)
     * @param position Item position within the showcase
     */
    void lookAtShowcaseItem(Player *cp, uint8_t showcase, unsigned char position);

    /**
     * @brief Sends item information when a player looks at an inventory item
     * 
     * Sends detailed information about an item in the player's belt or body slots.
     * 
     * @param cp Player performing the look action
     * @param position Inventory slot being examined
     */
    void lookAtInventoryItem(Player *cp, unsigned char position);

    /**
     * @brief Plays a sound effect for all players within range
     * 
     * @param pos Origin position of the sound
     * @param radius Hearing radius in coordinates
     * @param sound Sound effect ID to play
     */
    void makeSoundForAllPlayersInRange(const position &pos, int radius, unsigned short int sound) const;

    /**
     * @brief Shows a graphical effect for all players within range
     * 
     * @param pos Origin position of the effect
     * @param radius Visibility radius in coordinates
     * @param gfx Graphics effect ID to display
     */
    void makeGFXForAllPlayersInRange(const position &pos, int radius, unsigned short int gfx) const;

    /**
     * @brief Broadcasts a message to all characters within speech range
     * 
     * Sends a single-language message to all characters (players, monsters, NPCs)
     * within range based on talk type. Range depends on whisper/say/yell.
     * 
     * @param message Message text to send
     * @param tt Talk type (whisper, say, or yell)
     * @param cc Speaking character
     * @see sendMessageToAllCharsInRange(const std::string&, const std::string&, Character::talk_type, Character*)
     */
    void sendMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Character *cc) const;

    /**
     * @brief Broadcasts a bilingual message to all characters within speech range
     * 
     * Sends German and English versions of a message to all characters within
     * range. Players receive the appropriate language based on their client settings.
     * 
     * @param german German version of the message
     * @param english English version of the message
     * @param tt Talk type (whisper, say, or yell)
     * @param cc Speaking character
     */
    void sendMessageToAllCharsInRange(const std::string &german, const std::string &english, Character::talk_type tt,
                                      Character *cc) const;

    /**
     * @brief Sends a language-specific message to characters with that language skill
     * 
     * Only characters who understand the specified language will receive the message.
     * Messages are prefixed with language indicator (e.g., "[hum]" for human language).
     * 
     * @param message Message text
     * @param tt Talk type (whisper, say, or yell)
     * @param lang Language ID (see Language enum)
     * @param cc Speaking character
     * @todo This function is deprecated in favor of client-side language handling
     */
    void sendLanguageMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Language lang,
                                              Character *cc) const;

    /**
     * @brief Sends a message to all online players
     * 
     * Broadcasts a single-language message to every logged-in player regardless
     * of position. Used for server announcements.
     * 
     * @param message Message to broadcast
     * @see broadcast()
     */
    void sendMessageToAllPlayers(const std::string &message) const;

    /**
     * @brief Broadcasts a bilingual message to all online players
     * 
     * Sends German and English versions to all players, with each player
     * receiving their appropriate language version.
     * 
     * @param german German version of the message
     * @param english English version of the message
     */
    void broadcast(const std::string &german, const std::string &english) const override;

    /**
     * @brief Sends a message to all GMs with gmr_getgmcalls permission
     * 
     * Used for GM notifications, help requests, and administrative alerts.
     * 
     * @param message Message to send to GMs
     * @see gm_rights
     */
    void sendMessageToAdmin(const std::string &message) const;

    /**
     * @brief Converts a language ID to its skill name
     * 
     * @param languageId Language ID constant (languageHuman, languageDwarf, etc.)
     * @return Skill name string (e.g., "human language", "dwarf language")
     * @see languagePrefix()
     */
    static auto languageNumberToSkillName(int languageId) -> std::string;

    /**
     * @brief Gets the chat prefix for a language
     * 
     * Returns the bracketed prefix shown in chat for language-specific messages.
     * 
     * @param languageId Language ID constant
     * @return Prefix string (e.g., "[hum] ", "[dwa] ", or "" for common)
     * @see languageNumberToSkillName()
     */
    static auto languagePrefix(int languageId) -> std::string;

    /**
     * @brief Calculates the hearing range for a talk type
     * 
     * @param tt Talk type (whisper, say, or yell)
     * @return Range structure with radius and zRadius values
     */
    static auto getTalkRange(Character::talk_type tt) -> Range;

    /**
     * @brief Triggers player introduction to nearby characters
     * 
     * Called when a player speaks, allowing nearby players to learn their name
     * through the introduction system.
     * 
     * @param cp Player who is introducing themselves
     * @see forceIntroducePlayer()
     * @see ForceIntroduce()
     */
    void introduceMyself(Player *cp) const;

    /**
     * @brief Forces one player to learn another player's name
     * 
     * Used by GMs to grant knowledge of a player's identity without
     * normal introduction process.
     * 
     * @param cp Player to introduce
     * @param admin GM forcing the introduction
     * @see ForceIntroduce()
     */
    static void forceIntroducePlayer(Player *cp, Player *admin);

    /**
     * @brief Forces a player to learn character names from text
     * 
     * Parses a string of character names and adds them to the player's
     * known characters list.
     * 
     * @param player Player who will learn the names
     * @param text Space-separated list of character names
     * @see ForceIntroduceAll()
     */
    void ForceIntroduce(Player *player, const std::string &text) const;

    /**
     * @brief Forces a player to know all online character names
     * 
     * Grants knowledge of every currently logged-in player's name.
     * GM command utility.
     * 
     * @param player Player who will learn all names
     * @see ForceIntroduce()
     */
    void ForceIntroduceAll(Player *player) const;

    /**
     * @brief Sends current weather data to a specific player
     * 
     * @param cp Player to receive weather update
     * @see sendWeatherToAllPlayers()
     * @see setWeather()
     */
    void sendWeather(Player *cp) const;

    /**
     * @brief Broadcasts current weather to all online players
     * 
     * @see sendWeather()
     * @see setWeather()
     */
    void sendWeatherToAllPlayers();

    /**
     * @brief Broadcasts current in-game time to all online players
     * 
     * @see sendIGTime()
     * @see getTime()
     */
    void sendIGTimeToAllPlayers();

    /**
     * @brief Sends current in-game time to a specific player
     * 
     * @param cp Player to receive time update
     * @see sendIGTimeToAllPlayers()
     * @see getTime()
     */
    void sendIGTime(Player *cp) const;

    ////////// in WorldIMPLAdmin.cpp /////////////

    /**
     * @brief Broadcasts a message from a GM to all players
     * 
     * Sends a message from the specified GM to every online player.
     * Message appears as a say command in the game world.
     * 
     * @param cp GM sending the broadcast
     * @param message Message text to broadcast
     */
    void broadcast_command(Player *cp, const std::string &message) const;

    /**
     * @brief Kicks all non-GM players from the server
     * 
     * Forces disconnection of all regular players while leaving GMs online.
     * Used for emergency maintenance or server updates.
     * 
     * @param cp GM executing the command
     */
    void kickall_command(Player *cp);

    /**
     * @brief Kicks a specific player from the server
     * 
     * Forces disconnection of the named player.
     * 
     * @param cp GM executing the kick
     * @param player Name of the player to kick
     */
    void kickplayer_command(Player *cp, const std::string &player) const;

    /**
     * @brief Displays IP addresses of all online players to a GM
     * 
     * Shows connection information for all logged-in players.
     * Used for identifying multi-accounts or connection issues.
     * 
     * @param cp GM requesting the information
     */
    static void showIPS_Command(Player *cp);

    /**
     * @brief Creates an item in the GM's inventory
     * 
     * Generates a new item with the specified ID and adds it to the GM's
     * inventory for testing or events.
     * 
     * @param cp GM creating the item
     * @param itemid Item ID to create (as string)
     */
    static void create_command(Player *cp, const std::string &itemid);

    /**
     * @brief Spawns a monster near the GM
     * 
     * Creates a monster of the specified type at a position near the GM.
     * Used for testing or live events.
     * 
     * @param cp GM spawning the monster
     * @param monsterId Monster type ID (as string)
     */
    void spawn_command(Player *cp, const std::string &monsterId);

    /**
     * @brief Activates a specific logging facility
     * 
     * Enables detailed logging for a particular subsystem (e.g., "World", "Script").
     * Used for debugging specific server components.
     * 
     * @param cp GM enabling the log
     * @param log Name of the logging facility to activate
     * @see logoff_command()
     */
    void logon_command(Player *cp, const std::string &log);

    /**
     * @brief Deactivates a specific logging facility
     * 
     * Disables detailed logging for a particular subsystem.
     * 
     * @param cp GM disabling the log
     * @param log Name of the logging facility to deactivate
     * @see logon_command()
     */
    void logoff_command(Player *cp, const std::string &log);

    /**
     * @brief Teleports a GM to another player's location
     * 
     * Instantly moves the GM to the position of the named player.
     * Also known as "jump to" or "goto" command.
     * 
     * @param player GM to teleport
     * @param target Name of the target player
     * @see jumpto_command()
     */
    void teleportPlayerToOther(Player *player, const std::string &target) const;

    /**
     * @brief Kills all monsters on the map
     * 
     * Removes every monster from the world instantly. Used for clearing
     * spawns during events or after testing.
     * 
     * @param cp GM executing the command
     */
    void kill_command(Player *cp) const;

    /**
     * @brief Performs a full reload of tables, definitions, and scripts
     * 
     * Reloads all game data from the database and filesystem without
     * restarting the server. Equivalent to the former #r command.
     * This includes:
     * - Item, monster, NPC tables
     * - All Lua scripts
     * - Spawn points
     * - Quests and skills
     * 
     * @param cp GM initiating the reload
     * @see reload_defs()
     */
    void reload_command(Player *cp);

    /**
     * @brief Teleports a GM to a player's location
     * 
     * Alias for teleportPlayerToOther(). Substitutes the #j command.
     * 
     * @param cp GM jumping to another player
     * @param player Name of the target player
     * @see teleportPlayerToOther()
     */
    void jumpto_command(Player *cp, const std::string &player) const;

    /**
     * @brief Saves all world maps to disk
     * 
     * Forces immediate persistence of map data. Equivalent to #mapsave.
     * 
     * @param cp GM requesting the save
     * @see Save()
     */
    void save_command(Player *cp);

    /**
     * @brief Makes a player invisible to others
     * 
     * Sets invisibility flag, hiding the GM from other players' views.
     * The GM can still see and interact with the world normally.
     * 
     * @param cp GM to make invisible
     * @see makeVisible()
     */
    void makeInvisible(Player *cp) const;

    /**
     * @brief Speaks a message as another player
     * 
     * Allows a GM to send a message that appears to come from another
     * player. Used for roleplaying events or testing.
     * 
     * @param player GM using the command
     * @param text Format: "player_name message text"
     */
    void talkto_command(Player *player, const std::string &text) const;

    /**
     * @brief Makes an invisible player visible again
     * 
     * Removes invisibility flag, making the GM visible to other players.
     * 
     * @param cp GM to make visible
     * @see makeInvisible()
     */
    void makeVisible(Player *cp) const;

    /**
     * @brief Forces logout of all players
     * 
     * Disconnects every player from the server, including GMs.
     * Used for emergency shutdowns.
     */
    void forceLogoutOfAllPlayers();

    /**
     * @brief Forces logout of a specific player
     * 
     * Disconnects the named player from the server.
     * 
     * @param name Player name to log out
     * @return true if player was found and logged out, false otherwise
     */
    auto forceLogoutOfPlayer(const std::string &name) const -> bool;

    /**
     * @brief Sends detailed information about all online players to a GM
     * 
     * Displays comprehensive player data including position, stats, and
     * connection information.
     * 
     * @param admin GM requesting the information
     */
    static void sendAdminAllPlayerData(Player *admin);

    /**
     * @brief Warps a GM to specific coordinates
     * 
     * Server-side implementation of !warp_to x y z command. Teleports
     * the GM to the specified position.
     * 
     * @param player GM to warp
     * @param text Coordinates in format "x y z"
     */
    static void warpto_command(Player *player, const std::string &text);

    /**
     * @brief Summons a player to the GM's location
     * 
     * Teleports the named player to the GM's current position.
     * Server-side implementation of !summon command.
     * 
     * @param player GM doing the summoning
     * @param text Name of player to summon
     */
    void summon_command(Player *player, const std::string &text) const;

    /**
     * @brief Reloads only game data definitions without scripts
     * 
     * Reloads database tables (items, monsters, etc.) but does not
     * reload Lua scripts, NPCs, or spawn points. Faster than full reload.
     * 
     * @param cp GM requesting the reload
     * @return true if reload was successful, false otherwise
     * @see reload_command()
     */
    auto reload_defs(Player *cp) const -> bool;

    /**
     * @brief Imports warp fields from a text file
     * 
     * Reads warp field definitions from a file and adds them to the map.
     * Used for bulk teleporter setup.
     * 
     * @param cp GM importing the warp fields
     * @param filename Path to the warp field definition file
     * @return true if import was successful, false otherwise
     */
    auto importWarpFields(Player *cp, const std::string &filename) -> bool;

    /**
     * @brief Removes a warp field at specified coordinates
     * 
     * Deletes a teleporter from the map.
     * 
     * @param cp GM removing the warp field
     * @param text Coordinates in format "x y z"
     * @see addWarpField()
     */
    void removeTeleporter(Player *cp, const std::string &text);

    /**
     * @brief Shows all warp fields within a radius
     * 
     * Displays information about nearby warp fields to the GM.
     * 
     * @param cp GM requesting the list
     * @param text Radius to search (as string)
     * @see findWarpFieldsInRange()
     */
    void showWarpFieldsInRange(Player *cp, const std::string &text);
    //////////////////////////////////////////////////////////////////

    //////////// in WorldIMPLItemMoves.cpp ////////////////

    /**
     * @brief Global temporary item for atomic item transfer operations
     * 
     * Used as a temporary holding location during item moves. All item manipulation
     * functions use this global to transfer items between locations (map, inventory,
     * showcases). This avoids complex parameter passing but requires careful sequencing
     * of take/put operations.
     * 
     * @warning Not thread-safe - assumes single-threaded game loop execution
     */
    Item g_item;
    
    /**
     * @brief Global temporary container for atomic container transfer operations
     * 
     * Used when moving containers between locations. Works in conjunction with g_item
     * to handle container transfers with their contents intact.
     * 
     * @warning Not thread-safe - assumes single-threaded game loop execution
     */
    Container *g_cont{};

    /**
     * @brief Removes an item from the map and places it in g_item
     * 
     * @param cc Character taking the item
     * @param itemPosition Map position of the item
     * @return true if item was successfully taken, false otherwise
     */
    auto takeItemFromMap(Character *cc, const position &itemPosition) -> bool;

    /**
     * @brief Places g_item onto the map if the field is not blocked
     * 
     * @param cc Character placing the item
     * @param itemPosition Target map position
     * @return true if item was successfully placed, false if blocked
     */
    auto putItemOnMap(Character *cc, const position &itemPosition) -> bool;

    /**
     * @brief Places g_item onto the map regardless of blocking
     * 
     * @param cc Character placing the item
     * @param itemPosition Target map position
     * @return true if item was successfully placed
     */
    auto putItemAlwaysOnMap(Character *cc, const position &itemPosition) -> bool;

    /**
     * @brief Takes an item from character inventory and places it in g_item
     * 
     * @param cc Character whose inventory to take from
     * @param pos Inventory slot position
     * @param count Number of items to take
     * @return true if item was successfully taken, false otherwise
     */
    auto takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count) -> bool;

    /**
     * @brief Places g_item into character inventory at specified slot
     * 
     * Atomic operation that attempts to place g_item from the global temporary into
     * a character's inventory. Handles:
     * - Backpack placement (only containers, sets backPackContents)
     * - Body slot placement with equipment rules (armor body parts, two-handed weapons)
     * - Two-handed weapons block opposite hand slot with BLOCKEDITEM
     * - Stackable items merge with existing stacks if same type and data
     * - Updates character appearance to all visible players on success
     * - Clears g_item and g_cont on successful placement
     * 
     * @param cc Character whose inventory receives the item
     * @param pos Target inventory slot (0-17, or BACKPACK)
     * @return true if item was successfully placed, false if blocked or incompatible
     * @see takeItemFromInvPos()
     */
    auto putItemOnInvPos(Character *cc, unsigned char pos) -> bool;

    /**
     * @brief Takes an item from player inventory and places it in g_item
     * 
     * Player-specific version with additional checks.
     * 
     * @param cc Player whose inventory to take from
     * @param pos Inventory slot position
     * @param count Number of items to take
     * @return true if item was successfully taken, false otherwise
     */
    auto takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count) -> bool;

    /**
     * @brief Places g_item into player inventory
     * 
     * Player-specific version with additional checks and notifications.
     * 
     * @param cc Player whose inventory to place into
     * @param pos Target inventory slot
     * @return true if item was successfully placed, false otherwise
     */
    auto putItemOnInvPos(Player *cc, unsigned char pos) -> bool;

    /**
     * @brief Closes showcases for other players viewing a container
     * 
     * When a container is moved, this closes it for all other players
     * who had it open to prevent desyncs.
     * 
     * @param target Player moving the container
     * @param moved Container being moved
     */
    void closeShowcaseForOthers(Player *target, Container *moved) const;

    /**
     * @brief Closes showcase if container is moved out of range
     * 
     * @param moved Container that was moved
     * @param showcasePosition Position where container was located
     */
    void closeShowcaseIfNotInRange(Container *moved, const position &showcasePosition) const;

    /**
     * @brief Takes an item from an open showcase and places it in g_item
     * 
     * @param cc Player taking from showcase
     * @param showcase Showcase ID
     * @param pos Item position in showcase
     * @param count Number of items to take
     * @return true if item was successfully taken, false otherwise
     */
    auto takeItemFromShowcase(Player *cc, uint8_t showcase, unsigned char pos, Item::number_type count) -> bool;

    /**
     * @brief Places g_item into an open showcase
     * 
     * @param cc Player placing into showcase
     * @param showcase Showcase ID
     * @param pos Target slot in showcase
     * @return true if item was successfully placed, false otherwise
     */
    auto putItemInShowcase(Player *cc, uint8_t showcase, TYPE_OF_CONTAINERSLOTS pos) -> bool;

    /**
     * @brief Checks and triggers field scripts after item manipulation
     * 
     * @param field Field to check
     * @param itemPosition Position of the field
     */
    void checkField(const map::Field &field, const position &itemPosition) const;

    /**
     * @brief Moves an item from one map position to another
     * 
     * @param cp Player performing the move
     * @param oldPosition Source position
     * @param newPosition Destination position
     * @param count Number of items to move
     * @return true if move was successful, false otherwise
     */
    auto moveItemFromMapToMap(Player *cp, const position &oldPosition, const position &newPosition,
                              Item::number_type count) -> bool;

    /**
     * @brief Moves an item from map into an open showcase
     * 
     * @param cp Player performing the move
     * @param sourcePosition Map position of item
     * @param showcase Showcase ID
     * @param showcaseSlot Target slot in showcase
     * @param count Number of items to move
     */
    void moveItemFromMapIntoShowcase(Player *cp, const position &sourcePosition, uint8_t showcase,
                                     unsigned char showcaseSlot, Item::number_type count);

    /**
     * @brief Moves an item from map into player inventory
     * 
     * @param cp Player performing the move
     * @param sourcePosition Map position of item
     * @param inventorySlot Target inventory slot
     * @param count Number of items to move
     */
    void moveItemFromMapToPlayer(Player *cp, const position &sourcePosition, unsigned char inventorySlot,
                                 Item::number_type count);

    /**
     * @brief Moves an item from one showcase to another
     * 
     * @param cp Player performing the move
     * @param source Source showcase ID
     * @param pos Source slot position
     * @param dest Destination showcase ID
     * @param pos2 Destination slot position
     * @param count Number of items to move
     */
    void moveItemBetweenShowcases(Player *cp, uint8_t source, unsigned char pos, uint8_t dest, unsigned char pos2,
                                  Item::number_type count);

    /**
     * @brief Drops an item from showcase onto the map
     * 
     * @param cp Player performing the drop
     * @param showcase Showcase ID
     * @param pos Item slot in showcase
     * @param newPosition Target map position
     * @param count Number of items to drop
     */
    void dropItemFromShowcaseOnMap(Player *cp, uint8_t showcase, unsigned char pos, const position &newPosition,
                                   Item::number_type count);

    /**
     * @brief Moves an item from showcase to player inventory
     * 
     * @param cp Player performing the move
     * @param showcase Showcase ID
     * @param pos Item slot in showcase
     * @param cpos Target inventory slot
     * @param count Number of items to move
     */
    void moveItemFromShowcaseToPlayer(Player *cp, uint8_t showcase, unsigned char pos, unsigned char cpos,
                                      Item::number_type count);

    /**
     * @brief Moves an item between inventory slots
     * 
     * @param cp Player performing the move
     * @param opos Old inventory position
     * @param npos New inventory position
     * @param count Number of items to move
     */
    void moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count);

    /**
     * @brief Drops an item from player inventory onto the map
     * 
     * @param cp Player dropping the item
     * @param cpos Inventory slot position
     * @param newPosition Target map position
     * @param count Number of items to drop
     */
    void dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, const position &newPosition, Item::number_type count);

    /**
     * @brief Moves an item from player inventory into a showcase
     * 
     * @param cp Player performing the move
     * @param cpos Inventory slot position
     * @param showcase Showcase ID
     * @param pos Target slot in showcase
     * @param count Number of items to move
     */
    void moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, uint8_t showcase, unsigned char pos,
                                        Item::number_type count);

    /**
     * @brief Picks up an item from the map into player inventory
     * 
     * Automatically finds an appropriate inventory slot for the item.
     * 
     * @param cp Player picking up the item
     * @param itemPosition Map position of item
     * @return true if item was picked up, false otherwise
     */
    auto pickUpItemFromMap(Player *cp, const position &itemPosition) -> bool;

    /**
     * @brief Picks up all items at player's position
     * 
     * Attempts to pick up every item on the field the player is standing on.
     * 
     * @param cp Player picking up items
     */
    void pickUpAllItemsFromMap(Player *cp);

    /**
     * @brief Notifies visible characters that an item was removed from map
     * 
     * @param itemPosition Position where item was removed
     */
    void sendRemoveItemFromMapToAllVisibleCharacters(const position &itemPosition) const;

    /**
     * @brief Notifies visible characters that an item was placed on map
     * 
     * @param itemPosition Position where item was placed
     * @param it Item that was placed
     */
    void sendPutItemOnMapToAllVisibleCharacters(const position &itemPosition, const Item &it) const;

    /**
     * @brief Notifies visible characters that an item on map changed
     * 
     * @param id Item ID that changed
     * @param itemPosition Position of the item
     * @param it New item state
     */
    void sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, const position &itemPosition, const Item &it) const;

    /**
     * @brief Notifies players viewing a container that a slot changed
     * 
     * Version for when a container was moved.
     * 
     * @param cc Container that changed
     * @param slot Slot that changed
     * @param moved Container that was in the slot (if any)
     */
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot, Container *moved) const;

    /**
     * @brief Notifies players viewing a container that a slot changed
     * 
     * @param cc Container that changed
     * @param slot Slot that changed
     */
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot) const;

    //////////////////////////////////////In WorldIMPLScriptHelp.cpp//////////////////////////////////////////

    /**
     * @brief Deletes a dynamic NPC from the world
     * 
     * Removes an NPC that was created at runtime (not from database).
     * 
     * @param npcid NPC character ID to delete
     * @return true if NPC was found and deleted, false otherwise
     */
    auto deleteNPC(unsigned int npcid) -> bool override;

    /**
     * @brief Creates a dynamic NPC at runtime
     * 
     * Spawns a new NPC with the specified properties and script.
     * The NPC is not persisted to the database.
     * 
     * @param name NPC name
     * @param type Race type ID
     * @param pos Position to spawn at
     * @param sex NPC gender
     * @param scriptname Lua script file to attach
     * @return true if NPC was created successfully, false otherwise
     */
    auto createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos, Character::sex_type sex,
                          const std::string &scriptname) -> bool override;

    auto getPlayersOnline() const -> std::vector<Player *> override;
    auto getPlayerIdByName(const std::string &name, TYPE_OF_CHARACTER_ID &id) const -> bool override;

    auto getNPCS() const -> std::vector<NPC *> override;

    auto getCharactersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Character *> override;
    auto getPlayersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Player *> override;
    auto getMonstersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Monster *> override;
    auto getNPCSInRangeOf(const position &pos, uint8_t radius) const -> std::vector<NPC *> override;
    auto isPlayerNearby(const Character &character) const -> bool;

    auto getItemStats(const ScriptItem &item) const -> ItemStruct override;
    auto getItemStatsFromId(TYPE_OF_ITEM_ID id) const -> ItemStruct override;
    void changeQuality(ScriptItem item, int amount) override;

    /**
     * @brief Notifies scripts that a player is looking at an item
     * 
     * Allows item scripts to provide custom descriptions and information.
     * 
     * @param user Player looking at the item
     * @param item Item being examined
     * @param lookAt Structure to fill with description data
     */
    virtual void itemInform(Character *user, const ScriptItem &item, const ItemLookAt &lookAt);

    auto getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) const -> std::string override;
    auto changeItem(ScriptItem item) -> bool override;

    auto isCharacterOnField(const position &pos) const -> bool override;
    auto getCharacterOnField(const position &pos) const -> character_ptr override;

    auto erase(ScriptItem item, int amount) -> bool override;
    auto swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, int newQuality = 0) -> bool override;
    auto increase(ScriptItem item, int count) -> bool override;
    auto createFromId(TYPE_OF_ITEM_ID id, int count, const position &pos, bool always, int quality,
                      script_data_exchangemap const *data) -> ScriptItem override;

    auto createFromItem(const ScriptItem &item, const position &pos, bool always) -> bool override;

    auto createMonster(unsigned short id, const position &pos, short movepoints) -> character_ptr override;

    void gfx(unsigned short int gfxid, const position &pos) const override;
    void makeSound(unsigned short int soundid, const position &pos) const override;

    auto isItemOnField(const position &pos) -> bool override;
    auto getItemOnField(const position &pos) -> ScriptItem override;

    void changeTile(short int tileid, const position &pos) override;

    /**
     * @brief Creates a new map area without overwriting existing maps
     * 
     * Safe version that checks for overlaps before creating.
     * 
     * @param tile Default tile ID
     * @param origin Top-left corner position
     * @param height Map height
     * @param width Map width
     * @return true if area was created, false if it would overlap
     */
    auto createSavedArea(uint16_t tile, const position &origin, uint16_t height, uint16_t width) -> bool override;

    auto getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret) -> bool override;
    auto getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret) -> bool override;
    auto getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret) -> bool override;
    auto getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret) -> bool override;

    /**
     * @brief Sends a message to all monitoring clients
     * 
     * Monitoring clients are external tools that observe server state.
     * 
     * @param msg Message to send
     * @param id Message type ID (0=basic, 1=popup window)
     */
    void sendMonitoringMessage(const std::string &msg, unsigned char id = 0) const override;

    /**
     * @brief Bans a player for a specified duration
     * 
     * @param cp Player to ban
     * @param bantime Duration in seconds
     * @param gmid GM ID who issued the ban
     */
    void ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid) const;

    /**
     * @brief Sets login permissions for the server
     * 
     * @param player GM setting the login state
     * @param text "on" to enable logins, "off" to disable
     */
    void set_login(Player *player, const std::string &text);

protected:
    World(); ///< Protected constructor for testing

    static World *_self; ///< Singleton instance pointer

private:
    /**
     * @brief Logs when a required field is not found
     * 
     * @param function Function name where the error occurred
     * @param field Position that was not found
     */
    static void logMissingField(const std::string &function, const position &field);

    bool _is_login_allowed = true; ///< Whether regular players can log in
    bool _is_spawn_enabled = true; ///< Whether monsters should spawn

    /**
     * @brief Private constructor for World singleton
     * 
     * Initializes the world state:
     * - Sets lastTurnIGDay to current in-game day
     * - Records startTime for AP calculations
     * - Constructs scriptDir path from config datadir + SCRIPTSDIR
     * 
     * Called only by create(). The public World(const std::string&) constructor
     * exists but is never used in practice.
     * 
     * @see create()
     */
    explicit World(const std::string &dir);

    int lastTurnIGDay; ///< In-game day of last turntheworld() call (used to detect day changes for aging)

    Timer monstertimer{std::chrono::minutes(1)}; ///< 1-minute interval timer for spawn point processing

    /**
     * @brief Ages all map items by one tick
     */
    void ageMaps();

    /**
     * @brief Ages all items in character inventories
     */
    void ageInventory() const;

    std::string scriptDir; ///< Directory containing Lua scripts

    std::list<SpawnPoint> SpawnList; ///< All monster spawn points

    /**
     * @brief Loads spawn points from database
     * 
     * @return true if loading was successful, false otherwise
     */
    auto initRespawns() -> bool;

    using CommandMap = std::map<std::string, CommandType>;
    CommandMap GMCommands;     ///< Map of GM command names to handlers
    CommandMap PlayerCommands; ///< Map of player command names to handlers

    /**
     * @brief Shows information about a specific player to a GM
     * 
     * @param cp GM requesting information
     * @param tplayer Name of player to query
     */
    void who_command(Player *cp, const std::string &tplayer) const;

    /**
     * @brief Changes the tile in front of a GM
     * 
     * @param cp GM changing the tile
     * @param tile Tile ID to set
     */
    void tile_command(Player *cp, const std::string &tile);

    /**
     * @brief Enables turtle tile painting mode
     * 
     * @param cp GM enabling turtle mode
     * @param tile Tile ID to paint with
     */
    static void turtleon_command(Player *cp, const std::string &tile);

    /**
     * @brief Disables turtle tile painting mode
     * 
     * @param cp GM disabling turtle mode
     */
    static void turtleoff_command(Player *cp);

    /**
     * @brief Enables clipping (walking through walls)
     * 
     * @param cp GM enabling clipping
     */
    static void clippingon_command(Player *cp);

    /**
     * @brief Disables clipping
     * 
     * @param cp GM disabling clipping
     */
    static void clippingoff_command(Player *cp);

    /**
     * @brief Describes the tile and items in front of a GM
     * 
     * @param cp GM requesting information
     */
    void what_command(Player *cp);

    /**
     * @brief Saves all online players to database
     * 
     * @param cp GM requesting the save
     */
    void playersave_command(Player *cp) const;

    /**
     * @brief Creates a warp field on current tile
     * 
     * @param cp GM creating the warp
     * @param text Target coordinates "x y z"
     */
    void teleport_command(Player *cp, const std::string &text);

    /**
     * @brief Displays available GM commands
     * 
     * @param cp GM requesting help
     */
    static void gmhelp_command(Player *cp);

    /**
     * @brief Sends a GM help request ticket
     * 
     * @param player Player requesting GM help
     * @param ticket Help request message
     * @return true if ticket was submitted successfully
     */
    auto gmpage_command(Player *player, const std::string &ticket) const -> bool;

public:
    /**
     * @brief Processes ban command with parameters
     * 
     * @param cp GM issuing the ban
     * @param text Ban parameters (player name, duration, reason)
     */
    void ban_command(Player *cp, const std::string &text) const;

    /**
     * @brief Logs a GM help ticket to database
     * 
     * @param player Player submitting ticket
     * @param ticket Ticket message
     * @param automatic Whether ticket was auto-generated
     */
    void logGMTicket(Player *player, const std::string &ticket, bool automatic) const;

    /**
     * @brief Processes immediate player commands from queue
     * 
     * Handles commands that must be executed immediately outside
     * the normal game loop timing.
     */
    void checkPlayerImmediateCommands();

    /**
     * @brief Adds a player to the immediate command queue
     * 
     * @param player Player with urgent commands to process
     */
    void addPlayerImmediateActionQueue(Player *player);

private:
    map::WorldMap maps; ///< World map management system

    /**
     * @brief Gets all characters within radius (players, monsters, NPCs)
     * 
     * @param pos Center position
     * @param radius Search radius
     * @return Vector of all characters in range
     */
    auto getTargetsInRange(const position &pos, int radius) const -> std::vector<Character *>;

    /**
     * @brief Activates a language for the player
     * 
     * @param cp Player changing language
     * @param language Language name
     * @return true if language was activated successfully
     */
    static auto active_language_command(Player *cp, const std::string &language) -> bool;

    /**
     * @brief Registers all GM commands with their handlers
     */
    void InitGMCommands();

    /**
     * @brief Registers all player commands with their handlers
     */
    void InitPlayerCommands();

    /**
     * @brief Executes a user command if it exists
     * 
     * @param user User executing the command
     * @param input Full command string
     * @param commands Command map to search (GM or player)
     * @return true if command was found and executed
     */
    auto executeUserCommand(Player *user, const std::string &input, const CommandMap &commands) -> bool;

    /**
     * @brief Exports all maps to mapdir/export
     * 
     * @param cp GM requesting export
     * @return true if export was successful
     */
    auto exportMaps(Player *cp) const -> bool;

    /**
     * @brief Skips comment lines in an input stream
     * 
     * @param inputStream Stream to process
     */
    void ignoreComments(std::ifstream &inputStream);

    /**
     * @brief Reloads all game data tables from database
     * 
     * @param cp GM requesting reload
     * @return true if reload was successful
     */
    auto reload_tables(Player *cp) -> bool;

    /**
     * @brief Displays server version information
     * 
     * @param player Player requesting version info
     */
    static void version_command(Player *player);

    std::mutex immediatePlayerCommandsMutex;            ///< Mutex for immediate command queue
    std::queue<Player *> immediatePlayerCommands;       ///< Queue of players with immediate commands
};

#endif
