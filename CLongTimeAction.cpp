#include "CLongTimeAction.hpp"
#include "CWorld.hpp"
#include "CPlayer.hpp"
#include "CMilTimer.hpp"
#include "CCharacter.hpp"

#include "script/CLuaItemScript.hpp"
#include "script/CLuaTileScript.hpp"
#include "script/CLuaMonsterScript.hpp"
#include "script/CLuaNPCScript.hpp"
#include "script/CLuaMagicScript.hpp"

CLongTimeAction::CLongTimeAction(CPlayer * player, CWorld * world) : _owner(player) , _world(world), _actionrunning(false)
{
    _script.reset();
    _timetowaitTimer = NULL;
    _redoaniTimer = NULL;
    _redosoundTimer = NULL;
    _ani = 0;
    _sound = 0;    
    _targetId = 0;
}

void CLongTimeAction::setLastAction(boost::shared_ptr<CLuaScript> script, SouTar srce, SouTar trgt,unsigned short int counter, unsigned short int param, ActionType at)
{
    _script = script;
    _source = srce;
    _target = trgt;
    _counter = counter;
    _param = param;
    _at = at;
    if ( trgt.Character != NULL)_targetId = _target.Character->id;
    if ( srce.Character != NULL)
    {
        _sourceId = _source.Character->id;
        _sourceCharType = _source.Character->character;
    }
}

bool CLongTimeAction::checkAction()
{
    if ( _actionrunning )
    {
        //check if timetowaittimer is initialized and if we hit the next time
        if ( _timetowaitTimer && _timetowaitTimer->Next() )
        {
            successAction();
            return true;
        }
        if ( (_redoaniTimer) && (_redoaniTimer->Next()) && (_ani != 0) )
        {
            _world->makeGFXForAllPlayersInRange( _owner->pos.x, _owner->pos.y, _owner->pos.z, MAXVIEW, _ani);
        }
        if ( (_redosoundTimer) && (_redosoundTimer->Next()) && (_sound != 0) )
        {
            _world->makeSoundForAllPlayersInRange( _owner->pos.x, _owner->pos.y, _owner->pos.z, MAXVIEW, _sound);
        }
    }
    return false;
}

void CLongTimeAction::startLongTimeAction(unsigned short int timetowait, unsigned short int ani, unsigned short int redoani, unsigned short int sound, unsigned short int redosound)
{
    _actionrunning = true;
    _ani = ani;
    _sound = sound;
    
    //delete old timer and initialize new ones if necsessary
    if ( _timetowaitTimer ) delete _timetowaitTimer;
    _timetowaitTimer = new CMilTimer( timetowait * 100 );
    if ( _redoaniTimer ) {delete _redoaniTimer; _redoaniTimer = NULL;}
    if ( _ani != 0 && redoani != 0 ) _redoaniTimer = new CMilTimer( redoani * 100 );
    if ( _redosoundTimer ) {delete _redosoundTimer; _redosoundTimer = NULL;}
    if ( _sound != 0 && redosound != 0 ) _redosoundTimer = new CMilTimer( redosound * 100 );
    if ( _sound != 0)_world->makeSoundForAllPlayersInRange( _owner->pos.x, _owner->pos.y, _owner->pos.z, MAXVIEW, _sound);
    if ( _ani != 0)_world->makeGFXForAllPlayersInRange( _owner->pos.x, _owner->pos.y, _owner->pos.z, MAXVIEW, _ani); 

}

