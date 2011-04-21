# CMAKE CONFIGURATION
#####################
project(PtexMaya CXX)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)

# general path configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

# add the profiling configuration. Its essentially the release config, but
# compiles with profiling instructions, enabling gprof
if(CMAKE_CONFIGURATION_TYPES)
	list(APPend CMAKE_CONFIGURATION_TYPES Profile)
	list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
		"Add the configurations that we need"
		FORCE)
endif()

set( CMAKE_CXX_FLAGS_PROFILE "-O3 -DNDEBUG -pg" CACHE STRING
    "Flags used by the C++ compiler during PROFILE builds.")
set( CMAKE_EXE_LINKER_FLAGS_PROFILE
    "-pg" CACHE STRING
    "Flags used for linking binaries during PROFILE builds.")
set( CMAKE_SHARED_LINKER_FLAGS_PROFILE
    "-pg" CACHE STRING
    "Flags used by the shared libraries linker during PROFILE builds.")
mark_as_advanced(
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE
    CMAKE_SHARED_LINKER_FLAGS_PROFILE)

# CMAKE setUP AND CONFIGURATION
###############################
# setup modules
include(FindDoxygen DOXYGEN_SKIP_DOT)

# setup compiler
if(UNIX)
	# show all warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\ -Wall")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Profile")
	add_definitions(-DDEBUG)
endif()

if(DOXYGEN)
	set(DOXYFILE_SOURCE_DIRS src)
	include(UseDoxygen.cmake)
else(DOXYGEN)
	message(WARNING "Doxygen was not found - documentation will not be built")
endif()

# MAYA CONFIGURATION
######################
set(MAYA_VERSIONS 2012 2011 2010 2009 2008)
set(PTEX_LIBRARIES Ptex z)
set(MAYA_LIBRARIES Foundation OpenMaya OpenMayaAnim OpenMayaRender)
set(CUSTOM_DEFINITIONS -DREQUIRE_IOSTREAM -D_BOOL)

if(UNIX)
	set (MAYA_INSTALL_BASE_DEFAULT /usr/autodesk)
elseif(APPLE)
	set (MAYA_INSTALL_BASE_DEFAULT /Applications/Autodesk)
else(UNIX)
	set(MAYA_INSTALL_BASE_DEFAULT "c:/Program Files")
	lists(APPEND CUSTOM_DEFINITIONS -D_AFXDLL -D_MBCS -DNT_PLUGIN)
	
	set(LOCAL_WARNING_FLAGS /W3)
	set(LOCAL_RTTI_FLAGS /GR)
endif()

add_definitions(${CUSTOM_DEFINITIONS})

set(MAYA_INSTALL_BASE_PATH ${MAYA_INSTALL_BASE_DEFAULT} CACHE PATH
    "Path containing all your maya installations, like /usr/autodesk or /Applications/Autodesk/")

set(MAYA_INSTALL_BASE_SUFFIX "" CACHE STRING
    "Suffix to append to maya installation directories, like -x64 on 64 bit systems on linux")
    
set(PTEX_INCLUDE_DIR "" CACHE PATH
	"Directory containing the ptex headers")

set(PTEX_LIBRARY_DIR "" CACHE PATH
	"Directory containing the ptex headers")

# VERIFY PREREQUESITES
######################
if(NOT EXISTS ${PTEX_INCLUDE_DIR})
	message(SEND_ERROR "PTex header directory must be set, check your configuration")
endif()

if(NOT EXISTS ${PTEX_LIBRARY_DIR})
	message(SEND_ERROR "PTex library directory must be set, check your configuration")
endif()

if(NOT EXISTS ${MAYA_INSTALL_BASE_PATH})
	message(SEND_ERROR "Maya install location is not set, check your configuration")
endif()

# CONFIGURE Compiler/linker
###########################
include_directories(${PTEX_INCLUDE_DIR} src test SYSTEM)
link_directories(${PTEX_LIBRARY_DIR})

# CONFIGURE LIBRARIES
#####################


# TEST SETUP
############
enable_testing()

include(fun.cmake)

add_subdirectory(src)
add_subdirectory(test)


