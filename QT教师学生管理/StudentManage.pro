QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    leftpanel.cpp \
    sidebar.cpp \
    studentgrid.cpp

HEADERS += \
    mainwindow.h \
    leftpanel.h \
    sidebar.h \
    studentgrid.h
