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
#include "LongTimeCharacterEffects.hpp"
#include "MilTimer.hpp"
#include "Player.hpp"
#include "LongTimeEffect.hpp"
#include "LongTimeEffectTable.hpp"
#include "script/LuaLongTimeEffectScript.hpp"

extern CLongTimeEffectTable * LongTimeEffects;

CLongTimeCharacterEffects::CLongTimeCharacterEffects(CCharacter * owner) : _owner(owner)
{
    _effectTimer = new CMilTimer(100);
}

CLongTimeCharacterEffects::~CLongTimeCharacterEffects()
{
    delete _effectTimer;
    _effectTimer = NULL;
    effectList.clear();
}

bool CLongTimeCharacterEffects::find(uint16_t effectid, CLongTimeEffect * &effect)
{
    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        if ( (*it)->_effectId == effectid )
        {
            effect = (*it);
            return true;
        }
    }
    effect = NULL;
    return false;
}

bool CLongTimeCharacterEffects::find(std::string effectname, CLongTimeEffect * &effect)
{

    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        if ( (*it)->_effectName == effectname )
        {
            effect = (*it);
            return true;
        }
    }
    effect = NULL;
    return false;
}

void CLongTimeCharacterEffects::addEffect( CLongTimeEffect * effect)
{
    CLongTimeEffect * foundeffect;
    if ( !find(effect->_effectId, foundeffect ) )
    {   
        EFFECTLIST::iterator it = effectList.begin();
        bool inserted = false;
        while ( !inserted && it != effectList.end() )
        {
            if ( (*it)->_nextCalled >= effect->_nextCalled )
            {
                (*it)->_nextCalled -= effect->_nextCalled;
                effectList.insert( it, effect);
                inserted = true;
            }
            else
            {
                effect->_nextCalled -= (*it)->_nextCalled;
                it++;
            }
        }
        if ( !inserted )
        {
            effectList.push_back( effect );
        }
        LongTimeEffectStruct effectStr;
        if (effect->_firstadd && LongTimeEffects->find( effect->_effectId, effectStr) )
        {
            if ( effectStr.script )
            {
                effectStr.script->addEffect( effect , _owner);
            }
        }        
    }
    else
    {
        LongTimeEffectStruct effectStr;
        if (LongTimeEffects->find( effect->_effectId, effectStr) )
        {
            if ( effectStr.script )
            {

                effectStr.script->doubleEffect(foundeffect, _owner);
            }
        }
    }
    effect->firstAdd(); //set first add for this effect
}

bool CLongTimeCharacterEffects::removeEffect( uint16_t effectid)
{
    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        if ( (*it)->_effectId == effectid )
        {
            LongTimeEffectStruct effect;
            if (LongTimeEffects->find( (*it)->_effectId, effect) )
            {
                if ( effect.script )
                {
                    effect.script->removeEffect((*it), _owner);
                }
            }            
            effectList.erase(it);
            delete (*it);
            return true;
        }
    }
    return false;   
}

bool CLongTimeCharacterEffects::removeEffect( std::string name )
{
    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        if ( (*it)->_effectName == name )
        {
            LongTimeEffectStruct effect;
            if (LongTimeEffects->find( (*it)->_effectId, effect) )
            {
                if ( effect.script )
                {
                    effect.script->removeEffect((*it), _owner);
                }
            }
            effectList.erase(it);
            delete (*it);
            return true;
        }
    }
    return false; 
}

bool CLongTimeCharacterEffects::removeEffect( CLongTimeEffect * effect)
{
    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        if ( (*it) == effect )
        {
            LongTimeEffectStruct effectStr;
            if (LongTimeEffects->find( (*it)->_effectId, effectStr) )
            {
                if ( effectStr.script )
                {
                    effectStr.script->removeEffect((*it), _owner);
                }
            }            
            effectList.erase(it);
            delete effect;
            effect = NULL;
            return true;
        }
    }
    return false; 
}

void CLongTimeCharacterEffects::checkEffects()
{
	if ( _effectTimer->Next() )
    {
		//if (  _owner->character == CCharacter::player ) std::cout<<"checkEffect: "<<checkChar->name<<" size: "<<effectList.size()<<std::endl;
        CLongTimeEffect * effect;
        int emexit = 0;
        if ( !effectList.empty() && (effectList.front()->_nextCalled > 0) )
        {
            effectList.front()->_nextCalled--;
            //if ( _owner->character == CCharacter::player ) std::cout<<"decreased nextcalled: "<<effectList.front()->_nextCalled<<std::endl;
        }
        else
        {
            //if (  _owner->character == CCharacter::player ) std::cout<<"calling an effectscript"<<std::endl;
            while ( !effectList.empty() && (emexit < 200) && ( effectList.front()->_nextCalled <= 0 ) )
            {
                emexit++;
                effect = effectList.front();
                effectList.pop_front();
                if ( effect->callEffect( _owner) )
                {
                    addEffect(effect);
                }
                else
                {
                    LongTimeEffectStruct effectStr;
                    if (LongTimeEffects->find( effect->_effectId, effectStr) )
                    {
                        if ( effectStr.script )
                        {
                            effectStr.script->removeEffect(effect, _owner);
                        }
                    }                    
                }
            }
        }
    }
}

