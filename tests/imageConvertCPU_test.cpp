#include <gtest/gtest.h>
#include <cv/imageConvertCPU.hpp>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

TEST(grey10BitToRGB888, CreateEmpty) {
    ImageConvertCPU converter;

    EXPECT_EQ(converter.grey10BitToRGB888(0xffc0), 0xffffff);
    EXPECT_EQ(converter.grey10BitToRGB888(0x0000), 0x000000);
    EXPECT_EQ(converter.grey10BitToRGB888(0x0040), 0x000000);
    EXPECT_EQ(converter.grey10BitToRGB888(0x0100), 0x10101);

}

TEST(convert10BitGreyToRGB888, CreateEmpty) {
    ImageConvertCPU converter;
    Image image;
    const int width = 10;
    const int height = 10;
    const int pixelSize = 2;
    const int imageSize = width * height * pixelSize;
    image.init(width, height, width * pixelSize, imageSize, imageSize, V4L2_PIX_FMT_Y10, 0, 0);

    image.planes().resize(1);
    image.planes()[0] = new unsigned char[imageSize];

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0xffc0u;
    }
   
    ImageData data = converter.convert10BitGreyToRGB888(&image);
    cv::Mat img = cv::Mat(height, width, CV_8UC3, data.data);
    

    EXPECT_EQ(img.data[0], 0xff);
    EXPECT_EQ(img.data[1], 0xff);
    EXPECT_EQ(img.data[2], 0xff);

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0b00000011000000u;
    }
    data = converter.convert10BitGreyToRGB888(&image);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    EXPECT_EQ(img.data[0], 0x00);
    EXPECT_EQ(img.data[1], 0x00);
    EXPECT_EQ(img.data[2], 0x00);

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0b00001100000000u;
    }
    data = converter.convert10BitGreyToRGB888(&image);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    EXPECT_EQ(img.data[0], 0b00000011u);
    EXPECT_EQ(img.data[1], 0b00000011u);
    EXPECT_EQ(img.data[2], 0b00000011u);


}


