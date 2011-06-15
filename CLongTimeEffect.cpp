#include "db/ConnectionManager.h"
#include "CLongTimeEffect.hpp"
#include "CCharacter.hpp"
#include "CPlayer.hpp"
#include "CWorld.hpp"
#include "CLongTimeEffectTable.hpp"
#include "TableStructs.hpp"
#include "script/CLuaLongTimeEffectScript.hpp"

extern CLongTimeEffectTable * LongTimeEffects;


CLongTimeEffect::CLongTimeEffect(uint16_t effectId, uint32_t nextCalled) : _effectId(effectId), _effectName(""),  _nextCalled(nextCalled), _numberCalled(0), _lastCalled(0), _firstadd(true)
{
    LongTimeEffectStruct effect;
    LongTimeEffects->find(_effectId, effect);
    _effectName = effect.effectname;
    _values.clear();
}

CLongTimeEffect::CLongTimeEffect(std::string name, uint32_t nextCalled) : _effectId(0), _effectName(name), _nextCalled(nextCalled), _numberCalled(0), _lastCalled(0), _firstadd(true)
{
    LongTimeEffectStruct effect;
    LongTimeEffects->find(_effectId, effect);
    _effectId = effect.effectid;
    _values.clear();
}

CLongTimeEffect::~CLongTimeEffect()
{
    _values.clear();
}

bool CLongTimeEffect::callEffect(CCharacter * target)
{
    bool ret = false;
    LongTimeEffectStruct effect;
    if (LongTimeEffects->find(_effectId, effect) )
    {
        if ( effect.script )
        {
            ret = effect.script->callEffect(this, target);
            _lastCalled = _nextCalled;
            _numberCalled++;
        }
    }
    else
    {
        std::cout<<"can't find effect with id: "<<_effectId<<" to call the script."<<std::endl;
    }
    return ret;
}

void CLongTimeEffect::addValue(std::string name, uint32_t value)
{
    VALUETABLE::iterator it = _values.find(name.c_str());
    if ( it != _values.end() )
    {
        it->second = value;
    }
    else
    {
        char* sname = new char[name.length() + 1];
        strcpy( sname, name.c_str() );
        sname[ name.length() ] = 0;
        _values[ sname ] = value;
    }
}

void CLongTimeEffect::removeValue( std::string name )
{
    VALUETABLE::iterator it = _values.find(name.c_str());
    if ( it != _values.end() )
    {
        _values.erase(it);
    }   
}

bool CLongTimeEffect::findValue(std::string name, uint32_t &ret)
{
    VALUETABLE::iterator it = _values.find(name.c_str());
    if ( it != _values.end() )
    {
        ret = it->second;
        return true;
    } 
    else 
        return false;
}

bool CLongTimeEffect::save( uint32_t playerid )
{
    ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

    try 
    {
        di::insert(transaction, static_cast<uint32_t>(playerid), static_cast<uint16_t>(_effectId), static_cast<int32_t>(_nextCalled), static_cast<uint32_t>(_lastCalled),static_cast<uint32_t>(_numberCalled), "INSERT INTO playerlteffects (plte_playerid, plte_effectid, plte_nextcalled, plte_lastcalled, plte_numberCalled)");
		for ( VALUETABLE::iterator it = _values.begin(); it != _values.end(); ++it)
        {
			std::cout<<"inserting effektdata("<<_effectId<<") name: "<<it->first<<" value: "<<static_cast<uint32_t>(it->second)<<std::endl;
            di::insert(transaction, static_cast<uint32_t>(playerid), static_cast<uint16_t>(_effectId), it->first, static_cast<uint32_t>(it->second), "INSERT INTO playerlteffectvalues (pev_playerid, pev_effectid, pev_name, pev_value)");
        }
        transaction.commit();
        return true;
    }
    catch ( std::exception e)
    {
        std::cerr << "caught exception during saving lt effects: " << e.what() << std::endl;
        transaction.rollback();
        return false;
    }
    return true;
}

    
