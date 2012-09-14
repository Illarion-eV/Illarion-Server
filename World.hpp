/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WORLD_HH
#define WORLD_HH

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/*
#define World_DEBUG
#define World_ItemMove_DEBUG
#define World_CharMove_DEBUG
*/

//falls nicht auskommentiert, werden die Gespraeche der Player gespeichert
#define   LOG_TALK

#include <list>

#include "tuningConstants.hpp"
#include "Player.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "TccharacterVector.hpp"
#include "Map.hpp"
#include "MilTimer.hpp"
#include "Timer.hpp"
#include "constants.hpp"
#include "IdCounter.hpp"
#include "MapVector.hpp"
#include "NewClientView.hpp"
#include "SpawnPoint.hpp"
#include "data/NPCTable.hpp"
#include "Scheduler.hpp"
#include "data/SpellTable.hpp"
#include "data/TriggerTable.hpp"
#include "TableStructs.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
#include "MonitoringClients.hpp"
#include "InitialConnection.hpp"
#include "tvector.hpp"
#include "script/LuaLogoutScript.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include <exception>

extern CommonObjectTable *CommonItems; /**< a extern table for ItemAttributes @see CommonObjectTable* CommonItems **/

extern NamesObjectTable *ItemNames; /**< a extern table for Itemnames @see NamesObjectTable* ItemNames **/

extern WeaponObjectTable *WeaponItems;

extern ArmorObjectTable *ArmorItems;

//! eine Tabelle fuer Artefakt - Item Daten
extern ArtefactObjectTable *ArtefactItems;

//! eine Tabelle fuer Behaelter - Item Daten
extern ContainerObjectTable *ContainerItems;

//! eine Tabelle mit Item welche die Eigenschaften des Feldes auf dem sie liegen modifizieren
extern TilesModificatorTable *TilesModItems;

//! eine Tabelle mit allen Arten von Bodenplatten
extern TilesTable *Tiles;

//! ein struct fuer die Allgemeinen Attribute eines Item
extern CommonStruct tempCommon;

//! ein struct fuer Daten von Bodenplatten
extern TilesStruct tempTile;

//! ein struct fuer die Namen eines Item
extern NamesStruct tempNames;

//! ein struct fuer Daten einer Waffe
extern WeaponStruct tempWeapon;

//! ein struct fuer Daten einer Schutzkleidung
extern ArmorStruct tempArmor;

//! ein struct fuer Daten von Item
extern TilesModificatorStruct tempModificator;

//! ein struct fuer Zaubersprche
extern SpellTable *Spells;

//! ein Struct fuer Trigger
extern TriggerTable *Triggers;

//! Eine Tabelle mit MonsterRstungen
extern NaturalArmorTable *NaturalArmors;

//! Eine Tabelle mit Bonis fuer Monsterangriffe
extern MonsterAttackTable *MonsterAttacks;

extern ScheduledScriptsTable *scheduledScripts;

//! Ein zeiger auf das Standard Kampfscript
extern boost::shared_ptr<LuaWeaponScript> standardFightingScript;

