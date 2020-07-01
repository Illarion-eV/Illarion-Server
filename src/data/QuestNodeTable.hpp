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

#ifndef QUEST_NODE_TABLE_HPP
#define QUEST_NODE_TABLE_HPP

#include "globals.hpp"
#include "types.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

class LuaScript;

struct NodeStruct {
    std::string entrypoint;
    std::shared_ptr<LuaScript> script;
};

class QuestNodeTable {
private:
    template <typename Key> using Table = std::unordered_multimap<Key, NodeStruct>;
    template <typename Key> using TableIterator = typename Table<Key>::const_iterator;

    static std::unique_ptr<QuestNodeTable> instance;
    Table<TYPE_OF_ITEM_ID> itemNodes;
    Table<unsigned int> npcNodes;
    Table<unsigned int> monsterNodes;
    Table<position> triggerNodes;

    static constexpr auto normalEntryCount = 4;
    static constexpr auto triggerfieldEntryCount = 6;

    static constexpr auto typePosition = 0;
    static constexpr auto idPosition = 1;
    static constexpr auto functionPosition = 2;
    static constexpr auto scriptPosition = 3;
    static constexpr auto triggerCoordinateXPosition = 1;
    static constexpr auto triggerCoordinateYPosition = 2;
    static constexpr auto triggerCoordinateZPosition = 3;
    static constexpr auto triggerFunctionPosition = 4;
    static constexpr auto triggerScriptPosition = 5;

public:
    QuestNodeTable();
    QuestNodeTable(const QuestNodeTable &) = delete;
    auto operator=(const QuestNodeTable &) -> QuestNodeTable & = delete;
    QuestNodeTable(QuestNodeTable &&) = delete;
    auto operator=(QuestNodeTable &&) -> QuestNodeTable & = delete;
    ~QuestNodeTable() = default;

    template <typename Key> using TableRange = std::pair<TableIterator<Key>, TableIterator<Key>>;

    static auto getInstance() -> QuestNodeTable &;
    void reload();
    auto getItemNodes() const -> TableRange<TYPE_OF_ITEM_ID>;
    auto getNpcNodes() const -> TableRange<unsigned int>;
    auto getMonsterNodes() const -> TableRange<unsigned int>;
    auto getTriggerNodes() const -> TableRange<position>;

private:
    void readQuest(std::ifstream &questFile, std::filesystem::path &questPath);
    void clear();
};

#endif
