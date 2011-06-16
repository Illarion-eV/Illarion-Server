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


#ifndef _CLIENTCOMMANDS_HPP_
#define _CLIENTCOMMANDS_HPP_



/**
*@defgroup Clientcommands Clientcommands
*@ingroup Netinterface
*commands which are sent by the client
*/

/**
*@ingroup Clientcommands
*@file ClientCommands.hpp
*holds the implementation + definition of all the client commands
*/

#include "CPlayer.hpp"
#include "CWorld.hpp"
#include "CCharacter.hpp"
#include <string>
#include "types.h"
#include "CLogger.hpp"
#include "CWeaponObjectTable.h"
#include "CCommonObjectTable.h"
#include "CMonsterTable.hpp"
#include "CTilesTable.h"
#include <boost/shared_ptr.hpp>
#include "script/CLuaNPCScript.hpp"
#include "script/CLuaScript.hpp"
#include "script/CLuaItemScript.hpp"
#include "script/CLuaMagicScript.hpp"
#include "script/CLuaMonsterScript.hpp"
#include "script/CLuaTileScript.hpp"
#include "script/CLuaLookAtPlayerScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/CBasicClientCommand.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"


#include <list>

extern CWeaponObjectTable* WeaponItems;
extern CMonsterTable* MonsterDescriptions;
extern CCommonObjectTable* CommonItem;
extern CTilesTable* Tiles;
extern boost::shared_ptr<CLuaLookAtPlayerScript>lookAtPlayerScript;

/**
*@ingroup Clientcommands
*defines the definition bytes of the different client commands
*/
enum clientcommands
{
    CC_LOGIN_TS = 0x0D, /*<login*/
    CC_SCREENSIZE_TS = 0xA0,
    CC_CHARMOVE_TS = 0x10,
    CC_IMOVERSTART_TS = 0x20,
    CC_IMOVEREND_TS = 0x2A,
    CC_PSPINRSTART_TS = 0x40,
    CC_PSPINREND_TS = 0x49,
    CC_LOOKATMAPITEM_TS = 0xFF,
    CC_LOOKATCHARACTER_TS = 0x18,
    CC_USE_TS = 0xFE,
    CC_CAST_TS = 0xFD,
    CC_ATTACKPLAYER_TS = 0xFA,
    CC_INTRODUCE_TS = 0xF6,
    CC_SAY_TS = 0xF5,
    CC_SHOUT_TS = 0xF4,
    CC_WHISPER_TS = 0xF3,
    CC_REFRESH_TS = 0xF2,
    CC_LOGOUT_TS = 0xF1,
    CC_LOOKINTOCONTAINERONFIELD_TS = 0xEC,
    CC_LOOKINTOINVENTORY_TS = 0xEB,
    CC_LOOKINTOSHOWCASECONTAINER_TS = 0xEA,
    CC_CLOSECONTAINERINSHOWCASE_TS = 0xE9,
    CC_DROPITEMFROMSHOWCASEONMAP_TS = 0xE8,
    CC_MOVEITEMBETWEENSHOWCASES_TS = 0xE7,
    CC_MOVEITEMFROMMAPINTOSHOWCASE_TS = 0xE6,
    CC_MOVEITEMFROMMAPTOPLAYER_TS = 0xE5,
    CC_DROPITEMFROMPLAYERONMAP_TS = 0xE4,
    CC_MOVEITEMINSIDEINVENTORY_TS = 0xE3,
    CC_MOVEITEMFROMSHOWCASETOPLAYER_TS = 0xE2,
    CC_MOVEITEMFROMPLAYERTOSHOWCASE_TS = 0xE1,
    CC_LOOKATSHOWCASEITEM_TS = 0xE0,
    CC_LOOKATINVENTORYITEM_TS = 0xDF,
    CC_ATTACKSTOP_TS = 0xDE,
    CC_REQUESTSKILLS_TS = 0xDD,
    CC_LOOKATMENUITEM_TS = 0xDC,
    CC_KEEPALIVE_TS = 0xD8,
    CC_REQUESTAPPEARANCE_TS = 0x0E

};

/**
*@ingroup Netinterface
*simple class for storing data in a fifo implemented stack for the use commands
*/
class ByteStack
{
    public:
        ByteStack() : ok(true){};
        ~ByteStack()
        {
            stack.clear();
            paramstack.clear();
        };

        void push( unsigned char v )
        {
            stack.push_back( v );
        }

        unsigned char pop()
        {
            if ( !stack.empty() )
            {
                unsigned char ret = stack.front();
                stack.pop_front();
                return ret;
            }
            ok = false;
            return 0;
        }

        void push( short int v )
        {
            paramstack.push_back( v );
        }

        short int popparam()
        {
            if ( !paramstack.empty() )
            {
                short int ret = paramstack.front();
                paramstack.pop_front();
                return ret;
            }
            ok = false;
            return 0;
        }

        bool ok; //<will be false if more data wants to be red than inside the stack

    private:
        std::list<unsigned char>stack;
        std::list<short int>paramstack;
};

/**
*@ingroup Clientcommands
*client asked to send the appearance of another char
*/
class CRequestAppearanceTS : public CBasicClientCommand
{
    public:
        CRequestAppearanceTS() : CBasicClientCommand( CC_REQUESTAPPEARANCE_TS )
        {
        }

        virtual ~CRequestAppearanceTS(){};

        void decodeData()
        {
            id = getIntFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            CCharacter * ch = CWorld::get()->Players.findID( id );
            if ( ch == NULL )
            {
                ch = CWorld::get()->Monsters.findID( id );
                if ( ch == NULL )
                {
                    ch = CWorld::get()->Npc.findID( id );
                }
            }

            //check if we found a character befor
            if ( ch != NULL )
            {
                ch->updateAppearanceForPlayer( player, true );
            }

        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd(new CRequestAppearanceTS());
            return cmd;
        }

        TYPE_OF_CHARACTER_ID id;
};

/**
*@ingroup Clientcommands
*player looks at a character
*/
class CLookAtCharacterTS : public CBasicClientCommand
{
    public:
        /**
        *a player attacked another char
        */
        CLookAtCharacterTS() : CBasicClientCommand( CC_LOOKATCHARACTER_TS )
        {
            id = 0;
            mode = 0;
        }

        virtual ~CLookAtCharacterTS()
        {

        }

        void decodeData()
        {
            id = getIntFromBuffer();
            mode = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
             time( &(player->lastaction) );
             if ( id < MONSTER_BASE )
             {
                 CPlayer * pl = CWorld::get()->Players.findID( id );
                 if ( pl )
                 {
                     if ( lookAtPlayerScript )
                     {
                         lookAtPlayerScript->lookAtPlayer(player, pl, mode);
                     }
                 }
                 //code for player handling
             }
             else if ( id >= MONSTER_BASE && id < NPC_BASE )
             {
                 CMonster * monster = CWorld::get()->Monsters.findID( id );
                 if ( monster )
                 {
                     MonsterStruct mon;
                     if ( MonsterDescriptions->find( monster->getType(), mon ) )
                     {
                         if ( mon.script )
                         {
                             if ( mon.script->lookAtMonster(player, monster, mode ) )return;
                         }
                         std::string outtext;
                         switch ( player->getPlayerLanguage() )
                         {
                             case CLanguage::german:
                                 outtext = "Das ist: " + mon.name ;
                                break;
                             case CLanguage::english:
                                outtext =  "This is: " + mon.name ;
                                break;
                             default:
                                outtext +=  "unknown mother tongue" ;
                         }
                         boost::shared_ptr<CBasicServerCommand>cmd( new CCharDescription( id, outtext ) );
                         //player->inform( outtext );
                         player->Connection->addCommand( cmd );
                     }
                 }
             }
             else
             {
                 CNPC * npc = CWorld::get()->Npc.findID( id );
                 if ( npc )
                 {
                     if ( npc->getScript() )
                     {
                         if (npc->getScript()->lookAtNpc( player, mode ))return;
                     }
                     std::string outtext;
                     switch ( player->getPlayerLanguage() )
                     {
                         case CLanguage::german:
                             outtext = "Das ist: " + npc->name ;
                            break;
                         case CLanguage::english:
                            outtext =  "This is: " + npc->name ;
                            break;
                         default:
                            outtext +=  "unknown mother tongue" ;
                     }
                     boost::shared_ptr<CBasicServerCommand>cmd( new CCharDescription( id, outtext ) );
                     //player->inform( outtext );
                     player->Connection->addCommand( cmd );

                 }
             }


        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookAtCharacterTS() );
            return cmd;
        }

        TYPE_OF_CHARACTER_ID id;
        uint8_t mode;
};