bool CLongTimeAction::actionDisturbed(CCharacter * disturber)
{
    bool disturbed = false;
    checkSource();
    checkTarget();
    if ( _actionrunning )
    {
        if ( _script )
        {
            if ( (_at == AT_USE) )
            {
                //a itemscript
                if ( _source.Type == LUA_ITEM )
                {
                    boost::shared_ptr<CLuaItemScript>itScript = boost::dynamic_pointer_cast<CLuaItemScript>(_script);
                    disturbed = itScript->actionDisturbed(_owner, disturber);
                }
                //a tilescript
                else if ( _source.Type == LUA_FIELD )
                {
                    boost::shared_ptr<CLuaTileScript>tiScript = boost::dynamic_pointer_cast<CLuaTileScript>(_script);
                    disturbed = tiScript->actionDisturbed(_owner, disturber);
                    
                }
                //a character
                else if ( _source.Type == LUA_CHARACTER )
                {
                    if ( _sourceCharType == CCharacter::monster )
                    {
                        boost::shared_ptr<CLuaMonsterScript>monScript = boost::dynamic_pointer_cast<CLuaMonsterScript>(_script);
                        disturbed = monScript->actionDisturbed(_owner, disturber);
                    }
                    //a npc
                    else if (  _sourceCharType == CCharacter::npc )
                    {
                        boost::shared_ptr<CLuaNPCScript>npcScript = boost::dynamic_pointer_cast<CLuaNPCScript>(_script);
                        disturbed = npcScript->actionDisturbed(_owner, disturber);
                    }

                }
            }
            else if ( (_at == AT_MAGIC) )
            {
                boost::shared_ptr<CLuaMagicScript>mgScript = boost::dynamic_pointer_cast<CLuaMagicScript>(_script);
                disturbed = mgScript->actionDisturbed(_owner, disturber);
            }
            if ( disturbed )
            {
                abortAction();
                return true;
            }
            else
            {
                return false;
            }
        }
        else
            _actionrunning = false;
    }
    return false;
}

