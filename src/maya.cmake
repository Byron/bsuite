
set(MAYA_SOURCE_FILES 
	../maya_main.cpp
	../visnode.cpp
	../util.cpp)

# This function should be called from a subdirectory denoting the maya version
# to make sure the include directories are only provided for a single location, and 
# not for all locations
function(add_maya_build VERSION)
	set(_MAYA_LOCATION ${MAYA_INSTALL_BASE_PATH}/maya${VERSION}${MAYA_INSTALL_BASE_SUFFIX})
	set(_PROJECT Maya${VERSION})
	
	include_directories(${_MAYA_LOCATION}/include)
	link_directories(${_MAYA_LOCATION}/lib)
	
	add_library(${_PROJECT} SHARED ${MAYA_SOURCE_FILES} ${HEADER_FILES})
	target_link_libraries(${_PROJECT} ${MAYA_LIBRARIES} ${PTEX_LIBRARIES})
	
	set_target_properties(${_PROJECT} PROPERTIES OUTPUT_NAME ptexmaya)
	set_target_properties(${_PROJECT} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	
	if (WIN32)
		set_target_properties(${_PROJECT} PROPERTIES SUFFIX ".mll")
	endif()
endfunction(add_maya_build)



foreach(VERSION IN LISTS MAYA_VERSIONS)
	add_subdirectory(${VERSION})
endforeach()
