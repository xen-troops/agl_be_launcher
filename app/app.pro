TARGET = agl_surface_switcher
QT = quickcontrols2 dbus

SOURCES = main.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libhomescreen qlibwindowmanager qtappfw

RESOURCES += \
    agl_surface_switcher.qrc

include(app.pri)
