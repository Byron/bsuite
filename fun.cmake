
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
