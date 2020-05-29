/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Attribute.hpp"

Attribute::Attribute() {
    baseValue = 0;
    maximum = 0;
}

Attribute::Attribute(attribute_t value) {
    baseValue = value;
    maximum = 0;
}

Attribute::Attribute(attribute_t value, attribute_t maximum) {
    baseValue = value;
    this->maximum = maximum;
}

void Attribute::setBaseValue(attribute_t value) {
    if (value > maximum && maximum != 0) {
        baseValue = maximum;
    } else {
        baseValue = value;
    }
}

void Attribute::setValue(attribute_t value) {
    if (baseValue == 0) {
        baseValue = value;
        offset = 0;
    } else {
        if (value > maximum && maximum != 0) {
            offset = maximum - baseValue;
        } else {
            offset = value - baseValue;
        }
    }
}

auto Attribute::getBaseValue() const -> attribute_t {
    return baseValue;
}

auto Attribute::getValue() const -> attribute_t {
    int value = baseValue + offset;

    if (value < 0) {
        return 0;
    } else if (value > maximum && maximum != 0) {
        return maximum;
    } else {
        return value;
    }
}

void Attribute::increaseBaseValue(int amount) {
    int newValue = baseValue + amount;

    if (newValue < 0) {
        baseValue = 0;
    } else if (newValue > maximum && maximum != 0) {
        baseValue = maximum;
    } else {
        baseValue = newValue;
    }
}

void Attribute::increaseValue(int amount) {
    int value = getValue() + amount;

    if (value < 0) {
        setValue(0);
    } else if (value > maximum && maximum != 0) {
        setValue(maximum);
    } else {
        setValue(value);
    }
}
