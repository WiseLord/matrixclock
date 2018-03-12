#-------------------------------------------------
#
# Project created by QtCreator 2015-08-12T14:57:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = matrixclockeditor
TEMPLATE = app

SOURCES += main.cpp\
	mainwindow.cpp \
	lcdconverter.cpp \
	aboutdialog.cpp \
	fontpixel.cpp

HEADERS  += mainwindow.h \
	lcdconverter.h \
	aboutdialog.h \
	fontpixel.h \
	timespinbox.h
#    defines.h

FORMS    += mainwindow.ui \
	aboutdialog.ui

RESOURCES += \
	res.qrc \
	ts.qrc

TRANSLATIONS += \
    ts/editor_ru.ts \
    ts/editor_by.ts

tr.commands = lupdate \"$$_PRO_FILE_\" && lrelease \"$$_PRO_FILE_\"
commands = lupdate \"$$_PRO_FILE_\" && lrelease \"$$_PRO_FILE_\"

PRE_TARGETDEPS += tr
QMAKE_EXTRA_TARGETS += tr
