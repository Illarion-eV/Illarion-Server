//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _CCOMMANDFACTORY_HPP_
#define _CCOMMANDFACTORY_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
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
    ClientCommandPointer getCommand(unsigned char commandId);

private:

    typedef boost::unordered_map<unsigned char, BasicClientCommand *> COMMANDLIST;
    COMMANDLIST templateList; /*<the list which holds the templates for the concrete classes*/

};

#endif
