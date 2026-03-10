#include <gtest/gtest.h>

#include "LongTimeAction.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "netinterface/NetInterface.hpp"

#include <boost/asio/io_service.hpp>
#include <memory>

class TestWorld : public World {
public:
    TestWorld() { World::_self = this; }
};

class TestPlayer : public Player {
public:
    using Character::setId;
};

TEST(WorldImmediateCommandsTest, IgnoresQueuedPlayerAfterLogoutRemoval) {
    TestWorld world;
    TestPlayer player;

    player.setId(1001);

    world.Players.insert(&player);
    world.addPlayerImmediateActionQueue(&player);

    // Simulate logout path where the player is removed before the immediate queue is processed.
    world.Players.erase(player.getId());

    EXPECT_NO_FATAL_FAILURE(world.checkPlayerImmediateCommands());
}

TEST(WorldImmediateCommandsTest, ProcessesActivePlayerWithoutCrash) {
    TestWorld world;
    TestPlayer player;
    boost::asio::io_service ioService;

    player.setId(1002);
    player.Connection = std::make_shared<NetInterface>(ioService);
    player.Connection->online = true;

    world.Players.insert(&player);
    world.addPlayerImmediateActionQueue(&player);

    EXPECT_NO_FATAL_FAILURE(world.checkPlayerImmediateCommands());
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
