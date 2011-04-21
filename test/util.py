import os
from mrv.test.lib import *

class TestBase(unittest.TestCase):
	env_var_plugin_path = "TEST_PLUGIN_PATH"
	
	@classmethod
	def setUpAll(cls):
		if cls.env_var_plugin_path not in os.environ:
			raise AssertionError("Missing configuration variable: %s" % cls.env_var_plugin_path)
		#END handle conf
