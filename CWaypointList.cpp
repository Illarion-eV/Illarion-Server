#include "CWaypointList.hpp"
#include "CWorld.hpp"
#include "CField.hpp"

CWaypointList::CWaypointList(CCharacter * movechar) : _movechar(movechar)
{

}

void CWaypointList::addFromList(const luabind::object &list)
{
	if ( list.is_valid() )
	{
		  if ( luabind::type(list) == LUA_TTABLE )
		  {
				for (luabind::iterator it(list), end; it != end; ++it) 
				{
					position pos = luabind::object_cast<position>( *it );
					positions.push_back(pos);
				}  
		  }        
	}
}

luabind::object CWaypointList::getWaypoints()
{
	std::cout<<"getWaypoints"<<std::endl;
	lua_State* luaState = CWorld::get()->getCurrentScript()->getLuaState();
	luabind::object list = luabind::newtable( luaState );
		
	int index = 1;
	for ( std::list<position>::iterator it = positions.begin(); it != positions.end(); ++it)
	{
		list[index] = (*it);
		index++;
	}
	return list;

}

void CWaypointList::addWaypoint(position pos)
{
	positions.push_back(pos);
}

void CWaypointList::clear()
{
	positions.clear();
}

bool CWaypointList::checkPosition()
{
	if ( positions.empty() )return false;
	if ( _movechar->pos == positions.front() )
	{
		positions.pop_front();
	}
	return true;
}

bool CWaypointList::recalcStepList()
{
	if ( !checkPosition() )return false;
	steplist.clear();
	steplist = _movechar->getStepList(positions.front(),max_fields_for_waypoints);
	return ( !steplist.empty() );
}

bool CWaypointList::makeMove()
{
	if ( steplist.empty() )
	{
		if ( !recalcStepList() )return false;
	}
	if (!_movechar->move(steplist.front()))
	{
		return recalcStepList();
	}
	steplist.pop_front();
	return true;
}
