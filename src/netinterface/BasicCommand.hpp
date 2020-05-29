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


#ifndef _CBASICCOMMAND_HPP_
#define _CBASICCOMMAND_HPP_

/**
*@ingroup Netinterface
*Basic class for commands which can be sent to a client or received by the server,
*holding a unique byte to identify the command.
*/
class BasicCommand {
private:
    unsigned char definitionByte; /*<Unique command id*/
public:
    /**
     *Constructor which sets the definition byte
     *\param defByte A unique command id
     */
    explicit BasicCommand(unsigned char defByte);

    /**
     *Provides read access to the definition byte
     *\return The unique command id
    */
    [[nodiscard]] auto getDefinitionByte() const -> unsigned char {
        return definitionByte;
    };
};

#endif
