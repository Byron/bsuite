
# Create a test executable, and setup a test
# add the source files as last arguments
function(add_maya_test_executable name testname)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} ${PTEX_LIBRARIES} ${MAYA_LIBRARIES})
	set_target_properties(${name}
		PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test)
	get_target_property(execpath ${name} RUNTIME_OUTPUT_DIRECTORY)
	add_test(${testname} ${execpath}/${name})
endfunction(add_maya_test_executable)

# This function should be called from a subdirectory denoting the maya version
# to make sure the include directories are only provided for a single location, and 
# not for all locations
# It requires some variables from the central configuration.
function(add_maya_project VERSION)
	# set source files
	set(MAYA_SOURCE_FILES 
		../maya_main.cpp
		../visnode.cpp
		../util.cpp)

	set(_MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${VERSION}${MAYA_INSTALL_BASE_SUFFIX})
	set(_PROJECT Maya${VERSION})
	
	if(NOT EXISTS ${_MAYA_LOCATION})
		message(SEND_ERROR "maya was not found at ${_MAYA_LOCATION} - please assure your MAYA_VERSIONS are set correctly, as well as your MAYA_INSTALL_BASE_SUFFIX, which could be -x64 on 64 bit systems")
	endif()
	
	
	if(WIN32)
		set(PROJECT_SUFFIX .mll)
	elif(APPLE)
		set(PROJECT_SUFFIX .bundle)
	else()
		set(PROJECT_SUFFIX .so)
	endif()
	
	include_directories(${_MAYA_LOCATION}/include)
	link_directories(${_MAYA_LOCATION}/lib)
	
	add_library(${_PROJECT} SHARED ${MAYA_SOURCE_FILES} ${HEADER_FILES})
	target_link_libraries(${_PROJECT} ${MAYA_LIBRARIES} ${PTEX_LIBRARIES})
	
	set_target_properties(${_PROJECT} PROPERTIES 
											OUTPUT_NAME ptexmaya 
											SUFFIX ${PROJECT_SUFFIX}
											CLEAN_DIRECT_OUTPUT 1)
	
	if (WIN32)
		set_target_properties(${_PROJECT} PROPERTIES SUFFIX ".mll")
	endif()
endfunction(add_maya_project)

