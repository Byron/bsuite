
function(init_ptex RESULT)
	set(PTEX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdParty/ptex/install/include" CACHE PATH
		"Directory containing the ptex headers")
	
	set(PTEX_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/3rdParty/ptex/install/lib" CACHE PATH
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
	
	# VERIFY ZLIB
	#############
	include(FindZLIB)
	if(NOT ZLIB_FOUND OR NOT ZLIB_LIBRARY)
		message(WARNING "zlib library was not found, please adjust your cmake cache and try again")
		if(ZLIB_FOUND)
			message(SEND_ERROR "Zlib was found, but the ZLIB_LIBRARY variable was not set whatsoever. Please check cmake version")
		endif()
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
						"${ZLIB_LIBRARY}"
					LINK_MAYA_LIBRARIES
						mayabase
					LIBRARY_DIRS
						${PTEX_LIBRARY_DIR}
					DEFINES
						THIRD_PARTY_INTTYPES
						PTEX_STATIC
					WITH_TEST
					WITHOUT_EXCEPTIONS
					WITH_OPENMP
					)
endif()