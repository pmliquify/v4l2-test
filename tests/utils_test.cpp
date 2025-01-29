#include <gtest/gtest.h>
#include <utils/commandargs.hpp>

TEST(CommandArgsTest, ParseEmptyArgs) {
    const char* argv[] = {"program"};
    CommandArgs args(1, argv);
    EXPECT_TRUE(args.exists("program"));
}

TEST(CommandArgsTest, ParseOneArg) {
    const char* argv[] = {"program", "--test", "value"};
    CommandArgs args(3, argv);
    EXPECT_EQ(args.optionString("--test"),"value");
}
