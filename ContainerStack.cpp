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


#include "ContainerStack.hpp"

ContainerStack::ContainerStack() {}


ContainerStack::ContainerStack(const ContainerStack &source) {
    opencontainers = source.opencontainers;
}


ContainerStack &ContainerStack:: operator =(const ContainerStack &source) {
    if (this != &source) {
        opencontainers = source.opencontainers;
    }

    return *this;
}


ContainerStack::~ContainerStack() {}


void ContainerStack::startContainer(Container *cc, bool carry) {
    clear();
    openContainer(cc);
    inventory = carry;
}


void ContainerStack::openContainer(Container *cc) {
    opencontainers.push_back(cc);
}


bool ContainerStack::closeContainer() {
    if (opencontainers.empty()) {
        return false;
    } else {
        opencontainers.pop_back();

        if (!opencontainers.empty()) {
            return true;
        } else {
            inventory = false;
            return false;
        }
    }
}



bool ContainerStack::isOnTop(Container *cc) {
    if (opencontainers.empty()) {
        return false;
    } else {
        return (opencontainers.back() == cc);
    }
}



Container *ContainerStack::top() {
    if (opencontainers.empty()) {
        return NULL;
    } else {
        return opencontainers.back();
    }
}



bool ContainerStack::contains(Container *cc) {
    CONTAINERVECTOR::iterator theIterator;

    for (theIterator = opencontainers.begin(); theIterator < opencontainers.end(); ++theIterator) {
        if ((*theIterator) == cc) {
            return true;
        }
    }

    return false;
}



void ContainerStack::clear() {
    opencontainers.clear();
    inventory = false;
}



bool ContainerStack::inInventory() {
    return inventory;
}
