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


#include "World.hpp"
#include "Command.hpp"
#include <sstream>
#include <regex.h>
#include <list>
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/NamesObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
//We need this for the standard Fighting Script.
#include "script/LuaWeaponScript.hpp"
//For the reload scripts
#include "script/LuaReloadScript.hpp"
#include "script/LuaLearnScript.hpp"
#include <boost/shared_ptr.hpp>
#include "data/LongTimeEffectTable.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include "script/LuaLoginScript.hpp"
#include "PlayerManager.hpp"
#include "data/RaceSizeTable.hpp"
#include "Logger.hpp"
#include "data/ScriptVariablesTable.hpp"

#include <iostream>

extern LongTimeEffectTable * LongTimeEffects;
extern RaceSizeTable * RaceSizes;
extern ScriptVariablesTable * scriptVariables;
extern std::ofstream talkfile;
extern boost::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;
extern boost::shared_ptr<LuaDepotScript>depotScript;
extern boost::shared_ptr<LuaLoginScript>loginScript;
extern boost::shared_ptr<LuaLearnScript>learnScript;


void set_spawn_command( World*, Player*, const std::string& );
void import_maps_command( World*, Player*, const std::string& );
void create_area_command( World*, Player*, const std::string& );
void set_login( World*, Player*, const std::string& );

template< typename To, typename From> To stream_convert( const From& from ) {
	std::stringstream stream;
	stream << from;
	To to;
	stream >> to;
	return to;
}

// register any gm commands here...
void World::InitGMCommands() {

	GMCommands["what"] = new Command(&World::what_command);

	GMCommands["?"] = new Command(&World::gmhelp_command);

	GMCommands["warp_to"] = new Command(&World::warpto_command);
	GMCommands["w"] = GMCommands["warp_to"];

	GMCommands["summon"] = new Command(&World::summon_command);
	GMCommands["s"] = GMCommands["summon"];

	GMCommands["prison"] = new Command(&World::prison_command);
	GMCommands["p"] = GMCommands["prison"];

	GMCommands["ban"] = new Command(&World::ban_command);
	GMCommands["b"] = GMCommands["ban"];

	GMCommands["tile"] = new Command(&World::tile_command);
	GMCommands["t"] = GMCommands["tile"];

	GMCommands["who"] = new Command(&World::who_command);

	GMCommands["turtleon"] = new Command( &World::turtleon_command );
	GMCommands["ton"] = GMCommands["turtleon"];
	GMCommands["turtleoff"] = new Command( &World::turtleoff_command );
	GMCommands["toff"] = GMCommands["turtleoff"];

    GMCommands["logon"] = new Command( &World::logon_command );
    GMCommands["logoff"] = new Command( &World::logoff_command );

	GMCommands["clippingon"] = new Command( &World::clippingon_command );
	GMCommands["con"] = GMCommands["clippingon"];
	GMCommands["clippingoff"] = new Command( &World::clippingoff_command );
	GMCommands["coff"] = GMCommands["clippingoff"];

	GMCommands["playersave"] = new Command( &World::playersave_command );
	GMCommands["ps"] = GMCommands["playersave"];

	GMCommands["add_teleport"] = new Command( &World::teleport_command );

	GMCommands["set_spawn"] = new Command( &set_spawn_command );

	GMCommands["importmaps"] = new Command( &import_maps_command );

	GMCommands["create_area"] = new Command( &create_area_command );

	GMCommands["nologin"] = new Command( &set_login );

	GMCommands["forceintroduce"] = new Command( &World::ForceIntroduce );
	GMCommands["fi"] = GMCommands["forceintroduce"];
    GMCommands["forceintroduceall"] = new Command( &World::ForceIntroduceAll);
    GMCommands["fia"] = GMCommands["forceintroduceall"];

	GMCommands["exportmaps"] = new Command( &World::exportMaps );

	GMCommands["makeinvisible"] = new Command( &World::makeInvisible );
	GMCommands["mi"] = GMCommands["makeinvisible"];

	GMCommands["makevisible"] = new Command( &World::makeVisible );
	GMCommands["mv"] = GMCommands["makevisible"];

	//GMCommands["reloaddefinitions"] = new Command( &World::reload_defs );
	//GMCommands["rd"] = GMCommands["reloaddefinitions"];

	GMCommands["showwarpfields"] = new Command( &World::showWarpFieldsInRange );

	GMCommands["removewarpfield"] = new Command( &World::removeTeleporter );

	GMCommands["inform"] = new Command( &World::informChar );

    GMCommands["talkto"] = new Command( &World::talkto_command );
    GMCommands["tt"] = GMCommands["talkto"];

    GMCommands["nuke"] = new Command( &World::kill_command );

    GMCommands["fullreload"] = new Command( &World::reload_command );
    GMCommands["fr"] = GMCommands["fullreload"];

    GMCommands["mapsave"] = new Command( &World::save_command );

    GMCommands["jumpto"] = new Command( &World::jumpto_command );
    GMCommands["j"] = GMCommands["jumpto"];

    GMCommands["broadcast"] = new Command( &World::broadcast_command );
    GMCommands["bc"] = GMCommands["broadcast"];

    GMCommands["kickall"] = new Command( &World::kickall_command );
    GMCommands["ka"] = GMCommands["kickall"];

    GMCommands["kick"] = new Command( &World::kickplayer_command);
    GMCommands["k"] = GMCommands["kick"];

    GMCommands["showips"] = new Command( &World::showIPS_Command);
    GMCommands["create"] = new Command( &World::create_command);

	GMCommands["spawn"] = new Command( &World::spawn_command);

}

void World::logon_command( Player* cp, const std::string &log)
{
    if ( cp->hasGMRight( gmr_reload ) )
    {
        Logger::activateLog( log );
    }

}

void World::logoff_command( Player* cp, const std::string &log)
{
    if ( cp->hasGMRight( gmr_reload ) )
    {
        Logger::deactivateLog( log );
    }

}

void World::spawn_command( Player* cp, const std::string &monid)
{
    if ( cp->hasGMRight( gmr_basiccommands ) )
    {
		uint16_t id;
		std::stringstream ss;
		ss.str(monid);
		ss >> id;
		position pos = cp->pos;
		pos.x++;
        createMonster(id, pos, 0);
	}
}

void World::create_command( Player* cp, const std::string &itemid)
{
    if ( cp->hasGMRight( gmr_basiccommands ) )
    {
        TYPE_OF_ITEM_ID item;
        uint16_t quality = 333;
        uint32_t data = 0;
		std::stringstream ss;
		ss.str(itemid);
		ss >> item;
        ss >> quality;
        ss >> data;
        cp->createItem(item, 1, quality, data);
    }

}

void World::kill_command( Player* cp)
{
    if ( !cp->hasGMRight(gmr_reload) )return;
    std::cout<<" GM "<<cp->name<<" used Nuke command";
    MONSTERVECTOR::iterator mIterator;
    uint32_t counter = 0;
    for ( mIterator = Monsters.begin() ; mIterator != Monsters.end(); ++mIterator)
    {
        //Kill the monster we have found
       (*mIterator)->remove();
        ++counter;
    }
    std::cout<<" and killed "<<counter<<" monsters!"<<std::endl;
}

void World::reload_command( Player* cp)
{
    if ( cp->hasGMRight(gmr_reload) )
    {
        if (reload_tables(cp) )
            cp->sendMessage("DB tables loaded successfully!");
        else
            cp->sendMessage("RITICAL ERROR: Failure while loading DB tables!");
    }
}