// typedef for gm commands...
/**
* a class for holding gm or player commands
*/
class CommandType {

public:
    /**
    * pure virtual operator overloading
    */
    virtual bool operator()(World *, Player *, const std::string &) = 0;
    virtual ~CommandType() {}

private:
    CommandType &operator=(const CommandType &);
};

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
        blockingChar = NULL;
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
    typedef ccharactervector < Player * > PLAYERVECTOR;

    /**
    *a typedef for holding monsters
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    typedef ccharactervector < Monster * > MONSTERVECTOR;

    /**
    *a typedef for holding npc's
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    */
    typedef ccharactervector < NPC * > NPCVECTOR;

    /**
    *  a typedef for holding players which have logged out in a thread save
    * vector
    */
    typedef tvector<Player *> TSPLAYERVECTOR;

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
    *holds all player which have logged out (for deleting them)
    */
    //TSPLAYERVECTOR LostPlayers;

    /**
    *holds all monsters on the world
    *@todo: change the three vectors @see PLAYERVECTOR, @see MONSTERVECTOR, @see NPCVECTOR so there is only one HARVECTOR
    **/
    MONSTERVECTOR Monsters;

    /**
    * new Monsters which should be spawned so the server didn't crash on creating monsters from monsters
    */
    MONSTERVECTOR newMonsters;

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
    MonitoringClients *monitoringClientList;

    timeb now; /**< current time of the server used in @see turntheworld() **/

    unsigned long int timeStart;
    unsigned long int usedAP;

    short int ap; /**< actionpoints since the last loop call **/

    IdCounter *npcidc; /**< a counter for npc id's **/

    MapVector maps; /**< a vector which holds all the maps*/

    Map *tmap; /**< a temporary pointer to a map, used from different methods @see Map*/

    Scheduler *scheduler;/**< a pointer to the scheduler object @see Scheduler*/

    WeatherStruct weather;/**< a struct to the weather @see WeatherStruct */

    /**
    *inline function for setting the current weather and sending the data to all players online
    */
    inline void setWeather(WeatherStruct nWeather) {
        weather = nWeather;
        sendWeatherToAllPlayers();
    }

    /**
    *a function for equalyzing to positions
    */
    struct eqpos {
        bool operator()(position a, position b) const {
            return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
        }
    };

    /**
    * hashfunction for the position
    */
    struct poshash {
        hash < int > inthash;
        int operator()(const position a) const {
            int temp = (a.x * 1000 + a.y) * 1000 + a.z;
            return inthash(temp);
        }
    };

    //! parse GMCommands of the Form !<string1> <string2> and process them
    bool parseGMCommands(Player *cp, const std::string &text);
    //! parse PlayerCommands of the Form !<string1> <string2> and process them
    bool parsePlayerCommands(Player *cp, const std::string &text);


    /**
    * definition of a map which holds two positions
    */
    typedef hash_map < position, position, poshash, eqpos > POSITIONHASH;

    /**
    *a struct for saving different field attributes
    */
    typedef struct {
        unsigned char type;/**< type of the field*/
        unsigned long int flags; /**< flags of the field*/
    }
    s_fieldattrib;

    /**
    *definition of a map which holds the special attribs of fields
    */
    typedef hash_map < position, s_fieldattrib, poshash, eqpos > FIELDATTRIBHASH;

    /**
    *the map which holds the information of all the special fields of the map
    */
    FIELDATTRIBHASH specialfields;

    /**
    *the current script which is called
    */
    LuaScript *currentScript;

    ///////// in World.cpp ///////
    /**
    *the standard destructor of the server
    */
    ~World();

    /**
    *main loop for the world
    */
    void turntheworld();

    /**
    *checks all player aktions and makes them active
    */
    void checkPlayers();

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
    int getTime(std::string timeType);

    /**
        * function for maploading
        *
        *load several maps from the import dir
        *@return true if loading was successful, false otherwise
        */
    bool load_maps();

    bool import_all_maps();

    /**
    *function for maploading
    *this function loads a specified new map from the importdir
    *@param filename name of the mapfile which should be loaded
    *@return true if loading was successful, false otherwise
    */
    bool load_from_editor(std::string filename);

    /**
    *checks the command list of one player and put them into practize
    *@param cp the player which buffer is currently processed
    */
    void workout_CommandBuffer(Player* &cp);


    static World *create(std::string dir, time_t starttime);
    static World *get() throw(std::runtime_error);



    /**=============CWorldIMPLMonCommands.cpp===============*/
    void montool_kill_command(Player *c);
    void montool_reload_command(Player *c);
    void montool_kickall_command(Player *c);
    void montool_import_maps_command(Player *c);
    void montool_set_login(Player *c, const std::string &st);

    /**============ WorldIMPLTools.cpp ==================*/

    /**
    * checks the list of LostNpcs and deletes all the npcs which are in this and the normal npc list
    * @todo is a bad hack to resolve the problem that segfaults the server on selling cows, should be changed
    * with a better char handling from the server side (smart pointers, one list for all char types).
    */
    void deleteAllLostNPC();

    /**
    *finds the player with the lowes healthpoints in a given player vector
    *@param ppvec the vector which should be searched for the player with the lowest hp
    *@param call by reference, returns the player which was found
    *@return if true there was a player found otherwise false
    */
    bool findPlayerWithLowestHP(std::vector < Player * > * ppvec, Player* &found);

    inline LuaScript *getCurrentScript() {
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
    bool findWarpFieldsInRange(position pos, short int range, std::vector< boost::shared_ptr< position > > & warppositions);

    /**
    * returns a list of blocking objects between a startin position and a ending position
    * @param startingpos the starting position of the line of sight
    * @param endingpos the end of the line of sight calculation
    * @return list of all blocking objects between startingpos and endingpos.
    */
    std::list<BlockingObject> LoS(position startingpos, position endingpos);


    bool findPlayersInSight(position pos, uint8_t range, std::vector<Player *> &ret, Character::face_to direction);


    /**
    * searches for a special character
    * looks into all three vectors ( player, monster, npc ) for a character on the given field
    * can be found in WorldIMPLTools.cpp
    * @param posx the x coordinate of the field
    * @param posy the y coordinate of the field
    * @param posz the z coordinate of the field
    * @return a pointer to a character, NULL if no character was found
    * @todo has to be changed for only one charactervector
    */
    Character *findCharacterOnField(short int posx, short int posy, short int posz);

    /**
        * searches for a special player
        * looks for a player on the given field
        * can be found in WorldIMPLTools.cpp
        * @param posx the x coordinate of the field
        * @param posy the y coordinate of the field
        * @param posz the z coordinate of the field
        * @return a pointer to a player, NULL if no player was found
        */
    Player *findPlayerOnField(short int posx, short int posy, short int posz);


    /**
    * searches for a special character
    * can be found in WorldIMPLTools.cpp
    * looks into all three vectors ( player, monster, npc )  for a character with the given id
    * @param id the id of the character which should be found
    * @return a pointer to the character, NULL if the character wasn't found
    * @todo has to be changed for only one charactervetor
    */
    Character *findCharacter(TYPE_OF_CHARACTER_ID id);

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

    /**
    * kills one monster and drops his inventory on the map
    * can be found in WorldIMPLTools.cpp
    * @warning iterators on the monsters becomes invalid
    * @param monsterp a pointer to the monster which should be killed
    * @ true if the monster was deleted succesfully otherwise false
    */
    bool killMonster(Monster *monsterp);

    /**
    * kills one monster
    * drops the invventory on the map and returns the iterator of the monster
    * which is in the list after the killed monster.
    * @param monsterIt iterator to the monster which should be killed
    * @param newIt iterator the the next monster in the list
    */
    void killMonster(MONSTERVECTOR::iterator monsterIt, MONSTERVECTOR::iterator &newIt);

    /**
    * heals a character
    * depending on his foodlevel
    * @param cc the character which should be healed
    * @return true if there was some healing, otherwise false
    */
    bool doHealing(Character *cc);

    /**
    * looks for a field on the map
    * @param fip call by reference, the pointer to the field which was found
    * @param pos the position where the field has to be found
    * @return true if the field was found otherwise false
    */
    bool GetPToCFieldAt(Field* &fip, position pos);

    /**
    * looks for a field on the map
    * @param pos the position at which the field should be
    * @return a pointer to the field, NULL if there is no field at this position
    * @see GetPToCFieldAt()
    */
    Field *GetField(position pos);

    /**
    * looks for a field and the special map where it lies on
    * @param fip call by reference, pointer to the field which was found
    * @param map call by reference, pointer to the map on which the field lies
    * @return true if the field was found otherwise false
    */
    bool GetPToCFieldAt(Field* &fip, position pos, Map* &map);

    /**
    * looks for a field on the current map
    * @param fip call by reference, pointer to the field if it was found
    * @param x the x-coordinate
    * @param y the y-coordinate
    * @param z the z-coordinate
    * @return true if the field was found otherwise false
    */
    bool GetPToCFieldAt(Field* &fip, short int x, short int y, short int z);

    /**
    * looks for a field and the special map where it lies on
    * @param fip call by reference, pointer to the field if it was found
    * @param x the x-coordinate
    * @param y the y-coordinate
    * @param z the z-coordinate
    * @param map call by reference, pointer to the map on which the field lies
    * @return true if the field was found otherwise false
    */
    bool GetPToCFieldAt(Field* &fip, short int x, short int y, short int z, Map* &map);

    /**
    * looks for an empty field in the near of a given pos
    * @param cf call by reference, pointer to the field in the near of the given pos
    * @param x call by reference, x-coordinate of the field (changes to the x coordinate of the field which was returned in fip)
    * @param y call by reference, y-coordinate of the field (changes to the y coordinate of the field which was returned in fip)
    * @param z z-coordinate of the field
    * @return true if there was an empty field near the given coordinates otherwise false
    */
    bool findEmptyCFieldNear(Field* &cf, short int &x, short int &y, short int z);

    /**
    *returns for a item of the given id the value of an attrib back
    *@param ItemId the id of the Item
    *@return the value of the attribute or 0 if there was not such attribute
    *@param s which attribute
    *
    * s can be
    *
    * for armors:
    * <ul>
    * <li> "bodyparts" </li>
    * <li> "strokearmor" </li>
    * <li> "thrustarmor" </li>
    * <li> "armormagicdisturbance" </li>
    * </ul>
    *
    * @see ArmorStruct
    *
    * for commons:
    * <ul>
    * <li> "agingspeed" </li>
    * <li> "objectafterrot" </li>
    * <li> "volume" </li>
    * <li> "weight" </li>
    * </ul>
    *
    * @see CommonStruct
    *
    * for tiles modificators:
    * <ul>
    * <li> "modificator" </li>
    * </ul>
    *
    * @see TilesModifcators
    *
    * for weapons:
    * <ul>
    * <li> "accuracy" </li>
    * <li> "defence" </li>
    * <li> "range" </li>
    * <li> "weapontype" </li>
    * <li> "weaponmagicdisturbance" </li>
    * </ul>
    *
    *@see WeaponStruct
    *
    * for containers:
    * <ul>
    * <li> "containervolume" </li>
    * </ul>
    *
    * @see ContainerStruct
    *
    */
    int getItemAttrib(std::string s, TYPE_OF_ITEM_ID ItemID);

    /**
    *sends all players in sight of the tiles line the map
    *
    *@param startx the starting x position of the line
    *@param endx the ending x position of the line
    */
    void updatePlayerView(short int startx, short int endx);

    /**
    *closes the showcases of different players
    */
    void closeShowcasesForContainerPositions();

    /**
    *loads the world
    *
    *@param prefix the name of the world wich should be loaded
    */
    void Load(std::string prefix);

    /**
    *saves the world
    *
    *@param prefic the name under which the world should be saved
    */
    void Save(std::string prefix);

    /**
    *saves the names of all players online in a file.
    *@param name the name of the file where the names should be saved
    */
    void saveAllPlayerNamesToFile(std::string name);

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
    void setWeatherPart(std::string type, char value);

    void sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, position &pos);

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
    bool addWarpField(position where, position target, unsigned short int starttilenr, Item::id_type startitemnr);

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
    bool addWarpField(position where, position target, unsigned short int starttilenr, Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr);

    /**
    *adds a special field to a specific position
    *
    *@param where the position where the special field should be added
    *@param which which type of special field should be added
    *@see s_fieldattrib
    *@return true if the adding was succesfully otherwise false
    */
    bool makeSpecialField(position where, s_fieldattrib which);

    /**
    *adds a special field to a specific position
    *
    *@param x the x coordinate of the new special field
    *@param y the y coordinate of the new special field
    *@param z the z coordinate of the new special field
    *@param type the type of the new special field
    *@see s_fieldattrib.type
    *@param value the flags of the new special field
    *@return true if the adding was succesfully otherwise false
    */
    bool makeSpecialField(short int x, short int y, short int z, unsigned char type, unsigned long int value);

    /**
    *adds a teleportfield and a teleportfield back to the field one layer higher
    *
    *@param where the position for the warpfield
    *@param starttilenr the id of the new groundtile on the startfield. 0 for no changes
    *@param startitemnr the id of a item which should be placed at the startfield. 0 for no changes
    *@param targettilenr the id of the new grountile on the targetfield (same x,y position but z + 1 ). 0 for no changes
    *@param tergetitemnr tje od of an item which should be placed at the targetfield. 0 for no changes
    *@return true if the adding of the warpfield was successfully otherwise false
    */
    bool addWayUp(position where, unsigned short int starttilenr, Item::id_type startitemnr, unsigned short int targettilenr, Item::id_type targetitemnr);

    /**
    *removes a warpfield at a given position
    *
    *@param where the position of the warpfield which should be removed
    *@return true if the removing was succesfull otherwise false
    */
    bool removeWarpField(position where);

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

    /**
    *warps a Player through a warpfield
    *
    *@param cp pointer to the player which should be warped
    *@param cfstart the field which should be proofed if it is a warpfield
    *@return true if the warp was succesfull otherwise false
    */
    //bool warpPlayer( Player* cp, Field* cfstart );

    /**
    *warps a player to the next free field in the near of a given position
    *
    *@param cp a pointer to a player which should be warped
    *@param pos the position to which the player should be warped
    *@return true if the warp was succesfull otherwise false
    */
    //bool warpPlayer( Player* cp, position pos );

    /**
    *checks a field for special actions and activates this actions (Specielfield or items)
    *
    *@param cp a pointer to the character which walked on the field
    *@param cfstart a pointer to the field which should be checked for special actions
    */
    void checkFieldAfterMove(Character *cp, Field *cfstart);

    /**
    * a Player pushes a character
    *
    *@param cp a pointer to the player which is pushing the character
    *@param pushedCharId id of the character to be pushed
    *@param direction the direction of the movement relative to the pushing char
    *
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
    *@return true if the pushing action was succesfull otherwise false
    */
    bool pushCharacter(Player *cp, TYPE_OF_CHARACTER_ID pushedCharId, unsigned char direction);

