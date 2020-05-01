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

#include "Container.hpp"
#include "data/Data.hpp"
#include "World.hpp"

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
                it->second = nullptr;
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
            it->second = nullptr;
        }

        containers.clear();
    }
}

Item::number_type Container::mergeItem(Item item) {
    if (isItemStackable(item)) {
        auto it = items.begin();

        while ((it != items.end()) && (item.getNumber() > 0)) {
            Item &selectedItem = it->second;

            if (selectedItem.getId() == item.getId() && selectedItem.equalData(item)) {
                Item::number_type number = selectedItem.increaseNumberBy(item.getNumber());

                if (number != item.getNumber()) {
                    item.setNumber(number);
                    selectedItem.setMinQuality(item);
                }
            }

            ++it;
        }
    }

    return item.getNumber();
}

bool Container::InsertItem(const Item &item) {
    return InsertItem(item, true);
}

bool Container::InsertItem(Item item, bool merge) {
    if (items.size() < getSlotCount()) {
        if (item.isContainer()) {
            return InsertContainer(item, new Container(item.getId()));
        }

        if (merge) {
            auto leftOver = mergeItem(item);

            if (leftOver > 0) {
                item.setNumber(leftOver);
                insertIntoFirstFreeSlot(item);
            }
        } else {
            insertIntoFirstFreeSlot(item);
        }

        return true;
    }

    return false;

}

bool Container::InsertItem(Item item, TYPE_OF_CONTAINERSLOTS pos) {
    if (pos < getSlotCount()) {
        if (item.isContainer()) {
            return InsertContainer(item, new Container(item.getId()), pos);
        }

        auto it = items.find(pos);

        if (it != items.end()) {
            Item &selectedItem = it->second;

            if (isItemStackable(item)) {
                if (selectedItem.getId() == item.getId() && selectedItem.equalData(item)) {
                    int temp = selectedItem.getNumber() + item.getNumber();

                    auto maxStack = item.getMaxStack();

                    if (temp <= maxStack) {
                        selectedItem.setMinQuality(item);
                        selectedItem.setNumber(temp);
                        return true;
                    } else if (items.size() < getSlotCount()) {
                        item.setNumber(item.getNumber() - maxStack + selectedItem.getNumber());
                        selectedItem.setMinQuality(item);
                        selectedItem.setNumber(maxStack);
                        insertIntoFirstFreeSlot(item);
                        return true;
                    }
                }
            }
        } else if (items.size() < getSlotCount()) {
            items.insert(ITEMMAP::value_type(pos, item));
            return true;
        }
    }

    return false;
}

bool Container::InsertContainer(const Item &item, Container *cc) {
    if ((this != cc) && (items.size() < getSlotCount())) {
        Item titem = item;
        insertIntoFirstFreeSlot(titem, cc);
        return true;
    }

    return false;

}

bool Container::InsertContainer(const Item &item, Container *cc, TYPE_OF_CONTAINERSLOTS pos) {
    if ((this != cc) && (pos < getSlotCount())) {
        Item titem = item;

        auto iterat = items.find(pos);

        if (iterat != items.end()) {
            return InsertContainer(item, cc);
        } else {
            items.insert(ITEMMAP::value_type(pos, titem));
            containers.insert(CONTAINERMAP::value_type(pos, cc));
            World::get()->sendContainerSlotChange(this, pos);
            return true;
        }
    }

    return false;

}

bool Container::changeQualityAt(TYPE_OF_CONTAINERSLOTS nr, short int amount) {
    auto it = items.find(nr);

    if (it != items.end()) {
        Item &item = it->second;
        Item::quality_type tmpQuality = ((amount+item.getDurability())<100) ? (amount + item.getQuality()) : (item.getQuality() - item.getDurability() + 99);

        if (tmpQuality%100 > 1) {
            item.setQuality(tmpQuality);
            return true;
        } else {
            if (item.isContainer()) {
                auto iterat = containers.find(nr);

                if (iterat != containers.end()) {
                    containers.erase(iterat);
                }
            }

            items.erase(nr);
            return true;
        }
    }

    return false;
}

bool Container::TakeItemNr(TYPE_OF_CONTAINERSLOTS nr, Item &item, Container *&cc, Item::number_type count) {
    auto it = items.find(nr);

    if (it != items.end()) {
        Item &selectedItem = it->second;
        item = selectedItem;

        if (item.isContainer()) {
            items.erase(nr);
            auto iterat = containers.find(nr);

            if (iterat != containers.end()) {
                cc = (*iterat).second;
                containers.erase(iterat);
            } else {
                cc = new Container(item.getId());
            }

            return true;

        } else {
            cc = nullptr;

            if (isItemStackable(item) && count > 1) {
                if (selectedItem.getNumber() > count) {
                    selectedItem.setNumber(selectedItem.getNumber() - count);
                    item.setNumber(count);
                } else {
                    items.erase(nr);
                }
            } else {
                if (selectedItem.getNumber() > 1) {
                    selectedItem.setNumber(selectedItem.getNumber() - 1);
                    item.setNumber(1);
                } else {
                    items.erase(nr);
                }
            }
        }

        return true;
    } else {
        items.erase(nr);
        cc = nullptr;
        return false;
    }
}

