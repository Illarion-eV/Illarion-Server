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

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Character.hpp"
#include "Item.hpp"
#include "Showcase.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "script/LuaScript.hpp"

#include <chrono>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct WeatherStruct;
class Dialog;
class Timer;
class LongTimeAction;

/**
 * @brief GM permission flags for administrative commands
 * 
 * Bitflags controlling which GM commands a player can access.
 */
enum gm_rights {
    gmr_allowlogin = 1,         ///< Can login when server is closed
    gmr_basiccommands = 2,      ///< Basic commands (!who, !what, !fi, !inform)
    gmr_warp = 4,               ///< Can warp (#jump_to)
    gmr_summon = 8,             ///< Can summon players
    gmr_undef = 16,             ///< Undefined/reserved
    gmr_settiles = 32,          ///< Can modify tiles (ton/toff)
    gmr_clipping = 64,          ///< Can walk through walls
    gmr_warpfields = 128,       ///< Can manipulate warp fields
    gmr_import = 256,           ///< Can import maps/warpfields (includes createArea)
    gmr_visible = 512,          ///< Can change visibility state
    gmr_reload = 1024,          ///< Can reload tables (#r, !rd, !setspawn)
    gmr_ban = 2048,             ///< Can ban players
    gmr_loginstate = 4096,      ///< Can change login state
    gmr_save = 8192,            ///< Can save players/maps (!playersave, #mapsave)
    gmr_broadcast = 16384,      ///< Can broadcast messages
    gmr_forcelogout = 32768,    ///< Can force logout players
    gmr_getgmcalls = 65536,     ///< Receives GM help messages
    gmr_isnotshownasgm = 131072 ///< Hidden from GM list (unofficial GM state)
};

/**
 * @brief Player character controlled by a human client
 * 
 * Extends Character with network communication, client UI management, and player-specific
 * features including:
 * - Network connection and command processing
 * - Client UI (showcases, dialogs, inventory displays)
 * - Quest tracking and progress
 * - Player knowledge system (knows/introduces other players)
 * - GM administrative commands and permissions
 * - Save/load to database
 * - Crafting and merchant interactions
 * - Screen updates and visibility management
 * 
 * @note Most virtual methods from Character are overridden to send updates to client
 */
class Player : public Character {
public:
    /**
     * @brief Exception thrown when player must be logged out
     * 
     * Used when player authentication fails, data is corrupt, or server rejects login.
     */
    class LogoutException {
    public:
        /**
         * @brief Constructs logout exception with reason code
         * @param reason Logout reason code
         */
        explicit LogoutException(const char &reason) : m_reason(reason) {}

        /**
         * @brief Gets logout reason code
         * @return Reason code
         */
        [[nodiscard]] inline auto getReason() const -> const char & { return m_reason; }

    private:
        char m_reason; ///< Logout reason code
    };

    /**
     * @brief Look mode for examining items/characters
     */
    enum class LookMode { look = 0, stare = 1 };

    /**
     * @brief View direction for screen updates
     */
    enum viewdir { upper = 0, right = 1, lower = 2, left = 3 };

    /**
     * @brief Gets character type
     * @return Always returns player type
     */
    auto getType() const -> unsigned short override { return player; }

    /**
     * @brief Gets string representation of player
     * @return Player description string
     */
    auto to_string() const -> std::string override;

    /**
     * @brief Processes pending client commands
     */
    void workoutCommands();

    /**
     * @brief Checks and updates fight mode state
     */
    void checkFightMode();

    std::shared_ptr<NetInterface> Connection; ///< Network connection to client

    std::string last_ip; ///< Last known IP address

    int longTimeEffects{}; ///< Count of active long-term effects

    std::string pw;    ///< Password hash
    std::string email; ///< Email address

    bool newPlayer = false; ///< True if newly created character

    unsigned long int onlinetime{}; ///< Total time online in seconds

    time_t logintime{};     ///< Time of current login
    time_t lastsavetime{};  ///< Last database save time
    time_t lastkeepalive{}; ///< Last keepalive from client
    time_t lastaction{};    ///< Last player action time

