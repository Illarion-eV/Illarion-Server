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

#include "Container.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "World.hpp"

extern ContainerObjectTable *ContainerItems;

Container::Container(Item::id_type itemId): itemId(itemId) {
}

Container::Container(const Container &source) {
    *this = source;
}

Container &Container::operator=(const Container &source) {
    if (this != &source) {
        itemId = source.itemId;
        
        items.clear();
        items = source.items;

        if (!containers.empty()) {
            for (auto it = containers.begin(); it != containers.end(); ++it) {
                delete it->second;
                it->second = NULL;
            }

            containers.clear();
        }

        if (!source.containers.empty()) {

            for (auto it = source.containers.cbegin(); it != source.containers.cend(); ++it) {
                containers.insert(CONTAINERMAP::value_type(it->first, new Container(*(it->second))));
            }
        }

    }
    return *this;
}

Container::~Container() {
    items.clear();

    if (!containers.empty()) {

        for (auto it = containers.begin(); it != containers.end(); ++it) {
            delete it->second;
            it->second = NULL;
        }

        containers.clear();
    }
}

bool Container::InsertItem(Item it) {
    return InsertItem(it, false);
}

bool Container::InsertItem(Item item, bool merge) {
    if (items.size() < getSlotCount()) {
        if (item.isContainer()) {
            return InsertContainer(item, new Container(item.getId()));
        } else if (merge) {

            if (isItemStackable(item)) {
                auto it = items.begin();
                int temp;

                while ((it != items.end()) && (item.getNumber() > 0)) {
                    Item &selectedItem = it->second;
                    if (selectedItem.getId() == item.getId() && selectedItem.getData() == item.getData() && selectedItem.isComplete() && item.isComplete()) {
                        temp = selectedItem.getNumber() + item.getNumber();

                        if (temp <= MAXITEMS) {
                            selectedItem.setNumber(temp);
                            item.setNumber(0);
                        } else {
                            item.setNumber(item.getNumber() - MAXITEMS + selectedItem.getNumber());
                            selectedItem.setNumber(MAXITEMS);
                        }

                        selectedItem.setMinQuality(item);
                    }

                    ++it;
                }
            }

            if (item.getNumber() > 0) {
                insertIntoFirstFreeSlot(item);
            }

            return true;
        } else {
            insertIntoFirstFreeSlot(item);
            return true;
        }
    }

    return false;

}

bool Container::InsertItem(Item item, TYPE_OF_CONTAINERSLOTS pos) {
    if (pos < getSlotCount()) {
        if (item.isContainer()) {
            return InsertContainer(item, new Container(item.getId()));
        }

        Item &selectedItem = items[pos];

        if (selectedItem.getId() != 0) {
            if (isItemStackable(item)) {
                if (selectedItem.getId() == item.getId() && selectedItem.getData() == item.getData() && selectedItem.isComplete() && item.isComplete()) {
                    int temp = selectedItem.getNumber() + item.getNumber();

                    if (temp <= MAXITEMS) {
                        selectedItem.setMinQuality(item);
                        selectedItem.setNumber(temp);
                        return true;
                    } else if (items.size() < getSlotCount()) {
                        item.setNumber(item.getNumber() - MAXITEMS + selectedItem.getNumber());
                        selectedItem.setMinQuality(item);
                        selectedItem.setNumber(MAXITEMS);
                        insertIntoFirstFreeSlot(item);
                        return true;
                    }
                }
            }
        } else if (items.size() < getSlotCount()) {
            insertIntoFirstFreeSlot(item);
            return true;
        }
    }
    return false;
}

bool Container::InsertContainer(Item it, Container *cc) {
    if ((this != cc) && (items.size() < getSlotCount())) {
        Item titem = it;

        if (items.size() < MAXITEMS) {   // es ist noch Platz frei
            CONTAINERMAP::iterator iterat;
            CONTAINERMAP::key_type count = 0;
            // freie Container-ID finden
            iterat = containers.find(0);

            while ((iterat != containers.end()) && (count < (MAXITEMS - 1))) {
                count = count + 1;
                iterat = containers.find(count);
            }

            // dem Container seine neue ID zuweisen
            titem.setNumber(count);
            // den Container in die Itemliste einf�gen
            insertIntoFirstFreeSlot(titem);
            // den Inhalt des Containers in die Containermap mit der entsprechenden ID einf�gen
            containers.insert(CONTAINERMAP::value_type(count, cc));
            return true;
        }
    }
    return false;

}