std::vector<ScriptItem> Container::getItemList() {
    std::vector<ScriptItem> list;

    for (auto it = items.begin(); it != items.end(); ++it) {

        ScriptItem item(it->second);
        item.type = ScriptItem::it_container;
        item.itempos = it->first;
        item.inside = this;
        list.push_back(item);

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                iterat->second->addContentToList(list);
            }
        }

    }

    return list;
}

std::vector<ScriptItem> Container::getItemList(Item::id_type itemid) {
    std::vector<ScriptItem> list;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;

        if (item.getId() == itemid) {
            ScriptItem item(it->second);
            item.type = ScriptItem::it_container;
            item.itempos = it->first;
            item.inside = this;
            list.push_back(item);
        }

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                iterat->second->addContentToList(itemid , list);
            }
        }

    }

    return list;
}

void Container::addContentToList(Item::id_type itemid, std::vector<ScriptItem> &list) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        const Item &item = it->second;

        if (item.getId() == itemid) {
            ScriptItem item(it->second);
            item.type = ScriptItem::it_container;
            item.itempos = it->first;
            item.inside = this;
            list.push_back(item);
        }

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                iterat->second->addContentToList(itemid, list);
            }
        }

    }
}

void Container::addContentToList(std::vector<ScriptItem> &list) {
    for (auto it = items.begin(); it != items.end(); ++it) {

        ScriptItem item(it->second);
        item.type = ScriptItem::it_container;
        item.itempos = it->first;
        item.inside = this;
        list.push_back(item);

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                iterat->second->addContentToList(list);
            }
        }

    }
}

bool Container::viewItemNr(TYPE_OF_CONTAINERSLOTS nr, ScriptItem &item, Container *&cc) {
    auto it = items.find(nr);

    if (it != items.end()) {
        item = (ScriptItem)it->second;
        item.type = ScriptItem::it_container;
        item.itempos = nr;
        item.inside = this;

        if (item.isContainer()) {
            auto iterat = containers.find(nr);

            if (iterat != containers.end()) {
                cc = iterat->second;
            } else {
                cc = new Container(item.getId());
            }
        } else {
            cc = nullptr;
        }

        return true;
    } else {
        items.erase(nr);
        cc = nullptr;
        return false;
    }
}

int Container::increaseAtPos(unsigned char pos, int count) {
    auto it = items.find(pos);
    int temp = 0;

    if (it != items.end()) {
        Item &item = it->second;

        if (item.isContainer()) {
            return count;
        } else {
            temp = item.getNumber() + count;

            auto maxStack = item.getMaxStack();

            if (temp > maxStack) {
                item.setNumber(maxStack);
                temp = temp - maxStack;
            } else if (temp <= 0) {
                temp = count + item.getNumber();
                items.erase(pos);
            } else {
                item.setNumber(temp);
                temp = 0;
            }
        }
    }

    return temp;
}

bool Container::changeItem(ScriptItem &item) {
    auto it = items.find(item.itempos);

    if (it != items.end()) {
        if (!it->second.isContainer()) {
            it->second = item;
            return true;
        }
    }

    return false;
}

bool Container::swapAtPos(unsigned char pos, Item::id_type newid, Item::quality_type newQuality) {
    auto it = items.find(pos);

    if (it != items.end()) {
        Item &item = it->second;

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

void Container::Save(std::ofstream &where) {
    MAXCOUNTTYPE size = items.size();
    where.write((char *) & size, sizeof(size));

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;
        where.write((char *) &(it->first), sizeof(TYPE_OF_CONTAINERSLOTS));
        where.write((char *) &item, sizeof(Item));

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                (*iterat).second->Save(where);
            } else {
                size = 0;
                where.write((char *) & size, sizeof(size));
            }
        }
    }
}

void Container::Load(std::istream &where) {

    if (!containers.empty()) {

        for (auto it = containers.begin(); it != containers.end(); ++it) {
            delete it->second;
            it->second = nullptr;
        }
    }

    items.clear();
    containers.clear();

    MAXCOUNTTYPE size;
    where.read((char *) & size, sizeof(size));

    Container *tempc;

    TYPE_OF_CONTAINERSLOTS slot;
    Item tempi;

    for (int i = 0; i < size; ++i) {
        where.read((char *) & slot, sizeof(TYPE_OF_CONTAINERSLOTS));
        where.read((char *) & tempi, sizeof(tempi));

        if (tempi.isContainer()) {
            tempc = new Container(tempi.getId());
            tempc->Load(where);
            InsertContainer(tempi, tempc, slot);
        } else {
            InsertItem(tempi, slot);
        }
    }
}

