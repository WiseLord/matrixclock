#include "mainwindow.h"

#include <QDebug>
#include <QtWidgets>

#include "aboutdialog.h"
#include "fontpixel.h"

#include "../eeprom.h"
#include "../matrix.h"
#include "../display.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    lc = new LcdConverter();

    // Create hex table
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

    // Create translations table
    wgtTranslations->blockSignals(true);
    for (int y = 0; y < LABEL_END; y++)
        wgtTranslations->setItem(y, 0, new QTableWidgetItem());
    wgtTranslations->blockSignals(false);

    QGridLayout *grlt;
    FontPixel *fpx;
    for (int i = 0; i < 10; i++) {
        lwgtFontBig->addItem(QString::number(i));
        digBig[i] = new QWidget();
        grlt = new QGridLayout();
        grlt->setMargin(1);
        grlt->setSpacing(2);
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 8; k++) {
                fpx = new FontPixel();
                grlt->addWidget(fpx, k, j);
                connect(fpx, SIGNAL(clicked()), this, SLOT(updateFontBig()));
            }
        }
        digBig[i]->setLayout(grlt);
        stwgtFontBig->addWidget(digBig[i]);

        lwgtFontExtra->addItem(QString::number(i));
        digExtra[i] = new QWidget();
        grlt = new QGridLayout();
        grlt->setMargin(1);
        grlt->setSpacing(2);
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 8; k++) {
                fpx = new FontPixel();
                grlt->addWidget(fpx, k, j);
                connect(fpx, SIGNAL(clicked()), this, SLOT(updateFontExtra()));
            }
        }
        digExtra[i]->setLayout(grlt);
        stwgtFontExtra->addWidget(digExtra[i]);
    }
    lwgtFontBig->item(0)->setSelected(true);
    lwgtFontExtra->item(0)->setSelected(true);

    // Load default eeprom file
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
    // Reading file to QByteArray buffer

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

    // Load text labels
    wgtTranslations->blockSignals(true);

    buffer.open(QIODevice::ReadOnly);
    buffer.seek(EEPROM_LABELS);

    pos = buffer.pos();
    len = 0;
    buffer.getChar(&ch);

    for (int i = 0; i < LABEL_END && buffer.pos() < EEPROM_EXTRA_NUM_FONT; i++) {
        while (ch == 0x00 && buffer.pos() < EEPROM_EXTRA_NUM_FONT) {
            pos = buffer.pos();
            len = 0;
            buffer.getChar(&ch);
        }
        while (ch != 0x00 && buffer.pos() < EEPROM_EXTRA_NUM_FONT) {
            buffer.getChar(&ch);
            len++;
        }
        wgtTranslations->item(i, 0)->setText(lc->decode(eep.mid(pos, len), LcdConverter::MAP_CP1251));
    }
    wgtTranslations->blockSignals(false);

    buffer.close();

    // Processing other functions
    setParams();

    // Processing fonts
    loadFonts();

    // Processing alarms
    loadalarm();
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


void MainWindow::setParams()
{
    setBrightnessMax(eep[EEPROM_BR_MAX]);
    sbxBrmax->setValue(eep[EEPROM_BR_MAX]);

    setScroll(eep[EEPROM_SCROLL_INTERVAL]);
    sbxScroll->setValue(eep[EEPROM_SCROLL_INTERVAL]);

    cbxHoursignal->setChecked(eep[EEPROM_HOURSIGNAL]);
    cbxRotate->setChecked(eep[EEPROM_SCREEN_ROTATE]);
    cbxFontsize->setChecked(eep[EEPROM_BIGNUM]);
    cbxHourzero->setChecked(eep[EEPROM_HOURZERO]);

    cbxBmpTemp->setChecked(eep[EEPROM_SENS_MASK] & SENS_MASK_BMP_TEMP);
    cbxDhtTemp->setChecked(eep[EEPROM_SENS_MASK] & SENS_MASK_DHT_TEMP);
    cbxBmpPres->setChecked(eep[EEPROM_SENS_MASK] & SENS_MASK_BMP_PRES);
    cbxDhtHumi->setChecked(eep[EEPROM_SENS_MASK] & SENS_MASK_DHT_HUMI);
}

void MainWindow::loadFonts()
{
    QGridLayout *grlt;
    FontPixel *fpx;
    unsigned char data;

    // Load big font
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            grlt = dynamic_cast<QGridLayout*>(digBig[i]->layout());
            data = eep[EEPROM_BIG_NUM_FONT + i * 5 + j];
            for (int k = 0; k < 8; k++) {
                fpx = dynamic_cast<FontPixel*>(grlt->itemAtPosition(k, j)->widget());
                fpx->setChecked(data & 1<<k);
            }
        }
    }

    // Load extra font
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 6; j++) {
            grlt = dynamic_cast<QGridLayout*>(digExtra[i]->layout());
            data = eep[EEPROM_EXTRA_NUM_FONT + i * 6 + j];
            for (int k = 0; k < 8; k++) {
                fpx = dynamic_cast<FontPixel*>(grlt->itemAtPosition(k, j)->widget());
                fpx->setChecked(data & 1<<k);
            }
        }
    }
}

