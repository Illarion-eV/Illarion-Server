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

#include "db/QueryWhere.hpp"

#include "db/Query.hpp"

using namespace Database;

QueryWhere::QueryWhere(const Connection &connection) : connection(connection) {}

void QueryWhere::andConditions() {
    mergeConditions("AND");
}

void QueryWhere::orConditions() {
    mergeConditions("OR");
}

auto QueryWhere::buildQuerySegment() -> std::string {
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

    std::string cond1;

    if (conditions.empty()) {
        cond1 = std::move(conditionsStack.top());
        conditionsStack.pop();

        if (conditionsStack.empty()) {
            conditions = std::move(cond1);
            return;
        }
    } else {
        cond1 = std::move(conditions);
    }

    std::string cond2 = std::move(conditionsStack.top());
    conditionsStack.pop();

    conditions = "(" + cond1 + " " + operation + " " + cond2 + ")";
}
