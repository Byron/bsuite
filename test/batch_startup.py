import sys
import os

# we do nothing more than setting up information about the location of the 
# plugin we should test
plug_file = sys.argv[-1]
if not os.path.isfile(plug_file):
	raise AssertionError("Need first argument to be location to existing plugin, got %s" % plug_file)

os.environ['TEST_PLUGIN_PATH'] = sys.argv[-1]
sys.argv = sys.argv[:1]

import mrv.test.cmd.startup as startup; 
startup.nose()
