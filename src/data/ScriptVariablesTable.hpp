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

#ifndef _SCRIPT_VARIABLES_TABLE_HPP_
#define _SCRIPT_VARIABLES_TABLE_HPP_

#include "data/StructTable.hpp"

class ScriptVariablesTable : public StructTable<std::string, std::string> {
public:
    virtual std::string getTableName() override;
    virtual std::vector<std::string> getColumnNames() override;
    virtual std::string assignId(const Database::ResultTuple &row) override;
    virtual std::string assignTable(const Database::ResultTuple &row) override;

    bool find(const std::string &id, std::string &ret);
    void set(const std::string &id, const std::string &value);
    void set(const std::string &id, int32_t value);
    bool remove(const std::string &id);

    void save();

    virtual bool reloadBuffer() override;
    virtual void activateBuffer() override;

private:
    typedef StructTable<std::string, std::string> Base;
    bool first = true;
};

#endif

