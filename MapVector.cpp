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


#include "MapVector.hpp"
#include "Map.hpp"

MapVector::MapVector() : std::vector < Map* > () {
	lowX = 32767;
	highX = -32768;
}

void MapVector::clear()
{   std::vector < Map* >::clear();
    lowX = 32767;
    highX = -32768;
}

bool MapVector::mapInRangeOf( short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z ) {
	MapVector::iterator thisIterator;

	short int downright_X = upperleft_X + sizex - 1;
	short int downright_Y = upperleft_Y + sizey - 1;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( *thisIterator )->Z_Level == z ) {
			if ( ( ( *thisIterator )->Max_X >= upperleft_X ) && ( ( *thisIterator )->Min_X <= downright_X ) ) {
				if ( ( ( *thisIterator )->Max_Y >= upperleft_Y ) && ( ( *thisIterator )->Min_Y <= downright_Y ) ) {
					std::cout << "Map in range at Z:" << ( *thisIterator )->Z_Level <<
					" Min_X: " << ( *thisIterator )->Min_X <<
					" Max_X: " << ( *thisIterator )->Max_X <<
					" Min_Y: " << ( *thisIterator )->Min_Y <<
					" Max_Y: " << ( *thisIterator )->Max_Y <<
					std::endl;
					return true;
				}// y
			}// x
		}// z
	}// iterator

	return false;

}



bool MapVector::findAllMapsInRangeOf( char rnorth, char rsouth, char reast, char rwest, position pos, std::vector < Map* > &ret ) {
	MapVector::iterator thisIterator;
	bool found_one = false;

	short int upperleft_X = pos.x - rwest;
	short int downright_X = pos.x + reast;

	short int upperleft_Y = pos.y - rnorth;
	short int downright_Y = pos.y + rsouth;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( pos.z == ( *thisIterator )->Z_Level ) {
			if ( ( ( *thisIterator )->Max_X >= upperleft_X ) && ( ( *thisIterator )->Min_X <= downright_X ) ) {
				if ( ( ( *thisIterator )->Max_Y >= upperleft_Y ) && ( ( *thisIterator )->Min_Y <= downright_Y ) ) {
					ret.push_back( *thisIterator );
					found_one = true;
				}// y
			}// x
		}// z
	}// iterator

	return found_one;
}



bool MapVector::findAllMapsWithXInRangeOf( short int start, short int end, std::vector < Map* > &ret ) {
	MapVector::iterator thisIterator;
	bool found_one = false;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->Max_X >= start ) && ( ( *thisIterator )->Min_X <= end ) ) {
			ret.push_back( *thisIterator );
			found_one = true;
		}// x
	}// iterator

	return found_one;
}



bool MapVector::findMapForPos( short int x, short int y, short int z, Map* &map ) {
	MapVector::iterator thisIterator;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( z == ( *thisIterator )->Z_Level ) {
			if ( ( ( *thisIterator )->Max_X >= x ) && ( ( *thisIterator )->Min_X <= x ) ) {
				if ( ( ( *thisIterator )->Max_Y >= y ) && ( ( *thisIterator )->Min_Y <= y ) ) {
					map = ( *thisIterator );
					return true;
				}// y
			}// x
		}// z
	}// iterator

	map = NULL;
	return false;
}



bool MapVector::findMapForPos( position pos, Map* &map ) {
	return findMapForPos( pos.x, pos.y, pos.z, map );
}



bool MapVector::findLowestMapOverCharacter( position pos, Map* &lowmap ) {
	MapVector::iterator thisIterator;
	bool found_one = false;

	int ret = NOTHING;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ret > ( *thisIterator )->Z_Level ) && ( ( *thisIterator )->disappears ) ) {
			if ( ( *thisIterator )->isOverPositionInData(pos.x, pos.y, pos.z ) ) {
				ret = ( *thisIterator )->Z_Level;
				lowmap = ( *thisIterator );
				found_one = true;
			}
		}
	}// iterator
	return found_one;
}


bool MapVector::InsertMap( Map* newMap ) {
	if ( newMap != NULL ) {
		for ( MapVector::iterator thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
			if ( ( *thisIterator ) == newMap ) {
				return false;
			}
		}
		if ( newMap->Min_X < lowX )
			lowX = newMap->Min_X;
		if ( newMap->Max_X > highX )
			highX = newMap->Max_X;
		push_back( newMap );
		return true;
	}

	return false;

}

