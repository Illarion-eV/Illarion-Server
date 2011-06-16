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


#include "CMap.hpp"
#include "CCommonObjectTable.h"
#include "CContainerObjectTable.h"
#include "CLogger.hpp"
#include "CWorld.hpp"
#include "CPlayer.hpp"
#include <vector>
#include "netinterface/protocol/ServerCommands.hpp"

CMap::CMap( unsigned short int sizex, unsigned short int sizey ) {

#ifdef CMap_DEBUG
	std::cout << "CMap Konstruktor Start, Width:" << sizex << " Height:" << sizey << "\n";
#endif

	Width = sizex;
	Height = sizey;
	Min_X = 0;
	Max_X = 0;
	Min_Y = 0;
	Max_Y = 0;
	Z_Level = 0;
	Map_initialized = false;

	//das Array fr MainMap im Speicher anlegen
	MainMap = new CField * [ Width ];            // SCHRITT 1: ZEILEN EINRICHTEN.
	for ( int i = 0; i < Width; ++i ) {
		MainMap[ i ] = new CField[ Height ];      // SCHRITT 2: SPALTEN EINRICHTEN.
	}
	for ( int b = 0; b < Width; ++b ) {
		for ( int h = 0; h < Height; ++h ) {
			MainMap[ b ][ h ].setTileId( TRANSPARENT );
		}
	}

#ifdef CMap_DEBUG
	std::cout << "CMap Konstruktor Ende\n";
#endif

}


CMap::~CMap() {
#ifdef CMap_DEBUG
	std::cout << "CMap Destruktor Start" << std::endl;
#endif
	for ( int i = 0; i < Width; ++i ) {
		delete[] MainMap[ i ];
		MainMap[ i ] = NULL;
	}
	delete[] MainMap;
	MainMap = NULL;
#ifdef CMap_DEBUG
	std::cout << "CMap Destruktor Ende" << std::endl;
#endif
}



bool CMap::addItemToPos( Item it, MAP_POSITION pos ) {
	CField * cfnew;
	if ( GetPToCFieldAt( cfnew, pos.x, pos.y ) ) {
		if ( cfnew->addTopItem( it ) ) {
			return true;
		}
	}
	return false;
}



bool CMap::addContainerToPos( Item it, CContainer* cc, MAP_POSITION pos ) {

#ifdef CMap_DEBUG
	std::cout << "addContainerToPos: id: " << it.id << " x: " << pos.x << " y: " << pos.y << "\n";
#endif
	CField* cfnew;
	if ( GetPToCFieldAt( cfnew, pos.x, pos.y ) ) { // neues Feld vorhanden
		if ( cfnew->IsPassable() ) { // neues Feld begehbar
			if ( cfnew->items.size() < ( MAXITEMS - 1 ) ) { // noch Platz auf dem Feld
				if ( ContainerItems->find( it.id ) ) { // item ist ein Container
					CONTAINERHASH::iterator conmapn = maincontainers.find( pos );
					MAXCOUNTTYPE count = 0;
					if ( conmapn != maincontainers.end() ) { // containermap fr das Zielfeld gefunden
						CContainer::CONTAINERMAP::iterator iterat;
						// n�hste freie ID fr den Container auf dem Zielfeld suchen
						iterat = ( *conmapn ).second.find( count );
						while ( ( iterat != ( *conmapn ).second.end() ) && ( count < ( MAXITEMS - 2 ) ) ) {
							count++;
							iterat = ( *conmapn ).second.find( count );
						}
						// den Containerinhalt hinzufgen
						if ( count < ( MAXITEMS - 1 ) ) {
							( *conmapn ).second.insert( iterat, CContainer::CONTAINERMAP::value_type( count, cc ) );
						} else {
							return false;
						}
					} else { // neue containermap fr das Zielfeld anlegen
#ifdef CMap_DEBUG
						std::cout << "addContainerToPos: neue Containermap fr das Feld angelegt\n";
#endif
						conmapn = ( maincontainers.insert( CONTAINERHASH::value_type( pos, CContainer::CONTAINERMAP() ) ) ).first;
						// den Containerinhalt hinzufgen
						( *conmapn ).second.insert( CContainer::CONTAINERMAP::value_type( count, cc ) );
					}
					Item titem = it;
					titem.number = count;
					if ( !cfnew->addTopItem( titem ) ) {
						( *conmapn ).second.erase( count );
					} else {
						return true;
#ifdef CMap_DEBUG
						std::cout << "addContainerToPos: Container mit id: " << ( short int ) count << " auf das Feld gesetzt\n";
#endif

					}
				}
			}
		}
	}
	return false;
}


