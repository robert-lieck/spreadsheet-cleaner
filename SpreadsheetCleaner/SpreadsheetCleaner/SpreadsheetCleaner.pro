#-------------------------------------------------
#
# Project created by QtCreator 2015-08-15T22:13:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpreadsheetCleaner
TEMPLATE = app


SOURCES += main.cpp\
        SpreadsheetCleaner.cpp \
    SpreadsheetModel.cpp

HEADERS  += SpreadsheetCleaner.h \
    SpreadsheetModel.h

FORMS    += SpreadsheetCleaner.ui

QMAKE_CXXFLAGS += -std=c++0x
