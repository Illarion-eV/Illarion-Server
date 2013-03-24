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


#ifndef MAP_HH
#define MAP_HH

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
//#define Map_DEBUG

#include <string>
#include <unordered_map>
#include "MapException.hpp"
#include "globals.hpp"
#include "TableStructs.hpp"
#include "Item.hpp"
#include "Field.hpp"
#include "Container.hpp"

class CommonObjectTable;


extern std::vector<position> *contpos;


class Map {

public:

    //! Konstruktor
    // \param sizex die Anzahl der Felder in X-Richtung
    // \param sizey die Anzahl der Felder in Y-Richtung
    Map(unsigned short int sizex, unsigned short int sizey);

    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;

    //! Destruktor
    ~Map();

    //! l�dt eine gespeicherte Karte
    // \param name der Name der zu ladenden Karte
    // \param x_offs Verschiebung der zu ladenden Karte im physikalischen Feld
    // \param y_offs Verschiebung der zu ladenden Karte im physikalischen Feld
    // \return true falls Laden erfolgreich, false sonst
    bool Load(const std::string &name, unsigned short int x_offs, unsigned short int y_offs);

    //! speichert die aktuelle Karte
    // \param name Name unter der die Karte gespeichert werden soll
    // \return true falls Speichern erfolgreich, false sonst
    bool Save(const std::string &name);

    //! bereitet die Umrechnung von logischen x,y - Koordinaten in reale Feldindizes vor
    // \param minx die kleinste X-Koordinate
    // \param miny die kleinste Y-Koordinate
    // \param z die Ebene auf der die Karte liegt
    void Init(short int minx, short int miny, short int z);

    //! liefert in fip einen Zeiger auf das Field mit den entsprechenden Koordinaten zur�ck
    // \param fip der Zeiger auf das Field den Koordinaten x,y,z
    // \param x X-Koordinate
    // \param y Y-Koordinate
    // \return true falls das Feld existiert, false sonst
    bool GetPToCFieldAt(Field *&fip, short int x, short int y);

    //! liefert in fi eine Kopie des Feldes mit den entsprechenden Koordinaten zur�ck
    // \param fi eine Kopie des Field mit den Koordinaten x,y,z
    // \param x X-Koordinate
    // \param y Y-Koordinate
    // \return true falls das Feld existiert, false sonst
    bool GetCFieldAt(Field &fi, short int x, short int y);

    //! f�gt eine Kopie des Feldes fi an den entsprechenden Koordinaten ein
    // \param das Field zum Einf�gen
    // \param x X-Koordinate
    // \param y Y-Koordinate
    // \return true falls die x,y Koordinate existiert, false sonst
    bool PutCFieldAt(Field &fi, short int x, short int y);

    void ageItemsInHorizontalRange(short int xstart, short int xend);
    void ageContainers();

    //! setzt das Flag welches angibt, ob ein Spieler auf dem Feld ist auf t
    // \param x X-Koordinate
    // \param y Y-Koordinate
    // \param t true setzt einen Spieler auf das Feld, false l�scht ihn
    // \return true falls die x,y,z Koordinate existiert, false sonst
    bool SetPlayerAt(short int x, short int y, bool t);

    //! liefert die kleinste logisch m�gliche X-Koordinate zur�ck
    short GetMinX(void);

    //! liefert die kleinste logisch m�gliche Y-Koordinate zur�ck
    short GetMinY(void);

    //! liefert die gr��te logisch m�gliche X-Koordinate zur�ck
    short GetMaxX(void);

    //! liefert die gr��te logisch m�gliche Y-Koordinate zur�ck
    short GetMaxY(void);

    //! liefert die Anzahl der Felder in X-Richtung zur�ck
    unsigned short int GetWidth();

    //! liefert die Anzahl der Felder in Y-Richtung zur�ck
    unsigned short int GetHeight();

    //! definiert eine Template-Klasse "hash_map mit key position f�r ITEMVECTORMAP"
    typedef std::unordered_map<MAP_POSITION, Container::CONTAINERMAP> CONTAINERHASH;

    //! die Inhalte aller Container die direkt auf der Karte liegen mit der dazugeh�rigen Koordinate
    CONTAINERHASH maincontainers;

    //! legt ein Item auf ein Feld
    // \param it das Item welches auf das Feld gelegt werden soll
    // \param pos die X,Y-Koordinate des Feldes
    // \return true falls erfolgreich
    bool addItemToPos(Item it, MAP_POSITION pos);

    //! legt einen Container auf ein Feld
    // \param it ein Item welches einen Container darstellt
    // \param cc der Inhalt des Containers
    // \param pos die X,Y -Koordinate des Feldes
    // \return true falls der Container erfolgreich auf das Feld gelegt werden konnte
    bool addContainerToPos(Item it, Container *cc, MAP_POSITION pos);
    bool addAlwaysContainerToPos(Item it, Container *cc, MAP_POSITION pos);

    //! sucht ein betretbares Feld in der N�he von (x,y)
    // \param cf das gefundene Feld
    // \param x vor Ausf�hrung der Funktion der Startpunkt der Suche, danach die X-Koordinate des gefundenen Feldes
    // \param y vor Ausf�hrung der Funktion der Startpunkt der Suche, danach die Y-Koordinate des gefundenen Feldes
    // \return true falls ein Feld gefunden wurde, false sonst
    bool findEmptyCFieldNear(Field *&cf, short int &x, short int &y);

    //! die kleinste logisch m�gliche X-Koordinate
    short Min_X;

    //! die kleinste logisch m�gliche Y-Koordinate
    short Max_X;

    //! die gr��te logisch m�gliche X-Koordinate
    short Min_Y;

    //! die gr��te logisch m�gliche Y-Koordinate
    short Max_Y;

    //! die Anzahl der Felder in X-Richtung
    short Width;

    //! die Anzahl der Felder in Y-Richtung
    short Height;

    //! die Z-Koordinate der Karte
    short int Z_Level;

    //! true, falls die Karte schon initialisiert ist
    // ,also Min_X, Min_Y, Max_X und Max_Y definiert sind
    bool Map_initialized;

    //! Zeiger auf die Hauptebene der Karte
    Field **MainMap;

    //! konvertiert eine logische X-Koordinate in einen Feldindex
    // \throw Exception_CoordinateOutOfRange
    inline unsigned short int Conv_X_Koord(short int x);

    //! konvertiert eine logische Y-Koordinate in einen Feldindex
    // \throw Exception_CoordinateOutOfRange
    inline unsigned short int Conv_Y_Koord(short int y);

    inline   short int Conv_To_X(unsigned short int x);

    inline short int Conv_To_Y(unsigned short int y);

    std::vector<int>::iterator ercontit;
};

#endif
