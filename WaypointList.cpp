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


#include "WaypointList.hpp"
#include "World.hpp"
#include "Field.hpp"

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