bool CMap::addAlwaysContainerToPos( Item it, CContainer* cc, MAP_POSITION pos ) {

#ifdef CMap_DEBUG
	std::cout << "addAlwaysContainerToPos: id: " << it.id << " x: " << pos.x << " y: " << pos.y << std::endl;
#endif
	CField* cfnew;
	if ( GetPToCFieldAt( cfnew, pos.x, pos.y ) ) { // neues Feld vorhanden
		if ( ContainerItems->find( it.id ) ) { // item ist ein Container
			CONTAINERHASH::iterator conmapn = maincontainers.find( pos );
			MAXCOUNTTYPE count = 0;
			if ( conmapn != maincontainers.end() ) { // containermap fr das Zielfeld gefunden
				CContainer::CONTAINERMAP::iterator iterat;
				// n�hste freie ID fr den Container auf dem Zielfeld suchen
				iterat = ( *conmapn ).second.find( count );
				while ( ( iterat != ( *conmapn ).second.end() ) && ( count < ( MAXITEMS - 2 ) ) ) {
					count++;
					iterat = ( *conmapn ).second.find( count );
				}
				// den Containerinhalt hinzufgen
				if ( count < ( MAXITEMS - 1 ) ) {
					( *conmapn ).second.insert( iterat, CContainer::CONTAINERMAP::value_type( count, cc ) );
				} else {
					return false;
				}
			} else { // neue containermap fr das Zielfeld anlegen
#ifdef CMap_DEBUG
				std::cout << "addAlwaysContainerToPos: neue Containermap fr das Feld angelegt\n";
#endif
				conmapn = ( maincontainers.insert( CONTAINERHASH::value_type( pos, CContainer::CONTAINERMAP() ) ) ).first;
				// den Containerinhalt hinzufgen
				( *conmapn ).second.insert( CContainer::CONTAINERMAP::value_type( count, cc ) );
			}
			Item titem = it;
			titem.number = count;
			if ( !cfnew->PutTopItem( titem ) ) {
				( *conmapn ).second.erase( count );
			} else {
				return true;
#ifdef CMap_DEBUG
				std::cout << "addAlwaysContainerToPos: Container mit id: " << ( short int ) count << " auf das Feld gesetzt\n";
#endif

			}
		}

	}
	return false;
}


void CMap::Init( short int minx, short int miny, short int z, bool disappear ) {

	Min_X = minx;
	Min_Y = miny;
	Max_X = Width + Min_X - 1;
	Max_Y = Height + Min_Y - 1;
	Z_Level = z;
	disappears = disappear;
#ifdef CMap_DEBUG
	std::cout << "CMap: Init, Min_X:" << Min_X << " Max_X:" << Max_X
	<< " Min_Y:" << Min_Y << " Max_Y:" << Max_Y << " Z:" << Z_Level << "\n";
#endif

	Map_initialized = true;

}



