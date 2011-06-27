//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "db/ConnectionManager.hpp"

#include "World.hpp"
//#include "netinterface/ClientCommands.hpp"
#include "WeaponObjectTable.hpp"
#include "TilesTable.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "MonsterTable.hpp"
#include "TableStructs.hpp"
#include "CommonObjectTable.hpp"
#include "Logger.hpp"
//#include "MonitoringClients.hpp"
//#include "mtprotocol/CServerCommands.hpp"
#include "netinterface/BasicCommand.hpp"
#include "LongTimeAction.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "PlayerManager.hpp"
#include "WaypointList.hpp"




#include <dirent.h>
#include <sys/types.h>
#include <regex.h>
#include <algorithm>

//#define CWorld_DEBUG
// a map storing configuration options from a config file...
extern std::map<std::string, std::string> configOptions;
//Table with data of Monsters
extern CMonsterTable* MonsterDescriptions;
//Table with Item Descriptions
extern CCommonObjectTable * CommonItems;
//Table with Tile Descriptions
extern CTilesTable * Tiles;
//Map with opitions which messages are logged or not
//! eine Tabelle fr Waffen - Item Daten
extern CWeaponObjectTable* WeaponItems;

extern std::ofstream talkfile;

CWorld::CWorld * CWorld::_self;

CWorld * CWorld::create( std::string dir, time_t starttime)
{
    if ( !(_self) ) 
    {
        _self = new CWorld(dir,starttime);
        // init spawnlocations...
        _self->initRespawns();
        // initialise list of GM Commands
        _self->InitGMCommands();
        // initialise list of Player Commands
        _self->InitPlayerCommands();
        _self->monitoringClientList = new CMonitoringClients( _self );
        
    }
    return _self;
}

CWorld * CWorld::get() throw(std::runtime_error) 
{
    if ( !(_self) )throw std::runtime_error("world was not created");
    return _self; 
} 

CWorld::CWorld( std::string dir, time_t starttime ) {

    CLogger::writeMessage("CWorld_Debug","CWorld Konstruktor Start");
    start.time=starttime;
    start.millitm=0;
    nextXtoage = 0;
    
    lastTurnIGDay=getTime("day");

    last= 0;
    
    currentScript = NULL;
    
    directory = dir;
    scriptDir = dir + std::string( SCRIPTSDIR );

    fieldtimer[ 0 ] = new CTimer( 5 );     // 5 s
    fieldtimer[ 1 ] = new CTimer( 5 );     // 5 s
    fieldtimer[ 2 ] = new CTimer( 5 );     // 5 s

    monstertimer = new CTimer( 10 );     // 60 s

    schedulertimer = new CTimer( 1 );    // 1 s
    
    ScriptTimer = new CTimer( 1 ); //1 s

    npctimer = new CMilTimer( 2000 );       // 2 s
    
    monitoringclienttimer = new CMilTimer( 250 );

    gap = TIMEGAP;
    timecount = 1;
    last_age = time( NULL );
    ammount = 50;
    AgeItem = AgeItemUpStep1;

    srand( ( unsigned ) time( NULL ) );

    // Bewegung nach Norden
    moveSteps[ 0 ][ 0 ] = 0;      // x - Koordinate
    moveSteps[ 0 ][ 1 ] = -1;     // y - Koordinate
    moveSteps[ 0 ][ 2 ] = 0;      // z - Koordinate

    // NO
    moveSteps[ 1 ][ 0 ] = 1;
    moveSteps[ 1 ][ 1 ] = -1;
    moveSteps[ 1 ][ 2 ] = 0;

    // O
    moveSteps[ 2 ][ 0 ] = 1;
    moveSteps[ 2 ][ 1 ] = 0;
    moveSteps[ 2 ][ 2 ] = 0;

    // SO
    moveSteps[ 3 ][ 0 ] = 1;
    moveSteps[ 3 ][ 1 ] = 1;
    moveSteps[ 3 ][ 2 ] = 0;

    // S
    moveSteps[ 4 ][ 0 ] = 0;
    moveSteps[ 4 ][ 1 ] = 1;
    moveSteps[ 4 ][ 2 ] = 0;

    // SW
    moveSteps[ 5 ][ 0 ] = -1;
    moveSteps[ 5 ][ 1 ] = 1;
    moveSteps[ 5 ][ 2 ] = 0;

    // W
    moveSteps[ 6 ][ 0 ] = -1;
    moveSteps[ 6 ][ 1 ] = 0;
    moveSteps[ 6 ][ 2 ] = 0;

    // NW
    moveSteps[ 7 ][ 0 ] = -1;
    moveSteps[ 7 ][ 1 ] = -1;
    moveSteps[ 7 ][ 2 ] = 0;

    // up
    moveSteps[ 8 ][ 0 ] = 0;
    moveSteps[ 8 ][ 1 ] = 0;
    moveSteps[ 8 ][ 2 ] = 1;

    // down
    moveSteps[ 9 ][ 0 ] = 0;
    moveSteps[ 9 ][ 1 ] = 0;
    moveSteps[ 9 ][ 2 ] = -1;

    // no move
    moveSteps[ 10 ][ 0 ] = 0;
    moveSteps[ 10 ][ 1 ] = 0;
    moveSteps[ 10 ][ 2 ] = 0;

    npcidc   = new CIdCounter( dir + std::string( NPCDIR ) + std::string( NPCCOUNTERNAME ), NPC_BASE);

    buy[ CLanguage::german ] = "[NPC] Benutze mich, um etwas zu kaufen.";
    buy[ CLanguage::english ] = "[NPC] Use me to buy something.";
    buy[ CLanguage::french ] = "[NPC] Employez-moi pour acheter quelque chose.";

    sell[ CLanguage::german ] = "[NPC] Benutze mich mit einem Gegenstand, um diesen zu verkaufen.";
    sell[ CLanguage::english ] = "[NPC] Use me with an item to sell it.";
    sell[ CLanguage::french ] = "[NPC] Employez-moi avec un article pour le vendre.";

    welcome[ CLanguage::german ] = ":) Willkommen in Illarion, es sind %i andere Spieler online.";
    welcome[ CLanguage::english ] = ":) Welcome to Illarion. There are %i other players online.";
    welcome[ CLanguage::french ] = ":) Bienvenue �Illarion.  Il y a %i autres joueurs en ligne.";

    zuschwer[ CLanguage::german ] = "Du kannst nicht so viel tragen!";
    zuschwer[ CLanguage::english ] = "You can't carry that much!";
    zuschwer[ CLanguage::french ] = "Vous ne pouvez pas porter que beaucoup!";

    unsigned int templi = starttime;
    char temparr[ 80 ];
    sprintf ( temparr, "%u", templi );
    //Kann ggf deaktiviert werden aufgrund des Loggers der diese bernehmen wird.
    //-----------------------------------------------------------------------------
    talkfile.open( ( dir + std::string( TALKDIR ) + std::string ( temparr ) + std::string( ".talk" ) ).c_str() , std::ios::out | std::ios::app );
    talkfile  << "Start " << ctime( &starttime ) << " ----------------------------------" << std::endl;
    //-----------------------------------------------------------------------------
    CLogger::writeMessage("CWorld_Debug", "CWorld Konstruktor Ende");
}


