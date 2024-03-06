#include <cv/imageprint.hpp>
#include <cv/imagestats.hpp>
#include <stdio.h>



void print_stats(const Image *image, int sub) 
{
        unsigned short min, max, mean;
        ImageStats::stats(image, min, max, mean, sub);
        printf(" Stats (min: %4u, max: %4u; mean: %4u)\n", min, max, mean);
}

void print_byte_bit(char val)
{
        for(int b=7; b>=0; b--) {
                printf("%u", (char)((val >> b) & 0x01));
        }
}

void print_line_bit(const Image *image, int x1, int x2, int y)
{
        printf(" (%u, %u) ", x1, y);
        for (int x=x1; x<=x2; x=x+2) {
                unsigned short value = image->pixelValue(x, y);
                char val1 = value & 0x00ff;
                char val2 = (value >> 8) & 0x00ff;
                print_byte_bit(val2);
                print_byte_bit(val1);
                printf(" ");
        }
        printf("\n");
}

void print_line_dec(const Image *image, int x1, int x2, int y)
{
        printf(" (%u, %u) ", x1, y);
        for (int x=x1; x<=x2; x=x+2) {
                unsigned short value = image->pixelValue(x, y);
                printf("%04u ", value);
        }
        printf("\n");
}

void print_line_byte_hex(const Image *image, int x1, int x2, int y)
{
        printf(" (%u, %u) ", x1, y);
        for (int x=x1; x<=x2; x=x+2) {
                unsigned short value = image->pixelValue(x, y);
                printf("%04x ", value);
        }
        printf("\n");
}

// void print_bayer_pattern(char *data, int x, int y, int pitch)
// {
//         int shift = 6;
//         short val1 = *(short *)&data[(y  )*pitch + x  ] >> shift;
//         short val2 = *(short *)&data[(y  )*pitch + x+2] >> shift;
//         short val3 = *(short *)&data[(y+1)*pitch + x  ] >> shift;
//         short val4 = *(short *)&data[(y+1)*pitch + x+2] >> shift;
//         printf("\033[2J\033[1;1H");
//         printf("(%4d, %4d) %4d %4d\n", x, y, val1, val2);
//         printf("             %4d %4d\n",     val3, val4);
// }

void ImagePrint::print(const Image *image, unsigned int format, short x, short y, 
        unsigned long lastTimestamp, unsigned char count)
{
        if (x < 0) {
                x = image->width() / 2;
        }
        if (y < 0) {
                y = image->height() / 2;
        }

        printf("[#%04d, ts:%8ld, t:%4ld ms, %u, %u, %u, %c%c%c%c]", 
                image->sequence(), image->timestamp(), image->timestamp() - lastTimestamp, 
                image->width(), image->height(), image->bytesPerLine(),
                (image->pixelformat() >> 0 & 0xFF), (image->pixelformat() >> 8 & 0xFF), 
                (image->pixelformat() >> 16 & 0xFF), (image->pixelformat() >> 24 & 0xFF));

        switch(format) {
        case 0:  print_stats(image, 32); break;
        case 1:  print_line_bit(image, x, x + 4, y); break;
        case 10: print_line_dec(image, x, x + count-2, y); break;
        case 16: print_line_byte_hex(image, x, x + count-2, y); break;
        }
}