bool CMap::Create() {

#ifdef CMap_DEBUG
	std::cout << "CMap: Erstelle eine neue Welt.\n";
#endif

	// acker = field
	CField acker;
	acker.setTileId(4);
	acker.updateFlags();

	// wasser = water
	CField wasser;
	wasser.setTileId(6);
	wasser.updateFlags();

	// strasse = road
	CField strasse;
	strasse.setTileId(7);
	strasse.updateFlags();

	// dreck = dirt
	CField dreck;
	dreck.setTileId(8);
	dreck.updateFlags();

	// waldboden = forest soil
	CField waldboden;
	waldboden.setTileId(9);
	waldboden.updateFlags();

	Item loginpoint;
	loginpoint.id = 10;
	loginpoint.number = 1;
	CommonItems->find( loginpoint.id, tempCommon );
	// Anfangswert fr das Alter zuweisen
	loginpoint.wear = tempCommon.AgeingSpeed;

	Item tree;
	tree.id = 11;
	tree.number = 1;
	CommonItems->find( tree.id, tempCommon );
	// Anfangswert fr das Alter zuweisen
	tree.wear = tempCommon.AgeingSpeed;

	if ( Z_Level == 0 ) {
		// erstmal alles als als Gras - first times everything as as grass
		srand( ( unsigned ) time ( NULL ) );
		for ( unsigned short int x = 0; x < Width; ++x ) {
			for ( unsigned short int y = 0; y < Height; ++y ) {
				MainMap[ x ][ y ].setTileId( 11 + ( int ) ( 4.0 * rand() / ( RAND_MAX + 1.0 ) ) );
				MainMap[ x ][ y ].updateFlags();
			}
		}

		unsigned char zeile[1024];
		FILE* f;
		f = fopen("wueste.bmp", "rb"); // wueste = wild
		if (f!=NULL) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1) == 0 ) &&
								(x * 8 + 7 - xp < Width) &&
								(y < Height) ) {
							MainMap[ x * 8 + (7-xp) ][ y ].setTileId( 24 + ( int ) ( 4.0 * rand() / ( RAND_MAX + 1.0 ) ) );
							MainMap[ x * 8 + (7-xp) ][ y ].updateFlags();
						}
					}
				}
			}
			fclose(f);
		}
		f = fopen("wald.bmp", "rb"); // wald = forest
		if (f != NULL) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for (short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1) == 0 ) &&
								(x * 8 + 7 - xp < Width) &&
								( y < Height) ) {
							MainMap[ x * 8 + (7 - xp) ][ y ].setTileId( 28 + ( int ) ( 2.0 * rand() / ( RAND_MAX + 1.0 ) ) );
							MainMap[ x * 8 + (7 - xp) ][ y ].updateFlags();
						}
					}
				}
			}
			fclose( f );
		}
		f = fopen("wasser.bmp", "rb"); // wasser = water
		if (f != NULL) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f);
			fread( &bwidth, 4, 1, f);
			fread( &bheight, 4, 1, f);
			fread( &zeile, 36, 1, f);
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1 ) == 0 ) &&
								( x * 8 + 7 - xp < Width) &&
								( y < Height) ) {
							MainMap[ x*8+(7-xp) ][ y ].setTileId(6);
							MainMap[ x*8+(7-xp) ][ y ].updateFlags();
						}
					}
				}
			}
			fclose( f );
		}

		f = fopen("fels.bmp","rb"); // fels = rock
		if ( f != NULL ) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp ) & 1 ) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height ) ) {
							MainMap[ x * 8 + ( 7 - xp ) ][ y ].setTileId(54);
							MainMap[ x * 8 + ( 7 - xp ) ][ y ].updateFlags();
						}
					}
				}
			}
			fclose( f );
		}

		f = fopen("strassen.bmp", "rb"); // strassen = roads
		if ( f != NULL ) {
			int bwidth,bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for (short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp ) & 1) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height ) ) {
							MainMap[ x * 8 + (7 - xp) ][ y ].setTileId(7);
							MainMap[ x * 8 + (7 - xp) ][ y ].updateFlags();
						}
					}
				}
			}
			fclose(f);
		}

		Item tree;
		tree.id = 11;
		tree.number = 1;
		CommonItems->find( tree.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		tree.wear = tempCommon.AgeingSpeed;

		Item tree2;
		tree2.id = 299;
		tree2.number = 1;
		CommonItems->find( tree2.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		tree2.wear = tempCommon.AgeingSpeed;

		f = fopen("obstbaeume.bmp", "rb"); // obstbaeume = fruit rises
		if ( f != NULL ) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1 ) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height) ) {
							if ( rand() > RAND_MAX / 2 )
								MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( tree );
							else
								MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( tree2 );
                        }
					}
				}
			}
			fclose(f);
		}

		Item rock;
		rock.id = 232;
		rock.number = 1;
		CommonItems->find( rock.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		rock.wear = tempCommon.AgeingSpeed;

		Item rock2;
		rock2.id = 233;
		rock2.number = 1;
		CommonItems->find( rock2.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		rock2.wear = tempCommon.AgeingSpeed;

		f = fopen("rocks.bmp", "rb");
		if ( f != NULL ) {
			int bwidth,bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1 ) == 0 ) &&
								(x * 8 + 7 - xp < Width ) &&
								(y < Height) ) {
							if (rand() > RAND_MAX / 2)
								MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( rock );
							else
								MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( rock2 );
                        }
					}
				}
			}
			fclose(f);
		}

		Item palme;
		palme.id = 203;
		palme.number = 1;
		CommonItems->find( palme.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		palme.wear = tempCommon.AgeingSpeed;

		f = fopen("palmen.bmp", "rb");
		if (f != NULL) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1) == 0) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height) )
							MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( palme );
					}
				}
			}
			fclose(f);
		}

		Item Hecke;
		Hecke.id = 301;
		Hecke.number = 1;
		CommonItems->find( Hecke.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		Hecke.wear = tempCommon.AgeingSpeed;

		f = fopen("hecke.bmp", "rb");
		if (f != NULL) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp ) & 1) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height ) )
							MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( Hecke );
					}
				}
			}
			fclose(f);
		}

		Item Nadelbaum;
		Nadelbaum.id = 308;
		Nadelbaum.number = 1;
		CommonItems->find( Nadelbaum.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		Nadelbaum.wear = tempCommon.AgeingSpeed;

		f = fopen("nadelbaum.bmp", "rb"); // nadelbaum = coniferous tree
		if ( f != NULL ) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height) )
							MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( Nadelbaum );
					}
				}
			}
			fclose(f);
		}

		Item Pflanze;
		Pflanze.id = 273;
		Pflanze.number = 1;
		CommonItems->find( Pflanze.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		Pflanze.wear = tempCommon.AgeingSpeed;

		f = fopen("pflanzen.bmp", "rb"); // Pflanze = plant
		if ( f != NULL ) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1) ==0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height ) )
							MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( Pflanze );
					}
				}
			}
			fclose(f);
		}

		Item Pflanze2;
		Pflanze2.id = 274;
		Pflanze2.number = 1;
		CommonItems->find( Pflanze2.id, tempCommon );
		// Anfangswert fr das Alter zuweisen
		Pflanze2.wear = tempCommon.AgeingSpeed;

		f = fopen("sumpf.bmp", "rb"); // sumpf = sump
		if ( f != NULL ) {
			int bwidth, bheight;
			fread( &zeile, 18, 1, f );
			fread( &bwidth, 4, 1, f );
			fread( &bheight, 4, 1, f );
			fread( &zeile, 36, 1, f );
			for ( short y = bheight - 1; y >= 0; --y ) {
				fread( &zeile, bwidth / 8, 1, f );
				for ( short x = 0; x < bwidth / 8; ++x ) {
					for ( short xp = 7; xp >= 0; --xp ) {
						if ((((zeile[x] >> xp) & 1 ) == 0 ) &&
								( x * 8 + 7 - xp < Width ) &&
								( y < Height ) )
							MainMap[ x * 8 + (7 - xp) ][ y ].PutTopItem( Pflanze2 );
					}
				}
			}
			fclose(f);
		}

	}// Ende nur fr Z_Level == 0
	else {
		// erstmal alles als als Acker
		srand( ( unsigned ) time ( NULL ) );
		for ( unsigned short int x = 0; x < Width; ++x ) {
			for ( unsigned short int y = 0; y < Height; ++y ) {
				MainMap[ x ][ y ] = acker;
			}
		}
	}