#if 0
    //! bewegt das Monster in die durch d angegebene Richtung
    // \param cp Zeiger auf das Monster das bewegt werden soll
    // \param d Richtung der Bewegung: 0->Norden bis 7->NO im Uhrzeigersinn, 8 nach oben, 9 nach unten
    // \return true falls Bewegung ausgefhrt werden konnte, false sonst
    bool moveMonster(Monster *cp, unsigned char d, bool active);

    //! bewegt den NPC in die durch d angegebene Richtung
    // \param cn Zeiger auf den NPC der bewegt werden soll
    // \param d Richtung der Bewegung: 0->Norden bis 7->NO im Uhrzeigersinn, 8 nach oben, 9 nach unten
    // \return true falls Bewegung ausgefhrt werden konnte, false sonst
    bool moveNPC(NPC *cn, unsigned char d, bool active);
#endif

    //! teleportiert cm, falls cfstart ein WarpFeld ist
    // \param cp Zeiger auf das Monster das bewegt werden soll
    // \param cfstart das zu prfende Feld
    // \return true falls Bewegung ausgefhrt werden konnte, false sonst
    bool warpMonster(Monster *cm, Field *cfstart);

    //! dreht den Spieler in die durch d angegebene Richtung
    // \param cp Zeiger auf den Spieler der gedreht werden soll
    // \param d neue Blickrichtung : 0->Norden bis 7->NO im Uhrzeigersinn, 8 nach oben, 9 nach unten
    // \return true falls Drehung ausgefhrt werden konnte, false sonst
    bool spinPlayer(Player *cp, unsigned char d);

    //! schickt eine Verschiebungsmeldung (Player/NPC/Monster) an alle Player im Sichtbereich
    // \param ccp der bewegte Character
    void sendPassiveMoveToAllVisiblePlayers(Character *ccp);

    //! schickt eine Character-Drehung an alle Player im Sichtbereich von cc
    // \param cc der Character der sich gedreht hat
    void sendSpinToAllVisiblePlayers(Character *cc);

    //! schickt eine Character-Bewegungsmeldung an alle Player im Sichtbereich der Position
    // von cc (alt und neu) ausser an cc selbst
    // \param cc der bewegte Character
    // \param direction Richtung der Bewegung
    // \param netid PLAYERMOVE_TC, MONSTERMOVE_TC oder NPCMOVE_TC
    // \param waitpages Anzahl der Animationen fr die Bewegung
    void sendCharacterMoveToAllVisiblePlayers(Character *cc, unsigned char netid, unsigned char waitpages);

    //! send movement to all chars in range
    // \param cc the moved player
    // \param waitpages number of animations for movement
    void sendCharacterMoveToAllVisibleChars(Character *cc, unsigned char waitpages);

    //! schickt eine Character-Warpmeldung an alle Player im Sichtbereich der Position
    // von cc (alt und neu) ausser an cc selbst
    // \param cc der bewegte Character
    // \param oldpos die alte Position
    // \param netid PLAYERMOVE_TC, MONSTERMOVE_TC oder NPCMOVE_TC
    void sendCharacterWarpToAllVisiblePlayers(Character *cc, position oldpos, unsigned char netid);

    //! sendet die Daten aller Chars in dem std::vector an cp
    // fuer jeden Eintrag muss gelten: (cp->pos.z - RANGEDOWN) >= eintr->pos.z
    // \param vec der std::vector mit den Character
    // \param cp Player an den verschickt wird
    // \param sendSpin falls true, wird auch die Blickrichtung mit verschickt
    template< class T > void sendCharsInVector(std::vector < T * > &vec, Player *cp, bool sendSpin);

    //////////// in WorldIMPLTalk.cpp /////////////////
    bool sendTextInFileToPlayer(std::string filename, Player *cp);

    //! sendet an den Spieler den Namen des obersten Item auf einem Feld
    //  bzw. den Namen des Feldes
    // \param cp der Spieler der benachrichtigt werden soll
    // \param xo X-Abstand des Item zu cp
    // \param yo Y-Abstand des Item zu cp
    // \param zo Z-Abstand des Item zu cp
    void lookAtMapItem(Player *cp, short int x, short int y, short int z);

