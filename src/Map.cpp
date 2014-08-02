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

#include "netinterface/protocol/ServerCommands.hpp"

Map::Map(position origin, uint16_t width, uint16_t height)
    : origin(std::move(origin)), width(width), height(height),
      fields(width, std::vector<Field>(height, Field())) {}

bool Map::addItemToPos(Item item, MapPosition pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (cfnew->addTopItem(item)) {
            return true;
        }
    }

    return false;
}


bool Map::addContainerToPos(Item it, Container *cc, MapPosition pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (cfnew->IsPassable()) {
            if (cfnew->items.size() < (MAXITEMS - 1)) {
                if (it.isContainer()) {
                    auto conmapn = containers.find(pos);
                    MAXCOUNTTYPE count = 0;

                    if (conmapn != containers.end()) {
                        auto iterat = (*conmapn).second.find(count);

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
                        conmapn = (containers.emplace(pos, Container::CONTAINERMAP())).first;
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


bool Map::addAlwaysContainerToPos(Item it, Container *cc, MapPosition pos) {
    Field *cfnew;

    if (GetPToCFieldAt(cfnew, pos.x, pos.y)) {
        if (it.isContainer()) {
            auto conmapn = containers.find(pos);
            MAXCOUNTTYPE count = 0;

            if (conmapn != containers.end()) {
                auto iterat = (*conmapn).second.find(count);

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
                conmapn = (containers.emplace(pos, Container::CONTAINERMAP())).first;
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


bool Map::Save(const std::string &name) const {
    Logger::debug(LogFacility::World) << "Saving map " << name << Log::end;

    std::ofstream main_map { (name + "_map").c_str(), std::ios::binary | std::ios::out };
    std::ofstream main_item { (name + "_item").c_str(), std::ios::binary | std::ios::out };
    std::ofstream main_warp { (name + "_warp").c_str(), std::ios::binary | std::ios::out };
    std::ofstream all_container { (name + "_container").c_str(), std::ios::binary | std::ios::out };

    if ((main_map.good()) && (main_item.good()) && (main_warp.good()) && (all_container.good())) {
        main_map.write((char *) & width, sizeof(width));
        main_map.write((char *) & height, sizeof(height));
        main_map.write((char *) & origin.x, sizeof(origin.x));
        main_map.write((char *) & origin.y, sizeof(origin.y));
        main_map.write((char *) & origin.z, sizeof(origin.z));

        main_item.write((char *) & width, sizeof(width));
        main_item.write((char *) & height, sizeof(height));
        main_item.write((char *) & origin.x, sizeof(origin.x));
        main_item.write((char *) & origin.y, sizeof(origin.y));
        main_item.write((char *) & origin.z, sizeof(origin.z));

        main_warp.write((char *) & width, sizeof(width));
        main_warp.write((char *) & height, sizeof(height));
        main_warp.write((char *) & origin.x, sizeof(origin.x));
        main_warp.write((char *) & origin.y, sizeof(origin.y));
        main_warp.write((char *) & origin.z, sizeof(origin.z));

        all_container.write((char *) & width, sizeof(width));
        all_container.write((char *) & height, sizeof(height));
        all_container.write((char *) & origin.x, sizeof(origin.x));
        all_container.write((char *) & origin.y, sizeof(origin.y));
        all_container.write((char *) & origin.z, sizeof(origin.z));

        for (uint16_t x = 0; x < width; ++x) {
            for (uint16_t y = 0; y < height; ++y) {
                fields[ x ][ y ].Save(main_map, main_item, main_warp);
            }
        }

        unsigned long int fcount;
        MAXCOUNTTYPE icount;

        fcount = containers.size();
        all_container.write((char *) & fcount, sizeof(fcount));

        if (! containers.empty()) {
            for (auto ptr = containers.begin(); ptr != containers.end(); ++ptr) {
                all_container.write((char *) & ptr->first, sizeof ptr->first);

                icount = ptr->second.size();
                all_container.write((char *) & icount, sizeof(icount));

                if (!ptr->second.empty()) {
                    for (auto citer = ptr->second.begin(); citer != ptr->second.end(); ++citer) {
                        all_container.write((char *) & ((*citer).first), sizeof((*citer).first));
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


bool Map::GetPToCFieldAt(Field *&fip, int16_t x, int16_t y) {

    uint16_t tempx;
    uint16_t tempy;

    try {
        tempx = Conv_X_Koord(x);
        tempy = Conv_Y_Koord(y);
    } catch (FieldNotFound &e) {
        return false;
    }

    fip = &fields[ tempx ][ tempy ];

    return true;

}


bool Map::Load(const std::string &name) {

    Logger::debug(LogFacility::World) << "Loading map " << name << Log::end;

    std::ifstream main_map { (name + "_map").c_str(), std::ios::binary | std::ios::in };
    std::ifstream main_item { (name + "_item").c_str(), std::ios::binary | std::ios::in };
    std::ifstream main_warp { (name + "_warp").c_str(), std::ios::binary | std::ios::in };
    std::ifstream all_container { (name + "_container").c_str(), std::ios::binary | std::ios::in };

    if ((main_map.good()) && (main_item.good()) && (main_warp.good()) && (all_container.good())) {
        int16_t twidth[ 4 ];
        int16_t theight[ 4 ];
        int16_t tminx[ 4 ];
        int16_t tminy[ 4 ];
        int16_t tzlevel[ 4 ];

        main_map.read((char *) & twidth[ 0 ], sizeof(width));
        main_map.read((char *) & theight[ 0 ], sizeof(height));
        main_map.read((char *) & tminx[ 0 ], sizeof(origin.x));
        main_map.read((char *) & tminy[ 0 ], sizeof(origin.y));
        main_map.read((char *) & tzlevel[ 0 ], sizeof(origin.z));

        main_item.read((char *) & twidth[ 1 ], sizeof(width));
        main_item.read((char *) & theight[ 1 ], sizeof(height));
        main_item.read((char *) & tminx[ 1 ], sizeof(origin.x));
        main_item.read((char *) & tminy[ 1 ], sizeof(origin.y));
        main_item.read((char *) & tzlevel[ 1 ], sizeof(origin.z));

        main_warp.read((char *) & twidth[ 2 ], sizeof(width));
        main_warp.read((char *) & theight[ 2 ], sizeof(height));
        main_warp.read((char *) & tminx[ 2 ], sizeof(origin.x));
        main_warp.read((char *) & tminy[ 2 ], sizeof(origin.y));
        main_warp.read((char *) & tzlevel[ 2 ], sizeof(origin.z));

        all_container.read((char *) & twidth[ 3 ], sizeof(width));
        all_container.read((char *) & theight[ 3 ], sizeof(height));
        all_container.read((char *) & tminx[ 3 ], sizeof(origin.x));
        all_container.read((char *) & tminy[ 3 ], sizeof(origin.y));
        all_container.read((char *) & tzlevel[ 3 ], sizeof(origin.z));

        if ((twidth[ 0 ] == twidth[ 1 ]) && (twidth[ 1 ] == twidth[ 2 ]) && (twidth[ 2 ] == twidth[ 3 ])) {
            if ((theight[ 0 ] == theight[ 1 ]) && (theight[ 1 ] == theight[ 2 ]) && (theight[ 2 ] == theight[ 3 ])) {
                if ((tminx[ 0 ] == tminx[ 1 ]) && (tminx[ 1 ] == tminx[ 2 ]) && (tminx[ 2 ] == tminx[ 3 ])) {
                    if ((tminy[ 0 ] == tminy[ 1 ]) && (tminy[ 1 ] == tminy[ 2 ]) && (tminy[ 2 ] == tminy[ 3 ])) {
                        if ((tzlevel[ 0 ] == tzlevel[ 1 ]) && (tzlevel[ 1 ] == tzlevel[ 2 ]) && (tzlevel[ 2 ] == tzlevel[ 3 ])) {
                            origin.z = tzlevel[ 0 ];
                            uint16_t rightedge = twidth[ 0 ];
                            uint16_t lowedge = theight[ 0 ];

                            if ((rightedge <= width) && (lowedge <= height)) {

                                origin.x = tminx[ 0 ];
                                origin.y = tminy[ 0 ];

                                for (auto &c : containers) {
                                    for (auto &c2 : c.second) {
                                        delete c2.second;
                                        c2.second = nullptr;
                                    }
                                }

                                containers.clear();

                                //////////////////////////////
                                // Load the tiles and items //
                                //////////////////////////////
                                for (uint16_t x = 0; x < rightedge; ++x) {
                                    for (uint16_t y = 0; y < lowedge; ++y) {
                                        fields[ x ][ y ].Load(main_map, main_item, main_warp);
                                        fields[ x ][ y ].updateFlags();
                                    }
                                }

                                /////////////////////////
                                // Load the Containers //
                                /////////////////////////
                                unsigned long int fcount;
                                MAXCOUNTTYPE icount;
                                MAXCOUNTTYPE key;
                                MapPosition pos;
                                Container *tempc;
                                decltype(containers)::iterator conmapn;

                                all_container.read((char *) & fcount, sizeof(fcount));

                                for (unsigned long int i = 0; i < fcount; ++i) {
                                    all_container.read((char *) & pos, sizeof pos);
                                    all_container.read((char *) & icount, sizeof(icount));

                                    if (icount > 0) {
                                        conmapn = (containers.emplace(pos, Container::CONTAINERMAP())).first;

                                        for (MAXCOUNTTYPE k = 0; k < icount; ++k) {
                                            all_container.read((char *) & key, sizeof(key));

                                            Field field;

                                            if (GetCFieldAt(field, pos.x, pos.y)) {

                                                for (auto iter = field.items.begin(); iter != field.items.end(); iter++) {

                                                    if (iter->isContainer()) {
                                                        if (iter->getNumber() == key) {
                                                            tempc = new Container(iter->getId());
                                                            tempc->Load(all_container);
                                                            (*conmapn).second.insert(Container::CONTAINERMAP::value_type(key, tempc));
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

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


bool Map::GetCFieldAt(Field &fi, int16_t x, int16_t y) {

    uint16_t tempx;
    uint16_t tempy;

    try {
        tempx = Conv_X_Koord(x);
        tempy = Conv_Y_Koord(y);
    } catch (FieldNotFound &e) {
        return false;
    }

    fi = fields[ tempx ][ tempy ];

    return true;

}


void Map::age() {
    ageContainers();
    ageItems();
}

void Map::ageItems() {
    MapPosition pos;

    for (int16_t x = 0; x < width; ++x) {
        for (int16_t y = 0; y < height; ++y) {
            int8_t rotstate = fields[x][y].DoAgeItems();

            if (rotstate == -1) {
                pos.x=Conv_To_X(x);
                pos.y=Conv_To_Y(y);

                for (const auto &erased : erasedcontainers) {
                    auto conmapn = containers.find(pos);

                    if (conmapn != containers.end()) {
                        auto iterat = conmapn->second.find(erased);

                        if (iterat != conmapn->second.end()) {
                            conmapn->second.erase(iterat);
                        }
                    }
                }

                erasedcontainers.clear();

            }

            if (rotstate != 0) {
                position pos(Conv_To_X(x), Conv_To_Y(y), origin.z);
                Logger::debug(LogFacility::World) << "aged items, pos: " << pos << Log::end;
                std::vector<Player *> playersinview = World::get()->Players.findAllCharactersInScreen(pos);

                for (const auto &player : playersinview) {
                    Logger::debug(LogFacility::World) << "aged items, update needed for: " << *player << Log::end;
                    ServerCommandPointer cmd = std::make_shared<ItemUpdate_TC>(pos, fields[x][y].items);
                    player->Connection->addCommand(cmd);
                }
            }

        }
    }

}

void Map::ageContainers() {
    for (const auto &key_container : containers) {
        const auto &container = key_container.second;

        for (const auto &content : container) {
            if (content.second) {
                content.second->doAge();
            }
        }
    }
}

bool Map::SetPlayerAt(int16_t x, int16_t y, bool t) {

    Field *temp;

    if (GetPToCFieldAt(temp, x, y)) {
        temp->SetPlayerOnField(t);
        return true;
    }

    return false;
}

uint16_t Map::getHeight() const { return height; }

uint16_t Map::getWidth() const { return width; }

int16_t Map::getMinX() const { return origin.x; }

int16_t Map::getMinY() const { return origin.y; }

int16_t Map::getMaxX() const { return origin.x + width - 1; }

int16_t Map::getMaxY() const { return origin.y + height - 1; }

int16_t Map::getLevel() const { return origin.z; }

inline uint16_t Map::Conv_X_Koord(int16_t x) {
    
    uint16_t temp;
    temp = x - origin.x;

    if (temp >= width) {
        throw FieldNotFound();
    }

    return (temp);
}

inline uint16_t Map::Conv_Y_Koord(int16_t y) {

    uint16_t temp;
    temp = y - origin.y;

    if (temp >= height) {
        throw FieldNotFound();
    }

    return (temp);
}

inline int16_t Map::Conv_To_X(uint16_t x) {

    int16_t temp;
    temp = x + origin.x;

    return (temp);
}

inline int16_t Map::Conv_To_Y(uint16_t y) {

    int16_t temp;
    temp = y + origin.y;

    return (temp);
}

bool Map::findEmptyCFieldNear(Field *&cf, int16_t &x, int16_t &y) {

    int16_t startx = x;
    int16_t starty = y;

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

bool Map::intersects(const position &origin2, uint16_t width,
                     uint16_t height) const {
    return origin2.z == origin.z && getMaxX() >= origin2.x &&
           origin.x <= origin2.x + width - 1 && getMaxY() >= origin2.y &&
           origin.y <= origin2.y + height - 1;
}

bool Map::intersects(const Map &map) const {
    return map.origin.z == origin.z && getMaxX() >= map.origin.x &&
           origin.x <= map.getMaxX() && getMaxY() >= map.origin.y &&
           origin.y <= map.getMaxY();
}

