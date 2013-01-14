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


#include "MapVector.hpp"
#include "Map.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

MapVector::MapVector() {
    lowX = 32767;
    highX = -32768;
}

void MapVector::clear() {
    maps.clear();

    lowX = 32767;
    highX = -32768;
}

bool MapVector::mapInRangeOf(short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z) const {
    short int downright_X = upperleft_X + sizex - 1;
    short int downright_Y = upperleft_Y + sizey - 1;

    for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
        if ((*thisIterator)->Z_Level == z) {
            if (((*thisIterator)->Max_X >= upperleft_X) && ((*thisIterator)->Min_X <= downright_X)) {
                if (((*thisIterator)->Max_Y >= upperleft_Y) && ((*thisIterator)->Min_Y <= downright_Y)) {
                    std::cout << "Map in range at Z:" << (*thisIterator)->Z_Level <<
                              " Min_X: " << (*thisIterator)->Min_X <<
                              " Max_X: " << (*thisIterator)->Max_X <<
                              " Min_Y: " << (*thisIterator)->Min_Y <<
                              " Max_Y: " << (*thisIterator)->Max_Y <<
                              std::endl;
                    return true;
                }// y
            }// x
        }// z
    }// iterator

    return false;

}



bool MapVector::findAllMapsInRangeOf(char rnorth, char rsouth, char reast, char rwest, position pos, MapVector::map_vector_t &ret) const {
    bool found_one = false;

    short int upperleft_X = pos.x - rwest;
    short int downright_X = pos.x + reast;

    short int upperleft_Y = pos.y - rnorth;
    short int downright_Y = pos.y + rsouth;

    for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
        if (pos.z == (*thisIterator)->Z_Level) {
            if (((*thisIterator)->Max_X >= upperleft_X) && ((*thisIterator)->Min_X <= downright_X)) {
                if (((*thisIterator)->Max_Y >= upperleft_Y) && ((*thisIterator)->Min_Y <= downright_Y)) {
                    ret.push_back(*thisIterator);
                    found_one = true;
                }// y
            }// x
        }// z
    }// iterator

    return found_one;
}



bool MapVector::findAllMapsWithXInRangeOf(short int start, short int end, MapVector::map_vector_t &ret) const {
    bool found_one = false;

    for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
        if (((*thisIterator)->Max_X >= start) && ((*thisIterator)->Min_X <= end)) {
            ret.push_back(*thisIterator);
            found_one = true;
        }// x
    }// iterator

    return found_one;
}



bool MapVector::findMapForPos(short int x, short int y, short int z, MapVector::map_t &map) const {
    for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
        if (z == (*thisIterator)->Z_Level) {
            if (((*thisIterator)->Max_X >= x) && ((*thisIterator)->Min_X <= x)) {
                if (((*thisIterator)->Max_Y >= y) && ((*thisIterator)->Min_Y <= y)) {
                    map = (*thisIterator);
                    return true;
                }// y
            }// x
        }// z
    }// iterator

    map.reset();
    return false;
}



bool MapVector::findMapForPos(position pos, MapVector::map_t &map) const {
    return findMapForPos(pos.x, pos.y, pos.z, map);
}



bool MapVector::findLowestMapOverCharacter(position pos, MapVector::map_t &lowmap) const {
    bool found_one = false;

    int ret = NOTHING;

    for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
        if ((ret > (*thisIterator)->Z_Level) && ((*thisIterator)->disappears)) {
            if ((*thisIterator)->isOverPositionInData(pos.x, pos.y, pos.z)) {
                ret = (*thisIterator)->Z_Level;
                lowmap = (*thisIterator);
                found_one = true;
            }
        }
    }// iterator

    return found_one;
}


bool MapVector::InsertMap(MapVector::map_t newMap) {
    if (newMap) {
        for (auto thisIterator = maps.begin(); thisIterator < maps.end(); ++thisIterator) {
            if ((*thisIterator) == newMap) {
                return false;
            }
        }

        if (newMap->Min_X < lowX) {
            lowX = newMap->Min_X;
        }

        if (newMap->Max_X > highX) {
            highX = newMap->Max_X;
        }

        maps.push_back(newMap);
        return true;
    }

    return false;

}

void MapVector::ageContainers() {
    for (auto it = maps.begin(); it != maps.end(); ++it) {
        (*it)->ageContainers();
    }
}

