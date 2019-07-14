#ifndef MYIMAGES_H
#define MYIMAGES_H
#include <QImage>

class myImages
{
public:
    int imHeight, imWidth;
    int imStride;//количество байт в одной строке изображения
    QImage img();
    myImages();
};

#endif // MYIMAGES_H
