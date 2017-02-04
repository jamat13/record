TEMPLATE = app
TARGET = record
QT += sql

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport widgets
}

DEPENDPATH += .

# QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
# QMAKE_MAC_SDK = macosx10.11

CONFIG += release
# CONFIG += debug

macx {
    MAKEFILE = Makefile.qmake
}

# To get tty qDebug messages
# CONFIG += console

# Input
HEADERS += record.h \
	   helpers.h \
	   report.h \
	   night.h \
	   regular.h

FORMS +=   record.ui \
	   helpers.ui \
	   report.ui \
	   night.ui \
	   regular.ui

SOURCES += main.cpp \
           record.cpp \
	   helpers.cpp \
	   report.cpp \
	   night.cpp \
	   regular.cpp

ICON = record.icns
# RESOURCES = conscriptor.qrc

win32:DEFINES += QT_DLL

UI_DIR = build
MOC_DIR = build
OBJECTS_DIR = build

win32-g++: {
    win32:OBJECTS_DIR   = mingw_obj
#    win32:LIBS  += -L../serial/mingw -lqextserialport
}


macx {
    APP_QSS_FILES.files = record.qss
    APP_QSS_FILES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_QSS_FILES
}
