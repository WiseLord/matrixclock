#ifndef TIMESPINBOX_H
#define TIMESPINBOX_H

#include <qspinbox.h>

class TimeSpinBox: public QSpinBox
{
    Q_OBJECT

public:
    TimeSpinBox( QWidget * parent = 0) : QSpinBox(parent)
    {
    }

    virtual QString textFromValue(int value) const
    {
        return QString("%1").arg(value, 2 , 10, QChar('0'));
    }
};

#endif // TIMESPINBOX_H
