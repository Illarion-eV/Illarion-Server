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

#include "LongTimeEffect.hpp"

#include <memory>
#include <string>
#include <vector>

class Character;

class LongTimeCharacterEffects {
public:
    explicit LongTimeCharacterEffects(Character *owner);

    void addEffect(LongTimeEffect *effect);
    void addEffect(std::unique_ptr<LongTimeEffect> effect);
    auto find(uint16_t effectid, LongTimeEffect *&effect) const -> bool;
    auto find(const std::string &effectname, LongTimeEffect *&effect) const -> bool;
    auto removeEffect(uint16_t effectid) -> bool;
    auto removeEffect(const std::string &name) -> bool;
    auto removeEffect(const LongTimeEffect *effect) -> bool;

    void checkEffects();
    auto save() -> bool;
    auto load() -> bool;

private:
    using EFFECTS = std::vector<std::unique_ptr<LongTimeEffect>>;
    EFFECTS effects;

    Character *owner;

    int32_t time;
};

#endif
