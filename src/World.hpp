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
#include "MilTimer.hpp"
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

#include <list>
#include <memory>
#include <sys/timeb.h>
#include <unordered_map>

class Player;
class Monster;
class NPC;
class LuaScript;

/**
 * a class for holding gm or player commands
 */
using CommandType = std::function<bool(World *, Player *, const std::string &)>;

/**
 * a struct for holding Weather informations
 */
struct WeatherStruct {
    char cloud_density = defaultCloudDensity;                   // cloud density in percent
    char fog_density = defaultFogDensity;                       // fog density in percent
    char wind_dir = defaultWindDirection;                       // wind direction -100 blowing from east to 100 ???
    char gust_strength = defaultGustStrength;                   // wind strength in percent
    char percipitation_strength = defaultPercipitationStrength; // percipitation strength in percent
    char per_type = defaultPercipitationType;                   // type of percipitation: 0 == rain, 1 == snow
    char thunderstorm = defaultThunderstormIntensity;           // thunderstorm intensity in percent
    char temperature = defaultTemperature;                      // current temperature in °C

    static constexpr char defaultCloudDensity = 20;
    static constexpr char defaultFogDensity = 0;
    static constexpr char defaultWindDirection = 50;
    static constexpr char defaultGustStrength = 10;
    static constexpr char defaultPercipitationStrength = 0;
    static constexpr char defaultPercipitationType = 0;
    static constexpr char defaultThunderstormIntensity = 0;
    static constexpr char defaultTemperature = 20;
};

struct BlockingObject {
    enum BlockingType { BT_ITEM = 0, BT_CHARACTER = 1, BT_NONE = 2 };
    BlockingType blockingType;
    Character *blockingChar;
    ScriptItem blockingItem;
    BlockingObject() {
        blockingType = BT_NONE;
        blockingChar = nullptr;
    }
};

/**
 *this class represents the gameworld
 *this class contains the world of the gameserver
 *here is the main point for monsters, player, maps and npc's
 */
class World : public WorldScriptInterface {
public:
    auto operator=(const World &) -> World & = delete;
    World(const World &) = delete;
    World(World &&) = delete;
    auto operator=(World &&) -> World & = delete;

    /////////////////////////
    NewClientView clientview;
    /////////////////////////

    /**
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     */
    using PLAYERVECTOR = CharacterContainer<Player>;

    /**
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     */
    using MONSTERVECTOR = CharacterContainer<Monster>;

    /**
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     */
    using NPCVECTOR = CharacterContainer<NPC>;

    /**
     *holds all active player on the world
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     */
    PLAYERVECTOR Players;

    /**
     *sets a new tile on the map
     */
    void setNextTile(Player *cp, unsigned char tilenumber);

    /**
     *holds all monsters on the world
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     **/
    MONSTERVECTOR Monsters;

    /**
     * new Monsters which should be spawned so the server didn't crash on creating monsters from monsters
     */
    std::vector<Monster *> newMonsters;

    /**
     *holds all npc's on the world
     *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one
     *HARVECTOR
     **/
    NPCVECTOR Npc;

    /**
     *npcs which should be deleted
     */
    std::vector<TYPE_OF_CHARACTER_ID> LostNpcs;

    /**
     * holds the monitoring clients on the World
     */
    std::unique_ptr<MonitoringClients> monitoringClientList = nullptr;

    timeb now{}; /**< current time of the server used in @see turntheworld() **/

    unsigned long int timeStart;
    unsigned long int usedAP;

    short int ap{}; /**< actionpoints since the last loop call **/

    ClockBasedScheduler<std::chrono::steady_clock> scheduler;

    WeatherStruct weather; /**< a struct to the weather @see WeatherStruct */

    /**
     *inline function for setting the current weather and sending the data to all players online
     */
    inline void setWeather(WeatherStruct nWeather) {
        weather = nWeather;
        sendWeatherToAllPlayers();
    }

