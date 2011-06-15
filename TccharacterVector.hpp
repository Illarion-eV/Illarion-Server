#ifndef TCCHARACTERVECTOR_HH
#define TCCHARACTERVECTOR_HH

#include <string>
#include <vector>
#include "utility.hpp"

//! eine std::vector-Template-Klasse für CCharacter-Objekte
// Die Klasse erweitert die Funktionalität von std::vector um
// ccharacter - spezifische Suchfunktionen
template < class _Tp >
class ccharactervector : public std::vector < _Tp > {
	public:
		//! sucht in dem std::vector nach dem CCharacter mit Namen n
		// \param n der Name des CCharacter
		// \return _Tp Zeiger auf den gefunden CCharacter, NULL falls nicht gefunden
		_Tp find( std::string n );

		//! sucht in dem std::vector nach dem CCharacter mit der ID id
		// \param id die id des CCharacter
		// \return _Tp Zeiger auf den gefunden CCharacter, NULL falls nicht gefunden
		_Tp findID( TYPE_OF_CHARACTER_ID id );

		//! sucht in dem std::vector nach dem CCharacter mit den Koordinaten xc,yc,zc
		// \param xc X-Koordinate des gesuchten CCharacter
		// \param xc Y-Koordinate des gesuchten CCharacter
		// \param xc Z-Koordinate des gesuchten CCharacter
		// \return _Tp Zeiger auf den gefunden CCharacter, NULL falls nicht gefunden
		_Tp find( short int xc, short int yc, short int zc );

		//! sucht in dem std::vector nach dem CCharacter mit den Koordinaten xc,yc,zc
		// \param xc X-Koordinate des gesuchten CCharacter
		// \param xc Y-Koordinate des gesuchten CCharacter
		// \param xc Z-Koordinate des gesuchten CCharacter
		// \param ret Zeiger auf den gefunden CCharacter
		// \return true falls ein CCharacter gefunden wurde, false sonst
		bool find( short int xc, short int yc, short int zc, _Tp &ret );

		//! löscht in dem std::vector den CCharacter mit den Koordinaten xc,yc,zc
		// \param xc X-Koordinate des gesuchten CCharacter
		// \param xc Y-Koordinate des gesuchten CCharacter
		// \param xc Z-Koordinate des gesuchten CCharacter
		// \return true falls ein CCharacter gelöscht wurde, false sonst
		bool remove( short int xc, short int yc, short int zc );

		//! sucht in dem std::vector nach dem Character mit der Id id
		// \param id die ID des gesuchten CCharacter
		// \param newIt falls erfolgreich der Iterator auf den Character mit der ID id
		// \return true falls der CCharacter gefunden wurde, false sonst
		bool getIterator( TYPE_OF_CHARACTER_ID id, typename ccharactervector::iterator &newIt );

		//! sucht in dem std::vector nach CCharacter mit Koordinaten in der Nähe von (xc,yc,zc)
		// \param rnorth maximaler Abstand der CCharacter nach Norden
		// \param rsouth maximaler Abstand der CCharacter nach Süden
		// \param reast maximaler Abstand der CCharacter nach Osten
		// \param rwest maximaler Abstand der CCharacter nach Westen
		// \param rup maximaler Abstand der CCharacter nach oben
		// \param rdown maximaler Abstand der CCharacter nach unten
		// \return std::vector<_Tp> Vektor mit allen CCharacter mit Koordinate (x,y,z) für die gilt:
		// (zc-rup <= z <= zc+down) und (xc-rwest <= x <= xc+reast) und (yc-rnorth <= y <= yc+rwest)
		std::vector < _Tp > findAllCharactersInRangeOf(short int xc, short int yc, short int zc, int distancemetric );
		std::vector < _Tp > findAllCharactersInMaxRangeOf(short int xc, short int yc, short int zc, int distancemetric );

		//! sucht in dem std::vector nach lebenden CCharacter mit Koordinaten in der Nähe von (xc,yc,zc)
		// \param rnorth maximaler Abstand der CCharacter nach Norden
		// \param rsouth maximaler Abstand der CCharacter nach Süden
		// \param reast maximaler Abstand der CCharacter nach Osten
		// \param rwest maximaler Abstand der CCharacter nach Westen
		// \param rup maximaler Abstand der CCharacter nach oben
		// \param rdown maximaler Abstand der CCharacter nach unten
		// \return std::vector<_Tp> Vektor mit allen CCharacter mit Koordinate (x,y,z) für die gilt:
		// (zc-rup <= z <= zc+down) und (xc-rwest <= x <= xc+reast) und (yc-rnorth <= y <= yc+rwest)
		std::vector < _Tp > findAllAliveCharactersInRangeOf(short int xc, short int yc, short int zc, int distancemetric );
        
        /**
        * looks in the vector for all characters which are alive and coordinates in the near of the given coordinates 
        * but only at the same map ( z = z )
        */
        std::vector < _Tp > findAllAliveCharactersInRangeOfOnSameMap(short int xc, short int yc, short int zc, int distancemetric );

		//! sucht in dem std::vector nach CCharacter mit X-Koordinaten für die gilt startx <= x <= endx
		// \param startx kleinste X-Koordinate für akzeptierte CCharacter
		// \param endx größte X-Koordinate für akzeptierte CCharacter
		// \return true falls mindestens ein CCharacter gefunden wurde
		bool findAllCharactersWithXInRangeOf( short int startx, short int endx, std::vector < _Tp > &ret );

};


template < class _Tp > _Tp ccharactervector < _Tp > ::find( std::string n ) {

	typename ccharactervector::iterator thisIterator;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( comparestrings_nocase( ( *thisIterator)->name, n )) {
			return * thisIterator;
		}
	}

	return NULL;

}


