#include <v4l2image.hpp>
#include <stdlib.h>
#include <stdio.h>

V4L2Image::V4L2Image() :
        m_bufferIndex(0),
        m_width(0),
        m_height(0),
        m_bytesPerLine(0),
        m_pixelformat(0),
        m_sequence(0),
        m_timestamp(0),
        m_lastTimestamp(0)
{
}

void print_line_byte_hex(char *st, int x1, int x2, int y, int pitch)
{
	for (int x=x1; x<=x2; x=x+2) {
		char val1 = st[y*pitch + x];
		char val2 = st[y*pitch + x+1];
		printf("%02x%02x ", val1, val2);
	}
	printf("\n");
}

void print_line_dec(char *st, int x1, int x2, int y, int pitch, int shift)
{
	for (int x=x1; x<=x2; x=x+2) {
		short val16 = (*(short *)&st[y*pitch + x]) >> shift;
		printf("%04u ", val16);
	}
	printf("\n");
}

void print_byte_bit(char val)
{
	for(int b=7; b>=0; b--) {
		printf("%u", (char)((val >> b) & 0x01));
	}
}

void print_line_bit(char *st, int x1, int x2, int y, int pitch)
{
	for (int x=x1; x<=x2; x=x+2) {
		char val1 = st[y*pitch + x];
		char val2 = st[y*pitch + x+1];
		print_byte_bit(val2);
		print_byte_bit(val1);
		printf(" ");
	}
	printf("\n");
}

void print_bayer_pattern(char *st, int x, int y, int pitch)
{
	int shift = 6;
	short val1 = *(short *)&st[(y  )*pitch + x  ] >> shift;
	short val2 = *(short *)&st[(y  )*pitch + x+2] >> shift;
	short val3 = *(short *)&st[(y+1)*pitch + x  ] >> shift;
	short val4 = *(short *)&st[(y+1)*pitch + x+2] >> shift;
	printf("\033[2J\033[1;1H");
	printf("(%4d, %4d) %4d %4d\n", x, y, val1, val2);
	printf("             %4d %4d\n",     val3, val4);
}

void V4L2Image::print(u_int32_t format, int16_t x, int16_t y, u_int8_t count, int16_t shift)
{
        if (m_lastTimestamp == 0) {
		m_lastTimestamp = m_timestamp;   
        }
        if (x < 0) {
                x = m_width / 2;
        }
        if (y < 0) {
                y = m_height / 2;
        }

        printf("[#%04d, ts:%8ld, t:%4ld ms, %u, %u, %u, %c%c%c%c] (%u, %u) ", 
                m_sequence, m_timestamp, m_timestamp - m_lastTimestamp, 
                m_width, m_height, m_bytesPerLine,
                (m_pixelformat >> 0 & 0xFF), (m_pixelformat >> 8 & 0xFF), 
                (m_pixelformat >> 16 & 0xFF), (m_pixelformat >> 24 & 0xFF),
                x, y);

        switch(format) {
        case 1:  print_line_bit((char *)m_planes[0], x, x + 4, y, m_bytesPerLine); break;
	case 10: print_line_dec((char *)m_planes[0], x, x + count-2, y, m_bytesPerLine, shift); break;
        case 16: print_line_byte_hex((char *)m_planes[0], x, x + count-2, y, m_bytesPerLine); break;
        }

        m_lastTimestamp = m_timestamp;
}

int V4L2Image::stats(u_int16_t &min, u_int16_t &max, u_int16_t &mean, u_int8_t sub)
{
	char *st = (char *)m_planes[0];
	u_int32_t pitch = m_bytesPerLine;
	u_int32_t pixelwidth = m_bytesPerLine/m_width;
	u_int32_t xMax = m_width*pixelwidth;
	u_int32_t xStep = sub*pixelwidth;
	u_int32_t yMax = m_height*pitch;
	u_int32_t yStep = sub*pitch;
	u_int64_t sum = 0;
	min = -1;
	max = 0;
	mean = 0;
	

	for (u_int32_t y = 0; y < yMax; y += yStep) {
		for (u_int32_t x = 0; x < xMax; x += xStep) {
			u_int32_t index = y + x;
			char *pixel = st + index;
			u_int16_t val16 = (*(u_int16_t*)pixel);
			val16 = val16 >> m_shift;
			if (val16 < min) {
				min = val16;
			}
			if (val16 > max) {
				max = val16;
			}
			sum += val16;
		}
	}
	mean = sum / (m_width/sub * m_height/sub);
	return 0;
}