#ifdef CMap_DEBUG
	std::cout << "CMap: Ende Create\n";
#endif

	return true;

}


bool CMap::Save( std::string name ) {
	std::cout << "CMap: Save - start, speichere" << name << std::endl;
	if ( ! Map_initialized ) {
		std::cout << "CMap: Karte nicht gespeichert da noch nicht initialisiert" << std::endl;
		return false;
	}

	std::ofstream* main_map;
	std::ofstream* main_item;
	std::ofstream* all_container;
    std::ofstream* main_warp;

	std::string a, b, w, e;

	a = name + "_map";
	b = name + "_item";
    w = name + "_warp";
	e = name + "_container";

	main_map = new std::ofstream( a.c_str(), std::ios::binary | std::ios::out );
	main_item = new std::ofstream( b.c_str(), std::ios::binary | std::ios::out );
    main_warp = new std::ofstream( w.c_str(), std::ios::binary | std::ios::out );
	all_container = new std::ofstream( e.c_str(), std::ios::binary | std::ios::out );

	if ( ( main_map->good() ) && ( main_item->good() ) && ( main_warp->good() ) && ( all_container->good() ) ) {
		//Map-Gr�e schreiben - Write Map Size
		main_map->write( ( char* ) & Width, sizeof( Width ) );
		main_map->write( ( char* ) & Height, sizeof( Height ) );
		main_map->write( ( char* ) & Min_X, sizeof( Min_X ) );
		main_map->write( ( char* ) & Min_Y, sizeof( Min_Y ) );
		main_map->write( ( char* ) & Z_Level, sizeof( Z_Level ) );

		main_item->write( ( char* ) & Width, sizeof( Width ) );
		main_item->write( ( char* ) & Height, sizeof( Height ) );
		main_item->write( ( char* ) & Min_X, sizeof( Min_X ) );
		main_item->write( ( char* ) & Min_Y, sizeof( Min_Y ) );
		main_item->write( ( char* ) & Z_Level, sizeof( Z_Level ) );

        main_warp->write( ( char* ) & Width, sizeof( Width ) );
        main_warp->write( ( char* ) & Height, sizeof( Height ) );
        main_warp->write( ( char* ) & Min_X, sizeof( Min_X ) );
        main_warp->write( ( char* ) & Min_Y, sizeof( Min_Y ) );
        main_warp->write( ( char* ) & Z_Level, sizeof( Z_Level ) );
		
        all_container->write( ( char* ) & Width, sizeof( Width ) );
		all_container->write( ( char* ) & Height, sizeof( Height ) );
		all_container->write( ( char* ) & Min_X, sizeof( Min_X ) );
		all_container->write( ( char* ) & Min_Y, sizeof( Min_Y ) );
		all_container->write( ( char* ) & Z_Level, sizeof( Z_Level ) );

		// Felder speichern - Store fields
		for ( unsigned short int x = 0; x < Width; ++x ) {
			for ( unsigned short int y = 0; y < Height; ++y ) {
				MainMap[ x ][ y ].Save( main_map, main_item, main_warp );
			}
		}

		CONTAINERHASH::iterator ptr;
		CContainer::CONTAINERMAP::iterator citer;
		unsigned long int fcount;
		MAXCOUNTTYPE icount;

		// Anzahl der Felder mit Eintr�en fr Containern
		fcount = maincontainers.size();
		all_container->write( ( char* ) & fcount, sizeof( fcount ) );

		if ( ! maincontainers.empty() ) {
			for ( ptr = maincontainers.begin(); ptr != maincontainers.end(); ++ptr ) {
				// die Koordinate schreiben
				all_container->write( ( char* ) & ptr->first, sizeof ptr->first );

				// die Anzahl Container in der CONTAINERMAP an der aktuellen Koordinate
				icount = ptr->second.size();
				all_container->write( ( char* ) & icount, sizeof( icount ) );

				if ( !ptr->second.empty() ) {
					for ( citer = ptr->second.begin(); citer != ptr->second.end(); ++citer ) {
						// die Kennung des Container speichern
						all_container->write( ( char* ) & ( ( *citer ).first ), sizeof( ( *citer ).first ) );
						// jeden CContainer speichern
						( *citer ).second->Save( all_container );
					}
				}
			}
		}

#ifdef CMap_DEBUG
		std::cout << "CMap: Save - end\n";
#endif

		delete main_map;
		main_map = NULL;
		delete main_item;
		main_item = NULL;
        delete main_warp;
        main_warp = NULL;
		delete all_container;
		all_container = NULL;

		return true;

	} else {

		std::cerr << "CMap: ERROR SAVING FILES \n";

		delete main_map;
		main_map = NULL;
		delete main_item;
		main_item = NULL;
        delete main_warp;
        main_warp = NULL;
		delete all_container;
		all_container = NULL;
		return false;

	}

}


