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

#ifndef CHARACTER_PTR_HPP
#define CHARACTER_PTR_HPP

#include "types.hpp"

class Character;

/**
 * @brief A smart pointer for Character objects that stores character IDs instead of raw pointers.
 *
 * This class provides a safe way to reference characters by storing their ID and looking them up
 * through the World when accessed. This prevents dangling pointers when characters are removed
 * from the game world. The pointer automatically becomes invalid if the character no longer exists.
 */
class character_ptr {
    TYPE_OF_CHARACTER_ID id{0}; ///< The ID of the referenced character, or 0 if null.

public:
    /**
     * @brief Default constructor creating a null character pointer.
     */
    character_ptr() = default;

    /**
     * @brief Constructs a character pointer from a Character raw pointer.
     * @param p The Character pointer to wrap, or nullptr.
     */
    explicit character_ptr(Character *p);

    /**
     * @brief Gets the Character pointer, throwing if invalid.
     * @return A pointer to the Character.
     * @throws std::logic_error if the character no longer exists.
     */
    [[nodiscard]] auto get() const -> Character *;

    /**
     * @brief Implicit conversion to Character pointer.
     * @return A pointer to the Character.
     * @throws std::logic_error if the character no longer exists.
     */
    operator Character *() const;

    /**
     * @brief Arrow operator for accessing Character members.
     * @return A pointer to the Character.
     * @throws std::logic_error if the character no longer exists.
     */
    auto operator->() const -> Character *;

    /**
     * @brief Checks if the character pointer is valid (character still exists).
     * @return true if the character exists, false otherwise.
     */
    operator bool() const;

private:
    /**
     * @brief Looks up the Character pointer from the stored ID.
     * @return A pointer to the Character, or nullptr if not found.
     */
    [[nodiscard]] auto getPointerFromId() const -> Character *;
};

/**
 * @brief Helper function to get the raw pointer from a character_ptr.
 * @param p The character_ptr to extract from.
 * @return A pointer to the Character.
 * @throws std::logic_error if the character no longer exists.
 */
auto get_pointer(character_ptr const &p) -> Character *;

/**
 * @brief Checks if a character_ptr points to a valid character.
 * @param p The character_ptr to check.
 * @return true if the character exists, false otherwise.
 */
auto isValid(character_ptr const &p) -> bool;

#endif
