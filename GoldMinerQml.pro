QT += quick

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(QFtp/QFtp.pri)

HEADERS += \
    source/FindDevice/finddevicecontrollerbase.h \
    source/FindDevice/finddeviceexternalapp.h \
    source/FindDevice/finddevicesimulation.h \
    source/configcontroller.h \
    source/core.h \
    source/devicecontroller.h \
    source/devicemodel.h \
    source/ftpcontroller.h \
    source/myfunc.h

SOURCES += \
        source/FindDevice/finddevicecontrollerbase.cpp \
        source/FindDevice/finddeviceexternalapp.cpp \
        source/FindDevice/finddevicesimulation.cpp \
        source/configcontroller.cpp \
        source/core.cpp \
        source/devicecontroller.cpp \
        source/devicemodel.cpp \
        main.cpp \
        source/ftpcontroller.cpp \
        source/myfunc.cpp

RESOURCES += qml.qrc \
    otherResource.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

