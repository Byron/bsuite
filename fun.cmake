
include(CMakeParseArguments)


# =============================================================
# find_sourcefiles_recursive(RESULT_VARIABLE DIRECTORIES EXCLUDE_PATHS)
# Set RESULT_VARIABLE to a list of file paths relative to the given list of DIRECTORIES
# which should be a relative path. All paths are based on the current source directory.
# ======================================
function(find_sourcefiles_recursive RESULT_VARIABLE DIRECTORIES EXCLUDE_PATHS)
	foreach(SUBDIR ${DIRECTORIES})
		get_filename_component(ABSOLUTE_SUBDIR ${SUBDIR} ABSOLUTE)
		if(NOT EXISTS ${ABSOLUTE_SUBDIR})
			message(WARNING "In project ${PROJECT_ID}: source directory '${SUBDIR}' did not exist at ${ABSOLUTE_SUBDIR}. Please check your cmake configuration")
		endif()
		
		set(FILEGLOBS)
		foreach(FILESPEC IN LISTS FL_CMAKE_SOURCE_EXTENSIONS)
			list(APPEND FILEGLOBS ${SUBDIR}/${FILESPEC})
		endforeach()
		
		file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${FILEGLOBS})
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
function(append_to_target_property TARGET PROPERTY VALUES)
	get_target_property(CURR_VALUES ${TARGET} ${PROPERTY})
	if (CURR_VALUES)
		list(APPEND CURR_VALUES ${VALUES})
	else()
		set(CURR_VALUES ${VALUES})
	endif()
	set_target_properties(${TARGET} PROPERTIES ${PROPERTY} "${CURR_VALUES}")
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
# ======================================
function(add_maya_project)
	cmake_parse_arguments(PROJECT 
						""
						"NAME"
						"MAYA_VERSIONS;SOURCE_FILES;SOURCE_DIRS;INCLUDE_DIRS;LIBRARY_DIRS;LINK_LIBRARIES;DEFINES"
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
			set(PROJECT_SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
		endif()
		find_sourcefiles_recursive(PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIRS}")
	endif()
	
	if(NOT PROJECT_INCLUDE_DIRS)
		set(PROJECT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
	endif()
	
	if(NOT PROJECT_SOURCE_FILES)
		message(SEND_ERROR "Did not find any source file, or no source file specified. Use the SOURCE_FILES or SOURCE_DIRS parameter")
	endif()
	
	# HANDLE SUFFIX
	if(WIN32)
		set(PROJECT_SUFFIX .mll)
	elif(APPLE)
		set(PROJECT_SUFFIX .bundle)
	else()
		set(PROJECT_SUFFIX .so)
	endif()
	
	# OSX HANDLING
	if(APPLE)
		set(LIB_INSERT "/Maya.app/Contents/MacOS")
		set(INCLUDE_INSERT "/devkit")
	endif()
	
	if(WIN32)
		set(LINK_DIR_FLAG "\L"
	else()
		set(LINK_DIR_FLAG "-L"
	endif()
	
	# DIRECTORY LEVEL CONFIGURATION
	################################
	# Should only run once
	include_directories(${PROJECT_INCLUDE_DIRS})
	link_directories(${PROJECT_LIBRARY_DIRS})
	
	
	# FOR EACH MAYA VERSION TO GENERATE
	####################################
	foreach(MAYA_VERSION IN LISTS PROJECT_MAYA_VERSIONS)
		set(_MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${MAYA_VERSION}${MAYA_INSTALL_BASE_SUFFIX})
		set(PROJECT_ID ${PROJECT_NAME}_Maya${MAYA_VERSION})
		
		if(NOT EXISTS ${_MAYA_LOCATION})
			message(SEND_ERROR "maya was not found at ${_MAYA_LOCATION} - please assure your MAYA_VERSIONS are set correctly, as well as your MAYA_INSTALL_BASE_SUFFIX, which could be -x64 on 64 bit systems")
		endif()
		
		# MAYA INCLUDE DIR 
		set(MAYA_INCLUDE_DIR ${_MAYA_LOCATION}${INCLUDE_INSERT}/include)
		if(NOT EXISTS ${MAYA_INCLUDE_DIR})
			message(SEND_ERROR "Maya include directory at ${MAYA_INCLUDE_DIR} did not exist")
		endif()
		
		# LIBRARY INCLUDE DIR
		set(MAYA_LIB_DIR ${_MAYA_LOCATION}${LIB_INSERT}/lib)
		if(NOT EXISTS ${MAYA_LIB_DIR)
			message(SEND_ERROR "Maya library directory at ${MAYA_LIB_DIR} did not exist")
		endif()
		
		# CREATE TARGET
		###############
		add_library(${PROJECT_ID}
							MODULE
							${PROJECT_SOURCE_FILES})
		
		# TARGET LEVEL CONFIGURATION
		#############################
		append_to_target_property(${PROJECT_ID} LINK_FLAGS "${LINK_DIR_FLAG}${MAYA_LIB_DIR}")
		if (${CMAKE_BUILD_TYPE} MATCHES Release AND UNIX AND NOT APPLE)
			append_to_target_property(${PROJECT_ID} LINK_FLAGS "-Wl,--strip-all,-O2")
		endif()
		append_to_target_property(${PROJECT_ID} INCLUDE_DIRECTORIES ${MAYA_INCLUDE_DIR})
		
		target_link_libraries(${PROJECT_ID} 
										${DEFAULT_MAYA_LIBRARIES} 
										${PROJECT_LINK_LIBRARIES})
		
		if(PROJECT_DEFINES)
			append_to_target_property(${PROJECT_ID} COMPILE_DEFINITIONS "${PROJECT_DEFINES}")
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
	endforeach()# FOR EACH MAYA VERSION
endfunction()

