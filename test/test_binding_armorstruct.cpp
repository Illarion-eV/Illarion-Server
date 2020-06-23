#include <gmock/gmock.h>

#include "script/LuaTestSupportScript.hpp"
#include "World.hpp"

class MockWorld : public World {
public:
    MockWorld() { World::_self = this; }
};

class armor_bindings : public ::testing::Test {
public:
    ArmorStruct armor;
    MockWorld world;
};

TEST_F(armor_bindings, test_body_parts_property) {
    armor.BodyParts = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.BodyParts == 23)\n"
                                "armor.BodyParts = 42\n"
                                "assert(armor.BodyParts == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_body_parts_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.BodyParts);
}

TEST_F(armor_bindings, test_puncture_armor_property) {
    armor.PunctureArmor = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.PunctureArmor == 23)\n"
                                "armor.PunctureArmor = 42\n"
                                "assert(armor.PunctureArmor == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_puncture_armor_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.PunctureArmor);
}

TEST_F(armor_bindings, test_stroke_armor_property) {
    armor.StrokeArmor = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.StrokeArmor == 23)\n"
                                "armor.StrokeArmor = 42\n"
                                "assert(armor.StrokeArmor == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_stroke_armor_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.StrokeArmor);
}

TEST_F(armor_bindings, test_thrust_armor_property) {
    armor.ThrustArmor = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.ThrustArmor == 23)\n"
                                "armor.ThrustArmor = 42\n"
                                "assert(armor.ThrustArmor == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_thrust_armor_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.ThrustArmor);
}

TEST_F(armor_bindings, test_magic_disturbance_property) {
    armor.MagicDisturbance = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.MagicDisturbance == 23)\n"
                                "armor.MagicDisturbance = 42\n"
                                "assert(armor.MagicDisturbance == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_magic_disturbance_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.MagicDisturbance);
}

TEST_F(armor_bindings, test_absorb_property) {
    armor.Absorb = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.Absorb == 23)\n"
                                "armor.Absorb = 42\n"
                                "assert(armor.Absorb == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_absorb_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.Absorb);
}

TEST_F(armor_bindings, test_stiffness_property) {
    armor.Stiffness = 23;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.Stiffness == 23)\n"
                                "armor.Stiffness = 42\n"
                                "assert(armor.Stiffness == 23)\n"
                                "return armor\n"
                                "end",
                                "armor_stiffness_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(23, armor.Stiffness);
}

TEST_F(armor_bindings, test_type_property) {
    armor.Type = 5;
    LuaTestSupportScript script{"function test(armor)\n"
                                "assert(armor.Type == ArmorStruct.juwellery)\n"
                                "armor.Type = ArmorStruct.light\n"
                                "assert(armor.Type == ArmorStruct.juwellery)\n"
                                "return armor\n"
                                "end",
                                "armor_type_test"};

    script.test<ArmorStruct, ArmorStruct>(armor);
    EXPECT_EQ(5, armor.Type);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

