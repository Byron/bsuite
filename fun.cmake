
include(CMakeParseArguments)

################
# CONFIGURATION
################
set(ADD_PROJECT_NO_ARG_OPTS
		WITHOUT_EXCEPTIONS)
set(ADD_PROJECT_SINGLE_ARG_OPTS
		NAME
		TYPE
		LIBRARY_SUFFIX
		LIBRARY_PREFIX)
set(ADD_PROJECT_MULTI_ARG_OPTS
		SOURCE_FILES
		SOURCE_DIRS
		INCLUDE_DIRS
		LIBRARY_DIRS
		LINK_LIBRARIES
		DEFINES)


# =============================================================
# find_sourcefiles_recursive(RESULT_VARIABLE DIRECTORIES EXCLUDE_PATHS)
# Set RESULT_VARIABLE to a list of file paths relative to the given list of DIRECTORIES
# which should be a relative path. All paths are based on the current source directory.
# ======================================
function(find_sourcefiles_recursive RESULT_VARIABLE DIRECTORIES)
	foreach(SUBDIR ${DIRECTORIES})
		get_filename_component(ABSOLUTE_SUBDIR ${SUBDIR} ABSOLUTE)
		if(NOT EXISTS ${ABSOLUTE_SUBDIR})
			message(WARNING "In project ${PROJECT_ID}: source directory '${SUBDIR}' did not exist at ${ABSOLUTE_SUBDIR}. Please check your cmake configuration")
		endif()
		
		set(FILEGLOBS)
		foreach(FILESPEC *.cpp *.cxx *.c *.h *.hpp)
			list(APPEND FILEGLOBS ${SUBDIR}/${FILESPEC})
		endforeach()
		file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${FILEGLOBS})
		list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
	endforeach()
	set(${RESULT_VARIABLE} ${ALL_SOURCE_FILES} PARENT_SCOPE)
endfunction()

# ======================================
# append_to_target_property(TARGET PROPERTY VALUES)
# Set any property on target level instead of on directory 
# level.
# TARGET is the target name
# PROPERTY the target level property name
# VALUES is a semicolon separated list of values to set
# ======================================
function(append_to_target_property TARGET PROPERTY VALUES SPACE_SEPARATED)
	get_target_property(CURR_VALUES ${TARGET} ${PROPERTY})
	if (CURR_VALUES)
		if(SPACE_SEPARATED)
			set(CURR_VALUES "${CURR_VALUES} ${VALUES}")
		else()
			list(APPEND CURR_VALUES ${VALUES})
		endif()
	else()
		set(CURR_VALUES ${VALUES})
	endif()
	set_target_properties(${TARGET} PROPERTIES ${PROPERTY} "${CURR_VALUES}")
endfunction()



