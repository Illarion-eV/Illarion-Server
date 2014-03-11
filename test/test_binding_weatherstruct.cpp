#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() {
        World::_self = this;
    }
};

class weather_bindings : public ::testing::Test {
	public:
		WeatherStruct weather;
		MockWorld world;
};

TEST_F(weather_bindings, test_cloud_density_property) {
	weather.cloud_density = 23;
	LuaTestSupportScript script { "function test(weather)\n"
			"assert(weather.cloud_density == 23)\n"
			"weather.cloud_density = 42\n"
			"return weather\n"
			"end",
			"weather_cloud_density_test"
	};

	auto retval = script.test(weather);
	EXPECT_EQ(42, retval.cloud_density);
}

TEST_F(weather_bindings, test_fog_density_property) {
    weather.fog_density = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.fog_density == 23)\n"
            "weather.fog_density = 42\n"
            "return weather\n"
            "end",
            "weather_fog_density_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.fog_density);
}

TEST_F(weather_bindings, test_win_ddir_property) {
    weather.wind_dir = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.wind_dir == 23)\n"
            "weather.wind_dir = 42\n"
            "return weather\n"
            "end",
            "weather_wind_dir_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.wind_dir);
}

TEST_F(weather_bindings, test_gust_strength_property) {
    weather.gust_strength = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.gust_strength == 23)\n"
            "weather.gust_strength = 42\n"
            "return weather\n"
            "end",
            "weather_gust_strength_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.gust_strength);
}

TEST_F(weather_bindings, test_percipitation_strength_property) {
    weather.percipitation_strength = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.percipitation_strength == 23)\n"
            "weather.percipitation_strength = 42\n"
            "return weather\n"
            "end",
            "weather_percipitation_strength_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.percipitation_strength);
}

TEST_F(weather_bindings, test_percipitation_type_property) {
    weather.per_type = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.percipitation_type == 23)\n"
            "weather.percipitation_type = 42\n"
            "return weather\n"
            "end",
            "weather_percipitation_type_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.per_type);
}

TEST_F(weather_bindings, test_thunderstorm_property) {
    weather.thunderstorm = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.thunderstorm == 23)\n"
            "weather.thunderstorm = 42\n"
            "return weather\n"
            "end",
            "weather_thunderstorm_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.thunderstorm);
}

TEST_F(weather_bindings, test_temperature_property) {
    weather.temperature = 23;
    LuaTestSupportScript script { "function test(weather)\n"
            "assert(weather.temperature == 23)\n"
            "weather.temperature = 42\n"
            "return weather\n"
            "end",
            "weather_temperature_test"
    };

    auto retval = script.test(weather);
    EXPECT_EQ(42, retval.temperature);
}

TEST_F(weather_bindings, test_constructors) {
	{
	LuaTestSupportScript script { "function test()\n"
			"local foo = WeatherStruct()\n"
			"foo.cloud_density = 23\n"
			"return foo\n"
			"end",
			"weather_constructor1_test"
	};
	auto retval = script.test(weather);
	EXPECT_EQ(23, retval.cloud_density);
	}
	{
	LuaTestSupportScript script { "function test()\n"
			"local foo = WeatherStruct(23, 42, 5, 12, 6, 82, 9, 88)\n"
			"return foo\n"
			"end",
			"weather_constructor2_test"
	};
	auto retval = script.test(weather);
	EXPECT_EQ(23, retval.cloud_density);
	}
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

