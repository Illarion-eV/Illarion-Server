/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LONGTIMECHARACTEREFFECTS_HPP_
#define LONGTIMECHARACTEREFFECTS_HPP_

#include <string>
#include <vector>

class LongTimeEffect;
class Character;

class LongTimeCharacterEffects {
public:
    LongTimeCharacterEffects(Character *owner);

    void addEffect(LongTimeEffect *effect);
    bool find(uint16_t effectid, LongTimeEffect *&effect);
    bool find(const std::string &effectname, LongTimeEffect *&effect);
    bool removeEffect(uint16_t effectid);
    bool removeEffect(const std::string &name);
    bool removeEffect(LongTimeEffect *effect);

    void push_backEffect(LongTimeEffect *effect);
    void checkEffects();
    bool save();
    bool load();

private:
    typedef std::vector<LongTimeEffect *> EFFECTS;
    EFFECTS effects;

    Character *owner;

    int32_t time;
};

#endif