struct editor_maptile {
    int32_t x;
    int32_t y;
    unsigned short fieldID;
    unsigned short int musicID;
};

int mapfilter(const struct dirent *d) {
    return (0 == strstr(d->d_name, ".tiles.txt"))?0:1;
}

bool CWorld::load_maps() 
{
    // get all tiles files
    struct dirent** maplist;
    int numfiles = scandir((configOptions["datadir"] + "map/import/").c_str(), &maplist, mapfilter, alphasort);

    if (numfiles <= 0) 
    {
        perror("Could not import maps");
        return false;
    }

    bool ok = true;

    // iterate over all map files...
    while (numfiles--) 
    {

        // strip .tiles.txt from filename
        strstr(maplist[numfiles]->d_name, ".tiles.txt")[0] = '\0';
        CLogger::writeMessage("World_Imports", "importing: " +  configOptions["datadir"] + "map/import/" + maplist[numfiles]->d_name);

        if ( std::string(maplist[numfiles]->d_name).compare("oberwelt_0") )
        ok &= load_from_editor(configOptions["datadir"] + "map/import/" + maplist[numfiles]->d_name);

    }

    return ok;

}

//! create a new world from editor files (new format)
bool CWorld::load_from_editor(std::string filename)
{
    // first try to open mapfile
    CLogger::writeMessage("World_Imports", "try to Import map: " + filename);
    std::ifstream maptilesfile((filename + ".tiles.txt").c_str());
    if (!maptilesfile.good()) {
        CLogger::writeError("World_Imports", "could not open file: " + filename + ".tiles.txt");
        return false;
    }

    int mapstartx = -1, mapstarty = -1;

    std::map<unsigned int, editor_maptile> maptiles;

    editor_maptile temp_tile;

    char dummy;

        int h_level, h_x, h_y, h_width, h_height, oldy;

    // load map file header information
    maptilesfile >> dummy; maptilesfile >> dummy;  // read 'L: '
    maptilesfile >> h_level;  //read int (level)
    maptilesfile >> dummy; maptilesfile >> dummy;  // read 'X: '
    maptilesfile >> h_x;  //read int (x coord)
    maptilesfile >> dummy; maptilesfile >> dummy;  // read 'Y: '
    maptilesfile >> h_y;  //read int (y coord)
    maptilesfile >> dummy; maptilesfile >> dummy;  // read 'W: '
    maptilesfile >> h_width;  //read int (width)
    maptilesfile >> dummy; maptilesfile >> dummy;  // read 'H: '
    maptilesfile >> h_height;  //read int (height)
    oldy = -1;
    
    CLogger::writeMessage("World_Imports", "try to Import tiles: " + filename );
    // load all tiles from the file
    maptilesfile >> temp_tile.x;  // read an int.
    while (maptilesfile.good()) {
        if (mapstartx == -1)
            mapstartx = temp_tile.x;
        temp_tile.x -= mapstartx;
        std::string LogMessage="x: " + CLogger::toString(temp_tile.x) + " ";

        maptilesfile >> dummy;   // read a char (;)
        if (dummy != ';') {
            CLogger::writeError("World_Imports","maptile file contains errors! : " + CLogger::toString(dummy));
            return false;
        }

        maptilesfile >> temp_tile.y;    //read next int
        if (oldy != temp_tile.y) //log if we have read one complete line of the map
        {
            CLogger::writeMessage("World_Imports","currently reading the y lines for tiles : " + CLogger::toString(temp_tile.y),false);
            oldy = temp_tile.y;
        }
        if (mapstarty == -1)
            mapstarty = temp_tile.y;
        temp_tile.y -= mapstarty;
        LogMessage += "y: " + CLogger::toString(temp_tile.y) + " ";

        maptilesfile >> dummy;          // read a char (;)
        if (dummy != ';') {
            CLogger::writeError("World_Imports","maptile file contains errors! : " + CLogger::toString(dummy));
            return false;
        }

        maptilesfile >> temp_tile.fieldID;      // read an int (tile-id)
        LogMessage += "id: " + CLogger::toString(temp_tile.fieldID);

        maptilesfile >> dummy;          // read a char (;)
        if (dummy != ';') {
            CLogger::writeError("World_Imports","maptile file contains errors! : " + CLogger::toString(dummy));
            return false;
        }

        maptilesfile >> temp_tile.musicID;      // read a short uint (music-id)
        LogMessage += "musicID: " + CLogger::toString(temp_tile.musicID);

        maptilesfile >> dummy;          // read a char (;)
        if (dummy != ';') {
            CLogger::writeError("World_Imports","maptile file contains errors! : " + CLogger::toString(dummy));
            return false;
        }

        unsigned short int dummyvalue;
        maptilesfile >> dummyvalue;      // read a short uint (dummy)

        //CLogger::writeMessage("World_Imports",LogMessage, false);
        // store the tile in our map
        maptiles[temp_tile.x<<16|temp_tile.y] = temp_tile;

        maptilesfile >> temp_tile.x;    // read next x (int); if there is none, while will end
    }
    CLogger::writeMessage("World_Imports","maptilesfile was bad at x=" + CLogger::toString(temp_tile.x) + " y=" + CLogger::toString(temp_tile.y));


    // generate new map
    CMap* tempmap = new CMap(h_width, h_height);
    bool disappear=true;
    tempmap->Init(h_x, h_y, h_level, disappear);

    CField* tempf;
    int index_start;
    for (int x=0; x<h_width; ++x) {
        index_start = x << 16;
        for (int y=0; y<h_height; ++y) {
            temp_tile = maptiles[index_start | y];
            if ( tempmap->GetPToCFieldAt(tempf, temp_tile.x+h_x, temp_tile.y+h_y) ) {
                tempf->setTileId( temp_tile.fieldID );
                tempf->setMusicId( temp_tile.musicID );
                tempf->updateFlags();
            } 
            else 
            { 
                CLogger::writeError("CWorld", "could not get field for: " + CLogger::toString(x) + " " + CLogger::toString(y), false);
                CLogger::writeError("World_Imports", "could not get field for: " + CLogger::toString(x) + " " + CLogger::toString(y), false);
            }
        }
    }
    maptilesfile.close();
    maps.InsertMap( tempmap );

    // now try to load warpfields
    std::ifstream warpfile((filename + ".warps.txt").c_str());
    if( !warpfile.good() )
    {
        CLogger::writeError("World_Imports","could not open file: " + filename + ".warps.txt");
        return true;    // warps are not crucial
    }
    position start, target;
    start.z = h_level;
    warpfile >> start.x;
    while( warpfile.good() )
    {
        warpfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports","warp file contains errors! : " + CLogger::toString(dummy));
            return false; 
        }
        warpfile >> start.y;
        warpfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports","warp file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        warpfile >> target.x;
        warpfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports","warp file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        warpfile >> target.y;
        warpfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports","warp file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        warpfile >> target.z;

        start.x += h_x;
        start.y += h_y;
        GetField( start )->SetWarpField( target );

        warpfile >> start.x;
    }

    // next we try to load the items for the map
    std::ifstream mapitemsfile((filename + ".items.txt").c_str());
    if (!mapitemsfile.good()) {
        CLogger::writeError("World_Imports","could not open file: " + filename + ".items.txt");
        return true;    // items are not crucial
    }

    unsigned short dummy_specialflags;

    int x,y,z;
        z = 0;
    int lastx=-1, lasty=-1;
    //CField* cfp;
    Item it;
    oldy = -1;
    CLogger::writeMessage("World_Imports", "try to import items: " + filename);
    mapitemsfile >> x;
    while (mapitemsfile.good()) {
        it.quality = 333;
        it.wear = 255;
        it.number = 1;
        x -= mapstartx;
        x += h_x;
        std::string LogMessage =  "item: x: " + CLogger::toString(x) +  " ";
        mapitemsfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports", "mapitem file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        mapitemsfile >> y;
        if (oldy != y) //log if we have read one complete line of the map
        {
            CLogger::writeMessage("World_Imports","currently reading the y lines for items : " + CLogger::toString(y),false);
            oldy = y;
        }
        y -= mapstarty;
        y += h_y;
        LogMessage += "y: " + CLogger::toString(y) + " ";
        mapitemsfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports", "mapitem file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        if (lastx==x && lasty==y) {
            ++z;
        } else {
            z=0;
        }
        lastx=x; lasty=y;

        mapitemsfile >> dummy_specialflags;
        LogMessage += "specialflags: " + CLogger::toString(z) + " ";
        mapitemsfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports", "mapitem file contains errors! : " + CLogger::toString(dummy));
            return false;
        }

        mapitemsfile >> it.id;
        LogMessage += "id: " + CLogger::toString(it.id) + " ";

        mapitemsfile >> dummy;
        if (dummy != ';') {
            CLogger::writeError("World_Imports", "mapitem file contains errors! : " + CLogger::toString(dummy));
            return false;
        }
        
        //mapitemsfile >> it.data;
        uint32_t datatmp;
        mapitemsfile >> datatmp;
        
        it.setData(datatmp);
        
        LogMessage += "data: " + CLogger::toString(datatmp) + " ";

        if (mapitemsfile.good()) {
            if (mapitemsfile.get() == ';') {
                        mapitemsfile >> it.quality;
                        LogMessage += "quality: " + CLogger::toString(it.quality) + " ";
            } else {
                mapitemsfile.unget();
            }
        }

        // default value while mapeditor is out of order
        it.quality = 333;

        //if ( LogOptions["World_Imports"] )
        //  CLogger::writeMessage("World_Imports", LogMessage);

        // store the item in our map
        g_item = it;
        g_cont = NULL;
        if (!putItemAlwaysOnMap(NULL, x, y, h_level))
            CLogger::writeMessage("World_Imports", "could not put item");
        mapitemsfile >> x;
    }
    mapitemsfile.close();
    CLogger::writeMessage("World_Imports", "Import map: " + filename + " was successful!");

    return true;
}

