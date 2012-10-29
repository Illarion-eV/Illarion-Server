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

#include "data/SkillTable.hpp"

#include <iostream>
#include <string>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "Logger.hpp"

SkillTable::SkillTable() : m_dataOK(false) {
    reload();
}


void SkillTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "SkillTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("skills", "skl_skill_id");
        query.addColumn("skills", "skl_name");
        query.addServerTable("skills");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                TYPE_OF_SKILL_ID skillId = (TYPE_OF_SKILL_ID)((*itr)["skl_skill_id"].as<uint16_t>());
                std::string skillName = (*itr)["skl_name"].as<std::string>();

                m_table[skillId] = skillName;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into SkillTable" << std::endl;
#endif

    } catch (std::exception &e) {
        std::cout << "exception in skills loading: " << e.what() << std::endl;
        m_dataOK = false;
    }

}

bool SkillTable::find(TYPE_OF_SKILL_ID Id) const {
    TABLE::const_iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        return true;
    }
}

bool SkillTable::find(TYPE_OF_SKILL_ID Id, std::string &ret) const {
    TABLE::const_iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}

SkillTable::TABLE::const_iterator SkillTable::begin() const {
    return m_table.cbegin();
}

SkillTable::TABLE::const_iterator SkillTable::end() const {
    return m_table.cend();
}

void SkillTable::clearOldTable() {
    m_table.clear();
}

SkillTable::~SkillTable() {
}

