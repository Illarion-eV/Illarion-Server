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

#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

/**
 * @brief Represents a character attribute with base value, offset, and maximum.
 *
 * This class manages game character attributes (like strength, dexterity, etc.)
 * with support for temporary modifiers via offsets. The effective value is
 * calculated as base value + offset, clamped to the maximum.
 */
class Attribute {
public:
    using attribute_t = unsigned short; ///< The underlying type for attribute values.

private:
    attribute_t baseValue{0}; ///< The permanent base value of the attribute.
    int offset{0}; ///< Temporary modifier applied to the base value.
    attribute_t maximum{0}; ///< The maximum allowed value for this attribute.

public:
    /**
     * @brief Default constructor initializing all values to zero.
     */
    Attribute() = default;

    /**
     * @brief Constructs an attribute with a specific value and no maximum.
     * @param value The initial base value for the attribute.
     */
    explicit Attribute(attribute_t value);

    /**
     * @brief Constructs an attribute with a value and maximum limit.
     * @param value The initial base value for the attribute.
     * @param maximum The maximum allowed value for the attribute.
     */
    Attribute(attribute_t value, attribute_t maximum);

    /**
     * @brief Sets the base value of the attribute.
     * @param value The new base value to set.
     * @note If a maximum is set and value exceeds it, baseValue is clamped to maximum.
     */
    void setBaseValue(attribute_t value);

    /**
     * @brief Sets the effective value by adjusting the offset.
     *
     * This modifies the offset so that base + offset equals the desired value.
     * If baseValue is 0, sets baseValue directly instead of using offset.
     *
     * @param value The desired effective value.
     * @note Values are clamped to maximum if set. Negative values are clamped to 0.
     */
    void setValue(attribute_t value);

    /**
     * @brief Gets the base value without any offsets applied.
     * @return The base value of the attribute.
     */
    [[nodiscard]] auto getBaseValue() const -> attribute_t;

    /**
     * @brief Gets the effective value (base + offset), clamped to valid range.
     * @return The current effective value of the attribute.
     * @note Returns 0 if the calculated value is negative, or maximum if exceeding the max.
     */
    [[nodiscard]] auto getValue() const -> attribute_t;

    /**
     * @brief Increases the base value by the specified amount.
     * @param amount The amount to add to the base value (can be negative).
     * @note Result is clamped to [0, maximum]. If maximum is 0 (unset), only lower bound applies.
     */
    void increaseBaseValue(int amount);

    /**
     * @brief Increases the effective value by modifying the offset.
     * @param amount The amount to add to the current value (can be negative).
     * @note Result is clamped to [0, maximum]. If maximum is 0 (unset), only lower bound applies.
     */
    void increaseValue(int amount);
};

#endif