CWorld::~CWorld() {
     CLogger::writeMessage("CWorld_Debug","CWorld Destruktor Start");

    for ( CMapVector::iterator mapit = maps.begin(); mapit != maps.end(); ++mapit ) {
        delete ( *mapit );
        ( *mapit ) = NULL;
    }
    delete monstertimer;
    monstertimer = NULL;
    delete npctimer;
    npctimer = NULL;
    delete fieldtimer[ 0 ];
    fieldtimer[ 0 ] = NULL;
    delete fieldtimer[ 1 ];
    fieldtimer[ 1 ] = NULL;
    delete fieldtimer[ 2 ];
    fieldtimer[ 2 ] = NULL;
    delete npcidc;
    npcidc = NULL;
    delete monitoringclienttimer;
    monitoringclienttimer = NULL;
    delete monitoringClientList;
    monitoringClientList = NULL;

    time_t acttime4;
    time( &acttime4 );
    talkfile << "Ende  " << ctime( &acttime4 ) << " ----------------------------------" << std::endl;

    delete Scheduler;
    CLogger::writeMessage("CWorld_Debug", "CWorld Destruktor Ende");
}


void CWorld::turntheworld() {
    ftime( &now );

    unsigned long temp = ( ( now.time - start.time ) * 1000 ) + ( now.millitm - start.millitm );
    int thisIGDay = getTime("day");
    if (lastTurnIGDay!=thisIGDay) {
        sendIGTimeToAllPlayers();
        CLogger::writeMessage("CWorld_Debug", "lastTurnIGDay=" + CLogger::toString(lastTurnIGDay) + " thisIGDay= "+CLogger::toString(thisIGDay));
        lastTurnIGDay=thisIGDay;
    }
    ap = (temp / 100) - last;     // 1 actionPoint == 1/10 s

    if ( ap > MIN_AP_UPDATE ) {
        last = temp /100;
        
        checkPlayers();
        checkMonsters();
        checkNPC();
        if ( monitoringclienttimer->Next() ) monitoringClientList->CheckClients();
        //N�hsten Scheduler Cycle einleiten nur zu Testzwecke
        //Der Scheduler soll in einen eigenen Thread laufen.
        if( schedulertimer->next() ) Scheduler->NextCycle();
        if (ScriptTimer->next() ) ScheduledScriptsTable->nextCycle();
    }

    DoAge();
}