bool Container::changeQuality(Item::id_type id, short int amount) {
    short int tmpQuality;

    auto it = items.begin();
    while (it != items.end()) {
        Item &item = it->second;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {

                if ((*iterat).second->changeQuality(id, amount)) {
                    return true;
                }
            }

            ++it;
        } else if (item.getId() == id) {
            tmpQuality = ((amount+item.getDurability())<100) ? (amount + item.getQuality()) : (item.getQuality()-item.getDurability() + 99);

            if (tmpQuality%100 > 1) {
                item.setQuality(tmpQuality);
                return true;
            } else {
                items.erase(it);
                return true;
            }
        } else {
            ++it;
        }
    }

    return false;
}

bool Container::changeQualityAt(TYPE_OF_CONTAINERSLOTS nr, short int amount) {
    Item &item = items[nr];
    if (item.getId() != 0) {
        Item::quality_type tmpQuality = ((amount+item.getDurability())<100) ? (amount + item.getQuality()) : (item.getQuality() - item.getDurability() + 99);

        if (tmpQuality%100 > 1) {
            item.setQuality(tmpQuality);
            return true;
        } else {
            if (item.isContainer()) {
                auto iterat = containers.find(item.getNumber());

                if (iterat != containers.end()) {
                    containers.erase(iterat);
                }
            }

            item.reset();
            return true;
        }
    }

    return false;
}

bool Container::TakeItemNr(TYPE_OF_CONTAINERSLOTS nr, Item &item, Container* &cc, Item::number_type count) {
    Item &selectedItem = items[nr];
    if (selectedItem.getId() != 0) {
        item = selectedItem;

        if (item.isContainer()) {
            selectedItem.reset();
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                cc = (*iterat).second;
                containers.erase(iterat);
            } else {
                cc = new Container(item.getId());
            }

            return true;

        } else {
            cc = NULL;

            if (isItemStackable(item) && count > 1) {
                if (selectedItem.getNumber() > count) {
                    selectedItem.setNumber(selectedItem.getNumber() - count);
                    item.setNumber(count);
                } else {
                    selectedItem.reset();
                }
            } else {
                if (selectedItem.getNumber() > 1) {
                    selectedItem.setNumber(selectedItem.getNumber() - 1);
                    item.setNumber(1);
                } else {
                    selectedItem.reset();
                }
            }
        }

        return true;
    } else {
        item.reset();
        cc = NULL;
        return false;
    }
}

luabind::object Container::getItemList() {
    lua_State *_luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);
    int index = 1;
    MAXCOUNTTYPE pos = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {

        ScriptItem item = it->second;
        item.type = ScriptItem::it_container;
        item.itempos = pos;
        item.inside = this;
        list[index] = item;
        ++index;
        ++pos;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                iterat->second->increaseItemList(list, index);
            }
        }

    }

    return list;
}

luabind::object Container::getItemList(Item::id_type itemid) {
    lua_State *_luaState = World::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);
    int index = 1;
    MAXCOUNTTYPE pos = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        
        if (item.getId() == itemid) {
            ScriptItem item = it->second;
            item.type = ScriptItem::it_container;
            item.itempos = pos;
            item.inside = this;
            list[index] = item;
            index++;
        }

        ++pos;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                iterat->second->increaseItemList(itemid , list, index);
            }
        }

    }

    return list;
}

void Container::increaseItemList(Item::id_type itemid, luabind::object &list, int &index) {
    MAXCOUNTTYPE pos = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        
        if (item.getId() == itemid) {
            ScriptItem item = it->second;
            item.type = ScriptItem::it_container;
            item.itempos = pos;
            item.inside = this;
            list[index] = item;
            index++;
        }

        ++pos;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                iterat->second->increaseItemList(itemid , list, index);
            }
        }

    }
}

