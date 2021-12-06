#-------------------------------------------------
#
# Project created by QtCreator 2017-04-02T17:49:25
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++11   //
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = helloworld
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES +=\
        mainwindow.cpp \
    Src/Add.cpp \
    Src/aimd-rtt-estimator.cpp \
    Src/aimd-statistics-collector.cpp \
    Src/consumer.cpp \
    Src/data-fetcher.cpp \
    Src/discover-version-fixed.cpp \
    Src/discover-version-iterative.cpp \
    Src/discover-version.cpp \
    Src/ndncatchunks.cpp \
    Src/options.cpp \
    Src/pipeline-interests-aimd.cpp \
    Src/pipeline-interests-fixed-window.cpp \
    Src/pipeline-interests.cpp \
    nfd_start.cpp \
    filedlg.cpp \
    filedlg1.cpp \
    Src/md5.cpp \
    Src/ndn-thread.cpp \
    Src/ndn-catsegment-thread.cpp \
    Src/PipelineCalCulateSpeed.cpp \
    Src/PipelineCalCulateSpeed-Upload.cpp \
    Src/Publisher.cpp \
    Src/ndnputsegments.cpp

HEADERS  += mainwindow.h \
    Src/Add.h \
    Src/aimd-rtt-estimator.hpp \
    Src/aimd-statistics-collector.hpp \
    Src/common.hpp \
    Src/consumer.hpp \
    Src/data-fetcher.hpp \
    Src/discover-version-fixed.hpp \
    Src/discover-version-iterative.hpp \
    Src/discover-version.hpp \
    Src/ndncatchunks.hpp \
    Src/options.hpp \
    Src/pipeline-interests-aimd.hpp \
    Src/pipeline-interests-fixed-window.hpp \
    Src/pipeline-interests.hpp \
    Src/version.hpp \
    nfd_start.h \
    filedlg.h \
    filedlg1.h \
    Src/md5.h \
    Src/ndn-thread.hpp \
    Src/ndn-catsegment-thread.hpp \
    Src/PipelineCalCulateSpeed.hpp \
    Src/PipelineCalCulateSpeed-Upload.hpp \
    Src/PipelineCalCulateSpeed-Upload.hpp \
    Src/Publisher.hpp \
    Src/ndnputsegments.hpp

FORMS    += mainwindow.ui \
    nfd_start.ui \
    filedlg.ui \
    filedlg1.ui




#unix|win32: LIBS += -lboost_filesystem

#LIBS += -L/usr/lib/x86_64-linux-gnu/libboost_system.so.1.58.0

unix|win32: LIBS += -lboost_system

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/release/ -lndn-cxx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/debug/ -lndn-cxx
else:unix: LIBS += -L$$PWD/../../../../usr/local/lib/ -lndn-cxx

INCLUDEPATH += $$PWD/../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../usr/local/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/python2.7/config-x86_64-linux-gnu/release/ -lpython2.7
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/python2.7/config-x86_64-linux-gnu/debug/ -lpython2.7
else:unix: LIBS += -L$$PWD/../../../../../../usr/lib/python2.7/config-x86_64-linux-gnu/ -lpython2.7

INCLUDEPATH += $$PWD/../../../../../../usr/include/python2.7
DEPENDPATH += $$PWD/../../../../../../usr/include/python2.7
