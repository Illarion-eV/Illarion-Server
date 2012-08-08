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
#include "data/CommonObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include <sstream>

extern CommonObjectTable *CommonItems;
extern ContainerObjectTable *ContainerItems;

/*
number_type Item::increaseNumber(number_type number) {

}


number_type Item::reduceNumber(number_type number) {

}
*/

void Item::setMinQuality(const Item &item) {
    quality_type minQuality = (quality < item.quality) ? quality : item.quality;
    minQuality /= 100;
    quality_type minDurability = (getDurability() < item.getDurability()) ? getDurability() : item.getDurability();
    quality = minQuality * 100 + minDurability;
}


std::string Item::getData(std::string key) {
    return datamap[key];
}


void Item::setData(std::string key, std::string value) {
    datamap[key] = value;
}


void Item::setData(std::string key, int32_t value) {
    std::stringstream ss;
    ss << value;
    setData(key, ss.str());
}

void Item::reset() {
    id = 0;
    number = 0;
    wear = 0;
    quality = 333;
    data = 0;
    datamap.clear();
}


void Item::resetWear() {
    CommonStruct common;

    if (CommonItems->find(id, common)) {
        if (!common.rotsInInventory && common.AgingSpeed > wear) {
            wear = common.AgingSpeed;
        }
    }
}


void Item::save(std::ostream *obj) const {
    obj->write((char *) &id, sizeof(id_type));
    obj->write((char *) &number, sizeof(number_type));
    obj->write((char *) &wear, sizeof(wear_type));
    obj->write((char *) &quality, sizeof(quality_type));
    obj->write((char *) &data, sizeof(data_type));
    uint8_t mapsize = static_cast<uint8_t>(datamap.size());
    obj->write((char *) &mapsize, sizeof(uint8_t));

    for (auto it = datamap.begin(); it != datamap.end(); ++it) {
        uint8_t sz1 = static_cast<uint8_t>(it->first.size());
        uint8_t sz2 = static_cast<uint8_t>(it->second.size());
        obj->write((char *) &sz1 , sizeof(uint8_t));
        obj->write((char *) &sz2 , sizeof(uint8_t));
        obj->write((char *) it->first.data() , sz1);
        obj->write((char *) it->second.data() , sz2);
    }
}


void Item::load(std::istream *obj) {
    obj->read((char *) &id, sizeof(id_type));
    obj->read((char *) &number, sizeof(number_type));
    obj->read((char *) &wear, sizeof(wear_type));
    obj->read((char *) &quality, sizeof(quality_type));
    obj->read((char *) &data, sizeof(data_type));
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
        datamap[key] = value;
    }
}


bool Item::survivesAging() {
    if (wear != 255 && wear != 0) {
        --wear;
    }

    return wear > 0;
}

bool Item::isContainer() const {
    return ContainerItems->find(id);
}

TYPE_OF_WEIGHT Item::getWeight() const {
    CommonStruct common;

    if (CommonItems->find(id, common)) {
        return common.Weight;
    }

    return 0;
}

TYPE_OF_WORTH Item::getWorth() const {
    CommonStruct common;

    if (CommonItems->find(id, common)) {
        return common.Worth;
    }

    return 0;
}

bool Item::isComplete() const {
    return quality >= 100;
}

bool Item::isPermanent() const {
    return wear == PERMANENT_WEAR;
}

void Item::makePermanent() {
    wear = PERMANENT_WEAR;
}