void World::broadcast_command( Player* cp,const std::string &message)
{
#ifdef World_DEBUG
	std::cout << "World: Admin " << cp->name << " sagt: " << message << std::endl;
#endif
			if ( cp->hasGMRight( gmr_broadcast )  )
            {
#ifdef LOG_TALK
				time_t acttime = time(NULL);
				std::string talktime = ctime(&acttime);
				talktime[talktime.size()-1] = ':';
				talkfile << talktime << " ";
				talkfile << cp->name << "(" << cp->id << ") broadcasts: " << message << std::endl;
#endif

				sendMessageToAllPlayers( message );
			}
}

void World::kickall_command( Player* cp)
{
    if ( cp->hasGMRight( gmr_forcelogout ) )
    {
#ifdef World_DEBUG
	std::cout << "World: Admin " << cp->name << " wirft alle Player aus dem Spiel" << std::endl;
#endif
	forceLogoutOfAllPlayers();
    }
}

void World::kickplayer_command( Player * cp,const std::string &player)
{
    if ( cp->hasGMRight( gmr_forcelogout ) )
    {
        if ( forceLogoutOfPlayer( player ) )
        {
#ifdef World_DEBUG
            std::cout << "World: Admin " << cp->name << " wirft Player " << player << " aus dem Spiel!" <<std::endl;
#endif
        }
    }
}

void World::showIPS_Command( Player * cp)
{
    if ( cp->hasGMRight(gmr_basiccommands) )
    {
#ifdef World_DEBUG
		std::cout << "World: Admin " << cp->name << " l�t sich alle Spieler anzeigen!" << std::endl;
#endif
		sendAdminAllPlayerData( cp );
	}
}

void World::jumpto_command( Player * cp,const std::string & player )
{
    if ( cp->hasGMRight(gmr_warp) )
    {
		cp->closeAllShowcasesOfMapContainers();
		teleportPlayerToOther( cp, player );
	}
}

void World::save_command( Player* cp)
{
    if ( !cp->hasGMRight(gmr_save) ) return;

#ifdef World_DEBUG
    std::cout << "World: Admin " << cp->name << " Server speichern " << std::endl;
#endif

    PLAYERVECTOR::iterator pIterator;
    Field* tempf;
    for ( pIterator = Players.begin(); pIterator < Players.end(); ++pIterator ) {
        // Felder auf denen Spieler stehen als frei markieren, damit die flags richtig gespeichert werden
        if ( GetPToCFieldAt( tempf, ( *pIterator )->pos.x, ( *pIterator )->pos.y, ( *pIterator )->pos.z ) ) {
            tempf->SetPlayerOnField( false );
        }
    }

    std::cout << "Karten speichern" << std::endl;
    Save( "Illarion" );

    // Flags auf der Karte wieder setzen
    for ( pIterator = Players.begin(); pIterator < Players.end(); ++pIterator ) {
        if ( GetPToCFieldAt( tempf, ( *pIterator )->pos.x, ( *pIterator )->pos.y, ( *pIterator )->pos.z ) ) {
            tempf->SetPlayerOnField( true );
        }
    }

    std::string tmessage = "*** Maps saved! ***";
    cp->sendMessage( tmessage );
}

void World::talkto_command( Player* cp, const std::string& ts)
{
    if ( !cp->hasGMRight(gmr_basiccommands) )return; //quit if the player hasn't the right
    char* tokenize = new char[ ts.length() + 1 ]; //Neuen char mit gr�e des strings anlegen
    strcpy( tokenize, ts.c_str() ); //Copy ts to tokenize
    std::cout<<"Tokenizing "<<tokenize<<std::endl;
    char* token;
    if ( (token = strtok( tokenize, "," ) ) != NULL )
    {
        std::string player = token;
        if ( ( token = strtok( NULL, "\\" ) ) != NULL )
        {
            std::string message = token;
#ifdef AdminCommands_DEBUG
            std::cout<<"Try to find Player: "<<player<<std::endl;
#endif
            Player * tempPl = Players.find( player );
            if ( tempPl != NULL )
            {
#ifdef AdminCommands_DEBUG
                std::cout<<"found Player about name, sending message:"<<message<<std::endl;
#endif
                tempPl->inform( message );
                return;
            }
            else
            {
                TYPE_OF_CHARACTER_ID tid;

                std::stringstream ss;
                ss.str( player );
                ss >> tid;

#ifdef AdminCommands_DEBUG
                std::cout<<"try to find player about id: "<<tid<<std::endl;
#endif
                PLAYERVECTOR::iterator plIterator;
                for ( plIterator = Players.begin(); plIterator != Players.end(); ++plIterator)
                {
                    if ( (*plIterator)->id == tid )
                    {
#ifdef AdminCommands_DEBUG
                        std::cout<<"found player about id, sending message: "<<message<<std::endl;
#endif
                        (*plIterator)->inform( message);
                        return; //Break the loop because we sen our message
                    }
                } //end of for ( plIter..)
            } //end of else
        } //end if ( token = strtok( NULL...)
    } //end of ( token = strtok( tokenize , ",")
}

void World::makeInvisible( Player* cp ) {
	if ( !cp->hasGMRight(gmr_visible) )return;
    cp->isinvisible = true;
    sendRemoveCharToVisiblePlayers( cp->id, cp->pos );
}

void World::makeVisible( Player* cp )
{
    if ( !cp->hasGMRight(gmr_visible) )return;
	cp->isinvisible = false;
#ifdef DO_UNCONSCIOUS
        if ( (cp->lifestate | 1) && cp->battrib.hitpoints > UNCONSCIOUS  )
#else
        if ( cp->lifestate | 1 )
#endif
            cp->appearance = cp->appearance_alive();
        else
            cp->appearance = cp->appearance_dead();
	std::vector < Player* > ::iterator titerator;

	std::vector < Player* > temp = Players.findAllCharactersInRangeOf(cp->pos.x, cp->pos.y, cp->pos.z, MAXVIEW );
	for ( titerator = temp.begin(); titerator < temp.end(); ++titerator )
    { //Fr alle anderen Sichtbar machen
		if ( cp != ( *titerator ) )
        {
	        boost::shared_ptr<BasicServerCommand>cmd( new MoveAckTC( cp->id, cp->pos, PUSH, 0 ));
            ( *titerator)->Connection->addCommand( cmd );
		}
	}
    boost::shared_ptr<BasicServerCommand>cmd( new AppearanceTC( cp ) );
    cp->Connection->addCommand( cmd );
}

void World::ForceIntroduce(Player * cp, const std::string& ts ) {
    if ( !cp->hasGMRight(gmr_basiccommands) )return;
	Player * tempPl;
	tempPl = Players.find( ts );
	if ( tempPl != NULL ) {
		forceIntroducePlayer( tempPl, cp );
	} else {
		TYPE_OF_CHARACTER_ID tid;

		// convert arg to digit and try again...
		std::stringstream ss;
		ss.str(ts);
		ss >> tid;

		if ( tid ) {
			PLAYERVECTOR::iterator playerIterator;

			for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
				if ( ( *playerIterator )->id == tid ) {
					forceIntroducePlayer( (*playerIterator), cp );
				}
			}
		}
	}
}

void World::ForceIntroduceAll(Player * cp)
{
    if ( !cp->hasGMRight(gmr_basiccommands) )return;
    std::vector < Player* > temp = Players.findAllCharactersInRangeOf( cp->pos.x, cp->pos.y, cp->pos.z, MAXVIEW );
	std::vector < Player* > ::iterator titerator;
    for ( titerator = temp.begin(); titerator < temp.end(); ++titerator )
    { //Schleife durch alle Spieler in Sichtweise
		if ( cp != ( *titerator ) )
        {
            forceIntroducePlayer( (*titerator), cp);
        }
    }
}

