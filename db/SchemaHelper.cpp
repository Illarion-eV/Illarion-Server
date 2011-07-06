/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SchemaHelper.hpp"

namespace Database {
namespace SchemaHelper {
static std::string serverSchema;
static std::string accountSchema;

void setSchemata(const std::string &server, const std::string &account) {
    serverSchema = "\"" + server + "\"";
    accountSchema = "\"" + account + "\"";
}

std::string &getServerSchema() {
    return serverSchema;
}

std::string &getAccountSchema() {
    return accountSchema;
}
}
}