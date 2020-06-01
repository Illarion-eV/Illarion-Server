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

#include "a_star.hpp"

#include "World.hpp"
#include "data/Data.hpp"
#include "data/TilesTable.hpp"
#include "map/Field.hpp"

#include <utility>

namespace pathfinding {

using namespace boost;

character_out_edge_iterator::character_out_edge_iterator() = default;

character_out_edge_iterator::character_out_edge_iterator(int i, Position p, const world_map_graph &g)
        : position(std::move(p)), graph(&g), direction(i) {
    valid_step();
}

auto character_out_edge_iterator::operator*() const -> std::pair<Position, Position> {
    return std::make_pair(position, Position(position.first + character_moves[direction].first,
                                             position.second + character_moves[direction].second));
}

void character_out_edge_iterator::operator++() {
    ++direction;
    valid_step();
}

auto character_out_edge_iterator::operator==(const character_out_edge_iterator &iterator) const -> bool {
    return direction == iterator.direction;
}

void character_out_edge_iterator::valid_step() {
    Position new_pos(position.first + character_moves[direction].first,
                     position.second + character_moves[direction].second);
    bool moveToPossible = false;

    try {
        map::Field &field = World::get()->fieldAt(::position(new_pos.first, new_pos.second, graph->level));
        moveToPossible = field.moveToPossible();
    } catch (FieldNotFound &) {
    }

    while (direction < 8 && !moveToPossible && !(new_pos == graph->goal)) {
        ++direction;

        new_pos = Position(position.first + character_moves[direction].first,
                           position.second + character_moves[direction].second);

        try {
            map::Field &field = World::get()->fieldAt(::position(new_pos.first, new_pos.second, graph->level));
            moveToPossible = field.moveToPossible();
        } catch (FieldNotFound &) {
        }
    }
}

world_map_graph::world_map_graph(Position goal, int level) : goal(std::move(goal)), level(level) {}

auto source(const world_map_graph::edge_descriptor &e, const world_map_graph &g) -> world_map_graph::vertex_descriptor {
    return e.first;
}

auto target(const world_map_graph::edge_descriptor &e, const world_map_graph &g) -> world_map_graph::vertex_descriptor {
    return e.second;
}

auto out_edges(const world_map_graph::vertex_descriptor &v, const world_map_graph &g)
        -> std::pair<world_map_graph::out_edge_iterator, world_map_graph::out_edge_iterator> {
    using Iterator = world_map_graph::out_edge_iterator;
    return std::make_pair(Iterator(0, v, g), Iterator(8, v, g));
}

auto out_degree(const world_map_graph::vertex_descriptor &v, const world_map_graph &g)
        -> world_map_graph::degree_size_type {
    using Iterator = world_map_graph::out_edge_iterator;
    world_map_graph::degree_size_type count = 0;
    Iterator end(8, v, g);

    for (Iterator it(0, v, g); it != end; ++it) {
        ++count;
    }

    return count;
}

auto num_vertices(const world_map_graph &g) -> int { return 1000; }

distance_heuristic::distance_heuristic(Vertex goal) : goal(std::move(goal)) {
    Logger::debug(LogFacility::Other) << "heuristic goal (" << this->goal.first << ", " << this->goal.second << ")"
                                      << Log::end;
}

auto distance_heuristic::operator()(const Vertex &u) const -> Cost {
    Cost dx = goal.first - u.first;
    Cost dy = goal.second - u.second;
    Cost d = sqrt(dx * dx + dy * dy);
    Logger::debug(LogFacility::Other) << "from (" << u.first << ", " << u.second << ") to (" << goal.first << ", "
                                      << goal.second << "): " << d << Log::end;
    return d;
}

auto edge_hash::operator()(std::pair<Position, Position> const &e) const -> std::size_t {
    std::size_t seed = 0;
    boost::hash_combine(seed, e.first.first);
    boost::hash_combine(seed, e.first.second);
    boost::hash_combine(seed, e.second.first);
    boost::hash_combine(seed, e.second.second);
    return seed;
}

weight_calc::weight_calc(int level) : level(level) {}

auto weight_calc::operator[](key_type const &k) -> mapped_type & {
    if (find(k) == end()) {
        auto v = k.second;

        try {
            map::Field &field = World::get()->fieldAt(::position(v.first, v.second, level));
            auto tileId = field.getTileId();
            insert(std::make_pair(k, Data::Tiles[tileId].walkingCost));
        } catch (FieldNotFound &) {
            insert(std::make_pair(k, 1));
        }
    }

    return at(k);
}

auto vertex_hash::operator()(Position const &u) const -> std::size_t {
    std::size_t seed = 0;
    boost::hash_combine(seed, u.first);
    boost::hash_combine(seed, u.second);
    return seed;
}

auto vertex_index_hash::operator[](key_type const &k) -> mapped_type & {
    if (find(k) == end()) {
        insert(std::make_pair(k, ++discovery_counter));
    }

    if (discovery_counter > 400) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] No path found in vertex_index_hash!" << Log::end;
        throw not_found();
    }

    return at(k);
}

