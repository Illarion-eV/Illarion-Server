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

#ifndef RESULT_HPP
#define RESULT_HPP

#include <pqxx/result.hxx>
#include <pqxx/result_iterator.hxx>

namespace Database {
/**
 * @brief Type aliases for PostgreSQL query result handling.
 * 
 * These aliases wrap the pqxx library result types, providing a simplified
 * interface and hiding the implementation details. Result types include:
 * - Result: Complete result set from a query
 * - ResultTuple: Single row from a result set
 * - ResultField: Single field/column value from a row
 * - PResult: Pointer to a result set
 * 
 * @see pqxx::result for underlying implementation
 */

/**
 * @brief Query result set type.
 */
using Result = pqxx::result;

/**
 * @brief Single row from a result set.
 */
using ResultTuple = pqxx::row;

/**
 * @brief Single field/column value from a row.
 */
using ResultField = pqxx::field;

/**
 * @brief Pointer to a result set.
 */
using PResult = Result *;
} // namespace Database

#endif
