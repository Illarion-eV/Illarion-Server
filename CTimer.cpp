#include "CTimer.h"

CTimer::CTimer( unsigned long int timegap ) {
	gap = timegap;
	last = time( NULL ) - timegap;
}


CTimer::~CTimer() {}


bool CTimer::next() {
	time_t temp = time( NULL );     // liefert die Sekunden seit dem 1.1.1970
	realgap = temp - last;
	if ( realgap >= gap ) {
		last = temp;
		return true;
	} else {
		return false;
	}
}
