#-------------------------------------------------
#
# Project created by QtCreator 2011-04-20T23:09:23
#
#-------------------------------------------------

TARGET = Maya
TEMPLATE = app
INCLUDEPATH += ../PtexLib \
				../ZLib \
				$$(MAYA_LOCATION)/include

SOURCES += \
    PtexUVNode.cpp \
    PtexUVCmd.cpp \
    PtexColorNode.cpp \
    PluginMain.cpp \
    CmpMeshModifierNode.cpp \
    CmpMeshModifierCmd.cpp

OTHER_FILES += \
    AEptexTemplate.mel

HEADERS += \
    PtexUVNode.h \
    PtexUVCmd.h \
    PtexColorNode.h \
    CmpMeshModifierNode.h \
    CmpMeshModifierCmd.h
