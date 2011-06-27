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
#include "NamesObjectTable.hpp"
#include "TilesTable.hpp"
#include "CommonObjectTable.hpp"
#include "script/LuaItemScript.hpp"
#include "TableStructs.hpp"
#include "Player.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "Logger.hpp"

//Table with Item Descriptions for Itemscripts
extern CCommonObjectTable * CommonItems;

bool CWorld::sendTextInFileToPlayer( std::string filename, CPlayer* cp ) {
	if ( filename.length() == 0 ) return false;

	const unsigned char LINE_LENGTH = 255;
	FILE* fp;
	char line[ LINE_LENGTH ];

	fp = fopen( filename.c_str(), "r" );

	if ( fp != NULL ) 
    {
		while ( fgets( line, LINE_LENGTH, fp ) != NULL )
        {
            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( cp->pos.x, cp->pos.y, cp->pos.z, line ) );
            cp->Connection->addCommand(cmd);
		}

		fclose( fp );
		return true;
	} else {
		return false;
	}
}


void CWorld::sendMessageToPlayer( CPlayer* cp, std::string message ) 
{
    //cp->Connection->addCommand( boost::shared_ptr<CBasicServerCommand>( new CSayTC( cp->pos.x, cp->pos.y, cp->pos.z, message ) ) );
    cp->sendMessage(message);
}


void CWorld::sendMessageToAdmin( std::string message) {
	PLAYERVECTOR::iterator titerator;

	for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) 
    {
		if ( (*titerator)->hasGMRight(gmr_getgmcalls) ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( (*titerator)->pos.x, (*titerator)->pos.y, (*titerator)->pos.z, message ) );
			( *titerator )->Connection->addCommand(cmd);
		}
	}
}


std::string CWorld::languagePrefix(int Language) {
	if (Language==0) return "";
	else if (Language==1) return "[hum] ";
	else if (Language==2) return "[dwa] ";
	else if (Language==3) return "[elf] ";
	else if (Language==4) return "[liz] ";
	else if (Language==5) return "[orc] ";
	else if (Language==6) return "[hal] ";
	else if (Language==7) return "[fai] ";
	else if (Language==8) return "[gno] ";
	else if (Language==9) return "[gob] ";
	else if (Language==10) return "[anc] ";
	else return "";
}

std::string CWorld::languageNumberToSkillName(int languageNumber) {
	switch(languageNumber) {
		case 0:
			return "common language";
		case 1:
			return "human language";
		case 2:
			return "dwarf language";
		case 3:
			return "elf language";
		case 4:
			return "lizard language";
		case 5:
			return "orc language";
		case 6:
			return "halfling language";
		case 7:
			return "fairy language";
		case 8:
			return "gnome language";
		case 9:
			return "goblin language";
		case 10:
			return "ancient language";
		default:
			return "";

	}
}

void CWorld::sendMessageToAllPlayers( std::string message ) {

	PLAYERVECTOR::iterator titerator;

	for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) 
    {
        boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( (*titerator)->pos.x, (*titerator)->pos.y, (*titerator)->pos.z, message) );
		( *titerator )->Connection->addCommand(cmd);
	}

}

void CWorld::sendMessageToAllPlayers_2( std::string message[ LANGUAGECOUNT ] ) {

	PLAYERVECTOR::iterator titerator;

	for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) 
    {
        
		if ( ( ( *titerator )->getPlayerLanguage() < LANGUAGECOUNT ) ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( (*titerator)->pos.x, (*titerator)->pos.y, (*titerator)->pos.z, message[ ( *titerator )->getPlayerLanguage()]) );
            ( *titerator )->Connection->addCommand(cmd);
        } 
        else 
        {
            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( (*titerator)->pos.x, (*titerator)->pos.y, (*titerator)->pos.z, std::string( "?") ) );
            (*titerator )->Connection->addCommand(cmd);
		}
        
		        
	}

}


