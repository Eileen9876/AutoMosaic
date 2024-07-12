#ifndef IMAGEPROC_HPP
#define IMAGEPROC_HPP

#include <QImage>
#include <QPixmap>
#include <algorithm>

QImage MosaicImage(const QImage& image, const QRect& rect);
QImage MedianBlurredImage(const QImage& image, const QRect& rect);


QImage MosaicImage(const QImage& image, const QRect& rect)
{
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    QImage result = image.copy();

    int size = 5;

    for(int x = c1; x <= c2; x+=size) for(int y = r1; y <= r2; y+=size) {
            QRgb pixel = image.pixel(x, y);

            for(int i = 0; i <= size; i++) for(int j = 0; j <= size; j++) {
                    result.setPixel(x+i, y+j, pixel);
            }
    }

    return result;
}

QImage MedianBlurredImage(const QImage& image, const QRect& rect)
{
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    QImage result = image.copy();

    for(int x = c1; x <= c2; x ++) {
        for(int y = r1; y <= r2; y++) {

            int r[9] = { 0 }, g[9] = { 0 }, b[9] = { 0 };
            int rgb_idx = 0;

            for(int i = -1; i <= 1; i++) {
                for(int j = -1; j <= 1; j++) {
                    QRgb pixel = image.pixel(x+i, y+j);

                    r[rgb_idx] = qRed(pixel);
                    g[rgb_idx] = qGreen(pixel);
                    b[rgb_idx] = qBlue(pixel);

                    rgb_idx++;
                }
            }

            std::sort(r, r+9);
            std::sort(g, g+9);
            std::sort(b, b+9);

            result.setPixel(x, y, qRgb(r[4], g[4], b[4]));
        }
    }
    return result;
}


#endif // IMAGEPROC_HPP
