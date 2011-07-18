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

#include "db/QueryAssign.hpp"

using namespace Database;

QueryAssign::QueryAssign() : Query() {
};

QueryAssign::QueryAssign(const QueryAssign &org) {
}

QueryAssign::QueryAssign(const PConnection connection) : Query(connection) {
};

template<typename T>
void QueryAssign::addAssignColumn(const std::string &column, const T &value) {
    addAssignColumn<T>("", column, value);
}

template<typename T>
void QueryAssign::addAssignColumn(const std::string &table, const std::string &column, const T &value) {
    appendToStringList(assignColumns, escapeAndChainKeys(table, column) + " = " + quote<T>(value));
}

void QueryAssign::addAssignColumnNull(const std::string &column) {
    addAssignColumnNull("", column);
}

void QueryAssign::addAssignColumnNull(const std::string &table, const std::string &column) {
    appendToStringList(assignColumns, escapeAndChainKeys(table, column) + " = NULL");
}

std::string &QueryAssign::buildQuerySegment() {
    return assignColumns;
}
