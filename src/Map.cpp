//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "Map.hpp"

#include <vector>

#include "Logger.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "MapException.hpp"

#include "netinterface/protocol/ServerCommands.hpp"

extern std::vector<position> contpos;

Map::Map(unsigned short int sizex, unsigned short int sizey) : MainMap(sizex, std::vector<Field>(sizey, Field())) {
    Width = sizex;
    Height = sizey;
    Min_X = 0;
    Max_X = 0;
    Min_Y = 0;
    Max_Y = 0;
    Z_Level = 0;
    Map_initialized = false;
}

bool Map::addItemToPos(Item it, MAP_POSITION pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (cfnew->addTopItem(it)) {
            return true;
        }
    }

    return false;
}


bool Map::addContainerToPos(Item it, Container *cc, MAP_POSITION pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (cfnew->IsPassable()) {
            if (cfnew->items.size() < (MAXITEMS - 1)) {
                if (it.isContainer()) {
                    CONTAINERHASH::iterator conmapn = maincontainers.find(pos);
                    MAXCOUNTTYPE count = 0;

                    if (conmapn != maincontainers.end()) {
                        Container::CONTAINERMAP::iterator iterat;
                        iterat = (*conmapn).second.find(count);

                        while ((iterat != (*conmapn).second.end()) && (count < (MAXITEMS - 2))) {
                            count++;
                            iterat = (*conmapn).second.find(count);
                        }

                        if (count < (MAXITEMS - 1)) {
                            (*conmapn).second.insert(iterat, Container::CONTAINERMAP::value_type(count, cc));
                        } else {
                            return false;
                        }
                    } else {
                        conmapn = (maincontainers.insert(CONTAINERHASH::value_type(pos, Container::CONTAINERMAP()))).first;
                        (*conmapn).second.insert(Container::CONTAINERMAP::value_type(count, cc));
                    }

                    Item titem = it;
                    titem.setNumber(count);

                    if (!cfnew->addTopItem(titem)) {
                        (*conmapn).second.erase(count);
                    } else {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


bool Map::addAlwaysContainerToPos(Item it, Container *cc, MAP_POSITION pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (it.isContainer()) {
            CONTAINERHASH::iterator conmapn = maincontainers.find(pos);
            MAXCOUNTTYPE count = 0;

            if (conmapn != maincontainers.end()) {
                Container::CONTAINERMAP::iterator iterat;
                iterat = (*conmapn).second.find(count);

                while ((iterat != (*conmapn).second.end()) && (count < (MAXITEMS - 2))) {
                    count++;
                    iterat = (*conmapn).second.find(count);
                }

                if (count < (MAXITEMS - 1)) {
                    (*conmapn).second.insert(iterat, Container::CONTAINERMAP::value_type(count, cc));
                } else {
                    return false;
                }
            } else {
                conmapn = (maincontainers.insert(CONTAINERHASH::value_type(pos, Container::CONTAINERMAP()))).first;
                (*conmapn).second.insert(Container::CONTAINERMAP::value_type(count, cc));
            }

            Item titem = it;
            titem.setNumber(count);

            if (!cfnew->PutTopItem(titem)) {
                (*conmapn).second.erase(count);
            } else {
                return true;
            }
        }

    }

    return false;
}


void Map::Init(short int minx, short int miny, short int z) {
    Min_X = minx;
    Min_Y = miny;
    Max_X = Width + Min_X - 1;
    Max_Y = Height + Min_Y - 1;
    Z_Level = z;
    Map_initialized = true;
}


bool Map::Save(const std::string &name) {
    Logger::debug(LogFacility::World) << "Saving map " << name << Log::end;

    if (! Map_initialized) {
        Logger::warn(LogFacility::World) << "Can't save uninitialized map: " << name << Log::end;
        return false;
    }

    std::ofstream main_map { (name + "_map").c_str(), std::ios::binary | std::ios::out };
    std::ofstream main_item { (name + "_item").c_str(), std::ios::binary | std::ios::out };
    std::ofstream main_warp { (name + "_warp").c_str(), std::ios::binary | std::ios::out };
    std::ofstream all_container { (name + "_container").c_str(), std::ios::binary | std::ios::out };

    if ((main_map.good()) && (main_item.good()) && (main_warp.good()) && (all_container.good())) {
        // Write Map Size
        main_map.write((char *) & Width, sizeof(Width));
        main_map.write((char *) & Height, sizeof(Height));
        main_map.write((char *) & Min_X, sizeof(Min_X));
        main_map.write((char *) & Min_Y, sizeof(Min_Y));
        main_map.write((char *) & Z_Level, sizeof(Z_Level));

        main_item.write((char *) & Width, sizeof(Width));
        main_item.write((char *) & Height, sizeof(Height));
        main_item.write((char *) & Min_X, sizeof(Min_X));
        main_item.write((char *) & Min_Y, sizeof(Min_Y));
        main_item.write((char *) & Z_Level, sizeof(Z_Level));

        main_warp.write((char *) & Width, sizeof(Width));
        main_warp.write((char *) & Height, sizeof(Height));
        main_warp.write((char *) & Min_X, sizeof(Min_X));
        main_warp.write((char *) & Min_Y, sizeof(Min_Y));
        main_warp.write((char *) & Z_Level, sizeof(Z_Level));

        all_container.write((char *) & Width, sizeof(Width));
        all_container.write((char *) & Height, sizeof(Height));
        all_container.write((char *) & Min_X, sizeof(Min_X));
        all_container.write((char *) & Min_Y, sizeof(Min_Y));
        all_container.write((char *) & Z_Level, sizeof(Z_Level));

        // Felder speichern - Store fields
        for (unsigned short int x = 0; x < Width; ++x) {
            for (unsigned short int y = 0; y < Height; ++y) {
                MainMap[ x ][ y ].Save(main_map, main_item, main_warp);
            }
        }

        unsigned long int fcount;
        MAXCOUNTTYPE icount;

        // Anzahl der Felder mit Eintr�en fr Containern
        fcount = maincontainers.size();
        all_container.write((char *) & fcount, sizeof(fcount));

        if (! maincontainers.empty()) {
            for (auto ptr = maincontainers.begin(); ptr != maincontainers.end(); ++ptr) {
                // die Koordinate schreiben
                all_container.write((char *) & ptr->first, sizeof ptr->first);

                // die Anzahl Container in der CONTAINERMAP an der aktuellen Koordinate
                icount = ptr->second.size();
                all_container.write((char *) & icount, sizeof(icount));

                if (!ptr->second.empty()) {
                    for (auto citer = ptr->second.begin(); citer != ptr->second.end(); ++citer) {
                        // die Kennung des Container speichern
                        all_container.write((char *) & ((*citer).first), sizeof((*citer).first));
                        // jeden Container speichern
                        (*citer).second->Save(all_container);
                    }
                }
            }
        }

        return true;

    } else {

        Logger::error(LogFacility::World) << "Saving map failed: " << name << Log::end;
        return false;

    }

}


bool Map::GetPToCFieldAt(Field *&fip, short int x, short int y) {

    unsigned short int tempx;
    unsigned short int tempy;

    try {
        tempx = Conv_X_Koord(x);
        tempy = Conv_Y_Koord(y);
    } catch (Exception_CoordinateOutOfRange &e) {
        return false;
    }

    fip = &MainMap[ tempx ][ tempy ];

    return true;

}


bool Map::Load(const std::string &name, unsigned short int x_offs, unsigned short int y_offs) {

    Logger::debug(LogFacility::World) << "Loading map " << name  << " for position: " << x_offs << " " << y_offs << Log::end;

    std::ifstream main_map { (name + "_map").c_str(), std::ios::binary | std::ios::in };
    std::ifstream main_item { (name + "_item").c_str(), std::ios::binary | std::ios::in };
    std::ifstream main_warp { (name + "_warp").c_str(), std::ios::binary | std::ios::in };
    std::ifstream all_container { (name + "_container").c_str(), std::ios::binary | std::ios::in };

    if ((main_map.good()) && (main_item.good()) && (main_warp.good()) && (all_container.good())) {
        // Read map size and examine
        short int twidth[ 4 ];
        short int theight[ 4 ];
        short int tminx[ 4 ];
        short int tminy[ 4 ];
        short int tzlevel[ 4 ];

        main_map.read((char *) & twidth[ 0 ], sizeof(Width));
        main_map.read((char *) & theight[ 0 ], sizeof(Height));
        main_map.read((char *) & tminx[ 0 ], sizeof(Min_X));
        main_map.read((char *) & tminy[ 0 ], sizeof(Min_Y));
        main_map.read((char *) & tzlevel[ 0 ], sizeof(Z_Level));

        main_item.read((char *) & twidth[ 1 ], sizeof(Width));
        main_item.read((char *) & theight[ 1 ], sizeof(Height));
        main_item.read((char *) & tminx[ 1 ], sizeof(Min_X));
        main_item.read((char *) & tminy[ 1 ], sizeof(Min_Y));
        main_item.read((char *) & tzlevel[ 1 ], sizeof(Z_Level));

        main_warp.read((char *) & twidth[ 2 ], sizeof(Width));
        main_warp.read((char *) & theight[ 2 ], sizeof(Height));
        main_warp.read((char *) & tminx[ 2 ], sizeof(Min_X));
        main_warp.read((char *) & tminy[ 2 ], sizeof(Min_Y));
        main_warp.read((char *) & tzlevel[ 2 ], sizeof(Z_Level));

        all_container.read((char *) & twidth[ 3 ], sizeof(Width));
        all_container.read((char *) & theight[ 3 ], sizeof(Height));
        all_container.read((char *) & tminx[ 3 ], sizeof(Min_X));
        all_container.read((char *) & tminy[ 3 ], sizeof(Min_Y));
        all_container.read((char *) & tzlevel[ 3 ], sizeof(Z_Level));

        if ((twidth[ 0 ] == twidth[ 1 ]) && (twidth[ 1 ] == twidth[ 2 ]) && (twidth[ 2 ] == twidth[ 3 ])) {
            if ((theight[ 0 ] == theight[ 1 ]) && (theight[ 1 ] == theight[ 2 ]) && (theight[ 2 ] == theight[ 3 ])) {
                if ((tminx[ 0 ] == tminx[ 1 ]) && (tminx[ 1 ] == tminx[ 2 ]) && (tminx[ 2 ] == tminx[ 3 ])) {
                    if ((tminy[ 0 ] == tminy[ 1 ]) && (tminy[ 1 ] == tminy[ 2 ]) && (tminy[ 2 ] == tminy[ 3 ])) {
                        if ((tzlevel[ 0 ] == tzlevel[ 1 ]) && (tzlevel[ 1 ] == tzlevel[ 2 ]) && (tzlevel[ 2 ] == tzlevel[ 3 ])) {
                            // die Kartengr�en der verschiedenen Dateien stimmen berein
                            Z_Level = tzlevel[ 0 ];
                            unsigned short int rightedge = twidth[ 0 ] + x_offs;
                            unsigned short int lowedge = theight[ 0 ] + y_offs;

                            // geforderte Verschiebung beachten
                            if ((rightedge <= Width) && (lowedge <= Height)) {        // zu ladende Karte pa� in das aktuelle Array

                                Min_X = tminx[ 0 ] - x_offs;
                                Min_Y = tminy[ 0 ] - y_offs;

                                Max_X = Width + Min_X - 1;
                                Max_Y = Height + Min_Y - 1;

                                CONTAINERHASH::iterator ptr;
                                Container::CONTAINERMAP::iterator citer;

                                if (! maincontainers.empty()) {
                                    for (ptr = maincontainers.begin(); ptr != maincontainers.end(); ++ptr) {
                                        if (! ptr->second.empty()) {
                                            for (citer = ptr->second.begin(); citer != ptr->second.end(); ++citer) {
                                                delete(*citer).second;
                                                (*citer).second = nullptr;
                                            }
                                        }
                                    }
                                }

                                maincontainers.clear();

                                //////////////////////////////
                                // Load the tiles and items //
                                //////////////////////////////
                                for (unsigned short int x = x_offs; x < rightedge; ++x) {
                                    for (unsigned short int y = y_offs; y < lowedge; ++y) {
                                        MainMap[ x ][ y ].Load(main_map, main_item, main_warp);
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
                                Container *tempc;
                                CONTAINERHASH::iterator conmapn;

                                // Anzahl der Felder mit Eintr�en fr Containern
                                all_container.read((char *) & fcount, sizeof(fcount));

                                for (unsigned long int i = 0; i < fcount; ++i) {
                                    // die Koordinate lesen
                                    all_container.read((char *) & pos, sizeof pos);

                                    // die Anzahl der Container in der CONTAINERMAP fr die aktuelle Koordinate lesen
                                    all_container.read((char *) & icount, sizeof(icount));

                                    if (icount > 0) {
                                        // fr die Koordinate eine CONTAINERMAP anlegen
                                        conmapn = (maincontainers.insert(CONTAINERHASH::value_type(pos, Container::CONTAINERMAP()))).first;

                                        for (MAXCOUNTTYPE k = 0; k < icount; ++k) {
                                            // die Kennung des Container lesen
                                            all_container.read((char *) & key, sizeof(key));

                                            Field field;

                                            if (GetCFieldAt(field, pos.x, pos.y)) {

                                                for (auto iter = field.items.begin(); iter != field.items.end(); iter++) {

                                                    if (iter->isContainer()) {
                                                        if (iter->getNumber() == key) {
                                                            // Container laden
                                                            tempc = new Container(iter->getId());
                                                            tempc->Load(all_container);
                                                            // den Containerinhalt hinzufgen
                                                            (*conmapn).second.insert(Container::CONTAINERMAP::value_type(key, tempc));
                                                        }
                                                    }
                                                }
                                            }

                                            //=======================================

                                        }
                                    }
                                }

                                Map_initialized = true;

                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    Logger::error(LogFacility::World) << "Map: ERROR LOADING FILES: " << name << Log::end;

    return false;

}


bool Map::GetCFieldAt(Field &fi, short int x, short int y) {

    unsigned short int tempx;
    unsigned short int tempy;

    try {
        tempx = Conv_X_Koord(x);
        tempy = Conv_Y_Koord(y);
    } catch (Exception_CoordinateOutOfRange &e) {
        return false;
    }

    fi = MainMap[ tempx ][ tempy ];

    return true;

}


bool Map::PutCFieldAt(Field &fi, short int x, short int y) {

    unsigned short int tempx;
    unsigned short int tempy;

    try {
        tempx = Conv_X_Koord(x);
        tempy = Conv_Y_Koord(y);
    } catch (Exception_CoordinateOutOfRange &e) {
        return false;
    }

    MainMap[ tempx ][ tempy ] = fi;

    return true;

}

void Map::age() {
    ageContainers();
    ageItems();
}

void Map::ageItems() {
    position posZ;
    MAP_POSITION pos;

    for (short int x = 0; x < Width; ++x) {
        for (short int y = 0; y < Height; ++y) {
            int8_t rotstate = MainMap[x][y].DoAgeItems();

            if (rotstate == -1) {
                pos.x=Conv_To_X(x);
                pos.y=Conv_To_Y(y);

                for (const auto &erased : erasedcontainers) {
                    auto conmapn = maincontainers.find(pos);

                    if (conmapn != maincontainers.end()) {
                        auto iterat = conmapn->second.find(erased);

                        if (iterat != conmapn->second.end()) {
                            conmapn->second.erase(iterat);
                        }

                        posZ.x=pos.x;
                        posZ.y=pos.y;
                        posZ.z=Z_Level;
                        contpos.push_back(posZ);
                    }
                }

                erasedcontainers.clear();

            }

            if (rotstate != 0) {
                position pos(Conv_To_X(x), Conv_To_Y(y), Z_Level);
                Logger::debug(LogFacility::World) << "aged items, pos: " << pos << Log::end;
                std::vector<Player *> playersinview = World::get()->Players.findAllCharactersInScreen(pos);

                for (const auto &player : playersinview) {
                    Logger::debug(LogFacility::World) << "aged items, update needed for: " << *player << Log::end;
                    ServerCommandPointer cmd = std::make_shared<ItemUpdate_TC>(pos, MainMap[x][y].items);
                    player->Connection->addCommand(cmd);
                }
            }

        }
    }

}

void Map::ageContainers() {
    for (const auto &key_container : maincontainers) {
        const auto &container = key_container.second;

        for (const auto &content : container) {
            if (content.second) {
                content.second->doAge();
            }
        }
    }
}

bool Map::SetPlayerAt(short int x, short int y, bool t) {

    Field *temp;

    if (GetPToCFieldAt(temp, x, y)) {
        temp->SetPlayerOnField(t);
        return true;
    }

    return false;

}


unsigned short int Map::GetHeight() {

    return Height;

}


unsigned short int Map::GetWidth() {

    return Width;

}


short Map::GetMinX(void) {

    return Min_X;

}


short Map::GetMinY(void) {

    return Min_Y;

}


short Map::GetMaxX(void) {

    return Max_X;

}


short Map::GetMaxY(void) {

    return Max_Y;

}


inline
unsigned short int Map::Conv_X_Koord(short int x) {

    unsigned short int temp;
    temp = x - Min_X;

    if (temp >= Width) {
        throw Exception_CoordinateOutOfRange();
    }

    return (temp);

}


inline
unsigned short int Map::Conv_Y_Koord(short int y) {

    unsigned short int temp;
    temp = y - Min_Y;

    if (temp >= Height) {
        throw Exception_CoordinateOutOfRange();
    }

    return (temp);

}


inline
short int Map::Conv_To_X(unsigned short int x) {

    short int temp;
    temp = x + Min_X;

    return (temp);

}


inline
short int Map::Conv_To_Y(unsigned short int y) {

    short int temp;
    temp = y + Min_Y;

    return (temp);

}


bool Map::findEmptyCFieldNear(Field *&cf, short int &x, short int &y) {

    short int startx = x;
    short int starty = y;

    unsigned char d = 0;

    while (d < 6) {
        x = startx - d;

        while (x <= startx + d) {
            if (GetPToCFieldAt(cf, x, d + starty)) {
                if (cf->moveToPossible()) {
                    y = d + starty;
                    return true;
                }
            }

            if (GetPToCFieldAt(cf, x, starty - d)) {
                if (cf->moveToPossible()) {
                    y = starty - d;
                    return true;
                }
            }

            x++;
        }

        y = starty - d;

        while (y <= d + starty) {
            if (GetPToCFieldAt(cf, d + startx, y)) {
                if (cf->moveToPossible()) {
                    x = d + startx;
                    return true;
                }
            }

            if (GetPToCFieldAt(cf, startx - d, y)) {
                if (cf->moveToPossible()) {
                    x = startx - d;
                    return true;
                }
            }

            y++;
        }

        d++;
    }

    return false;

}

