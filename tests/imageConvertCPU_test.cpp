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

TEST(convert10BitGreyToRGB888, Convert) {
    ImageConvertCPU converter;
    Image image;
    const int width = 10;
    const int height = 10;
    const int pixelSize = 2;
    const int scaleFactor = 1;
    const int scaleFactor2 = 2;

    const int imageSize = width * height * pixelSize;
    image.init(width, height, width * pixelSize, imageSize, imageSize, V4L2_PIX_FMT_Y10, 0, 0);

    image.planes().resize(1);
    image.planes()[0] = new unsigned char[imageSize];

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0xffc0u;
    }
   
    ImageData data = converter.convert10BitGreyToRGB888(&image,scaleFactor);
    cv::Mat img = cv::Mat(height, width, CV_8UC3, data.data);
    

    EXPECT_EQ(img.data[0], 0xff);
    EXPECT_EQ(img.data[1], 0xff);
    EXPECT_EQ(img.data[2], 0xff);

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0b00000011000000u;
    }
    data = converter.convert10BitGreyToRGB888(&image, scaleFactor);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    EXPECT_EQ(img.data[0], 0x00);
    EXPECT_EQ(img.data[1], 0x00);
    EXPECT_EQ(img.data[2], 0x00);

    for (int i = 0; i < imageSize; i+=2) {
        *reinterpret_cast<u_int16_t*>(&image.planes()[0][i]) = 0b00001100000000u;
    }
    data = converter.convert10BitGreyToRGB888(&image, scaleFactor);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    EXPECT_EQ(img.data[0], 0b00000011u);
    EXPECT_EQ(img.data[1], 0b00000011u);
    EXPECT_EQ(img.data[2], 0b00000011u);

    ImageData data2 = converter.convert10BitGreyToRGB888(&image, scaleFactor2);
    
    EXPECT_EQ(data2.size, 3 * width * height / scaleFactor2 / scaleFactor2);

    ImageData data4 = converter.convert10BitGreyToRGB888(&image, 4);
    EXPECT_EQ(data4.size, 3 * (width / 4)* (height  / 4));
}

TEST(convert10BitPackedBayerToRGB888, ConvertSRGGB10P) {
    ImageConvertCPU converter;
    Image image;
    const int width = 4;
    const int height = 2;
    const int pixelStep = 4; // 4 pixels per 5 bytes
    const int bytesPerPixelImage = 5;
    const int scaleFactor = 1;
    const int imageSize = (width * height * bytesPerPixelImage) / pixelStep;
    image.init(width, height, width * bytesPerPixelImage / pixelStep, imageSize, imageSize, V4L2_PIX_FMT_SRGGB10P, 0, 0);
    image.planes().resize(1);
    image.planes()[0] = new unsigned char[imageSize];

    // Test 1: All R pixels set to 1023
    std::vector<std::vector<uint16_t>> bayer(height, std::vector<uint16_t>(width, 0));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if ((y % 2 == 0) && (x % 2 == 0)) bayer[y][x] = 1023; // R
        }
    }
    for (int y = 0; y < height; ++y) {
        uint16_t *row = bayer[y].data();
        unsigned char *dst = image.planes()[0] + y * 10;
        dst[0] = (row[0] >> 2) & 0xFF;
        dst[1] = (row[1] >> 2) & 0xFF;
        dst[2] = (row[2] >> 2) & 0xFF;
        dst[3] = (row[3] >> 2) & 0xFF;
        dst[4] = ((row[0] & 0x3) << 0) |
                 ((row[1] & 0x3) << 2) |
                 ((row[2] & 0x3) << 4) |
                 ((row[3] & 0x3) << 6);
    }
    ImageData data = converter.convert10BitPackedBayerToRGB888(&image, scaleFactor);
    cv::Mat img = cv::Mat(height, width, CV_8UC3, data.data);
    cv::Vec3b p00 = img.at<cv::Vec3b>(0,0);
    EXPECT_GT(p00[2], p00[1] + 40);
    EXPECT_GT(p00[2], p00[0] + 40);
    if (data.size) free(data.data);

    // Test 2: All G pixels set to 1023
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) bayer[y][x] = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (((y % 2 == 0) && (x % 2 == 1)) || ((y % 2 == 1) && (x % 2 == 0))) bayer[y][x] = 1023; // G
        }
    }
    for (int y = 0; y < height; ++y) {
        uint16_t *row = bayer[y].data();
        unsigned char *dst = image.planes()[0] + y * 10;
        dst[0] = (row[0] >> 2) & 0xFF;
        dst[1] = (row[1] >> 2) & 0xFF;
        dst[2] = (row[2] >> 2) & 0xFF;
        dst[3] = (row[3] >> 2) & 0xFF;
        dst[4] = ((row[0] & 0x3) << 0) |
                 ((row[1] & 0x3) << 2) |
                 ((row[2] & 0x3) << 4) |
                 ((row[3] & 0x3) << 6);
    }
    data = converter.convert10BitPackedBayerToRGB888(&image, scaleFactor);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    cv::Vec3b p01 = img.at<cv::Vec3b>(0,1);
    cv::Vec3b p10 = img.at<cv::Vec3b>(1,0);
    // Green dominance at (0,1) and (1,0): G channel (index 1) should be much higher than R (2) and B (0)
    EXPECT_GT(p01[1], p01[0] + 20);
    EXPECT_GT(p01[1], p01[2] + 20);
    EXPECT_GT(p10[1], p10[0] + 20);
    EXPECT_GT(p10[1], p10[2] + 20);
    if (data.size) free(data.data);

    // Test 3: All B pixels set to 1023
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) bayer[y][x] = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if ((y % 2 == 1) && (x % 2 == 1)) bayer[y][x] = 1023; // B
        }
    }
    for (int y = 0; y < height; ++y) {
        uint16_t *row = bayer[y].data();
        unsigned char *dst = image.planes()[0] + y * 10;
        dst[0] = (row[0] >> 2) & 0xFF;
        dst[1] = (row[1] >> 2) & 0xFF;
        dst[2] = (row[2] >> 2) & 0xFF;
        dst[3] = (row[3] >> 2) & 0xFF;
        dst[4] = ((row[0] & 0x3) << 0) |
                 ((row[1] & 0x3) << 2) |
                 ((row[2] & 0x3) << 4) |
                 ((row[3] & 0x3) << 6);
    }
    data = converter.convert10BitPackedBayerToRGB888(&image, scaleFactor);
    img = cv::Mat(height, width, CV_8UC3, data.data);
    cv::Vec3b p11 = img.at<cv::Vec3b>(1,1);
    cv::Vec3b p13 = img.at<cv::Vec3b>(1,3);
    // Blue dominance at (1,1) and (1,3): B channel (index 0) should be much higher than G (1) and R (2)
    EXPECT_GT(p11[0], p11[1] + 40);
    EXPECT_GT(p11[0], p11[2] + 40);
    EXPECT_GT(p13[0], p13[1] + 40);
    EXPECT_GT(p13[0], p13[2] + 40);
    if (data.size) free(data.data);
}


