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

#include <memory>
#include <unordered_map>
#include "netinterface/BasicClientCommand.hpp"

/**
*factory class which holds templates of BasicServerCommand classes
*an returns an empty command given by an id
*/
class CommandFactory {
public:

    CommandFactory();
    ~CommandFactory();

    /**
    *returns a pointer to an emtpy Server Command
    *@param commandId the id of the command which we want to use
    *@return a pointer to an empty command with the given commandId
    */
    auto getCommand(unsigned char commandId) -> ClientCommandPointer;

private:

    using COMMANDLIST = std::unordered_map<unsigned char, std::unique_ptr<BasicClientCommand>>;
    COMMANDLIST templateList;

};

#endif
