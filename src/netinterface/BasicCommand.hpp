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

#ifndef CBASICCOMMAND_HPP
#define CBASICCOMMAND_HPP

/**
 * @ingroup Netinterface
 * @brief Base class for all network commands exchanged between client and server.
 * 
 * BasicCommand provides the foundation for the command pattern used in network
 * communication. Each command type has a unique identifier byte used for:
 * - Command type identification during deserialization
 * - Protocol version compatibility checking
 * - Message routing and dispatch
 * 
 * Derived classes include:
 * - BasicClientCommand: Commands sent from client to server
 * - BasicServerCommand: Commands sent from server to client
 * 
 * @see BasicClientCommand
 * @see BasicServerCommand
 * @see CommandFactory
 */
class BasicCommand {
private:
    unsigned char definitionByte; ///< Unique command type identifier

public:
    /**
     * @brief Creates a command with a specific type identifier.
     * 
     * @param defByte Unique command ID byte
     */
    explicit BasicCommand(unsigned char defByte);

    /**
     * @brief Gets the command type identifier.
     * 
     * @return Unique command ID byte
     */
    [[nodiscard]] auto getDefinitionByte() const -> unsigned char { return definitionByte; };
};

#endif
