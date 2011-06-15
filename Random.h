#ifndef RANDOM_HH
#define RANDOM_HH

#include <utility>

//! erzeugt eine Zufallszahl mit maximalem Bereich von [0..0xFFFF]
// ( verwendet rand() )
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min <= max eine Zufallszahl im Bereich [min..max], sonst undefiniert
unsigned short int unsignedShortRandom( const unsigned short int min, const unsigned short int max );

//! erzeugt eine Zufallszahl mit maximalem Bereich von [-32,768..32,767]
// ( verwendet rand() )
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min <= max eine Zufallszahl im Bereich [min..max], sonst undefiniert
signed short int signedShortRandom( const signed short int min, const signed short int max );

//! erzeugt eine Zufallszahl mit maximalem Bereich von [0..0xFFFF] bzw. [-32,768..32,767]
// und verwendet dazu unsignedShortRandom(..) bzw. signedShortRandom(..)
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min und max für unsignedShortRandom(..) gültig sind,
// oder min und max für signedShortRandom(..) gültig sind,
// eine Zufallszahl im Bereich [min..max], sonst undefiniert
int rnd( int min, int max );

inline int rnd(std::pair<unsigned short, unsigned short> in) {
	return rnd(in.first, in.second);
}

//! erzeugt eine Zufallszahl mit maximalem Bereich von [0..0xFFFF]
// ( verwendet eine eigene Zufallsfunktion )
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min <= max eine Zufallszahl im Bereich [min..max], sonst undefiniert
unsigned short int unsignedShortRandom_2( const unsigned short int min, const unsigned short int max );

//! erzeugt eine Zufallszahl mit maximalem Bereich von [-32,768..32,767]
// ( verwendet eine eigene Zufallsfunktion )
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min <= max eine Zufallszahl im Bereich [min..max], sonst undefiniert
signed short int signedShortRandom_2( const signed short int min, const signed short int max );

//! erzeugt eine Zufallszahl mit maximalem Bereich von [0..0xFFFF] bzw. [-32,768..32,767]
// und verwendet dazu unsignedShortRandom_2(..) bzw. signedShortRandom_2(..)
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
// \return falls min und max für unsignedShortRandom_2(..) gültig sind,
// oder min und max für signedShortRandom_2(..) gültig sind,
// eine Zufallszahl im Bereich [min..max], sonst undefiniert
int rnd_2( int min, int max );

//! erzeugt Zufallszahlen (mittels rnd, rnd_2 - Grenzen beachten !)
// und gibt diese auf cout aus
// \param nCount Anzahl der zu erzeugenden Zufallszahlen
// \param min kleinstes zugelassenes Ergebnis
// \param max größtes zugelassenes Ergebnis
void RunRandom( const int nCount, const int min, const int max );

//! initialisiert den Zufallsgenerator
void initRandom();

//! initialisiert den Zufallsgenerator für die *_2 Funktionen
void initRandom_2( unsigned char inc );

#endif

