
add_maya_project(NAME
					lidartools
				INCLUDE_DIRS
					. 
					..
				LINK_LIBRARIES
					yalas
				SOURCE_DIRS
					.
					../base
				WITH_TEST
				)