private:
    void lookAtTile(Player *cp, unsigned short int tile, short int x, short int y, short int z);

public:
    //! sendet an den Spieler den Namen des Item an einer Position im showcase
    // \param cp der Spieler der benachrichtigt werden soll
    // \param showcase der showcase in dem das Item liegt
    // \param position die Position des Item im showcase
    void lookAtShowcaseItem(Player *cp, unsigned char showcase, unsigned char position);

    //! sendet an den Spieler den Namen des Item an einer Position im Inventory
    // \param cp der Spieler der benachrichtigt werden soll
    // \param position die Position des Item im Inventory
    void lookAtInventoryItem(Player *cp, unsigned char position);

    //! erzeugt fuer alle Spieler in der Naehe von xc,yc,zc einen Soundeffekt
    // \param xc X-Koordinate des Mittelpunktes
    // \param yc Y-Koordinate des Mittelpunktes
    // \param zc Z-Koordinate der Ebene
    // \param rn Entfernung Richtung Norden in der der Sound noch gehoert wird
    // \param rs Entfernung Richtung Sden in der der Sound noch gehoert wird
    // \param re Entfernung Richtung Osten in der der Sound noch gehoert wird
    // \param rw Entfernung Richtung Westen in der der Sound noch gehoert wird
    // \param ru Entfernung nach oben in der der Sound noch gehoert wird
    // \param rd Entfernung nach unten in der der Sound noch gehoert wird
    // \param sound ID fuer den Sound
    void makeSoundForAllPlayersInRange(short int xc, short int yc, short int zc, int distancemetric, unsigned short int sound);

    //! erzeugt fr alle Spieler in der Naehe von xc,yc,zc einen Grafikeffekt
    // \param xc X-Koordinate des Mittelpunktes
    // \param yc Y-Koordinate des Mittelpunktes
    // \param zc Z-Koordinate der Ebene
    // \param rn Entfernung Richtung Norden in der der Grafikeffekt noch gesehen wird
    // \param rs Entfernung Richtung Sden in der der Grafikeffekt noch gesehen wird
    // \param re Entfernung Richtung Osten in der der Grafikeffekt noch gesehen wird
    // \param rw Entfernung Richtung Westen in der der Grafikeffekt noch gesehen wird
    // \param ru Entfernung nach oben in der der Grafikeffekt noch gesehen wird
    // \param rd Entfernung nach unten in der der Grafikeffekt noch gesehen wird
    // \param gfx ID fuer den Sound
    void makeGFXForAllPlayersInRange(short int xc, short int yc, short int zc, int distancemetric, unsigned short int gfx);

#if 0
    //! schickt an alle Spieler in der Naehe von xc,yc,zc eine Nachricht
    // \param message die Nachricht die geschickt werden soll
    // \param xc X-Koordinate des Mittelpunktes
    // \param yc Y-Koordinate des Mittelpunktes
    // \param zc Z-Koordinate der Ebene
    // \param rn Entfernung Richtung Norden in der die Message noch gehoert wird
    // \param rs Entfernung Richtung Sden in der die Message noch gehoert wird
    // \param re Entfernung Richtung Osten in der die Message noch gehoert wird
    // \param rw Entfernung Richtung Westen in der die Message noch gehoert wird
    // \param ru Entfernung nach oben in der die Message noch gehoert wird
    // \param rd Entfernung nach unten in der die Message noch gehoert wird
    // \param mid ID fr SAY, SHOUT, WHISPER usw.
    void sendMessageToAllPlayersInRange(std::string message, short int xc, short int yc, short int zc, int distancemetric, unsigned char mid);