void CWorld::sendLanguageMessageToAllCharsInRange( std::string message, CCharacter::talk_type tt, unsigned char lang, CCharacter* cc)
{
	uint16_t range = 0;
	// how far can we be heard?
	switch (tt) 
    {
		case CCharacter::tt_say:
			range = MAXVIEW;
			break;
		case CCharacter::tt_whisper:
			range = 2;
			break;
		case CCharacter::tt_yell:
			range = 30;
			break;
	}
	//determine spoken language skill

    // get all Players
	std::vector<CPlayer*> players = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
	// get all NPCs
	std::vector<CNPC*> npcs = Npc.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
	// get all Monsters
	std::vector<CMonster*> monsters = Monsters.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

	// alter message because of the speakers inability to speak...
	std::string spokenMessage,tempMessage;
	spokenMessage=cc->alterSpokenMessage(message,cc->getLanguageSkill(cc->activeLanguage));

	// tell all OTHER players... (but tell them what they understand due to their inability to do so)
	// tell the player himself what he wanted to say
	//std::cout << "message in CWorldIMPLTalk:" << message;
	if ((message[0]=='#') && (message[1]=='m') && (message[2]=='e')) 
    {
		for ( PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) 
        {
            if ((*it)->getPlayerLanguage() == static_cast<CLanguage::LanguageType>(lang) )
            {
                (*it)->receiveText(tt, message, cc);
            }
		}
	} 
    else 
    {
		for ( PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) 
        {
            if ( (*it)->getPlayerLanguage() == static_cast<CLanguage::LanguageType>(lang) )
            {
                if ((*it)->id!=cc->id) 
                {
                    tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
                    (*it)->receiveText(tt, tempMessage, cc);
                } 
                else 
                    (*it)->receiveText(tt, languagePrefix(cc->activeLanguage)+message, cc);
            }
		}
	}


	// tell all npcs
	for ( NPCVECTOR::iterator it = npcs.begin(); it != npcs.end(); ++it) {
		tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
		(*it)->receiveText(tt, tempMessage, cc);
	}
	// tell all monsters
	for ( MONSTERVECTOR::iterator it = monsters.begin(); it != monsters.end(); ++it)
		(*it)->receiveText(tt, message, cc);

}


void CWorld::sendMessageToAllCharsInRange( std::string message, CCharacter::talk_type tt, CCharacter* cc) {
	uint16_t range = 0;
	// how far can we be heard?
	switch (tt) {
		case CCharacter::tt_say:
			range = MAXVIEW;
			break;
		case CCharacter::tt_whisper:
			range = 2;
			break;
		case CCharacter::tt_yell:
			range = 30;
			break;
	}
	//determine spoken language skill

	//int activeLanguageSkill;



	// get all Players
	std::vector<CPlayer*> players = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
	    
	// get all NPCs
	std::vector<CNPC*> npcs = Npc.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);
	
	// get all Monsters
	std::vector<CMonster*> monsters = Monsters.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, range);

	// alter message because of the speakers inability to speak...
	std::string spokenMessage,tempMessage;
	spokenMessage=cc->alterSpokenMessage(message,cc->getLanguageSkill(cc->activeLanguage));

	// tell all OTHER players... (but tell them what they understand due to their inability to do so)
	// tell the player himself what he wanted to say
	//std::cout << "message in CWorldIMPLTalk:" << message;
	if ((message[0]=='#') && (message[1]=='m') && (message[2]=='e')) {
		for ( PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
			(*it)->receiveText(tt, message, cc);
		}
	} else {
		for ( PLAYERVECTOR::iterator it = players.begin(); it != players.end(); ++it) {
			if ((*it)->id!=cc->id) {
				tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
				(*it)->receiveText(tt, tempMessage, cc);
			} else (*it)->receiveText(tt, languagePrefix(cc->activeLanguage)+message, cc);
		}
	}


	// tell all npcs
	for ( NPCVECTOR::iterator it = npcs.begin(); it != npcs.end(); ++it) {
	    tempMessage=languagePrefix(cc->activeLanguage)+(*it)->alterSpokenMessage(spokenMessage,(*it)->getLanguageSkill(cc->activeLanguage));
		(*it)->receiveText(tt, tempMessage, cc);
	}
	// tell all monsters
	for ( MONSTERVECTOR::iterator it = monsters.begin(); it != monsters.end(); ++it)
		(*it)->receiveText(tt, message, cc);
}


