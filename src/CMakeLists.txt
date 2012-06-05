
foreach(VERSION IN LISTS MAYA_VERSIONS)
	add_subdirectory(${VERSION})
endforeach()
