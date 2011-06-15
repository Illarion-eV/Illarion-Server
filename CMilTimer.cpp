#include "CMilTimer.hpp"

CMilTimer::CMilTimer( long milsec ) {
	gap = milsec;
	ftime( &last );
}



bool CMilTimer::Next() {
	timeb now;
	ftime( &now );

	long temp = ( ( now.time - last.time ) * 1000 ) + ( now.millitm - last.millitm );

	if ( gap > temp ) {
		return false;
	} else {
		last = now;
		return true;
	}
}