void CWorld::makeGFXForAllPlayersInRange( short int xc, short int yc, short int zc, int distancemetric ,unsigned short int gfx ) {

	std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( xc, yc, zc, distancemetric );
	std::vector < CPlayer* > ::iterator titerator;

	for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
    {
        boost::shared_ptr<CBasicServerCommand>cmd( new CGraphicEffectTC( xc, yc, zc, gfx ) );
        (*titerator)->Connection->addCommand( cmd );
	}

}


void CWorld::makeSoundForAllPlayersInRange( short int xc, short int yc, short int zc, int distancemetric, unsigned short int sound ) {
	std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( xc, yc, zc, distancemetric );
	std::vector < CPlayer* > ::iterator titerator;

	for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) {
        boost::shared_ptr<CBasicServerCommand>cmd(new CSoundTC( xc, yc, zc, sound ) );
		( *titerator )->Connection->addCommand( cmd );
	}
}



void CWorld::lookAtMapItem( CPlayer* cp, short int x, short int y, short int z ) {

	CField* cfold;
	Item titem;

	if ( GetPToCFieldAt( cfold, x, y, z ) ) 
    { // Feld vorhanden
		if ( cfold->ViewTopItem( titem ) ) 
        { 
            if ( titem.quality >= 100 )
            {
                // mindesten 1 Item vorhanden auf dem alten Feld
                //CommonStrut to find a Item
                /*
                CommonStruct com;
                if ( CommonItems->find( titem.id, com) ) 
                {
                    ScriptItem n_item = titem;
                    n_item.type = ScriptItem::it_field;
                    n_item.pos = position( x, y, z);
                    n_item.owner = cp;
                    // Es gibt ein Script für das Item
                    if ( com.script ) {
                        //Falls ein LookAtItem Script erfolgreich ausgeführt wurde die Funktion beenden.
                        if ( com.script->LookAtItem(cp, n_item) )return;
                    }
                }*/
                boost::shared_ptr<CLuaItemScript> script = CommonItems->findScript( titem.id );
                ScriptItem n_item = titem;
                n_item.type = ScriptItem::it_field;
                n_item.pos = position( x, y, z );
                n_item.owner = cp;
                // Es gibt ein Script für das Item
                if ( script ) 
                {
                     //Falls ein LookAtItem Script erfolgreich ausgeführt wurde die Funktion beenden.
                     if ( script->LookAtItem(cp, n_item) )return;
                }
            }
			if ( ItemNames->find( titem.id, tempNames ) ) 
            { // Namen für das Item gefunden
				std::string outtext;
				switch ( cp->getPlayerLanguage() ) 
                {
					case CLanguage::german:
						outtext +=  tempNames.German ;
						break;
					case CLanguage::english:
						outtext +=  tempNames.English ;
						break;
					case CLanguage::french:
						outtext +=  tempNames.French ;
						break;
					default:
						outtext +=  "unknown mother tongue" ;
				}
                if ( titem.quality < 100 )outtext += " ( " + CLogger::toString(titem.quality) + "% )";
                boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfMapItemTC( x, y, z,  outtext ));
				cp->Connection->addCommand(cmd);
			} 
            else 
            { // kein Name vorhanden, benutze Tile Namen statt dessen
				if ( Tiles->find( cfold->getTileId(), tempTile ) ) 
                { // Namen für die Bodenplatte gefunden
                    boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfMapItemTC( x, y, z,  tempTile.German ));
					switch ( cp->getPlayerLanguage() ) 
                    {
						case CLanguage::german:
                            cp->Connection->addCommand(cmd);
							break;
						case CLanguage::english:
                            cmd.reset( new CNameOfMapItemTC( x, y, z,  tempTile.English ) );
                            cp->Connection->addCommand(cmd);
							break;
						case CLanguage::french:
                            cmd.reset(  new CNameOfMapItemTC( x, y, z,  tempTile.French ) );
                            cp->Connection->addCommand(cmd);
							break;
						default:
                            cmd.reset( new CNameOfMapItemTC( x, y, z,  tempTile.English ) );
                            cp->Connection->addCommand(cmd);
					}
				} else {
					std::cerr << "Tile Nr.: " << cfold->getTileId() << "nicht gefunden\n";
	
					// kein Name vorhanden
                    boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfMapItemTC( x, y, z,  "unbekannt" ));
					switch ( cp->getPlayerLanguage() ) 
                    {
						case CLanguage::german:
                            cp->Connection->addCommand(cmd);
							break;
						case CLanguage::english:
                            cmd.reset( new CNameOfMapItemTC( x, y, z,  "unknown" ) );
                            cp->Connection->addCommand(cmd);
                            break;
						default:
                            cmd.reset( new CNameOfMapItemTC( x, y, z,  "?" ) );
							cp->Connection->addCommand( cmd );
					}
				}
			}
		} 
        else 
        { // Namen der Bodenplatte senden
			if ( Tiles->find( cfold->getTileId(), tempTile ) ) 
            { // Namen für die Bodenplatte gefunden
                boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfMapItemTC( x, y, z,  tempTile.German ));
				switch ( cp->getPlayerLanguage() ) 
                {
					case CLanguage::german:
                        cp->Connection->addCommand(cmd);
						break;
					case CLanguage::english:
                        cmd.reset( new CNameOfMapItemTC( x, y, z,  tempTile.English ) );
                        cp->Connection->addCommand( cmd );
						break;
					default:
                        cmd.reset( new CNameOfMapItemTC( x, y, z,  "?" ) );
                        cp->Connection->addCommand( cmd );
				}
			} 
            else 
            {
				std::cerr << "Tile Nr.: " << cfold->getTileId() << "nicht gefunden\n";

				// kein Name vorhanden
                boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfMapItemTC( x, y, z,  "unbekannt" ) );
				switch ( cp->getPlayerLanguage() ) 
                {
					case CLanguage::german:
                        cp->Connection->addCommand(cmd);
						break;
					case CLanguage::english:
                        cmd.reset( new CNameOfMapItemTC( x, y, z,  "unknown" ) );
                        cp->Connection->addCommand( cmd );
						break;
					default:
                        cmd.reset( new CNameOfMapItemTC( x, y, z,  "?" ) );
                        cp->Connection->addCommand( cmd );
                        break;
				}
			}
		}
	}
}


