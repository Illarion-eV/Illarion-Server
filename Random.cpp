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


#include "Random.hpp"

#include <cstdlib>   // für rand(..)
#include <iostream> // für cout
#include <iomanip>  // für cout.setw()
#include <ctime>     // für time

// wird für die Zufallsfunktionen *_2(..) gebraucht, 731 ist
// ein "magischer" Startwert, der annähernd gleichverteilte
// Zufallszahlen bewirkt
unsigned short int lastRandom = 731;

unsigned short int unsignedShortRandom( const unsigned short int min, const unsigned short int max ) {

	unsigned short int r = rand();     // [0..0xFFFF]
	unsigned long m = ( ( ( unsigned long ) max + 1L - ( unsigned long ) min ) * ( unsigned long ) r ) / ( unsigned long ) ( 0xFFFF + 1L );    // [0..max-min]
	unsigned long s = ( unsigned long ) min + m ;    // [min..max]
	return ( s );

}


signed short int signedShortRandom( const signed short int min, const signed short int max ) {
	unsigned short int r = rand();     // [0..0xFFFF]
	r >>= 1;    // [0..0x7FFF] notwendig, da sonst bei der nächsten Multiplikation ein Überlauf auftreten kann
	signed long m = ( ( ( signed long ) max + 1L - ( signed long ) min ) * ( signed long ) r ) / ( signed long ) ( 0xFFFF + 1L );    // [0..max-min]
	signed long s = ( signed long ) min + m ;    // [min..max]
	//std::cout << std::setw(13) << r << std::setw(13) << m << std::setw(13) << s << std::endl;
	return ( s );
}


int rnd( int min, int max ) {
	if ( min < 0 ) {
		return signedShortRandom( min, max );
	} else {
		return unsignedShortRandom( min, max );
	}
}


unsigned short int unsignedShortRandom_2( const unsigned short int min, const unsigned short int max ) {
	lastRandom = lastRandom * 941;
	unsigned short int r = lastRandom;     // [0..0xFFFF]
	unsigned long m = ( ( ( unsigned long ) max + 1L - ( unsigned long ) min ) * ( unsigned long ) r ) / ( unsigned long ) ( 0xFFFF + 1L );    // [0..max-min]
	unsigned long s = ( unsigned long ) min + m ;    // [min..max]
	//std::cout << std::setw(13) << r << std::setw(13) << m << std::setw(13) << s << std::endl;
	return ( s );
}


signed short int signedShortRandom_2( const signed short int min, const signed short int max ) {
	lastRandom = lastRandom * 941;
	unsigned short int r = lastRandom;     // [0..0xFFFF]
	r >>= 1;    // [0..0x7FFF] notwendig, da sonst bei der nächsten Multiplikation ein Überlauf auftreten kann
	signed long m = ( ( ( signed long ) max + 1L - ( signed long ) min ) * ( signed long ) r ) / ( signed long ) ( 0xFFFF + 1L );    // [0..max-min]
	signed long s = ( signed long ) min + m ;    // [min..max]
	//std::cout << std::setw(13) << r << std::setw(13) << m << std::setw(13) << s << std::endl;
	return ( s );
}


int rnd_2( int min, int max ) {
	if ( min < 0 ) {
		return signedShortRandom_2( min, max );
	} else {
		return unsignedShortRandom_2( min, max );
	}
}



void RunRandom( const int nCount, const int min, const int max ) {
	std::cout << "Demonstrate random(" << min << " bis " << max << ")...";
	for ( int i = 0; i < nCount; ++i ) {
		if ( i % 4 == 0 ) std::cout << std::endl;
		std::cout << std::setw( 8 ) << rnd( min, max ) << std::setw( 8 ) << rnd_2( min, max );
	}
	std::cout << std::endl << std::endl;
}



void initRandom() {
	srand( ( unsigned ) time( NULL ) );
}



void initRandom_2( unsigned char inc ) {
	lastRandom = lastRandom + inc;
}