void CWorld::checkPlayers() 
{
    static boost::shared_ptr<CLuaLogoutScript>logoutScript( new CLuaLogoutScript( "server.logout" ) );
    
    bool effect = false;
    if ( fieldtimer[ 0 ]->next() ) {
        effect = true;
    }

    time_t tempkeepalive;
    time( &tempkeepalive );
    int temptime;

    PLAYERVECTOR::iterator playerIterator = Players.begin();
    while ( playerIterator < Players.end() ) {
        if ( ( *playerIterator )->Connection->online ) {
            temptime = tempkeepalive - ( *playerIterator )->lastkeepalive;
            if ( (( temptime >= 0 ) && ( temptime <= 15 )) ) // 15s timeout
            {
                ( *playerIterator )->actionPoints += ap;
                ( *playerIterator )->fightPoints += ap;
                
                if ( ( *playerIterator )->actionPoints > P_MAX_AP ) 
                {
                    ( *playerIterator )->actionPoints = P_MAX_AP;
                }

                if ( ( *playerIterator )->fightPoints > P_MAX_FP )
                {
                    ( *playerIterator )->fightPoints = P_MAX_FP;
                }

                if ( ( *playerIterator )->actionPoints >= P_MIN_AP ) {
                    ( *playerIterator)->workoutCommands();
                }
                ( *playerIterator )->ltAction->checkAction();
                ( *playerIterator )->effects->checkEffects();
                if ( effect ) do_LongTimeEffects( *playerIterator );
                

            }
            // User timed out.
            else {
                CLogger::writeMessage("CWorld",  "Player " + ( *playerIterator )->name + " Timed Out " + CLogger::toString(temptime));
                boost::shared_ptr<CBasicServerCommand>cmd( new CLogOutTC(UNSTABLECONNECTION) );
                ( *playerIterator )->Connection->shutdownSend( cmd );
            }
            playerIterator++;
        } else {
            std::string temp_name = ( *playerIterator )->name;
            TYPE_OF_CHARACTER_ID temp_id = ( *playerIterator )->id;

            position temp_pos = ( *playerIterator )->pos;

            CLogger::writeMessage("CWorld", "Player " + temp_name + " is offline");
            CField* tempf;
            if ( GetPToCFieldAt( tempf, ( *playerIterator )->pos.x, ( *playerIterator )->pos.y, ( *playerIterator )->pos.z ) ) {
                tempf->SetPlayerOnField( false );
            }
            
            std::cout<<"logout of "<<(*playerIterator)->name<<std::endl;
            try
            {
                std::cout<<"calling on logout!"<<std::endl;
                logoutScript->onLogout(( *playerIterator ));
            }
            catch (ScriptException &e)
            {
                std::cerr<<"Logout Script: Failure on calling on logout!"<<std::endl;
            }            
            
            CPlayerManager::get()->getLogOutPlayers().non_block_push_back( *playerIterator );
            playerIterator = Players.erase( playerIterator );
            sendRemoveCharToVisiblePlayers( temp_id, temp_pos );
        }
    }
}

