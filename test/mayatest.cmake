
# Assure configuration

set(TEST_MRV_PATH "" CACHE FILEPATH
	"Path to the tmrv executable, providing facilities to test maya plugins using the mrv development framework")

set(TEST_MAYA_VERSION 2010 CACHE STRING
	"Maya version to use for testing")

if(EXISTS ${TEST_MRV_PATH})
	add_test(NAME maintest 
		COMMAND python ${TEST_MRV_PATH} ${TEST_MAYA_VERSION} batch_startup.py $<TARGET_FILE:Maya${TEST_MAYA_VERSION}>)
else()
	message(WARNING "Tests deactivated as tmrv path is not provided, check your configuration")
endif()
