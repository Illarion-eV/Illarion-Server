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

#include "db/DeleteQuery.hpp"

#include <sstream>

#include "db/ConnectionManager.hpp"

using namespace Database;

DeleteQuery::DeleteQuery(const DeleteQuery &org) {
    Query(org);
    WhereQuery(org);
    table = org.table;
}

DeleteQuery::DeleteQuery() {
    DeleteQuery(ConnectionManager::getInstance().getConnection());
}

DeleteQuery::DeleteQuery(const PConnection connection) {
    Query(connection);
}

DeleteQuery::~SelectQuery() {
    ~WhereQuery();
}

void DeleteQuery::setServerTable(const std::string &table) {
    this.table = escapeAndChainKeys(Database::SchemaHelper::getServerSchema(), table);
}

void DeleteQuery::setAccountTable(const std::string &table) {
    this.table = escapeAndChainKeys(Database::SchemaHelper::getAccountSchema(), table);
}

Result DeleteQuery::execute() {
    std::stringstream ss;
    ss << "DELETE FROM ";
    ss << table;
    ss << QueryWhere::buildQuerySegment()
    ss << ";";

    setQuery(ss.str());
    Query::execute();
}