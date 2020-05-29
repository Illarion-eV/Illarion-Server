#include <gmock/gmock.h>

#include "World.hpp"
#include "map/Field.hpp"
#include <vector>
#include <map>
#include <sstream>

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class map_import_tests : public ::testing::Test {
	public:
        MockWorld world;
        const short map_x = 47;
        const short map_y = 11;
        const short map_level = 42;
        const short map_h = 6;
        const short map_w = 5;
};

TEST_F(map_import_tests, importTiles) {
    struct SimpleField {
        unsigned short tile;
        unsigned short music;
    };

    std::vector<std::vector<SimpleField>> tiles = {
        {{6, 2}, {6, 2}, {6, 2}, {40, 2}, {40, 2}, {40, 0}},
        {{11, 2}, {6, 2}, {6, 2}, {6, 2}, {6, 2}, {6, 0}},
        {{6, 20}, {8742, 20}, {25126, 20}, {17, 20}, {2598, 20}, {6, 0}},
        {{6, 20}, {17, 20}, {17, 20}, {17, 20}, {1574, 20}, {0, 0}},
        {{6, 0}, {4646, 0}, {10790, 0}, {3622, 0}, {6, 0}, {0, 0}}
    };

    world.import();

    for (unsigned short x = 0; x < map_w; ++x) {
        for (unsigned short y = 0; y < map_h; ++y) {
            auto &field = world.fieldAt(position(map_x + x, map_y + y, map_level));
            SimpleField &simple = tiles[x][y];
            
            std::ostringstream trace;
            trace << "x: " << x << ", y: " << y;
            SCOPED_TRACE(trace.str());
            
            EXPECT_EQ(simple.tile, field.getTileCode());
            EXPECT_EQ(simple.music, field.getMusicId());
        }
    }
}


TEST_F(map_import_tests, importWarps) {
    std::map<std::pair<short, short>, position> warps;
    warps[std::make_pair(0, 4)] = {0, 2, 4};
    warps[std::make_pair(0, 5)] = {0, 28, 4};
    warps[std::make_pair(4, 3)] = {1, 23, 54};

    world.import();

    for (unsigned short x = 0; x < map_w; ++x) {
        for (unsigned short y = 0; y < map_h; ++y) {
            auto &field = world.fieldAt(position(map_x + x, map_y + y, map_level));

            std::ostringstream trace;
            trace << "x: " << x << ", y: " << y;
            SCOPED_TRACE(trace.str());

            const auto it = warps.find(std::make_pair(x, y));

            if (it == warps.end()) {
                EXPECT_FALSE(field.isWarp());
            } else {
                auto &warp_target = it->second;

                EXPECT_TRUE(field.isWarp());

                position target;
                field.getWarp(target);
                EXPECT_EQ(warp_target, target);
            }
        }
    }
}

TEST_F(map_import_tests, importItems) {
    struct SimpleItem {
        unsigned short id;
        unsigned short quality;
        std::map<std::string, std::string> data;
    };

    std::map<std::pair<short, short>, std::vector<SimpleItem>> items;
    items[std::make_pair(0, 3)] = {{2622, 111, {{"rareness", "2"}}}, {2615, 0, {{"craftedBy", "me"}}}};
    items[std::make_pair(3, 2)] = {{2609, 123, {{";\\=crazy=;", ";=\\crazy\\\\"}}}};
    items[std::make_pair(4, 1)] = {{651, 42, {}}, {2579, 0, {{"nameDe", "German Äöß"}, {"nameEn", "English"}}}};

    world.import();

    for (unsigned short x = 0; x < map_w; ++x) {
        for (unsigned short y = 0; y < map_h; ++y) {
            auto &field = world.fieldAt(position(map_x + x, map_y + y, map_level));

            std::ostringstream trace;
            trace << "x: " << x << ", y: " << y;
            SCOPED_TRACE(trace.str());

            const auto it = items.find(std::make_pair(x, y));

            if (it == items.end()) {
                EXPECT_EQ(0, field.itemCount());
            } else {
                auto &itemStack = it->second;

                EXPECT_EQ(itemStack.size(), field.itemCount());

                for (size_t i = 0; i < itemStack.size(); ++i) {
                    auto &simple_item = itemStack[i];

                    std::ostringstream trace;
                    trace << "no. of item in stack: " << i;
                    SCOPED_TRACE(trace.str());

                    auto item = field.getStackItem(i);
                    EXPECT_EQ(simple_item.id, item.getId());
                    EXPECT_EQ(1, item.getNumber());
                    EXPECT_EQ(simple_item.quality, item.getQuality());

                    size_t dataCount = 0;

                    for (auto dataIter = item.getDataBegin(); dataIter != item.getDataEnd(); ++dataIter, ++dataCount) {
                        const auto &key = dataIter->first;
                        const auto &value = dataIter->second;

                        std::ostringstream trace;
                        trace << "found data: " << key << " -> " << value;
                        SCOPED_TRACE(trace.str());
                        
                        EXPECT_EQ(simple_item.data[key], item.getData(key));
                    }
                }
            }
        }
    }
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