    std::chrono::time_point<std::chrono::steady_clock> reachingTargetField = {}; ///< Movement timing

    std::string location; ///< Player location (for profile)
    std::string realname; ///< Real name (for profile)

    uint8_t screenwidth{0};  ///< Client screen width
    uint8_t screenheight{0}; ///< Client screen height

    /**
     * @brief Gets screen/view range based on resolution
     * @return View distance
     */
    auto getScreenRange() const -> Coordinate override;

private:
    /**
     * @brief Checks if item ID is in depot array
     * @param itemId Item ID to check
     * @return True if in depot
     */
    bool isIdInDepotArray(int itemId);

    std::set<uint32_t> visibleChars;                                     ///< Character IDs visible to player
    std::unordered_set<TYPE_OF_CHARACTER_ID> knownPlayers;               ///< Players this player knows
    std::unordered_map<TYPE_OF_CHARACTER_ID, std::string> namedPlayers;  ///< Custom names for players

    using CLIENTCOMMANDLIST = std::queue<ClientCommandPointer>;
    CLIENTCOMMANDLIST immediateCommands; ///< High-priority commands
    CLIENTCOMMANDLIST queuedCommands;    ///< Normal priority commands
    std::mutex commandMutex;             ///< Protects command queues

public:
    /**
     * @brief Receives command from client for processing
     * @param cmd Command pointer
     */
    void receiveCommand(const ClientCommandPointer &cmd);

    /**
     * @brief Stops attack and updates client
     */
    void stopAttack() override;

    /**
     * @brief Checks if player is newly created
     * @return True if new player
     */
    auto isNewPlayer() const -> bool override;

    /**
     * @brief Native language selector
     * @param german German text
     * @param english English text
     * @return Reference to text in player's language
     */
    auto nls(const std::string &german, const std::string &english) const -> const std::string &;

    /**
     * @brief Sets alive state and handles death
     * @param alive New alive state
     */
    void setAlive(bool alive) override;

    /**
     * @brief Checks and updates burden/overtaxed state
     */
    void checkBurden();

    /**
     * @brief Sends GM page/help request
     * @param ticket Help message
     * @return True if sent
     */
    auto pageGM(const std::string &ticket) -> bool override;

    /**
     * @brief Sends character appearance to client
     * @param id Character ID
     * @param appearance Appearance command
     * @param always Force send even if already visible
     */
    void sendCharAppearance(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &appearance, bool always);

    /**
     * @brief Removes character from client view
     * @param id Character ID
     * @param removechar Remove command
     */
    void sendCharRemove(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &removechar);

    std::unique_ptr<LongTimeAction> ltAction; ///< Current long-time action

    // Long-time action management
    /**
     * @brief Starts long-time action with UI feedback
     * @param wait Time to completion (1/10 seconds)
     * @param ani Animation ID
     * @param redoani Animation repeat interval
     * @param sound Sound ID
     * @param redosound Sound repeat interval
     */
    void startAction(unsigned short int wait, unsigned short int ani = 0, unsigned short int redoani = 0,
                     unsigned short int sound = 0, unsigned short int redosound = 0) override;

    /**
     * @brief Aborts current action
     */
    void abortAction() override;

    /**
     * @brief Marks action as successful
     */
    void successAction() override;

    /**
     * @brief Handles action disturbance
     * @param disturber Character that disturbed action
     */
    void actionDisturbed(Character *disturber) override;

    /**
     * @brief Changes action source to character
     * @param cc Source character
     * @note Lua: [:changeSource]
     */
    void changeSource(Character *cc) override;

    /**
     * @brief Changes action source to item
     * @param sI Source item
     * @note Lua: [:changeSource]
     */
    void changeSource(const ScriptItem &sI) override;

    /**
     * @brief Changes action source to position
     * @param pos Source position
     * @note Lua: [:changeSource]
     */
    void changeSource(const position &pos) override;

    /**
     * @brief Changes action source to speech and aborts non-talk actions
     * @param text Speech text
     * @param talkType Talk type
     * @note Lua: [:changeSource]
     */
    void changeSource(const std::string &text, talk_type talkType) override;

