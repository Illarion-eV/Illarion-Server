/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SCRIPT_FOWARDER_HPP_
#define _SCRIPT_FOWARDER_HPP_

#include <stdint.h>
#include <string>
#include "Character.hpp"

uint32_t getPlayerLanguageLua(const Character*);

void inform_lua2(const Character* character, const std::string& message, Character::informType type);
void inform_lua4(const Character* character, const std::string& message);
void inform_lua1(const Character* character, const std::string& german, const std::string& english, Character::informType type);
void inform_lua3(const Character* character, const std::string& german, const std::string& english);

void log_lua(const std::string &message);

#endif
