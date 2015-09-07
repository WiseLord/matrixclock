#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "aboutdialog.h"

#include "../eeprom.h"
#include "../matrix.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    lc = new LcdConverter();

    /* Create hex table */
    wgtHexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wgtHexTable->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    wgtHexTable->verticalHeader()->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    wgtHexTable->horizontalHeader()->setFont(QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family(), 9, QFont::Bold));
    for (int y = 0; y < 32; y++) {
        wgtHexTable->setVerticalHeaderItem(y, new QTableWidgetItem(QString("%1").arg(y * 16, 4, 16, QChar('0')).toUpper()));
        for (int x = 0; x < 16; x++)
            wgtHexTable->setItem(y, x, new QTableWidgetItem());
    }
    for (int x = 0; x < 16; x++)
        wgtHexTable->setHorizontalHeaderItem(x, new QTableWidgetItem(QString("%1").arg(x, 0, 16).toUpper()));

    /* Create translations table */
    wgtTranslations->blockSignals(true);
    for (int y = 0; y < LABEL_END; y++)
        wgtTranslations->setItem(y, 0, new QTableWidgetItem());
    wgtTranslations->blockSignals(false);

    /* Load default eeprom file */
    readEepromFile(EEPROM_RESOURCE);
}

void MainWindow::updateHexTable(int pos)
{
    QTableWidgetItem *item = wgtHexTable->item(pos / 16, pos % 16);
    item->setText(eep.mid(pos, 1).toHex().toUpper());
    if (item->text() == "FF")
        item->setTextColor(Qt::gray);
    else if (item->text() == "00" && (pos) >= EEPROM_LABELS)
        item->setTextColor(Qt::blue);
    else
        item->setTextColor(Qt::black);
}

void MainWindow::updateHexTable()
{
    for (int pos = 0; pos < 512; pos++)
        updateHexTable(pos);
}

void MainWindow::readEepromFile(QString name)
{
    /* Reading file to QByteArray buffer */

    QFile file(name);

    if (!file.open(QIODevice::ReadOnly))
        return;
    if (name != EEPROM_RESOURCE) {
        actionSaveEeprom->setEnabled(true);
        fileName = name;
        Ui_MainWindow::statusBar->showMessage(
                    tr("File") + " " + fileName + " " + tr("loaded"));
    } else {
        actionSaveEeprom->setEnabled(false);
        fileName.clear();
        Ui_MainWindow::statusBar->showMessage("Default eeprom loaded");
    }

    eep = file.readAll();
    file.close();
    updateHexTable();

    // Processing translations

    QBuffer buffer(&eep);
    char ch;
    int pos, len;

    /* Load text labels */
    wgtTranslations->blockSignals(true);

    buffer.open(QIODevice::ReadOnly);
    buffer.seek(EEPROM_LABELS);

    pos = buffer.pos();
    len = 0;
    buffer.getChar(&ch);

    for (int i = 0; i < LABEL_END && buffer.pos() < EEPROM_SIZE; i++) {
        while (ch == 0x00 && buffer.pos() < EEPROM_SIZE) {
            pos = buffer.pos();
            len = 0;
            buffer.getChar(&ch);
        }
        while (ch != 0x00 && buffer.pos() < EEPROM_SIZE) {
            buffer.getChar(&ch);
            len++;
        }
        wgtTranslations->item(i, 0)->setText(lc->decode(eep.mid(pos, len), LcdConverter::MAP_CP1251));
    }
    wgtTranslations->blockSignals(false);

    buffer.close();

    // Processing other functions
    setOther();
}

void MainWindow::saveEepromFile(QString name)
{
    fileName = name;

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly)) {
        Ui_MainWindow::statusBar->showMessage(tr("Can't save") + " " + name);
        return;
    }
    file.write(eep);
    file.close();
    Ui_MainWindow::statusBar->showMessage(tr("Saved as") + " " + name);

}

void MainWindow::openEeprom()
{
    QString name = QFileDialog::getOpenFileName(this,
        tr("Open eeprom binary"),
        "../eeprom/",
        tr("EEPROM files (*.bin);;All files (*.*)"));

    readEepromFile(name);
}

void MainWindow::saveEeprom()
{
    saveEepromFile(fileName);
}

void MainWindow::saveEepromAs()
{
    QString name = QFileDialog::getSaveFileName(this,
        tr("Save eeprom binary"),
        "../eeprom/" + fileName,
        tr("EEPROM files (*.bin)"));

    if (name.isEmpty())
        return;

    saveEepromFile(name);
}

void MainWindow::loadDefaultEeprom()
{
    readEepromFile(EEPROM_RESOURCE);
}

void MainWindow::updateTranslation(int row, int column)
{
    Q_UNUSED(row); Q_UNUSED(column);

    QBuffer buffer(&eep);

    buffer.open(QIODevice::WriteOnly);
    buffer.seek(EEPROM_LABELS);

    for (int i = 0; i < LABEL_END; i++) {
        QString str = wgtTranslations->item(i, 0)->text();
        if (str.isEmpty())
            str = " ";
        buffer.write(lc->encode(str, LcdConverter::MAP_CP1251));
        buffer.putChar('\0');
    }

    while (buffer.pos() < EEPROM_SIZE) {
        buffer.putChar(0xFF);
    }

    buffer.close();

    updateHexTable();
}

void MainWindow::about()
{
    AboutDialog dlg;

    dlg.exec();
}

void MainWindow::aboutQt()
{
    qApp->aboutQt();
}

void MainWindow::setOther()
{
    setFontsize(eep[EEPROM_BIGNUM] - 1);
    cbxFontsize->setCurrentIndex(eep[EEPROM_BIGNUM] - 1);

    setBrightnessMax(eep[EEPROM_BR_MAX]);
    sbxBrmax->setValue(eep[EEPROM_BR_MAX]);

    setScroll(eep[EEPROM_SCROLL_INTERVAL]);
    sbxScroll->setValue(eep[EEPROM_SCROLL_INTERVAL]);

    cbxRotate->setChecked(eep[EEPROM_SCREEN_ROTATE]);
    cbxHourzero->setChecked(eep[EEPROM_HOURZERO]);
}

void MainWindow::setFontsize(int value)
{
    if (value >= NUM_END)
        value = NUM_NORMAL;
    eep[EEPROM_BIGNUM] = (char)value + 1;
    updateHexTable(EEPROM_BIGNUM);
}

void MainWindow::setBrightnessMax(int value)
{
    if (value < MATRIX_MIN_BRIGHTNESS)
        value = MATRIX_MIN_BRIGHTNESS;
    if (value > MATRIX_MAX_BRIGHTNESS)
        value = MATRIX_MAX_BRIGHTNESS;

    eep[EEPROM_BR_MAX] = (char)value;
    updateHexTable(EEPROM_BR_MAX);
}

void MainWindow::setScroll(int value)
{
    eep[EEPROM_SCROLL_INTERVAL] = (unsigned char)value;
    updateHexTable(EEPROM_SCROLL_INTERVAL);
}

void MainWindow::setRotate()
{
    eep[EEPROM_SCREEN_ROTATE] = cbxRotate->isChecked();
    updateHexTable(EEPROM_SCREEN_ROTATE);
}

void MainWindow::setHourzero()
{
    eep[EEPROM_HOURZERO] = cbxHourzero->isChecked();
    updateHexTable(EEPROM_HOURZERO);
}
