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
#include <boost/lexical_cast.hpp>

extern CommonObjectTable *CommonItems;
extern ContainerObjectTable *ContainerItems;

Item::Item(id_type id, number_type number, wear_type wear, quality_type quality, const luabind::object &datamap):
    id(id), number(number), wear(wear), quality(quality), datamap(1) {
    setData(datamap);
}

Item::number_type Item::increaseNumberBy(Item::number_type count) {
    const CommonStruct &common = CommonItems->find(id);

    if (common.isValid()) {
        count += getNumber();

        if (count >= common.MaxStack) {
            setNumber(common.MaxStack);
            count -= common.MaxStack;
        } else {
            setNumber(count);
            count = 0;
        }
    }

    return count;
}

void Item::setMinQuality(const Item &item) {
    quality_type minQuality = (quality < item.quality) ? quality : item.quality;
    minQuality /= 100;
    quality_type minDurability = (getDurability() < item.getDurability()) ? getDurability() : item.getDurability();
    quality = minQuality * 100 + minDurability;
}

void Item::setData(const luabind::object &datamap) {
    using namespace luabind;
    auto mapType = type(datamap);

    if (mapType == LUA_TTABLE) {
        for (iterator it(datamap), end; it != end; ++it) {
            std::string key;

            try {
                key = object_cast<std::string>(it.key());
            } catch (cast_failed &e) {
                throw std::logic_error("Usage of invalid data map key. Data map keys must be strings.");
            }

            try {
                std::string value = object_cast<std::string>(*it);
                setData(key, value);
            } catch (cast_failed &e) {
                try {
                    int32_t intValue = object_cast<int32_t>(*it);
                    setData(key, intValue);
                } catch (cast_failed &e) {
                    throw std::logic_error("Usage of invalid data map value. Data map values must be numbers or strings.");
                }
            }
        }
    } else if (mapType == LUA_TNIL) {
        this->datamap.clear();
    } else {
        throw std::logic_error("Usage of invalid data map type. Data maps must be tables or nil.");
    }
}


bool Item::hasData(const luabind::object &datamap) {
    using namespace luabind;
    auto mapType = type(datamap);

    if (mapType == LUA_TTABLE) {
        bool isSameData = true;

        iterator it(datamap), end;

        if (it == end) {
            return hasNoData();
        }

        for (; it != end && isSameData; ++it) {
            std::string key;

            try {
                key = object_cast<std::string>(it.key());
            } catch (cast_failed &e) {
                throw std::logic_error("Usage of invalid data map key. Data map keys must be strings.");
            }

            std::string value;

            try {
                value = object_cast<std::string>(*it);
            } catch (cast_failed &e) {
                try {
                    int32_t intValue = object_cast<int32_t>(*it);
                    std::stringstream ss;
                    ss << intValue;
                    value = ss.str();
                } catch (cast_failed &e) {
                    throw std::logic_error("Usage of invalid data map value. Data map values must be numbers or strings.");
                }
            }

            isSameData = (getData(key) == value);
        }

        return isSameData;
    } else if (mapType != LUA_TNIL) {
        throw std::logic_error("Usage of invalid data map type. Data maps must be tables or nil.");
    }

    return true;
}

bool Item::hasNoData() const {
    return datamap.size() == 0;
}

std::string Item::getData(std::string key) {
    return datamap[key];
}


void Item::setData(std::string key, std::string value) {
    if (value.length() > 0) {
        datamap[key] = value;
    } else {
        datamap.erase(key);
    }
}


void Item::setData(std::string key, int32_t value) {
    std::stringstream ss;
    ss << value;
    setData(key, ss.str());
}

uint16_t Item::getDepot() {
    uint16_t depotId;

    try {
        depotId = boost::lexical_cast<uint16_t>(getData("depot"));
    } catch (boost::bad_lexical_cast) {
        depotId = 1;
    }

    return depotId;
}

void Item::reset() {
    id = 0;
    number = 0;
    wear = 0;
    quality = 333;
    datamap.clear();
}


void Item::resetWear() {
    const CommonStruct &common = CommonItems->find(id);

    if (common.isValid()) {
        if (!common.rotsInInventory && common.AgeingSpeed > wear) {
            wear = common.AgeingSpeed;
        }
    }
}


void Item::save(std::ostream *obj) const {
    obj->write((char *) &id, sizeof(id_type));
    obj->write((char *) &number, sizeof(number_type));
    obj->write((char *) &wear, sizeof(wear_type));
    obj->write((char *) &quality, sizeof(quality_type));
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


bool Item::survivesAgeing() {
    if (wear != 255 && wear != 0) {
        --wear;
    }

    return wear > 0;
}

bool Item::isContainer() const {
    return ContainerItems->find(id);
}

TYPE_OF_WEIGHT Item::getWeight() const {
    const CommonStruct &common = CommonItems->find(id);

    if (common.isValid()) {
        return common.Weight * number;
    }

    return 0;
}

TYPE_OF_WORTH Item::getWorth() const {
    const CommonStruct &common = CommonItems->find(id);

    if (common.isValid()) {
        return common.Worth * number;
    }

    return 0;
}

Item::number_type Item::getMaxStack() const {
    const CommonStruct &common = CommonItems->find(id);;

    if (common.isValid()) {
        return common.MaxStack;
    }

    return 0;
}

bool Item::isPermanent() const {
    return wear == PERMANENT_WEAR;
}

void Item::makePermanent() {
    wear = PERMANENT_WEAR;
}
