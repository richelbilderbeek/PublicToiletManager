# Qt version of Public Toilet Manager.
# The original C++Builder/VCL version is PTM5Beta.bpr
#
# Build with:
#
#   qmake publictoiletmanager.pro
#   make           # nmake or jom when using MSVC
TEMPLATE = app
TARGET = publictoiletmanager
QT += core gui widgets multimedia
CONFIG += c++17

SOURCES += \
  qtmain.cpp \
  qtpublictoiletmanagerwidget.cpp

HEADERS += \
  qtpublictoiletmanagerwidget.h

RESOURCES += publictoiletmanager.qrc