bool CMap::GetPToCFieldAt( CField* &fip, short int x, short int y ) {

	unsigned short int tempx;
	unsigned short int tempy;
	try {
		tempx = Conv_X_Koord( x );
		tempy = Conv_Y_Koord( y );
	} catch ( Exception_CoordinateOutOfRange e ) {
		return false;
	}

	fip = &MainMap[ tempx ][ tempy ];

	return true;

}


bool CMap::Load( std::string name, unsigned short int x_offs, unsigned short int y_offs ) {

	std::cout << "CMap: Load - start, lade " << name  << " position: " << x_offs << " " << y_offs << std::endl;

	std::ifstream* main_map;
	std::ifstream* main_item;
    std::ifstream* main_warp;
	std::ifstream* all_container;

	std::string a, b, w, e;

	a = name + "_map";
	b = name + "_item";
    w = name + "_warp";
	e = name + "_container";

	main_map = new std::ifstream( a.c_str(), std::ios::binary | std::ios::in );
	main_item = new std::ifstream( b.c_str(), std::ios::binary | std::ios::in );
    main_warp = new std::ifstream( w.c_str(), std::ios::binary | std::ios::in );
	all_container = new std::ifstream( e.c_str(), std::ios::binary | std::ios::in );

	if ( ( main_map->good() ) && ( main_item->good() ) && ( main_warp->good() ) && ( all_container->good() ) ) {
		//Map-Gr�e lesen und berprfen - Read map size and examine
		short int twidth[ 4 ];
		short int theight[ 4 ];
		short int tminx[ 4 ];
		short int tminy[ 4 ];
		short int tzlevel[ 4 ];

		main_map->read( ( char* ) & twidth[ 0 ], sizeof( Width ) );
		main_map->read( ( char* ) & theight[ 0 ], sizeof( Height ) );
		main_map->read( ( char* ) & tminx[ 0 ], sizeof( Min_X ) );
		main_map->read( ( char* ) & tminy[ 0 ], sizeof( Min_Y ) );
		main_map->read( ( char* ) & tzlevel[ 0 ], sizeof( Z_Level ) );

		main_item->read( ( char* ) & twidth[ 1 ], sizeof( Width ) );
		main_item->read( ( char* ) & theight[ 1 ], sizeof( Height ) );
		main_item->read( ( char* ) & tminx[ 1 ], sizeof( Min_X ) );
		main_item->read( ( char* ) & tminy[ 1 ], sizeof( Min_Y ) );
		main_item->read( ( char* ) & tzlevel[ 1 ], sizeof( Z_Level ) );

        main_warp->read( ( char* ) & twidth[ 2 ], sizeof( Width ) );
        main_warp->read( ( char* ) & theight[ 2 ], sizeof( Height ) );
        main_warp->read( ( char* ) & tminx[ 2 ], sizeof( Min_X ) );
        main_warp->read( ( char* ) & tminy[ 2 ], sizeof( Min_Y ) );
        main_warp->read( ( char* ) & tzlevel[ 2 ], sizeof( Z_Level ) );

		all_container->read( ( char* ) & twidth[ 3 ], sizeof( Width ) );
		all_container->read( ( char* ) & theight[ 3 ], sizeof( Height ) );
		all_container->read( ( char* ) & tminx[ 3 ], sizeof( Min_X ) );
		all_container->read( ( char* ) & tminy[ 3 ], sizeof( Min_Y ) );
		all_container->read( ( char* ) & tzlevel[ 3 ], sizeof( Z_Level ) );

		if ( ( twidth[ 0 ] == twidth[ 1 ] ) && ( twidth[ 1 ] == twidth[ 2 ] ) && ( twidth[ 2 ] == twidth[ 3 ] ) ) {
			if ( ( theight[ 0 ] == theight[ 1 ] ) && ( theight[ 1 ] == theight[ 2 ] ) && ( theight[ 2 ] == theight[ 3 ] ) ) {
				if ( ( tminx[ 0 ] == tminx[ 1 ] ) && ( tminx[ 1 ] == tminx[ 2 ] ) && ( tminx[ 2 ] == tminx[ 3 ] ) ) {
					if ( ( tminy[ 0 ] == tminy[ 1 ] ) && ( tminy[ 1 ] == tminy[ 2 ] ) && ( tminy[ 2 ] == tminy[ 3 ] ) ) {
						if ( ( tzlevel[ 0 ] == tzlevel[ 1 ] ) && ( tzlevel[ 1 ] == tzlevel[ 2 ] ) && ( tzlevel[ 2 ] == tzlevel[ 3 ] ) ) {
							// die Kartengr�en der verschiedenen Dateien stimmen berein
							Z_Level = tzlevel[ 0 ];
							unsigned short int rightedge = twidth[ 0 ] + x_offs;
							unsigned short int lowedge = theight[ 0 ] + y_offs;
							// geforderte Verschiebung beachten
							if ( ( rightedge <= Width ) && ( lowedge <= Height ) ) {  // zu ladende Karte pa� in das aktuelle Array

								Min_X = tminx[ 0 ] - x_offs;
								Min_Y = tminy[ 0 ] - y_offs;

								Max_X = Width + Min_X - 1;
								Max_Y = Height + Min_Y - 1;

								CONTAINERHASH::iterator ptr;
								CContainer::CONTAINERMAP::iterator citer;

								if ( ! maincontainers.empty() ) {
									for ( ptr = maincontainers.begin(); ptr != maincontainers.end(); ++ptr ) {
										if ( ! ptr->second.empty() ) {
											for ( citer = ptr->second.begin(); citer != ptr->second.end(); ++citer ) {
												delete ( *citer ).second;
												( *citer ).second = NULL;
											}
										}
									}
								}

								maincontainers.clear();

								//////////////////////////////
								// Load the tiles and items //
								//////////////////////////////
								for ( unsigned short int x = x_offs; x < rightedge; ++x ) {
									for ( unsigned short int y = y_offs; y < lowedge; ++y ) {
										MainMap[ x ][ y ].Load( main_map, main_item, main_warp );
										// Added 2002-12-29 //
										MainMap[ x ][ y ].updateFlags();
									}
								}

								/////////////////////////
								// Load the Containers //
								/////////////////////////
								unsigned long int fcount;
								MAXCOUNTTYPE icount;
								MAXCOUNTTYPE key;
								MAP_POSITION pos;
								CContainer* tempc;
								CONTAINERHASH::iterator conmapn;

								// Anzahl der Felder mit Eintr�en fr Containern
								all_container->read( ( char* ) & fcount, sizeof( fcount ) );

								for ( unsigned long int i = 0; i < fcount; ++i ) {
									// die Koordinate lesen
									all_container->read( ( char* ) & pos, sizeof pos );

									// die Anzahl der Container in der CONTAINERMAP fr die aktuelle Koordinate lesen
									all_container->read( ( char* ) & icount, sizeof( icount ) );

									if ( icount > 0 ) {
										// fr die Koordinate eine CONTAINERMAP anlegen
										conmapn = ( maincontainers.insert( CONTAINERHASH::value_type( pos, CContainer::CONTAINERMAP() ) ) ).first;

										for ( MAXCOUNTTYPE k = 0; k < icount; ++k ) {
											// die Kennung des Container lesen
											all_container->read( ( char* ) & key, sizeof( key ) );

											//Suchen der ContainerItems auf den Feld
											CField field;
											//Laden des feldes.
											if ( GetCFieldAt(field, pos.x, pos.y) ) {
												ITEMVECTOR::iterator iter;
												//Schleife durch alle Items auf dem Feld
												for ( iter = field.items.begin(); iter != field.items.end(); iter++) {
													//das Item ist ein Containeritem
													ContainerStruct cont;
													if ( ContainerItems->find( iter->id, cont ) ) {
														//Der Container ist unser gesuchte Container
														if ( iter->number == key ) {
															// CContainer laden
															tempc = new CContainer(cont.ContainerVolume);
															tempc->Load( all_container );
															// den Containerinhalt hinzufgen
															( *conmapn ).second.insert( CContainer::CONTAINERMAP::value_type( key, tempc ) );
														}
													}
												}
											}
											//=======================================

										}
									}
								}

								Map_initialized = true;

								delete main_map;
								main_map = NULL;
								delete main_item;
								main_item = NULL;
                                delete main_warp;
                                main_warp = 0;
								delete all_container;
								all_container = NULL;
#ifdef CMap_DEBUG
								std::cout << "CMap: Load - end \n";
#endif
								return true;
							}
						}
					}
				}
			}
		}
	}

	std::cerr << "CMap: ERROR LOADING FILES \n";

	delete main_map;
	delete main_item;
    delete main_warp;
	delete all_container;

	return false;

}


