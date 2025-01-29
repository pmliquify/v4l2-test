#include <gtest/gtest.h>
#include <cv/image.hpp>

TEST(ImageTest, CreateEmpty) {
    Image img;
    EXPECT_EQ(img.width(), 0);
    EXPECT_EQ(img.height(), 0);
    EXPECT_EQ(img.pixelformat(), 0);
}


