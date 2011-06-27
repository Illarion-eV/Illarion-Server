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


#include "ContainerStack.hpp"

CContainerStack::CContainerStack() {}


CContainerStack::CContainerStack( const CContainerStack& source ) {
	opencontainers = source.opencontainers;
}


CContainerStack& CContainerStack:: operator =( const CContainerStack& source ) {
	if ( this != &source ) {
		opencontainers = source.opencontainers;
	}
	return *this;
}


CContainerStack::~CContainerStack() {}


void CContainerStack::startContainer( CContainer* cc, bool carry ) {
	clear();
	openContainer( cc );
	inventory = carry;
}


void CContainerStack::openContainer( CContainer* cc ) {
	opencontainers.push_back( cc );
}


bool CContainerStack::closeContainer() {
	if ( opencontainers.empty() ) {
		return false;
	} else {
		opencontainers.pop_back();
		if ( !opencontainers.empty() ) {
			return true;
		} else {
			inventory = false;
			return false;
		}
	}
}



bool CContainerStack::isOnTop( CContainer* cc ) {
	if ( opencontainers.empty() ) {
		return false;
	} else {
		return ( opencontainers.back() == cc );
	}
}



CContainer* CContainerStack::top() {
	if ( opencontainers.empty() ) {
		return NULL;
	} else {
		return opencontainers.back();
	}
}



bool CContainerStack::contains( CContainer* cc ) {
	CONTAINERVECTOR::iterator theIterator;

	for ( theIterator = opencontainers.begin(); theIterator < opencontainers.end(); ++theIterator ) {
		if ( ( *theIterator ) == cc ) {
			return true;
		}
	}
	return false;
}



void CContainerStack::clear() {
	opencontainers.clear();
	inventory = false;
}



bool CContainerStack::inInventory() {
	return inventory;
}
