#Specify the exec name
TARGET=Weathering
#put the .o files in
OBJECTS_DIR=$$PWD/obj
#core Qt libs to use add more if needed
QT+=gui opengl core

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
        cache()
        DEFINES +=QT5BUILD
}
# where to put moc auto generated files
MOC_DIR=moc
# on a mac we don't create a .app bundle file ( for ease of multiplatform use)
CONFIG-=app_bundle

#add source files
SOURCES+= $$PWD/src/*.cpp

#add header files
HEADERS+= $$PWD/include/*.h

#add include path for Qt and make
INCLUDEPATH+=./include
#Where our exe is going to live
DESTDIR=$$PWD
# add other files
OTHER_FILES+= $$PWD/README.md \
              $$PWD/shaders/*.glsl \
              $$PWD/models/*.obj \
#default to console app
CONFIG+=console
# note each command you add needs a ; as it will be run as a single line
# first check if we are shadow building or not easiest way is to check out against current
!equals(PWD, $${OUT_PWD}){
        copydata.commands = echo "creating destination dirs" ;
        # now make a dir
        copydata.commands += mkdir -p $$OUT_PWD/shaders ;
        copydata.commands += echo "copying files" ;
        # then copy the files
        copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/ ;
        # now make sure the first target is built before copy
        first.depends = $(first) copydata
        export(first.depends)
        export(copydata.commands)
        # now add it as an extra target
        QMAKE_EXTRA_TARGETS += first copydata
}
# use this to suppress some warning from boost
QMAKE_CXXFLAGS_WARN_ON += "-Wno-unused-parameter"
# basic compiler flags (not all appropriate for all platforms)
QMAKE_CXXFLAGS+= -msse -msse2 -msse3
macx:QMAKE_CXXFLAGS+= -arch x86_64
macx:INCLUDEPATH+=/usr/local/Cellar/boost/1.56.0/include
linux-g++:QMAKE_CXXFLAGS +=  -march=native
linux-g++-64:QMAKE_CXXFLAGS +=  -march=native
# define the _DEBUG flag for the graphics lib
DEFINES +=NGL_DEBUG
DEFINES +=PWL_DEBUG

unix:LIBS += -L/usr/local/lib
# add the ngl lib
#unix:LIBS += -lglut32
unix:LIBS +=  -L/$(HOME)/NGL/lib -l NGL
unix:LIBS +=  -L/$(HOME)/PWL/lib -lPWL

# now if we are under unix and not on a Mac (i.e. linux)
linux-*{
                linux-*:QMAKE_CXXFLAGS +=  -march=native
                DEFINES += LINUX
}
DEPENDPATH+=include
# if we are on a mac define DARWIN
macx:DEFINES += DARWIN
# this is where to look for includes
INCLUDEPATH += $$(HOME)/NGL/include/
INCLUDEPATH += $$(HOME)/PWL/include/

win32: {
        PRE_TARGETDEPS+=C:/NGL/lib/NGL.lib
        PRE_TARGETDEPS+=C:/PWL/lib/PWL.lib
        INCLUDEPATH+=-I c:/boost
        DEFINES+=GL42
        DEFINES += WIN32
        DEFINES+=_WIN32
        DEFINES+=_USE_MATH_DEFINES
        LIBS += -LC:/NGL/lib/ -lNGL
        LIBS += -LC:/PWL/lib/ -lPWL
        DEFINES+=NO_DLL
}