void CWorld::lookAtMenueItem( CPlayer* cp, unsigned char position, TYPE_OF_ITEM_ID itemid ) 
{
    std::string name="";
	if ( ItemNames->find( itemid, tempNames ) ) { // Namen für das Item gefunden
		switch ( cp->getPlayerLanguage() ) {
			case CLanguage::german:
				name = tempNames.German;
				break;
			case CLanguage::english:
				name = tempNames.English;
				break;
			default:
				name = std::string( "unknown mother tongue" );
		}
	}
    else 
    { // kein Name vorhanden
		switch ( cp->getPlayerLanguage() ) 
        {
			case CLanguage::german:
				name = std::string( "unbekannt" );
				break;
			case CLanguage::english:
				name = std::string( "unknown" );
				break;
			default:
				name = std::string( "?" );
		}
	}
    boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfShowCaseItemTC( 2, position,name) );
	cp->Connection->addCommand(cmd);
}



void CWorld::lookAtShowcaseItem( CPlayer* cp, unsigned char showcase, unsigned char position ) {

	ScriptItem titem;

	if ( showcase < MAXSHOWCASES ) {
		CContainer * ps = cp->showcases[ showcase ].top();
		if ( ps != NULL ) { // source - CContainer gefunden
			CContainer * tc;
			if ( ps->viewItemNr( position, titem, tc ) ) 
            {
				// Ausführen eines LookAt Scriptes
                if ( titem.quality >= 100 )
                {
                    boost::shared_ptr<CLuaItemScript> script = CommonItems->findScript( titem.id );
                    ScriptItem n_item = titem;
                    if ( showcase == 0 )
                        n_item.type = ScriptItem::it_showcase1;
                    else
                        n_item.type = ScriptItem::it_showcase2;
                    n_item.pos = cp->pos;
                    n_item.owner = cp;
                    n_item.itempos = position;
                    // Es gibt ein Script für das Item
                    if ( script ) 
                    {
                        //falls ein LookAtItem ausgeführt werden kann die Funktion beenden
                        if (script->LookAtItem(cp, n_item) )return;
                    }                    
                    /*
                    
                    CommonStruct com;
                    if ( CommonItems->find( titem.id, com) ) 
                    {
                        ScriptItem n_item = titem;
                        if ( showcase == 0 )
                            n_item.type = ScriptItem::it_showcase1;
                        else
                            n_item.type = ScriptItem::it_showcase2;
                        n_item.pos = cp->pos;
                        n_item.owner = cp;
                        n_item.itempos = position;
                        // Es gibt ein Script für das Item
                        if ( com.script ) 
                        {
                            //falls ein LookAtItem ausgeführt werden kann die Funktion beenden
                            if (com.script->LookAtItem(cp, n_item) )return;
                        }
                    }*/
                    // Ende ausführen des LookAt Scriptes
                }
                std::string outtext;
				if ( ItemNames->find( titem.id, tempNames ) ) { // Namen für das Item gefunden

					switch ( cp->getPlayerLanguage()) {
						case CLanguage::german:
							outtext +=  tempNames.German ;
							break;
						case CLanguage::english:
							outtext +=  tempNames.English ;
							break;
						case CLanguage::french:
							outtext +=  tempNames.French ;
							break;
						default:
							outtext +=  "unknown mother tongue" ;
					}
                    if ( titem.quality < 100)outtext += " ( " + CLogger::toString(titem.quality) + "% )";
                } else { // kein Name vorhanden
					switch ( cp->getPlayerLanguage() ) {
						case CLanguage::german:
							outtext = std::string( "unbekannt" );
							break;
						case CLanguage::english:
							outtext = std::string( "unknown" );
							break;
						default:
							outtext = std::string( "?" );
					}
				}
                boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfShowCaseItemTC( showcase, position, outtext) );
				cp->Connection->addCommand(cmd);
			}
		}
	}
}



