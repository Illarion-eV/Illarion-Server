#ifndef _WAYPOINTLIST_HPP_
#define _WAYPOINTLIST_HPP_


#include "globals.h"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "CCharacter.hpp"

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
