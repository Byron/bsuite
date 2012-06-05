
function(init_ptex RESULT)
	set(PTEX_INCLUDE_DIR "" CACHE PATH
		"Directory containing the ptex headers")
	
	set(PTEX_LIBRARY_DIR "" CACHE PATH
		"Directory containing the ptex headers")
	
	# VERIFY PREREQUESITES
	######################
	if(NOT EXISTS ${PTEX_INCLUDE_DIR})
		message(WARNING "PTex header directory must be set, check your configuration")
		return()
	endif()
	
	if(NOT EXISTS ${PTEX_LIBRARY_DIR})
		message(WARNING "PTex library directory must be set, check your configuration")
		return()
	endif()
	
	set(${RESULT} 1 PARENT_SCOPE)
endfunction()

init_ptex(WAS_SUCCESSFUL)



if(WAS_SUCCESSFUL)
	add_maya_project(NAME
						ptexvis
					INCLUDE_DIRS
						. 
						..
						${PTEX_INCLUDE_DIR}
					LINK_LIBRARIES
						Ptex
						z
					LIBRARY_DIRS
						${PTEX_LIBRARY_DIR}
					WITH_TEST
					)
endif()