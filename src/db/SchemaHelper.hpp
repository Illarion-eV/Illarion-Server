/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SCHEMA_HELPER_HPP_
#define _SCHEMA_HELPER_HPP_

#include <string>

namespace Database {
class SchemaHelper {
private:
    static std::string serverSchema;
    static std::string accountSchema;
public:
    static void setSchemata();
    static auto getServerSchema() -> const std::string &;
    static auto getAccountSchema() -> const std::string &;
};
}

#endif // _SCHEMA_HELPER_HPP_
