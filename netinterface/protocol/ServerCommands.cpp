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


#include "globals.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "ContainerObjectTable.hpp"
#include "Logger.hpp"
#include "RaceSizeTable.hpp"
#include "ServerCommands.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"


//! eine Tabelle fuer Behaelter - Item Daten
extern CContainerObjectTable* ContainerItems;
extern CRaceSizeTable* RaceSizes;

        CItemUpdate_TC::CItemUpdate_TC( position fieldpos, ITEMVECTOR & items ) : CBasicServerCommand( SC_ITEMUPDATE_TC )
        {
            CLogger::writeMessage("rot_update", "sending new itemstack for pos("+CLogger::toString(fieldpos.x)+", "+CLogger::toString(fieldpos.y)+", "+CLogger::toString(fieldpos.z)+")",false);
            addShortIntToBuffer( fieldpos.x );
            addShortIntToBuffer( fieldpos.y );
            addShortIntToBuffer( fieldpos.z );
            int16_t size = static_cast<unsigned char>(items.size());
            if ( size > 255 ) size = 255;
            addUnsignedCharToBuffer( static_cast<uint8_t>(size) );
            ITEMVECTOR::iterator it;
            for ( it = items.begin(); it != items.end(); ++it)
            {
                //we added 255 items
                if (size <= 0) break;
                addShortIntToBuffer( it->id );
                addUnsignedCharToBuffer( it->number );
                CLogger::writeMessage("rot_update", "adding item id: "+CLogger::toString(it->id)+" count: "+CLogger::toString(static_cast<int>(it->number)),false);
                size--;
            }
        }
        
        CItemUpdate_TC::CItemUpdate_TC( int16_t px, int16_t py, int16_t pz, ITEMVECTOR &items ) : CBasicServerCommand( SC_ITEMUPDATE_TC ) 
        {
            addShortIntToBuffer( px );
            addShortIntToBuffer( py );
            addShortIntToBuffer( pz );
            int16_t size = static_cast<unsigned char>(items.size());
            if ( size > 255 ) size = 255;
            addUnsignedCharToBuffer(static_cast<uint8_t>(size) );
            ITEMVECTOR::iterator it;
            for ( it = items.begin(); it != items.end(); ++it)
            {
                //we added 255 items
                if (size <= 0) break;
                addShortIntToBuffer( it->id );
                addUnsignedCharToBuffer( it->number );
                size--;
            }
        }
        
        CCharDescription::CCharDescription(TYPE_OF_CHARACTER_ID id, std::string description) : CBasicServerCommand( SC_LOOKATCHARRESULT_TC ) 
        {
            addIntToBuffer(id);
            addStringToBuffer( description );
        }
        
        CAppearanceTC::CAppearanceTC( CCharacter* cc ) : CBasicServerCommand( SC_APPEARANCE_TC )
        {
            addIntToBuffer( cc->id );
            addShortIntToBuffer( cc->appearance );
            addUnsignedCharToBuffer( RaceSizes->getRelativeSize( cc->race, cc->battrib.body_height ) );
            addUnsignedCharToBuffer( static_cast<unsigned char>( 0 ) );
            addUnsignedCharToBuffer( cc->hair );
            addUnsignedCharToBuffer( cc->beard );
            addUnsignedCharToBuffer( cc->hairred );
            addUnsignedCharToBuffer( cc->hairgreen );
            addUnsignedCharToBuffer( cc->hairblue );
            addUnsignedCharToBuffer( cc->skinred );
            addUnsignedCharToBuffer( cc->skingreen );
            addUnsignedCharToBuffer( cc->skinblue );
            addShortIntToBuffer( cc->GetItemAt(1).id );
            addShortIntToBuffer( cc->GetItemAt(3).id );
            addShortIntToBuffer( cc->GetItemAt(11).id );
            addShortIntToBuffer( cc->GetItemAt(5).id );
            addShortIntToBuffer( cc->GetItemAt(6).id );
            addShortIntToBuffer( cc->GetItemAt(9).id );
            addShortIntToBuffer( cc->GetItemAt(10).id );
            addUnsignedCharToBuffer( cc->getWeaponMode() );
            uint8_t deathflag = cc->IsAlive() ? 0 : 1;
            addUnsignedCharToBuffer( deathflag );
        }

        CAnimationTC::CAnimationTC( TYPE_OF_CHARACTER_ID id, uint8_t animID ) : CBasicServerCommand( SC_ANIMATION_TC )
        {
            addIntToBuffer(id);
            addUnsignedCharToBuffer( animID );
        }

        CBookTC::CBookTC( uint16_t bookID ) : CBasicServerCommand( SC_BOOK_TC )
        {   
            addShortIntToBuffer( bookID );
        }
        
        CRemoveCharTC::CRemoveCharTC(TYPE_OF_CHARACTER_ID id) : CBasicServerCommand( SC_REMOVECHAR_TC ) 
        {
            addIntToBuffer(id);
        }
        
        CUpdateTimeTC::CUpdateTimeTC(unsigned char hour, unsigned char minute, unsigned char day, unsigned char month, short int year) : CBasicServerCommand( SC_UPDATETIME_TC ) 
        {
            addUnsignedCharToBuffer( hour );
            addUnsignedCharToBuffer( minute );
            addUnsignedCharToBuffer( day );
            addUnsignedCharToBuffer( month );
            addShortIntToBuffer( year );
        }
        
        CLogOutTC::CLogOutTC(unsigned char reason) : CBasicServerCommand( SC_LOGOUT_TC ) 
        {
            addUnsignedCharToBuffer( reason );
        }

        CTargetLostTC::CTargetLostTC() : CBasicServerCommand( SC_TARGETLOST_TC ) 
        {
        }

        CAttackAckknowledgedTC::CAttackAckknowledgedTC() : CBasicServerCommand( SC_ATTACKACKKNOWLEDGED_TC ) 
        {
        }

        CNameOfInventoryItemTC::CNameOfInventoryItemTC( unsigned char pos, std::string name ) : CBasicServerCommand( SC_NAMEOFINVENTORYITEM_TC ) 
        {
            addUnsignedCharToBuffer( pos );
            addStringToBuffer( name );
        }

        CNameOfShowCaseItemTC::CNameOfShowCaseItemTC(unsigned char showcase, unsigned char pos, std::string name ) : CBasicServerCommand( SC_NAMEOFSHOWCASEITEM_TC ) 
        {
            addUnsignedCharToBuffer( showcase );
            addUnsignedCharToBuffer( pos );
            addStringToBuffer( name );
        }

        CNameOfMapItemTC::CNameOfMapItemTC( short int x, short int y, short int z, std::string name ) : CBasicServerCommand( SC_NAMEOFMAPITEM_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addStringToBuffer( name );
        }

        CItemPutTC::CItemPutTC( short int x, short int y, short int z, Item &item ) : CBasicServerCommand( SC_ITEMPUT_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addShortIntToBuffer( item.id );
            if ( ContainerItems->find( item.id ) ) 
            {
                addUnsignedCharToBuffer( 1 );
            } 
            else 
            {
                addUnsignedCharToBuffer( item.number );
            }            
        }

        CItemSwapTC::CItemSwapTC(position pos, unsigned short int id, Item &item) : CBasicServerCommand( SC_MAPITEMSWAP ) 
        {
            addShortIntToBuffer( pos.x );
            addShortIntToBuffer( pos.y );
            addShortIntToBuffer( pos.z );
            addShortIntToBuffer( id ); /**no id is sended*/
            addShortIntToBuffer( item.id );
            //addUnsignedCharToBuffer( flags ); /**no flags are sended and needed*/
             if ( ContainerItems->find( item.id ) ) 
            {
                addUnsignedCharToBuffer( 1 );
            } 
            else 
            {
                addUnsignedCharToBuffer( item.number );
            }            
            //addUnsignedCharToBuffer( flags );
        }
        
        CItemSwapTC::CItemSwapTC(short int x, short int y, short int z, unsigned short int id, Item &item) : CBasicServerCommand( SC_MAPITEMSWAP ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addShortIntToBuffer( id );
            addShortIntToBuffer( item.id );
            //addUnsignedCharToBuffer( flags );
            if ( ContainerItems->find( item.id ) ) 
            {
                addUnsignedCharToBuffer( 1 );
            } 
            else 
            {
                addUnsignedCharToBuffer( item.number );
            }            
            //addUnsignedCharToBuffer( flags );
        }        

        CItemRemoveTC::CItemRemoveTC( short int x, short int y, short int z ) : CBasicServerCommand( SC_ITEMREMOVE_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
        }

        CAViewPlayersTC::CAViewPlayersTC() : CBasicServerCommand( SC_AVIEWPLAYERS_TC ) 
        {
            CWorld::PLAYERVECTOR::iterator titerator;
            unsigned short int count = CWorld::get()->Players.size();
            addShortIntToBuffer( count );
            for ( titerator = CWorld::get()->Players.begin(); titerator < CWorld::get()->Players.end(); ++titerator ) 
            {
                addStringToBuffer( ( *titerator )->name );
                addStringToBuffer( ( *titerator )->last_ip );
            }            
        }

        CSoundTC::CSoundTC(short int x, short int y, short int z, unsigned short int id) : CBasicServerCommand( SC_SOUND_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addShortIntToBuffer( id );
        }

        CGraphicEffectTC::CGraphicEffectTC(short int x, short int y, short int z, unsigned short int id) : CBasicServerCommand( SC_GRAPHICEFFECT_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addShortIntToBuffer( id );
        }

        CStartPlayerMenuTC::CStartPlayerMenuTC(UserMenuStruct menu) : CBasicServerCommand( SC_STARTPLAYERMENU_TC ) 
        {
            int count = menu.Items.size();
    
            addUnsignedCharToBuffer( count );
            std::list<TYPE_OF_ITEM_ID>::iterator it;
            for ( int i = 0; i < count; ++i ) 
            {
                   addShortIntToBuffer( menu.Items.front() );
                   menu.Items.pop_front();
            }
            menu.Items.clear();
        }

        CUpdateShowCaseTC::CUpdateShowCaseTC(unsigned char showcase, ITEMVECTOR &items) : CBasicServerCommand( SC_UPDATESHOWCASE_TC ) 
        {
            ITEMVECTOR::iterator theIterator;
            addUnsignedCharToBuffer( showcase );
        
            MAXCOUNTTYPE size = items.size();
            addUnsignedCharToBuffer( size );
            for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) 
            {
                addShortIntToBuffer( theIterator->id );
                if ( ContainerItems->find( theIterator->id ) ) 
                {
                    addUnsignedCharToBuffer( 1 );
                } 
                else 
                {
                    addUnsignedCharToBuffer( theIterator->number );
                }
            }
        }

        CMapStripeTC::CMapStripeTC( position pos, CNewClientView::stripedirection dir) : CBasicServerCommand( SC_MAPSTRIPE_TC ) 
        {
            addShortIntToBuffer( pos.x );
            addShortIntToBuffer( pos.y );
            addShortIntToBuffer( pos.z );
            addUnsignedCharToBuffer( static_cast<unsigned char>( dir ) );
            CField ** fields =  CWorld::get()->clientview.mapStripe;
            uint8_t numberOfTiles = CWorld::get()->clientview.getMaxTiles();
            addUnsignedCharToBuffer( numberOfTiles );
            for ( int i = 0; i < numberOfTiles; ++i )
            {
                if( fields[i] )
                {
                    addShortIntToBuffer( fields[i]->getTileCode() );
                    addShortIntToBuffer( fields[i]->getMusicId() );
                    addUnsignedCharToBuffer( static_cast<unsigned char>( fields[i]->items.size()) );
                    ITEMVECTOR::iterator theIterator;    
                    for ( theIterator = fields[i]->items.begin(); theIterator < fields[i]->items.end(); ++theIterator ) 
                    {
                        addShortIntToBuffer( theIterator->id );
                        if ( ContainerItems->find( theIterator->id ) ) 
                        {
                            addUnsignedCharToBuffer( 1 );
                        } 
                        else 
                        {
                            addUnsignedCharToBuffer( static_cast<unsigned char>( theIterator->number ) );
                        }
                    }
                }
                else
                {
                    addShortIntToBuffer( -1 );
                    addShortIntToBuffer( 0 );
                    addUnsignedCharToBuffer( 0 );
                }
            }
        }

        CMapCompleteTC::CMapCompleteTC() : CBasicServerCommand( SC_MAPCOMPLETE_TC ) 
        {
        }

        CMoveAckTC::CMoveAckTC( TYPE_OF_CHARACTER_ID id, position pos, unsigned char mode, unsigned char waitpages) : CBasicServerCommand( SC_MOVEACK_TC ) 
        {
            addIntToBuffer( id );
            addShortIntToBuffer( pos.x );
            addShortIntToBuffer( pos.y );
            addShortIntToBuffer( pos.z );
            addUnsignedCharToBuffer( mode );
            addUnsignedCharToBuffer( waitpages );
        }

        CIntroduceTC::CIntroduceTC(TYPE_OF_CHARACTER_ID id, std::string name) : CBasicServerCommand( SC_INTRODUCE_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( name ); 
        }

        CShoutTC::CShoutTC(int16_t x, int16_t y, int16_t z, std::string text) : CBasicServerCommand( SC_SHOUT_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addStringToBuffer( text );
        }

        CWhisperTC::CWhisperTC(int16_t x, int16_t y, int16_t z, std::string text) : CBasicServerCommand( SC_WHISPER_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addStringToBuffer( text );
        }

        CSayTC::CSayTC(int16_t x, int16_t y, int16_t z, std::string text) : CBasicServerCommand( SC_SAY_TC ) 
        {
            addShortIntToBuffer( x );
            addShortIntToBuffer( y );
            addShortIntToBuffer( z );
            addStringToBuffer( text );
        }

        CMusicTC::CMusicTC(short int title) : CBasicServerCommand( SC_MUSIC_TC ) 
        {
            addShortIntToBuffer( title );
        }

        CMusicDefaultTC::CMusicDefaultTC() : CBasicServerCommand( SC_MUSICDEFAULT_TC )
        {
        }

        CUpdateAttribTC::CUpdateAttribTC(std::string name, short int value) : CBasicServerCommand( SC_UPDATEATTRIB_TC ) 
        {
            addStringToBuffer( name );
            addShortIntToBuffer( value );
        }

        CUpdateMagicFlagsTC::CUpdateMagicFlagsTC(unsigned char type, uint32_t flags) : CBasicServerCommand( SC_UPDATEMAGICFLAGS_TC ) 
        {
            addUnsignedCharToBuffer( type );
            addIntToBuffer( flags );
        }

        CClearShowCaseTC::CClearShowCaseTC(unsigned char id) : CBasicServerCommand( SC_CLEARSHOWCASE_TC ) 
        {
            addUnsignedCharToBuffer( id ); 
        }

        CUpdateSkillTC::CUpdateSkillTC(std::string name, unsigned char type, unsigned short int major, unsigned short int minor) : CBasicServerCommand( SC_UPDATESKILL_TC ) 
        {
            addStringToBuffer( name );
            addUnsignedCharToBuffer( type );
            addShortIntToBuffer( major );
            addShortIntToBuffer( minor );
        }

        CUpdateWeatherTC::CUpdateWeatherTC( WeatherStruct weather ) : CBasicServerCommand( SC_UPDATEWEATHER_TC ) 
        {
            addUnsignedCharToBuffer( weather.cloud_density );
            addUnsignedCharToBuffer(weather.fog_density );
            addUnsignedCharToBuffer(weather.wind_dir);
            addUnsignedCharToBuffer(weather.gust_strength);
            addUnsignedCharToBuffer(weather.percipitation_strength);
            addUnsignedCharToBuffer(static_cast<unsigned char>(weather.per_type));
            addUnsignedCharToBuffer(weather.thunderstorm);
            addUnsignedCharToBuffer(weather.temperature);
        }
        
        CUpdateWeatherTC::CUpdateWeatherTC( uint8_t cd, uint8_t fd, uint8_t wd, uint8_t gs, uint8_t ps, uint8_t pt, uint8_t ts, uint8_t tp ) : CBasicServerCommand( SC_UPDATEWEATHER_TC ) 
        {
            addUnsignedCharToBuffer( cd );
            addUnsignedCharToBuffer( fd );
            addUnsignedCharToBuffer( wd );
            addUnsignedCharToBuffer( gs );
            addUnsignedCharToBuffer( ps );
            addUnsignedCharToBuffer( pt );
            addUnsignedCharToBuffer( ts );
            addUnsignedCharToBuffer( tp );
        }        

        CIdTC::CIdTC(int id) : CBasicServerCommand( SC_ID_TC ) 
        {
            addIntToBuffer(id);
        }

        CUpdateInventoryPosTC::CUpdateInventoryPosTC(unsigned char pos, TYPE_OF_ITEM_ID id, unsigned char number) : CBasicServerCommand( SC_UPDATEINVENTORYPOS_TC ) 
        {
            addUnsignedCharToBuffer( pos );
            addShortIntToBuffer( id );
            addUnsignedCharToBuffer( number ); 
        }

        CSetCoordinateTC::CSetCoordinateTC( position pos ) : CBasicServerCommand( SC_SETCOORDINATE_TC )
        {
            addShortIntToBuffer(pos.x);
            addShortIntToBuffer(pos.y);
            addShortIntToBuffer(pos.z);
        }
        
        CSetCoordinateTC::CSetCoordinateTC( short int x, short int y, short int z ) : CBasicServerCommand( SC_SETCOORDINATE_TC )
        {
            addShortIntToBuffer(x);
            addShortIntToBuffer(y);
            addShortIntToBuffer(z);
        }

        CPlayerSpinTC::CPlayerSpinTC( unsigned char faceto, TYPE_OF_CHARACTER_ID id ) : CBasicServerCommand( SC_PLAYERSPIN_TC )
        {
            addUnsignedCharToBuffer( faceto );
            addIntToBuffer( id );
        }