// init the respawn locations... for now still hardcoded...
bool CWorld::initRespawns() {
    // if we have monsters, we need to delete their spawnpoints...
    for ( MONSTERVECTOR::iterator monsterIterator = Monsters.begin(); monsterIterator != Monsters.end(); ++monsterIterator) {
        // if the spawn is set to NULL it is regarded as no spawnpoint.
        (*monsterIterator)->remove();
        (*monsterIterator)->setSpawn(NULL);
    }
    SpawnList.clear();

    // read spawnpoints from db

    try {
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

        std::vector<TYPE_OF_ITEM_ID> ids;
        std::vector<int> pos[3];
        std::vector<uint16_t> range;
        std::vector<uint16_t> spawnrange;
        std::vector<uint16_t> min_spawntime;
        std::vector<uint16_t> max_spawntime;
        std::vector<bool> spawnall;

        size_t rows = di::select_all<
                      di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Boolean
                      >(transaction, ids, pos[0], pos[1], pos[2], range, spawnrange, min_spawntime, max_spawntime,spawnall,
                        "SELECT spp_id, spp_x, spp_y, spp_z, spp_range, spp_spawnrange, spp_minspawntime, spp_maxspawntime, spp_spawnall FROM spawnpoint");

        if (rows > 0) {
            for (size_t i = 0; i < rows; ++i) 
            {
                position the_pos(pos[0][i], pos[1][i], pos[2][i]);
                CSpawnPoint newSpawn( the_pos , range[i], spawnrange[i], min_spawntime[i], max_spawntime[i], spawnall[i]);
                CLogger::writeMessage("World_Inits", "load spawnpoint: " + CLogger::toString(ids[i]));
                newSpawn.load(ids[i]);
                SpawnList.push_back(newSpawn);
                CLogger::writeMessage("World_Inits", "added spawnpoint " + CLogger::toString(pos[0][i]) + "," + CLogger::toString(pos[1][i]) + "," + CLogger::toString(pos[2][i]));
            }

        } else {
            return false;
        }

        return true; // everything went well
    } catch (std::exception e) {
        CLogger::writeError("World_Inits","got exception in load SpawnPoints: " + CLogger::toString(e.what()) );
            return false;
    }

}