/**
*@ingroup Clientcommands
*character casts something
*/
class CCastTS : public CBasicClientCommand
{
    public:
        CCastTS() : CBasicClientCommand( CC_CAST_TS )
        {
            cid = 0;
            xc = 0;
            yc = 0;
            zc = 0;
            pos = 0;
            paramtemp = 0;
            counter = 0;
            showcase = 0;

        }

        virtual ~CCastTS()
        {
        };

        void decodeData()
        {
            spellId = static_cast<unsigned long int>(getIntFromBuffer() );
            cid = getUnsignedCharFromBuffer();
            switch ( cid )
            {
                case UID_KOORD:
                    xc = static_cast<short int>( getShortIntFromBuffer() );
                    yc = static_cast<short int>( getShortIntFromBuffer() );
                    zc = static_cast<short int>( getShortIntFromBuffer() );
                    break;

                case UID_SHOWC:
                    showcase = getUnsignedCharFromBuffer();
                    pos = getUnsignedCharFromBuffer();
                    break;
                case UID_INV:
                    pos = getUnsignedCharFromBuffer();
                    break;
                case UID_VAR:
                    paramtemp = getShortIntFromBuffer();
                    break;
                case UID_MAGICWAND:
                    break;
            }
            //counter = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " is casting!");
            CLogger::writeMessage("Casting", player->name + " is casting!");

			bool paramOK = true;
			//CScript* skript = NULL;

			boost::shared_ptr<CLuaMagicScript> LuaMageScript;
			// berprfen, ob der Spieler die Runen beherrscht
			if ( ( spellId & player->magic.flags[ player->magic.type ] ) == spellId )
            {
                SpellStruct CastedSpell;
				if ( Spells->find(spellId,player->magic.type, CastedSpell ) )LuaMageScript = CastedSpell.script;
			}
            CLogger::writeMessage("Casting", player->name + " can't cast the spell: " + CLogger::toString(spellId) + " , flags: " + CLogger::toString(player->magic.flags[ player->magic.type ]),false);

			//Source des Castens zuweisen
			SouTar Source, Target;
			Source.Character = dynamic_cast<CCharacter*>(player);
			Source.pos = player->pos;
			Source.Type = LUA_CHARACTER;

            switch( cid )
            {
                case UID_KOORD:

                    CField * temp;
			        CLogger::writeMessage("Casting",player->name + " trys to cast on a coordinate pos(" + CLogger::toString(xc) + "," + CLogger::toString(yc) + "," + CLogger::toString(zc)+")",false);

					if ( LuaMageScript )
                    {
						if ( !CWorld::get()->GetPToCFieldAt( temp, xc, yc, zc ) )
                        {
							CLogger::writeError("CWorld_Debug", "cant find field for casting at pos(" + CLogger::toString(xc) + "," + CLogger::toString(yc) + "," + CLogger::toString(zc) + ") !");
					        paramOK = false;
						}
                        else
                        {
							// Feld gefunden
							if ( temp->IsPlayerOnField() || temp->IsMonsterOnField() || temp->IsNPCOnField() )
                            {
								CCharacter * tmpCharacter = CWorld::get()->findCharacterOnField( xc, yc, zc );
								if ( tmpCharacter != NULL )
                                {
                                    //Nothing to do here
                                }
                                else
                                {
                                    CLogger::writeMessage("Casting","Character found at target field!",false);
                                }
								// Character auf Feld ist ein Spieler
								if ( ( tmpCharacter->character == CCharacter::player ) && ( LuaMageScript ) )
                                {
									CLogger::writeMessage("Casting","Target Character: player",false);
                                    //Lua Script zuweisung
									Target.Character = tmpCharacter;
									Target.pos = tmpCharacter->pos;
									Target.Type = LUA_CHARACTER;
								}
								// Character auf Feld ist ein NPC
								else if ( ( tmpCharacter->character == CCharacter::npc ) && ( LuaMageScript ) )
                                {
                                    CLogger::writeMessage("Casting","Target Character: NPC",false);
									//Lua Script zuweisung
									Target.Character = tmpCharacter;
									Target.pos = tmpCharacter->pos;
									Target.Type = LUA_CHARACTER;
								}
                                else if ( ( tmpCharacter->character == CCharacter::monster ) && ( LuaMageScript ) )
                                {
									CLogger::writeMessage("Casting","Target Character: monster",false);
                                    //Lua Script zuweisung
									Target.Character = tmpCharacter;
									Target.pos = tmpCharacter->pos;
									Target.Type = LUA_CHARACTER;
								}
							}
                            else
                            {
								Item it;
								if ( temp->ViewTopItem( it ) )
                                {
                                    CLogger::writeMessage("Casting","Item found at target field!",false);
									if ( LuaMageScript )
                                    {
										//Lua Script zuweisung
										Target.pos = position(xc, yc, zc); //Position des TargetItems
										Target.Type = LUA_ITEM;
										Target.item = it;
										Target.item.type = ScriptItem::it_field;
										Target.item.pos = position(xc, yc, zc); //Position des TargetItems
										Target.item.owner = player;
									}
								}
                                else
                                {
                                    CLogger::writeMessage("Casting","empty field!",false);

									if ( LuaMageScript )
                                    {
										//Lua Script zuweisung
										Target.pos = position(xc, yc, zc); //Position des TargetItems
										Target.Type = LUA_FIELD;
									}
								} // Item oder leer ?
							} // character ?
						} // Feld vorhanden ?
					} // skript != NULL ?
					else
                    {
						std::cerr<<"LuaMageScript false, paramOK = false!"<<std::endl;
						paramOK = false;
					}
					break;

                case UID_SHOWC:

#ifdef CWorld_DEBUG
					std::cout << "showcase: " << ( int ) showcase << " pos: " << ( int ) pos << std::endl;
#endif

                    CLogger::writeMessage("Casting", player->name + " is casting in showcas: "+CLogger::toString(showcase)+" pos "+CLogger::toString(pos));
					if ( LuaMageScript )
                    {
						if ( showcase < MAXSHOWCASES )
                        {
							CContainer * ps = player->showcases[ showcase ].top();
							if ( ps != NULL )
                            {
#ifdef CWorld_DEBUG
								std::cout << "CContainer gefunden" << std::endl;
#endif
								ScriptItem tempi;
								CContainer* tempc;
								if ( ps->viewItemNr( pos, tempi, tempc ) )
                                {
#ifdef CWorld_DEBUG
									std::cout << "pos gefunden" << std::endl;
#endif
									if ( LuaMageScript )
                                    {
										Target.Type = LUA_ITEM;
										ps->viewItemNr( pos, Target.item, tempc );
										Target.item.pos = position(xc, yc, zc);
										if ( showcase == 0 )Target.item.type = ScriptItem::it_showcase1;
										else Target.item.type = ScriptItem::it_showcase2;
										Target.item.itempos = pos;
										Target.item.owner = player;
										Target.pos = position(xc, yc, zc);
									}
								}
                                else
                                {
									std::cerr<<"ps->viewItemNr false, paramOK = false!"<<std::endl;
									paramOK = false;
								}
							}
                            else
                            {
								std::cerr<<"ps!=Null false, paramOK = false!"<<std::endl;
								paramOK = false;
							}
						}
                        else
                        {
							std::cerr << "showcase < MAXSHOWCASE false, paramOK = false!"<<std::endl;
							paramOK = false;
						}
					} // LuaMageScript == NULL ?
					break;
				case UID_MAGICWAND:

				//UID_MAGICWAND wird immer gesandt wenn kein Ziel gewaehlt wird.
#ifdef CWorld_DEBUG
					std::cout << "UID_MAGICWAND" << std::endl;
#endif
                    CLogger::writeMessage("Casting","Cast with Wand",false);
					if ( (player->attackmode) && ( player->enemyid != 0 ) && (  LuaMageScript ) )
                    {
						bool zauberstab=false;

						if (( player->characterItems[ LEFT_TOOL ].id != 0 ) && ( player->characterItems[ LEFT_TOOL ].id != BLOCKEDITEM ) ) {
							if ( WeaponItems->find( player->characterItems[ LEFT_TOOL ].id, tempWeapon ) )
                            {
								if ( tempWeapon.WeaponType == 13 )
                                {
									zauberstab = true;
									std::cout << "Zauberstab in der Hand -> OK" << std::endl;
								}
							}
						}

						if (( player->characterItems[ RIGHT_TOOL ].id != 0 ) && ( player->characterItems[ RIGHT_TOOL ].id != BLOCKEDITEM ) )
                        {
							if ( WeaponItems->find( player->characterItems[ RIGHT_TOOL ].id, tempWeapon ) )
                            {
								if ( tempWeapon.WeaponType == 13 )
                                {
									zauberstab = true;
									std::cout << "Zauberstab in der Hand -> OK" << std::endl;
								}
							}
						}


						if (zauberstab)
                        {
							switch (player->enemytype)
                            {

									//Muss spaeter angepasst werden wenn es nur noch einen Charactervektor gibt.

								case CCharacter::player:
#ifdef CWorld_DEBUG
									std::cout << "Gegner ist ein Spieler" << std::endl;
#endif
									if ( LuaMageScript )
                                    {
										Target.Type = LUA_CHARACTER;
										Target.Character = CWorld::get()->findCharacter( player->enemyid );
										if ( Target.Character != NULL ) Target.pos = Target.Character->pos;
										else
                                        {
											paramOK = false;
											std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
										}
									}
									break;

								case CCharacter::npc:
#ifdef CWorld_DEBUG
									std::cout << "Gegner ist ein NPC" << std::endl;
#endif
									if ( LuaMageScript )
                                    {
										Target.Type = LUA_CHARACTER;
										Target.Character = CWorld::get()->findCharacter( player->enemyid );
										if ( Target.Character != NULL ) Target.pos = Target.Character->pos;
										else
                                        {
											paramOK = false;
											std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
										}
									}
									break;

								case CCharacter::monster:
#ifdef CWorld_DEBUG
									std::cout << "Gegner ist ein Monster" << std::endl;
#endif

									if ( LuaMageScript )
                                    {
										Target.Type = LUA_CHARACTER;
										Target.Character = CWorld::get()->findCharacter( player->enemyid );
										if ( Target.Character != NULL ) Target.pos = Target.Character->pos;
										else {
											paramOK = false;
											std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
										}
									}
									break;
							} // switch
						} // zauberstab
						else
                        {
							std::cout<<"Zauberstab = false, paramOK = false!"<<std::endl;
							paramOK = false;
						}

					} // enemyid != 0
					else
                    {

						if ( !LuaMageScript  )
                        {
							std::cout<<"LuaMageScript nicht gesetzt, paramOK = false!"<<std::endl;
							paramOK = false;
						}
					}
					break;

				case UID_INV:

#ifdef CWorld_DEBUG
					std::cout << "cast mit Inv" << std::endl;
#endif
                    CLogger::writeMessage("Casting","cast in inventory",false);
					if ( LuaMageScript )
                    {
						if ( pos < ( MAX_BELT_SLOTS + MAX_BODY_ITEMS ) ) {
#ifdef CWorld_DEBUG
							std::cout << "gltiger Wert" << std::endl;
#endif
							if ( player->characterItems[ pos ].id != 0 )
                            {
#ifdef CWorld_DEBUG
								std::cout << "Position " << ( int ) pos << " am Koerper ist besetzt" << std::endl;
#endif
								if ( LuaMageScript )
                                {
									Target.Type = LUA_ITEM;
									Target.item = (ScriptItem)player->characterItems[ pos ];
									Target.item.pos = player->pos;
									if ( pos < MAX_BODY_ITEMS )
										Target.item.type = ScriptItem::it_inventory;
									else
										Target.item.type = ScriptItem::it_belt;
									Target.item.itempos = pos;
									Target.item.owner = player;
									Target.pos = player->pos;
								}
							} else {
								std::cerr<<"cp->characterItems[pos].id != 0 false, paramOK = false!"<<std::endl;
								paramOK = false;
							}
						} else {
							std::cerr<<"pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS), paramOK = false!"<<std::endl;
							paramOK = false;
						}
					} // skript != NULL ?

					break;

            } // end of switch CID

			if ( LuaMageScript )
            {
                CLogger::writeMessage("Casting","try to call magic script",false);
                player->ltAction->setLastAction( LuaMageScript, Source, Target, counter , paramtemp, CLongTimeAction::AT_MAGIC);
                std::string msg;
#ifdef CWorld_DEBUG
				std::cout<<"Try to call LuaMageScript with paramtemp: "<<paramtemp<<std::endl;
				std::cout<<"paramOK: "<<paramOK<<std::endl;
#endif
#ifdef DO_UNCONSCIOUS
				if ( ( paramOK ) && player->IsAlive() && player->IsConscious() && (player->GetStatus() < 10) )
#else
				if ( ( paramOK ) && player->IsAlive() && (player->GetStatus() < 10) )
                {
#endif

					switch ( Target.Type )
                    {
						case LUA_NONE:
							LuaMageScript->CastMagic(player,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION) );
                            msg = "Casted spell: " + CLogger::toString(spellId);
							break;
						case LUA_FIELD:
							LuaMageScript->CastMagicOnField(player,Target.pos,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                            msg = "Casted spell: " + CLogger::toString(spellId) + " on field at pos(" + CLogger::toString( Target.pos.x) + "," + CLogger::toString( Target.pos.y ) + "," + CLogger::toString( Target.pos.z ) + ")";
							break;
						case LUA_CHARACTER:
							LuaMageScript->CastMagicOnCharacter(player,Target.Character,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
							if ( Target.Character->character == CCharacter::monster ) {
								MonsterStruct monStruct;
								CMonster * temp = dynamic_cast<CMonster*>(Target.Character);
								if ( MonsterDescriptions->find( temp->getType(), monStruct) ) {
									if ( monStruct.script ) {
										monStruct.script->onCasted(temp,player);
									}
								} else {
									std::cerr<<"Didn't finde Monster Description for: "<< temp->getType() << " can't call onCasted!"<<std::endl;
								}

							}
                            msg = "Casted spell: " + CLogger::toString(spellId) + " on character: " + Target.Character->name + "(" + CLogger::toString(Target.Character->id) + ")";
							break;
						case LUA_ITEM:
							LuaMageScript->CastMagicOnItem(player,Target.item,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                            msg = "Casted spell: " + CLogger::toString(spellId) + " on item: " + CLogger::toString(Target.item.id);
							break;
						default:
							LuaMageScript->CastMagic(player,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                            msg = "Casted spell: " + CLogger::toString(spellId) + " on item: " + CLogger::toString(Target.item.id);
					} //Ende Switch
                    //monitoringClientList->sendCommand( new CSendActionTS( player->id, player->name, 2, msg));
                } //ENde if player->IsAlive
                CLogger::writeMessage("Casting","all succeeded",false);
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd(new CCastTS() );
            return cmd;

        }

        unsigned char showcase;
        unsigned char pos;
        short int xc,yc,zc;
        short int paramtemp;
        unsigned char cid;
        unsigned char counter;
        unsigned long int spellId;
};


/**
*@ingroup Clientcommands
*character uses something
*/
class CUseTS : public CBasicClientCommand
{
    public:
        CUseTS() : CBasicClientCommand( CC_USE_TS )
        {
        }

        virtual ~CUseTS()
        {
        };

        void decodeData()
        {
            unsigned char count = getUnsignedCharFromBuffer();
			bs.push( count );
            if ( count > 3 )
            {
                CLogger::writeMessage("CWorld_Debug ", "count:" + CLogger::toString(count) + " to large, set to 3!");
				count = 3;
			}
            for ( ; count > 0 ; --count)
            {
                unsigned char uid = getUnsignedCharFromBuffer();
                bs.push( uid );
                switch ( uid )
                {
                    case UID_SKILL:
                        break;

                    case UID_KOORD:
                        bs.push( getShortIntFromBuffer() );
                        bs.push( getShortIntFromBuffer() );
                        bs.push( getShortIntFromBuffer() );
                        break;

                    case UID_SHOWC:
                        bs.push( getUnsignedCharFromBuffer() );
                        bs.push( getUnsignedCharFromBuffer() );
                        break;

                    case UID_INV:
                        bs.push( getUnsignedCharFromBuffer() );
                        break;
                    case UID_VAR:
                        bs.push( getShortIntFromBuffer() );
                        break;
                }
            }
            counter = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
std::cout << "USE_PERFORM_START" << std::endl;
            time( &(player->lastaction) );
            player->ltAction->abortAction();
			unsigned char count = bs.pop();
			if ( count > 3 )
            {
                CLogger::writeMessage("CWorld_Debug ", "count:" + CLogger::toString(count) + " to large, set to 3!");
				count = 3;
			}

            CLogger::writeMessage("Use", player->name + " uses something, count: " + CLogger::toString(static_cast<int>(count)));
 			bool first = true;
			bool paramOK = true;
			//CScript* skript = NULL;

			//Parameter fr Lua Item Scripte
			boost::shared_ptr<CLuaItemScript> LuaScript;
			boost::shared_ptr<CLuaNPCScript> LuaNPCScript;
			boost::shared_ptr<CLuaMonsterScript> LuaMonsterScript;
			boost::shared_ptr<CLuaTileScript> LuaTileScript;
			SouTar Source, Target;
			CommonStruct com;
			TilesStruct Tile;

			unsigned char showcase = 0;
			unsigned char pos = 0;
			short int xc = 0;
			short int yc = 0;
			short int zc = 0;
			short int paramtemp = 0;

			for ( ; count > 0; --count )
            {
				unsigned char uid = bs.pop();
                CLogger::writeMessage("Use", "use ID :" + CLogger::toString(static_cast<int>(uid)), false );
				switch ( uid )
                {
					case UID_SKILL:
						break;

					case UID_KOORD:

						CField* temp;
						xc = static_cast<short int>(bs.popparam());
						yc = static_cast<short int>(bs.popparam());
						zc = static_cast<short int>(bs.popparam());

                        CLogger::writeMessage("Use", "UID_KOORD",false);
                        CLogger::writeMessage("Use","xc: " +CLogger::toString(static_cast<int>(xc) ) + " yc: " + CLogger::toString( static_cast<int>(yc) ) + " zc: " + CLogger::toString( static_cast<int>(zc) ), false);

                        if ( !CWorld::get()->GetPToCFieldAt( temp, xc, yc, zc ) )
                        {
							CLogger::writeError("CWorld_Debug","Use UID_KOORD field not found!");
                            CLogger::writeMessage("Use","Use UID_KOORD field not found at pos ( " + CLogger::toString(static_cast<int>(xc) ) + "," + CLogger::toString( static_cast<int>(yc) ) + "," + CLogger::toString( static_cast<int>(zc) ) + ")");
							paramOK = false;
						}
                        else
                        { // Feld gefunden
							//Prfen ob sich irgendeine art Char auf dem Feld befindet (Spaeter nur noch IsCharOnField vorerst noch alle Arten pruefen
							if ( temp->IsPlayerOnField() || temp->IsNPCOnField() || temp->IsMonsterOnField() )
                            {
                                CLogger::writeMessage("Use", "Character on field found!", false);
								CCharacter *tmpCharacter = CWorld::get()->findCharacterOnField(xc, yc, zc);
								if (tmpCharacter != NULL)
                                {
									//Zuweisen des Targets fr LuaScripte wenn ParamOK true ist, ein LuaScriptGeladen wurden und es nicht der erste Durchgang ist
									if ( !first && paramOK && ( LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript ) )
                                    {
										Target.pos = position( xc, yc, zc );
										Target.Character = tmpCharacter;
										Target.Type = LUA_CHARACTER;
									} //Wenn kein Luascript geladen ist oder es der erste durchgang ist.
									else if (tmpCharacter->character == CCharacter::player)
                                    {
                                        CLogger::writeMessage("Use","Character is a player!",false);
										if ( first )
                                        {
										     //TODO Add Playerrace scripts
										}
									}// end tmpCharacter->type == CCharacter::player
									else if (tmpCharacter->character == CCharacter::npc)
                                    {
                                        CLogger::writeMessage("Use","Character is a NPC!", false);
										if ( first )
                                        {
											CNPC * scriptNPC = dynamic_cast<CNPC*>(tmpCharacter);
											LuaNPCScript = scriptNPC->getScript();
											if ( LuaNPCScript )
                                            {
												Source.pos = scriptNPC->pos;
												Source.Character = scriptNPC;
												Source.Type = LUA_CHARACTER;
											}
										}
									}// end else if ( tmpCharacter->type == CCharacter::NPC )
									else if ( tmpCharacter->character == CCharacter::monster )
                                    {
                                        CLogger::writeMessage("Use","Character is a monster!",false);
										if ( first )
                                        {
											CMonster * scriptMonster = dynamic_cast<CMonster*>(tmpCharacter);
											MonsterStruct monStruct;
											if ( MonsterDescriptions->find(scriptMonster->getType(),monStruct) )
                                            	LuaMonsterScript = monStruct.script;
											else
                                            {
												CLogger::writeError("CWorld_Debug","try to use Monster but id: " + CLogger::toString(scriptMonster->getType()) +" not found in database!");
                                                CLogger::writeMessage("Use","try to use Monster but id: " + CLogger::toString(scriptMonster->getType()) +" not found in database!", false);
											}
											if ( LuaMonsterScript )
                                            {
												Source.pos = scriptMonster->pos;
												Source.Character = scriptMonster;
												Source.Type = LUA_CHARACTER;
											}
										}
									}//end if ( tmpCharacter->type == CCharacter::Monster )
								} else {
									CLogger::writeError("CWorld_Debug", "Character on field (" + CLogger::toString(xc) + "," + CLogger::toString(yc) + "," + CLogger::toString(zc) + ") not found!");
                                    CLogger::writeMessage("Use", "Character on field (" + CLogger::toString(xc) + "," + CLogger::toString(yc) + "," + CLogger::toString(zc) + ") not found!", false);
                                }
							} //end  temp->IsPlayerOnField() || temp->IsNPCOnField() || temp->IsMonsterOnField()
							else
                            {
                                CLogger::writeMessage("Use","no character on field!", false);
								Item it;
								if ( temp->ViewTopItem( it ) ) {
                                    CLogger::writeMessage("Use","Item on field", false);
									if ( first )
                                    {
										LuaScript = CommonItems->findScript( it.id );
										if ( LuaScript )
                                        {
											Source.Type = LUA_ITEM;
											temp->ViewTopItem(Source.item);
											Source.item.pos = position(xc, yc, zc); //Position des SourceItems
											Source.item.type = ScriptItem::it_field; //Position des SourceItems
											Source.item.owner = player; //Owner des Items
											Source.pos = position(xc, yc, zc);
											first = false;
										}
									}
                                    else if ( LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript )
                                    {
										Target.Type = LUA_ITEM;
										temp->ViewTopItem(Target.item);
										Target.item.pos = position(xc, yc, zc);
										Target.item.type = ScriptItem::it_field;
										Target.item.owner = player;
										Target.pos = position(xc, yc, zc);
									}
								} else {
                                    CLogger::writeMessage("Use","empty field!",false);
                                    if ( first )
                                    {

										if ( Tiles->find(temp->getTileId(),Tile) )
											LuaTileScript = Tile.script;
										if ( LuaTileScript )
                                        {
											Source.Type = LUA_FIELD;
											Source.pos = position(xc, yc, zc);
											first = false;
										}
									}
                                    else if ( LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript )
                                    {
										Target.Type = LUA_FIELD;
										Target.pos = position(xc, yc, zc);
									}
								}
							}
						}

						break;

					case UID_SHOWC:

						showcase = bs.pop();
						pos = bs.pop();
                        CLogger::writeMessage("Use", "showcase: " + CLogger::toString(static_cast<int>(showcase) ) + " pos: " + CLogger::toString(static_cast<int>(pos) ),false);
						if ( showcase < MAXSHOWCASES )
                        {
							CContainer * ps = player->showcases[ showcase ].top();
							if ( ps != NULL )
                            {
                                 CLogger::writeMessage("Use", "Container gefunden!", false);
								ScriptItem tempi;
								CContainer* tempc;
								if ( ps->viewItemNr( pos, tempi, tempc ) )
                                {
                                    CLogger::writeMessage("Use", "pos found item id: " + CLogger::toString(tempi.id),false);
									if ( first )
                                    {
										LuaScript = CommonItems->findScript(tempi.id);
										if ( LuaScript )
                                        {
											Source.Type = LUA_ITEM;
											ps->viewItemNr( pos, Source.item, tempc );
											Source.item.pos = position(xc, yc, zc);
											if ( showcase == 0 )
												Source.item.type = ScriptItem::it_showcase1;
											else
												Source.item.type = ScriptItem::it_showcase2;
											Source.item.itempos = pos;
											Source.item.owner = player;
											Source.pos = position(xc, yc, zc);
											first = false;
										}
									}
                                    else if ( LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript )
                                    {
										Target.Type = LUA_ITEM;
										ps->viewItemNr( pos, Target.item, tempc );
										Target.pos = player->pos;
										Target.item.pos = position(xc, yc, zc);
										if ( showcase == 0 )
											Target.item.type = ScriptItem::it_showcase1;
										else
											Target.item.type = ScriptItem::it_showcase2;
										Target.item.itempos = pos;
										Target.item.owner = player;
									}
								}
                                else
                                {
									paramOK = false;
								}
							}
                            else
                            {
								paramOK = false;
							}
						}
                        else
                        {
							paramOK = false;
						}

						break;

					case UID_INV:

						pos = bs.pop();
						if ( pos < ( MAX_BELT_SLOTS + MAX_BODY_ITEMS ) )
                        {
                            CLogger::writeMessage("Use", "position approved!",false);
							if ( player->characterItems[ pos ].id != 0 )
                            {
                                CLogger::writeMessage("Use","at position " + CLogger::toString(static_cast<int>(pos) ) + " on body, is an item with id: " + CLogger::toString(player->characterItems[ pos ].id),false);
								if ( first )
                                {

									LuaScript = CommonItems->findScript( player->characterItems[ pos ].id ) ;
									if ( LuaScript )
                                    {
										Source.Type = LUA_ITEM;
										Source.item = (ScriptItem)player->characterItems[ pos ];
										Source.item.pos = player->pos;
										if ( pos < MAX_BODY_ITEMS )
											Source.item.type = ScriptItem::it_inventory;
										else
											Source.item.type = ScriptItem::it_belt;
										Source.item.itempos = pos;
										Source.item.owner = player;
										Source.pos = player->pos;

										first = false;
									}
								}
                                else if ( LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript )
                                {
									Target.Type = LUA_ITEM;
									Target.item = (ScriptItem)player->characterItems[ pos ];
									Target.item.pos = player->pos;
									if ( pos < MAX_BODY_ITEMS )
										Target.item.type = ScriptItem::it_inventory;
									else
										Target.item.type = ScriptItem::it_belt;
									Target.item.itempos = pos;
									Target.item.owner = player;
									Target.pos = player->pos;
									first = false;
								}
							}
                            else
                            {
								paramOK = false;
							}
						}
                        else
                        {
							paramOK = false;
						}

						break;

					case UID_VAR:
						paramtemp = bs.popparam();
                        CLogger::writeMessage("Use","received menu event " + CLogger::toString(static_cast<int>(paramtemp) ),false);
						break;

					default:

						paramOK = false;
						break;
				}
			}

            if ( !bs.ok )
            {
                std::cout<<"error while popping for use events!"<<std::endl;
                paramOK = false;
            }

            CLogger::writeMessage("Use", "received counter parameter: " + CLogger::toString(static_cast<int>(counter) ),false) ;

            CLogger::writeMessage("Use_Scripts",""); //Insert time only
            CLogger::writeMessage("Use_Scripts","=========Use Script Start=============",false);
            CLogger::writeMessage("Use_Scripts","Source pos (" + CLogger::toString(Source.pos.x) + "," + CLogger::toString(Source.pos.y) + "," + CLogger::toString(Source.pos.z) + ")",false);
            CLogger::writeMessage("Use_Scripts","Source type: " + CLogger::toString(Source.Type), false);
            CLogger::writeMessage("Use_Scripts", "Source Character: " + CLogger::toString(Source.Character),false);
            CLogger::writeMessage("Use_Scripts","",false); //Insert blank line
            CLogger::writeMessage("Use_Scripts","Target pos (" + CLogger::toString(Target.pos.x) + "," + CLogger::toString(Target.pos.y) + "," + CLogger::toString(Target.pos.z) + ")",false);
            CLogger::writeMessage("Use_Scripts","Target Type: " + CLogger::toString(Target.Type),false);
            CLogger::writeMessage("Use_Scripts","Target Character: " + CLogger::toString(Target.Character),false);
            CLogger::writeMessage("Use_Scripts","==========Use Script End=============",false);
            CLogger::writeMessage("Use_Scripts","",false); //Insert blank line

            std::string msg;
            if ( Source.Type == LUA_ITEM && Source.item.quality < 100)
            {
                 if ( player->getPlayerLanguage() == 0)player->inform("Du kannst keine unfertigen Gegenstände benutzen!");
                 else player->inform("You can't use unfinished items!");
            }
			else if ( LuaScript )
            {
                player->ltAction->setLastAction( LuaScript, Source, Target, counter , paramtemp, CLongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS
				if ( ( paramOK ) && player->IsAlive() && player->IsConscious() )
#else
				if ( ( paramOK ) && player->IsAlive() )
#endif
                {

					if ( Source.Type == LUA_ITEM && ( Target.Type == LUA_ITEM || Target.Type == LUA_NONE ) )
                    {
                        LuaScript->UseItem( player, Source.item, Target.item, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp), static_cast<unsigned char>(LTS_NOLTACTION) );
                        msg = "Used Item: " + CLogger::toString(Source.item.id) + " with item: " + CLogger::toString(Target.item.id);
                    }
					else if ( Source.Type == LUA_ITEM && Target.Type == LUA_CHARACTER )
                    {
						// msg set first since character might be deleted!  --vilarion
						msg = "Used Item: " + CLogger::toString(Source.item.id) + " with character: " + Target.Character->name + "(" +  CLogger::toString(Target.Character->id) + ")";
						LuaScript->UseItemWithCharacter( player, Source.item, Target.Character, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp), static_cast<unsigned char>(LTS_NOLTACTION) );
                    }
					else if ( Source.Type == LUA_ITEM && Target.Type == LUA_FIELD )
                    {
						LuaScript->UseItemWithField( player, Source.item, Target.pos, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp), static_cast<unsigned char>(LTS_NOLTACTION) );
                        msg = "Used Item: " + CLogger::toString(Source.item.id) + " with empty field: Pos(" + CLogger::toString(Target.pos.x) + "," + CLogger::toString(Target.pos.y) + "," + CLogger::toString(Target.pos.z) + ")";
                    }
                }
			}
            else if ( LuaNPCScript )
            {
                player->ltAction->setLastAction( LuaNPCScript, Source, Target, counter , paramtemp, CLongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS
				if ( ( paramOK ) && player->IsAlive() && cp->IsConscious() )
#else
				if ( ( paramOK ) && player->IsAlive() )
#endif
                {
					if ( Source.Type == LUA_CHARACTER && ( Target.Type == LUA_NONE ) )
                    {
						LuaNPCScript->useNPC(player, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                        msg = "Used NPC: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ")";
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_FIELD )
                    {
						LuaNPCScript->useNPCWithField(player, Target.pos, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
                        msg = "Used NPC: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with empty field at pos(" + CLogger::toString(Target.pos.x) + "," + CLogger::toString(Target.pos.y) + "," + CLogger::toString(Target.pos.z) + ")";
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_ITEM )
                    {
						LuaNPCScript->useNPCWithItem(player, Target.item, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
                        msg = "Used NPC: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with Item: " + CLogger::toString(Target.item.id);
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_CHARACTER )
                    {
						LuaNPCScript->useNPCWithCharacter(player, Target.Character, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
                        msg = "Used NPC: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with character: " + Target.Character->name + "(" + CLogger::toString(Target.Character->id) + ")";
                    }
                }

			}
            else if ( LuaMonsterScript )
            {
                player->ltAction->setLastAction( LuaMonsterScript, Source, Target, counter , paramtemp, CLongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS
				if ( ( paramOK ) && player->IsAlive() && player->IsConscious() )
#else
                if ( ( paramOK ) && player->IsAlive() )
#endif
				{
					if ( Source.Type == LUA_CHARACTER && ( Target.Type == LUA_NONE ) )
                    {
						LuaMonsterScript->useMonster(Source.Character,player,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                        msg = "Used Monster: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ")";
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_FIELD )
                    {
						LuaMonsterScript->useMonsterWithField(Source.Character,player,Target.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                        msg = "Used Monster: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with empty field at pos(" + CLogger::toString(Target.pos.x) + "," + CLogger::toString(Target.pos.y) + "," + CLogger::toString(Target.pos.z) + ")";
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_ITEM )
                    {
						LuaMonsterScript->useMonsterWithItem(Source.Character,player,Target.item,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                         msg = "Used Monster: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with Item: " + CLogger::toString(Target.item.id);
                    }
					else if ( Source.Type == LUA_CHARACTER && Target.Type == LUA_CHARACTER )
                    {
						LuaMonsterScript->useMonsterWithCharacter(Source.Character,player,Target.Character,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                        msg = "Used Monster: " + Source.Character->name + "(" + CLogger::toString(Source.Character->id) + ") with character: " + Target.Character->name + "(" + CLogger::toString(Target.Character->id) + ")";
                    }
				}
			}
            else if ( LuaTileScript )
            {
                player->ltAction->setLastAction( LuaTileScript, Source, Target, counter , paramtemp, CLongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS
                if ( ( paramOK ) && player->IsAlive() && player->IsConscious() )
#else
                if ( ( paramOK ) && player->IsAlive() )
#endif
				{
				    if ( Source.Type == LUA_FIELD && Target.Type == LUA_NONE )
					    LuaTileScript->useTile(player,Source.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
				    else if ( Source.Type == LUA_FIELD && Target.Type == LUA_FIELD )
					    LuaTileScript->useTileWithField(player,Source.pos,Target.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
				    else if ( Source.Type == LUA_FIELD && Target.Type == LUA_ITEM )
					    LuaTileScript->useTileWithItem(player,Source.pos,Target.item, counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
				    else if ( Source.Type == LUA_FIELD && Target.Type == LUA_CHARACTER )
					    LuaTileScript->useTileWithCharacter(player,Source.pos,Target.Character, counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION) );
				}
			}
            boost::shared_ptr<CBasicServerCommand>cmd( new CBBSendActionTC(player->id, player->name, 3,msg) );
            CWorld::get()->monitoringClientList->sendCommand(cmd);

        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CUseTS() );
            return cmd;
        }

        unsigned char counter;
        ByteStack bs;
};

/**
*@ingroup Clientcommands
*character looks at an menu item
*/
class CLookAtMenuItemTS : public CBasicClientCommand
{
    public:
        CLookAtMenuItemTS() : CBasicClientCommand( CC_LOOKATMENUITEM_TS )
        {
        }

        virtual ~CLookAtMenuItemTS(){};

        void decodeData()
        {
            pos = getUnsignedCharFromBuffer();
            id = getShortIntFromBuffer();
        }

          void performAction( CPlayer * player )
        {
            CLogger::writeMessage("CWorld_Debug",player->name + "looks at an item from a menu.");
            time( &(player->lastaction) );
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
            {
                CWorld::get()->lookAtMenueItem( player, pos, id);
                player->actionPoints -= P_LOOK_COST;
            }
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookAtMenuItemTS() );
			return cmd;
        }
        TYPE_OF_ITEM_ID id;
        unsigned char pos;
};

/**
*@ingroup Clientcommands
*client gives signal that he is ok
*/
class CKeepAliveTS : public CBasicClientCommand
{
    public:
        CKeepAliveTS() : CBasicClientCommand( CC_KEEPALIVE_TS )
        {
        }

        virtual ~CKeepAliveTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
             CLogger::writeMessage("CWorld_Debug", "KEEPALIVE_TS von Spieler " + player->name );
			// Release Jailed Players if sentance has expired
			if ( player->GetStatus() == JAILEDFORTIME )
            {
				time_t tempkeepalive;
				time( &tempkeepalive );
				int temptime = tempkeepalive - player->lastkeepalive;
				if ( ( player->GetStatusTime() - temptime ) <= 0 )
                {
					player->SetStatus( 0 );
					player->SetStatusTime( 0 );

					position warpto;
                    std::stringstream ssx(configOptions["release_x"]);
                    ssx >> warpto.x;
                    std::stringstream ssy(configOptions["release_y"]);
                    ssy >> warpto.y;
                    std::stringstream ssz(configOptions["release_z"]);
                    ssz >> warpto.z;

                    player->Warp( warpto );
					//CWorld::get()->warpPlayer( player, warpto );
				}
                else
                {
					player->SetStatusTime( player->GetStatusTime() - temptime );
				}
			}

			time( &(player->lastkeepalive) );
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CKeepAliveTS() );
            return cmd;
        }
};

/**
*@ingroup Clientcommands
*client asked to send his skills
*/
class CRequestSkillsTS : public CBasicClientCommand
{
    public:
        CRequestSkillsTS() : CBasicClientCommand( CC_REQUESTSKILLS_TS )
        {
        }

        virtual ~CRequestSkillsTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->sendAllSkills();
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd(new CRequestSkillsTS());
            return cmd;
        }
};

/**
*@ingroup Clientcommands
*char stopps an attack
*/
class CAttackStopTS : public CBasicClientCommand
{
    public:
        CAttackStopTS() : CBasicClientCommand( CC_ATTACKSTOP_TS )
        {
        }

        virtual ~CAttackStopTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            player->attackmode = false;
            //hast to be changed to a command
            boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC() );
            player->Connection->addCommand(cmd);
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CAttackStopTS() );
            return cmd;
        }
};

/**
*@ingroup Clientcommands
*char looks at an item in his inventory
*/
class CLookAtInventoryItemTS : public CBasicClientCommand
{
    public:
        CLookAtInventoryItemTS() : CBasicClientCommand( CC_LOOKATINVENTORYITEM_TS )
        {
        }

        ~CLookAtInventoryItemTS(){};

        void decodeData()
        {
            pos = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            CLogger::writeMessage("CWorld_Debug",player->name + " looks at an item in the inventory." );
            time( &(player->lastaction) );
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->lookAtInventoryItem( player, pos );
				player->actionPoints -= P_LOOK_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookAtInventoryItemTS() );
            return cmd;
        }
        unsigned char pos;
};

/**
*@ingroup Clientcommands
*char looks at an item in his showcase
*/
class CLookAtShowCaseItemTS : public CBasicClientCommand
{
    public:
        CLookAtShowCaseItemTS() : CBasicClientCommand( CC_LOOKATSHOWCASEITEM_TS )
        {
        }

        virtual ~CLookAtShowCaseItemTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            CLogger::writeMessage("CWorld_Debug", player->name + " looks at an item in a container.");
            time( &(player->lastaction) );
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->lookAtShowcaseItem( player, showcase, pos );
				player->actionPoints -= P_LOOK_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookAtShowCaseItemTS() );
			return cmd;
        }
        unsigned char showcase;
        unsigned char pos;
};

/**
*@ingroup Clientcommands
*char moves item from inventory to a showcase
*/
class CMoveItemFromPlayerToShowCaseTS : public CBasicClientCommand
{
    public:
        CMoveItemFromPlayerToShowCaseTS() : CBasicClientCommand( CC_MOVEITEMFROMPLAYERTOSHOWCASE_TS )
        {
        }

        virtual ~CMoveItemFromPlayerToShowCaseTS(){};

        void decodeData()
        {
            cpos = getUnsignedCharFromBuffer();
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + "moves an item from the inventory to showcase!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemFromPlayerIntoShowcase( player, cpos, showcase, pos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CMoveItemFromPlayerToShowCaseTS() );
            return cmd;
        }
        unsigned char showcase;
        unsigned char pos;
        unsigned char cpos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from showcase to his inventory
*/
class CMoveItemFromShowCaseToPlayerTS : public CBasicClientCommand
{
    public:
        CMoveItemFromShowCaseToPlayerTS() : CBasicClientCommand( CC_MOVEITEMFROMSHOWCASETOPLAYER_TS )
        {
        }

        virtual ~CMoveItemFromShowCaseToPlayerTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            cpos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " moves an item from the shocase to the inventory!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemFromShowcaseToPlayer( player, showcase, pos, cpos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CMoveItemFromShowCaseToPlayerTS() );
            return cmd;
        }
        unsigned char showcase;
        unsigned char pos;
        unsigned char cpos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves items inside his inventory
*/
class CMoveItemInsideInventoryTS : public CBasicClientCommand
{
    public:
        CMoveItemInsideInventoryTS() : CBasicClientCommand( CC_MOVEITEMINSIDEINVENTORY_TS )
        {
        }

        virtual ~CMoveItemInsideInventoryTS(){};

        void decodeData()
        {
            opos = getUnsignedCharFromBuffer();
            npos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + "moves an item inside the inventory!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemBetweenBodyParts( player, opos, npos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CMoveItemInsideInventoryTS() );
            return cmd;
        }
        unsigned char opos;
        unsigned char npos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char droppes a item from his inventory to the map
*/
class CDropItemFromInventoryOnMapTS : public CBasicClientCommand
{
    public:
        CDropItemFromInventoryOnMapTS() : CBasicClientCommand( CC_DROPITEMFROMPLAYERONMAP_TS )
        {
        }

        virtual ~CDropItemFromInventoryOnMapTS(){};

        void decodeData()
        {
            pos = getUnsignedCharFromBuffer();
            xc = static_cast<short int>( getShortIntFromBuffer() );
            yc = static_cast<short int>( getShortIntFromBuffer() );
            zc = static_cast<short int>( getShortIntFromBuffer() );
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " throws an item from inventory on the map!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsConscious() )
#endif
			{
				CWorld::get()->dropItemFromPlayerOnMap( player, pos, xc, yc, zc, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CDropItemFromInventoryOnMapTS() );
            return cmd;
        }
        unsigned char pos;
        short int xc,yc,zc;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from map to his inventory
*/
class CMoveItemFromMapToPlayerTS : public CBasicClientCommand
{
    public:
        CMoveItemFromMapToPlayerTS() : CBasicClientCommand( CC_MOVEITEMFROMMAPTOPLAYER_TS )
        {
        }

        virtual ~CMoveItemFromMapToPlayerTS(){};

        void decodeData()
        {
            dir = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " moves an Item from the map to the inventory!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemFromMapToPlayer( player, dir, pos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CMoveItemFromMapToPlayerTS() );
            return cmd;
        }
        unsigned char dir;
        unsigned char pos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from map into a open showcase
*/
class CMoveItemFromMapIntoShowCaseTS : public CBasicClientCommand
{
    public:
        CMoveItemFromMapIntoShowCaseTS() : CBasicClientCommand( CC_MOVEITEMFROMMAPINTOSHOWCASE_TS )
        {
        }

        virtual ~CMoveItemFromMapIntoShowCaseTS(){};

        void decodeData()
        {
            dir = getUnsignedCharFromBuffer();
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " moves an item from the map to the showcase!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemFromMapIntoShowcase( player, dir, showcase, pos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd(new CMoveItemFromMapIntoShowCaseTS());
            return cmd;
        }
        unsigned char dir;
        unsigned char showcase;
        unsigned char pos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item between two showcases
*/
class CMoveItemBetweenShowCasesTS : public CBasicClientCommand
{
    public:
        CMoveItemBetweenShowCasesTS() : CBasicClientCommand( CC_MOVEITEMBETWEENSHOWCASES_TS )
        {
        }

        virtual ~CMoveItemBetweenShowCasesTS(){};

        void decodeData()
        {
            source = getUnsignedCharFromBuffer();
            spos = getUnsignedCharFromBuffer();
            dest = getUnsignedCharFromBuffer();
            dpos = getUnsignedCharFromBuffer();
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug",player->name + " moves an item between showcases!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItemBetweenShowcases( player, source, spos, dest, dpos, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CMoveItemBetweenShowCasesTS() );
            return cmd;
        }
        unsigned char source;
        unsigned char spos;
        unsigned char dest;
        unsigned char dpos;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char droppes a item from a showcase to the map
*/
class CDropItemFromShowCaseOnMapTS : public CBasicClientCommand
{
    public:
        CDropItemFromShowCaseOnMapTS() : CBasicClientCommand( CC_DROPITEMFROMSHOWCASEONMAP_TS )
        {
        }

        virtual ~CDropItemFromShowCaseOnMapTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            xc = static_cast<short int>( getShortIntFromBuffer() );
            yc = static_cast<short int>( getShortIntFromBuffer() );
            zc = static_cast<short int>( getShortIntFromBuffer() );
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug",player->name + " moves an item from showcase to the map!" );
#ifdef DO_UNCONSCIOUS
			if ( player->IsConscious() )
#endif
			{
				CWorld::get()->dropItemFromShowcaseOnMap( player, showcase, pos, xc, yc, zc, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CDropItemFromShowCaseOnMapTS() );
            return cmd;
        }
        unsigned char showcase;
        unsigned char pos;
        short int xc,yc,zc;
        unsigned char count;
};

/**
*@ingroup Clientcommands
*char closes a showcase container
*/
class CCloseContainerInShowCaseTS : public CBasicClientCommand
{
    public:
        CCloseContainerInShowCaseTS() : CBasicClientCommand( CC_CLOSECONTAINERINSHOWCASE_TS )
        {
        }

        virtual ~CCloseContainerInShowCaseTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
            //getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " closes a container in the showcase");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
                CWorld::get()->closeContainerInShowcase( player, showcase );
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CCloseContainerInShowCaseTS() );
            return cmd;
        }
        unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char looks into a showcase container
*/
class CLookIntoShowCaseContainerTS : public CBasicClientCommand
{
    public:
        CLookIntoShowCaseContainerTS() : CBasicClientCommand( CC_LOOKINTOSHOWCASECONTAINER_TS )
        {
        }

        virtual ~CLookIntoShowCaseContainerTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " looks into a container in a showcase!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsConscious() )
#endif
			{
				CWorld::get()->lookIntoShowcaseContainer( player, showcase, pos );
				player->actionPoints -= P_LOOK_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookIntoShowCaseContainerTS() );
            return cmd;
        }
        unsigned char showcase;
        unsigned char pos;
};

/**
*@ingroup Clientcommands
*char looks into the inventory
*/
class CLookIntoInventoryTS : public CBasicClientCommand
{
    public:
        CLookIntoInventoryTS() : CBasicClientCommand( CC_LOOKINTOINVENTORY_TS )
        {
        }

        virtual ~CLookIntoInventoryTS(){};

        void decodeData()
        {
            showcase = getUnsignedCharFromBuffer();
        }

          void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " looks into his backpack");
#ifdef DO_UNCONSCIOUS
			if ( player->IsConscious() )
#endif
			{
				CWorld::get()->lookIntoBackPack( player, showcase );
				player->actionPoints -= P_LOOK_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookIntoInventoryTS() );
            return cmd;
        }
        unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char looks into a container on a map field
*/
class CLookIntoContainerOnFieldTS : public CBasicClientCommand
{
    public:
        CLookIntoContainerOnFieldTS() : CBasicClientCommand( CC_LOOKINTOCONTAINERONFIELD_TS )
        {
        }

        virtual ~CLookIntoContainerOnFieldTS(){};

        void decodeData()
        {
            direction = getUnsignedCharFromBuffer();
            showcase = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " looks into a container on the map");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
 				CWorld::get()->lookIntoContainerOnField( player, direction, showcase );
				player->actionPoints -= P_LOOK_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookIntoContainerOnFieldTS() );
			return cmd;
        }
        unsigned char direction;
        unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char logged out
*/
class CLogOutTS : public CBasicClientCommand
{
    public:
        /**
        *a client logged out
        */
        CLogOutTS() : CBasicClientCommand( CC_LOGOUT_TS )
        {
        }

        virtual ~CLogOutTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " logt aus" );
			player->Connection->closeConnection();
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLogOutTS() );
			return cmd;
        }
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class CWhisperTS : public CBasicClientCommand
{
    public:
        /**
        *a player talked
        */
        CWhisperTS() : CBasicClientCommand( CC_WHISPER_TS )
        {
        }

        virtual ~CWhisperTS(){};

        void decodeData()
        {
            text = getStringFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            CLogger::writeMessage("CWorld_Debug", player->name + " whispers something!");
			player->talk(CCharacter::tt_whisper, text);
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CWhisperTS() );
			return cmd;
        }
        std::string text;
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class CShoutTS : public CBasicClientCommand
{
    public:
        /**
        *a player talked
        */
        CShoutTS() : CBasicClientCommand( CC_SHOUT_TS )
        {
        }

        virtual ~CShoutTS(){};

        void decodeData()
        {
            text = getStringFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->talk(CCharacter::tt_yell, text);
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CShoutTS() );
			return cmd;
        }
        std::string text;
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class CSayTS : public CBasicClientCommand
{
    public:
        /**
        *a player talked
        */
        CSayTS() : CBasicClientCommand( CC_SAY_TS )
        {
        }

        virtual ~CSayTS(){};

        void decodeData()
        {
            text = getStringFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            CLogger::writeMessage("CWorld_Debug", player->name + " whispers something!");
			if ( ( !player->hasGMRight(gmr_isnotshownasgm) && !player->isAdmin() ) || !CWorld::get()->parseGMCommands(player, text))
            {
				if ( !CWorld::get()->parsePlayerCommands( player, text ) )  // did we issue a player command?
					player->talk(CCharacter::tt_say, text);
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CSayTS() );
			return cmd;
        }
        std::string text;
};

/**
*@ingroup Clientcommands
*char wants a clientupdate for the map
*/
class CRefreshTS : public CBasicClientCommand
{
    public:
        /**
        *a clientupdate should be sendet to the client
        */
        CRefreshTS() : CBasicClientCommand( CC_REFRESH_TS )
        {
        }

        virtual ~CRefreshTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            CLogger::writeMessage("CWorld_Debug", player->name + " want sended a refresh_ts, sending map!");
            //update the current mapview
            player->sendFullMap();
			CWorld::get()->sendAllVisibleCharactersToPlayer( player, true );
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd(new CRefreshTS() );
            return cmd;
        }
};

/**
*@ingroup Clientcommands
*char introduces to another one
*/
class CIntroduceTS : public CBasicClientCommand
{
    public:
        /**
        *a player attacked another char
        */
        CIntroduceTS() : CBasicClientCommand( CC_INTRODUCE_TS )
        {
        }

        virtual ~CIntroduceTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            CLogger::writeMessage("CWorld_Debug",player->name + " introduces himself!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->introduceMyself( player );
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CIntroduceTS() );
            return cmd;
        }
};

/**
*@ingroup Clientcommands
*char attacks another char
*/
class CAttackPlayerTS : public CBasicClientCommand
{
    public:
        /**
        *a player attacked another char
        */
        CAttackPlayerTS() : CBasicClientCommand( CC_ATTACKPLAYER_TS )
        {

        }