    /**
     * @brief Clears action source
     * @note Lua: [:changeSource]
     */
    void changeSource() override;

    /**
     * @brief Gets idle time since last action
     * @return Seconds idle
     * @note Lua: [:idleTime]
     */
    auto idleTime() const -> uint32_t override;

    /**
     * @brief Sends book to client
     * @param bookID Book ID
     * @note Lua: [:sendBook]
     */
    void sendBook(uint16_t bookID) override;

    /**
     * @brief Sends character description to client
     * @param id Character ID
     * @param desc Description text
     * @note Lua: [:sendCharDescription]
     */
    void sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc) override;

    // Constructors and initialization
    /**
     * @brief Constructs player with network connection
     * @param newConnection Client connection
     * @throws LogoutException if authentication/loading fails
     */
    explicit Player(std::shared_ptr<NetInterface> newConnection);

    /**
     * @brief Default constructor for testing
     */
    Player() = default;

    /**
     * @brief Validates username and password
     * @throws LogoutException if invalid
     */
    void check_logindata();

    /**
     * @brief Checks if player has GM permission
     * @param right Permission to check
     * @return True if has permission
     */
    auto hasGMRight(gm_rights right) const -> bool;

    /**
     * @brief Saves player to database
     * @return True if successful
     */
    auto save() noexcept -> bool;

    /**
     * @brief Loads player from database
     * @return True if successful
     */
    auto load() noexcept -> bool;

    /**
     * @brief Completes login process and spawns player
     * @throws LogoutException if no valid spawn position
     */
    void login();

    /**
     * @brief Loads GM flags from database
     * @return True if successful
     */
    auto loadGMFlags() noexcept -> bool;

    // Map and screen updates
    /**
     * @brief Sends one Z-level area relative to player's position
     * @param zoffs Z-offset from current position
     */
    void sendRelativeArea(Coordinate zoffs);

    /**
     * @brief Sends complete map (all Z-levels)
     */
    void sendFullMap();

    /**
     * @brief Sends complete map stripe in direction
     * @param direction View direction for stripe
     * @param extraStripeForDiagonalMove Send additional stripe for diagonal movement
     */
    void sendDirStripe(viewdir direction, bool extraStripeForDiagonalMove);

    /**
     * @brief Sends step stripes after movement
     * @param dir Movement direction
     */
    void sendStepStripes(direction dir);

    /**
     * @brief Sends single field update to client
     * @param pos Field position
     */
    void sendField(const position &pos);

    /**
     * @brief Checks if action is currently running
     * @return True if action active
     */
    auto actionRunning() const -> bool override;

    /**
     * @brief Increases poison value and updates client
     * @param value Amount to increase
     */
    void increasePoisonValue(int value) override;

    /**
     * @brief Gets minimum action points
     * @return Min AP
     */
    auto getMinActionPoints() const -> int override;

    /**
     * @brief Gets maximum action points
     * @return Max AP
     */
    auto getMaxActionPoints() const -> int override;

    /**
     * @brief Gets minimum fight points
     * @return Min FP
     */
    auto getMinFightPoints() const -> int override;

    /**
     * @brief Gets maximum fight points
     * @return Max FP
     */
    auto getMaxFightPoints() const -> int override;

    // Showcase (container UI) management
    /**
     * @brief Opens container in showcase UI
     * @param container Container to display
     * @param item Item representing container
     * @param carry True if container is carried
     */
    void openShowcase(Container *container, const ScriptItem &item, bool carry);

    /**
     * @brief Updates showcase display
     * @param container Container to update
     */
    void updateShowcase(Container *container) const;

    /**
     * @brief Updates single slot in showcase
     * @param container Container being updated
     * @param slot Slot index to update
     */
    void updateShowcaseSlot(Container *container, TYPE_OF_CONTAINERSLOTS slot) const;

    /**
     * @brief Checks if showcase ID is open
     * @param showcase Showcase ID
     * @return True if open
     */
    auto isShowcaseOpen(uint8_t showcase) const -> bool;

    /**
     * @brief Checks if container has open showcase
     * @param container Container to check
     * @return True if open
     */
    auto isShowcaseOpen(Container *container) const -> bool;

    /**
     * @brief Checks if showcase is in inventory
     * @param showcase Showcase ID
     * @return True if in inventory (not on ground)
     */
    auto isShowcaseInInventory(uint8_t showcase) const -> bool;

    /**
     * @brief Gets showcase ID for container
     * @param container Container to find
     * @return Showcase ID (255 if not found)
     */
    auto getShowcaseId(Container *container) const -> uint8_t;

    /**
     * @brief Gets container for showcase ID
     * @param showcase Showcase ID
     * @return Container pointer (nullptr if not found)
     */
    auto getShowcaseContainer(uint8_t showcase) const -> Container *;

    /**
     * @brief Closes showcase by ID
     * @param showcase Showcase ID
     */
    void closeShowcase(uint8_t showcase);

    /**
     * @brief Closes showcase for container
     * @param container Container to close
     */
    void closeShowcase(Container *container);

    /**
     * @brief Closes showcases when player moves
     */
    void closeOnMove();

    /**
     * @brief Closes all showcases of ground containers
     */
    void closeAllShowcasesOfMapContainers();

    /**
     * @brief Closes all showcases
     */
    void closeAllShowcases();

    /**
     * @brief Looks into nested container in showcase
     * @param showcase Showcase ID
     * @param pos Slot containing nested container
     */
    void lookIntoShowcaseContainer(uint8_t showcase, unsigned char pos);

    /**
     * @brief Looks into backpack
     * @return True if successful
     */
    auto lookIntoBackPack() -> bool;

    /**
     * @brief Looks into container on adjacent field
     * @param dir Direction to container
     * @return True if successful
     */
    auto lookIntoContainerOnField(direction dir) -> bool;

    /**
     * @brief Changes item quality and updates client
     * @param pos Item position
     * @param amount Quality change amount
     */
    void changeQualityAt(unsigned char pos, int amount) override;

    /**
     * @brief Checks if this is a monitoring client
     * @return True if monitoring client
     */
    inline auto isMonitoringClient() const -> bool { return monitoringClient; }

    /**
     * @brief Teaches magic spell to player and updates client
     * @param type Magic school (0-3)
     * @param flag Spell flag
     */
    void teachMagic(unsigned char type, unsigned char flag) override;

    /**
     * @brief Sets active magic school and updates client
     * @param newMagType New magic type
     */
    void setMagicType(magic_type newMagType) override {
        Character::setMagicType(newMagType);
        sendMagicFlags(newMagType);
    }

    /**
     * @brief Gets localized skill name
     * @param s Skill ID
     * @return Skill name in player's language
     */
    auto getSkillName(TYPE_OF_SKILL_ID s) const -> std::string override;

    /**
     * @brief Gets player's preferred language
     * @return Language (German or English)
     */
    auto getPlayerLanguage() const -> Language override;

    /**
     * @brief Sends all visible characters to client
     */
    void sendCharacters();

    /**
     * @brief Sends equipped item data to client
     * @param cpos Body position
     */
    void sendCharacterItemAtPos(unsigned char cpos);

    /**
     * @brief Sends weather update to client
     * @param weather Weather data
     */
    void sendWeather(WeatherStruct weather);

    /**
     * @brief Ages inventory items and updates client
     */
    void ageInventory() override;

    /**
     * @brief Creates items and updates client
     * @param id Item ID
     * @param number Quantity
     * @param quality Quality
     * @param data Script data
     * @return Amount not created
     */
    auto createItem(Item::id_type id, Item::number_type number, Item::quality_type quality,
                    script_data_exchangemap const *data) -> int override;

    /**
     * @brief Processes skill learning
     * @param skill Skill being trained
     * @param actionPoints AP cost
     * @param opponent Difficulty
     */
    void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) override;

    /**
     * @brief Increases skill and updates client
     * @param skill Skill ID
     * @param amount Amount to increase
     * @return New skill level
     */
    auto increaseSkill(TYPE_OF_SKILL_ID skill, int amount) -> int override;

    /**
     * @brief Deletes all skills and updates client
     */
    void deleteAllSkills() override;

    /**
     * @brief Erases items from inventory and updates client
     * @param itemid Item ID to erase
     * @param count Amount to erase
     * @param data Script data to match
     * @return Amount not erased
     */
    auto eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr) -> int override;

    /**
     * @brief Increases item count at position and updates client
     * @param pos Item position
     * @param count Amount to increase
     * @return Amount not added
     */
    auto increaseAtPos(unsigned char pos, int count) -> int override;

    /**
     * @brief Creates items at position and updates client
     * @param pos Item position
     * @param newid Item ID
     * @param count Quantity
     * @return Amount not created
     */
    auto createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) -> int override;

    /**
     * @brief Swaps item at position and updates client
     * @param pos Item position
     * @param newid New item ID
     * @param newQuality New quality
     * @return True if successful
     */
    auto swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int newQuality = 0) -> bool override;

    /**
     * @brief Sends backpack contents update to client
     */
    void updateBackPackView();

    /**
     * @brief Sends all skills to client
     */
    void sendAllSkills();

    /**
     * @brief Sends magic flags to client
     * @param type Magic school
     */
    void sendMagicFlags(int type);

    /**
     * @brief Sends single skill update to client
     * @param skill Skill ID
     * @param major Major level
     * @param minor Minor level
     */
    void sendSkill(TYPE_OF_SKILL_ID skill, int major, int minor);

    /**
     * @brief Sets skill and updates client
     * @param skill Skill ID
     * @param major Major level
     * @param minor Minor level
     * @return New major level
     */
    auto setSkill(TYPE_OF_SKILL_ID skill, int major, int minor) -> int override;

    /**
     * @brief Saves base attributes to database
     * @return True if successful
     */
    auto saveBaseAttributes() -> bool override;

    /**
     * @brief Sends attribute update to client
     * @param attribute Attribute to send
     */
    void sendAttrib(Character::attributeIndex attribute);

    /**
     * @brief Handles attribute change and updates client
     * @param attribute Changed attribute
     */
    void handleAttributeChange(Character::attributeIndex attribute) override;

    /**
     * @brief Starts music playback
     * @param title Music track ID
     */
    void startMusic(short int title) override;

    /**
     * @brief Resets to default music
     */
    void defaultMusic() override;

    /**
     * @brief Sends text file contents to client
     * @param filename File name
     * @return True if sent
     */
    auto sendTextInFile(const std::string &filename) -> bool;

    // Player status management
    /**
     * @brief Gets player account status
     * @return Status code (OK, banned, jailed, etc.)
     */
    auto getStatus() const -> unsigned char;

    /**
     * @brief Sets player account status
     * @param status New status
     */
    void setStatus(unsigned char status);

    /**
     * @brief Gets status expiration time
     * @return Time when status resets
     */
    auto getStatusTime() const -> time_t;

    /**
     * @brief Sets status expiration time
     * @param time Expiration time
     */
    void setStatusTime(time_t time);

    /**
     * @brief Gets GM who banned/jailed player
     * @return GM name
     */
    auto getStatusGM() const -> std::string;

    /**
     * @brief Sets GM who banned/jailed player
     * @param gm GM character ID
     */
    void setStatusGM(TYPE_OF_CHARACTER_ID gm);

    /**
     * @brief Gets ban/jail reason
     * @return Reason text
     */
    auto getStatusReason() const -> std::string;

    /**
     * @brief Sets ban/jail reason
     * @param reason Reason text
     */
    void setStatusReason(const std::string &reason);

    // World map turtle graphics
    /**
     * @brief Sets turtle graphics mode active
     * @param tturtleActive True to enable
     */
    void setTurtleActive(bool tturtleActive);

    /**
     * @brief Gets turtle graphics mode state
     * @return True if active
     */
    auto getTurtleActive() const -> bool;

    /**
     * @brief Sets turtle tile type
     * @param tturtletile Tile ID
     */
    void setTurtleTile(unsigned char tturtletile);

    /**
     * @brief Gets turtle tile type
     * @return Tile ID
     */
    auto getTurtleTile() const -> unsigned char;

    // Clipping and admin
    /**
     * @brief Sets clipping (collision detection) state
     * @param tclippingActive True to enable
     */
    void setClippingActive(bool tclippingActive) override;

    /**
     * @brief Gets clipping state
     * @return True if enabled
     */
    auto getClippingActive() const -> bool override;

    /**
     * @brief Sets admin permission flags
     * @param tAdmin Bitflags for permissions
     */
    void setAdmin(uint32_t tAdmin);

    /**
     * @brief Checks if player has admin rights
     * @return True if admin
     */
    auto isAdmin() const -> bool override;

    // Communication
    /**
     * @brief Makes player talk and updates all nearby
     * @param tt Talk type
     * @param message Message text
     */
    void talk(talk_type tt, const std::string &message) override;

    /**
     * @brief Informs player with message
     * @param message Message text
     * @param type Message type
     */
    void inform(const std::string &message, informType type = informServer) const override;

    /**
     * @brief Informs player with language-specific message
     * @param german German text
     * @param english English text
     * @param type Message type
     */
    void inform(const std::string &german, const std::string &english, informType type = informServer) const override;

    /**
     * @brief Turns player to face direction
     * @param dir Direction
     */
    void turn(direction dir) override;

    /**
     * @brief Turns player toward position
     * @param pos Position
     */
    void turn(const position &pos) override;

    /**
     * @brief Receives text from another character
     * @param tt Talk type
     * @param message Message text
     * @param cc Speaking character
     */
    void receiveText(talk_type tt, const std::string &message, Character *cc) override;

    // Player knowledge system
    /**
     * @brief Checks if player knows another player
     * @param player Player to check
     * @return True if knows
     */
    auto knows(Player *player) const -> bool;

    /**
     * @brief Adds player to known players
     * @param player Player to know
     */
    void getToKnow(Player *player);

    /**
     * @brief Introduces another player
     * @param player Player being introduced
     */
    void introducePlayer(Player *player) override;

    /**
     * @brief Assigns custom name to player
     * @param playerId Player ID
     * @param name Custom name
     */
    void namePlayer(TYPE_OF_CHARACTER_ID playerId, const std::string &name);

    /**
     * @brief Gets custom name for player
     * @param player Player to check
     * @return Custom name (empty if none)
     */
    auto getCustomNameOf(Player *player) const -> std::string;

    // Movement
    /**
     * @brief Checks if player can move to field
     * @param field Target field
     * @return True if movement possible
     */
    auto moveToPossible(const map::Field &field) const -> bool override;

    using Character::move;

    /**
     * @brief Moves player with specific mode
     * @param dir Direction
     * @param mode Movement mode (walk/run)
     * @return True if successful
     */
    auto move(direction dir, uint8_t mode) -> bool;

    /**
     * @brief Checks if player is overtaxed by burden
     * @return True if overtaxed
     */
    auto isOvertaxed() -> bool;

    /**
     * @brief Warps player to position with validation
     * @param newPos Target position
     * @return True if successful
     */
    auto Warp(const position &newPos) -> bool override;

    /**
     * @brief Forces warp without validation
     * @param newPos Target position
     * @return True if successful
     */
    auto forceWarp(const position &newPos) -> bool override;

    /**
     * @brief Opens depot container
     * @param item Depot item
     */
    void openDepot(const ScriptItem &item);

    // Quest system
    /**
     * @brief Sets quest progress and updates client
     * @param questid Quest ID
     * @param progress New progress value
     */
    void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress) override;

    /**
     * @brief Sends list of available quests to client
     */
    void sendAvailableQuests();

    /**
     * @brief Sends single quest progress to client
     * @param questId Quest ID
     * @param progress Progress value
     */
    void sendQuestProgress(TYPE_OF_QUEST_ID questId, TYPE_OF_QUESTSTATUS progress);

    /**
     * @brief Sends all quest progress to client
     */
    void sendCompleteQuestProgress();

    /**
     * @brief Gets quest progress
     * @param questid Quest ID
     * @param time Output: time since update
     * @return Quest status
     */
    auto getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const -> TYPE_OF_QUESTSTATUS override;

