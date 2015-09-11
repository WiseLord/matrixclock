#include "fontpixel.h"

FontPixel::FontPixel ()
{
    setFlat(true);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setStyleSheet(" \
        QPushButton { \
            border-radius: 3px; \
            background-color: #FFFFFF; \
        } \
        QPushButton:checked { \
            background-color: #000000; \
        } \
    ");
}

