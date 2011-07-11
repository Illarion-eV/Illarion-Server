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

#include "db/QueryWhere.hpp"

using namespace Database;

QueryWhere() : Query();

QueryWhere::QueryWhere(const QueryWhere &org) : Query(org) {
    conditionsStack(org.conditionsStack);
    conditions = org.conditions;
}

QueryWhere(const PConnection connection) : Query(connection);

QueryWhere::~QueryWhere() {
    while (!conditionsStack.empty()) {
        delete conditions.top();
        conditions.pop();
    }
}

void QueryWhere::addEqualCondition(const std::string &column, const std::string &value) {
    addEqualCondition("", column, value);
}

void QueryWhere::addEqualCondition(const std::string &table, const std::string &column, const std::string &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + quote<std::string>(value)));
}

void QueryWhere::addEqualCondition(const std::string &column, const uint32_t &value) {
    addEqualCondition("", column, value);
}

void QueryWhere::addEqualCondition(const std::string &table, const std::string &column, const uint32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + quote<uint32_t>(value)));
}

void QueryWhere::addEqualCondition(const std::string &column, const int32_t &value) {
    addEqualCondition("", column, value);
}

void QueryWhere::addEqualCondition(const std::string &table, const std::string &column, const int32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " = " + quote<int32_t>(value)));
}

void QueryWhere::addNotEqualCondition(const std::string &column, const std::string &value) {
    addNotEqualCondition("", column, value);
}

void QueryWhere::addNotEqualCondition(const std::string &table, const std::string &column, const std::string &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + quote<std::string>(value)));
}

void QueryWhere::addNotEqualCondition(const std::string &column, const uint32_t &value) {
    addNotEqualCondition("", column, value);
}

void QueryWhere::addNotEqualCondition(const std::string &table, const std::string &column, const uint32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + quote<uint32_t>(value)));
}

void QueryWhere::addNotEqualCondition(const std::string &column, const int32_t &value) {
    addNotEqualCondition("", column, value);
}

void QueryWhere::addNotEqualCondition(const std::string &table, const std::string &column, const int32_t &value) {
    conditions.push(new std::string(escapeAndChainKeys(table, column) + " != " + quote<int32_t>(value)));
}

void QueryWhere::andConditions() {
    mergeConditions("AND");
}

void QueryWhere::orConditions() {
    mergeConditions("OR");
}

std::string &QueryWhere::buildQuerySegment() {
    while (!conditionsStack.empty()) {
        andConditions();
    }

    if (!conditions.empty()) {
        return " WHERE " + conditions;
    }

    return "";
}

void QueryWhere::mergeConditions(const std::string &operation) {
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
