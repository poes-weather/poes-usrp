# -------------------------------------------------
# Project created by QtCreator 2009-11-20T15:22:48
# -------------------------------------------------
TARGET = POES-USRP
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    decoder/hrptblock.cpp \
    satellite/station/stationdialog.cpp \
    satellite/station/station.cpp \
    utils/plist.cpp \
    satellite/kepler/tledialog.cpp \
    satellite/predict/Satellite.cpp \
    settings.cpp \
    utils/utils.cpp \
    satellite/satutil.cpp \
    satellite/orbitdata/orbitdialog.cpp \
    satellite/predict/satpassdialog.cpp \
    satellite/trackthread.cpp \
    satellite/track/trackwidget.cpp \
    imagewidget.cpp \
    satellite/active/activesatdialog.cpp \
    rig/rigdialog.cpp \
    rig/rig.cpp \
    decoder/block.cpp \
    decoder/mn1lrptblock.cpp \
    decoder/ReedSolomon.cpp \
    decoder/lritblock.cpp \
    decoder/ljpeg/ljpegreader.cpp \
    decoder/ljpeg/ljpegdecompressor.cpp \
    decoder/ljpeg/ljpegcomponent.cpp \
    decoder/ljpeg/ljpeghuffmantable.cpp \
    decoder/mn1hrptblock.cpp \
    rig/rotorpindialog.cpp \
    rig/rotor.cpp \
    rig/stepper.cpp \
    rig/gs232b.cpp \
    rig/alphaspid.cpp \
    rig/qextserialport/qextserialport.cpp \
    satellite/predict/satscript.cpp \
    decoder/fy1hrptblock.cpp \
    utils/textwindow.cpp \
    tools/gauge.cpp \
    tools/gps/gpsdialog.cpp \
    tools/gps/gps.cpp \
    rig/jrk.cpp \
    rig/jrkconfdialog.cpp \
    decoder/ahrptblock.cpp \
    decoder/cadu.cpp \
    utils/azeldialog.cpp \
    tools/cadusplitterdialog.cpp \
    rig/monstrum.cpp \
    satellite/property/satprop.cpp \
    satellite/property/satpropdialog.cpp \
    satellite/property/rgbconf.cpp \
    satellite/property/ndvi.cpp \
    rig/usb/usbdevice.cpp \
    rig/usb/tusb.cpp \
    rig/jrkusb.cpp
HEADERS += mainwindow.h \
    decoder/hrptblock.h \
    version.h \
    os.h \
    satellite/station/stationdialog.h \
    satellite/station/station.h \
    utils/plist.h \
    satellite/kepler/tledialog.h \
    satellite/predict/Satellite.h \
    satellite/predict/satcalc.h \
    settings.h \
    utils/utils.h \
    config.h \
    satellite/satutil.h \
    satellite/orbitdata/orbitdialog.h \
    satellite/predict/satpassdialog.h \
    satellite/trackthread.h \
    satellite/track/trackwidget.h \
    imagewidget.h \
    satellite/active/activesatdialog.h \
    rig/rigdialog.h \
    rig/rig.h \
    decoder/block.h \
    decoder/mn1lrptblock.h \
    decoder/ReedSolomon.h \
    decoder/RiceDecompression.h \
    decoder/PacketDecompression.h \
    decoder/lritblock.h \
    decoder/ljpeg/ljpegdecompressor.h \
    decoder/ljpeg/ljpegcomponent.h \
    decoder/ljpeg/ljpegreader.h \
    decoder/ljpeg/ljpeghuffmantable.h \
    decoder/ljpeg/ljpeg.h \
    decoder/mn1hrptblock.h \
    rig/rotorpindialog.h \
    rig/rotor.h \
    rig/stepper.h \
    rig/gs232b.h \
    rig/alphaspid.h \
    rig/qextserialport/qextserialport.h \
    satellite/predict/satscript.h \
    decoder/fy1hrptblock.h \
    utils/textwindow.h \
    tools/gauge.h \
    tools/gps/gpsdialog.h \
    tools/gps/gps.h \
    rig/jrk.h \
    rig/jrkconfdialog.h \
    decoder/ahrptblock.h \
    decoder/cadu.h \
    utils/azeldialog.h \
    tools/cadusplitterdialog.h \
    rig/monstrum.h \
    satellite/property/satprop.h \
    satellite/property/satpropdialog.h \
    satellite/property/rgbconf.h \
    satellite/property/ndvi.h \
    rig/usb/usbdevice.h \
    rig/usb/tusb.h \
    rig/jrkusb.h \
    rig/jrk_protocol.h
DEFINES += _CRT_SECURE_NO_WARNINGS
FORMS += mainwindow.ui \
    satellite/station/stationdialog.ui \
    satellite/kepler/tledialog.ui \
    satellite/orbitdata/orbitdialog.ui \
    satellite/predict/satpassdialog.ui \
    satellite/track/trackwidget.ui \
    imagewidget.ui \
    satellite/active/activesatdialog.ui \
    rig/rigdialog.ui \
    rig/rotorpindialog.ui \
    utils/textwindow.ui \
    tools/gps/gpsdialog.ui \
    rig/jrkconfdialog.ui \
    utils/azeldialog.ui \
    tools/cadusplitterdialog.ui \
    satellite/property/satpropdialog.ui
RESOURCES += application.qrc
QT += network
INCLUDEPATH += decoder \
    satellite \
    satellite/predict \
    satellite/kepler \
    satellite/station \
    satellite/orbitdata \
    satellite/track \
    satellite/active \
    satellite/property \
    utils \
    rig \
    rig/usb \
    rig/qextserialport \
    decoder/ljpeg \
    tools \
    tools/gps

# --------------------------------------------------------------------------------
# uncomment the two lines below if you have installed image plugins and clean + build
# QTPLUGIN += qjpeg qgif qtiff qmng
# DEFINES += HAVE_IMAGE_PLUGINS
# --------------------------------------------------------------------------------

LIBS += -Ldecoder/LritRice
#DEFINES += DEBUG_GPS

# --------------------------------------------------------------------------------
# Mr Linus Thorvalds (linux) settings
# --------------------------------------------------------------------------------
unix { 
    DEFINES += _TTY_LINUX_
    SOURCES += rig/OakFeatureReports.cpp \
        rig/OakHidBase.cpp \
        rig/qextserialport/posix_qextserialport.cpp
    HEADERS += rig/OakFeatureReports.h \
        rig/OakHidBase.h
    INCLUDEPATH += /usr/local/include

    # libusb
    INCLUDEPATH += /usr/include
    LIBS += -lusb

    # DSP and FEC Library, http://www.ka9q.net/code/fec/
    #DEFINES += HAVE_LIBFEC
    #LIBS += -L/usr/local/lib -lfec
}


# --------------------------------------------------------------------------------
# Mac OSX settings
# --------------------------------------------------------------------------------

macx {

}


# --------------------------------------------------------------------------------
# Mr Bill Gates (win32) settings
# --------------------------------------------------------------------------------
win32 { 
    DEFINES += __HRPT_WIN__ _TTY_WIN_
    RC_FILE = POES-Decoder.rc
    HEADERS += rig/WinIo.h
    LIBS += -LC:/Programming/HRPT/HRPT-Decoder/branches/1.0.0.6 -lWinIo
    SOURCES += rig/qextserialport/win_qextserialport.cpp
    
    debug:
    
    release:
}


OTHER_FILES += 




