#include <boost/assign/list_of.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>
#include <cmath>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/graph/astar_search.hpp>
#include <map>

#include "Character.hpp"
#include "World.hpp"
#include "data/TilesTable.hpp"

extern TilesTable *Tiles;

namespace pathfinding {

using namespace boost;

typedef std::pair<int, int> Position;
Position character_moves[8] = {Position(0, -1), Position(1, -1), Position(1, 0), Position(1, 1), Position(0, 1), Position(-1, 1), Position(-1, 0), Position(-1, -1)};
unordered_map<Position, Character::direction> character_directions = assign::map_list_of
        (Position(0, -1), Character::dir_north)
        (Position(1, -1), Character::dir_northeast)
        (Position(1, 0), Character::dir_east)
        (Position(1, 1), Character::dir_southeast)
        (Position(0, 1), Character::dir_south)
        (Position(-1, 1), Character::dir_southwest)
        (Position(-1, 0), Character::dir_west)
        (Position(-1, -1), Character::dir_northwest);

struct world_map_graph;

struct character_out_edge_iterator: public boost::forward_iterator_helper<character_out_edge_iterator, Position, std::ptrdiff_t, Position *, Position> {
    character_out_edge_iterator() {}
    character_out_edge_iterator(int i, Position p, const world_map_graph &g): position(p), graph(&g), direction(i) {
        valid_step();
    }
    std::pair<Position, Position> operator*() const {
        return std::make_pair(position, Position(position.first + character_moves[direction].first, position.second + character_moves[direction].second));
    }
    void operator++() {
        ++direction;
        valid_step();
    }
    bool operator==(const character_out_edge_iterator &iterator) const {
        return direction == iterator.direction;
    }

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
    world_map_graph(Position goal, int level): goal(goal), level(level) {}
    Position goal;
    int level;
};

void character_out_edge_iterator::valid_step() {
    Position new_pos(position.first + character_moves[direction].first, position.second + character_moves[direction].second);
    Field *new_field = World::get()->GetField(::position(new_pos.first, new_pos.second, graph->level));

    while (direction < 8 && (new_field == 0 ||
                             !(new_field->moveToPossible() || new_pos == graph->goal))) {
        ++direction;
        new_pos = Position(position.first + character_moves[direction].first, position.second + character_moves[direction].second);
        new_field = World::get()->GetField(::position(new_pos.first, new_pos.second, graph->level));
    }
}

world_map_graph::vertex_descriptor
source(world_map_graph::edge_descriptor e, const world_map_graph &g) {
    return e.first;
}

world_map_graph::vertex_descriptor
target(world_map_graph::edge_descriptor e, const world_map_graph &g) {
    return e.second;
}

std::pair<world_map_graph::out_edge_iterator, world_map_graph::out_edge_iterator>
out_edges(world_map_graph::vertex_descriptor v, const world_map_graph &g) {
    typedef world_map_graph::out_edge_iterator Iterator;
    return std::make_pair(Iterator(0, v, g), Iterator(8, v, g));
}

world_map_graph::degree_size_type
out_degree(world_map_graph::vertex_descriptor v, const world_map_graph &g) {
    typedef world_map_graph::out_edge_iterator Iterator;
    world_map_graph::degree_size_type count = 0;
    Iterator end(8, v, g);

    for (Iterator it(0, v, g); it != end; ++it) {
        ++count;
    }

    return count;
}

int num_vertices(const world_map_graph &g) {
    return 1000;
}

typedef float Cost;

class distance_heuristic : public astar_heuristic<world_map_graph, Cost> {
public:
    typedef graph_traits<world_map_graph>::vertex_descriptor Vertex;
    distance_heuristic(Vertex goal): goal(goal) {
        // std::cout << "heuristic goal (" << this->goal.first << ", " << this->goal.second << ")" << std::endl;
    }
    Cost operator()(Vertex u) {
        Cost dx = goal.first - u.first;
        Cost dy = goal.second - u.second;
        Cost d = sqrt(dx * dx + dy * dy);
        // std::cout << "from (" << u.first << ", " << u.second << ") to (" << goal.first << ", " << goal.second << "): " << d << std::endl;
        return d;
    }
private:
    Vertex goal;
};

template <typename T>
struct weight_calc {
    typedef Cost result_type;
    weight_calc(int level): level(level) {}
    Cost operator()(T e) const {
        auto v = e.second;
        Field *field = World::get()->GetField(::position(v.first, v.second, level));

        if (!field) {
            return 1;
        }

        auto tileId = field->getTileId();

        if (!Tiles->find(tileId, tempTile)) {
            return 1;
        }

        return tempTile.walkingCost;
    }
private:
    int level;
    mutable TilesStruct tempTile;
};

template <typename T>
struct vertex_index_hash {
    typedef int result_type;
    vertex_index_hash(): discovery_hash(0) {}
    int operator()(T v) const {
        int &id = discovery_hash[v];
        if (id == 0) {
            id = ++discovery_counter;
        }
        return id;
    }

private:
    mutable unordered_map<Position, int> discovery_hash;
    mutable int discovery_counter;
};

struct vertex_hash : std::unary_function<Position, std::size_t> {
    std::size_t operator()(Position const &u) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, u.first);
        boost::hash_combine(seed, u.second);
        return seed;
    }
};

