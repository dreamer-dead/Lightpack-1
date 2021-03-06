#-------------------------------------------------
#
# Project created by QtCreator 2013-06-11T16:47:52
#
#-------------------------------------------------

QT       += widgets

DESTDIR = ../lib
TARGET = grab
TEMPLATE = lib
CONFIG += staticlib

include(../build-config.prf)

# Grabber types configuration
include(configure-grabbers.prf)

LIBS += -lprismatik-math

INCLUDEPATH += ./include \
               ../math/include \
               ..

CONFIG(gcc):QMAKE_CXXFLAGS += -std=c++11
CONFIG(clang) {
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
    LIBS += -stdlib=libc++
}

DEFINES += $${SUPPORTED_GRABBERS}
# Linux/UNIX platform
unix:!macx {
    contains(DEFINES, X11_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/X11Grabber.hpp
        GRABBERS_SOURCES += X11Grabber.cpp
    }
}

# Mac platform
macx {
    contains(DEFINES, MAC_OS_CG_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/MacOSGrabber.hpp
        GRABBERS_SOURCES += MacOSGrabber.cpp
    }
}

# Windows platform
win32 {
    contains(DEFINES, WINAPI_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/WinAPIGrabber.hpp
        GRABBERS_SOURCES += WinAPIGrabber.cpp
    }

    contains(DEFINES, WINAPI_EACH_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/WinAPIGrabberEachWidget.hpp
        GRABBERS_SOURCES += WinAPIGrabberEachWidget.cpp
    }

    contains(DEFINES, D3D9_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/D3D9Grabber.hpp
        GRABBERS_SOURCES += D3D9Grabber.cpp
    }

    contains(DEFINES, D3D10_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/D3D10Grabber.hpp
        GRABBERS_SOURCES += D3D10Grabber.cpp
    }
}

# Common Qt grabbers
contains(DEFINES, QT_GRAB_SUPPORT) {
    GRABBERS_HEADERS += \
        include/QtGrabberEachWidget.hpp \
        include/QtGrabber.hpp

    GRABBERS_SOURCES += \
        QtGrabberEachWidget.cpp \
        QtGrabber.cpp
}

HEADERS += \
    include/calculations.hpp \
    include/GrabbedArea.hpp \
    include/GrabberBase.hpp \
    include/ColorProvider.hpp \
    include/GrabberContext.hpp \
    $${GRABBERS_HEADERS}

SOURCES += \
    calculations.cpp \
    GrabberBase.cpp \
    GrabberContext.cpp \
    include/ColorProvider.cpp \
    $${GRABBERS_SOURCES}

win32 {
    !isEmpty( DIRECTX_SDK_DIR ) {
        # This will suppress gcc warnings in DX headers.
        CONFIG(gcc) {
            QMAKE_CXXFLAGS += -isystem "\"$${DIRECTX_SDK_DIR}/Include\""
        } else {
            INCLUDEPATH += "\"$${DIRECTX_SDK_DIR}/Include\""
        }
    }

    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    CONFIG(msvc) {
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    }

    HEADERS += \
            ../common/msvcstub.h \
            include/WinUtils.hpp \
            include/WinDXUtils.hpp
    SOURCES += \
            WinUtils.cpp \
            WinDXUtils.cpp
}

macx {
    INCLUDEPATH += /System/Library/Frameworks
}

OTHER_FILES += \
    configure-grabbers.prf
