#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "lcdconverter.h"

#define EEPROM_RESOURCE ":/res/matrixclock_en.bin"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void updateHexTable(int pos);
    void updateHexTable();

private:
    LcdConverter *lc;
    QString fileName;
    QByteArray eep;

    QWidget *digBig[10];
    QWidget *digExtra[10];

    QFont fontHex;

    void readEepromFile(QString name);
    void saveEepromFile(QString name);

    void setParams();
    void loadFonts();
    void loadalarm();

private slots:
    void about();
    void aboutQt();

    void openEeprom();
    void saveEeprom();
    void saveEepromAs();
    void loadDefaultEeprom();

    void updateTranslation(int row, int column);

    void setBrightnessMax(int value);
    void setScroll(int value);
    void setRotate(int value);
    void setCorrection(int value);
    void setFontsize();
    void setHoursignal();
    void setHourzero();
    void setSensMask();

    void updateFontBig();
    void updateFontExtra();

    void setAlarmHour(int value);
    void setAlarmMinute(int value);
    void setAlarmDays();
    void setAlarmTimeout(int value);
};

#endif // MAINWINDOW_H