    //! parse GMCommands of the Form !<string1> <string2> and process them
    auto parseGMCommands(Player *user, const std::string &text) -> bool;
    //! parse PlayerCommands of the Form !<string1> <string2> and process them
    auto parsePlayerCommands(Player *player, const std::string &text) -> bool;

    /**
     *the current script which is called
     */
    LuaScript *currentScript;

    ~World() override = default;

    /**
     *main loop for the world
     */
    void turntheworld();

    /**
     *checks all player aktions and makes them active
     */
    void checkPlayers();

    void invalidatePlayerDialogs() const;

    /**
     *checks all actions of the monsters and updates them
     */
    void checkMonsters();

    /**
     *checks all actions of the NPC's and updates them
     */
    void checkNPC();

    /**
     *init method for npc's
     *loads the npc's from the db and sets them on the map
     */
    void initNPC();

    /**
     *method for creating and starting the scheduler
     */
    void initScheduler();

    /**
     * Method for returning the current illarion time
     *
     *@param timeType <"year"|"month"|"day"|"hour"|"minute"|"second">
     *@return an int which is the current illarion time from the type
     */
    auto getTime(const std::string &timeType) const -> int override;

    /**
     *checks the command list of one player and put them into practize
     *@param cp the player which buffer is currently processed
     */
    void workout_CommandBuffer(Player *&cp);

    void allowLogin(bool allow) { _is_login_allowed = allow; }
    auto isLoginAllowed() const -> bool { return _is_login_allowed; }

    void enableSpawn(bool enable) { _is_spawn_enabled = enable; }
    auto isSpawnEnabled() const -> bool { return _is_spawn_enabled; }

    static auto create() -> World *;
    static auto get() -> World *;

    /**============ WorldIMPLTools.cpp ==================*/

    /**
     * checks the list of LostNpcs and deletes all the npcs which are in this and the normal npc list
     * @todo is a bad hack to resolve the problem that segfaults the server on selling cows, should be changed
     * with a better char handling from the server side (smart pointers, one list for all char types).
     */
    void deleteAllLostNPC();

    inline auto getCurrentScript() const -> LuaScript * { return currentScript; }
    inline void setCurrentScript(LuaScript *script) { currentScript = script; }

    /**
     *saves all online players a table in the db
     */
    void updatePlayerList() const;

    /**
     * finds all warpfields in a given range
     * can be found in WorldIMPLTools.cpp
     * @param pos the position from which the warpfields should be found
     * @param range the roung around pos which should be searched for warpfields
     * @param call by reference, returns a hashmap with the warpfields which where found
     */
    auto findWarpFieldsInRange(const position &pos, short int range, std::vector<position> &warppositions) -> bool;

    auto blockingLineOfSight(const position &startingpos, const position &endingpos) const
            -> std::list<BlockingObject> override;

    auto findTargetsInSight(const position &pos, uint8_t range, std::vector<Character *> &ret,
                            Character::face_to direction) const -> bool;
    auto findCharacterOnField(const position &pos) const -> Character *;
    auto findPlayerOnField(const position &pos) const -> Player *;

    /**
     * searches for a special character
     * can be found in WorldIMPLTools.cpp
     * looks into all three vectors ( player, monster, npc )  for a character with the given id
     * @param id the id of the character which should be found
     * @return a pointer to the character, nullptr if the character wasn't found
     * @todo has to be changed for only one charactervetor
     */
    virtual auto findCharacter(TYPE_OF_CHARACTER_ID id) -> Character *;

    /**
     *deletes all monsters and npcs from the map and emptys the lists
     * can be found in WorldIMPLTools.cpp
     *@see World::Monsters @see World::Npc
     */
    void takeMonsterAndNPCFromMap();

    /**
     * a player attacks
     * @param cp the player who is attacking
     */
    auto characterAttacks(Character *cp) const -> bool;

