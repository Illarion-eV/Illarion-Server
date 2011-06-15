#ifndef CMAPEXCEPTION_HH
#define CMAPEXCEPTION_HH

//! die Oberklasse aller Exception die in CMap auftreten können
class CMapException: public std::exception {}
;

//! wird geworfen, wenn Bereichsüberschreitungen bei der Umrechnung von logischen Koordinaten in physikalische Indizes auftreten
class Exception_CoordinateOutOfRange: public CMapException {}
;

class Exception_DifferentSizes: public CMapException {}
;

class Exception_DifferentOffsets: public CMapException {}
;

class Exception_MapToSmall: public CMapException {}
;


class Exception_FileCorrupt: public CMapException {}
;

#endif