private:
    /**
     * @brief Handles warp side effects
     */
    void handleWarp();

    static constexpr auto dialogLimit = 100; ///< Maximum concurrent dialogs

    /**
     * @brief Sends dialog request to client
     * @tparam DialogType Dialog class type
     * @tparam DialogCommandType Network command type
     * @param dialog Dialog to send
     */
    template <class DialogType, class DialogCommandType> void requestDialog(DialogType *dialog) {
        if (dialog == nullptr) {
            LuaScript::triggerScriptError("Dialog must not be nil!");
        }

        if (dialogs.size() < dialogLimit) {
            unsigned int dialogId = dialogCounter;

            while (dialogs.find(dialogId) != dialogs.end()) {
                ++dialogId;
            }

            dialogs[dialogId] = std::make_shared<DialogType>(*dialog);
            ServerCommandPointer cmd = std::make_shared<DialogCommandType>(*dialog, dialogId);
            Connection->addCommand(cmd);
        } else {
            inform("ERROR: Unable to open more than 100 dialogs.");
        }
    }

    /**
     * @brief Gets dialog by ID
     * @tparam DialogType Dialog class type
     * @param dialogId Dialog ID
     * @return Shared pointer to dialog (nullptr if not found)
     */
    template <class DialogType> auto getDialog(unsigned int dialogId) const -> std::shared_ptr<DialogType> {
        try {
            return std::dynamic_pointer_cast<DialogType>(dialogs.at(dialogId));
        } catch (std::out_of_range &e) {
            return std::shared_ptr<DialogType>();
        }
    }

