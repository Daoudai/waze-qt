#-------------------------------------------------
#
# Project created by QtCreator 2011-10-26T18:40:46
#
#-------------------------------------------------

QT       += core gui sql declarative network webkit

TARGET = wazeqt
TEMPLATE = lib
CONFIG += staticlib mobility

MOBILITY = location systeminfo multimedia contacts

DEFINES += TOUCH_SCREEN SSD QTMOBILITY FREEMAP_IL PLAY_CLICK LOCALE_SAFE

INCLUDEPATH += \
    .. \
    ../address_search \
    ../editor \
    ../navigate \
    ../Realtime \
    ../ssd \
    ../websvc_trans

SOURCES += \
    roadmap_main.cc \
    roadmap_fileselection.cc \
    roadmap_canvas.cc \
    qt_progress.cc \
    qt_main.cc \
    qt_dialog.cc \
    qt_canvas.cc \
    roadmap_device.cc \
    roadmap_tile_storage_qtsql.cc \
    roadmap_native_keyboard.cc \
    roadmap_gpsqtm.cc \
    qt/qt_gpsaccessor.cc \
    qt/roadmap_sound.cc \
    qt/qt_sound.cc \
    qt/qt_contactslistmodel.cc \
    qt/roadmap_editbox.cc \
    qt/qt_keyboard_dialog.cpp \
    qt/tts_db_qtsqlite.cc \
    qt/roadmap_qtbrowser.cc \
    qt/qt_webview.cc \
    qt/roadmap_net.cc \
    qt/qt_network.cc \
    qt/roadmap_zlib.cc \
    qt/roadmap_spawn.cc \
    qt/roadmap_path.cc \
    qt/roadmap_file.cc

HEADERS += \
    qt_progress.h \
    qt_main.h \
    qt_dialog.h \
    qt_canvas.h \
    roadmap_gpsqtm.h \
    qt/qt_gpsaccessor.h \
    qt/qt_sound.h \
    qt/roadmap_qtmain.h \
    qt/qt_contactslistmodel.h \
    qt/qt_keyboard_dialog.h \
    qt/roadmap_qtbrowser.h \
    qt/qt_webview.h \
    qt/qt_network.h






















