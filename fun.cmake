
set(PTEX_LIBRARIES Ptex z)

# This function should be called from a subdirectory denoting the maya version
# to make sure the include directories are only provided for a single location, and 
# not for all locations
function(add_maya_build VERSION)
	set(_MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${VERSION}${MAYA_INSTALL_BASE_SUFFIX})
	set(_PROJECT Maya${VERSION})
	
	include_directories(${_MAYA_LOCATION}/include)
	link_directories(${_MAYA_LOCATION}/lib)
	
	add_library(${_PROJECT} SHARED ${SOURCE_FILES} ${HEADER_FILES})
	target_link_libraries(${_PROJECT} ${MAYA_LIBRARIES} ${PTEX_LIBRARIES})
	
	set_target_properties(${_PROJECT} PROPERTIES OUTPUT_NAME ptexmaya${VERSION})
	set_target_properties(${_PROJECT} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	
	if (WIN32)
		set_target_properties(${_PROJECT} PROPERTIES SUFFIX ".mll")
	endif()
endfunction(add_maya_build)

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
