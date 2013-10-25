QT       += xml network

QT       -= gui

TARGET = libqtcommon
TEMPLATE = lib

DEFINES += BT_EXPERIENCE_TODO_REVIEW_ME
DEFINES += MEDIAPLAYER_DISABLE_HARDWARE_FUNCTIONS MEDIAPLAYER_MULTIPLE_PLAYERS

INCLUDEPATH += ../libqtdevices

SOURCES += connectiontester.cpp \
	list_manager.cpp \
	mediaplayer.cpp \
	scaleconversion.cpp \
	scenevodevicescond.cpp \
	signalshandler.cpp \
	treebrowser.cpp \
	xmlclient.cpp \
	xmldevice.cpp \
	entryinfo.cpp

HEADERS += connectiontester.h \
	list_manager.h \
	mediaplayer.h \
	scaleconversion.h \
	scenevodevicescond.h \
	signalshandler.h \
	treebrowser.h \
	xmlclient.h \
	xmldevice.h \
	entryinfo.h