    auto killMonster(TYPE_OF_CHARACTER_ID id) -> bool;

    auto fieldAt(const position &pos) -> map::Field & override;
    auto fieldAt(const position &pos) const -> const map::Field & override;
    auto fieldAtOrBelow(position &pos) -> map::Field &;
    auto walkableFieldNear(const position &pos) -> map::Field &;
    void makePersistentAt(const position &pos) override;
    void removePersistenceAt(const position &pos) override;
    auto isPersistentAt(const position &pos) const -> bool override;

    static auto getItemAttrib(const std::string &s, TYPE_OF_ITEM_ID ItemID) -> int;

    void Load();
    void Save() const;
    void import();
    auto createMap(const std::string &name, const position &origin, uint16_t width, uint16_t height, uint16_t tile)
            -> bool {
        return maps.createMap(name, origin, width, height, tile);
    }

    void sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, const position &pos) const;

    void sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health) const;

    /**============in WorldIMPLCharacterMoves.cpp==================*/

    /**
     *sends all character to a client which the player sees
     *
     *@param cp pointer to the player which should recive the data
     *@param sendSpin if true the direction of the chars is also sendet
     */
    void sendAllVisibleCharactersToPlayer(Player *cp, bool sendSpin);

    /**
     *adds a warpfield to a specific groundtile
     *
     *changes also the groundtile and adds a item on it
     *@param where the starting position of the warpfield (from where is the char warped)
     *@param target the ending position of the warpfield (to where is the char warped)
     *@param starttilenr the id of the new groundtile which should be set. 0 for no changes
     *@param startitemnr the id of a new item which should be placed at the start position. 0 for no changes
     *@return true if the adding of the warpfield was succesfull otherwise false
     */
    auto addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                      Item::id_type startitemnr) -> bool;

    /**
     *adds a two way warpfield to the map
     *
     *changes also the groundtile/item of the startpos and endinpos and adds a way back
     *
     *@param where the starting position of the warpfield (from where is the char warped)
     *@param target the ending position of the warpfield (to where is the char warped)
     *@param starttilenr the id of the new groundtile at the starting position. 0 for no changes
     *@param startitemnr the id of a new item which should be added at the starting pos. 0 for no changes
     *@param targettilenr the id of the new groundtile at the target position. 0 for no changes
     *@param targetitemnr the id of a new item which should be added at the target pos. 0 for no changes
     *@return true if the adding of the warpfield was succesfull otherwise false
     */
    auto addWarpField(const position &where, const position &target, unsigned short int starttilenr,
                      Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr) -> bool;

    /**
     *removes a warpfield at a given position
     *
     *@param where the position of the warpfield which should be removed
     *@return true if the removing was succesfull otherwise false
     */
    auto removeWarpField(const position &pos) -> bool;

    /**
     * looks for the targetposition of a Warpfield
     *
     *@param where the starting coordinate of the warpfield
     *@param target call by reference, returns the target position of the warpfield
     *@return true if the warpfield was found otherwise false
     */
    // bool findWarpFieldTarget( position where, position &target );

    /**
     *calls a triggerscript if a character is moving to a triggerfield or away from one
     *
     *@param cc the character which is moving
     *@param true if the char is moving to the field, false if he is moving away from the field
     */
    static void triggerFieldMove(Character *cc, bool moveto);

    /**
     * update the character container about the position change
     *@param cc the character which is moving
     *@param from the old position
     *@param to the new position
     */
    void moveTo(Character *cc, const position &to);

    /**
     *moves a player to the direction d
     *@param cp pointer to the player which should be moved
     *@param d the direction to he is moving:
     *
     *<ul>
     *<li> 0 - North</li>
     *<li> 1 - Northwest (not used)</li>
     *<li> 2 - West </li>
     *<li> 3 - Southwest (not used) </li>
     *<li> 4 - South </li>
     *<li> 5 - Southeast </li>
     *<li> 6 - East </li>
     *<li> 7 - Northeast </li>
     *<li> 8 - up </li>
     *<li> 9 - down </li>
     *
     *@param walkcost call by reference, returns the walking cost of the move
     *@param active true if the walking should be animated otherwise false
     *@param pushSelf true if the player was pushing himself otherwise false
     *@return true if the movement was succesful otherwise false
     *@todo Player->move was implemented this function can be deleted
     **/
    auto pushPlayer(Player *cp, unsigned char d, short int &walkcost) -> bool;

    static void checkFieldAfterMove(Character *character, const map::Field &field);
    void sendPassiveMoveToAllVisiblePlayers(Character *ccp) const;
    void sendSpinToAllVisiblePlayers(Character *cc) const;
    void sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char moveType,
                                              TYPE_OF_WALKINGCOST duration) const;
    void sendCharacterMoveToAllVisibleChars(Character *cc, TYPE_OF_WALKINGCOST duration) const;
    void sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char moveType) const;
    template <class T> void sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin);

    void lookAtMapItem(Player *player, const position &pos, uint8_t stackPos);

