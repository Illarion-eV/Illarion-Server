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

#include "db/SelectQuery.hpp"

#include <sstream>

#include "db/Connection.hpp"

using namespace Database;

SelectQuery::SelectQuery(const SelectQuery &org) {
    Query(org);
    conditions(org.conditions);
    columns = org.columns;
    tables = org.tables;
}

SelectQuery::SelectQuery(const boost::shared_ptr<Connection> &connection) {
    Query(connection);
}

SelectQuery::~SelectQuery() {
    while (!conditions.empty()) {
        delete conditions.top();
        conditions.pop();
    }
}

void SelectQuery::addColumn(const std::string &column) {
    appendToStringList(columns, escapeKey(column));
}

void SelectQuery::addColumn(const std::string &table, const std::string &column) {
    appendToStringList(columns, escapeAndChainKeys(table, column));
}

void SelectQuery::addServerTable(const std::string &table) {
    appendToStringList(tables, escapeAndChainKeys(Database::SchemaHelper::getServerSchema(), table));
}

void SelectQuery::addAccountTable(const std::string &table) {
    appendToStringList(tables, escapeAndChainKeys(Database::SchemaHelper::getAccountSchema(), table));
}

void SelectQuery::addEqualCondition(const std::string &column, const std::string &value) {
    addEqualCondition("", column, value);
}

void SelectQuery::addEqualCondition(const std::string &table, const std::string &column, const std::string &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + getConnection().quote<std::string>(value)));
}

void SelectQuery::addEqualCondition(const std::string &column, const uint32_t &value) {
    addEqualCondition("", column, value);
}

void SelectQuery::addEqualCondition(const std::string &table, const std::string &column, const uint32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + getConnection().quote<uint32_t>(value)));
}

void SelectQuery::addEqualCondition(const std::string &column, const int32_t &value) {
    addEqualCondition("", column, value);
}

void SelectQuery::addEqualCondition(const std::string &table, const std::string &column, const int32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + getConnection().quote<int32_t>(value)));
}

void SelectQuery::addNotEqualCondition(const std::string &column, const std::string &value) {
    addNotEqualCondition("", column, value);
}

void SelectQuery::addNotEqualCondition(const std::string &table, const std::string &column, const std::string &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + getConnection().quote<std::string>(value)));
}

void SelectQuery::addNotEqualCondition(const std::string &column, const uint32_t &value) {
    addNotEqualCondition("", column, value);
}

void SelectQuery::addNotEqualCondition(const std::string &table, const std::string &column, const uint32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + getConnection().quote<uint32_t>(value)));
}

void SelectQuery::addNotEqualCondition(const std::string &column, const int32_t &value) {
    addNotEqualCondition("", column, value);
}

void SelectQuery::addNotEqualCondition(const std::string &table, const std::string &column, const int32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + getConnection().quote<int32_t>(value)));
}

void SelectQuery::andConditions() {
    mergeConditions("AND");
}

void SelectQuery::orConditions() {
    mergeConditions("OR");
}

Result SelectQuery::execute() {
    /* First build the query, then send it to the generic query class */
    while (!conditionsStack.empty()) {
        andConditions();
    }

    std::stringstream ss;
    ss << "SELECT ";
    ss << columns;
    ss << " FROM ";
    ss << tables;

    if (!conditions.empty()) {
        ss << " WHERE ";
        ss << conditions;
    }

    ss << ";";

    setQuery(ss.str());
    Base::execute();
}

std::string &SelectQuery::escapeKey(const std::string &key) {
    if (key.at(0) == '"' && key.at(key.length() - 1) == '"' && !key.empty()) {
        return key;
    }

    return "\"" + key + "\"";
}

std::string &SelectQuery::escapeAndChainKeys(const std::string &key1, const std::string &key2) {
    if (!key1.empty() && !key2.empty()) {
        return escapeKey(key1) + "." + escapeKey(key2);
    } else if (key1.empty()) {
        return escapeKey(key2);
    } else if (key2.empty()) {
        return escapeKey(key1);
    }

    return "";
}

void SelectQuery::appendToStringList(std::string &list, const std::string &newEntry) {
    if (list.empty()) {
        list += ", ";
    }

    list += newEntry;
}

void SelectQuery::mergeConditions(const std::string &operation) {
    if (conditionsStack.empty()) {
        return;
    }

    std::string *cond1;
    std::string *cond2;
    bool removeFirst = false;

    if (conditions.empty()) {
        cond1 = conditionsStack.top();
        conditionsStack.pop();

        if (conditionsStack.size() == 1) {
            conditions = *cond1;
            delete cond1;
            return;
        }

        removeFirst = true;
    } else {
        if (conditionsStack.size() < 1) {
            return;
        }

        cond1 = &conditions;
    }

    cond2 = conditionsStack.top();
    conditionsStack.pop();

    conditions =  "(" + cond1 + " " + operation + " " + cond2 + ")";

    if (removeFirst) {
        delete cond1;
    }

    delete cond2;
}
