QT += core gui widgets

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    algorithms.cpp \
    gui/draggablecircle.cpp \
    gui/widget.cpp \
    main.cpp \
    mylinef.cpp

HEADERS += \
    algorithms.h \
    gui/draggablecircle.h \
    gui/flexibledoublespinbox.h \
    gui/widget.h \
    mylinef.h \
    tests.h

FORMS += \
	gui/widget.ui