public:
    // Dialog handlers
    /**
     * @brief Requests input dialog from player
     * @param inputDialog Dialog to display
     */
    void requestInputDialog(InputDialog *inputDialog) override;

    /**
     * @brief Processes input dialog response
     * @param dialogId Dialog ID
     * @param success True if confirmed
     * @param input User input text
     */
    void executeInputDialog(unsigned int dialogId, bool success, const std::string &input);

    /**
     * @brief Requests message dialog
     * @param messageDialog Dialog to display
     */
    void requestMessageDialog(MessageDialog *messageDialog) override;

    /**
     * @brief Processes message dialog close
     * @param dialogId Dialog ID
     */
    void executeMessageDialog(unsigned int dialogId);

    /**
     * @brief Requests merchant trade dialog
     * @param merchantDialog Dialog to display
     */
    void requestMerchantDialog(MerchantDialog *merchantDialog) override;

    /**
     * @brief Handles merchant dialog cancellation
     * @param dialogId Dialog ID
     */
    void executeMerchantDialogAbort(unsigned int dialogId);

    /**
     * @brief Handles merchant buy transaction
     * @param dialogId Dialog ID
     * @param index Item index in merchant's list
     * @param amount Quantity to buy
     */
    void executeMerchantDialogBuy(unsigned int dialogId, MerchantDialog::index_type index,
                                  Item::number_type amount) const;

    /**
     * @brief Handles merchant sell transaction
     * @param dialogId Dialog ID
     * @param location Item location
     * @param slot Container slot
     * @param amount Quantity to sell
     */
    void executeMerchantDialogSell(unsigned int dialogId, uint8_t location, TYPE_OF_CONTAINERSLOTS slot,
                                   Item::number_type amount);

    /**
     * @brief Handles merchant item examination
     * @param dialogId Dialog ID
     * @param list List type
     * @param slot Item slot
     */
    void executeMerchantDialogLookAt(unsigned int dialogId, uint8_t list, uint8_t slot);

    /**
     * @brief Requests selection dialog
     * @param selectionDialog Dialog to display
     */
    void requestSelectionDialog(SelectionDialog *selectionDialog) override;

    /**
     * @brief Processes selection dialog response
     * @param dialogId Dialog ID
     * @param success True if confirmed
     * @param index Selected index
     */
    void executeSelectionDialog(unsigned int dialogId, bool success, SelectionDialog::index_type index);

    /**
     * @brief Requests crafting dialog
     * @param craftingDialog Dialog to display
     */
    void requestCraftingDialog(CraftingDialog *craftingDialog) override;

    /**
     * @brief Handles crafting dialog cancellation
     * @param dialogId Dialog ID
     */
    void executeCraftingDialogAbort(unsigned int dialogId);

    /**
     * @brief Handles crafting request
     * @param dialogId Dialog ID
     * @param craftIndex Recipe index
     * @param craftAmount Quantity to craft
     */
    void executeCraftingDialogCraft(unsigned int dialogId, uint8_t craftIndex, uint8_t craftAmount);

    /**
     * @brief Handles crafting completion
     * @param dialogId Dialog ID
     */
    void executeCraftingDialogCraftingComplete(unsigned int dialogId);

    /**
     * @brief Handles crafting abortion
     * @param dialogId Dialog ID
     */
    void executeCraftingDialogCraftingAborted(unsigned int dialogId);

    /**
     * @brief Handles craftable item examination
     * @param dialogId Dialog ID
     * @param craftIndex Recipe index
     */
    void executeCraftingDialogLookAtCraftable(unsigned int dialogId, uint8_t craftIndex);

    /**
     * @brief Handles crafting ingredient examination
     * @param dialogId Dialog ID
     * @param craftIndex Recipe index
     * @param craftIngredient Ingredient index
     */
    void executeCraftingDialogLookAtIngredient(unsigned int dialogId, uint8_t craftIndex, uint8_t craftIngredient);

    /**
     * @brief Provides look-at for crafting result
     * @param dialogId Dialog ID
     * @param lookAt Look-at data to populate
     */
    void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt) override;

    /**
     * @brief Provides look-at for crafting ingredient
     * @param dialogId Dialog ID
     * @param lookAt Look-at data to populate
     */
    void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt) override;

    /**
     * @brief Invalidates all open dialogs
     */
    void invalidateDialogs();

    /**
     * @brief Closes dialogs when player moves
     */
    void closeDialogsOnMove();

    /**
     * @brief Logs admin action
     * @param message Log message
     */
    void logAdmin(const std::string &message) override;

