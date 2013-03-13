#include "World.hpp"
#include "data/TilesTable.hpp"

#include "a_star.hpp"

namespace pathfinding {

using namespace boost;

character_out_edge_iterator::character_out_edge_iterator(): graph(nullptr), direction(8) {
}

character_out_edge_iterator::character_out_edge_iterator(int i, const Position &p, const world_map_graph &g): position(p), graph(&g), direction(i) {
    valid_step();
}

std::pair<Position, Position> character_out_edge_iterator::operator*() const {
    return std::make_pair(position, Position(position.first + character_moves[direction].first, position.second + character_moves[direction].second));
}

void character_out_edge_iterator::operator++() {
    ++direction;
    valid_step();
}

bool character_out_edge_iterator::operator==(const character_out_edge_iterator &iterator) const {
    return direction == iterator.direction;
}

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

world_map_graph::world_map_graph(const Position &goal, int level): goal(goal), level(level) {
}

world_map_graph::vertex_descriptor
source(const world_map_graph::edge_descriptor &e, const world_map_graph &g) {
    return e.first;
}

world_map_graph::vertex_descriptor
target(const world_map_graph::edge_descriptor &e, const world_map_graph &g) {
    return e.second;
}

std::pair<world_map_graph::out_edge_iterator, world_map_graph::out_edge_iterator>
out_edges(const world_map_graph::vertex_descriptor &v, const world_map_graph &g) {
    typedef world_map_graph::out_edge_iterator Iterator;
    return std::make_pair(Iterator(0, v, g), Iterator(8, v, g));
}

world_map_graph::degree_size_type
out_degree(const world_map_graph::vertex_descriptor &v, const world_map_graph &g) {
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

distance_heuristic::distance_heuristic(const Vertex &goal): goal(goal) {
    // std::cout << "heuristic goal (" << this->goal.first << ", " << this->goal.second << ")" << std::endl;
}

Cost distance_heuristic::operator()(const Vertex &u) {
    Cost dx = goal.first - u.first;
    Cost dy = goal.second - u.second;
    Cost d = sqrt(dx * dx + dy * dy);
    // std::cout << "from (" << u.first << ", " << u.second << ") to (" << goal.first << ", " << goal.second << "): " << d << std::endl;
    return d;
}

std::size_t edge_hash::operator()(std::pair<Position, Position> const &e) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, e.first.first);
    boost::hash_combine(seed, e.first.second);
    boost::hash_combine(seed, e.second.first);
    boost::hash_combine(seed, e.second.second);
    return seed;
}

weight_calc::weight_calc(int level): level(level) {
}

auto weight_calc::operator[](key_type const &k) -> mapped_type& {
    mapped_type &cost = base::operator[](k);

    if (cost == 0.0) {
        auto v = k.second;
        Field *field = World::get()->GetField(::position(v.first, v.second, level));

        if (!field) {
            cost = 1;
        } else {
            auto tileId = field->getTileId();
            cost = Data::Tiles[tileId].walkingCost;
        }
    }

    return cost;
}

std::size_t vertex_hash::operator()(Position const &u) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, u.first);
    boost::hash_combine(seed, u.second);
    return seed;
}

auto vertex_index_hash::operator[](key_type const &k) -> mapped_type& {
    int &id = base::operator[](k);

    if (id == 0) {
        id = ++discovery_counter;
    }

    if (id > 400) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] No path found in vertex_index_hash!" << Log::end;
        throw not_found();
    }

    return id;
}

astar_ex_visitor::astar_ex_visitor(const Position &goal): goal(goal) {
}

void astar_ex_visitor::examine_vertex(const Position &u, const world_map_graph &) {
    if (u == goal) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] Path found in astar_ex_visitor!" << Log::end;
        throw found_goal();
    }
}

void astar_ex_visitor::discover_vertex(const Position &u, const world_map_graph &) {
    ++node_counter;

    if (node_counter > 400) {
        Logger::debug(LogFacility::Other) << "[PATH FINDING] No path found in astar_ex_visitor!" << Log::end;
        throw not_found();
    }
}

auto dist_map::operator[](key_type const &k) -> mapped_type& {
    if (find(k) == end()) {
        insert(std::make_pair(k, std::numeric_limits<Cost>::max()));
    }

    return at(k);
}

auto pred_map::operator[](key_type const &k) -> mapped_type& {
    if (find(k) == end()) {
        insert(std::make_pair(k, k));
    }

    return at(k);
}

bool a_star(const ::position &start_pos, const ::position &goal_pos, std::list<direction> &steps) {
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

    weight_calc w(goal_pos.z);
    boost::associative_property_map<weight_calc> weight(w);

    pred_map predecessor;
    boost::associative_property_map<pred_map> pred_pmap(predecessor);

    dist_map distance;
    distance[start] = 0;
    boost::associative_property_map<dist_map> dist_pmap(distance);

    dist_map rank;
    rank[start] = h(start);
    boost::associative_property_map<dist_map> rank_pmap(rank);

    vertex_index_hash vertex_hash;
    boost::associative_property_map<vertex_index_hash> index(vertex_hash);

    astar_ex_visitor visitor(goal);

    try {
        astar_search_no_init(g, start, h, weight_map(weight).rank_map(rank_pmap).
                             vertex_index_map(index).predecessor_map(pred_pmap).distance_map(dist_pmap).visitor(visitor));
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
                Logger::error(LogFacility::Other) << "direction (" << dir.first << ", " << dir.second << ") is invalid" << Log::end;
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

}