struct found_goal {};
struct not_found {};

struct astar_ex_visitor: public boost::default_astar_visitor {
    astar_ex_visitor(Position goal): goal(goal), node_counter(0) {};

    void examine_vertex(Position u, const world_map_graph &) {
        if (u == goal) {
            throw found_goal();
        }
    }

    void discover_vertex(Position u, const world_map_graph &) {
        ++node_counter;
        if (node_counter > 400) {
            throw not_found();
        }
    }

private:
    Position goal;
    int node_counter;
};

class dist_map: public boost::unordered_map<Position, Cost, vertex_hash> {
public:
    mapped_type &operator[](key_type const &k) {
        if (find(k) == end()) {
            insert(std::make_pair(k, std::numeric_limits<Cost>::max()));
        }

        return at(k);
    }
};

class pred_map: public boost::unordered_map<Position, Position, vertex_hash> {
public:
    mapped_type &operator[](key_type const &k) {
        if (find(k) == end()) {
            insert(std::make_pair(k, k));
        }

        return at(k);
    }
};

bool a_star(::position &start_pos, ::position &goal_pos, std::list<Character::direction> &steps) {
    steps.clear();

    if (start_pos.z != goal_pos.z || start_pos == goal_pos) {
        return false;
    }

    typedef graph_traits<world_map_graph>::vertex_descriptor vertex;
    typedef graph_traits<world_map_graph>::edge_descriptor edge;

    vertex start(start_pos.x, start_pos.y);
    vertex goal(goal_pos.x, goal_pos.y);
    world_map_graph g(goal, goal_pos.z);

    distance_heuristic h(goal);
    h(start);

    //static_property_map<Cost> weight(1);
    function_property_map<weight_calc<edge>, edge, Cost> weight(goal_pos.z);

    pred_map predecessor;
    boost::associative_property_map<pred_map> pred_pmap(predecessor);

    dist_map distance;
    distance[start] = 0;
    boost::associative_property_map<dist_map> dist_pmap(distance);

    dist_map rank;
    rank[start] = h(start);
    boost::associative_property_map<dist_map> rank_pmap(rank);

    function_property_map<vertex_index_hash<vertex>, vertex, int> index;

    astar_ex_visitor visitor(goal);

    try {
        astar_search_no_init(g, start, h, weight_map(weight).rank_map(rank_pmap).
                             vertex_index_map(index).predecessor_map(pred_pmap).distance_map(dist_pmap).visitor(visitor));
    } catch (found_goal fg) {
        vertex v = goal;
        vertex pre = predecessor[v];

        while (v != pre) {
            int dx = v.first - pre.first;
            int dy = v.second - pre.second;
            vertex dir(dx, dy);
            steps.push_front(character_directions[dir]);

            v = pre;
            pre = predecessor[v];
        }

        return true;
    } catch (not_found nf) {
        return false;
    }

    return false;
}

}