void World::informChar(Player* cp) {
    if ( !cp->hasGMRight(gmr_basiccommands) )return;
	if (cp->informCharacter) {
		cp->setInformChar(false);
	} else {
		cp->setInformChar(true);
	}
}

void World::teleportPlayerToOther( Player* cp, std::string ts ) {
    if ( !cp->hasGMRight(gmr_warp) )return;
	Player * tempPl;
	tempPl = Players.find( ts );
	if ( tempPl != NULL ) {
		cp->Warp(tempPl->pos);
		//warpPlayer( cp, tempPl->pos );
	} else {
		TYPE_OF_CHARACTER_ID tid;

		// convert arg to digit and try again...
		std::stringstream ss;
		ss.str(ts);
		ss >> tid;

		if ( tid ) {
			PLAYERVECTOR::iterator playerIterator;

			for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
				if ( ( *playerIterator )->id == tid ) {
					//warpPlayer( cp, ( *playerIterator )->pos );
					cp->Warp( (*playerIterator)->pos);
				}
			}
		}
	}
}


void World::forceLogoutOfAllPlayers() {
   	Field* tempf;
	PLAYERVECTOR::iterator playerIterator;

	for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
		if ( GetPToCFieldAt( tempf, ( *playerIterator )->pos.x, ( *playerIterator )->pos.y, ( *playerIterator )->pos.z ) ) {
			tempf->SetPlayerOnField( false );
		}
        boost::shared_ptr<BasicServerCommand>cmd( new LogOutTC( SERVERSHUTDOWN ) );
        (*playerIterator)->Connection->shutdownSend(cmd);
		//( *playerIterator )->Connection->closeConnection();
		PlayerManager::get()->getLogOutPlayers().non_block_push_back( *playerIterator );
    }
	Players.clear();
}


bool World::forceLogoutOfPlayer(std::string name )
{
 	Player * temp = Players.find( name );
	if ( temp != NULL )
    {
        boost::shared_ptr<BasicServerCommand>cmd( new LogOutTC( BYGAMEMASTER ) );
        temp->Connection->shutdownSend(cmd);
	//temp->Connection->closeConnection();
     	return true;
	}
    else
    {
		return false;
	}
}


void World::sendAdminAllPlayerData( Player* &admin )
{
    if ( !admin->hasGMRight(gmr_basiccommands) )return;
    boost::shared_ptr<BasicServerCommand>cmd(new AViewPlayersTC() );
    admin->Connection->addCommand( cmd );

}


void World::welcomePlayer( Player* cp )
{
	if ( cp != NULL )
    {
		if ( ( cp->getPlayerLanguage() < LANGUAGECOUNT ) )
        {
			char tempstring[ 80 ];
			int count=0;
			for (PLAYERVECTOR::iterator it = Players.begin(); it!= Players.end(); it++)
				if (!(*it)->isAdmin()) count++;
			sprintf ( tempstring, welcome[ cp->getPlayerLanguage() ].c_str(), count );
            boost::shared_ptr<BasicServerCommand>cmd( new SayTC( cp->pos.x, cp->pos.y, cp->pos.z, std::string(tempstring) ));
            cp->Connection->addCommand(cmd);
#ifdef LOG_TALK
			talkfile << "Admin sagt: " << std::string( tempstring ) << std::endl;
#endif

		}
	}
}

// !warp_to X<,| >Y[<,| >Z] || !warp_to Z
void World::warpto_command( Player* cp, const std::string& ts )
{
    if ( !cp->hasGMRight(gmr_warp) )return;
	position warpto;
	char* tokenize = new char[ ts.length() + 1 ];

	strcpy( tokenize, ts.c_str() );
	std::cout << "Tokenizing " << tokenize << std::endl;
	char* thistoken;
	if ( (thistoken = strtok( tokenize, " ," )) != NULL ) {
		if ( ReadField( thistoken, warpto.x ) ) {
			if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
				if ( ReadField( thistoken, warpto.y ) ) {
					if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
						if ( ReadField( thistoken, warpto.z ) ) {
							//warpPlayer( cp, warpto );
							cp->forceWarp(warpto);
						}
					}
					// Must give X and Y, but not Z
					else {
						warpto.z = cp->pos.z;
						cp->forceWarp(warpto);
						//warpPlayer( cp, warpto );
					}
				}
			}
                        // Enable !warp_to Z for easy level change
                        else {
                                warpto.z = warpto.x;
                                warpto.x = cp->pos.x;
                                warpto.y = cp->pos.y;
                                cp->forceWarp(warpto);
                                //warpPlayer( cp, warpto );
                        }
		}
	}

	delete [] tokenize;
}


// !summon <player>
void World::summon_command( Player* cp, const std::string& tplayer ) {
    if ( !cp->hasGMRight(gmr_summon) )return;

	std::cout << "Summoning player: " << tplayer << std::endl;
	Player * tempPl;
	tempPl = Players.find( tplayer );
	if ( tempPl != NULL ) {
		std::cout << "Warping player: " << tempPl->name << std::endl;
		//warpPlayer( tempPl, cp->pos );
		tempPl->Warp(cp->pos);
	} else {
		std::cout << "Looking for number" << std::endl;
		TYPE_OF_CHARACTER_ID tid;

		// convert arg to digit and try again...
		std::stringstream ss;
		ss.str(tplayer);
		ss >> tid;

		if ( tid ) {
			std::cout << "Looking for number " << tid << std::endl;
			PLAYERVECTOR::iterator playerIterator;

			for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
				if ( ( *playerIterator )->id == tid ) {
					std::cout << "Warping player: " << ( *playerIterator)->name << std::endl;
					//warpPlayer( ( *playerIterator ), cp->pos );
					(*playerIterator)->Warp(cp->pos);
				}
			}
		}
	}

}


// !prison <time> <player>
void World::prison_command( Player* cp, const std::string& timeplayer ) {
    if ( !cp->hasGMRight(gmr_prison) )return;

	std::cout << cp->name << " prisoning player: " << timeplayer << std::endl;

	char* tokenize = new char[ timeplayer.length() + 1 ];
	short int jailtime = 0;

	strcpy( tokenize, timeplayer.c_str() );
	std::cout << "Tokenizing " << tokenize << std::endl;
	char* thistoken;
	if ( (thistoken = strtok( tokenize, " " )) != NULL ) {
		if ( ReadField( thistoken, jailtime ) ) {
			char* tcharp = strtok( NULL, "\\" );
			if ( tcharp != NULL ) {
				std::string tplayer = tcharp;
				position warpto;
                std::stringstream ssx(configOptions["jail_x"]);
                ssx >> warpto.x;
                std::stringstream ssy(configOptions["jail_y"]);
                ssy >> warpto.y;
                std::stringstream ssz(configOptions["jail_z"]);
                ssz >> warpto.z;

				Player * tempPl;
				tempPl = Players.find( tplayer );
				if ( tempPl == NULL ) {
					TYPE_OF_CHARACTER_ID tid;

					// convert arg to digit and try again...
					std::stringstream ss;
					ss.str(tplayer);
					ss >> tid;

					if ( tid ) {
						PLAYERVECTOR::iterator playerIterator;

						for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
							if ( ( *playerIterator )->id == tid ) {
								tempPl = ( *playerIterator );
							}
						}

					}
				}

				if ( tempPl != NULL ) {
					std::string tmessage = "*** Jailed " + tempPl->name;
					cp->sendMessage( tmessage );

					std::cout << cp->name << " jailed player: " << tempPl->name << " for " << jailtime << std::endl;
					if ( jailtime >= 0 ) {
						if ( jailtime > 0 ) {
							tempPl->SetStatus( JAILEDFORTIME );       // Jailed for time
							tempPl->SetStatusTime( jailtime * 60 );   // Jailed for seconds
							tempPl->SetStatusGM( cp->id );          // Jailed by who
						} else if ( jailtime == 0 ) {
							tempPl->SetStatus( JAILED );              // Jailed indefinately
							tempPl->SetStatusTime( 0 );               // Jailed for seconds
							tempPl->SetStatusGM( cp->id );          // Jailed by who
						}
						tmessage = cp->name + " jailed you for " + ( jailtime == 0 ? "eternity" : stream_convert<std::string>( (short int &) jailtime ) + " minutes");
						tempPl->sendMessage( tmessage );
						//warpPlayer( tempPl, warpto );
						tempPl->Warp(warpto);
					}
				} else {
					std::string tmessage = "*** Could not find " + tplayer;
					std::cout << tmessage << std::endl;
					cp->sendMessage( tmessage );
				}
			}
		}
	}

	delete [] tokenize;

}


