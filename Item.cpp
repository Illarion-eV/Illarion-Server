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


#include "Item.hpp"
#include <sstream>

std::string Item::getData(std::string key) {
    return data_map[key];
}


void Item::setData(std::string key, std::string value) {
    data_map[key] = value;
}


void Item::save(std::ostream *obj) {
    obj->write((char *) &id, sizeof(TYPE_OF_ITEM_ID));
    obj->write((char *) &number, sizeof(unsigned char));
    obj->write((char *) &wear, sizeof(unsigned char));
    obj->write((char *) &quality, sizeof(uint16_t));
    obj->write((char *) &data, sizeof(uint32_t));
    uint8_t mapsize = static_cast<uint8_t>(data_map.size());
    obj->write((char *) &mapsize, sizeof(uint8_t));

    for (DATA_MAP::iterator it = data_map.begin(); it != data_map.end(); ++it) {
        uint8_t sz1 = static_cast<uint8_t>(it->first.size());
        uint8_t sz2 = static_cast<uint8_t>(it->second.size());
        obj->write((char *) &sz1 , sizeof(uint8_t));
        obj->write((char *) &sz2 , sizeof(uint8_t));
        obj->write((char *) it->first.data() , sz1);
        obj->write((char *) it->second.data() , sz2);
    }
}

void Item::load(std::istream *obj) {
    obj->read((char *) &id, sizeof(TYPE_OF_ITEM_ID));
    obj->read((char *) &number, sizeof(unsigned char));
    obj->read((char *) &wear, sizeof(unsigned char));
    obj->read((char *) &quality, sizeof(uint16_t));
    obj->read((char *) &data, sizeof(uint32_t));
    uint8_t tempsize;
    obj->read((char *) &tempsize, sizeof(uint8_t));
    char readStr[255];

    for (int i = 0; i < tempsize; ++i) {
        uint8_t sz1 = 0;
        uint8_t sz2 = 0;
        obj->read((char *) &sz1, sizeof(uint8_t));
        obj->read((char *) &sz2, sizeof(uint8_t));
        obj->read((char *) readStr, sz1);
        std::string key(readStr,sz1);
        obj->read((char *) readStr, sz2);
        std::string value(readStr,sz2);
        data_map[key] = value;
    }
}