void CWorld::lookAtInventoryItem( CPlayer* cp, unsigned char position ) 
{
	if ( cp->characterItems[ position ].id != 0 ) 
    { // Position mit Item belegt
        std::string outtext;
		if ( ItemNames->find( cp->characterItems[ position ].id, tempNames ) ) 
        { // Namen für das Item gefunden
			Item titem = cp->characterItems[ position ];
            if ( titem.quality >= 100 )
            {
                // Ausführen eines LookAt Scriptes
                boost::shared_ptr<CLuaItemScript> script = CommonItems->findScript( cp->characterItems[ position ].id );
                ScriptItem n_item = cp->characterItems[ position ];
                if ( position < MAX_BODY_ITEMS )
                    n_item.type = ScriptItem::it_inventory;
                else
                    n_item.type = ScriptItem::it_belt;
                    n_item.itempos = position;
                    n_item.pos = cp->pos;
                    n_item.owner = cp;
                if ( script ) 
                {
                    //Falls ein LookAt Script ausgeführt wurde die Funktion verlassen.
                     if ( script->LookAtItem(cp, n_item) ) return;
                }
                
                /*
                CommonStruct com;
                if ( CommonItems->find( cp->characterItems[ position ].id, com) ) {
                    ScriptItem n_item = cp->characterItems[ position ];
                    if ( position < MAX_BODY_ITEMS )
                        n_item.type = ScriptItem::it_inventory;
                    else
                        n_item.type = ScriptItem::it_belt;
                    n_item.itempos = position;
                    n_item.pos = cp->pos;
                    n_item.owner = cp;
                    if ( com.script ) {
                        //Falls ein LookAt Script ausgeführt wurde die Funktion verlassen.
                        if ( com.script->LookAtItem(cp, n_item) ) return;
                    }
                }
                */
                // Ende ausführen des LookAt Scriptes
            }
			switch ( cp->getPlayerLanguage() ) 
            {
				case CLanguage::german:
					outtext +=  tempNames.German ;
					break;
				case CLanguage::english:
					outtext +=  tempNames.English ;
					break;
				case CLanguage::french:
					outtext +=  tempNames.French ;
					break;
				default:
					outtext += "unknown mother tongue" ;
			}
            if ( titem.quality  < 100 )outtext += " ( " + CLogger::toString(titem.quality) + "% )";
		} else { // kein Name vorhanden
			switch ( cp->getPlayerLanguage() ) {
				case CLanguage::german:
					outtext = std::string( "unbekannt" );
					break;
				case CLanguage::english:
					outtext = std::string( "unknown" );
					break;
				default:
					outtext = std::string( "?" );
			}
		}
        boost::shared_ptr<CBasicServerCommand>cmd( new CNameOfInventoryItemTC( position, outtext) );
		cp->Connection->addCommand(cmd);
	}
}