// !ban <time> [m|h|d] <player>
void World::ban_command( Player* cp, const std::string& timeplayer ) {
    if ( !cp->hasGMRight(gmr_ban) )return;

	std::cout << cp->name << " banning player: " << timeplayer << std::endl;

	char* tokenize = new char[ timeplayer.length() + 1 ];
	short int jailtime = 0;

	strcpy( tokenize, timeplayer.c_str() );
	std::cout << "Tokenizing " << tokenize << std::endl;

	char* thistoken;
	if ( (thistoken = strtok( tokenize, " " )) != NULL ) {
		if ( ReadField( thistoken, jailtime ) ) {
			char* tcharp = strtok( NULL, " " );
			if ( tcharp != NULL ) {
				int multiplier = 0;

				std::string tplayer;
				std::string timescale = tcharp;
				if ( timescale == "m" ) {
					multiplier = 60;
					timescale = "";
				} else if ( timescale == "h" ) {
					multiplier = 3600;
					timescale = "";
				} else if ( timescale == "d" ) {
					multiplier = 86400;
					timescale = "";
				}

				char* tcharp = strtok( NULL, "\\" );
				if ( tcharp != NULL ) {
					tplayer = tcharp;
					if ( timescale != "" )
						tplayer = timescale + " " + tplayer;
				} else {
					tplayer = timescale;
					timescale = "d";
					multiplier = 86400;
				}

				Player * tempPl;
				tempPl = Players.find( tplayer );
				if ( tempPl == NULL ) {
					TYPE_OF_CHARACTER_ID tid;

					// convert arg to digit and try again...
					std::stringstream ss;
					ss.str(tplayer);
					ss >> tid;

					if ( tid ) {
						PLAYERVECTOR::iterator playerIterator;

						for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
							if ( ( *playerIterator )->id == tid ) {
								tempPl = ( *playerIterator );
							}
						}
					}
				}

				if ( tempPl != NULL ) {

					ban( tempPl, jailtime * multiplier, cp->id);

					std::cout << cp->name << " banned player: " << tempPl->name << " for " << jailtime << timescale << std::endl;
					std::string tmessage = "*** Banned " + tempPl->name;
					cp->sendMessage( tmessage );

				} else {
					std::string tmessage = "*** Could not find " + tplayer;
					std::cout << tmessage << std::endl;
					cp->sendMessage( tmessage );
				}
			}
		}
	}

	delete [] tokenize;

}

void World::banbyname( Player* cp, short int banhours, std::string tplayer ) {
    if ( !cp->hasGMRight(gmr_ban) )return;

	Player * tempPl;
	tempPl = Players.find( tplayer );

	if ( tempPl != NULL ) {

		ban( tempPl, static_cast<int>( banhours * 3600 ), cp->id );

		std::cout << cp->name << " banned player: " << tempPl->name << " for " << banhours << " hours" << std::endl;
		std::string tmessage = "*** Banned " + tempPl->name;
		cp->sendMessage( tmessage );

	} else {
		std::string tmessage = "*** Could not find " + tplayer;
		std::cout << tmessage << std::endl;
		cp->sendMessage( tmessage );
	}

}

void World::banbynumber( Player* cp, short int banhours, TYPE_OF_CHARACTER_ID tid ) {
    if ( !cp->hasGMRight(gmr_ban) )return;
	Player * tempPl = NULL;

	PLAYERVECTOR::iterator playerIterator;

	for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
		if ( ( *playerIterator )->id == tid ) {
			tempPl = ( *playerIterator );
		}
	}

	if ( tempPl != NULL ) {

		ban( tempPl, static_cast<int>( banhours * 3600 ), cp->id );

		std::cout << cp->name << " banned player: " << tempPl->name << " for " << banhours << " hours" << std::endl;
		std::string tmessage = "*** Banned " + tempPl->name;
		cp->sendMessage( tmessage );

	} else {
		std::string tmessage = "*** Could not find " + stream_convert<std::string>( tid );
		std::cout << tmessage << std::endl;
		cp->sendMessage( tmessage );
	}


}


void World::ban( Player* cp, int bantime, TYPE_OF_CHARACTER_ID gmid ) {
    if ( bantime >= 0 ) {
		if ( bantime > 0 ) {
			cp->SetStatus( BANNEDFORTIME );       // Banned for time
			time_t ttime;
			time( &ttime );
			// Banned for seconds
			cp->SetStatusTime( ttime + bantime );  // Banned for expire
			cp->SetStatusGM( gmid );             // Banned by who
		} else if ( bantime == 0 ) {
			cp->SetStatus( BANNED );               // Banned indefinately
			cp->SetStatusTime( 0 );
			cp->SetStatusGM( gmid );             // Banned by who
		}

		forceLogoutOfPlayer( cp->name );

	}

}


// !who [player]
void World::who_command( Player* cp, const std::string& tplayer ) {
    if ( !cp->hasGMRight(gmr_basiccommands) )return;

	if (tplayer == "") {

		std::string tmessage = "";
		PLAYERVECTOR::iterator playerIterator;

		for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
			if ( tmessage.length() > 0 && tmessage.length() + 10 + ( *playerIterator )->name.length() > 104 ) {
				cp->sendMessage( tmessage );
				tmessage = "";
			}
			if ( tmessage.length() > 0 ) {
				tmessage = tmessage + ", ";
			}
			tmessage = tmessage + ( *playerIterator )->name +
					   "(" + stream_convert<std::string>( ( *playerIterator )->id ) + ")";
		}
		if ( tmessage.length() > 0 ) {
			cp->sendMessage( tmessage );
		}
	} else {

		Player * tempPl;
		tempPl = Players.find( tplayer );
		if ( tempPl == NULL ) {
			TYPE_OF_CHARACTER_ID tid;

			// convert arg to digit and try again...
			std::stringstream ss;
			ss.str(tplayer);
			ss >> tid;

			if ( tid ) {
				PLAYERVECTOR::iterator playerIterator;

				for ( playerIterator = Players.begin(); playerIterator < Players.end(); ++playerIterator ) {
					if ( ( *playerIterator )->id == tid ) {
						tempPl = ( *playerIterator );
					}
				}
			}
		}

		if ( tempPl != NULL ) {
			std::string tmessage = tempPl->name +
								   "(" + stream_convert<std::string>( tempPl->id ) + ")";
			if ( tempPl->GetStatus() == 20 ) {
				tmessage = tmessage + " JAILED FOREVER by " + tempPl->GetStatusGM();
			} else if ( tempPl->GetStatus() == 21 ) {
				tmessage = tmessage + " JAILED for " +
						   stream_convert<std::string>( tempPl->GetStatusTime() ) +
						   " seconds by " + tempPl->GetStatusGM();

			}
			tmessage = tmessage + " x" + stream_convert<std::string>( tempPl->pos.x );
			tmessage = tmessage + " y" + stream_convert<std::string>( tempPl->pos.y );
			tmessage = tmessage + " z" + stream_convert<std::string>( tempPl->pos.z );
			tmessage = tmessage + " HPs:" + stream_convert<std::string>( tempPl->battrib.hitpoints );
			tmessage = tmessage + ( (tempPl->IsAlive()) ? " Alive" : " Dead" );
			tmessage = tmessage + " Mental Capacity: " + stream_convert<std::string>( tempPl->getMentalCapacity() );
#ifdef DO_UNCONSCIOUS
			if ( ! tempPl->IsConscious() ) {
				tmessage = tmessage + " (Unconscious)";
			}
#endif
			tmessage = tmessage + ( (tempPl->getPlayerLanguage() == 0 ) ? " German" : " English" );

			cp->sendMessage( tmessage );
		}
	}
}


