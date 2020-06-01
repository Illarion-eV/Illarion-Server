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

#ifndef A_STAR_HPP
#define A_STAR_HPP

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/operators.hpp>
#include "types.hpp"
#include "globals.hpp"

namespace pathfinding {

using namespace boost;

using Position = std::pair<int, int>;
static const std::vector<Position> character_moves = {Position(0, -1), Position(1, -1), Position(1, 0), Position(1, 1), Position(0, 1), Position(-1, 1), Position(-1, 0), Position(-1, -1)};
static const unordered_map<Position, direction> character_directions = assign::map_list_of
        (Position(0, -1), dir_north)
        (Position(1, -1), dir_northeast)
        (Position(1, 0), dir_east)
        (Position(1, 1), dir_southeast)
        (Position(0, 1), dir_south)
        (Position(-1, 1), dir_southwest)
        (Position(-1, 0), dir_west)
        (Position(-1, -1), dir_northwest);

struct world_map_graph;

struct character_out_edge_iterator: public boost::forward_iterator_helper<character_out_edge_iterator, Position, std::ptrdiff_t, Position *, Position> {
    character_out_edge_iterator();
    character_out_edge_iterator(int i, Position p, const world_map_graph &g);

    auto operator*() const -> std::pair<Position, Position>;
    void operator++();
    auto operator==(const character_out_edge_iterator &iterator) const -> bool;

protected:
    void valid_step();
    Position position;
    const world_map_graph *graph{nullptr};
    int direction{8};
};

struct world_map_graph {
    using vertex_descriptor = Position;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using adjacency_iterator = void;
    using out_edge_iterator = character_out_edge_iterator;
    using in_edge_iterator = void;
    using edge_iterator = void;
    using vertex_iterator = void;
    using degree_size_type = int;
    using vertices_size_type = int;
    using edges_size_type = int;
    using directed_category = directed_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;
    using traversal_category = incidence_graph_tag;
    world_map_graph(Position goal, int level);
    Position goal;
    int level;
};

auto
source(const world_map_graph::edge_descriptor &e, const world_map_graph &g) -> world_map_graph::vertex_descriptor;

auto
target(const world_map_graph::edge_descriptor &e, const world_map_graph &g) -> world_map_graph::vertex_descriptor;

auto
out_edges(const world_map_graph::vertex_descriptor &v, const world_map_graph &g) -> std::pair<world_map_graph::out_edge_iterator, world_map_graph::out_edge_iterator>;

auto
out_degree(const world_map_graph::vertex_descriptor &v, const world_map_graph &g) -> world_map_graph::degree_size_type;

auto num_vertices(const world_map_graph &g) -> int;

using Cost = float;

class distance_heuristic : public astar_heuristic<world_map_graph, Cost> {
public:
    using Vertex = graph_traits<world_map_graph>::vertex_descriptor;
    explicit distance_heuristic(Vertex goal);
    auto operator()(const Vertex &u) -> Cost;

private:
    Vertex goal;
};

struct edge_hash : std::unary_function<std::pair<Position, Position>, std::size_t> {
    auto operator()(std::pair<Position, Position> const &e) const -> std::size_t;
};

struct weight_calc: public boost::unordered_map<std::pair<Position, Position>, Cost, edge_hash> {
    explicit weight_calc(int level);
    auto operator[](const key_type &k) -> mapped_type &;

private:
    const int level;
};

struct vertex_hash : std::unary_function<Position, std::size_t> {
    auto operator()(Position const &u) const -> std::size_t;
};

struct found_goal {};
struct not_found {};

struct vertex_index_hash: public unordered_map<Position, int, vertex_hash> {
    auto operator[](const key_type &k) -> mapped_type &;

private:
    mutable int discovery_counter = 0;
};

struct astar_ex_visitor: public boost::default_astar_visitor {
    explicit astar_ex_visitor(Position goal);

    void examine_vertex(const Position &u, const world_map_graph & /*unused*/);
    void discover_vertex(const Position &u, const world_map_graph & /*unused*/);

private:
    Position goal;
    int node_counter = 0;
};

class dist_map: public boost::unordered_map<Position, Cost, vertex_hash> {
public:
    auto operator[](key_type const &k) -> mapped_type &;
};

class pred_map: public boost::unordered_map<Position, Position, vertex_hash> {
public:
    auto operator[](key_type const &k) -> mapped_type &;
};

auto a_star(const ::position &start_pos, const ::position &goal_pos, std::list<direction> &steps) -> bool;

}

#endif