void CWorld::checkMonsters() {

    // respawn ?
    if ( monstertimer->next() ) 
    {
        std::list<CSpawnPoint>::iterator it;
        if (configOptions["do_spawn"] != "false") {
            for ( it = SpawnList.begin(); it != SpawnList.end(); ++it ) 
            {
                it->spawn();
            }
        } else {
            CLogger::writeMessage("Monster","CWorld::checkMonsters() spawning disabled!");
        }
    }

    bool effect = false;
    if ( fieldtimer[ 1 ]->next() ) {
        effect = true;
    }

    std::vector < CPlayer* > temp;
    MONSTERVECTOR::iterator monsterIterator = Monsters.begin();

    if (ap > 1) {
        --ap;
    }
    while ( monsterIterator < Monsters.end() ) 
    {
        if ( ( *monsterIterator )->IsAlive() )
        {               // monster alive
            ( *monsterIterator )->actionPoints += ap;
            ( *monsterIterator )->fightPoints += ap;

            if ( ( *monsterIterator )->actionPoints > NP_MAX_AP ) {     // too many AP
                ( *monsterIterator )->actionPoints = NP_MAX_AP;
            }

            if ( ( *monsterIterator )->fightPoints > NP_MAX_FP ) {     // too many FP
                ( *monsterIterator )->fightPoints = NP_MAX_FP;
            }
            
            (*monsterIterator)->effects->checkEffects();
            
            MonsterStruct monStruct;
            bool foundMonster = MonsterDescriptions->find( (*monsterIterator)->getType(), monStruct ) ;

            if ( ( *monsterIterator )->actionPoints >= NP_MIN_AP ) 
            {
				if ( !(*monsterIterator)->getOnRoute() )
				{
					//set lastTargetSeen to false if we reach the position where the target was seen the last time
					if ( ( *monsterIterator )->pos == (*monsterIterator)->lastTargetPosition ) (*monsterIterator )->lastTargetSeen = false;
					// enough AP
					//searh for all players which can be attacked from the monster directly
					
					//get attackrange of the weapon
					Item itl = (*monsterIterator)->GetItemAt(LEFT_TOOL);
					Item itr = (*monsterIterator)->GetItemAt(RIGHT_TOOL);
					
					WeaponStruct theWeapon;
					uint16_t range=1;
					
					if (WeaponItems->find( itr.id, theWeapon ))
						range=theWeapon.Range;
					else if (WeaponItems->find( itl.id, theWeapon ))
						range=theWeapon.Range;
					//===============================================
					temp.clear();           
					//temp = Players.findAllAliveCharactersInRangeOf(( *monsterIterator )->pos.x, ( *monsterIterator )->pos.y, ( *monsterIterator )->pos.z, range );
					findPlayersInSight( (*monsterIterator)->pos, static_cast<uint8_t>(range), temp, (*monsterIterator)->faceto);
					bool has_attacked=false;
					//If we have found players which can be attacked directly and the monster can attack
					CPlayer * foundP = 0;
                    if (( !temp.empty() ) && ( *monsterIterator )->canAttack() ) 
					{ //angreifen
						//search for the target via script or the player with the lowest hp
                        if( !monStruct.script || !monStruct.script->setTarget( *monsterIterator, temp, foundP ) )
                        {
                            findPlayerWithLowestHP( &temp, foundP );
                        }
						if ( foundP ) 
						{
							//let the monster attack the player with the lowest hp->assigned this player as target
							( *monsterIterator )->enemyid = foundP->id;
							( *monsterIterator )->enemytype = CCharacter::player;
							( *monsterIterator )->lastTargetPosition = foundP->pos;
							( *monsterIterator )->lastTargetSeen = true;
							if ( foundMonster )
							{
							   //check if we have a pointer to a script 
							   if ( monStruct.script )
							   {
								   //Wenn Scriptaufruf erfolgreich den aktuellen schleifenablauf abbrechen.           
								   if ( monStruct.script->enemyNear( (*monsterIterator) ,foundP) )
								   { 
									   continue; //Schleife fr dieses Monster abbrechen. Da es schon etwas diesne Schleifendurchlauf getan hat.
								   }
							   }                 
						   }
						   else
						   CLogger::writeError("Monster","cant find a monster id for checking the script!");
						   //attack the player which we have found
						   (*monsterIterator)->turn( foundP->pos );
                           if( ( *monsterIterator )->fightPoints >= NP_MIN_FP )   // enough FP to fight?
						       has_attacked = characterAttacks( *monsterIterator );
                           else
                               has_attacked = true;
						}
					}

					if (!has_attacked) { //bewegen
						temp.clear();
						findPlayersInSight( (*monsterIterator)->pos, static_cast<uint8_t>(9), temp, (*monsterIterator)->faceto);
						//temp = Players.findAllAliveCharactersInRangeOf( ( *monsterIterator )->pos.x, ( *monsterIterator )->pos.y, ( *monsterIterator )->pos.z, 9 );
						
						bool makeRandomStep=true;
						
						if ( ( !temp.empty() ) && (( *monsterIterator )->canAttack()) ) {
						    CPlayer * foundP2 = 0;
                            //search for the target via script or the player with the lowest hp
                            if( !monStruct.script || !monStruct.script->setTarget( *monsterIterator, temp, foundP2 ) )
                            {   
                                findPlayerWithLowestHP( &temp, foundP2 );
                            }
                            if ( foundP2 )  // if the script returned a valid character...
							{
							   (*monsterIterator)->lastTargetSeen = true;
							   ( *monsterIterator )->lastTargetPosition = foundP2->pos;
							   //Call enemyNear Script when enemy found
							   if ( foundMonster )
							   {
								   if ( monStruct.script )
								   {
									   //Wenn Scriptaufruf erfolgreich den aktuellen schleifenablauf abbrechen.          
									   if ( monStruct.script->enemyOnSight( (*monsterIterator) ,foundP2) )
									   { 
										   continue; //abort all other walking actions because the script has returned TRUE
									   }
								   } 
								   makeRandomStep=false;
								   (*monsterIterator)->performStep( foundP2->pos );
							   }
							   else
								   CLogger::writeMessage("Monster","cant find the monster id for calling a script!");
							   
							}
						}
						else if ( (*monsterIterator)->lastTargetSeen )
						{
						    makeRandomStep=false;
							( *monsterIterator)->performStep( (*monsterIterator)->lastTargetPosition );
						}
						if ( makeRandomStep )
						{ // No player in range or pig/sheep OR we didn't find anything in getTarget...
							int tempr=unsignedShortRandom( 1, 25);

							if (tempr <= 5) {
								MonsterStruct monsterdef;
								//Monsterdefinition suchen
								if (MonsterDescriptions->find( (*monsterIterator)->getType() , monsterdef) ) {
									if ( monsterdef.canselfheal ) //Prfen ob Monster sich selbst heilen kann
									{
										if ( doHealing( *monsterIterator ) ) //Heilen
										{
											( *monsterIterator )->actionPoints -= NP_REGENERATE_COST;
										}
									}
								} 
								else 
									CLogger::writeError("Monster","Data for Healing not Found for monsterrace: " + CLogger::toString((*monsterIterator)->getType()));
							} 
                            else
                            {
								CSpawnPoint* spawn = (*monsterIterator)->getSpawn();
								// is the monster created by a spawnpoint or summoned?
								if (spawn) 
                                {
                                
									char movedir[4];
									int yoffs = (*monsterIterator)->pos.y - spawn->get_y();
									int xoffs = (*monsterIterator)->pos.x - spawn->get_x();
                                    
                                    //Monster nicht mehr ins spawn reichweite daher den spawnpunkt entfernen
                                    if ( abs(xoffs) >= spawn->getRange() || abs(yoffs) >= spawn->getRange() )
                                    {
                                        //Monster ausserhalb der Spawn reichweiter daher vom Spawn entfernen
                                        (*monsterIterator)->setSpawn(NULL);
                                        unsigned int type = (*monsterIterator)->getType();
                                        spawn->dead(type);
                                    }    
                                    
                                    // monster north of spawnpoint?
									movedir[0] = (yoffs < 0) ? spawn->getRange() + yoffs : spawn->getRange();
                                    // monster northeast of spawnpoint?
                                    movedir[1] = (yoffs < 0 || xoffs > 0) ? spawn->getRange() - std::max(-yoffs,xoffs) : spawn->getRange();
									// monster east of spawnpoint?
									movedir[2] = (xoffs > 0) ? spawn->getRange() - xoffs : spawn->getRange();
                                    // monster southeast of spawnpoint?
                                    movedir[3] = (yoffs > 0 || xoffs > 0) ? spawn->getRange() - std::max(yoffs,xoffs) : spawn->getRange();
									// monster south of spawnpoint?
									movedir[4] = (yoffs > 0) ? spawn->getRange() - yoffs : spawn->getRange();
                                    // monster southwest of spawnpoint?
                                    movedir[5] = (yoffs > 0 || xoffs < 0) ? spawn->getRange() - std::max(yoffs,-xoffs) : spawn->getRange();
									// monster west of spawnpoint?
									movedir[6] = (xoffs < 0) ? spawn->getRange() + xoffs : spawn->getRange();
                                    // monster northwest of spawnpoint?
                                    movedir[7] = (yoffs < 0 || xoffs < 0) ? spawn->getRange() + std::min(yoffs,xoffs) : spawn->getRange();

									// each direction has chance of movedir[direction] / sum_movedir[i]
									tempr = unsignedShortRandom(1,
                                        movedir[0] + movedir[1] + movedir[2] + movedir[3]
                                      + movedir[4] + movedir[5] + movedir[6] + movedir[7]);
                                        // maybe we want to make no move at all?

									for ( uint8_t i = 0; i < 8; ++i ) {
										if (tempr < movedir[i]) {
											( *monsterIterator)->move((CCharacter::direction)i);
											break;
										} else {
											tempr -= movedir[i];
										}
									}
								} 
                                if ( !spawn)
                                {
                                     ( *monsterIterator)->move((CCharacter::direction)unsignedShortRandom(0,7));
								}

								// movementrate below normal if noone is near
								( *monsterIterator )->actionPoints -= 20;
								//std::cout << "Bewege Monster zuf�lig" << std::endl;
							}
						}
					}//angreifen/bewegen
				}
				else //Character is on route
				{
					//get attackrange of the weapon
					Item itl = (*monsterIterator)->GetItemAt(LEFT_TOOL);
					Item itr = (*monsterIterator)->GetItemAt(RIGHT_TOOL);
					
					WeaponStruct theWeapon;
					uint16_t range=1;
					
					if (WeaponItems->find( itr.id, theWeapon ))	range=theWeapon.Range;
					else if (WeaponItems->find( itl.id, theWeapon ))range=theWeapon.Range;
					//===============================================
					temp.clear();           
					findPlayersInSight( (*monsterIterator)->pos, static_cast<uint8_t>(range), temp, (*monsterIterator)->faceto);
					//If we have found players which can be attacked directly and the monster can attack
					if ( !temp.empty() ) 
					{ //angreifen
						CPlayer * foundP;
						//search for the player with the lowes hp
						if ( findPlayerWithLowestHP( &temp, foundP ) ) 
						{
							if ( foundMonster && monStruct.script )monStruct.script->enemyNear( (*monsterIterator) ,foundP);
						   else CLogger::writeError("Monster","cant find a monster id for checking the script!");
						
						}
					}
					
					//check if there is a player on sight
					temp.clear();
					findPlayersInSight( (*monsterIterator)->pos, static_cast<uint8_t>(9), temp, (*monsterIterator)->faceto);
					if ( !temp.empty() )
					{
						CPlayer * foundP;
						if ( findPlayerWithLowestHP( &temp, foundP ) ) 
						{
						   //Call enemyNear Script when enemy found
						   if ( foundMonster && monStruct.script )monStruct.script->enemyOnSight( (*monsterIterator) ,foundP);
						   //else CLogger::writeMessage("Monster","cant find the monster id for calling a script!");
						}
					}
					if (!(*monsterIterator)->waypoints->makeMove())
					{
						(*monsterIterator)->setOnRoute(false);
						if ( foundMonster && monStruct.script)monStruct.script->abortRoute( (*monsterIterator) );
						else CLogger::writeMessage("Monster","cant find the monster id for calling a script!");
					}
				}
            } // ausreichend actionpoints

            if ( effect ) do_LongTimeEffects( *monsterIterator );
            monsterIterator++;
        } //alive ?
        else 
		{ // tot
            MONSTERVECTOR::iterator newIt;
            killMonster( monsterIterator, newIt );
            monsterIterator = newIt;
        } //alive ?
    }//alle Monster abgearbeitet
	//Adding all new monsters to monsterlist und calling the onSpawn Script entry
	for ( MONSTERVECTOR::iterator newMonIt = newMonsters.begin(); newMonIt != newMonsters.end(); ++newMonIt)
	{
		Monsters.push_back( (*newMonIt) );
		MonsterStruct monStruct;
        bool foundMonster = MonsterDescriptions->find( (*newMonIt)->getType(), monStruct ) ;
		if ( foundMonster && monStruct.script )
		{
			monStruct.script->onSpawn( (*newMonIt) );
		}

	}
	//clear new monsters so they can be added
	newMonsters.clear();
}