        virtual ~CAttackPlayerTS(){};

        void decodeData()
        {
            enemyid = getIntFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
#ifdef DO_UNCONSCIOUS
		    if (player->IsAlive() && player->GetStatus() < 10 && player->IsConscious() )
#else
		    if ( player->IsAlive() && player->GetStatus() < 10 )
#endif
            {
                player->ltAction->abortAction();
                if ( player->IsAlive() )
                {
                    player->enemyid = enemyid;
                    player->attackmode = true;
                    player->enemytype = CCharacter::player;
                    if (player->enemyid >= MONSTER_BASE)player->enemytype = CCharacter::monster;
                    if (player->enemyid >= NPC_BASE)player->enemytype = CCharacter::npc;
                    boost::shared_ptr<CBasicServerCommand>cmd( new CAttackAckknowledgedTC() );
                    player->Connection->addCommand(cmd);
                    player->startMusic(FIGHTMUSIC);
                    //monitoringClientList->sendCommand( new CSendActionTS(player->id, player->name, 0, "Starts an attack: " + CLogger::toString(player->enemyid) ) );
                    CWorld::get()->characterAttacks( player );
                }
                else
                {
                    player->attackmode = false;
                }
            }
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CAttackPlayerTS() );
            return cmd;
        }
        uint32_t enemyid;

};