#endif

    //! send a message to all chars near pos
    //! \param message what the char says
    //! \param tt did he shout/talk/whisper?
    //! \param the talking char
    void sendMessageToAllCharsInRange(std::string message, Character::talk_type tt, Character *cc);

    /**
    *Sends a message in a specific language to all chars in range with this language
    *@param message what the chars says
    *@param tt shout/talk/whisper
    *@param lang the language to which players the message is send
    *@param cc the talking char
    */
    void sendLanguageMessageToAllCharsInRange(std::string message, Character::talk_type tt, unsigned char lang, Character *cc);

    //! schickt an alle Player eine Nachricht
    // \param message die Nachricht die geschickt werden soll
    void sendMessageToAllPlayers(std::string message);

    //! schickt eine Nachricht an alle GM's im Spiel.
    // \param message die zu schickende Nachricht
    void sendMessageToAdmin(std::string message);

    //! converts the number of a language to the proper skill name.
    // \param language number
    std::string languageNumberToSkillName(int languageNumber);

    //! converts the language number into the proper language prefix for talking
    //\param language number
    std::string languagePrefix(int Language);

    //! schickt an alle Spieler auf den Nachbarfeldern von cp dessen Namen
    // \param cp der Player der sich den anderen bekannt macht
    void introduceMyself(Player *cp);

    //Stellt den Player dem entsprechenden Admin vor
    void forceIntroducePlayer(Player *cp, Player *admin);

    //GM Funktion um eine Vorstellen zu erzwingen.
    void ForceIntroduce(Player *cp, const std::string &ts);

    //GM Funktion um ein Vorstellen aller Personen im Sichtbereich zu erzwingen
    void ForceIntroduceAll(Player *cp);

    void message(std::string message[3], Player *cp);

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

    void spawn_command(Player *cp, const std::string &monsterid);

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
    void teleportPlayerToOther(Player *cp, const std::string ts);

    //! toetet alles auf der Karte befindlichen Monster
    void kill_command(Player *cp);

    //! sets the informCharacter-flag
    // \param cp the corresponding Player to be informed
    void informChar(Player *cp);

    //! resambles the former #r command, reloads all tables, definitions and scripts
    // \param cp is the GM performing this full reload
    void reload_command(Player *cp);

    //! substitutes #j <name>, jump to a player of a given name
    // \param cp is the jumping GM
    // \param ts name of the player to jump to
    void jumpto_command(Player *cp, const std::string &ts);

    //! resambles the former #mapsave command, saves the map
    // \param cp the corresponding GM with correct rights who initiates this mapsave
    void save_command(Player *cp);

    //! Macht spieler cp unsichtbar
    void makeInvisible(Player *cp);

    void talkto_command(Player *cp, const std::string &ts);

    //! Macht spieler cp sichtbar
    void makeVisible(Player *cp);

    //! wirft alle Player aus dem Spiel
    void forceLogoutOfAllPlayers();

    //! wirft einen aktiven Player aus dem Spiel
    // \param name der Spieler der herausgeworfen werden soll
    // \return true falls der Player gefunden wurde, false sonst
    bool forceLogoutOfPlayer(std::string name);

    //! sendet einem Admin die Daten aller aktiven Player
    // \param admin der Admin an den die Daten gesandt werden sollen
    void sendAdminAllPlayerData(Player* &admin);

    // ! Server side implemented !warp_to x y z
    void warpto_command(Player *cp, const std::string &ts);

    // ! Server side implemented !summon Player
    void summon_command(Player *cp, const std::string &ts);

    // ! Server side implemented !prison Player
    void prison_command(Player *cp, const std::string &tplayer);

    // ! relaods only the definition from the db no Monsterspawns and no NPC's are loaded.
    bool reload_defs(Player *cp);

    // ! adds Warpfields to map from textfile
    bool importWarpFields(Player *cp,const std::string &filename);

    // ! Deletes a Warpfield
    void removeTeleporter(Player *cp, const std::string &ts);

    // ! Shows a list of Warpfields
    void showWarpFieldsInRange(Player *cp, const std::string &ts);
    //////////////////////////////////////////////////////////////////

    //////////// in WorldIMPLItemMoves.cpp ////////////////

    //! wird von den takeItem* und putItem* -Funktionen als Zwischenablage genutzt
    Item g_item;

    //! wird von den takeItem* und putItem* -Funktionen als Zwischenablage genutzt
    Container *g_cont;

    //! nimmt ein Item von der Karte und speichert es in g_item und ggf. g_cont
    // \param cc der Character der das Item anhebt
    // \param x alte X-Koordinate des Item
    // \param y alte Y-Koordinate des Item
    // \param z alte Z-Koordinate des Item
    // \return true falls erfolgreich, false sonst
    bool takeItemFromMap(Character *cc, short int x, short int y, short int z);

    //! wirft das Item aus g_item und ggf. g_cont auf Karte
    // \param cc der Character der das Item wegwirft
    // \param x neue X-Koordinate des Item
    // \param y neue Y-Koordinate des Item
    // \param z neue Z-Koordinate des Item
    // \return true falls erfolgreich, false sonst
    bool putItemOnMap(Character *cc, short int x, short int y, short int z);
    bool putItemAlwaysOnMap(Character *cc, short int x, short int y, short int z);

    //! nimmt ein Item aus dem Inventory von cc und speichert es in g_item und ggf. g_cont
    // \param cc der Character von dem das Item genommen wird
    // \param pos Position des Item im Inventory
    // \param count Anzahl
    // \return true falls erfolgreich, false sonst
    bool takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count);

    //! legt das Item aus g_item und ggf. g_cont an eine Inventoryposition
    // \param cc der Character der das Item bekommt
    // \param pos Position des Item im Inventory
    // \return true falls erfolgreich, false sonst
    bool putItemOnInvPos(Character *cc, unsigned char pos);

    //! nimmt ein Item aus dem Inventory von cc und speichert es in g_item und ggf. g_cont
    // und schickt ein Update an cc
    // \param cc der Player von dem das Item genommen wird
    // \param pos Position des Item im Inventory
    // \param count Anzahl
    // \return true falls erfolgreich, false sonst
    bool takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count);

    //! legt das Item aus g_item und ggf. g_cont an eine Inventoryposition
    // und schickt ein Update an cc
    // \param cc der Player der das Item bekommt
    // \param pos Position des Item im Inventory
    // \return true falls erfolgreich, false sonst
    bool putItemOnInvPos(Player *cc, unsigned char pos);

    //! close showcases of this container for other player
    void closeShowcaseForOthers(Player *target, Container *moved);

    //! close showcases for container for everyone not in range
    void closeShowcaseIfNotInRange(Container *moved, short int x, short int y, short int z);

    //! nimmt ein Item aus einem Schaukaste von cc und speichert es in g_item und ggf. g_cont
    // und schickt ein Update an cc
    // \param cc der Player von dem das Item genommen wird
    // \param showcase die Nummer das Schaukastens
    // \param pos Position des Item im showcase
    // \param count Anzahl
    // \return true falls erfolgreich, false sonst
    bool takeItemFromShowcase(Player *cc, unsigned char showcase, unsigned char pos, Item::number_type count);

    //! legt das Item aus g_item und ggf. g_cont in einen Schaukasten
    // und schickt ein Update an cc
    // \param cc der Player der das Item bekommt
    // \param showcase die Nummer das Schaukastens
    // \param die bevorzugte Position (um das Item mit einem auf der Position zusammenzufassen)
    // \return true falls erfolgreich, false sonst
    bool putItemInShowcase(Player *cc, unsigned char showcase, TYPE_OF_CONTAINERSLOTS pos);

    //! berprft ein Feld auf besondere Item und WarpFelder und fhrt entsprechende Aktionen aus
    // \param cfstart das zu berprfende Feld
    // \param x die X-Koordinate von cfstart
    // \param y die Y-Koordinate von cfstart
    // \param z die Z-Koordinate von cfstart
    void checkField(Field *cfstart, short int x, short int y, short int z);

    //! verschiebt das oberste Item des Feldes,welches in der durch d angegebene Richtung vom cp aus gesehen liegt,
    //  um das durch xo,yo,zo gegebene Offset
    // \param cc Zeiger auf den Character der die Verschiebung durchfhrt
    // \param d Richtung in der das Item liegt vom Player aus gesehen: 0->Norden bis 7->NO im Uhrzeigersinn, 8 nach oben, 9 nach unten
    // \param xc X-Zielkoordinate
    // \param yc Y-Zielkoordinate
    // \param zc Z-Zielkoordinate
    // \param count Anzahl
    // \return true falls Verschiebung ausgefhrt werden konnte, false sonst
    bool moveItem(Character *cc, unsigned char d, short int xc, short int yc, short int zc, Item::number_type count);

    //! ein Spieler verschiebt ein Item von der Karte in ein showcase
    // \param cp Player der verschiebt
    // \param direction Richtung in der das Item relativ zum Spieler liegt
    // \param showcase das showcase in welches das Item verschoben werden soll
    // \param pos Zielposition
    // \param count Anzahl
    void moveItemFromMapIntoShowcase(Player *cp, char direction, unsigned char showcase, unsigned char pos, Item::number_type count);

    //! ein Spieler verschiebt ein Item von der Karte an seine Koerper
    // \param cp Player der verschiebt
    // \param direction Richtung in der das Item relativ zum Spieler liegt
    // \param cpos Position am Koerper an welche das Item verschoben werden soll
    // \param count Anzahl
    void moveItemFromMapToPlayer(Player *cp, char direction, unsigned char cpos, Item::number_type count);

    //! ein Spieler verschiebt ein Item zwischen showcases
    // \param cp Player der verschiebt
    // \param source die Ansicht, aus welcher das Item verschoben wird
    // \param pos die Position (Nummer) des Item welches verschoben wird
    // \param dest die Ansicht, in welche das Item verschoben wird
    // \param pos2 die Position auf welches das Item verschoben werden soll
    // \parm count Anzahl
    void moveItemBetweenShowcases(Player *cp, unsigned char source, unsigned char pos, unsigned char dest, unsigned char pos2, Item::number_type count);

    //! wirft ein Item aus einem showcase auf die Karte
    // \param cp Player der das Item wirft
    // \param showcase die Ansicht, aus welcher das Item verschoben wird
    // \param pos die Position (Nummer) des Item welches verschoben wird
    // \param xc X-Koordinate der neuen Itemposition
    // \param yc Y-Koordinate der neuen Itemposition
    // \param zc Z-Koordinate der neuen Itemposition
    // \parm count Anzahl
    void dropItemFromShowcaseOnMap(Player *cp, unsigned char showcase, unsigned char pos, short int xc, short int yc, short int zc, Item::number_type count);

    //! verschiebt ein Item aus einem showcase an den Koerper des Player
    // \param cp Player der das Item verschiebt
    // \param showcase die Ansicht, aus der das Item verschoben werden soll
    // \param pos die Position des Item in der Ansicht
    // \param cpos die Position am Koerper wohin das Item verschoben wird
    // \param count Anzahl
    void moveItemFromShowcaseToPlayer(Player *cp, unsigned char showcase, unsigned char pos, unsigned char cpos, Item::number_type count);

    //! verschiebt ein Item von einem Koerperteil zu einem anderen
    // \param cp Player der das Item verschiebt
    // \param opos die Position des Item welches verschoben wird
    // \param npos die neue Position des Item
    // \parm count Anzahl
    void moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count);

    //! wirft eines der Item vom Koerper des Player auf die Karte
    // \param cp Player der das Item wirft
    // \param cpos die Position des Item welches verschoben wird
    // \param xc X-Koordinate der neuen Itemposition
    // \param yc Y-Koordinate der neuen Itemposition
    // \param zc Z-Koordinate der neuen Itemposition
    // \param count Anza
    void dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, short int xc, short int yc, short int zc, Item::number_type count);

    //! wirft eines der Item vom Koerper des Monster auf die Karte
    // \param cm Monster der das Item wirft
    // \param cpos die Position des Item welches verschoben wird
    // \param xo X-Offset der neuen Itemposition relativ zum Monster
    // \param yo Y-Offset der neuen Itemposition relativ zum Monster
    // \param zo Z-Offset der neuen Itemposition relativ zum Monster
    // \param count Anzahl
    void dropItemFromMonsterOnMap(Monster *cm, unsigned char cpos, char xo, char yo, char zo, Item::number_type count);

    //! ein Spieler verschiebt ein Item vom Koerper in ein showcase
    // \param cp Player der verschiebt
    // \param cpos die Position des Item welches verschoben wird
    // \param showcase das showcase in welches das Item verschoben werden soll
    // \param pos die Position auf welche das Item verschoben werden soll
    // \param count Anzahl
    void moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, unsigned char showcase, unsigned char pos, Item::number_type count);

    //! schickt eine 'Item entfernt' - Meldung an alle Player im Sichtbereich der Koordinate xo,yo,zo
    // \param id die Identifikationsnummer des durchfhrenden Player
    // \param xo X-Koordinate der alten Position des verschonbenen Item
    // \param yo Y-Koordinate der alten Position des verschonbenen Item
    // \param zo Z-Koordinate der alten Position des verschonbenen Item
    void sendRemoveItemFromMapToAllVisibleCharacters(TYPE_OF_ITEM_ID id, short int xo, short int yo, short int zo, Field *cfp);

    //! schickt eine 'neues Item auf der Karte' - Meldung an alle Player im Sichtbereich der Koordinate xn,yn,zn
    // \param xn X-Koordinate der Position des neuen Items
    // \param yn Y-Koordinate der Position des neuen Items
    // \param zn Z-Koordinate der Position des neuen Items
    // \param it Item welches auf die Karte gelegt wird
    void sendPutItemOnMapToAllVisibleCharacters(short int xn, short int yn, short int zn, Item &it, Field *cfp);

    /**
    *simulates a swap on a field (sends put and delete in fast time)
    */
    void sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, short int xn, short int yn, short int zn, Item &it, Field *cfp);


    //! benachrichtigt alle betroffenen Spieler von der Verschiebung des
    // Container moved aus dem Container cc
    // \param cc der Container dessen Inhalt sich aendert
    // \param moved der Container der verschoben wird
    void sendChangesOfContainerContentsCM(Container *cc, Container *moved);

    //! benachrichtigt alle betroffenen Spieler von der Aenderung des
    // Inhaltes des Container cc
    // \param cc der Container dessen Inhalt sich aendert
    void sendChangesOfContainerContentsIM(Container *cc);

    //! ein Spieler schaut in einen Container auf einem Feld
    // \param cp Player der sich den Inhalt ansieht
    // \param direction die Richtung in der der Container relativ zum Spieler liegt
    // \param showcase die Ansicht, in welcher der Inhalt des Container dargestellt werden soll
    // \return true, falls ein Container gefunden wurde
    bool lookIntoContainerOnField(Player *cp, char direction, unsigned char showcase);

    //! ein Spieler schaut in seinen Rucksack
    // \param cp Player der sich den Inhalt ansieht
    // \param showcase die Ansicht, in welcher der Inhalt des Container dargestellt werden soll
    // \return true, falls ein Container gefunden wurde
    bool lookIntoBackPack(Player *cp, unsigned char showcase);

    //deactivated, function is now in Player
    //bool lookIntoDepot( Player* cp, unsigned char showcase );

    //! ein Spieler schaut in einen Container in einem showcase
    // \param cp Player der sich den Inhalt ansieht
    // \param showcase die Ansicht, in welcher der Inhalt des Container dargestellt werden soll
    // \param pos die Position des Container im Container
    // \return true, falls ein Container gefunden wurde
    void lookIntoShowcaseContainer(Player *cp, unsigned char showcase, unsigned char pos);

    //! Prft ob ein Item stackable ist oder nicht
    //\param item das Item welches geprft werden soll
    //\return true, falls das Item Stapelbar ist.
    bool isStackable(Item item);

    //! ein Spieler schliesst einen Container in einem showcase
    // \param cp Player schliessender Spieler
    // \param showcase die Ansicht, in welcher der Inhalt des Container dargestellt werden soll
    void closeContainerInShowcase(Player *cp, unsigned char showcase);

    //! Array mit der Definition der Bewegung fr move
    char moveSteps[ 11 ][ 3 ]; // TODO move this to a better location

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
    bool createDynamicNPC(std::string name, Character::race_type type, position pos, /*CCharacter::face_to dir,*/ Character::sex_type sex, std::string scriptname);


    /**
    *calculates the line of sight between two positions and returns a list of all
    *objects which are in the way of sight to lua
    *@param startinpos the starting position for the line of sight
    *@param endingpos the ending position for the line of sight
    *@return a luabind list with all the item or characters in the way
    */
    luabind::object LuaLoS(position startingpos, position endingpos);

    /**
    *creates a luabind list with all the players which are currently online and returns it
    *to lua
    *@return the lua list with all the players currently online
    */
    luabind::object getPlayersOnline();

    /**
    *creates a luabind list with all the npcs which are currently online and returns it
    *to lua
    *@return the lua list with all the npcs currently online
    */
    luabind::object getNPCS();

    /**
    *creates a lua list with all the characters in range around a specific position
    *@param posi the position from where the character around should be calculated
    *@param range the range around the position for calculating
    *@return a lua list with all the characters (including monsters, npcs, players) around this char
    */
    luabind::object getCharactersInRangeOf(position posi, uint8_t range);
    luabind::object getPlayersInRangeOf(position posi, uint8_t range);
    luabind::object getMonstersInRangeOf(position posi, uint8_t range);
    luabind::object getNPCSInRangeOf(position posi, uint8_t range);

    //Sucht zu einem Item die gesamten Stats wie Gewicht heraus
    //\param item, das Item zu dem die Stats heraus gesucht werden sollen.
    CommonStruct getItemStats(ScriptItem item);
    CommonStruct getItemStatsFromId(TYPE_OF_ITEM_ID id);

    //Aendert die Qualitaet eines ScriptItems.
    //param item, das Item das geaendert werden soll
    //param amount, der Wert um den die Qualitaet geaendert werden soll
    void changeQuality(ScriptItem item, short int amount);

    void itemInform(Character *user, ScriptItem item, ItemLookAt lookAt);

    //Liefert den Namen eines Items mit einer bestimmten id zurck
    //\param itemid, id des items zu dem der Name geliefert werden soll
    //\param language, die Sprache in der der Name zurck gegeben werden sollte
    //\ret der name in der entsprechenden Sprache.
    std::string getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language);

    //Aendert ein ScriptItem
    bool changeItem(ScriptItem item);

    void changeQualityOfItemAt(position pos, short int amount);

    //Prft ob sich auf dem Feld ein Character befindet
    //\param position die zu prfen ist
    //\return true oder false
    bool isCharacterOnField(position pos);

    //Liefert einen Zeiger auf einen Character
    //\param pos, die Position auf der sich der Character befinden soll
    //\return Zeiger auf den Character
    fuse_ptr<Character> getCharacterOnField(position pos);

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
    ScriptItem createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool allways, int quali, const luabind::object &data);

    //Erzeugt auf den Angegebenen Feld ein bestimmtes Item
    //\ param item, das Item was erzeugt werden soll
    //\ param pos, Position des Items
    //\ return bool Wert der angibt ob das Erstellen geklappt hat.
    bool createFromItem(ScriptItem item, position pos, bool allways);

    //Erzeugt ein Monster mit der entsprechenden ID auf dem Feld
    //\ param id, das Monster welches Erzeugt werden soll
    //\ param pos, die Position des Monsters
    //\ return fuse_ptr<Character> Valid monster on success, invalid monster on failure
    fuse_ptr<Character> createMonster(unsigned short id, position pos, short movepoints);

    //Zeigt eine Grafik auf einem bestimmten Feld an
    //\gfxid, ID der anzuzeigenden Grafik
    //\pos, Position wo diese Grafik angezeigt werden soll
    void gfx(unsigned short int gfxid, position pos);

    //Spielt einen Soundeffekt auf einem bestimmten Feld ab
    //\soundid, ID des abzuspielenden Effektes
    //\pos, Position wo dieser Effekt abgespielt werden soll
    void makeSound(unsigned short int soundid, position pos);

    //!Prft ob auf einen FEld ein ITem liegt
    //\pos die Position des feldes
    //\return true wenn auf den Feld ein ITem liegt
    bool isItemOnField(position pos);

    //!Liefert ein Item zurck welches auf dem Feld liegt
    //\pos die Position des Feldes
    //\return das ScriptITem auf dem Feld;
    ScriptItem getItemOnField(position pos);

    //!Aendert ein Bodentile
    //\tileid, neue Id des bodentiles
    //\pos, Position des zu aendernden Bodentiles.
    void changeTile(short int tileid, position pos);


    //!Sendet ein Map update zu allen spielern in bereich range um pos
    //\pos, position von der ausgegangen wird
    //\bereich von dem die Spieler genommen werden sollen.
    void sendMapUpdate(position pos, uint8_t range);

    //!Sicherer CreateArea command, prft erst ab ob er eine vorhandene Map berschreibt.
    //\tileid: standard Tile
    //\pos: begin der neuen Karte
    //\height: Hoehe der neuen Karte
    //\width: breite der neuen Karte
    //\return true wenn das einfgen klappte, ansonsten false wenns zu berlagerungen kommt.
    bool createSavedArea(uint16_t tileid, position pos, uint16_t height, uint16_t width);

    //! Laedt eine Armor Struct anhand einer id.
    //\return true false der Armorstruct gefunden wurde
    bool getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret);

    //! Laedt einen Weapon Struct anhand einer id.
    //\return true false der WeaponStruct gefunden wurde.
    bool getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret);

    //! Laedt ein MonsterRst Struct anhand einer id.
    //\return true false der Struct gefunden wurde.
    bool getNaturalArmor(Character::race_type id, MonsterArmor &ret);

    //! Laedt ein Attack Boni struct anhand einer id
    //\return true wenn das Struct gefunden wurde
    bool getMonsterAttack(Character::race_type id, AttackBoni &ret);

    /**
    *sends a Message to All Monitoring Clients
    *@param msg the message string which should be sended
    *@param id the id of the msg ( 1 are message which displayed in a window 0 basic message)
    */
    void sendMonitoringMessage(std::string msg, unsigned char id);

    /**
     * bans a player for the bantime
     * @param cp pointer to the player which should been banned
     * @param bantime in seconds
     * @gmid the id of the gm which has banned the player
     */
    void ban(Player *cp, int bantime, TYPE_OF_CHARACTER_ID gmid);

