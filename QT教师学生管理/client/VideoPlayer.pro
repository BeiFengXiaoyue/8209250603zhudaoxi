QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    topbar.cpp \
    sidebar.cpp \
    playerwidget.cpp \
    searchpage.cpp

HEADERS += \
    mainwindow.h \
    topbar.h \
    sidebar.h \
    playerwidget.h \
    searchpage.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
