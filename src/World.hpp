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

#ifndef WORLD_HH
#define WORLD_HH

#define LOG_TALK

#include <sys/timeb.h>

#include <memory>
#include <list>
#include <unordered_map>

#include "NewClientView.hpp"
#include "CharacterContainer.hpp"
#include "SpawnPoint.hpp"
#include "TableStructs.hpp"
#include "Character.hpp"
#include "Language.hpp"
#include "Timer.hpp"
#include "MilTimer.hpp"
#include "MonitoringClients.hpp"
#include "Scheduler.hpp"
#include "character_ptr.hpp"
#include "map/WorldMap.hpp"

#include "data/MonsterTable.hpp"
#include "data/MonsterAttackTable.hpp"

class Player;
class Monster;
class NPC;
class LuaScript;

// typedef for gm commands...
/**
* a class for holding gm or player commands
*/
typedef std::function<bool(World *, Player *, const std::string &)> CommandType;

/**
* a struct for holding Weather informations
*/
struct WeatherStruct {
    char cloud_density; /**< how much clouds are on the illarion sky in percent*/
    char fog_density; /**< how much fog in percent*/
    char wind_dir; /**< wind direction -100 blowing from east to 100 */
    char gust_strength; /**< wind strength in percent*/
    char percipitation_strength; /**< percipitation strength in percent*/
    char per_type; /**< what type of percipitation 0 rain 1 snow*/
    char thunderstorm; /**< how much thunderstorm, intensity in percent*/
    char temperature; /**< holds the current temperature in degree celsius*/

    /**
    *small constructor for initializing
    * cloud_density = 20%
    * fog_density = 0%
    * wind_dir = 50
    * gust_strength = 10%
    * percipitation_strength = 0%
    * per_type = 0
    * thunderstorm = 0%
    * temperature = 20C
    * the value from a normal not too cold/warm day
    */
    WeatherStruct() {
        cloud_density = 20;
        fog_density = 0;
        wind_dir = 50;
        gust_strength = 10;
        percipitation_strength = 0;
        per_type = 0;
        thunderstorm = 0;
        temperature = 20;
    }

    /**
    * constructor for initializing weather struct with values
    *
    *@param cd for initializing cloud_density
    *@param fd for intitializing fog_density
    *@param wd for initializing wind_dir
    *@param gs for initializing gust_strength
    *@param ps for initializing percipitation_strength
    *@param pt for initializing per_type
    *@param ts for initializing thunderstorm
    *@param te for initializing temperature
    */
    WeatherStruct(char cd, char fd, char wd, char gs, char ps, char pt,char ts, char te) {
        cloud_density = cd;
        fog_density = fd;
        wind_dir = wd;
        gust_strength = gs;
        percipitation_strength = ps;
        per_type = pt;
        thunderstorm = ts;
        temperature = te;
    }

};

struct BlockingObject {
    enum BlockingType { BT_ITEM = 0, BT_CHARACTER = 1, BT_NONE = 2};
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
class World {

public:
    /////////////////////////
    NewClientView clientview;
    /////////////////////////

    /**
    *a typedef for holding Players
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    typedef CharacterContainer<Player> PLAYERVECTOR;

    /**
    *a typedef for holding monsters
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    typedef CharacterContainer<Monster> MONSTERVECTOR;

    /**
    *a typedef for holding npc's
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    typedef CharacterContainer<NPC> NPCVECTOR;

    /**
    *holds all active player on the world
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    PLAYERVECTOR Players;

    /**
    *sets a new tile on the map
    */
    void setNextTile(Player *cp, unsigned char tilenumber);

    /**
    *holds all monsters on the world
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    **/
    MONSTERVECTOR Monsters;

    /**
    * new Monsters which should be spawned so the server didn't crash on creating monsters from monsters
    */
    std::vector<Monster *> newMonsters;

    /**
    *holds all npc's on the world
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    **/
    NPCVECTOR Npc;

    /**
    *npcs which should be deleted
    */
    std::vector< TYPE_OF_CHARACTER_ID > LostNpcs;

    /**
     * holds the monitoring clients on the World
     */
    std::unique_ptr<MonitoringClients> monitoringClientList = nullptr;

