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

#ifndef _LONGTIMEEFFECT_HPP_
#define _LONGTIMEEFFECT_HPP_


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
    bool findValue(const std::string &name, uint32_t &ret);

    bool callEffect(Character *target);
    bool save(uint32_t playerid, int32_t currentTime);

    bool isFirstAdd() const {
        return firstadd;
    }
    void firstAdd() {
        firstadd = false;
    }

    uint16_t getEffectId() const;
    std::string getEffectName() const;
    int32_t getExecuteIn() const;
    void setExecuteIn(int32_t time);
    int32_t getExecutionTime() const;
    void setExecutionTime(int32_t offset);
    uint32_t getNumberOfCalls() const;
    void setNumberOfCalls(uint32_t calls);

    static LTEPriority priority;

private:
    uint16_t effectId;
    std::string effectName;
    int32_t executeIn;
    int32_t executionTime = 0;
    uint32_t numberOfCalls = 0;
    bool firstadd = true;

    typedef std::unordered_map<std::string, uint32_t> VALUES;
    VALUES values;
};

struct LTEPriority {
    bool operator()(const LongTimeEffect *lhs, const LongTimeEffect *rhs) const {
        // effects with higher execution time have lower priority
        return lhs->getExecutionTime() > rhs->getExecutionTime();
    }
};

#endif