void Container::increaseItemList(luabind::object &list, int &index) {
    MAXCOUNTTYPE pos = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {

        ScriptItem item = it->second;
        item.type = ScriptItem::it_container;
        item.itempos = pos;
        item.inside = this;
        list[index] = item;
        ++index;
        ++pos;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                // rekursiv verarbeiten
                iterat->second->increaseItemList(list, index);
            }
        }

    }
}

bool Container::viewItemNr(TYPE_OF_CONTAINERSLOTS nr, ScriptItem &item, Container* &cc) {
    Item &selectedItem = items[nr];

    if (selectedItem.getId() != 0) {
        item = selectedItem;
        item.type = ScriptItem::it_container;
        item.itempos = nr;
        item.inside = this;

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                cc = iterat->second;
            } else {
                cc = new Container(item.getId());
            }
        } else {
            cc = NULL;
        }

        return true;
    } else {
        item.reset();
        cc = NULL;
        return false;
    }
}

int Container::increaseAtPos(unsigned char pos, Item::number_type count) {
    Item &item = items[pos];
    int temp = 0;

    if (item.getId() != 0) {
        if (item.isContainer()) {
            return count;
        } else {
            temp = item.getNumber() + count;

            if (temp > 255) {
                item.setNumber(255);
                temp = temp - 255;
            } else if (temp <= 0) {
                temp = count + item.getNumber();
                item.reset();
            } else {
                item.setNumber(temp);
                temp = 0;
            }
        }
    }

    return temp;
}

bool Container::changeItem(ScriptItem &item) {
    MAXCOUNTTYPE tcount = 0;
    auto it = items.begin();

    while ((it != items.end()) && (tcount != item.itempos)) {
        ++tcount;
        ++it;
    }

    if (it != items.end()) {
        if (!it->second.isContainer()) {
            it->second = item;
            return true;
        }
    }

    return false;
}

bool Container::swapAtPos(unsigned char pos, Item::id_type newid, Item::quality_type newQuality) {
    Item &item = items[pos];
    if (item.getId() != 0) {
        if (!item.isContainer()) {
            item.setId(newid);

            if (newQuality > 0) {
                item.setQuality(newQuality);
            }

            return true;
        }
    }

    return false;
}

void Container::Save(std::ofstream *where) {
    MAXCOUNTTYPE size = items.size();
    where->write((char *) & size, sizeof(size));

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        where->write((char *) &(it->first), sizeof(TYPE_OF_CONTAINERSLOTS));
        where->write((char *) &item, sizeof(Item));

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                (*iterat).second->Save(where);
            } else {
                size = 0;
                where->write((char *) & size, sizeof(size));
            }
        }
    }
}

void Container::Load(std::istream *where) {

    if (!containers.empty()) {

        for (auto it = containers.begin(); it != containers.end(); ++it) {
            delete it->second;
            it->second = NULL;
        }
    }

    items.clear();
    containers.clear();

    MAXCOUNTTYPE size;
    where->read((char *) & size, sizeof(size));

    Container *tempc;

    TYPE_OF_CONTAINERSLOTS slot;
    Item tempi;

    for (int i = 0; i < size; ++i) {
        where->read((char *) & slot, sizeof(TYPE_OF_CONTAINERSLOTS));
        where->read((char *) & tempi, sizeof(tempi));

        if (tempi.isContainer()) {
            tempc = new Container(tempi.getId());
            tempc->Load(where);
            InsertContainer(tempi, tempc);
        } else {
            InsertItem(tempi, slot);
        }
    }
}

int Container::countItem(Item::id_type itemid) {
    int temp = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        if (item.getId() == itemid && item.isComplete()) {
            temp = temp + item.getNumber();
        }

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                temp = temp + iterat->second->countItem(itemid);
            }
        }
    }

    return temp;
}

