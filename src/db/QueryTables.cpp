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

#include "db/QueryTables.hpp"

#include "db/Query.hpp"
#include "db/SchemaHelper.hpp"

using namespace Database;

void QueryTables::addServerTable(const std::string &table) {
    if (oneTable && !tables.empty()) {
        throw std::logic_error("Only one table is allowed for this query.");
    }

    Query::appendToStringList(tables, Query::escapeAndChainKeys(Database::SchemaHelper::getServerSchema(), table));
}

void QueryTables::addAccountTable(const std::string &table) {
    if (oneTable && !tables.empty()) {
        throw std::logic_error("Only one table is allowed for this query.");
    }

    Query::appendToStringList(tables, Query::escapeAndChainKeys(Database::SchemaHelper::getAccountSchema(), table));
}

void QueryTables::setServerTable(const std::string &table) {
    tables = Query::escapeAndChainKeys(Database::SchemaHelper::getServerSchema(), table);
}

void QueryTables::setAccountTable(const std::string &table) {
    tables = Query::escapeAndChainKeys(Database::SchemaHelper::getAccountSchema(), table);
}

void QueryTables::setOnlyOneTable(const bool &enabled) { oneTable = enabled; }

auto QueryTables::buildQuerySegment() const -> const std::string & { return tables; }