private:
    void logMissingField(const std::string &function, const position &field);

private:

    /**
    * the constructor of World, private because of singleton pattern
    *
    *@param dir the main directory of the server
    *@param time_t the starting time of the server
    */
    World(std::string dir, time_t starttime);
    World &operator = (const World &other) {
        return *_self;
    }
    World(const World &other) {}

    static World *_self;

    std::string zuschwer[ 3 ];

    //! IG day of last turntheworld
    int lastTurnIGDay;

    //! Timer fr die Monster - Respawn
    Timer *monstertimer;

    //! Timer fr Effekte auf Feldern
    Timer *fieldtimer[ 3 ];

    //! Timer fr die NPC
    MilTimer *npctimer;

    //! Timer fr den Scheduler
    Timer *schedulertimer;

    Timer *ScriptTimer; //< Tuner for scheduled scripts.

    MilTimer *monitoringclienttimer;

    //! zur durchfhrung aller Langzeiteffekte
    void do_LongTimeEffects(Character *cc);

    //! das home-Verzeichnis des Servers
    std::string directory;

    //! Zeitpunkt der letzten Alterung (Anzahl der Sekunden seit 1.1.1970)
    time_t last_age;     //(32 Bit Integer)

    //! Anzahl der Sekunden zwischen den Alterungsschritten
    static const long gap = 180;

    //! X-Koordinate bei der die Alterung fortgesetzt wird
    short int nextXtoage;

    //! X-Koordinate nach der die Alterung gestoppt wird
    short int lastXtoage;

    //! entspricht (Anzahl-1) Spalten, die in einem Durchlauf gealtert werden
    unsigned short int ammount;

    //! Breite der zu alternden Karte
    unsigned short int Width;

    //! Hoehe der zu alternden Karte
    unsigned short int Height;

    //! Sekunden seit der letzten vollstaendigen Alterung der Karte
    long realgap;

    //! Anzahl der kommpletten Durchlaeufe durch die Karte, seit die altersresistentesten Items gealtert wurden
    unsigned short int timecount;

    //! ages all world items if necessary
    // \return true iff any aging was done
    bool DoAge();

    //! Fhrt die Alterung der Item im Inventory aller Player durch
    // und schickt ggf. ein Update an die Spieler
    // \param funct
    // \see Item.h
    void AgeInventory();

    //! das Verzeichnis mit den Skripten
    std::string scriptDir;

    // spawnplaces...
    std::list<SpawnPoint> SpawnList;

    // initmethod for spawn places...
    bool initRespawns();

    // Convert a std::string to a short int
    bool ReadField(const char *inp, signed short int &outp);
    // Convert a std::string to a long short int
    bool ReadField(const char *inp, signed long int &outp);

    // hashmap containing all GM Commands
    std::map< std::string, CommandType * > GMCommands;
    // hashmap containing all Player Commands
    std::map< std::string, CommandType * > PlayerCommands;

    typedef std::map< std::string, CommandType * >::iterator CommandIterator;

    // Send player information to GMs
    void who_command(Player *cp, const std::string &tplayer);

    // Ban a player
    void ban_command(Player *cp, const std::string &timeplayer);
    void banbyname(Player *cp, short int banhours, std::string tplayer);
    void banbynumber(Player *cp, short int banhours, TYPE_OF_CHARACTER_ID tid);


    // Change tile in front of admin
    void tile_command(Player *cp, const std::string &ttilenumber);

    // Turtle tile commands
    void turtleon_command(Player *cp, const std::string &ttilenumber);
    void turtleoff_command(Player *cp);

    // Clipping on/off command
    void clippingon_command(Player *cp);
    void clippingoff_command(Player *cp);

    // Describe to admin tile in front of them
    void what_command(Player *cp);

    // Save all online players
    void playersave_command(Player *cp);

    // Create telport warp on current tile to x, y, z
    void teleport_command(Player *cp, const std::string &tplayer);

    // Give help for GM commands
    void gmhelp_command(Player *cp);

    //Sendet eine Nachricht an alle GM's
    bool gmpage_command(Player *player, const std::string &ticket);

public:
    void logGMTicket(Player *Player, const std::string &ticket, const std::string &prefix);

private:
    bool active_language_command(Player *cp, const std::string &language);

    //Fgt einen Spieler unter falschen namen in die Eigene Namensliste hinzu
    void name_command(Player *cp, const std::string &ts);

    // Allow players to change title prefix and suffix
    bool prefix_command(Player *cp, const std::string &tprefix);
    bool suffix_command(Player *cp, const std::string &tprefix);
    // prison command for players
    bool player_prison_command(Player *cp, const std::string &input);

    // register any GM commands here...
    void InitGMCommands();
    // register any Player commands here...
    void InitPlayerCommands();

    // export maps to mapdir/export
    bool exportMaps(Player *cp);

    //! reload all tables
    bool reload_tables(Player *cp);
};


#define MAX_PATH_FIND 50

/**
* struct which holds two koordinates and a distance
*/
struct feld {
    short int x,y; /**< the x,y koordinates*/
    short int cost, estCost, totalCost;  /**< movement costs and estimated movement costs */
    feld *parent;
};

/**
* a list of fields which should be searched
* AFAIK for the A pathfinding algorithm
*/
typedef std::vector<feld> SuchFeldListe;

#endif