void CWorld::checkNPC() 
{
    deleteAllLostNPC();
    bool effect = false;
    if ( fieldtimer[ 2 ]->next() ) 
    {
        effect = true;
    }

    std::vector < CPlayer* > temp;
    std::vector < CPlayer* > ::iterator tpit;
    NPCVECTOR::iterator npcIterator = Npc.begin();
    while ( npcIterator < Npc.end() ) {

        if ( ( *npcIterator )->IsAlive() ) 
		{
            ( *npcIterator )->actionPoints += ap;

            if ( ( *npcIterator )->actionPoints > NP_MAX_AP ) 
			{
                ( *npcIterator )->actionPoints = NP_MAX_AP;
            }
            
            (*npcIterator)->effects->checkEffects();

            if ( ( *npcIterator )->actionPoints > NP_MIN_AP && !(*npcIterator)->getScript()) 
			{
                // Wiederbeleber definieren
                if ( (( *npcIterator )->race == CCharacter::healer) || ( ( *npcIterator )->getHealer() ) )
				{
                    temp = Players.findAllCharactersInRangeOf(( *npcIterator )->pos.x, ( *npcIterator )->pos.y, ( *npcIterator )->pos.z, 2 );
                    for ( tpit = temp.begin(); tpit < temp.end(); ++tpit ) {
                        if ( !( *tpit )->IsAlive() ) {
#ifdef DO_UNCONSCIOUS
                            ( *tpit )->increaseAttrib( "hitpoints", UNCONSCIOUS + 100 );
#else
                            ( *tpit )->increaseAttrib( "hitpoints", 100 );
#endif
                            //sendSpinToAllVisiblePlayers( ( *tpit ), PLAYERSPIN_TC );
                        }
                        ( *npcIterator )->actionPoints -= 10;
                    }
                }
				
				
            } // AP && !script
            else if (( *npcIterator )->actionPoints > NP_MIN_AP) 
			{
                // we have a script...
                // let's execute the command for this cycle
                boost::shared_ptr<CLuaNPCScript> npcscript = (*npcIterator)->getScript();
                npcscript->nextCycle();
				temp.clear();
				findPlayersInSight( (*npcIterator)->pos, static_cast<uint8_t>(1), temp, (*npcIterator)->faceto);
				if ( !temp.empty() ) 
				{ //angreifen
					CPlayer * foundP;
					//search for the player with the lowes hp
					if ( findPlayerWithLowestHP( &temp, foundP ) ) 
					{
						npcscript->characterNear( (*npcIterator) ,foundP);
					}
				}
				temp.clear();
				findPlayersInSight( (*npcIterator)->pos, static_cast<uint8_t>(9), temp, (*npcIterator)->faceto);
				if ( !temp.empty() )
				{
					CPlayer * foundP;
					if ( findPlayerWithLowestHP( &temp, foundP ) ) 
					{
						npcscript->characterOnSight( (*npcIterator) ,foundP);
					}
				}
                if ( (*npcIterator)->getOnRoute() && !(*npcIterator)->waypoints->makeMove())
                {
				    (*npcIterator)->setOnRoute(false);
					npcscript->abortRoute( (*npcIterator) );
				}
            }

            if ( effect )
                do_LongTimeEffects( *npcIterator );
            npcIterator++;
        } // alive
        else {
            // Behandlung von toten NPC -> wiederbeleben
            ( *npcIterator )->increaseAttrib( "hitpoints", MAXHPS );
            sendSpinToAllVisiblePlayers( ( *npcIterator ) );
            npcIterator++;
        }
    }
}