/**
*@ingroup Clientcommands
*char looks at a map item
*/
class CLookAtMapItemTS : public CBasicClientCommand
{
    public:
        /**
        *a spin occured special handling to get direction which is encoded in the definition byte
        */
        CLookAtMapItemTS() : CBasicClientCommand( CC_LOOKATMAPITEM_TS )
        {
        }

        virtual ~CLookAtMapItemTS(){};

        void decodeData()
        {
            x = getShortIntFromBuffer();
            y = getShortIntFromBuffer();
            z = getShortIntFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
             CLogger::writeMessage("CWorld_Debug",player->name + " looks at a map item.");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->lookAtMapItem( player, x, y, z );
				player->actionPoints -= P_LOOK_COST;
			}

        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLookAtMapItemTS() );
            return cmd;
        }
        short int x,y,z;

};

/**
*@ingroup Clientcommands
*char starts to spin around
*/
class CPSpinActionTS : public CBasicClientCommand
{
    public:
        /**
        *a spin occured special handling to get direction which is encoded in the definition byte
        */
        CPSpinActionTS(uint8_t dir) : CBasicClientCommand( CC_PSPINRSTART_TS )
        {
            direction = dir;
        }

        virtual ~CPSpinActionTS(){};

        void decodeData()
        {
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", "Player changes his dircetion: " + player->name + " temp: " + CLogger::toString(direction) );

#ifdef DO_UNCONSCIOUS
			if ( player->IsConscious() )
#endif
			{
				if ( CWorld::get()->spinPlayer( player, direction ) )
                {
					player->actionPoints -= P_SPIN_COST;
				}
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CPSpinActionTS(direction) );
            return cmd;
        }
        uint8_t direction;
};