    timeb now; /**< current time of the server used in @see turntheworld() **/

    unsigned long int timeStart;
    unsigned long int usedAP;

    short int ap; /**< actionpoints since the last loop call **/

    ClockBasedScheduler<std::chrono::steady_clock> scheduler;

    WeatherStruct weather;/**< a struct to the weather @see WeatherStruct */

    /**
    *inline function for setting the current weather and sending the data to all players online
    */
    inline void setWeather(WeatherStruct nWeather) {
        weather = nWeather;
        sendWeatherToAllPlayers();
    }

    //! parse GMCommands of the Form !<string1> <string2> and process them
    bool parseGMCommands(Player *user, const std::string &text);
    //! parse PlayerCommands of the Form !<string1> <string2> and process them
    bool parsePlayerCommands(Player *player, const std::string &text);

    /**
    *the current script which is called
    */
    LuaScript *currentScript;

    virtual ~World();

    /**
    *main loop for the world
    */
    void turntheworld();

    /**
    *checks all player aktions and makes them active
    */
    void checkPlayers();

    void invalidatePlayerDialogs();

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
    int getTime(const std::string &timeType);

    /**
    *checks the command list of one player and put them into practize
    *@param cp the player which buffer is currently processed
    */
    void workout_CommandBuffer(Player *&cp);

    void allowLogin(bool allow) {
        _is_login_allowed = allow;
    }
    bool isLoginAllowed() const {
        return _is_login_allowed;
    }

    void enableSpawn(bool enable) {
        _is_spawn_enabled = enable;
    }
    bool isSpawnEnabled() const {
        return _is_spawn_enabled;
    }

    static World *create();
    static World *get();


    /**============ WorldIMPLTools.cpp ==================*/

    /**
    * checks the list of LostNpcs and deletes all the npcs which are in this and the normal npc list
    * @todo is a bad hack to resolve the problem that segfaults the server on selling cows, should be changed
    * with a better char handling from the server side (smart pointers, one list for all char types).
    */
    void deleteAllLostNPC();

    inline LuaScript *getCurrentScript() const {
        return currentScript;
    }
    inline void setCurrentScript(LuaScript *script) {
        currentScript = script;
    }


    /**
    *saves all online players a table in the db
    */
    void updatePlayerList();

    /**
    * finds all warpfields in a given range
    * can be found in WorldIMPLTools.cpp
    * @param pos the position from which the warpfields should be found
    * @param range the roung around pos which should be searched for warpfields
    * @param call by reference, returns a hashmap with the warpfields which where found
    */
    bool findWarpFieldsInRange(const position &pos, short int range, std::vector<position> &warppositions);

    /**
    * returns a list of blocking objects between a startin position and a ending position
    * @param startingpos the starting position of the line of sight
    * @param endingpos the end of the line of sight calculation
    * @return list of all blocking objects between startingpos and endingpos.
    */
    std::list<BlockingObject> LoS(const position &startingpos, const position &endingpos) const;


    bool findTargetsInSight(const position &pos, uint8_t range, std::vector<Character *> &ret, Character::face_to direction) const;
    Character *findCharacterOnField(const position &pos) const;
    Player *findPlayerOnField(const position &pos) const;


    /**
    * searches for a special character
    * can be found in WorldIMPLTools.cpp
    * looks into all three vectors ( player, monster, npc )  for a character with the given id
    * @param id the id of the character which should be found
    * @return a pointer to the character, nullptr if the character wasn't found
    * @todo has to be changed for only one charactervetor
    */
    virtual Character *findCharacter(TYPE_OF_CHARACTER_ID id);

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
    bool characterAttacks(Character *cp);

    bool killMonster(TYPE_OF_CHARACTER_ID id);

    map::Field &fieldAt(const position &pos);
    const map::Field &fieldAt(const position &pos) const;
    map::Field &fieldAtOrBelow(position &pos);
    map::Field &walkableFieldNear(const position &pos);
    void makePersistentAt(const position &pos);
    void removePersistenceAt(const position &pos);
    bool isPersistentAt(const position &pos) const;

    int getItemAttrib(const std::string &s, TYPE_OF_ITEM_ID ItemID);