template < class _Tp > _Tp ccharactervector < _Tp > ::findID( TYPE_OF_CHARACTER_ID id ) {
	typename ccharactervector::iterator thisIterator;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->id ) == id ) {
			return * thisIterator;
		}
	}

	return NULL;

}


template < class _Tp > _Tp ccharactervector < _Tp > ::find( short int xc, short int yc, short int zc ) {
	typename ccharactervector::iterator thisIterator;
	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->pos.x ) == xc ) {
			if ( ( ( *thisIterator )->pos.y ) == yc ) {
				if ( ( ( *thisIterator )->pos.z ) == zc ) {
					return * thisIterator;
				}// z
			}// y
		}// x
	}// iterator

	return NULL;

}


template < class _Tp > bool ccharactervector < _Tp > ::find( short int xc, short int yc, short int zc, _Tp &ret ) {

	typename ccharactervector::iterator thisIterator;
	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->pos.x ) == xc ) {
			if ( ( ( *thisIterator )->pos.y ) == yc ) {
				if ( ( ( *thisIterator )->pos.z ) == zc ) {
					ret = *thisIterator;
					return true;
				}// z
			}// y
		}// x
	}// iterator

	return false;

}


template < class _Tp > bool ccharactervector < _Tp > ::remove( short int xc, short int yc, short int zc ) {

	typename ccharactervector::iterator thisIterator;
	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->pos.x ) == xc ) {
			if ( ( ( *thisIterator )->pos.y ) == yc ) {
				if ( ( ( *thisIterator )->pos.z ) == zc ) {
					erase( thisIterator );
					return true;
				}// z
			}// y
		}// x
	}// iterator

	return false;

}


template < class _Tp > bool ccharactervector < _Tp > ::getIterator( TYPE_OF_CHARACTER_ID id, typename ccharactervector::iterator &newIt ) {

	typename ccharactervector::iterator thisIterator;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->id ) == id ) {
			newIt = thisIterator;
			return true;
		}
	}

	return false;

}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllCharactersInRangeOf(short int xc, short int yc, short int zc, int distancemetric ) {

	std::vector < _Tp > temp;
	typename ccharactervector::iterator thisIterator;
	short int px;
	short int py;
	short int pz;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		pz = ( *thisIterator )->pos.z - zc;
		if ( ( -RANGEDOWN <= pz ) && ( pz <= RANGEUP ) ) {
			px = ( *thisIterator )->pos.x - xc;
			py = ( *thisIterator )->pos.y - yc;
			if ( (abs(px) + abs(py)) <= distancemetric ) {
				temp.push_back( *thisIterator );
			}
		}// z
	}// iterator

	return temp;

}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllCharactersInMaxRangeOf(short int xc, short int yc, short int zc, int distancemetric ) {

	std::vector < _Tp > temp;
	typename ccharactervector::iterator thisIterator;
	short int px;
	short int py;
	short int pz;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		pz = ( *thisIterator )->pos.z - zc;
		if ( ( -RANGEDOWN <= pz ) && ( pz <= RANGEUP ) ) {
			px = ( *thisIterator )->pos.x - xc;
			py = ( *thisIterator )->pos.y - yc;
			if ( (abs(px) <= distancemetric ) && (abs(py) <=distancemetric)) {
				temp.push_back( *thisIterator );
			}
		}// z
	}// iterator

	return temp;

}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllAliveCharactersInRangeOf(short int xc, short int yc, short int zc, int distancemetric ) {
	std::vector < _Tp > temp;
	typename ccharactervector::iterator thisIterator;
	short int px;
	short int py;
	short int pz;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		pz = ( *thisIterator )->pos.z - zc;
		if ( ( -RANGEDOWN <= pz ) && ( pz <= RANGEUP ) ) {
			px = ( *thisIterator )->pos.x - xc;
			py = ( *thisIterator )->pos.y - yc;
			if ( ( (abs(px) + abs(py)) <= distancemetric ) ||
					( ( distancemetric == 1 ) && ( abs(px) == 1 ) && ( abs(py) == 1) ) ) // Allow angle attacks
			{
				if ( ( *thisIterator )->IsAlive() ) {
					temp.push_back( *thisIterator );
				}
			}
		}// z
	}// iterator
	return temp;
}

template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllAliveCharactersInRangeOfOnSameMap(short int xc, short int yc, short int zc, int distancemetric ) {
	std::vector < _Tp > temp;
	typename ccharactervector::iterator thisIterator;
	short int px;
	short int py;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) 
    {
        //z coordinate is the same
        if ( (*thisIterator)->pos.z == zc )
        {
            px = ( *thisIterator )->pos.x - xc;
            py = ( *thisIterator )->pos.y - yc;
            if ( ( (abs(px) + abs(py)) <= distancemetric ) || ( ( distancemetric == 1 ) && ( abs(px) == 1 ) && ( abs(py) == 1) ) ) // Allow angle attacks
            {
                    if ( ( *thisIterator )->IsAlive() ) temp.push_back( *thisIterator );
            }
        }
	}// iterator
	return temp;
}


template < class _Tp > bool ccharactervector < _Tp > ::findAllCharactersWithXInRangeOf( short int startx, short int endx, std::vector < _Tp > &ret ) {
	typename ccharactervector::iterator thisIterator;
	bool found_one = false;

	for ( thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator ) {
		if ( ( ( *thisIterator )->pos.x >= startx ) && ( ( *thisIterator )->pos.x <= endx ) ) {
			ret.push_back( *thisIterator );
			found_one = true;
		}// x
	}// iterator

	return found_one;

}

#endif
