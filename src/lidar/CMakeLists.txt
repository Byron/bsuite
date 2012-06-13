
add_maya_project(NAME
					lidartools
				INCLUDE_DIRS
					. 
					..
				LINK_LIBRARIES
					yalas
				LINK_MAYA_LIBRARIES
					mayabase
				WITH_TEST
				)