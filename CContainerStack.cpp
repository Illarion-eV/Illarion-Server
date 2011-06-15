#include "CContainerStack.h"

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