void World::tile_command( Player* cp, const std::string& ttilenumber ) {
    if ( !cp->hasGMRight(gmr_settiles) )return;
	short int tilenumber = 0;
	if ( ReadField( ttilenumber.c_str(), tilenumber ) ) {
		setNextTile( cp, tilenumber );
	}

}


void World::setNextTile( Player* cp, unsigned char tilenumber ) {

	position tpos = cp->getFrontalPosition();

	Field* tempf;
	if ( GetPToCFieldAt(tempf, tpos.x, tpos.y, tpos.z) )
    {
		tempf->setTileId( tilenumber );
		tempf->updateFlags();
	}

    //update the current area
    cp->sendRelativeArea( 0 );
	sendAllVisibleCharactersToPlayer( cp, true );

}


void World::turtleon_command( Player* cp, const std::string& ttilenumber ) {
    if ( !cp->hasGMRight(gmr_settiles) )return;
	short int tilenumber = 0;

	if ( ReadField( ttilenumber.c_str(), tilenumber ) ) {
		cp->setTurtleActive( true );
		cp->setTurtleTile( tilenumber );
	}
}


void World::turtleoff_command( Player* cp ) {
    if ( !cp->hasGMRight(gmr_settiles) )return;
	cp->setTurtleActive( false );
}


void World::clippingon_command( Player* cp ) {
    if ( !cp->hasGMRight(gmr_clipping) )return;
	cp->setClippingActive( true );
}


void World::clippingoff_command( Player* cp ) {
    if ( !cp->hasGMRight(gmr_clipping) )return;
	cp->setClippingActive( false );
}


void World::what_command( Player* cp ) {
	position front = cp->getFrontalPosition();

    cp->sendMessage( "Facing:" );
	std::stringstream message;

    message << "- Position (" << front.x << ", " << front.y << ", " << front.z << ")";
    cp->sendMessage( message.str() );
	Field* tempf;
	if ( GetPToCFieldAt(tempf, front) ) {
        message.str("");

		message << "- Tile " << tempf->getTileId();
        cp->sendMessage( message.str() );
        Item top;
        if (tempf->ViewTopItem(top)) {
            message.str("");

            message << "- Item " << top.id;
            if (cp->hasGMRight(gmr_basiccommands)) {
                message << ",  Quality " << top.quality;
                message << ",  Data " << top.data;
            }
            cp->sendMessage( message.str() );
        }
	}
}


void World::playersave_command( Player* cp ) {
    if ( !cp->hasGMRight(gmr_save) )return;

	PLAYERVECTOR::iterator pIterator;
	unsigned long int thisonlinetime = 0;

	std::string placeholder;

	for ( pIterator = Players.begin(); pIterator < Players.end(); ++pIterator ) {
		if ( ! ( *pIterator )->save() ) {
			thisonlinetime = 0;
		}
	}

	std::string tmessage = "*** All online players saved! ***";
	cp->sendMessage( tmessage );

}


// !teleport X<,| >Y[<,| >Z]
void World::teleport_command( Player* cp, const std::string& ts ) {

    if ( !cp->hasGMRight(gmr_warp) )return;

	position teleportto;
	char* tokenize = new char[ ts.length() + 1 ];

	strcpy( tokenize, ts.c_str() );
	std::cout << "Tokenizing " << tokenize << std::endl;
	char* thistoken;
	if ( (thistoken = strtok( tokenize, " ," )) != NULL ) {
		if ( ReadField( thistoken, teleportto.x ) ) {
			if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
				if ( ReadField( thistoken, teleportto.y ) ) {
					if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
						if ( ReadField( thistoken, teleportto.z ) ) {
							if ( addWarpField( cp->pos, teleportto, 0, 0 ) ) {
								std::string tmessage = "*** Warp Field Added! ***";
								cp->sendMessage( tmessage );
							} else {
								std::string tmessage = "*** Warp Field *NOT* Added! ***";
								cp->sendMessage( tmessage );
							};
						}
					}
				}
			}
		}
	}

	delete [] tokenize;
}