private:
    static void lookAtTile(Player *cp, unsigned short int tile, const position &pos);

public:
    //! sendet an den Spieler den Namen des Item an einer Position im showcase
    // \param cp der Spieler der benachrichtigt werden soll
    // \param showcase der showcase in dem das Item liegt
    // \param position die Position des Item im showcase
    void lookAtShowcaseItem(Player *cp, uint8_t showcase, unsigned char position);

    //! sendet an den Spieler den Namen des Item an einer Position im Inventory
    // \param cp der Spieler der benachrichtigt werden soll
    // \param position die Position des Item im Inventory
    void lookAtInventoryItem(Player *cp, unsigned char position);

    void makeSoundForAllPlayersInRange(const position &pos, int radius, unsigned short int sound) const;
    void makeGFXForAllPlayersInRange(const position &pos, int radius, unsigned short int gfx) const;

    //! send a message to all chars near pos
    //! \param message what the char says
    //! \param tt did he shout/talk/whisper?
    //! \param the talking char
    void sendMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Character *cc) const;
    void sendMessageToAllCharsInRange(const std::string &german, const std::string &english, Character::talk_type tt,
                                      Character *cc) const;

    /**
     *Sends a message in a specific language to all chars in range with this language
     *@param message what the chars says
     *@param tt shout/talk/whisper
     *@param lang the language to which players the message is send
     *@param cc the talking char
     *@todo remove deprecated function
     */
    void sendLanguageMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Language lang,
                                              Character *cc) const;

    void sendMessageToAllPlayers(const std::string &message) const;
    void broadcast(const std::string &german, const std::string &english) const override;

    void sendMessageToAdmin(const std::string &message) const;

    static auto languageNumberToSkillName(int languageId) -> std::string;
    static auto languagePrefix(int languageId) -> std::string;

    static auto getTalkRange(Character::talk_type tt) -> Range;

    void introduceMyself(Player *cp) const;
    static void forceIntroducePlayer(Player *cp, Player *admin);
    void ForceIntroduce(Player *player, const std::string &text) const;
    void ForceIntroduceAll(Player *player) const;

    void sendWeather(Player *cp) const;
    void sendWeatherToAllPlayers();

    void sendIGTimeToAllPlayers();
    void sendIGTime(Player *cp) const;

    ////////// in WorldIMPLAdmin.cpp /////////////

    // Sendet eine Message an alle Spieler
    // param cp: der Spieler der die Message sendet
    // param message: die nachricht die geschickt wird
    void broadcast_command(Player *cp, const std::string &message) const;

    // Kickt alle Spieler aus dem Spiel
    // param cp: der Spieler welche alle Spieler kickt
    void kickall_command(Player *cp);

    // Kickt einen einzelnen Spieler aus dem Spiel
    // param cp: der Spieler welcher jemanden kickt
    // param player: der Spieler welcher gekickt wird
    void kickplayer_command(Player *cp, const std::string &player) const;

    // Zeigt dem Spieler nutzerdaten (IP's) an
    static void showIPS_Command(Player *cp);

    /**
     *creates an item in the inventory of the gm
     */
    static void create_command(Player *cp, const std::string &itemid);

    void spawn_command(Player *cp, const std::string &monsterId);

    /**
     * activates a specific logfile
     */
    void logon_command(Player *cp, const std::string &log);

    /**
     * deactivates a specific logfile
     */
    void logoff_command(Player *cp, const std::string &log);

    //! teleportiert einen Player zu einem anderen
    // \param cp der zu teleportierende Player
    // \param ts der Name des Ziel - Player
    void teleportPlayerToOther(Player *player, const std::string &target) const;

    //! toetet alles auf der Karte befindlichen Monster
    void kill_command(Player *cp) const;

    //! resambles the former #r command, reloads all tables, definitions and scripts
    // \param cp is the GM performing this full reload
    void reload_command(Player *cp);

    //! substitutes #j <name>, jump to a player of a given name
    // \param cp is the jumping GM
    // \param ts name of the player to jump to
    void jumpto_command(Player *cp, const std::string &player) const;

    //! resambles the former #mapsave command, saves the map
    // \param cp the corresponding GM with correct rights who initiates this mapsave
    void save_command(Player *cp);

    //! Macht spieler cp unsichtbar
    void makeInvisible(Player *cp) const;

    void talkto_command(Player *player, const std::string &text) const;

    //! Macht spieler cp sichtbar
    void makeVisible(Player *cp) const;

    //! wirft alle Player aus dem Spiel
    void forceLogoutOfAllPlayers();

    //! wirft einen aktiven Player aus dem Spiel
    // \param name der Spieler der herausgeworfen werden soll
    // \return true falls der Player gefunden wurde, false sonst
    auto forceLogoutOfPlayer(const std::string &name) const -> bool;

    //! sendet einem Admin die Daten aller aktiven Player
    // \param admin der Admin an den die Daten gesandt werden sollen
    static void sendAdminAllPlayerData(Player *admin);

    // ! Server side implemented !warp_to x y z
    static void warpto_command(Player *player, const std::string &text);

    // ! Server side implemented !summon Player
    void summon_command(Player *player, const std::string &text) const;

    // ! relaods only the definition from the db no Monsterspawns and no NPC's are loaded.
    auto reload_defs(Player *cp) const -> bool;

    // ! adds Warpfields to map from textfile
    auto importWarpFields(Player *cp, const std::string &filename) -> bool;

    // ! Deletes a Warpfield
    void removeTeleporter(Player *cp, const std::string &text);

    // ! Shows a list of Warpfields
    void showWarpFieldsInRange(Player *cp, const std::string &text);
    //////////////////////////////////////////////////////////////////

    //////////// in WorldIMPLItemMoves.cpp ////////////////

    Item g_item;
    Container *g_cont{};

    auto takeItemFromMap(Character *cc, const position &itemPosition) -> bool;
    auto putItemOnMap(Character *cc, const position &itemPosition) -> bool;
    auto putItemAlwaysOnMap(Character *cc, const position &itemPosition) -> bool;
    auto takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count) -> bool;
    auto putItemOnInvPos(Character *cc, unsigned char pos) -> bool;
    auto takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count) -> bool;
    auto putItemOnInvPos(Player *cc, unsigned char pos) -> bool;

    void closeShowcaseForOthers(Player *target, Container *moved) const;
    void closeShowcaseIfNotInRange(Container *moved, const position &showcasePosition) const;

    auto takeItemFromShowcase(Player *cc, uint8_t showcase, unsigned char pos, Item::number_type count) -> bool;
    auto putItemInShowcase(Player *cc, uint8_t showcase, TYPE_OF_CONTAINERSLOTS pos) -> bool;
    void checkField(const map::Field &field, const position &itemPosition) const;

    auto moveItemFromMapToMap(Player *cp, const position &oldPosition, const position &newPosition,
                              Item::number_type count) -> bool;
    void moveItemFromMapIntoShowcase(Player *cp, const position &sourcePosition, uint8_t showcase,
                                     unsigned char showcaseSlot, Item::number_type count);
    void moveItemFromMapToPlayer(Player *cp, const position &sourcePosition, unsigned char inventorySlot,
                                 Item::number_type count);
    void moveItemBetweenShowcases(Player *cp, uint8_t source, unsigned char pos, uint8_t dest, unsigned char pos2,
                                  Item::number_type count);
    void dropItemFromShowcaseOnMap(Player *cp, uint8_t showcase, unsigned char pos, const position &newPosition,
                                   Item::number_type count);
    void moveItemFromShowcaseToPlayer(Player *cp, uint8_t showcase, unsigned char pos, unsigned char cpos,
                                      Item::number_type count);
    void moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count);
    void dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, const position &newPosition, Item::number_type count);
    void moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, uint8_t showcase, unsigned char pos,
                                        Item::number_type count);
    auto pickUpItemFromMap(Player *cp, const position &itemPosition) -> bool;
    void pickUpAllItemsFromMap(Player *cp);

    void sendRemoveItemFromMapToAllVisibleCharacters(const position &itemPosition) const;
    void sendPutItemOnMapToAllVisibleCharacters(const position &itemPosition, const Item &it) const;
    void sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, const position &itemPosition, const Item &it) const;
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot, Container *moved) const;
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot) const;

    //////////////////////////////////////In WorldIMPLScriptHelp.cpp//////////////////////////////////////////

    auto deleteNPC(unsigned int npcid) -> bool override;

    auto createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos, Character::sex_type sex,
                          const std::string &scriptname) -> bool override;

    auto getPlayersOnline() const -> std::vector<Player *> override;

    auto getNPCS() const -> std::vector<NPC *> override;

    auto getCharactersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Character *> override;
    auto getPlayersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Player *> override;
    auto getMonstersInRangeOf(const position &pos, uint8_t radius) const -> std::vector<Monster *> override;
    auto getNPCSInRangeOf(const position &pos, uint8_t radius) const -> std::vector<NPC *> override;

    auto getItemStats(const ScriptItem &item) const -> ItemStruct override;
    auto getItemStatsFromId(TYPE_OF_ITEM_ID id) const -> ItemStruct override;
    void changeQuality(ScriptItem item, short int amount) override;
    virtual void itemInform(Character *user, const ScriptItem &item, const ItemLookAt &lookAt);
    auto getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) const -> std::string override;
    auto changeItem(ScriptItem item) -> bool override;

    auto isCharacterOnField(const position &pos) const -> bool override;
    auto getCharacterOnField(const position &pos) const -> character_ptr override;

    auto erase(ScriptItem item, int amount) -> bool override;
    auto swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, unsigned short int newQuality = 0) -> bool override;
    auto increase(ScriptItem item, short int count) -> bool override;
    auto createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, const position &pos, bool always, int quality,
                      script_data_exchangemap const *data) -> ScriptItem override;

    auto createFromItem(const ScriptItem &item, const position &pos, bool always) -> bool override;

    auto createMonster(unsigned short id, const position &pos, short movepoints) -> character_ptr override;

    void gfx(unsigned short int gfxid, const position &pos) const override;
    void makeSound(unsigned short int soundid, const position &pos) const override;

    auto isItemOnField(const position &pos) -> bool override;
    auto getItemOnField(const position &pos) -> ScriptItem override;

    void changeTile(short int tileid, const position &pos) override;

    //! Sicherer CreateArea command, prft erst ab ob er eine vorhandene Map berschreibt.
    //\tileid: standard Tile
    //\pos: begin der neuen Karte
    //\height: Hoehe der neuen Karte
    //\width: breite der neuen Karte
    //\return true wenn das einfgen klappte, ansonsten false wenns zu berlagerungen kommt.
    auto createSavedArea(uint16_t tile, const position &origin, uint16_t height, uint16_t width) -> bool override;

    auto getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret) -> bool override;
    auto getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret) -> bool override;
    auto getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret) -> bool override;
    auto getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret) -> bool override;

    /**
     *sends a Message to All Monitoring Clients
     *@param msg the message string which should be sended
     *@param id the id of the msg ( 1 are message which displayed in a window 0 basic message)
     */
    void sendMonitoringMessage(const std::string &msg, unsigned char id = 0) const override;

    /**
     * bans a player for the bantime
     * @param cp pointer to the player which should been banned
     * @param bantime in seconds
     * @gmid the id of the gm which has banned the player
     */
    void ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid) const;

    void set_login(Player *player, const std::string &text);