bool CMap::GetCFieldAt( CField &fi, short int x, short int y ) {

	unsigned short int tempx;
	unsigned short int tempy;
	try {
		tempx = Conv_X_Koord( x );
		tempy = Conv_Y_Koord( y );
	} catch ( Exception_CoordinateOutOfRange e ) {
		return false;
	}

	fi = MainMap[ tempx ][ tempy ];

	return true;

}


bool CMap::PutCFieldAt( CField &fi, short int x, short int y ) {

	unsigned short int tempx;
	unsigned short int tempy;
	try {
		tempx = Conv_X_Koord( x );
		tempy = Conv_Y_Koord( y );
	} catch ( Exception_CoordinateOutOfRange e ) {
		return false;
	}

	MainMap[ tempx ][ tempy ] = fi;

	return true;

}


void CMap::DoAgeItems_XFromTo( short int xstart, short int xend, ITEM_FUNCT funct ) {

	short int tempMinX;
	short int tempMaxX;
	position posZ;
	try {
		tempMinX = Conv_X_Koord( xstart );
	} catch ( Exception_CoordinateOutOfRange e ) {
		tempMinX = 0;
	}

	try {
		tempMaxX = Conv_X_Koord( xend );
	} catch ( Exception_CoordinateOutOfRange e ) {
		tempMaxX = Width - 1;
	}

	MAP_POSITION pos;
	for ( short int x = tempMinX; x <= tempMaxX; ++x )
    {
		for ( short int y = 0; y < Height; ++y ) 
        {
            int8_t rotstate = MainMap[x][y].DoAgeItems( funct );
            //CLogger::writeMessage("rot_update", "aged items, rotstate: " + CLogger::toString( static_cast<int>(rotstate) ) ); 
            if ( rotstate == -1 )
            {
                //a container was rotted
                pos.x=Conv_To_X(x);
				pos.y=Conv_To_Y(y);
				// mindestens ein Containeritem wurde gel�cht -> mit Hilfe von erasedcontainers
				//   die Inhalte l�chen
				for ( ercontit = erasedcontainers->begin(); ercontit != erasedcontainers->end(); ++ercontit ) {
					CONTAINERHASH::iterator conmapn = maincontainers.find( pos );
					if ( conmapn != maincontainers.end() ) { // containermap fr das Zielfeld gefunden
						CContainer::CONTAINERMAP::iterator iterat;
						iterat = ( *conmapn ).second.find( *ercontit );
						if (iterat != ( *conmapn ).second.end()) {
							std::cout << "Containerinhalt auf Feld wird geloescht !" << std::endl;
							( *conmapn ).second.erase(iterat);
						}
						posZ.x=pos.x;
						posZ.y=pos.y;
						posZ.z=Z_Level;
						contpos->push_back(posZ);
					}
				}
				erasedcontainers->clear();
                
            }
            if ( rotstate != 0 )
            {
                position pos(Conv_To_X(x), Conv_To_Y(y), Z_Level);
                CLogger::writeMessage("rot_update", "aged items, pos: " + CLogger::toString(pos.x) + " " + CLogger::toString(pos.y) + " " + CLogger::toString(pos.z) );
                //a update is needed
                std::vector<CPlayer*> playersinview = CWorld::get()->Players.findAllCharactersInRangeOf( pos.x,pos.y, pos.z, MAXVIEW );
                //iterate through all the players in range and send the update for this field
                for ( std::vector<CPlayer*>::iterator it = playersinview.begin(); it != playersinview.end();++it)
                {
                    CLogger::writeMessage("rot_update", "aged items, update needed for: " + (*it)->name ); 
                    boost::shared_ptr<CBasicServerCommand>cmd( new CItemUpdate_TC(pos, MainMap[x][y].items ) );
                    (*it)->Connection->addCommand( cmd );
                }
            }

		}
	}

}