/**
*@ingroup Clientcommands
*a char move has occured
*/
class CCharMoveTS : public CBasicClientCommand
{
    public:
        /**
        *a character has started moving
        */
        CCharMoveTS() : CBasicClientCommand( CC_CHARMOVE_TS )
        {
        }

        virtual ~CCharMoveTS(){};

        void decodeData()
        {
	    charid = getIntFromBuffer();
	    direction = getUnsignedCharFromBuffer();
	    mode = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            if ( charid == player->id && ( mode == NORMALMOVE || mode == RUNNING ) )
	    {
	        player->ltAction->abortAction();
                CLogger::writeMessage("CWorld_Debug", "Playermove from Player: " + player->name );

                if ( player->getTurtleActive() && player->hasGMRight(gmr_settiles) && mode == NORMALMOVE )
                {
                    CWorld::get()->setNextTile( player, player->getTurtleTile() );
                    CLogger::writeMessage("Player_Moves","Turtle was active, new tile set at pos: " + CLogger::toString(player->pos.x) + "," + CLogger::toString(player->pos.y) + "," + CLogger::toString(player->pos.z) + " tile: " + CLogger::toString(player->getTurtleTile()));
                }

                if ( player->move( static_cast<CCharacter::direction>(direction), mode ) )player->closeAllShowcasesOfMapContainers();
            }
	    else if ( mode == PUSH )
	    {
                player->ltAction->abortAction();
                CLogger::writeMessage("CWorld_Debug", "Player pushes another: " + player->name);
                if ( player->IsAlive() )
                {
                    if ( CWorld::get()->pushCharacter( player, charid, direction ) )
                    {
                        player->actionPoints -= P_PUSH_COST;
                    }
                }
            }
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CCharMoveTS() );
			return cmd;
        }

        TYPE_OF_CHARACTER_ID charid;
        unsigned char direction;
	unsigned char mode;
};