void CLongTimeAction::abortAction()
{
    checkSource();
    checkTarget();    
    if ( _actionrunning )
    {
        if ( _script )
        {
            if ( (_at == AT_USE) )
            {
                //a itemscript
                if ( _source.Type == LUA_ITEM )
                {
                    boost::shared_ptr<CLuaItemScript>itScript = boost::dynamic_pointer_cast<CLuaItemScript>(_script);
                    if ( _target.Type == LUA_ITEM || _target.Type == LUA_NONE )
                        itScript->UseItem( _owner, _source.item, _target.item, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    else if ( _target.Type == LUA_CHARACTER )
                        itScript->UseItemWithCharacter( _owner, _source.item, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    else if ( _target.Type == LUA_FIELD )
                        itScript->UseItemWithField( _owner, _source.item, _target.pos, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONABORTED) );
                }
                //a tilescript
                else if ( _source.Type == LUA_FIELD )
                {
                    boost::shared_ptr<CLuaTileScript>tiScript = boost::dynamic_pointer_cast<CLuaTileScript>(_script);
                    if ( _target.Type == LUA_NONE )
                        tiScript->useTile(_owner, _source.pos, _counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                    else if ( _target.Type == LUA_FIELD )
                        tiScript->useTileWithField(_owner , _source.pos, _target.pos, _counter , static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    else if ( _target.Type == LUA_ITEM )
                        tiScript->useTileWithItem(_owner, _source.pos, _target.item, _counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    else if ( _target.Type == LUA_CHARACTER )
                        tiScript->useTileWithCharacter(_owner, _source.pos, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                }
                //a character
                else if ( _source.Type == LUA_CHARACTER )
                {
                    //a monster
                    if (  _sourceCharType == CCharacter::monster )
                    {
                        boost::shared_ptr<CLuaMonsterScript>monScript = boost::dynamic_pointer_cast<CLuaMonsterScript>(_script);
                        if ( _target.Type == LUA_NONE )
                            monScript->useMonster(_source.Character,_owner,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                        else if ( _target.Type == LUA_FIELD )
                            monScript->useMonsterWithField(_source.Character,_owner,_target.pos,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                        else if ( _target.Type == LUA_ITEM )
                            monScript->useMonsterWithItem(_source.Character,_owner,_target.item,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                        else if ( _target.Type == LUA_CHARACTER )
                            monScript->useMonsterWithCharacter(_source.Character,_owner,_target.Character,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                    }
                    //a npc
                    else if (  _sourceCharType == CCharacter::npc )
                    {
                        boost::shared_ptr<CLuaNPCScript>npcScript = boost::dynamic_pointer_cast<CLuaNPCScript>(_script);
                        if ( _target.Type == LUA_NONE )
                            npcScript->useNPC(_owner, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED));
                        else if ( _target.Type == LUA_FIELD )
                            npcScript->useNPCWithField(_owner, _target.pos, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                        else if ( _target.Type == LUA_ITEM )
                            npcScript->useNPCWithItem(_owner, _target.item, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                        else if ( _target.Type == LUA_CHARACTER )
                            npcScript->useNPCWithCharacter(_owner, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    }

                }
            }
            else if ( (_at == AT_MAGIC)  )
            {
                boost::shared_ptr<CLuaMagicScript>mgScript = boost::dynamic_pointer_cast<CLuaMagicScript>(_script);
                if ( _target.Type == LUA_NONE )
                    mgScript->CastMagic(_owner,_counter,_param,static_cast<unsigned char>(LTS_ACTIONABORTED) );
                else if ( _target.Type == LUA_FIELD )
                    mgScript->CastMagicOnField(_owner,_target.pos,_counter,_param,static_cast<unsigned char>(LTS_ACTIONABORTED));
                else if ( _target.Type == LUA_CHARACTER )
                    mgScript->CastMagicOnCharacter(_owner,_target.Character,_counter,_param,static_cast<unsigned char>(LTS_ACTIONABORTED));
                    //Todo add ki handling here
                else if ( _target.Type == LUA_ITEM )
                    mgScript->CastMagicOnItem(_owner,_target.item,_counter,_param,static_cast<unsigned char>(LTS_ACTIONABORTED));
            } 
        }
    }
    _actionrunning = false;
    _script.reset();
    delete _redoaniTimer; _redoaniTimer = NULL;
    delete _redosoundTimer; _redosoundTimer = NULL;
    delete _timetowaitTimer; _timetowaitTimer = NULL;
    _ani = 0;
    _sound = 0;
    
    
}

void CLongTimeAction::successAction()
{
    checkSource();
    checkTarget();
    if ( _actionrunning )
    {
        _actionrunning = false;
        if ( _script )
        {
            if ( (_at == AT_USE) )
            {
                //a itemscript
                if ( _source.Type == LUA_ITEM )
                {
                    if ( _source.item.quality >= 100 )
                    {
                        boost::shared_ptr<CLuaItemScript>itScript = boost::dynamic_pointer_cast<CLuaItemScript>(_script);
                        if ( _target.Type == LUA_ITEM || _target.Type == LUA_NONE )
                            itScript->UseItem( _owner, _source.item, _target.item, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                        else if ( _target.Type == LUA_CHARACTER )
                            itScript->UseItemWithCharacter( _owner, _source.item, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                        else if ( _target.Type == LUA_FIELD )
                            itScript->UseItemWithField( _owner, _source.item, _target.pos, _counter, static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                    }
                }
                //a tilescript
                else if ( _source.Type == LUA_FIELD )
                {
                    boost::shared_ptr<CLuaTileScript>tiScript = boost::dynamic_pointer_cast<CLuaTileScript>(_script);
                    if ( _target.Type == LUA_NONE )
                        tiScript->useTile(_owner, _source.pos, _counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    else if ( _target.Type == LUA_FIELD )
                        tiScript->useTileWithField(_owner , _source.pos, _target.pos, _counter , static_cast<TYPE_OF_ITEM_ID>(_param), static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                    else if ( _target.Type == LUA_ITEM )
                        tiScript->useTileWithItem(_owner, _source.pos, _target.item, _counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                    else if ( _target.Type == LUA_CHARACTER )
                        tiScript->useTileWithCharacter(_owner, _source.pos, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                }
                //a character
                else if ( _source.Type == LUA_CHARACTER )
                {
                    //a monster
                    if (  _sourceCharType == CCharacter::monster )
                    {
                        boost::shared_ptr<CLuaMonsterScript>monScript = boost::dynamic_pointer_cast<CLuaMonsterScript>(_script);
                        if ( _target.Type == LUA_NONE )
                            monScript->useMonster(_source.Character,_owner,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        else if ( _target.Type == LUA_FIELD )
                            monScript->useMonsterWithField(_source.Character,_owner,_target.pos,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        else if ( _target.Type == LUA_ITEM )
                            monScript->useMonsterWithItem(_source.Character,_owner,_target.item,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        else if ( _target.Type == LUA_CHARACTER )
                            monScript->useMonsterWithCharacter(_source.Character,_owner,_target.Character,_counter,static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    }
                    //a npc
                    else if (  _sourceCharType == CCharacter::npc )
                    {
                        boost::shared_ptr<CLuaNPCScript>npcScript = boost::dynamic_pointer_cast<CLuaNPCScript>(_script);
                        if ( _target.Type == LUA_NONE )
                            npcScript->useNPC(_owner, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        else if ( _target.Type == LUA_FIELD )
                            npcScript->useNPCWithField(_owner, _target.pos, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                        else if ( _target.Type == LUA_ITEM )
                            npcScript->useNPCWithItem(_owner, _target.item, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                        else if ( _target.Type == LUA_CHARACTER )
                            npcScript->useNPCWithCharacter(_owner, _target.Character, _counter, static_cast<TYPE_OF_ITEM_ID>(_param),static_cast<unsigned char>(LTS_ACTIONABORTED) );
                    }
                }
            }
            else if (  (_at == AT_MAGIC) )
            {
                boost::shared_ptr<CLuaMagicScript>mgScript = boost::dynamic_pointer_cast<CLuaMagicScript>(_script);
                if ( _target.Type == LUA_NONE )
                    mgScript->CastMagic(_owner,_counter,_param,static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL) );
                else if ( _target.Type == LUA_FIELD )
                    mgScript->CastMagicOnField(_owner,_target.pos,_counter,_param,static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                else if ( _target.Type == LUA_CHARACTER )
                {
                    mgScript->CastMagicOnCharacter(_owner,_target.Character,_counter,_param,static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    //Todo add ki handling here
                }
                else if ( _target.Type == LUA_ITEM )
                    mgScript->CastMagicOnItem(_owner,_target.item,_counter,_param,static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
        
            }
        }
    }
    if ( !_actionrunning)
    {
        _script.reset();
        delete _redoaniTimer; _redoaniTimer = NULL;
        delete _redosoundTimer; _redosoundTimer = NULL;
        delete _timetowaitTimer; _timetowaitTimer = NULL;
        _ani = 0;
        _sound = 0;
    }
    
}

void CLongTimeAction::changeSource( CCharacter * cc )
{
    _source.Type = LUA_CHARACTER;
    _source.pos = cc->pos;
    _source.Character = cc;
    _sourceId = cc->id;
    _sourceCharType = cc->character;
}

void CLongTimeAction::changeSource( ScriptItem sI )
{
    _source.Type = LUA_ITEM;
    _source.pos = sI.pos;
    _source.item = sI;
    _sourceId = 0;
    _sourceCharType = CCharacter::player;
}

void CLongTimeAction::changeSource( position pos )
{
    _source.Type = LUA_FIELD;
    _source.pos = pos;
    _sourceId = 0;
    _sourceCharType = CCharacter::player;
}

void CLongTimeAction::changeSource()
{
    _source.Type = LUA_NONE;
    _sourceId = 0;
    _sourceCharType = CCharacter::player;
}

void CLongTimeAction::changeTarget( CCharacter * cc )
{
    _target.Type = LUA_CHARACTER;
    _target.pos = cc->pos;
    _target.Character = cc;
    _targetId = cc->id;
}

void CLongTimeAction::changeTarget( ScriptItem sI )
{
    _target.Type = LUA_ITEM;
    _target.pos = sI.pos;
    _target.item = sI;
    _targetId = 0;
}

void CLongTimeAction::checkTarget()
{
    if ( _targetId == 0 )
    {
        _source.Character=NULL;
        return;
    }
    else
    {
        if ( _targetId < MONSTER_BASE )
        {
            //player
            if (CWorld::get()->Players.findID( _targetId ) == NULL )_target.Character=NULL;
        }
        else if ( _targetId >= MONSTER_BASE && _targetId < NPC_BASE )
        {
            //monster
            if (CWorld::get()->Monsters.findID( _targetId ) == NULL )_target.Character=NULL;
        }
        else
        {
            if (CWorld::get()->Npc.findID( _targetId ) == NULL )_target.Character=NULL;
        }
        
    }
}

void CLongTimeAction::checkSource()
{
    if ( _sourceId == 0 )
    {
        _source.Character=NULL;
        _sourceCharType=0;
        return;
    }
    else
    {
        if ( _sourceId < MONSTER_BASE )
        {
            //player
            if (CWorld::get()->Players.findID( _sourceId ) == NULL )
            {
                _source.Character=NULL;
                 _sourceCharType=0;
            }
        }
        else if ( _sourceId >= MONSTER_BASE && _sourceId < NPC_BASE )
        {
            //monster
            if (CWorld::get()->Monsters.findID( _sourceId ) == NULL )
            {
                _source.Character=NULL;
                _sourceCharType=0;
            }
        }
        else
        {
            if (CWorld::get()->Npc.findID( _sourceId ) != NULL )
            {
                _source.Character=NULL;
                _sourceCharType=0;
            }
        }
        
    }
    
}

void CLongTimeAction::changeTarget( position pos )
{
    _target.Type = LUA_FIELD;
    _target.pos = pos;
    _targetId = 0;    
}

void CLongTimeAction::changeTarget()
{
    _target.Type = LUA_NONE;
}