void CMap::ApplyToCFields_XFromTo( short int xstart, short int xend, CField::CFIELD_FUNCT funct ) {

	short int tempMinX;
	short int tempMaxX;

	try {
		tempMinX = Conv_X_Koord( xstart );
	} catch ( Exception_CoordinateOutOfRange e ) {
		tempMinX = 0;
	}

	try {
		tempMaxX = Conv_X_Koord( xend );
	} catch ( Exception_CoordinateOutOfRange e ) {
		tempMaxX = Width - 1;
	}

	for ( short int x = tempMinX; x <= tempMaxX; ++x ) {
		for ( short int y = 0; y < Height; ++y ) {
			funct( &( MainMap [ x ][ y ] ) );
		}
	}
}


bool CMap::SetPlayerAt( short int x, short int y, bool t ) {

	CField * temp;
	if ( GetPToCFieldAt( temp, x, y ) ) {
		temp->SetPlayerOnField( t );
		return true;
	}

	return false;

}


unsigned short int CMap::GetHeight() {

	return Height;

}


unsigned short int CMap::GetWidth() {

	return Width;

}


short CMap::GetMinX( void ) {

	return Min_X;

}


short CMap::GetMinY( void ) {

	return Min_Y;

}


short CMap::GetMaxX( void ) {

	return Max_X;

}


short CMap::GetMaxY( void ) {

	return Max_Y;

}