void World::gmhelp_command( Player* cp ) {
    if ( !cp->hasGMRight(gmr_basiccommands) )return;

	std::string tmessage = " <> - parameter.  [] - optional.  | = choice.  () = shortcut";
	cp->sendMessage( tmessage );
	if (cp->hasGMRight(gmr_basiccommands))
    {
        tmessage = "!what - sends different informations of the field or the character in front of you.";
        cp->sendMessage( tmessage );
        tmessage = "!who [player] - List all players online or a single player if specified.";
        cp->sendMessage( tmessage );
        tmessage = "!forceintroduce <char id|char name> - (!fi) introduces the char to all gms in range.";
        cp->sendMessage( tmessage );
        tmessage = "!forceintroduceall - (!fia) introduces all chars in sight to you.";
        cp->sendMessage( tmessage );
        tmessage = "!inform - gives you some more informations while fighting.";
        cp->sendMessage( tmessage );
        tmessage = "!talkto <playername|id>, <message> - (!tt) sends a message to a specific player important is the , after the id or name!";
        cp->sendMessage( tmessage );
        tmessage = "!broadcast <message> - (!bc) Broadcasts the message <message> to all players IG.";
        cp->sendMessage( tmessage );
        tmessage = "!create id [quality [data]] creates an item in your inventory.";

    }
    if ( cp->hasGMRight(gmr_warp) )
    {
        tmessage = "!warp <x> <y> [z] - (!w) to x, y, z location.";
        cp->sendMessage( tmessage );
        tmessage = "!add_teleport <x> <y> <z> - Adds a teleportfield in front of you to the field <x> <y> <z>.";
        cp->sendMessage( tmessage );
        tmessage = "!showwarpfields <range> - Shows all warpfields in the range <range>.";
        cp->sendMessage( tmessage );
        tmessage = "!removewarpfield <x> <y> <z> - Removes the warpfield at the position <x> <y> <z>.";
        cp->sendMessage( tmessage );
        tmessage = "!jumpto <playerid|name> - (!j) teleports you to the player.";
        cp->sendMessage( tmessage );

    }
    if ( cp->hasGMRight(gmr_summon) )
    {
        tmessage = "!summon <player> - (!s) Summons a player to you.";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_prison) )
    {
        tmessage = "!prison <time> <player> - (!p) Jails the player <player> for <time> amount minutes. 0 is forever.";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_ban) )
    {
        tmessage = "!ban <time> [m|h|d] <player> - (!b) Bans the player <player> for <time> [m]inutes/[h]ours/[d]ays.";
        cp->sendMessage( tmessage );
    }
    if (cp->hasGMRight(gmr_settiles) )
    {
        tmessage = "!tile <tilenumber> - (!t) changes the tile in front of you to the tile with id <tilenumber>.";
        cp->sendMessage( tmessage );
        tmessage = "!turtleon <tile number> - (!ton) Change tiles to tile # when you walk (advanced)";
        cp->sendMessage( tmessage );
        tmessage = "!turtleoff - (!toff) Stop changing tiles as you walk.";
        cp->sendMessage( tmessage );
    }
    if (cp->hasGMRight(gmr_clipping) )
    {
        tmessage = "!clippingon - (!con) Turn clipping on (Can't walk through walls)";
        cp->sendMessage( tmessage );
        tmessage = "!clippingoff - (!coff) Turn clipping off (Walk anywhere)";
        cp->sendMessage( tmessage );
    }
    if (cp->hasGMRight(gmr_save) )
    {
        tmessage = "!playersave - (!ps) saves all players to the database.";
        cp->sendMessage( tmessage );
        tmessage = "!mapsave - Saves the map after changes where made.";
        cp->sendMessage( tmessage );
    }
    if (cp->hasGMRight(gmr_reload))
    {
        tmessage = "!set_spawn <true|false> - activates/deactivates the spawning of monsters.";
        cp->sendMessage( tmessage );
        tmessage = "!reloaddefinitions - (!rd) reloads all datas without spawnpoints, so no new monsters are spawned. (deactivated)";
        cp->sendMessage( tmessage );
        tmessage = "!nuke - kills all Monster on the map (to clean the map after a spawn reload).";
        cp->sendMessage( tmessage );
        tmessage = "!fullreload - (!fr) reloads all database tables";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_import) )
    {
        tmessage = "!importmaps <x> <y> <z> - Imports new maps from the Editor to the location.";
        cp->sendMessage( tmessage );
        tmessage = "create_area <x> <y> <z> <with> <height> <filltile> - Creates a new map at <x> <y> <z> with the dimensions <heigt> <with> and the tile <fillile>.";
        cp->sendMessage( tmessage );
        tmessage = "!exportmaps - Exports the current maps (dunno if that works?).";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_loginstate))
    {
        tmessage = "!nologin <true|false> - changes the login state, with true only gm's can log in.";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_visible) )
    {
        tmessage = "!makeinvisible - (!mi) makes you invisible for other chars. NOT FOR MONSTERS.";
        cp->sendMessage( tmessage );
        tmessage = "!makevisible - (!mv) makes you visible if you are invisible.";
        cp->sendMessage( tmessage );
    }
    if ( cp->hasGMRight(gmr_forcelogout) )
    {
        tmessage = "!kickall - Kicks all players out of the game.";
        cp->sendMessage( tmessage );
        tmessage = "!kick <playerid> - Kicks the player with the id out of the game.";
        cp->sendMessage( tmessage );
    }
}

//! parse GMCommands of the form !<string1> <string2> and process them
bool World::parseGMCommands(Player* cp, const std::string& text) {

	// did we find a command?
	bool done = false;

	// use a regexp to match for commands...
	regex_t expression;
	regcomp(&expression, "^!([^ ]+) ?(.*)?$",REG_ICASE|REG_EXTENDED);
	regmatch_t matches[3];

	if (regexec(&expression, text.c_str(), 3, matches, 0) == 0) {
		// we found something...
		CommandIterator it = GMCommands.find(text.substr(matches[1].rm_so, matches[1].rm_eo-1));
		// do we have a matching command?
		if (it != GMCommands.end()) {
			if (matches[2].rm_so != -1)  // !bla something
			{
				(*it->second)(this, cp, text.substr(matches[2].rm_so));
			} else // !bla
			{
				(*it->second)(this, cp, "");
			}
			done = true;
		}
	}

	regfree(&expression);

	return done;

}

extern MonsterTable* MonsterDescriptions;

void reportError( Player* cp, std::string msg )
{
    std::cerr << "ERROR: " << msg << std::endl;
    cp->sendMessage( "ERROR: " + msg );
}

void reportScriptError( Player* cp, std::string serverscript, std::string what )
{
    reportError( cp, "Failed to reload server." + serverscript + ": " + what );
}

void reportTableError( Player* cp, std::string dbtable )
{
    reportError( cp, "Failed to reload DB table: " + dbtable );
}