protected:
    World(); // used for testcases
    static World *_self;

private:
    static void logMissingField(const std::string &function, const position &field);

    bool _is_login_allowed = true;
    bool _is_spawn_enabled = true;

    /**
     * the constructor of World, private because of singleton pattern
     *
     *@param dir the main directory of the server
     *@param time_t the starting time of the server
     */
    explicit World(const std::string &dir);

    //! IG day of last turntheworld
    int lastTurnIGDay;

    static constexpr auto minute = 60;
    Timer monstertimer{minute};

    void ageMaps();
    void ageInventory() const;

    std::string scriptDir;

    // spawnplaces...
    std::list<SpawnPoint> SpawnList;

    // initmethod for spawn places...
    auto initRespawns() -> bool;

    using CommandMap = std::map<std::string, CommandType>;
    CommandMap GMCommands;
    CommandMap PlayerCommands;

    // Send player information to GMs
    void who_command(Player *cp, const std::string &tplayer) const;

    // Change tile in front of admin
    void tile_command(Player *cp, const std::string &tile);

    // Turtle tile commands
    static void turtleon_command(Player *cp, const std::string &tile);
    static void turtleoff_command(Player *cp);

    // Clipping on/off command
    static void clippingon_command(Player *cp);
    static void clippingoff_command(Player *cp);

    // Describe to admin tile in front of them
    void what_command(Player *cp);

    // Save all online players
    void playersave_command(Player *cp) const;

    // Create telport warp on current tile to x, y, z
    void teleport_command(Player *cp, const std::string &text);

    // Give help for GM commands
    static void gmhelp_command(Player *cp);

    // Sendet eine Nachricht an alle GM's
    auto gmpage_command(Player *player, const std::string &ticket) const -> bool;

public:
    void ban_command(Player *cp, const std::string &text) const;
    void logGMTicket(Player *player, const std::string &ticket, bool automatic) const;
    void checkPlayerImmediateCommands();
    void addPlayerImmediateActionQueue(Player *player);

private:
    map::WorldMap maps;

    auto getTargetsInRange(const position &pos, int radius) const -> std::vector<Character *>;

    static auto active_language_command(Player *cp, const std::string &language) -> bool;

    // register any GM commands here...
    void InitGMCommands();
    // register any Player commands here...
    void InitPlayerCommands();
    auto executeUserCommand(Player *user, const std::string &input, const CommandMap &commands) -> bool;

    // export maps to mapdir/export
    auto exportMaps(Player *cp) const -> bool;

    void ignoreComments(std::ifstream &inputStream);

    //! reload all tables
    auto reload_tables(Player *cp) -> bool;

    static void version_command(Player *player);

    std::mutex immediatePlayerCommandsMutex;
    std::queue<Player *> immediatePlayerCommands;
};

#endif
