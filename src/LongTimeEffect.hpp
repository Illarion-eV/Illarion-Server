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

#ifndef LONGTIMEEFFECT_HPP
#define LONGTIMEEFFECT_HPP

#include <string>
#include <unordered_map>

class Character;
class Player;
struct LTEPriority;

class LongTimeEffect {
public:
    LongTimeEffect(uint16_t effectId, int32_t executeIn);

    void addValue(const std::string &name, uint32_t value);
    void removeValue(const std::string &name);
    auto findValue(const std::string &name, uint32_t &ret) -> bool;

    auto callEffect(Character *target) -> bool;
    auto save(uint32_t playerid, int32_t currentTime) -> bool;

    auto isFirstAdd() const -> bool {
        return firstadd;
    }
    void firstAdd() {
        firstadd = false;
    }

    auto getEffectId() const -> uint16_t;
    auto getEffectName() const -> std::string;
    auto getExecuteIn() const -> int32_t;
    void setExecuteIn(int32_t time);
    auto getExecutionTime() const -> int32_t;
    void setExecutionTime(int32_t offset);
    auto getNumberOfCalls() const -> uint32_t;
    void setNumberOfCalls(uint32_t calls);

    static LTEPriority priority;

private:
    uint16_t effectId;
    std::string effectName;
    int32_t executeIn;
    int32_t executionTime = 0;
    uint32_t numberOfCalls = 0;
    bool firstadd = true;

    using VALUES = std::unordered_map<std::string, uint32_t>;
    VALUES values;
};

struct LTEPriority {
    auto operator()(const LongTimeEffect *lhs, const LongTimeEffect *rhs) const -> bool {
        // effects with higher execution time have lower priority
        return lhs->getExecutionTime() > rhs->getExecutionTime();
    }
};

#endif