astar_ex_visitor::astar_ex_visitor(Position goal) : goal(std::move(goal)) {}

void astar_ex_visitor::examine_vertex(const Position &u, const world_map_graph & /*unused*/) {
    if (u == goal) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] Path found in astar_ex_visitor!" << Log::end;
        throw found_goal();
    }
}

void astar_ex_visitor::discover_vertex(const Position &u, const world_map_graph & /*unused*/) {
    ++node_counter;

    if (node_counter > 400) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] No path found in astar_ex_visitor!" << Log::end;
        throw not_found();
    }
}

auto dist_map::operator[](key_type const &k) -> mapped_type & {
    if (find(k) == end()) {
        insert(std::make_pair(k, std::numeric_limits<Cost>::max()));
    }

    return at(k);
}

auto pred_map::operator[](key_type const &k) -> mapped_type & {
    if (find(k) == end()) {
        insert(std::make_pair(k, k));
    }

    return at(k);
}

auto a_star(const ::position &start_pos, const ::position &goal_pos, std::list<direction> &steps) -> bool {
    steps.clear();

    if (start_pos.z != goal_pos.z || start_pos == goal_pos) {
        return false;
    }

    using vertex = graph_traits<world_map_graph>::vertex_descriptor;

    vertex start(start_pos.x, start_pos.y);
    vertex goal(goal_pos.x, goal_pos.y);
    world_map_graph g(goal, goal_pos.z);

    distance_heuristic h(goal);
    h(start);

    weight_calc weights(goal_pos.z);
    boost::associative_property_map<weight_calc> weight(weights);

    pred_map predecessor;
    boost::associative_property_map<pred_map> pred_pmap(predecessor);

    dist_map distance;
    distance[start] = 0;
    boost::associative_property_map<dist_map> dist_pmap(distance);

    dist_map rank;
    rank[start] = h(start);
    boost::associative_property_map<dist_map> rank_pmap(rank);

    vertex_index_hash index_map;
    boost::associative_property_map<vertex_index_hash> index(index_map);

    astar_ex_visitor visitor(goal);

    using color_map_t = boost::unordered_map<Position, default_color_type, vertex_hash>;
    color_map_t color_map;
    boost::associative_property_map<color_map_t> color(color_map);

    try {
        astar_search_no_init(g, start, h, visitor, pred_pmap, rank_pmap, dist_pmap, weight, color, index, std::less<>(),
                             std::plus<>(), std::numeric_limits<Cost>::max(), Cost());
    } catch (found_goal &fg) {
        vertex v = goal;
        vertex pre = predecessor[v];

        while (v != pre) {
            int dx = v.first - pre.first;
            int dy = v.second - pre.second;
            vertex dir(dx, dy);

            try {
                steps.push_front(character_directions.at(dir));
            } catch (std::out_of_range &) {
                Logger::error(LogFacility::Other)
                        << "direction (" << dir.first << ", " << dir.second << ") is invalid" << Log::end;
            }

            v = pre;
            pre = predecessor[v];
        }

        return true;
    } catch (not_found &nf) {
        return false;
    }

    return false;
}

} // namespace pathfinding
