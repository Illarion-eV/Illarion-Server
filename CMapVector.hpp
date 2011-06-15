#ifndef CMAPVECTOR_HH
#define CMAPVECTOR_HH

#include <vector>
#include "globals.h"

class CMap;

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/* #define CMapVector_DEBUG */

//! eine std::vector-Klasse für CMap-Objekte.
// Die Klasse erweitert die Funktionalität von std::vector um
// CMap - spezifische Suchfunktionen
class CMapVector : private std::vector < CMap* > {
	public:
	  using std::vector < CMap* >::iterator;
	  using std::vector < CMap* >::begin;
	  using std::vector < CMap* >::end;
	  using std::vector < CMap* >::size;

		CMapVector();
		
		void clear();

		//! sucht in dem std::vector nach CMap mit Koordinaten in der Nähe von pos
		// \param rnorth maximaler Abstand der CMap nach Norden
		// \param rsouth maximaler Abstand der CMap nach Süden
		// \param reast maximaler Abstand der CMap nach Osten
		// \param rwest maximaler Abstand der CMap nach Westen
		// \param pos der Mittelpunkt des Sichtfensters
		// \param ret der Vektor mit den gefundenen Karten
		// \return true, falls mindestens eine Karte gunden wurde, false sonst
		bool findAllMapsInRangeOf( char rnorth, char rsouth, char reast, char rwest, position pos, std::vector < CMap* > &ret );

		//! prüft, ob sich eine CMap im angegebenen Bereich befindet
		// \param upperleft_X obere linke X-Koordinate
		// \param upperleft_Y obere linke Y-Koordinate
		// \param sizex Breite des Bereiches
		// \param sizey Höhe des Bereiches
		// \param z Z-Koordinate der Ebene
		// \return true falls siche eine CMap in dem Bereich befindet
		bool mapInRangeOf( short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z);

		//! sucht in dem std::vector nach der CMap auf der pos liegt
		// \param pos die gesuchte Koordinate
		// \param map die gesucht CMap, falls sie gefunden wurde
		// \return false falls keine passende CMap gefunden wurde, sonst true
		bool findMapForPos( position pos, CMap* &map );

		//! sucht in dem std::vector nach der CMap auf der (x,y,z) liegt
		// \param x die gesuchte X-Koordinate
		// \param y die gesuchte Y-Koordinate
		// \param z die gesuchte Z-Koordinate
		// \param map die gesucht CMap, falls sie gefunden wurde
		// \return false falls keine passende CMap gefunden wurde, sonst true
		bool findMapForPos( short int x, short int y, short int z, CMap* &map );

		//! sucht in dem std::vector nach allen CMap die die X-Koordinaten von start bis end berühren
		// \param start kleinste X-Koordinate
		// \param end größte X-Koordinate
		// \param ret ein std::vector mit Zeigern auf alle gefundenen CMap
		// \return true, falls mindestens eine CMap gefunden wurde, false sonst
		bool findAllMapsWithXInRangeOf( short int start, short int end, std::vector < CMap* > &ret );

		//! sucht die niedrigste Karte über einer Position
		// \param pos die Position
		// \param lowmap die gefundene Karte
		// \return true falls eine Karte gefunden wurde, false sonst
		bool findLowestMapOverCharacter( position pos, CMap* &lowmap );

		//! fügt eine CMap in den std::vector ein und aktualisiert low_X und high_X
		// \param newMap die einzufügende CMap
		// \return true falls die CMap eingefügt werden konnte, false sonst
		bool InsertMap( CMap* newMap );

    short int getLowX(){ return lowX; };
    short int getHighX(){ return highX; };

  private:
		//! die kleinste X-Koordinate aller CMap im std::vector
		short int lowX;

		//! die großte X-Koordinate aller CMap im std::vector
		short int highX;
};
#endif
