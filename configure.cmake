
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
	message(STATUS "CMAKE_BUILD_TYPE was set to do a Release build. For debug builds, specify -DCMAKE_BUILD_TYPE=Debug on the commandline")
endif()

# CMAKE CONFIGURATION
#####################
project(BSuite CXX)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)

# general path configuration
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE_LOWER})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE_LOWER})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE_LOWER})

if(NOT QTCREATOR)
	message(STATUS "In QtCreator, when running cmake, make sure you specify -DQTCREATOR=1 as additional commandline arguments to have the maya headers parsed.")
else()
	message(WARNING "The QTCREATOR variable is set - disable it in your builds, otherwise the maya plugins may not work correctly. It needs to be set when updating the creator project only !")
endif()

# add the profiling configuration. Its essentially the release config, but
# compiles with profiling instructions, enabling gprof
if(UNIX)
	if(CMAKE_CONFIGURATION_TYPES)
		list(APPEND CMAKE_CONFIGURATION_TYPES Profile)
		list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
			"Add the configurations that we need"
			FORCE)
	endif()
endif() # unix


# COMPILER FLAGS
##################
if(UNIX)
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
		
	# make sure we see everything! Don't export anything by default
	# for now, without -pedantic, as it prevents compilation of maya thanks to 'extra ;'  - don't know how to disable this
	# Also, the architecture is hardcoded, this is a problem for older maya versions which where 32 bit on osx
	# TODO: put the arch in the config, but make sure its correct on osx for maya plugins automatically
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -Wall -Wno-long-long -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-comment -Wcast-qual")
	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -fno-gnu-keywords -fpascal-strings")
	endif()
endif() #unix

if(${CMAKE_BUILD_TYPE} MATCHES Debug OR ${CMAKE_BUILD_TYPE} MATCHES Profile)
	add_definitions(-DDEBUG)
endif()

# CMAKE SETUP AND CONFIGURATION
###############################
# setup modules
include(FindDoxygen DOXYGEN_SKIP_DOT)

if(DOXYGEN)
	set(DOXYFILE_SOURCE_DIRS src)
	include(UseDoxygen.cmake)
else(DOXYGEN)
	message(WARNING "Doxygen was not found - documentation will not be built")
endif()


# MAYA CONFIGURATION
######################
set(DEFAULT_MAYA_VERSIONS 2012 2011 2010 2009 2008)
set(DEFAULT_MAYA_LIBRARIES Foundation OpenMaya OpenMayaAnim OpenMayaRender OpenMayaFX OpenMayaUI)

if(UNIX AND NOT APPLE AND ${CMAKE_CXX_SIZEOF_DATA_PTR} EQUAL 8)
	set(DEFAULT_MAYA_INSTALL_SUFFIX -x64)
endif()

set(CUSTOM_DEFINITIONS -DREQUIRE_IOSTREAM -D_BOOL)

if(UNIX)
	if(APPLE)
		set (MAYA_INSTALL_BASE_DEFAULT /Applications/Autodesk)
		list(APPEND CUSTOM_DEFINITIONS -DOSMac_ -DOSMacOSX_ -DOSMac_MachO_)
	else()
		set (MAYA_INSTALL_BASE_DEFAULT /usr/autodesk)
		list(APPEND CUSTOM_DEFINITIONS -DLINUX)
	endif()
else()
	set(MAYA_INSTALL_BASE_DEFAULT "c:/Program Files/Autodesk")
	list(APPEND CUSTOM_DEFINITIONS -D_AFXDLL -D_MBCS -DNT_PLUGIN)
	
	set(LOCAL_WARNING_FLAGS /W3)
	set(LOCAL_RTTI_FLAGS /GR)
endif()

add_definitions(${CUSTOM_DEFINITIONS})

set(MAYA_INSTALL_BASE_PATH "${MAYA_INSTALL_BASE_DEFAULT}" CACHE PATH
    "Path containing all your maya installations, like /usr/autodesk or /Applications/Autodesk/")

set(MAYA_INSTALL_BASE_SUFFIX "${DEFAULT_MAYA_INSTALL_SUFFIX}" CACHE STRING
    "Suffix to append to maya installation directories, like -x64 on 64 bit systems on linux")

set(MAYA_BUILD_VERSIONS "${DEFAULT_MAYA_VERSIONS}" CACHE STRING
    "a semicolon separated list of all maya versions you would like to have build, like 2008;2009 ")
    
if(NOT EXISTS ${MAYA_INSTALL_BASE_PATH})
	message(SEND_ERROR "Maya install location is not set or does not exist at '${MAYA_INSTALL_BASE_PATH}, check your configuration")
endif()

if(APPLE)
	set(CMAKE_FRAMEWORK_PATH "/Developer/SDKs/MacOSX10.6.sdk" CACHE STRING
		"Directory containing all the osx 10.6 headers")
	
	if(NOT EXISTS ${CMAKE_FRAMEWORK_PATH})
		message(SEND_ERROR "SYSROOT include directory not found at ${CMAKE_FRAMEWORK_PATH} - please configure it in your cmake cache and try again")
	endif()
endif()


# TESTING
##########
enable_testing()

set(TEST_TMRV_PATH "" CACHE FILEPATH
	"Path to the tmrv executable, providing facilities to test maya plugins using the mrv development framework")

set(TEST_MAYA_VERSION 2010 CACHE STRING
	"Maya version to use for testing")