void CWorld::message( std::string message[3], CPlayer* cp ) 
{
	if (cp != NULL) 
    {

        std::string out="";
		switch ( cp->getPlayerLanguage() ) {
			case CLanguage::german:
				out = message[CLanguage::german];
				break;
			case CLanguage::english:
				out = message[CLanguage::english];
				break;
			case CLanguage::french:
				out = message[CLanguage::french];
				break;
		}
        boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( cp->pos.x, cp->pos.y, cp->pos.z ,out ) );
		cp->Connection->addCommand(cmd);
	}
}

void CWorld::forceIntroducePlayer( CPlayer* cp, CPlayer* Admin ) 
{
	std::string tstring = "";
	if ( (cp->prefix != "") && (cp->prefix != "NULL") )
		tstring = cp->prefix + std::string( " " );
	tstring = tstring + cp->name;
	if ( (cp->suffix != "") && (cp->suffix != "NULL") )
		tstring = tstring + std::string( " " ) + cp->suffix;
    boost::shared_ptr<CBasicServerCommand>cmd( new CIntroduceTC( cp->id,tstring) );    
	Admin->Connection->addCommand(cmd);
}

void CWorld::introduceMyself( CPlayer* cp ) {
	std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( cp->pos.x, cp->pos.y, cp->pos.z, 2 );
	std::vector < CPlayer* > ::iterator titerator;

	for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
    {
		std::string tstring = "";
		if ( (cp->prefix != "") && (cp->prefix != "NULL") )
			tstring = cp->prefix + std::string( " " );
		tstring = tstring + cp->name;
		if ( (cp->suffix != "") && (cp->suffix != "NULL") )
			tstring = tstring + std::string( " " ) + cp->suffix;
        boost::shared_ptr<CBasicServerCommand>cmd( new CIntroduceTC( cp->id,tstring) );
		( *titerator )->Connection->addCommand(cmd);
	}
}

void CWorld::sendWeather( CPlayer* cp)
{
    cp->sendWeather( weather );
}

void CWorld::sendIGTime( CPlayer* cp)
{
    boost::shared_ptr<CBasicServerCommand>cmd( new CUpdateTimeTC(static_cast<unsigned char>(getTime("hour")),static_cast<unsigned char>(getTime("minute")),static_cast<unsigned char>(getTime("day")),static_cast<unsigned char>(getTime("month")), static_cast<short int>(getTime("year")) ));
    cp->Connection->addCommand(cmd);  
}

void CWorld::sendIGTimeToAllPlayers()
{
    PLAYERVECTOR::iterator titerator;
    for ( titerator = Players.begin(); titerator != Players.end(); ++titerator )
    {
        sendIGTime( (*titerator) );
    }
}

void CWorld::sendWeatherToAllPlayers()
{
    PLAYERVECTOR::iterator titerator;
	for ( titerator = Players.begin(); titerator != Players.end(); ++titerator ) 
    {
		(*titerator)->sendWeather( weather );
	}
}

