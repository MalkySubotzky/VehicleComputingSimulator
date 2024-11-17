#include <gtest/gtest.h>
#include "global_properties.h"
#include "test_helper.h"

// Check build sensors
TEST(SensorsBuild, SensorsTest)
{
    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.sensors.size(), 4);
}

#pragma region Basic Condition Test

// Insert BasicCondition into instanceGP.conditions
TEST(EqualCondition, BasicConditionTest)
{
    int num = 100;
    FieldValue fv = num;
    testCondition("[8]=(Speed,100)", 8, "Speed", fv, FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.conditions.size(), 1);
}

// Check the update status
TEST(SimpleUpdateStatus, UpdateTest)
{
    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.trueConditions.size(), 1);
}

// Check the update status with BasicConditionNotEqual
TEST(NotEqualCondition, BasicConditionTest)
{
    int num = 100;
    FieldValue fv = num;
    testCondition("[8]!=(Speed,500)", 8, "Speed", fv, FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.trueConditions.size(), 1);
}

// Check the update status with BasicConditionLetter
TEST(LetterCondition, BasicConditionTest)
{
    int num = 500;
    FieldValue fv = num;
    testCondition("[8]<(Speed,500)", 8, "Speed", fv, FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.trueConditions.size(), 0);
}

// Check the update status with BasicConditionBigger
TEST(BiggerCondition, BasicConditionTest)
{
    int num = 800;
    FieldValue fv = num;
    testCondition("[8]>(Speed,500)", 8, "Speed", fv, FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.trueConditions.size(), 1);
}

#pragma endregion

#pragma region Complex Condition Test

// Check building Or operator
TEST(OrCondition, ComplexConditionTest)
{
    int num = 500;
    FieldValue fv = num;
    testCondition("[8]|(=(Speed,500),=(Message,aaa))", 8, "Speed", fv,
                  FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.conditions.size(), 1);
    EXPECT_EQ(instanceGP.trueConditions.size(), 1);
}

// Check building And operator
TEST(AndCondition, ComplexConditionTest)
{
    int num = 500;
    FieldValue fv = num;
    testCondition("[8]&(=(Speed,500),=(Message,\"aaa\"))", 8, "Speed", fv,
                  FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.conditions.size(), 1);
    EXPECT_EQ(instanceGP.trueConditions.size(), 0);
}

// Check building And operator
TEST(OrAndCondition, ComplexConditionTest)
{
    int num = 800;
    FieldValue fv = num;
    testCondition(
        "|([8]&(=(Status,true),=(Speed,500),=(Message,\"aaa\")),[8]=(Speed,800))",
        8, "Speed", fv, FieldType::SIGNED_INT);

    GlobalProperties &instanceGP = GlobalProperties::getInstance();

    EXPECT_EQ(instanceGP.trueConditions.size(), 1);
}

#pragma endregion

int main(int argc, char **argv)
{
    Input::s_setPathBson();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}