/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Attribute.hpp"

Attribute::Attribute() {
    value = 0;
    baseValue = 0;
    maximum = 0;
}

Attribute::Attribute(attribute_t value) {
    this->value = value;
    baseValue = value;
    maximum = 0;
}

Attribute::Attribute(attribute_t value, attribute_t maximum) {
    this->value = value;
    baseValue = value;
    this->maximum = maximum;
}

void Attribute::setValue(attribute_t value) {
    if (value > maximum && maximum != 0) {
        this->value = maximum;
    } else {
        this->value = value;
    }

    if (baseValue == 0) {
        baseValue = this->value;
    }
}

auto Attribute::getValue() const -> attribute_t {
    return value;
}

void Attribute::increaseValue(int amount) {
    int newValue = value + amount;

    if (newValue < 0) {
        value = 0;
    } else if (newValue > maximum && maximum != 0) {
        value = maximum;
    } else {
        value = newValue;
    }
}

