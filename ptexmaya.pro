#-------------------------------------------------
#
# Project created by QtCreator 2011-04-20T23:09:23
#
#-------------------------------------------------

TARGET = Maya
TEMPLATE = app
DEFINES = REQUIRE_IOSTREAM \
			LINUX
INCLUDEPATH += ../PtexLib \
				../ZLib \
				$$(MAYA_LOCATION)/include \
				$$(PTEX_INCLUDE_DIR)

SOURCES += \
    PtexUVNode.cpp \
    PtexUVCmd.cpp \
    PtexColorNode.cpp \
    PluginMain.cpp \
    CmpMeshModifierNode.cpp \
    CmpMeshModifierCmd.cpp \
    src/maya_main.cpp \
    src/visnode.cpp \
    src/util.cpp

OTHER_FILES += \
    AEptexTemplate.mel

HEADERS += \
    PtexUVNode.h \
    PtexUVCmd.h \
    PtexColorNode.h \
    CmpMeshModifierNode.h \
    CmpMeshModifierCmd.h \
    src/visnode.h \
    src/util.h \
    src/typ.h