# ======================================
#add_project(
#					NAME name
#					TYPE EXECUTABLE|STATIC|MODULE|SHARED
#					[LIBRARY_SUFFIX .ext]
#					[LIBRARY_PREFIX lib]
#					SOURCE_FILES file1 [...fileN]
#					SOURCE_DIRS dir1 [...dirN]
#					INCLUDE_DIRS dir1 [... dirN]
#					LIBRARY_DIRS dir1 [... dirN]
#					LINK_LIBRARIES lib1 [...libN]
#					DEFINES def1 [...defN]
#					WITHOUT_EXCEPTIONS
#					WITH_TEST
#				)
# Generic function to add a project of a certain type.
# NAME
#	The name of the project. It can also appear as LINK_LIBRARIES argument
#	in one of the other projects to link against it, which is useful if TYPE
#	is an ARCHIVE.
# TYPE
#	EXECUTABLE if the target should be an executable that can be run from the
#	commandline
#	STATIC if the target should be a static library that others can link against.
#	MODULE if the target should be a shared module, which is supposed to be loaded
#	by calls to dlopen().
#	SHARED if the target should be a shared library that can be linked in by other
#	executables (or libraries) using the system's linker at program startup.
#	This is useful if the same code is used by multiple shared libararies or if
#	you want to makes sure that certain objects only exist exactly once in a program.
# LIBRARY_SUFFIX (optional)
#	If set, the suffix will be used as file extension in case of 
#	MODULE or SHARED library target's output names.
#	If unset, the system default will be used.
# LIBRARY_PREFIX (optional)
#	If set, the string will be prefixed to the output names of MODULE or SHARED
#	library targets.
# SOURCE_FILES (optional, if SOURCE_DIRS is set)
#	Explicit list of source files, relative to the current source directory
# SOURCE_DIRS (optional, if SOURCE_FILES is set)
#	A list of directories where source files should be searched recursively
#	If no source dir is set, the current one will be used
# INCLUDE_DIRS (optionsl)
#	List directories to be added to the include path
#	If no include directory is used, the current one will be used.
# LIBRARY_DIRS
#	List of directories in which the linker should look for libraries.
# LINK_LIBRARIES
#	List of library names that should be linked into your output file
# DEFINES
#	Defines that should be set for your project
# WITHOUT_EXCEPTIONS
#	If set, exceptions will be disabled.
function(add_project)
	cmake_parse_arguments(PROJECT 
						"${ADD_PROJECT_NO_ARG_OPTS}"
						"${ADD_PROJECT_SINGLE_ARG_OPTS}"
						"${ADD_PROJECT_MULTI_ARG_OPTS}"
						${ARGN})
	
	if(NOT PROJECT_NAME)
		message(SEND_ERROR "NAME must be set")
	endif()
	
	if (NOT PROJECT_TYPE)
		message(SEND_ERROR "TYPE must be set")
	endif()
	
	# FIGURE OUT TYPE
	##################
	if (${PROJECT_TYPE} MATCHES EXECUTABLE)
		set(IS_EXECUTABLE YES)
	elseif (${PROJECT_TYPE} MATCHES "STATIC|MODULE|SHARED")
		set(IS_LIBRARY YES)
	else()
		message(SEND_ERROR "project TYPE ${PROJECT_TYPE} is not supported")
	endif()
	
	# set source files
	if(NOT PROJECT_SOURCE_FILES)
		if(NOT PROJECT_SOURCE_DIRS)
			set(PROJECT_SOURCE_DIRS .)
		endif()
		find_sourcefiles_recursive(PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIRS}")
	endif()
	
	if(NOT PROJECT_INCLUDE_DIRS)
		set(PROJECT_INCLUDE_DIRS .)
	endif()
	
	if(NOT PROJECT_SOURCE_FILES)
		message(SEND_ERROR "Did not find any source file, or no source file specified. Use the SOURCE_FILES or SOURCE_DIRS parameter")
	endif()
	
	# For now, its just the same, maybe later its not ... 
	set(PROJECT_ID ${PROJECT_NAME})
	
	# DIRECTORY LEVEL CONFIGURATION
	################################
	# Should only run once
	include_directories(${PROJECT_INCLUDE_DIRS})
	link_directories(${PROJECT_LIBRARY_DIRS})
	
	message(STATUS "Setting up ${PROJECT_NAME} (${PROJECT_TYPE})")
	
	# CREATE TARGET
	###############
	if (IS_EXECUTABLE)
		add_executable(${PROJECT_ID}
						${PROJECT_SOURCE_FILES})
	else()
		add_library(${PROJECT_ID}
							${PROJECT_TYPE}
							${PROJECT_SOURCE_FILES})
	endif()
	# TARGET LEVEL CONFIGURATION
	#############################
	append_to_target_property(${PROJECT_ID} LINK_FLAGS "${LINK_DIR_FLAG}${MAYA_LIB_DIR}" YES)
	if (${CMAKE_BUILD_TYPE} MATCHES Release AND UNIX AND NOT APPLE AND NOT ${PROJECT_TYPE} MATCHES STATIC)
		append_to_target_property(${PROJECT_ID} LINK_FLAGS "-Wl,--strip-all,-O2" YES)
	endif()
	
	if(PROJECT_WITHOUT_EXCEPTIONS AND UNIX)
		# TODO: windows version ... 
		append_to_target_property(${PROJECT_ID} COMPILE_FLAGS -fno-exceptions YES)
	endif()
	
	target_link_libraries(${PROJECT_ID}
									${PROJECT_LINK_LIBRARIES})
	
	if(PROJECT_DEFINES)
		append_to_target_property(${PROJECT_ID} COMPILE_DEFINITIONS "${PROJECT_DEFINES}" NO)
	endif()
	
	set_target_properties(${PROJECT_ID} PROPERTIES
											OUTPUT_NAME 
													${PROJECT_NAME}
											CLEAN_DIRECT_OUTPUT 1)
	
	if (PROJECT_LIBRARY_PREFIX)
		set_target_properties(${PROJECT_ID} PROPERTIES
											PREFIX ${PROJECT_LIBRARY_PREFIX})
	endif()
	if (PROJECT_LIBRARY_SUFFIX)
		set_target_properties(${PROJECT_ID} PROPERTIES
											SUFFIX ${PROJECT_LIBRARY_SUFFIX})
	endif()