    /**
    *sends all players in sight of the tiles line the map
    *
    *@param startx the starting x position of the line
    *@param endx the ending x position of the line
    */
    void updatePlayerView(short int startx, short int endx);

    void Load();
    void Save() const;
    void import();
    bool createMap(const std::string &name, const position &origin,
                   uint16_t width, uint16_t height, uint16_t tile) {
        return maps.createMap(name, origin, width, height, tile);
    }



    /**
    *@brief changes one part of the weather and sends the new weather to all players
    *@param value the new value of the weather part
    *@param type the part of the weather which should be changed
    *<ul>
    *<li> "cloud_density" how much clouds are on the sky </li>
    *<li> "fog_densitiy" how much fog </li>
    *<li> "wind_dir" from which direction blows the wind </li>
    *<li> "gust_strength" how strong are the wind gusts </li>
    *<li> "percipitation_strength" how strong is the percipitation </li>
    *<li> "thunderstorm" is there any thunderstorm and how strong is it </li>
    *<li> "temperature" the temperature in C </li>
    */
    void setWeatherPart(const std::string &type, char value);

    void sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, const position &pos);

    void sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health);

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
    bool addWarpField(const position &where, const position &target, unsigned short int starttilenr, Item::id_type startitemnr);

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
    bool addWarpField(const position &where, const position &target, unsigned short int starttilenr, Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr);

    /**
    *removes a warpfield at a given position
    *
    *@param where the position of the warpfield which should be removed
    *@return true if the removing was succesfull otherwise false
    */
    bool removeWarpField(const position &pos);

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
    void TriggerFieldMove(Character *cc, bool moveto);

    /**
    * update the character container about the position change
    *@param cc the character which is moving
    *@param from the old position
    *@param to the new position
    */
    void moveTo(Character *cc, const position& to);


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
    bool pushPlayer(Player *cp, unsigned char d, short int &walkcost);

    void checkFieldAfterMove(Character *character, const map::Field &field);
    void sendPassiveMoveToAllVisiblePlayers(Character *ccp);
    void sendSpinToAllVisiblePlayers(Character *cc);
    void sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char moveType, TYPE_OF_WALKINGCOST duration);
    void sendCharacterMoveToAllVisibleChars(Character *cc, TYPE_OF_WALKINGCOST duration);
    void sendCharacterWarpToAllVisiblePlayers(Character *cc, const position &oldpos, unsigned char moveType);
    template<class T> void sendCharsInVector(const std::vector<T *> &vec, Player *cp, bool sendSpin);

    void lookAtMapItem(Player *player, const position &pos, uint8_t stackPos);

