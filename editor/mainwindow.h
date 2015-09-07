#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "lcdconverter.h"

#define EEPROM_RESOURCE ":/res/eeprom.bin"

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

    void readEepromFile(QString name);
    void saveEepromFile(QString name);

private slots:
    void about();
    void aboutQt();

    void openEeprom();
    void saveEeprom();
    void saveEepromAs();
    void loadDefaultEeprom();

    void updateTranslation(int row, int column);

    void setOther();
    void setFontsize(int value);
    void setBrightnessMax(int value);
    void setScroll(int value);
    void setRotate();
    void setHourzero();
    void setSensMask();
};

#endif // MAINWINDOW_H
