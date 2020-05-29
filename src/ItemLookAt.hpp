/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ITEMLOOKAT_HPP_
#define _ITEMLOOKAT_HPP_

#include <string>
#include "types.hpp"

class ItemLookAt {
public:
    static const TYPE_OF_ITEMLEVEL MAX_ITEMLEVEL = 100;
    static const uint8_t MAX_GEM_LEVEL = 10;
    static const uint8_t MAX_DURABILITY = 100;

    enum Rareness {
        commonItem = 1,
        uncommonItem = 2,
        rareItem = 3,
        epicItem = 4,
    };

    void setName(const std::string &name) {
        this->name = name;
    }
    [[nodiscard]] auto getName() const -> const std::string & {
        return name;
    }

    void setRareness(Rareness rareness) {
        this->rareness = rareness;
    }
    [[nodiscard]] auto getRareness() const -> Rareness {
        return rareness;
    }

    void setDescription(const std::string &description) {
        this->description = description;
    }
    [[nodiscard]] auto getDescription() const -> const std::string & {
        return description;
    }

    void setCraftedBy(const std::string &craftedBy) {
        this->craftedBy = craftedBy;
    }
    [[nodiscard]] auto getCraftedBy() const -> const std::string & {
        return craftedBy;
    }

    void setType(const std::string &type) {
        this->type = type;
    }
    [[nodiscard]] auto getType() const -> const std::string & {
        return type;
    }

    void setLevel(TYPE_OF_ITEMLEVEL level) {
        if (level <= MAX_ITEMLEVEL) {
            this->level = level;
        }
    }
    [[nodiscard]] auto getLevel() const -> TYPE_OF_ITEMLEVEL {
        return level;
    }

    void setUsable(bool usable) {
        this->usable = usable;
    }
    [[nodiscard]] auto isUsable() const -> bool {
        return usable;
    }

    void setWeight(TYPE_OF_WEIGHT weight) {
        this->weight = weight;
    }
    [[nodiscard]] auto getWeight() const -> TYPE_OF_WEIGHT {
        return weight;
    }

    void setWorth(TYPE_OF_WORTH worth) {
        this->worth = worth;
    }
    [[nodiscard]] auto getWorth() const -> TYPE_OF_WORTH {
        return worth;
    }

    void setQualityText(const std::string &qualityText) {
        this->qualityText = qualityText;
    }
    [[nodiscard]] auto getQualityText() const -> const std::string & {
        return qualityText;
    }

    void setDurabilityText(const std::string &durabilityText) {
        this->durabilityText = durabilityText;
    }
    [[nodiscard]] auto getDurabilityText() const -> const std::string & {
        return durabilityText;
    }

    void setDurabilityValue(uint8_t durabilityValue) {
        if (durabilityValue <= MAX_DURABILITY) {
            this->durabilityValue = durabilityValue;
        }
    }
    [[nodiscard]] auto getDurabilityValue() const -> uint8_t {
        return durabilityValue;
    }

    void setDiamondLevel(uint8_t diamondLevel) {
        if (diamondLevel <= MAX_GEM_LEVEL) {
            this->diamondLevel = diamondLevel;
        }
    }
    [[nodiscard]] auto getDiamondLevel() const -> uint8_t {
        return diamondLevel;
    }

    void setEmeraldLevel(uint8_t emeraldLevel) {
        if (emeraldLevel <= MAX_GEM_LEVEL) {
            this->emeraldLevel = emeraldLevel;
        }
    }
    [[nodiscard]] auto getEmeraldLevel() const -> uint8_t {
        return emeraldLevel;
    }

    void setRubyLevel(uint8_t rubyLevel) {
        if (rubyLevel <= MAX_GEM_LEVEL) {
            this->rubyLevel = rubyLevel;
        }
    }
    [[nodiscard]] auto getRubyLevel() const -> uint8_t {
        return rubyLevel;
    }

    void setSapphireLevel(uint8_t sapphireLevel) {
        if (sapphireLevel <= MAX_GEM_LEVEL) {
            this->sapphireLevel = sapphireLevel;
        }
    }
    [[nodiscard]] auto getSapphireLevel() const -> uint8_t {
        return sapphireLevel;
    }

    void setAmethystLevel(uint8_t amethystLevel) {
        if (amethystLevel <= MAX_GEM_LEVEL) {
            this->amethystLevel = amethystLevel;
        }
    }
    [[nodiscard]] auto getAmethystLevel() const -> uint8_t {
        return amethystLevel;
    }

    void setObsidianLevel(uint8_t obsidianLevel) {
        if (obsidianLevel <= MAX_GEM_LEVEL) {
            this->obsidianLevel = obsidianLevel;
        }
    }
    [[nodiscard]] auto getObsidianLevel() const -> uint8_t {
        return obsidianLevel;
    }

    void setTopazLevel(uint8_t topazLevel) {
        if (topazLevel <= MAX_GEM_LEVEL) {
            this->topazLevel = topazLevel;
        }
    }
    [[nodiscard]] auto getTopazLevel() const -> uint8_t {
        return topazLevel;
    }

    void setBonus(uint8_t bonus) {
        this->bonus = bonus;
    }
    [[nodiscard]] auto getBonus() const -> uint8_t {
        return bonus;
    }

    auto operator==(const ItemLookAt& rhs) const -> bool;

    [[nodiscard]] auto isValid() const -> bool {
        return name.length() > 0;
    }

private:
    std::string name;
    Rareness rareness = commonItem;
    std::string description;
    std::string craftedBy;
    std::string type;
    TYPE_OF_ITEMLEVEL level = 0;
    bool usable = true;
    TYPE_OF_WEIGHT weight = 0;
    TYPE_OF_WORTH worth = 0;
    std::string qualityText;
    std::string durabilityText;
    uint8_t durabilityValue = 0;
    uint8_t diamondLevel = 0;
    uint8_t emeraldLevel = 0;
    uint8_t rubyLevel = 0;
    uint8_t sapphireLevel = 0;
    uint8_t amethystLevel = 0;
    uint8_t obsidianLevel = 0;
    uint8_t topazLevel = 0;
    uint8_t bonus = 0;
};

#endif