private:
    void lookAtTile(Player *cp, unsigned short int tile, const position &pos);

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

    void makeSoundForAllPlayersInRange(const position &pos, int radius, unsigned short int sound);
    void makeGFXForAllPlayersInRange(const position &pos, int radius, unsigned short int gfx);

    //! send a message to all chars near pos
    //! \param message what the char says
    //! \param tt did he shout/talk/whisper?
    //! \param the talking char
    void sendMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Character *cc);
    void sendMessageToAllCharsInRange(const std::string &german, const std::string &english, Character::talk_type tt, Character *cc);

    /**
    *Sends a message in a specific language to all chars in range with this language
    *@param message what the chars says
    *@param tt shout/talk/whisper
    *@param lang the language to which players the message is send
    *@param cc the talking char
    *@todo remove deprecated function
    */
    void sendLanguageMessageToAllCharsInRange(const std::string &message, Character::talk_type tt, Language lang, Character *cc);

    void sendMessageToAllPlayers(const std::string &message);
    void broadcast(const std::string &german, const std::string &english);

    //! schickt eine Nachricht an alle GM's im Spiel.
    // \param message die zu schickende Nachricht
    void sendMessageToAdmin(const std::string &message);

    //! converts the number of a language to the proper skill name.
    // \param language number
    std::string languageNumberToSkillName(int languageNumber);

    //! converts the language number into the proper language prefix for talking
    //\param language number
    std::string languagePrefix(int Language);

    Range getTalkRange(Character::talk_type tt) const;

    //! schickt an alle Spieler auf den Nachbarfeldern von cp dessen Namen
    // \param cp der Player der sich den anderen bekannt macht
    void introduceMyself(Player *cp);

    //Stellt den Player dem entsprechenden Admin vor
    void forceIntroducePlayer(Player *cp, Player *admin);

    //GM Funktion um eine Vorstellen zu erzwingen.
    void ForceIntroduce(Player *player, const std::string &text);

    //GM Funktion um ein Vorstellen aller Personen im Sichtbereich zu erzwingen
    void ForceIntroduceAll(Player *player);

    //Sends the current weather to one player
    void sendWeather(Player *cp);

    //Sends the current IG time to all players online
    void sendIGTimeToAllPlayers();

    //Sends the current IG Time to one player
    void sendIGTime(Player *cp);

    /**
    *sends the current weather to all players online
    */
    void sendWeatherToAllPlayers();

    ////////// in WorldIMPLAdmin.cpp /////////////

    //Sendet eine Message an alle Spieler
    //param cp: der Spieler der die Message sendet
    //param message: die nachricht die geschickt wird
    void broadcast_command(Player *cp, const std::string &message);

    //Kickt alle Spieler aus dem Spiel
    //param cp: der Spieler welche alle Spieler kickt
    void kickall_command(Player *cp);

    //Kickt einen einzelnen Spieler aus dem Spiel
    //param cp: der Spieler welcher jemanden kickt
    //param player: der Spieler welcher gekickt wird
    void kickplayer_command(Player *cp, const std::string &player);

    //Zeigt dem Spieler nutzerdaten (IP's) an
    void showIPS_Command(Player *cp);

    /**
    *creates an item in the inventory of the gm
    */
    void create_command(Player *cp, const std::string &itemid);

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
    void teleportPlayerToOther(Player *player, const std::string &target);

    //! toetet alles auf der Karte befindlichen Monster
    void kill_command(Player *cp);

    //! resambles the former #r command, reloads all tables, definitions and scripts
    // \param cp is the GM performing this full reload
    void reload_command(Player *cp);

    //! substitutes #j <name>, jump to a player of a given name
    // \param cp is the jumping GM
    // \param ts name of the player to jump to
    void jumpto_command(Player *cp, const std::string &player);

    //! resambles the former #mapsave command, saves the map
    // \param cp the corresponding GM with correct rights who initiates this mapsave
    void save_command(Player *cp);

    //! Macht spieler cp unsichtbar
    void makeInvisible(Player *cp);

    void talkto_command(Player *player, const std::string &text);

    //! Macht spieler cp sichtbar
    void makeVisible(Player *cp);

    //! wirft alle Player aus dem Spiel
    void forceLogoutOfAllPlayers();

    //! wirft einen aktiven Player aus dem Spiel
    // \param name der Spieler der herausgeworfen werden soll
    // \return true falls der Player gefunden wurde, false sonst
    bool forceLogoutOfPlayer(const std::string &name);

    //! sendet einem Admin die Daten aller aktiven Player
    // \param admin der Admin an den die Daten gesandt werden sollen
    void sendAdminAllPlayerData(Player *admin);

    // ! Server side implemented !warp_to x y z
    void warpto_command(Player *player, const std::string &text);

    // ! Server side implemented !summon Player
    void summon_command(Player *player, const std::string &text);

    // ! relaods only the definition from the db no Monsterspawns and no NPC's are loaded.
    bool reload_defs(Player *cp);

    // ! adds Warpfields to map from textfile
    bool importWarpFields(Player *cp,const std::string &filename);

    // ! Deletes a Warpfield
    void removeTeleporter(Player *cp, const std::string &text);

    // ! Shows a list of Warpfields
    void showWarpFieldsInRange(Player *cp, const std::string &text);
    //////////////////////////////////////////////////////////////////

    //////////// in WorldIMPLItemMoves.cpp ////////////////

    Item g_item;
    Container *g_cont;

    bool takeItemFromMap(Character *cc, const position &itemPosition);
    bool putItemOnMap(Character *cc, const position &itemPosition);
    bool putItemAlwaysOnMap(Character *cc, const position &itemPosition);
    bool takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count);
    bool putItemOnInvPos(Character *cc, unsigned char pos);
    bool takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count);
    bool putItemOnInvPos(Player *cc, unsigned char pos);

    void closeShowcaseForOthers(Player *target, Container *moved);
    void closeShowcaseIfNotInRange(Container *moved, const position &showcasePosition);

    bool takeItemFromShowcase(Player *cc, uint8_t showcase, unsigned char pos, Item::number_type count);
    bool putItemInShowcase(Player *cc, uint8_t showcase, TYPE_OF_CONTAINERSLOTS pos);
    void checkField(const map::Field &field, const position &itemPosition);

    bool moveItemFromMapToMap(Player *cp, const position &oldPosition, const position &newPosition, Item::number_type count);
    void moveItemFromMapIntoShowcase(Player *cp, const position &sourcePosition, uint8_t showcase, unsigned char showcaseSlot, Item::number_type count);
    void moveItemFromMapToPlayer(Player *cp, const position &sourcePosition, unsigned char inventorySlot, Item::number_type count);
    void moveItemBetweenShowcases(Player *cp, uint8_t source, unsigned char pos, uint8_t dest, unsigned char pos2, Item::number_type count);
    void dropItemFromShowcaseOnMap(Player *cp, uint8_t showcase, unsigned char pos, const position &newPosition, Item::number_type count);
    void moveItemFromShowcaseToPlayer(Player *cp, uint8_t showcase, unsigned char pos, unsigned char cpos, Item::number_type count);
    void moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count);
    void dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, const position &newPosition, Item::number_type count);
    void moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, uint8_t showcase, unsigned char pos, Item::number_type count);
    bool pickUpItemFromMap(Player *cp, const position &itemPosition);
    void pickUpAllItemsFromMap(Player *cp);

    void sendRemoveItemFromMapToAllVisibleCharacters(const position &itemPosition);
    void sendPutItemOnMapToAllVisibleCharacters(const position &itemPosition, const Item &it);
    void sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, const position &itemPosition, const Item &it);
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot, Container *moved);
    void sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot);

    //////////////////////////////////////In WorldIMPLScriptHelp.cpp//////////////////////////////////////////

    /**
    *deletes an NPC

    *@param npc The npc to be deleted
    *@return success of deletion
    */
    bool deleteNPC(unsigned int npcid);

    /**
    *creates a dynamic NPC

    *@param name Name of the NPC
    *@param type Race type
    *@param pos position
    *@param sex gender
    *@param scriptname scriptname
    *@return success of creation
    */
    bool createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos, /*CCharacter::face_to dir,*/ Character::sex_type sex, const std::string &scriptname);

    /**
    *creates a list with all the players which are currently online and returns it
    *to lua
    *@return the list with all the players currently online
    */
    std::vector<Player *> getPlayersOnline() const;

    /**
    *creates a list with all the npcs which are currently online and returns it
    *to lua
    *@return the list with all the npcs currently online
    */
    std::vector<NPC *> getNPCS() const;

    /**
    *creates a list with all the characters in range around a specific position
    *@param posi the position from where the character around should be calculated
    *@param range the range around the position for calculating
    *@return a list with all the characters (including monsters, npcs, players) around this char
    */
    std::vector<Character *> getCharactersInRangeOf(const position &pos, uint8_t radius) const;
    std::vector<Player *> getPlayersInRangeOf(const position &pos, uint8_t radius) const;
    std::vector<Monster *> getMonstersInRangeOf(const position &pos, uint8_t radius) const;
    std::vector<NPC *> getNPCSInRangeOf(const position &pos, uint8_t radius) const;

    //Sucht zu einem Item die gesamten Stats wie Gewicht heraus
    //\param item, das Item zu dem die Stats heraus gesucht werden sollen.
    ItemStruct getItemStats(const ScriptItem &item);
    ItemStruct getItemStatsFromId(TYPE_OF_ITEM_ID id);

    //Aendert die Qualitaet eines ScriptItems.
    //param item, das Item das geaendert werden soll
    //param amount, der Wert um den die Qualitaet geaendert werden soll
    void changeQuality(ScriptItem item, short int amount);

    virtual void itemInform(Character *user, const ScriptItem &item, const ItemLookAt &lookAt);

    //Liefert den Namen eines Items mit einer bestimmten id zurck
    //\param itemid, id des items zu dem der Name geliefert werden soll
    //\param language, die Sprache in der der Name zurck gegeben werden sollte
    //\ret der name in der entsprechenden Sprache.
    virtual std::string getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language);

    //Aendert ein ScriptItem
    bool changeItem(ScriptItem item);

    //Prft ob sich auf dem Feld ein Character befindet
    //\param position die zu prfen ist
    //\return true oder false
    bool isCharacterOnField(const position &pos) const;

    //Liefert einen Zeiger auf einen Character
    //\param pos, die Position auf der sich der Character befinden soll
    //\return Zeiger auf den Character
    character_ptr getCharacterOnField(const position &pos) const;

    //Loescht ein ScriptItem
    //\ param Item, das Item welches geloescht werden soll
    //\ param amount, Anzahl der Items
    //\ return bool Wert der Angibt ob das Item erfolgreich geloescht werden konnte
    bool erase(ScriptItem item, int amount); //, int amount);

    //Tauscht ein ScriptItem durch ein anderes Item aus
    //\ param Item, das Item welches ausgetauscht werden soll
    //\ param newItem, Id des neuen Items welches erstellt werden soll
    //\ return bool Wert der Angibt ob das Item erfolgreich getauscht wurde
    bool swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, unsigned short int newQuality=0);

    //Erhoeht die Anzahl eines ScriptItems um einen bestimmten Wert
    //\ param Item, das Item welches erhoeht werden soll
    //\ param count, die anzahl um die erhoeht werden soll
    //\ return bool Wert ob das erhoehen der Anzahl erfolgreich war.
    bool increase(ScriptItem item, short int count);


    //Erzeugt auf dem Angegebenen Feld ein Item mit einer bestimmten ID
    //\ param id Id des neu zu erzeugenden Items
    //\ param count Anzahl des Items
    //\ param pos Position des Items
    //\ return bool Wert ob das Erstellen geklappt hat.
    //\ quali int, das die qualitaet angibt
    ScriptItem createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, const position &pos, bool always, int quality, script_data_exchangemap const *data);

    //Erzeugt auf den Angegebenen Feld ein bestimmtes Item
    //\ param item, das Item was erzeugt werden soll
    //\ param pos, Position des Items
    //\ return bool Wert der angibt ob das Erstellen geklappt hat.
    bool createFromItem(ScriptItem item, const position &pos, bool always);

    //Erzeugt ein Monster mit der entsprechenden ID auf dem Feld
    //\ param id, das Monster welches Erzeugt werden soll
    //\ param pos, die Position des Monsters
    //\ return character_ptr Valid monster on success, invalid monster on failure
    character_ptr createMonster(unsigned short id, const position &pos, short movepoints);

    //Zeigt eine Grafik auf einem bestimmten Feld an
    //\gfxid, ID der anzuzeigenden Grafik
    //\pos, Position wo diese Grafik angezeigt werden soll
    void gfx(unsigned short int gfxid, const position &pos);

    //Spielt einen Soundeffekt auf einem bestimmten Feld ab
    //\soundid, ID des abzuspielenden Effektes
    //\pos, Position wo dieser Effekt abgespielt werden soll
    void makeSound(unsigned short int soundid, const position &pos);

    //!Prft ob auf einen FEld ein ITem liegt
    //\pos die Position des feldes
    //\return true wenn auf den Feld ein ITem liegt
    bool isItemOnField(const position &pos);

    //!Liefert ein Item zurck welches auf dem Feld liegt
    //\pos die Position des Feldes
    //\return das ScriptITem auf dem Feld;
    ScriptItem getItemOnField(const position &pos);

    //!Aendert ein Bodentile
    //\tileid, neue Id des bodentiles
    //\pos, Position des zu aendernden Bodentiles.
    void changeTile(short int tileid, const position &pos);


    //!Sendet ein Map update zu allen spielern in bereich range um pos
    //\pos, position von der ausgegangen wird
    //\bereich von dem die Spieler genommen werden sollen.
    void sendMapUpdate(const position &pos, uint8_t radius);

    //!Sicherer CreateArea command, prft erst ab ob er eine vorhandene Map berschreibt.
    //\tileid: standard Tile
    //\pos: begin der neuen Karte
    //\height: Hoehe der neuen Karte
    //\width: breite der neuen Karte
    //\return true wenn das einfgen klappte, ansonsten false wenns zu berlagerungen kommt.
    bool createSavedArea(uint16_t tile, const position &origin, uint16_t height, uint16_t width);

    //! Laedt eine Armor Struct anhand einer id.
    //\return true false der Armorstruct gefunden wurde
    bool getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret);

    //! Laedt einen Weapon Struct anhand einer id.
    //\return true false der WeaponStruct gefunden wurde.
    bool getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret);

    //! Laedt ein MonsterRst Struct anhand einer id.
    //\return true false der Struct gefunden wurde.
    bool getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret);

    //! Laedt ein Attack Boni struct anhand einer id
    //\return true wenn das Struct gefunden wurde
    bool getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret);

    /**
    *sends a Message to All Monitoring Clients
    *@param msg the message string which should be sended
    *@param id the id of the msg ( 1 are message which displayed in a window 0 basic message)
    */
    void sendMonitoringMessage(const std::string &msg, unsigned char id = 0);

    /**
     * bans a player for the bantime
     * @param cp pointer to the player which should been banned
     * @param bantime in seconds
     * @gmid the id of the gm which has banned the player
     */
    void ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid);

    void set_login(Player *player, const std::string &text);