private:
    /**
     * @brief Starts crafting action sequence
     * @param stillToCraft Remaining items to craft
     * @param craftingTime Time per item
     * @param sfx Sound effect ID
     * @param sfxDuration Sound duration
     * @param dialogId Associated dialog ID
     */
    void startCrafting(uint8_t stillToCraft, uint16_t craftingTime, uint16_t sfx, uint16_t sfxDuration,
                       uint32_t dialogId);

    Language _player_language{};          ///< Player's preferred language
    unsigned char status{};               ///< Account status (OK, banned, jailed, etc.)
    time_t statustime{};                  ///< Status expiration time
    TYPE_OF_CHARACTER_ID statusgm{};      ///< GM who set status
    std::string statusreason;             ///< Status reason text

    bool turtleActive{};                  ///< Turtle graphics mode enabled
    unsigned char turtletile{};           ///< Turtle tile type

    bool clippingActive = true;           ///< Collision detection enabled
    uint32_t admin{};                     ///< Admin permission bitflags
    bool questWriteLock{};                ///< Quest write lock

    LoadLevel loadLevel = LoadLevel::unburdened; ///< Current burden level
    bool monitoringClient{};              ///< Is monitoring client connection

    const uint8_t BACKPACK_SHOWCASE = 0;  ///< Backpack showcase ID
    uint8_t showcaseCounter{};            ///< Next showcase ID

    using ShowcaseMap = std::unordered_map<uint8_t, std::unique_ptr<Showcase>>;
    ShowcaseMap showcases;                ///< Open container showcases

    unsigned int dialogCounter{};         ///< Next dialog ID

    using DialogMap = std::unordered_map<unsigned int, std::shared_ptr<Dialog>>;
    DialogMap dialogs;                    ///< Open dialogs

    using QuestStatusTimePair = std::pair<TYPE_OF_QUESTSTATUS, int>;
    using QuestMap = std::unordered_map<TYPE_OF_QUEST_ID, QuestStatusTimePair>;
    QuestMap quests;                      ///< Quest progress tracking
};

#endif
