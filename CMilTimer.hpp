#ifndef CMILTIMER_HH
#define CMILTIMER_HH

#include <sys/timeb.h>


//! ein Timer mit Millisekunden-Auflösung
class CMilTimer {

	public:

		//! Konstruktor
		// \param milsec Anzahl der Millisekunden zwischen zwei Schritten
		CMilTimer( long milsec );

		//! dient zur Abfrage, ob die Zeit schon abgelaufen ist
		// \return true falls die im Konstruktor angegebene Zeit seit
		// dem letzten erfolgreichen Aufruf abgelaufen ist, sonst false
		bool Next();


	private:

		//! letzte Alterung
		timeb last;

		//! die geforderte Zeitspanne in Millisekunden
		long gap;

};

#endif