void MainWindow::loadalarm()
{
    sbxAlarmHour->setValue(eep[EEPROM_ALARM_HOUR]);
    sbxAlarmMinute->setValue(eep[EEPROM_ALARM_MIN]);

    cbxAlarmMo->setChecked(eep[EEPROM_ALARM_MON + 0]);
    cbxAlarmTu->setChecked(eep[EEPROM_ALARM_MON + 1]);
    cbxAlarmWe->setChecked(eep[EEPROM_ALARM_MON + 2]);
    cbxAlarmTh->setChecked(eep[EEPROM_ALARM_MON + 3]);
    cbxAlarmFr->setChecked(eep[EEPROM_ALARM_MON + 4]);
    cbxAlarmSa->setChecked(eep[EEPROM_ALARM_MON + 5]);
    cbxAlarmSu->setChecked(eep[EEPROM_ALARM_MON + 6]);

    sbxAlarmTimeout->setValue(eep[EEPROM_ALARM_TIMEOUT]);
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

    while (buffer.pos() < EEPROM_EXTRA_NUM_FONT) {
        buffer.putChar(0xFF);
    }

    buffer.close();

    updateHexTable();
}

void MainWindow::setBrightnessMax(int value)
{
    if (value < 0)
        value = 0;
    if (value > 15)
        value = 15;

    eep[EEPROM_BR_MAX] = (char)value;
    updateHexTable(EEPROM_BR_MAX);
}

void MainWindow::setScroll(int value)
{
    eep[EEPROM_SCROLL_INTERVAL] = (unsigned char)value;
    updateHexTable(EEPROM_SCROLL_INTERVAL);
}

void MainWindow::setFontsize()
{
    eep[EEPROM_BIGNUM] = cbxFontsize->isChecked();
    updateHexTable(EEPROM_BIGNUM);
}

void MainWindow::setRotate()
{
    eep[EEPROM_SCREEN_ROTATE] = cbxRotate->isChecked();
    updateHexTable(EEPROM_SCREEN_ROTATE);
}

void MainWindow::setHoursignal()
{
    eep[EEPROM_HOURSIGNAL] = cbxHoursignal->isChecked();
    updateHexTable(EEPROM_HOURSIGNAL);
}

void MainWindow::setHourzero()
{
    eep[EEPROM_HOURZERO] = cbxHourzero->isChecked();
    updateHexTable(EEPROM_HOURZERO);
}

void MainWindow::setSensMask()
{
    char mask = 0;

    if (cbxBmpTemp->isChecked()) mask |= SENS_MASK_BMP_TEMP;
    if (cbxDhtTemp->isChecked()) mask |= SENS_MASK_DHT_TEMP;
    if (cbxBmpPres->isChecked()) mask |= SENS_MASK_BMP_PRES;
    if (cbxDhtHumi->isChecked()) mask |= SENS_MASK_DHT_HUMI;

    eep[EEPROM_SENS_MASK] = mask;
    updateHexTable(EEPROM_SENS_MASK);
}

void MainWindow::updateFontBig()
{
    QGridLayout *grlt;
    FontPixel *fpx;
    unsigned char data;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            grlt = dynamic_cast<QGridLayout*>(digBig[i]->layout());
            data = 0;
            for (int k = 0; k < 8; k++) {
                fpx = dynamic_cast<FontPixel*>(grlt->itemAtPosition(k, j)->widget());
                if (fpx->isChecked())
                    data |= (1<<k);
                else
                    data &= ~(1<<k);
            }
            eep[EEPROM_BIG_NUM_FONT + i * 5 + j] = data;
        }
    }
    updateHexTable();
}

void MainWindow::updateFontExtra()
{
    QGridLayout *grlt;
    FontPixel *fpx;
    unsigned char data;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 6; j++) {
            grlt = dynamic_cast<QGridLayout*>(digExtra[i]->layout());
            data = 0;
            for (int k = 0; k < 8; k++) {
                fpx = dynamic_cast<FontPixel*>(grlt->itemAtPosition(k, j)->widget());
                if (fpx->isChecked())
                    data |= (1<<k);
                else
                    data &= ~(1<<k);
            }
            eep[EEPROM_EXTRA_NUM_FONT + i * 6 + j] = data;
        }
    }
    updateHexTable();
}

void MainWindow::setAlarmHour(int value)
{
    eep[EEPROM_ALARM_HOUR] = (char)value;
    updateHexTable(EEPROM_ALARM_HOUR);
}

void MainWindow::setAlarmMinute(int value)
{
    eep[EEPROM_ALARM_MIN] = (char)value;
    updateHexTable(EEPROM_ALARM_MIN);
}

void MainWindow::setAlarmDays()
{
    eep[EEPROM_ALARM_MON + 0] = cbxAlarmMo->isChecked();
    eep[EEPROM_ALARM_MON + 1] = cbxAlarmTu->isChecked();
    eep[EEPROM_ALARM_MON + 2] = cbxAlarmWe->isChecked();
    eep[EEPROM_ALARM_MON + 3] = cbxAlarmTh->isChecked();
    eep[EEPROM_ALARM_MON + 4] = cbxAlarmFr->isChecked();
    eep[EEPROM_ALARM_MON + 5] = cbxAlarmSa->isChecked();
    eep[EEPROM_ALARM_MON + 6] = cbxAlarmSu->isChecked();

    for (int i = 0; i < 7; i++)
        updateHexTable(EEPROM_ALARM_MON + i);
}

void MainWindow::setAlarmTimeout(int value)
{
    eep[EEPROM_ALARM_TIMEOUT] = (unsigned char)value;
    updateHexTable(EEPROM_ALARM_TIMEOUT);
}

