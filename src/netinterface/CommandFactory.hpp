//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CCOMMANDFACTORY_HPP
#define CCOMMANDFACTORY_HPP

#include "netinterface/BasicClientCommand.hpp"

#include <memory>
#include <unordered_map>

/**
 * @brief Factory for creating client command instances from network protocol IDs.
 *
 * Maintains a registry of command templates indexed by protocol byte identifiers.
 * When a command byte is received from the network, the factory clones the
 * corresponding template to create a new command instance ready for data parsing.
 *
 * This pattern allows:
 * - Fast command instantiation without complex switch statements
 * - Easy addition of new command types
 * - Separation of command structure from instantiation logic
 *
 * @note Constructor pre-registers all game and admin command types
 * @see BasicClientCommand for the command base class
 */
class CommandFactory {
public:
    /**
     * @brief Constructs factory and registers all known command types.
     *
     * Pre-populates the template list with instances of all client commands
     * including player actions (movement, combat, chat) and admin commands
     * (warp, attribute changes, broadcasts).
     */
    CommandFactory();

    /**
     * @brief Creates a new command instance for the given protocol ID.
     * @param commandId Network protocol byte identifying the command type
     * @return Shared pointer to new command instance, or null if ID unknown
     *
     * Returns a clone of the registered template for thread-safe reuse.
     */
    auto getCommand(unsigned char commandId) -> ClientCommandPointer;

private:
    using COMMANDLIST = std::unordered_map<unsigned char, std::unique_ptr<BasicClientCommand>>;
    COMMANDLIST templateList; ///< Map of protocol IDs to command templates
};

#endif