/**
*@ingroup Clientcommands
*char starts to move a item
*/
class CIMoverActionTS : public CBasicClientCommand
{
    public:
        /**
        *a item move occured special handling to get direction which is encoded in the definition byte
        */
        CIMoverActionTS(uint8_t dir) : CBasicClientCommand( CC_IMOVERSTART_TS )
        {
            direction = dir;
        }

        virtual ~CIMoverActionTS(){};

        void decodeData()
        {
            xc = static_cast<char>( getShortIntFromBuffer() );
            yc = static_cast<char>( getShortIntFromBuffer() );
            zc = static_cast<char>( getShortIntFromBuffer() );
            count = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
            player->ltAction->abortAction();
            CLogger::writeMessage("CWorld_Debug", player->name + " tryes to move an Item!");
#ifdef DO_UNCONSCIOUS
			if ( player->IsAlive() && player->IsConscious() )
#else
			if ( player->IsAlive() )
#endif
			{
				CWorld::get()->moveItem( player, direction, xc, yc, zc, count );
				player->actionPoints -= P_ITEMMOVE_COST;
			}
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CIMoverActionTS(direction) );
			return cmd;
        }
        short int xc, yc, zc;
        uint8_t count;
        uint8_t direction;
};


//=========================================Login Command=========================
/**
*@ingroup Clientcommands
*a player tries to login
*/
class CLoginCommandTS : public CBasicClientCommand
{
    public:
        CLoginCommandTS() : CBasicClientCommand( CC_LOGIN_TS )
        {
        }

        virtual ~CLoginCommandTS(){};

        void decodeData()
        {
            clientVersion = getUnsignedCharFromBuffer();
            loginName = getStringFromBuffer();
            passwort = getStringFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            time( &(player->lastaction) );
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CLoginCommandTS() );
			return cmd;
        }

        unsigned short clientVersion;
        std::string loginName;
        std::string passwort;
};
//==============================End Login Command================================

//=========================================Screen Size Command=========================
/**
*@ingroup Clientcommands
*a player tries to login
*/
class CScreenSizeCommandTS : public CBasicClientCommand
{
    public:
        CScreenSizeCommandTS() : CBasicClientCommand( CC_SCREENSIZE_TS )
        {
        }

        virtual ~CScreenSizeCommandTS(){};

        void decodeData()
        {
            width = getUnsignedCharFromBuffer();
            height = getUnsignedCharFromBuffer();
        }

        void performAction( CPlayer * player )
        {
            // Set screen width and screen height
            player->screenwidth = width;
            player->screenheight = height;
            player->sendFullMap();
        }

        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CScreenSizeCommandTS() );
			return cmd;
        }

        uint8_t width;
        uint8_t height;
};
//==============================End Screen Size Command================================

#endif
