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

#ifndef ITEMLOOKAT_HPP
#define ITEMLOOKAT_HPP

#include "types.hpp"

#include <string>

/**
 * @brief Contains detailed information about an item for display when a player examines it.
 *
 * This class stores all the information shown when a player looks at an item,
 * including its name, description, rarity, crafting details, gem levels, and durability.
 */
class ItemLookAt {
public:
    static const TYPE_OF_ITEMLEVEL MAX_ITEMLEVEL = 100; ///< Maximum item level.
    static const uint8_t MAX_GEM_LEVEL = 10; ///< Maximum level for gem enhancements.
    static const uint8_t MAX_DURABILITY = 100; ///< Maximum durability value.

    /**
     * @brief Item rarity levels affecting item quality and appearance.
     */
    enum Rareness {
        commonItem = 1,   ///< Common (white) item.
        uncommonItem = 2, ///< Uncommon (green) item.
        rareItem = 3,     ///< Rare (blue) item.
        epicItem = 4,     ///< Epic (purple) item.
    };

    /**
     * @brief Sets the item's display name.
     * @param name The name to display.
     */
    void setName(const std::string &name) { this->name = name; }

    /**
     * @brief Gets the item's display name.
     * @return The item name.
     */
    [[nodiscard]] auto getName() const -> const std::string & { return name; }

    /**
     * @brief Sets the item's rarity level.
     * @param rareness The rarity level.
     */
    void setRareness(Rareness rareness) { this->rareness = rareness; }

    /**
     * @brief Gets the item's rarity level.
     * @return The rarity level.
     */
    [[nodiscard]] auto getRareness() const -> Rareness { return rareness; }

    /**
     * @brief Sets the item's description text.
     * @param description The description to display.
     */
    void setDescription(const std::string &description) { this->description = description; }

    /**
     * @brief Gets the item's description text.
     * @return The description.
     */
    [[nodiscard]] auto getDescription() const -> const std::string & { return description; }

    /**
     * @brief Sets the name of the character who crafted this item.
     * @param craftedBy The crafter's name.
     */
    void setCraftedBy(const std::string &craftedBy) { this->craftedBy = craftedBy; }

    /**
     * @brief Gets the name of the character who crafted this item.
     * @return The crafter's name.
     */
    [[nodiscard]] auto getCraftedBy() const -> const std::string & { return craftedBy; }

    /**
     * @brief Sets the item's type description.
     * @param type The type description.
     */
    void setType(const std::string &type) { this->type = type; }

    /**
     * @brief Gets the item's type description.
     * @return The type description.
     */
    [[nodiscard]] auto getType() const -> const std::string & { return type; }

    /**
     * @brief Sets the item's level requirement (clamped to MAX_ITEMLEVEL).
     * @param level The item level.
     */
    void setLevel(TYPE_OF_ITEMLEVEL level) {
        if (level <= MAX_ITEMLEVEL) {
            this->level = level;
        }
    }

    /**
     * @brief Gets the item's level requirement.
     * @return The item level.
     */
    [[nodiscard]] auto getLevel() const -> TYPE_OF_ITEMLEVEL { return level; }

    /**
     * @brief Sets whether the item is usable.
     * @param usable true if the item can be used.
     */
    void setUsable(bool usable) { this->usable = usable; }

    /**
     * @brief Checks if the item is usable.
     * @return true if the item can be used.
     */
    [[nodiscard]] auto isUsable() const -> bool { return usable; }

    /**
     * @brief Sets the item's weight.
     * @param weight The weight value.
     */
    void setWeight(TYPE_OF_WEIGHT weight) { this->weight = weight; }

    /**
     * @brief Gets the item's weight.
     * @return The weight value.
     */
    [[nodiscard]] auto getWeight() const -> TYPE_OF_WEIGHT { return weight; }

    /**
     * @brief Sets the item's monetary worth.
     * @param worth The worth value.
     */
    void setWorth(TYPE_OF_WORTH worth) { this->worth = worth; }

    /**
     * @brief Gets the item's monetary worth.
     * @return The worth value.
     */
    [[nodiscard]] auto getWorth() const -> TYPE_OF_WORTH { return worth; }

    /**
     * @brief Sets the quality description text.
     * @param qualityText The quality description.
     */
    void setQualityText(const std::string &qualityText) { this->qualityText = qualityText; }

    /**
     * @brief Gets the quality description text.
     * @return The quality description.
     */
    [[nodiscard]] auto getQualityText() const -> const std::string & { return qualityText; }

    /**
     * @brief Sets the durability description text.
     * @param durabilityText The durability description.
     */
    void setDurabilityText(const std::string &durabilityText) { this->durabilityText = durabilityText; }

    /**
     * @brief Gets the durability description text.
     * @return The durability description.
     */
    [[nodiscard]] auto getDurabilityText() const -> const std::string & { return durabilityText; }

    /**
     * @brief Sets the durability value (clamped to MAX_DURABILITY).
     * @param durabilityValue The durability value (0-100).
     */
    void setDurabilityValue(uint8_t durabilityValue) {
        if (durabilityValue <= MAX_DURABILITY) {
            this->durabilityValue = durabilityValue;
        }
    }

    /**
     * @brief Gets the durability value.
     * @return The durability value (0-100).
     */
    [[nodiscard]] auto getDurabilityValue() const -> uint8_t { return durabilityValue; }

