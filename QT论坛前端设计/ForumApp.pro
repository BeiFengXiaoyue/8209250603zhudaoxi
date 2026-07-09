QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sidebar.cpp \
    commentarea.cpp

HEADERS += \
    mainwindow.h \
    sidebar.h \
    commentarea.h