protected:
    World(); // used for testcases
    static World *_self;

private:
    void logMissingField(const std::string &function, const position &field);

    bool _is_login_allowed = true;
    bool _is_spawn_enabled = true;

    /**
    * the constructor of World, private because of singleton pattern
    *
    *@param dir the main directory of the server
    *@param time_t the starting time of the server
    */
    explicit World(const std::string &dir);
    World &operator=(const World &) = delete;
    World(const World &) = delete;

    //! IG day of last turntheworld
    int lastTurnIGDay;

    // check spawns every minute
    Timer monstertimer {60};

    void ageMaps();
    void ageInventory();

    //! das Verzeichnis mit den Skripten
    std::string scriptDir;

    // spawnplaces...
    std::list<SpawnPoint> SpawnList;

    // initmethod for spawn places...
    bool initRespawns();

    typedef std::map<std::string, CommandType> CommandMap;
    CommandMap GMCommands;
    CommandMap PlayerCommands;

    // Send player information to GMs
    void who_command(Player *cp, const std::string &tplayer);

    // Change tile in front of admin
    void tile_command(Player *cp, const std::string &tile);

    // Turtle tile commands
    void turtleon_command(Player *cp, const std::string &tile);
    void turtleoff_command(Player *cp);

    // Clipping on/off command
    void clippingon_command(Player *cp);
    void clippingoff_command(Player *cp);

    // Describe to admin tile in front of them
    void what_command(Player *cp);

    // Save all online players
    void playersave_command(Player *cp);

    // Create telport warp on current tile to x, y, z
    void teleport_command(Player *cp, const std::string &text);

    // Give help for GM commands
    void gmhelp_command(Player *cp);

    //Sendet eine Nachricht an alle GM's
    bool gmpage_command(Player *player, const std::string &ticket);

public:
    void ban_command(Player *cp, const std::string &text);
    void logGMTicket(Player *player, const std::string &ticket, bool automatic);
    void checkPlayerImmediateCommands();
    void addPlayerImmediateActionQueue(Player* player);

private:
    map::WorldMap maps;

    std::vector<Character *> getTargetsInRange(const position &pos, int radius) const;
    
    bool active_language_command(Player *cp, const std::string &language);

    // register any GM commands here...
    void InitGMCommands();
    // register any Player commands here...
    void InitPlayerCommands();
    bool executeUserCommand(Player *user, const std::string &input, const CommandMap &commands);

    // export maps to mapdir/export
    bool exportMaps(Player *cp) const;

    void ignoreComments(std::ifstream &inputStream);

    //! reload all tables
    bool reload_tables(Player *cp);

    void version_command(Player *player);

    std::mutex immediatePlayerCommandsMutex;
    std::queue<Player*> immediatePlayerCommands;
};

#endif

