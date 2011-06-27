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


#ifndef _WAYPOINTLIST_HPP_
#define _WAYPOINTLIST_HPP_


#include "globals.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "Character.hpp"

class CWaypointList
{
	public:
		static const uint8_t max_fields_for_waypoints = 12;
		
		
	
		CWaypointList(CCharacter * movechar);
	
		/**
		* füllt wegpunktliste mit werten aus einer lua liste
		*/
		void addFromList(const luabind::object &list);
		
		/**
		* Liefert eine Lua liste mit allen noch ab zu arbeitenden Wegpunkten
		*/
		luabind::object getWaypoints();
		
		/**
		* fügt einen einzelnen Wegpunkt hinzu
		*/
		void addWaypoint(position pos);
		
		/**
		* leert die Liste
		*/
		void clear();
		
		/**
		* Bewegt den Character um einen Wegpunkt zu erreichen
		* @return true wenn ein Schritt ausgeführt wurde um den Weg zu erreichen ansonsten false.
		*/
		bool makeMove();
		
				
	private:	
		
		/**
		* ab zu arbeitende Positionen
		*/
		std::list<position> positions;
		CCharacter * _movechar;
		
		/**
		* liste mit schritten um die nächste Position zu erreichen
		*/
		std::list<CCharacter::direction> steplist;
		
		/**
		* Prüft ob die Zielposition erreicht wurde
		* @return true wenn eine Prüfung ok war, false wenn innerhalb einer bestimmten Anzahl von versuchen das Ziel nicht erreicht wurde
		*/
		bool checkPosition();
		bool recalcStepList();
		
		
};
#endif
