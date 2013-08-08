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

#ifndef _ATTRIBUTE_HPP_
#define _ATTRIBUTE_HPP_

class Attribute {
public:
    typedef unsigned short attribute_t;

private:
    attribute_t baseValue;
    short int offset = 0;
    attribute_t maximum;

public:
    Attribute();
    Attribute(attribute_t value);
    Attribute(attribute_t value, attribute_t maximum);
    void setBaseValue(attribute_t value);
    void setValue(attribute_t value);
    attribute_t getBaseValue() const;
    attribute_t getValue() const;
    void increaseBaseValue(int amount);
    void increaseValue(int amount);
};

#endif