int Container::countItem(Item::id_type itemid, script_data_exchangemap const *data) const {
    int temp = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        const Item &item = it->second;

        if (item.getId() == itemid && (data == nullptr || item.hasData(*data))) {
            temp = temp + item.getNumber();
        }

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                temp = temp + iterat->second->countItem(itemid, data);
            }
        }
    }

    return temp;
}

int Container::weight() {
    return recursiveWeight(0);
}

int Container::recursiveWeight(int rekt) {
    int temprekt = rekt + 1;

    if (rekt > MAXIMALEREKURSIONSTIEFE) {
        throw RecursionException();
    }

    uint32_t temp = 0;

    for (auto it = items.begin(); it != items.end(); ++it) {
        Item &item = it->second;

        const auto &itemStruct = Data::Items[item.getId()];

        if (item.isContainer()) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                temp += iterat->second->recursiveWeight(temprekt);
            }

            temp += itemStruct.Weight;
        } else {
            temp += (itemStruct.Weight * item.getNumber());
        }
    }

    if (temp > 30000) {
        return 30000;
    } else {
        return temp;
    }
}

int Container::eraseItem(Item::id_type itemid, Item::number_type count, script_data_exchangemap const *data) {

    int temp = count;

    auto it = items.begin();

    while (it != items.end()) {
        Item &item = it->second;

        if (item.isContainer() && (temp > 0)) {
            auto iterat = containers.find(it->first);

            if (iterat != containers.end()) {
                temp = (*iterat).second->eraseItem(itemid, temp, data);
            }

            ++it;
        } else if ((item.getId() == itemid && (data == nullptr || item.hasData(*data))) && (temp > 0)) {

            if (temp >= item.getNumber()) {
                temp = temp - item.getNumber();
                it = items.erase(it);

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

void Container::doAge(bool inventory) {
    if (!items.empty()) {
        auto it = items.begin();

        while (it != items.end()) {
            Item &item = it->second;

            const auto &itemStruct = Data::Items[item.getId()];

            if (!inventory || itemStruct.rotsInInventory) {
                if (!item.survivesAgeing()) {
                    if (item.getId() != itemStruct.ObjectAfterRot) {
                        item.setId(itemStruct.ObjectAfterRot);

                        const auto &afterRotItemStruct = Data::Items[itemStruct.ObjectAfterRot];

                        if (afterRotItemStruct.isValid()) {
                            item.setWear(itemStruct.AgeingSpeed);
                        }

                        ++it;
                    } else {

                        if (item.isContainer()) {
                            auto iterat = containers.find(it->first);

                            if (iterat != containers.end()) {
                                containers.erase(iterat);
                            }
                        }

                        it = items.erase(it);
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

void Container::resetWear() {
    for (auto &item : items) {
        item.second.resetWear();
    }

    for (auto &container : containers) {
        container.second->resetWear();
    }
}

TYPE_OF_CONTAINERSLOTS Container::getSlotCount() const {
    return Data::ContainerItems[itemId];
}

bool Container::isItemStackable(const Item &item) {
    const auto &com = Data::Items[item.getId()];
    return com.MaxStack > 1;
}

void Container::insertIntoFirstFreeSlot(Item &item) {
    TYPE_OF_CONTAINERSLOTS freeSlot = getFirstFreeSlot();
    TYPE_OF_CONTAINERSLOTS slotCount = getSlotCount();

    if (freeSlot < slotCount) {
        items.insert(ITEMMAP::value_type(freeSlot, item));
        World::get()->sendContainerSlotChange(this, freeSlot);
    }
}

void Container::insertIntoFirstFreeSlot(Item &item, Container *container) {
    TYPE_OF_CONTAINERSLOTS freeSlot = getFirstFreeSlot();
    TYPE_OF_CONTAINERSLOTS slotCount = getSlotCount();

    if (freeSlot < slotCount) {
        items.insert(ITEMMAP::value_type(freeSlot, item));
        containers.insert(CONTAINERMAP::value_type(freeSlot, container));
        World::get()->sendContainerSlotChange(this, freeSlot);
    }
}

TYPE_OF_CONTAINERSLOTS Container::getFirstFreeSlot() const {
    TYPE_OF_CONTAINERSLOTS slotCount = getSlotCount();
    TYPE_OF_CONTAINERSLOTS i = 0;

    while (i < slotCount && items.find(i) != items.end()) {
        ++i;
    }

    return i;
}