bool World::reload_defs( Player * cp ) {
    if ( !cp->hasGMRight(gmr_reload) ) return false;

    sendMessageToAllPlayers( "### The server is reloading, this may cause some lag ###" );

	std::string server;
	bool ok = true;

	CommonObjectTable* CommonItems_temp = 0;
	NamesObjectTable* ItemNames_temp = 0;
	WeaponObjectTable* WeaponItems_temp = 0;
	ArmorObjectTable* ArmorItems_temp = 0;
	ContainerObjectTable* ContainerItems_temp = 0;
	TilesModificatorTable* TilesModItems_temp = 0;
	MonsterTable* MonsterDescriptions_temp = 0;
	TilesTable* Tiles_temp = 0;
	SpellTable* Spells_temp = 0;
	TriggerTable* Trigger_temp = 0;
	MonsterAttackTable* MonsterAttacks_temp = 0;
	NaturalArmorTable* NaturalArmors_temp = 0;
    ScheduledScriptsTable* ScheduledScripts_temp = 0;
    LongTimeEffectTable * LongTimeEffects_temp = 0;
    RaceSizeTable * RaceSizes_temp = 0;

	CommonItems_temp = new CommonObjectTable();
	if (CommonItems_temp == NULL || !CommonItems_temp->dataOK())
    {
        reportTableError( cp, "common" );
		ok = false;
    }

	if (ok) {
		ItemNames_temp = new NamesObjectTable();
		if (ItemNames_temp == NULL || !ItemNames_temp->dataOK())
        {
            reportTableError( cp, "itemname" );
		    ok = false;
        }
	}

    if (ok) {
		RaceSizes_temp = new RaceSizeTable();
		if (RaceSizes_temp == NULL || !RaceSizes_temp->isDataOk())
        {
            reportTableError( cp, "raceattr" );
		    ok = false;
        }
	}

	if (ok) {
		WeaponItems_temp = new WeaponObjectTable();
		if (WeaponItems_temp == NULL || !WeaponItems_temp->dataOK())
        {
            reportTableError( cp, "weapon" );
		    ok = false;
        }
	}

	if (ok) {
		ArmorItems_temp = new ArmorObjectTable();
		if (ArmorItems_temp == NULL || !ArmorItems_temp->dataOK())
        {
            reportTableError( cp, "armor" );
		    ok = false;
        }
	}

	if (ok) {
		ContainerItems_temp = new ContainerObjectTable();
		if (ContainerItems_temp == NULL || !ContainerItems_temp->dataOK())
        {
            reportTableError( cp, "container" );
		    ok = false;
        }
	}

	if (ok) {
		TilesModItems_temp = new TilesModificatorTable();
		if (TilesModItems_temp == NULL || !TilesModItems_temp->dataOK())
        {
            reportTableError( cp, "tilesmodificators" );
		    ok = false;
        }
	}

	if (ok) {
		Tiles_temp = new   TilesTable();
		if (Tiles_temp == NULL || !Tiles_temp->dataOK())
        {
            reportTableError( cp, "tiles" );
		    ok = false;
        }
	}

	if (ok) {
		MonsterDescriptions_temp = new MonsterTable();
		if (MonsterDescriptions_temp == NULL || !MonsterDescriptions_temp->dataOK())
        {
            reportTableError( cp, "monster" );
		    ok = false;
        }
	}

	if (ok) {
		Spells_temp = new SpellTable();
		if (Spells_temp == NULL || !Spells_temp->isDataOK() )
        {
            reportTableError( cp, "spells" );
		    ok = false;
        }
	}

	if (ok) {
		Trigger_temp = new TriggerTable();
		if (Trigger_temp == NULL || !Trigger_temp->isDataOK() )
        {
            reportTableError( cp, "triggerfields" );
		    ok = false;
        }
	}

    if (ok)
    {
        LongTimeEffects_temp = new LongTimeEffectTable();
        if ( LongTimeEffects_temp == NULL || !LongTimeEffects->dataOK() )
        {
            reportTableError( cp, "longtimeeffects" );
		    ok = false;
        }
    }

	if (ok) {
	    MonsterAttacks_temp = new MonsterAttackTable();
	    if (MonsterAttacks_temp == NULL || !MonsterAttacks_temp->isDataOk() )
        {
            reportTableError( cp, "monsterattack" );
		    ok = false;
        }
    }

    if (ok) {
        NaturalArmors_temp = new NaturalArmorTable();
        if (NaturalArmors_temp == NULL || !NaturalArmors_temp->isDataOk() )
        {
            reportTableError( cp, "naturalarmor" );
		    ok = false;
        }
    }

    if (ok) {
        std::cerr << "Attempting to reload Scheduler" << std::endl;
        ScheduledScripts_temp = new ScheduledScriptsTable();
        std::cerr << "Created new Scheduler" << std::endl;
        if (ScheduledScripts_temp == NULL || !ScheduledScripts_temp->dataOK() )
        {
            reportTableError( cp, "scheduledscripts" );
		    ok = false;
        }
    }

	if (!ok)
    {
		if (CommonItems_temp != NULL)
			delete CommonItems_temp;
		if (ItemNames_temp != NULL)
			delete ItemNames_temp;
		if (WeaponItems_temp != NULL)
			delete WeaponItems_temp;
		if (ArmorItems_temp != NULL)
			delete ArmorItems_temp;
		if (ContainerItems_temp != NULL)
			delete ContainerItems_temp;
		if (TilesModItems_temp != NULL)
			delete TilesModItems_temp;
		if (Tiles_temp != NULL)
			delete Tiles_temp;
		if (MonsterDescriptions_temp != NULL)
			delete MonsterDescriptions_temp;
		if (Spells_temp != NULL)
			delete Spells_temp;
		if (Trigger_temp != NULL)
			delete Trigger_temp;
		if (NaturalArmors_temp != NULL)
		    delete NaturalArmors_temp;
		if (MonsterAttacks_temp != NULL)
            delete MonsterAttacks_temp;
        if (ScheduledScripts_temp != NULL)
            delete ScheduledScripts_temp;
        if (LongTimeEffects_temp != NULL)
            delete LongTimeEffects_temp;
        if (RaceSizes_temp != NULL)
            delete RaceSizes_temp;
        //if (ScriptVar_temp != NULL)
        //    delete ScriptVar_temp;

	} else {
		// if everything went well, delete old tables and set up new tables
        //Mutex für login logout sperren so das aktuell keiner mehr einloggen kann
        PlayerManager::get()->setLoginLogout(true);
		delete CommonItems;
		CommonItems = CommonItems_temp;
		delete ItemNames;
		ItemNames = ItemNames_temp;
		delete WeaponItems;
		WeaponItems = WeaponItems_temp;
		delete ArmorItems;
		ArmorItems = ArmorItems_temp;
		delete ContainerItems;
		ContainerItems = ContainerItems_temp;
		delete TilesModItems;
		TilesModItems = TilesModItems_temp;
		delete Tiles;
		Tiles = Tiles_temp;
		delete MonsterDescriptions;
		MonsterDescriptions = MonsterDescriptions_temp;
		delete Spells;
		Spells = Spells_temp;
		delete Triggers;
		Triggers = Trigger_temp;
		delete NaturalArmors;
		NaturalArmors = NaturalArmors_temp;
		delete MonsterAttacks;
		MonsterAttacks = MonsterAttacks_temp;
        delete scheduledScripts;
        scheduledScripts = ScheduledScripts_temp;
        delete RaceSizes;
        RaceSizes = RaceSizes_temp;
        delete LongTimeEffects;
        LongTimeEffects = LongTimeEffects_temp;
        // delete scriptVariables;
        // scriptVariables = ScriptVar_temp;
        //Mutex entsperren.
        PlayerManager::get()->setLoginLogout(false);
		//Reload the standard Fighting script
        try
        {
			boost::shared_ptr<LuaWeaponScript> tmpScript(new LuaWeaponScript( "server.standardfighting" ));
			standardFightingScript = tmpScript;
        }
        catch (ScriptException &e)
        {
            reportScriptError( cp, "standardfighting", e.what() );
            ok = false;
        }

        try
        {
            boost::shared_ptr<LuaLookAtPlayerScript>tmpScript(new LuaLookAtPlayerScript( "server.playerlookat" ));
            lookAtPlayerScript = tmpScript;
        }
        catch(ScriptException &e)
        {
            reportScriptError( cp, "playerlookat", e.what() );
            ok = false;
        }

        try
        {
            boost::shared_ptr<LuaLoginScript>tmpScript(new LuaLoginScript( "server.login" ));
            loginScript = tmpScript;
        }
        catch(ScriptException &e)
        {
            reportScriptError( cp, "login", e.what() );
            ok = false;
        }

        try
        {
            boost::shared_ptr<LuaLearnScript>tmpScript(new LuaLearnScript( "server.learn" ));
            learnScript = tmpScript;
        }
        catch(ScriptException &e)
        {
            reportScriptError( cp, "learn", e.what() );
            ok = false;
        }


        try
        {
			boost::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript( "server.reload_defs" ));
			bool onReloadOk = tmpScript->onReload();
            if (!onReloadOk) reportError( cp, "server.reload_defs.onReload returned false" );
            ok = ok && onReloadOk;
        }
        catch (ScriptException &e)
        {
            reportScriptError( cp, "reload_defs", e.what() );
            ok = false;
        }

        try
        {
            boost::shared_ptr<LuaDepotScript>tmpScript(new LuaDepotScript( "server.depot" ));
            depotScript = tmpScript;
        }
        catch(ScriptException &e)
        {
            reportScriptError( cp, "depot", e.what() );
            ok = false;
        }

	}


	if (ok) cp->sendMessage( " *** Definitions reloaded *** " );
	else cp->sendMessage( "RITICAL ERROR: Failure while reloading definitions" );
	return ok;
}


bool World::reload_tables( Player * cp ) {

    scriptVariables->save();

    LuaScript::shutdownLua();

    bool ok = reload_defs( cp );

	if (ok) {
		// reload respawns
		initRespawns();

		//reload NPC's
		initNPC();

		try
	        {
			boost::shared_ptr<LuaReloadScript> tmpScript(new LuaReloadScript( "server.reload_tables" ));
			ok = tmpScript->onReload();
	        }
	        catch (ScriptException &e)
	        {
            	reportScriptError( cp, "reload_tables", e.what() );
                ok = false;
        	}

	}

	return ok;
}