    /**
     * @brief Sets the diamond gem level (clamped to MAX_GEM_LEVEL).
     * @param diamondLevel The gem level (0-10).
     */
    void setDiamondLevel(uint8_t diamondLevel) {
        if (diamondLevel <= MAX_GEM_LEVEL) {
            this->diamondLevel = diamondLevel;
        }
    }

    /**
     * @brief Gets the diamond gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getDiamondLevel() const -> uint8_t { return diamondLevel; }

    /**
     * @brief Sets the emerald gem level (clamped to MAX_GEM_LEVEL).
     * @param emeraldLevel The gem level (0-10).
     */
    void setEmeraldLevel(uint8_t emeraldLevel) {
        if (emeraldLevel <= MAX_GEM_LEVEL) {
            this->emeraldLevel = emeraldLevel;
        }
    }

    /**
     * @brief Gets the emerald gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getEmeraldLevel() const -> uint8_t { return emeraldLevel; }

    /**
     * @brief Sets the ruby gem level (clamped to MAX_GEM_LEVEL).
     * @param rubyLevel The gem level (0-10).
     */
    void setRubyLevel(uint8_t rubyLevel) {
        if (rubyLevel <= MAX_GEM_LEVEL) {
            this->rubyLevel = rubyLevel;
        }
    }

    /**
     * @brief Gets the ruby gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getRubyLevel() const -> uint8_t { return rubyLevel; }

    /**
     * @brief Sets the sapphire gem level (clamped to MAX_GEM_LEVEL).
     * @param sapphireLevel The gem level (0-10).
     */
    void setSapphireLevel(uint8_t sapphireLevel) {
        if (sapphireLevel <= MAX_GEM_LEVEL) {
            this->sapphireLevel = sapphireLevel;
        }
    }

    /**
     * @brief Gets the sapphire gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getSapphireLevel() const -> uint8_t { return sapphireLevel; }

    /**
     * @brief Sets the amethyst gem level (clamped to MAX_GEM_LEVEL).
     * @param amethystLevel The gem level (0-10).
     */
    void setAmethystLevel(uint8_t amethystLevel) {
        if (amethystLevel <= MAX_GEM_LEVEL) {
            this->amethystLevel = amethystLevel;
        }
    }

    /**
     * @brief Gets the amethyst gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getAmethystLevel() const -> uint8_t { return amethystLevel; }

    /**
     * @brief Sets the obsidian gem level (clamped to MAX_GEM_LEVEL).
     * @param obsidianLevel The gem level (0-10).
     */
    void setObsidianLevel(uint8_t obsidianLevel) {
        if (obsidianLevel <= MAX_GEM_LEVEL) {
            this->obsidianLevel = obsidianLevel;
        }
    }

    /**
     * @brief Gets the obsidian gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getObsidianLevel() const -> uint8_t { return obsidianLevel; }

    /**
     * @brief Sets the topaz gem level (clamped to MAX_GEM_LEVEL).
     * @param topazLevel The gem level (0-10).
     */
    void setTopazLevel(uint8_t topazLevel) {
        if (topazLevel <= MAX_GEM_LEVEL) {
            this->topazLevel = topazLevel;
        }
    }

    /**
     * @brief Gets the topaz gem level.
     * @return The gem level (0-10).
     */
    [[nodiscard]] auto getTopazLevel() const -> uint8_t { return topazLevel; }

    /**
     * @brief Sets the item's bonus value.
     * @param bonus The bonus value.
     */
    void setBonus(uint8_t bonus) { this->bonus = bonus; }

    /**
     * @brief Gets the item's bonus value.
     * @return The bonus value.
     */
    [[nodiscard]] auto getBonus() const -> uint8_t { return bonus; }

    /**
     * @brief Compares two ItemLookAt objects for equality.
     * @param rhs The other ItemLookAt to compare against.
     * @return true if all properties match.
     */
    auto operator==(const ItemLookAt &rhs) const -> bool;

    /**
     * @brief Checks if this ItemLookAt contains valid data.
     * @return true if the item has a name, false otherwise.
     */
    [[nodiscard]] auto isValid() const -> bool { return name.length() > 0; }

private:
    std::string name; ///< The item's display name.
    Rareness rareness = commonItem; ///< The item's rarity level.
    std::string description; ///< The item's description text.
    std::string craftedBy; ///< Name of the character who crafted this item.
    std::string type; ///< The item's type description.
    TYPE_OF_ITEMLEVEL level = 0; ///< The item's level requirement.
    bool usable = true; ///< Whether the item can be used.
    TYPE_OF_WEIGHT weight = 0; ///< The item's weight.
    TYPE_OF_WORTH worth = 0; ///< The item's monetary worth.
    std::string qualityText; ///< Quality description text.
    std::string durabilityText; ///< Durability description text.
    uint8_t durabilityValue = 0; ///< Durability value (0-100).
    uint8_t diamondLevel = 0; ///< Diamond gem enhancement level (0-10).
    uint8_t emeraldLevel = 0; ///< Emerald gem enhancement level (0-10).
    uint8_t rubyLevel = 0; ///< Ruby gem enhancement level (0-10).
    uint8_t sapphireLevel = 0; ///< Sapphire gem enhancement level (0-10).
    uint8_t amethystLevel = 0; ///< Amethyst gem enhancement level (0-10).
    uint8_t obsidianLevel = 0; ///< Obsidian gem enhancement level (0-10).
    uint8_t topazLevel = 0; ///< Topaz gem enhancement level (0-10).
    uint8_t bonus = 0; ///< Item bonus value.
};

#endif