void CWorld::workout_CommandBuffer( CPlayer* &cp) 
{
    

}


bool CWorld::ReadField( const char* inp, signed short int &outp ) {
    char** error = NULL;
    long int temp=strtol(inp,error,10);
    if (error != NULL) {
        std::cerr << "ERROR in ReadField to signed short int" << std::endl;
    } else {
        if ((temp <= (0x7FFF)) && (temp >= -0x8000)) {
            outp = temp;
            return true;
        } else {
            std::cerr << "RANGE ERROR on ReadField to signed short int" << std::endl;
        }
    }

    return false;
}

bool CWorld::ReadField( const char* inp, signed long int &outp ) {
    char** error=NULL;
    signed long int temp=strtol(inp,error,10);
    if (error != NULL) {
        std::cerr << "ERROR in ReadField to signed long int" << std::endl;;
        return false;
    } else {
        outp = temp;
        return true;
    }
}



// Init method for NPC's
void CWorld::initNPC() {
    NPCVECTOR::iterator npcIterator;
/*
    for ( npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator ) {
            // call reload script
            if ( ( *npcIterator )->getScript() )
            {
                ( *npcIterator )->getScript()->beforeReload();
        }
    }
*/
    std::cout<<" Lade und setze NPC's neu: \n";
    npcidc->set(NPC_BASE); //Counter wieder auf Basis setzen.
    CField* tempf; //alte NPC's l�chen
    for ( npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator ) {
        if ( GetPToCFieldAt( tempf, ( *npcIterator )->pos.x, ( *npcIterator )->pos.y, ( *npcIterator )->pos.z ) ) {
            //tempf->SetNPCOnField( false );
            tempf->removeChar();
        }
        sendRemoveCharToVisiblePlayers( ( *npcIterator )->id, ( *npcIterator )->pos );
        delete( *npcIterator );
        *npcIterator = NULL;
    }
    Npc.clear(); //Ende des L�chens der alten NPC's
    CNPCTable * NPCTbl;
    NPCTbl = new CNPCTable();
    delete NPCTbl;
    std::cout<<"Laden der NPC's beendet! \n";
}
void CWorld::initScheduler() {
    std::cout<<"Scheduler init \n";
    Scheduler = new CScheduler(this);
    //===========Globale Tasks wie Wetter Gezeiteneffekte etc einfgen=========
    //CSchedulerObject * globalPoisonTask; //neuen Task anlegen
    //globalPoisonTask = new CSGlobalPoison(Scheduler->GetCurrentCycle()+2); //erster Zyklus in 2 sec
    //Scheduler->AddTask(globalPoisonTask); //Task hinzufgen
    CSchedulerObject * globalPlLearning; //Task anlegen der die Geistige Aufnahmef�igkeit aller 10 sec bei Spielern wieder senkt
    globalPlLearning = new CSGlobalPlayerLearnrate(Scheduler->GetCurrentCycle()+5);
    Scheduler->AddTask(globalPlLearning);
    CSchedulerObject * globalMonLearning; //Task anlegen der die Geistige Aufnahmef�igkeit aller 30 sec bei Monstern wieder senkt
    globalMonLearning = new CSGlobalMonsterLearnrate(Scheduler->GetCurrentCycle()+10);
    Scheduler->AddTask(globalMonLearning);
    //CSchedulerObject * ItemNextCycleScript; //Task anlegen der nextCycle bei Scripts ausfhrt
    //ItemNextCycleScript = new CSItemScriptCycle(Scheduler->GetCurrentCycle()+10);
    //Scheduler->AddTask(ItemNextCycleScript);
    CSchedulerObject * globalTempAttribTask;
    globalTempAttribTask = new CSTempAttribCycle(Scheduler->GetCurrentCycle()+2);
    Scheduler->AddTask(globalTempAttribTask);
    //=========================================================================
    std::cout<<"Scheduler init end \n";
}
