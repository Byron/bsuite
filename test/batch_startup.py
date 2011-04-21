import sys
import os

# we do nothing more than setting up information about the location of the 
# plugin we should test
if len(sys.argv) != 2:
	raise AssertionError("Need first argument to be location to plugin we should test, got %s" % ", ".join(sys.argv))

os.environ['TEST_PLUGIN_PATH'] = sys.argv[1]
sys.argv = sys.argv[:1]

import mrv.test.cmd.startup as startup; 
startup.nose()