inline
unsigned short int CMap::Conv_X_Koord( short int x ) {

	unsigned short int temp;
	temp = x - Min_X;
	if ( temp >= Width ) {
		throw Exception_CoordinateOutOfRange();
	}

	return ( temp );

}


inline
unsigned short int CMap::Conv_Y_Koord( short int y ) {

	unsigned short int temp;
	temp = y - Min_Y;
	if ( temp >= Height ) {
		throw Exception_CoordinateOutOfRange();
	}

	return ( temp );

}


inline
short int CMap::Conv_To_X( unsigned short int x ) {

	short int temp;
	temp = x + Min_X;

	return ( temp );

}


inline
short int CMap::Conv_To_Y( unsigned short int y ) {

	short int temp;
	temp = y + Min_Y;

	return ( temp );

}


bool CMap::findEmptyCFieldNear( CField* &cf, short int &x, short int &y ) {

	short int startx = x;
	short int starty = y;

	unsigned char d = 0;
	while ( d < 6 ) {
		x = startx - d;
		while ( x <= startx + d ) {
			if ( GetPToCFieldAt( cf, x, d + starty ) ) {
				if ( cf->moveToPossible() ) {
					y = d + starty;
					return true;
				}
			}
			if ( GetPToCFieldAt( cf, x, starty - d ) ) {
				if ( cf->moveToPossible() ) {
					y = starty - d;
					return true;
				}
			}
			x++;
		}//Schleife durch x

		y = starty - d;
		while ( y <= d + starty ) {
			if ( GetPToCFieldAt( cf, d + startx, y ) ) {
				if ( cf->moveToPossible() ) {
					x = d + startx;
					return true;
				}
			}
			if ( GetPToCFieldAt( cf, startx - d, y ) ) {
				if ( cf->moveToPossible() ) {
					x = startx - d;
					return true;
				}
			}
			y++;
		}//Schleife durch y
		d++;
	}//Schleife durch Abstand d

	return false;

}


bool CMap::coversPositionInView( position pos ) {

	if ( Z_Level > pos.z ) {      // Die effektiv sichtbare Mittelpunktkoordinate fr die gegeben Karte bestimmen
		int view_X = pos.x - ( (Z_Level - pos.z) * LEVELDISTANCE );
		int view_Y = pos.y + ( (Z_Level - pos.z) * LEVELDISTANCE );

		if ( ( Max_X >= view_X ) && ( Min_X <= view_X ) ) {
			if ( ( Max_Y >= view_Y ) && ( Min_Y <= view_Y ) ) {
				CField * cftemp;
				if ( GetPToCFieldAt( cftemp, view_X, view_Y ) ) {
					if ( cftemp->getTileId() != TRANSPARENT ) return true;
				}
			}
		}
	}

	return false;

}


bool CMap::isOverPositionInData( short int x, short int y, short int z ) {
	if ( Z_Level > z ) {
		if ( ( Max_X >= x ) && ( Min_X <= x ) ) {
			if ( ( Max_Y >= y ) && ( Min_Y <= y ) ) {
				CField * cftemp;
				if ( GetPToCFieldAt( cftemp, x, y ) ) {
					if ( cftemp->getTileId() != TRANSPARENT ) return true;
				}
			}
		}
	}

	return false;

}


bool CMap::isOverMapInData( CMap* refmap ) {

	if ( refmap == NULL ) {
		return false;
	}

	if ( Z_Level > refmap->Z_Level ) {
		if ( ( refmap->Max_X >= Max_X ) && ( refmap->Min_X <= Min_X ) ) {
			if ( ( refmap->Max_Y >= Max_Y ) && ( refmap->Min_Y <= Min_Y ) ) {
				return true;
			}
		}
	}

	return false;

}


bool CMap::isFullyCoveredBy( CMap* refmap ) {

	if ( refmap == NULL ) {
		return false;
	}

	if ( Z_Level < refmap->Z_Level ) {
		int shift = (refmap->Z_Level - Z_Level) * LEVELDISTANCE;

		if ( ( refmap->Max_X + shift >= Max_X ) && ( refmap->Min_X + shift <= Min_X ) ) {
			if ( ( refmap->Max_Y - shift >= Max_Y ) && ( refmap->Min_Y - shift <= Min_Y ) ) {
				return true;
			}
		}
	}

	return false;

}


bool CMap::isVisibleFromInView( position pos, int distancemetric ) {

	return true;

	// Just get rid of compile warnings... //
	// Was previously commented out //
#if 0
	int viewCenter_X = pos.x - ( (pos.z - Z_Level) * LEVELDISTANCE );
	int viewCenter_Y = pos.y + ( (pos.z - Z_Level) * LEVELDISTANCE );


	if ((Min_X < (viewCenter_X + distancemetric)) && (Max_X > (viewCenter_X - distancemetric) ) ) {
		if ((Min_Y < (viewCenter_Y + distancemetric)) && (Max_Y > (viewCenter_Y - distancemetric) ) ) {
			return true;
		}
	}

	return false;
#endif
}

