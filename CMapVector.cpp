#include "CMapVector.hpp"
#include "CMap.hpp"

CMapVector::CMapVector() : std::vector < CMap* > () {
	lowX = 32767;
	highX = -32768;
}

void CMapVector::clear()
{   std::vector < CMap* >::clear();
    lowX = 32767;
    highX = -32768;
}

bool CMapVector::mapInRangeOf( short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z ) {
	CMapVector::iterator thisIterator;

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



bool CMapVector::findAllMapsInRangeOf( char rnorth, char rsouth, char reast, char rwest, position pos, std::vector < CMap* > &ret ) {
	CMapVector::iterator thisIterator;
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



bool CMapVector::findAllMapsWithXInRangeOf( short int start, short int end, std::vector < CMap* > &ret ) {
	CMapVector::iterator thisIterator;
	bool found_one = false;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->Max_X >= start ) && ( ( *thisIterator )->Min_X <= end ) ) {
			ret.push_back( *thisIterator );
			found_one = true;
		}// x
	}// iterator

	return found_one;
}



bool CMapVector::findMapForPos( short int x, short int y, short int z, CMap* &map ) {
	CMapVector::iterator thisIterator;

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



bool CMapVector::findMapForPos( position pos, CMap* &map ) {
	return findMapForPos( pos.x, pos.y, pos.z, map );
}



bool CMapVector::findLowestMapOverCharacter( position pos, CMap* &lowmap ) {
	CMapVector::iterator thisIterator;
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


bool CMapVector::InsertMap( CMap* newMap ) {
	if ( newMap != NULL ) {
		for ( CMapVector::iterator thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
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

