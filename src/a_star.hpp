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

#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_

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

typedef std::pair<int, int> Position;
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
    character_out_edge_iterator(int i, const Position &p, const world_map_graph &g);

    std::pair<Position, Position> operator*() const;
    void operator++();
    bool operator==(const character_out_edge_iterator &iterator) const;

protected:
    void valid_step();
    Position position;
    const world_map_graph *graph;
    int direction;
};

struct world_map_graph {
    typedef Position vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef void adjacency_iterator;
    typedef character_out_edge_iterator out_edge_iterator;
    typedef void in_edge_iterator;
    typedef void edge_iterator;
    typedef void vertex_iterator;
    typedef int degree_size_type;
    typedef int vertices_size_type;
    typedef int edges_size_type;
    typedef directed_tag directed_category;
    typedef disallow_parallel_edge_tag edge_parallel_category;
    typedef incidence_graph_tag traversal_category;
    world_map_graph(const Position &goal, int level);
    Position goal;
    int level;
};

world_map_graph::vertex_descriptor
source(const world_map_graph::edge_descriptor &e, const world_map_graph &g);

world_map_graph::vertex_descriptor
target(const world_map_graph::edge_descriptor &e, const world_map_graph &g);

std::pair<world_map_graph::out_edge_iterator, world_map_graph::out_edge_iterator>
out_edges(const world_map_graph::vertex_descriptor &v, const world_map_graph &g);

world_map_graph::degree_size_type
out_degree(const world_map_graph::vertex_descriptor &v, const world_map_graph &g);

int num_vertices(const world_map_graph &g);

typedef float Cost;

class distance_heuristic : public astar_heuristic<world_map_graph, Cost> {
public:
    typedef graph_traits<world_map_graph>::vertex_descriptor Vertex;
    distance_heuristic(const Vertex &goal);
    Cost operator()(const Vertex &u);

private:
    Vertex goal;
};

struct edge_hash : std::unary_function<std::pair<Position, Position>, std::size_t> {
    std::size_t operator()(std::pair<Position, Position> const &e) const;
};

struct weight_calc: public boost::unordered_map<std::pair<Position, Position>, Cost, edge_hash> {
    weight_calc(int level);
    mapped_type &operator[](const key_type &v);

private:
    const int level;
};

struct vertex_hash : std::unary_function<Position, std::size_t> {
    std::size_t operator()(Position const &u) const;
};

struct found_goal {};
struct not_found {};

struct vertex_index_hash: public unordered_map<Position, int, vertex_hash> {
    mapped_type &operator[](const key_type &v);

private:
    mutable int discovery_counter = 0;
};

struct astar_ex_visitor: public boost::default_astar_visitor {
    astar_ex_visitor(const Position &goal);

    void examine_vertex(const Position &u, const world_map_graph &);
    void discover_vertex(const Position &u, const world_map_graph &);

private:
    Position goal;
    int node_counter = 0;
};

class dist_map: public boost::unordered_map<Position, Cost, vertex_hash> {
public:
    mapped_type &operator[](key_type const &k);
};

class pred_map: public boost::unordered_map<Position, Position, vertex_hash> {
public:
    mapped_type &operator[](key_type const &k);
};

bool a_star(const ::position &start_pos, const ::position &goal_pos, std::list<direction> &steps);

}

#endif