int Container::countItem(Item::id_type itemid, Item::data_type data) {
    int temp = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        if (item.getId() == itemid && item.getData() == data && item.isComplete()) {
            temp = temp + item.getNumber();
        }

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                temp = temp + iterat->second->countItem(itemid, data);
            }
        }
    }

    return temp;
}

int Container::weight(int rekt) {
    int temprekt = rekt + 1;

    if (rekt > MAXIMALEREKURSIONSTIEFE) {
        throw RekursionException();
    }

    uint32_t temp = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        if (!CommonItems->find(item.getId(), tempCommon)) {
            tempCommon.Weight = 0;
        }

        if (item.isContainer()) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                temp += iterat->second->weight(temprekt);
            }

            temp += tempCommon.Weight;
        } else {
            temp += (tempCommon.Weight * item.getNumber());
        }
    }

    if (temp > 30000) {
        return 30000;
    } else {
        return temp;
    }
}

int Container::_eraseItem(Item::id_type itemid, Item::number_type count, Item::data_type data, bool useData) {
    int temp = count;

    auto it = items.begin();

    while (it != items.end()) {
        Item &item = it->second;
        if (item.isContainer() && (temp > 0)) {
            auto iterat = containers.find(item.getNumber());

            if (iterat != containers.end()) {
                temp = (*iterat).second->_eraseItem(itemid, temp, data, useData);
            }
            ++it;
        } else if ((item.getId() == itemid && (!useData || item.getData() == data) && item.isComplete()) && (temp > 0)) {

            if (temp >= item.getNumber()) {
                temp = temp - item.getNumber();
                item.reset();

            } else {
                item.setNumber(item.getNumber() - temp);
                temp = 0;
                ++it;
            }
        } else {
            ++it;
        }
    }

    return temp;
}

int Container::eraseItem(Item::id_type itemid, Item::number_type count) {
    return _eraseItem(itemid, count, 0, false);
}

int Container::eraseItem(Item::id_type itemid, Item::number_type count, Item::data_type data) {
    return _eraseItem(itemid, count, data, true);
}

void Container::doAge(bool inventory) {
    if (!items.empty()) {
        auto it = items.begin();

        while (it != items.end()) {
            Item &item = it->second;
            if (!CommonItems->find(item.getId(), tempCommon)) {
                tempCommon.ObjectAfterRot = item.getId();
                tempCommon.rotsInInventory = false;
            }

            if (!inventory || (inventory && tempCommon.rotsInInventory)) {
                if (!item.survivesAging()) {
                    tempCommon.ObjectAfterRot = item.getId();

                    if (item.getId() != tempCommon.ObjectAfterRot) {
                        item.setId(tempCommon.ObjectAfterRot);

                        if (CommonItems->find(tempCommon.ObjectAfterRot, tempCommon)) {
                            item.setWear(tempCommon.AgingSpeed);
                        }

                        ++it;
                    } else {

                        if (item.isContainer()) {
                            auto iterat = containers.find(item.getNumber());

                            if (iterat != containers.end()) {
                                containers.erase(iterat);
                            }
                        }

                        item.reset();
                    }
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    if (!containers.empty()) {
        for (auto it = containers.begin(); it != containers.end(); ++it) {
            it->second->doAge(inventory);
        }
    }

}

TYPE_OF_CONTAINERSLOTS Container::getSlotCount() {
    return ContainerItems->find(itemId);
}

bool Container::isItemStackable(Item item) {
    CommonStruct com;

    if (CommonItems->find(item.getId(), com)) {
        return com.isStackable;
    } else {
        std::cerr<<"Item mit folgender id konnte nicht gefunden werden in Funktion isItemStackable(Item item): "<<item.getId()<<" !" <<std::endl;
        return false;
    }

    return false;
}

void Container::insertIntoFirstFreeSlot(Item &item) {
    uint32_t firstFreeSlot = getSlotCount();
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->second.getId() == 0 && it->first < firstFreeSlot) {
            firstFreeSlot = it->first;
        }
    }

    if (firstFreeSlot < getSlotCount()) {
        items[firstFreeSlot] = item;
    }
}