bool CLongTimeCharacterEffects::save()
{
    //di::postgres::enable_trace_query = true;
    ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
    if ( _owner->character != CCharacter::player )
    {
        std::cout<<"called save for CLongtimeCharacterEffects but owner was no player"<<std::endl;
        return false;
    }
    CPlayer * player = dynamic_cast<CPlayer*>(_owner);
    if ( !_owner )
    {
        std::cout<<"error saving long time effects owner was NULL!"<<std::endl;
        //di::postgres::enable_trace_query = false;
        return false;
    }
    try 
    {
        {
            std::stringstream query;
            query << "DELETE FROM playerlteffects WHERE plte_playerid = " << transaction.quote(player->id);
            di::exec(transaction, query.str());
        }
        {
            std::stringstream query;
            query << "DELETE FROM playerlteffectvalues WHERE pev_playerid = " << transaction.quote(player->id);
            di::exec(transaction, query.str());
        }
        transaction.commit();
    }
    catch ( std::exception e)
    {
        std::cerr << "caught exception during saving LongTimeCharacterEffects effects: " << e.what() << std::endl;
        transaction.rollback();
        //di::postgres::enable_trace_query = false;
        return false;
    }   
    std::cout<<"deleting old data was successfull inserting new one"<<std::endl;
	bool allok = true;
    for ( EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it)
    {
        allok &= (*it)->save( player->id );
    }   
    //di::postgres::enable_trace_query = false;
    std::cout<<"saved data: "<<allok<<std::endl;
    return allok;
}

bool CLongTimeCharacterEffects::load()
{
    std::cout<<"try to load effects" <<std::endl;
    if ( _owner->character != CCharacter::player )
    {
        std::cout<<"called load for CLongtimeCharacterEffects but owner was no player"<<std::endl;
        return false;
    }
    CPlayer * player = dynamic_cast<CPlayer*>(_owner);
    try
    {
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        std::vector<uint16_t>effectid;
        std::vector<uint32_t>nextcalled;
        std::vector<uint32_t>lastcalled;
        std::vector<uint32_t>numbercalled;
        
        //Load the effect
        std::stringstream Query;
        Query << "SELECT plte_effectid, plte_nextcalled, plte_lastcalled, plte_numberCalled FROM playerlteffects WHERE plte_playerid = " <<transaction.quote(player->id)<<" ORDER BY plte_nextcalled ASC";
        size_t rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer>
                                    (transaction, effectid, nextcalled, lastcalled, numbercalled, Query.str() );
        if ( rows > 0 )
        {
            for ( size_t i = 0; i < rows; ++i )
            {
                CLongTimeEffect * effect = new CLongTimeEffect(effectid[i], nextcalled[i]);
                //set firstadd to false;
                effect->firstAdd();
                effect->_lastCalled = lastcalled[i];
                effect->_numberCalled = numbercalled[i];
                
                //Loading the values to the effect
                Query.str("");
                Query.clear();
                
                Query << "SELECT pev_name, pev_value FROM playerlteffectvalues WHERE pev_playerid = " << transaction.quote(player->id) << " and pev_effectid = " << transaction.quote(effect->_effectId); 
                
                std::vector<std::string>name;
                std::vector<uint32_t>value;
                
                size_t rows2 = di::select_all<di::Varchar, di::Integer>
                                             (transaction, name, value, Query.str() );
                if ( rows2 > 0 )
                {
                    for ( size_t y = 0; y < rows2; ++y)
                    {
                        effect->addValue(name[y],value[y]);
                    }
                }

               
                effectList.push_back(effect);
                //call the load script
                LongTimeEffectStruct effectStr;
                if (LongTimeEffects->find( effect->_effectId, effectStr) )
                {
                    if ( effectStr.script )
                    {
                        effectStr.script->loadEffect( effect, player);
                    }
                }
                
            }
        }
        std::cout<<"effects laoded" <<std::endl;

        return true;
    }
    catch ( std::exception e)
    {
        std::cerr << "Error while loading longtimeeffects for player: " << player->name << "(" << player->id << ") what: " << e.what() << std::endl;
        return false;
    }
}