endfunction()


# ======================================
#add_maya_project(
#					NAME name
#					MAYA_VERSIONS version1 [versionN]
#					SOURCE_FILES file1 [...fileN]
#					SOURCE_DIRS dir1 [...dirN]
#					INCLUDE_DIRS dir1 [... dirN]
#					LIBRARY_DIRS dir1 [... dirN]
#					LINK_LIBRARIES lib1 [...libN]
#					DEFINES def1 [...defN]
#					WITHOUT_EXCEPTIONS
#					WITH_TEST
#				)
#
# This function allows to configure an average project and automatically add
# maya include and library directories to the path. Default maya libraries will be
# linked automatically, but you may add additional ones.
#
# NAME
# 	The name of the project - it will also be the basename of the executable
# MAYA_VERSIONS (optional)
# 	one or more version of maya to use for headers and libraries, e.g. 2011, 2008
# 	If unset, the globally configured list of maya versions is used
# WITH_TEST
#	If set, a python based test will be run which loads your compiled plugin.
#	In your mrv/nose test implementation, you will have to verify that your plugin
#	is loaded.
# All other arguments are documented in the add_project method
# ======================================
function(add_maya_project)
	cmake_parse_arguments(PROJECT 
						"WITH_TEST;${ADD_PROJECT_NO_ARG_OPTS}"
						"${ADD_PROJECT_SINGLE_ARG_OPTS}"
						"MAYA_VERSIONS;${ADD_PROJECT_MULTI_ARG_OPTS}"
						${ARGN})
	
	if(NOT PROJECT_NAME)
		message(SEND_ERROR "NAME must be set")
	endif()
	
	if(NOT PROJECT_MAYA_VERSIONS)
		set(PROJECT_MAYA_VERSIONS ${MAYA_BUILD_VERSIONS})
	endif()
	
	if(NOT PROJECT_MAYA_VERSIONS)
		message(SEND_ERROR "Not a single maya version set must be set")
	endif()
	
	# set source files
	if(NOT PROJECT_SOURCE_FILES)
		if(NOT PROJECT_SOURCE_DIRS)
			set(PROJECT_SOURCE_DIRS .)
		endif()
		find_sourcefiles_recursive(PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIRS}")
	endif()
	
	if(NOT PROJECT_INCLUDE_DIRS)
		set(PROJECT_INCLUDE_DIRS .)
	endif()
	
	if(NOT PROJECT_SOURCE_FILES)
		message(SEND_ERROR "Did not find any source file, or no source file specified. Use the SOURCE_FILES or SOURCE_DIRS parameter")
	endif()
	
	# HANDLE SUFFIX
	if(WIN32)
		set(PROJECT_SUFFIX .mll)
	elseif(APPLE)
		set(PROJECT_SUFFIX .bundle)
	else()
		set(PROJECT_SUFFIX .so)
	endif()
	
	# OSX HANDLING
	if(APPLE)
		set(LIB_INSERT "Maya.app/Contents/MacOS")
		set(INCLUDE_SUFFIX "devkit/include")
	else()
		set(LIB_INSERT "lib")
		set(INCLUDE_SUFFIX "include")
	endif()
	
	if(WIN32)
		set(LINK_DIR_FLAG "\L")
		set(INCL_DIR_FLAG "\I")
	else()
		set(LINK_DIR_FLAG "-L")
		set(INCL_DIR_FLAG "-I")
	endif()
	
	# TODO: just build an arglist for the more generic ADD_PROJECT - put our code around it.
	# Building argument lists is somewhat of a hassle, so we leave the working, but redundant code ... 
	
	# DIRECTORY LEVEL CONFIGURATION
	################################
	# Should only run once
	include_directories(${PROJECT_INCLUDE_DIRS})
	link_directories(${PROJECT_LIBRARY_DIRS})
	
	# assure we have agl
	if(APPLE)
		include_directories(${OSX_AGL_INCLUDE_DIR})
	endif()
	
	
	# FOR EACH MAYA VERSION TO GENERATE
	####################################
	foreach(MAYA_VERSION IN LISTS PROJECT_MAYA_VERSIONS)
		set(_MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${MAYA_VERSION}${MAYA_INSTALL_BASE_SUFFIX})
		set(PROJECT_ID ${PROJECT_NAME}_Maya${MAYA_VERSION})
		
		if(NOT EXISTS ${_MAYA_LOCATION})
			message(SEND_ERROR "maya was not found at ${_MAYA_LOCATION} - please assure your MAYA_VERSIONS are set correctly, as well as your MAYA_INSTALL_BASE_SUFFIX, which could be -x64 on 64 bit systems")
			return()
		endif()
		
		# MAYA INCLUDE DIR 
		set(MAYA_INCLUDE_DIR ${_MAYA_LOCATION}/${INCLUDE_SUFFIX})
		if(NOT EXISTS ${MAYA_INCLUDE_DIR})
			message(SEND_ERROR "Maya include directory at ${MAYA_INCLUDE_DIR} did not exist")
			return()
		endif()
		
		# LIBRARY INCLUDE DIR
		set(MAYA_LIB_DIR ${_MAYA_LOCATION}/${LIB_INSERT})
		if(NOT EXISTS ${MAYA_LIB_DIR})
			message(SEND_ERROR "Maya library directory at ${MAYA_LIB_DIR} did not exist")
			return()
		endif()
		
		set(LATEST_MAYA_INCLUDE_DIR ${MAYA_INCLUDE_DIR})
		
		message(STATUS "Setting up ${PROJECT_NAME} for maya ${MAYA_VERSION}")
		# CREATE TARGET
		###############
		add_library(${PROJECT_ID}
							MODULE
							${PROJECT_SOURCE_FILES})
		
		# TARGET LEVEL CONFIGURATION
		#############################
		append_to_target_property(${PROJECT_ID} LINK_FLAGS "${LINK_DIR_FLAG}${MAYA_LIB_DIR}" YES)
		if (${CMAKE_BUILD_TYPE} MATCHES Release AND UNIX AND NOT APPLE)
			append_to_target_property(${PROJECT_ID} LINK_FLAGS "-Wl,--strip-all,-O2" YES)
		endif()
		# its ignored by implementation ! Its hard to set per-project includes
		#pend_to_target_property(${PROJECT_ID} INCLUDE_DIRECTORIES ${MAYA_INCLUDE_DIR})
		#set_property(TARGET ${PROJECT_ID}
		#			APPEND PROPERTY INCLUDE_DIRECTORIES ${MAYA_INCLUDE_DIR})
		append_to_target_property(${PROJECT_ID} COMPILE_FLAGS "${INCL_DIR_FLAG}${MAYA_INCLUDE_DIR}" YES)
		if(PROJECT_WITHOUT_EXCEPTIONS AND UNIX)
			# TODO: windows version ... 
			append_to_target_property(${PROJECT_ID} COMPILE_FLAGS -fno-exceptions YES)
		endif()
		
		target_link_libraries(${PROJECT_ID} 
										${DEFAULT_MAYA_LIBRARIES} 
										${PROJECT_LINK_LIBRARIES})
		
		if(PROJECT_DEFINES)
			append_to_target_property(${PROJECT_ID} COMPILE_DEFINITIONS "${PROJECT_DEFINES}" NO)
		endif()
		
		set_target_properties(${PROJECT_ID} PROPERTIES
												OUTPUT_NAME 
														${PROJECT_NAME}
												PREFIX
														""				# make sure we don't get the 'lib' prefix on linux
												SUFFIX
														${PROJECT_SUFFIX}
												LIBRARY_OUTPUT_DIRECTORY
													${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MAYA_VERSION}
												CLEAN_DIRECT_OUTPUT 1)
												
		if(PROJECT_WITH_TEST)
			# Assure configuration
			if(EXISTS ${TEST_TMRV_PATH})
				add_test(NAME
							${PROJECT_ID}
						WORKING_DIRECTORY
							${CMAKE_SOURCE_DIR}/test
						COMMAND 
							python ${TEST_TMRV_PATH} ${MAYA_VERSION} 
									--mrv-mayapy batch_startup.py 
									$<TARGET_FILE:${PROJECT_ID}>)
				message(STATUS "Adding test for ${PROJECT_NAME} for maya ${MAYA_VERSION}")
			else()
				message(STATUS "Tests for ${PROJECT_ID} deactivated as tmrv path is not provided, check your configuration")
			endif() # project with tmrv path
		endif() # project with test
	endforeach()# FOR EACH MAYA VERSION
	
	# FOR CONVENIENCE, SET THE LATEST MAYA VERSION INCLUDE PATH
	###########################################################
	# To help qtcreator, we need to set the maya include on directory level.
	# Otherwise it will not know the maya headers, which reduces convenience quite a bit
	# As the include comes last, it will not be effective for the actual compile
	include_directories(${LATEST_MAYA_INCLUDE_DIR})
	
endfunction()

