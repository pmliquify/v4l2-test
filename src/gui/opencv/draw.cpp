#include "draw.hpp"


void drawImageInfo(Mat img, Image *image, int x, int y)
{
        char info[200];
        sprintf(info, "[#%5u, ts: %8lu] (w: %u, h: %u)",
                image->sequence(), image->timestamp(),
                image->width(), image->height());

        Text text(x, y);
        text.setText(info);
        text.draw(img);
}