// enable/disable spawnpoints
void set_spawn_command( World*, Player* player, const std::string& in) {
    if ( !player->hasGMRight(gmr_reload) )return;
	std::cout << "set spawn to '" << in << "' by: " << player->name << std::endl;
	configOptions["do_spawn"] = in;
}


void import_maps_command( World* world, Player* player, const std::string& param )
{
    if ( !player->hasGMRight(gmr_import) )return;
	world->load_maps();
}

// create a new area starting at x,y,z with dimension w,h, filltile ft (create_area x y z w h ft)
void create_area_command( World* world, Player* player,const std::string& params) {
    if ( !player->hasGMRight(gmr_import) )return;
	std::stringstream ss(params);
	int x,y,z,w,h, filltile;
	x=y=z=w=h=filltile=-65535;
	ss >> x;
	ss >> y;
	ss >> z;
	ss >> w;
	ss >> h;
	ss >> filltile;

	if (x==-65535 || y == -65535 || z == -65535 || w < 1 || h < 1 || filltile < 0) {
		std::cout << "error in create_area_command issued by " << player->name << "!" << std::endl;
		std::cout << "positions: " << x << "\t" << y << '\t' << z << '\t' << w << '\t' << h << '\t' << std::endl;
		return;
	}

	Map* tempmap = new Map(w,h);
	bool disappear=true;
	tempmap->Init(x, y, z, disappear);

	Field* tempf;

	for (int _x=0; _x<w; ++_x)
		for (int _y=0; _y<h; ++_y) {
			if (tempmap->GetPToCFieldAt(tempf, _x+x, _y+y)) {
				tempf->setTileId( filltile );
				tempf->updateFlags();
			} else std::cerr << "error in create map: " << x << " " << y << " " << z << " " << _x << " " << _y << " " << filltile << std::endl;

		}

	world->maps.InsertMap(tempmap);

	std::string tmessage = "map inserted.";
	player->sendMessage( tmessage );
	std::cerr << "Map created by " << player->name << " on " << x << " - " << y << " - " << z << " with w: " << w << " h: " << h << "ft: " << filltile << std::endl;

}

void set_login( World* world, Player* player, const std::string& st) {
    if ( !player->hasGMRight(gmr_loginstate) )return;
	configOptions["disable_login"] = st;
	std::cout << "nologin set to " << st << std::endl;
	std::string tmessage = "nologin set to: " + st;
	player->sendMessage( tmessage );
}

bool World::exportMaps(Player* cp) {
    if ( !cp->hasGMRight(gmr_import) )return false;
	std::string exportdir = directory + std::string( MAPDIR ) + "export/";
    int16_t minX, minY;
	for (MapVector::iterator mapIt = maps.begin(); mapIt != maps.end(); ++mapIt) {
        minX = (*mapIt)->GetMinX();
        minY = (*mapIt)->GetMinY();
		// create base filename
		std::string filebase = exportdir + "e_" + toString(minX) + "_" + toString(minY) + "_" + toString((*mapIt)->Z_Level) + ".";
		// export fields file
		std::ofstream fieldsf((filebase + "tiles.txt").c_str());
		// export items file
		std::ofstream itemsf((filebase + "items.txt").c_str());
        // export warps file
        std::ofstream warpsf((filebase + "warps.txt").c_str());
		if( !fieldsf.good() || !itemsf.good() || !warpsf.good() ) {
			std::cerr << "could not open output files for item export: " << std::endl;
			std::cerr << filebase << "tiles.txt" << std::endl;
			std::cerr << filebase << "items.txt" << std::endl;
            std::cerr << filebase << "warps.txt" << std::endl;
			return false;
		}

        // export tiles header
        fieldsf << "L: " << (*mapIt)->Z_Level << std::endl;
        fieldsf << "X: " << minX << std::endl;
        fieldsf << "Y: " << minY << std::endl;
        fieldsf << "W: " << (*mapIt)->GetWidth() << std::endl;
        fieldsf << "H: " << (*mapIt)->GetHeight() << std::endl;

		// iterate over the map and export...
        short int x, y;
		for (y = minY; y <= (*mapIt)->GetMaxY(); ++y) {
			for (x = minX; x <= (*mapIt)->GetMaxX(); ++x) {
				Field field;
				if ((*mapIt)->GetCFieldAt(field, x, y)) {
					fieldsf << x-minX << ";" << y-minY << ";" << field.getTileCode() << ";" << field.getMusicId() << ";0" << std::endl;
                    if( field.IsWarpField() )
                    {
                        position target;
                        field.GetWarpField( target );
                        warpsf << x-minX << ";" << y-minY << ";" << target.x << ";" << target.y << ";" << target.z << std::endl;
                    }
					ITEMVECTOR itemsv;
					field.giveExportItems(itemsv);
					unsigned short itemnum = 0;
					ITEMVECTOR::iterator it;
                    for (it = itemsv.begin(); it != itemsv.end(); ++it, ++itemnum) {
                        itemsf << x-minX << ";" << y-minY << ";" << itemnum << ";" << it->id << ";" << it->data;
                        if (it->quality != 333) itemsf << ";" << it->quality;
                        itemsf << std::endl;
                    }
				}
			}
		}

        fieldsf.close();
        itemsf.close();
        warpsf.close();

    }

	return true;
}

void World::removeTeleporter( Player* cp, const std::string& ts ) {
    if ( !cp->hasGMRight(gmr_warpfields) )return;
	position teleport;
	char* tokenize = new char[ ts.length() + 1 ];

	strcpy( tokenize, ts.c_str() );
	std::cout << "Tokenizing " << tokenize << std::endl;
	char* thistoken;
	if ( (thistoken = strtok( tokenize, " ," )) != NULL ) {
		if ( ReadField( thistoken, teleport.x ) ) {
			if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
				if ( ReadField( thistoken, teleport.y ) ) {
					if ( (thistoken = strtok( NULL, " ," )) != NULL ) {
						if ( ReadField( thistoken, teleport.z ) ) {
							if ( removeWarpField( teleport) ) {
								std::string tmessage = "*** Warp Field deleted! ***";
								cp->sendMessage( tmessage );
							} else {
								std::string tmessage = "*** Warp Field *NOT* deleted! ***";
								cp->sendMessage( tmessage );
							};
						}
					}
				}
			}
		}
	}
	delete [] tokenize;
}

void World::showWarpFieldsInRange( Player* cp, const std::string& ts ) {
    if ( !cp->hasGMRight(gmr_warpfields) )return;
	short int range = 0;
	if ( ReadField( ts.c_str(), range ) ) {
	    std::vector< boost::shared_ptr< position > > warpfieldsinrange;
		if ( findWarpFieldsInRange( cp->pos, range, warpfieldsinrange ) ) {
			std::vector< boost::shared_ptr< position > >::iterator it;
			std::string message;
			cp->sendMessage( "Start list of warpfields:" );
			for (it = warpfieldsinrange.begin(); it != warpfieldsinrange.end(); ++it) {
                position target;
                GetField( **it )->GetWarpField( target );
				message = "Warpfield at (x,y,z) " + stream_convert<std::string>( (*it)->x ) + "," + stream_convert<std::string> ( (*it)->y ) + "," + stream_convert<std::string>( (*it)->z ) + " Target (x,y,z) : " + stream_convert<std::string>( target.x ) + "," + stream_convert<std::string>( target.y ) + "," + stream_convert<std::string>( target.z );
				cp->sendMessage(message);
			}
			cp->sendMessage( "End list of warpfields." );
		}
	}
}