bool MapVector::exportTo(const std::string &exportDir) const {
    for (auto mapIt = maps.begin(); mapIt != maps.end(); ++mapIt) {
        int16_t minX = (*mapIt)->GetMinX();
        int16_t minY = (*mapIt)->GetMinY();
        // create base filename
        std::string filebase = exportDir + "e_" + boost::lexical_cast<std::string>(minX)
                               + "_" + boost::lexical_cast<std::string>(minY)
                               + "_" + boost::lexical_cast<std::string>((*mapIt)->Z_Level) + ".";
        // export fields file
        std::ofstream fieldsf((filebase + "tiles.txt").c_str());
        // export items file
        std::ofstream itemsf((filebase + "items.txt").c_str());
        // export warps file
        std::ofstream warpsf((filebase + "warps.txt").c_str());

        if (!fieldsf.good() || !itemsf.good() || !warpsf.good()) {
            std::cerr << "could not open output files for item export: " << std::endl;
            std::cerr << filebase << "tiles.txt" << std::endl;
            std::cerr << filebase << "items.txt" << std::endl;
            std::cerr << filebase << "warps.txt" << std::endl;
            return false;
        }

        // export tiles header
        fieldsf << "V: 2" << std::endl;
        fieldsf << "L: " << (*mapIt)->Z_Level << std::endl;
        fieldsf << "X: " << minX << std::endl;
        fieldsf << "Y: " << minY << std::endl;
        fieldsf << "W: " << (*mapIt)->GetWidth() << std::endl;
        fieldsf << "H: " << (*mapIt)->GetHeight() << std::endl;

        // iterate over the map and export...
        short int x, y;

        for (y = minY; y <= (*mapIt)->GetMaxY(); ++y) {
            for (x = minX; x <= (*mapIt)->GetMaxX(); ++x) {
                Field field;

                if ((*mapIt)->GetCFieldAt(field, x, y)) {
                    fieldsf << x-minX << ";" << y-minY << ";" << field.getTileCode() << ";" << field.getMusicId() << std::endl;

                    if (field.IsWarpField()) {
                        position target;
                        field.GetWarpField(target);
                        warpsf << x-minX << ";" << y-minY << ";" << target.x << ";" << target.y << ";" << target.z << std::endl;
                    }

                    ITEMVECTOR itemsv;
                    field.giveExportItems(itemsv);

                    for (auto it = itemsv.cbegin(); it != itemsv.cend(); ++it) {
                        itemsf << x-minX << ";" << y-minY << ";" << it->getId() << ";" << it->getQuality();

                        for (auto dataIt = it->getDataBegin(); dataIt != it->getDataEnd(); ++dataIt) {
                            using boost::algorithm::replace_all;

                            std::string key = dataIt->first;
                            std::string value = dataIt->second;

                            replace_all(key, "\\", "\\\\");
                            replace_all(key, "=", "\\=");
                            replace_all(key, ";", "\\;");
                            replace_all(value, "\\", "\\\\");
                            replace_all(value, "=", "\\=");
                            replace_all(value, ";", "\\;");

                            itemsf << ";" << key << "=" << value;
                        }

                        itemsf << std::endl;
                    }
                }
            }
        }

        fieldsf.close();
        itemsf.close();
        warpsf.close();

    }

    return true;
}

void MapVector::saveToDisk(const std::string &prefix) const {
    std::ofstream mapinitfile((prefix + "_initmaps").c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

    if (!mapinitfile.good()) {
#ifdef World_DEBUG
        std::cerr << "Could not create initmaps!" << std::endl;
#endif

    } else {
        unsigned short int size = maps.size();
        std::cout << "Saving " << size << " maps." << std::endl;
        mapinitfile.write((char *) & size, sizeof(size));
        char mname[200];

        for (auto mapI = maps.begin(); mapI != maps.end(); ++mapI) {
            mapinitfile.write((char *) & (*mapI)->Z_Level, sizeof((*mapI)->Z_Level));
            mapinitfile.write((char *) & (*mapI)->Min_X, sizeof((*mapI)->Min_X));
            mapinitfile.write((char *) & (*mapI)->Min_Y, sizeof((*mapI)->Min_Y));

            mapinitfile.write((char *) & (*mapI)->Width, sizeof((*mapI)->Width));
            mapinitfile.write((char *) & (*mapI)->Height, sizeof((*mapI)->Height));

            mapinitfile.write((char *) & (*mapI)->disappears, sizeof((*mapI)->disappears));

            sprintf(mname, "%s_%6d_%6d_%6d", prefix.c_str(), (*mapI)->Z_Level, (*mapI)->Min_X, (*mapI)->Min_Y);
            (*mapI)->Save(mname);
        }

        mapinitfile.close();
    }
}

