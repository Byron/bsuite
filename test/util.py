import os
from mrv.test.lib import *
import mrv.maya

import maya.cmds as cmds
import maya.OpenMaya as api

__all__ = ('cmds', 'api', 'TestMayaBase')

class TestMayaBase(unittest.TestCase):
	env_var_plugin_path = "TEST_PLUGIN_PATH"
	test_plugin_path = None
	
	@classmethod
	def setUpAll(cls):
		if cls.env_var_plugin_path not in os.environ:
			raise AssertionError("Missing configuration variable: %s" % cls.env_var_plugin_path)
		#END handle conf
		cls.test_plugin_path = os.environ[cls.env_var_plugin_path]
		if not cmds.pluginInfo(cls.test_plugin_path, q=1, loaded=1):
			cmds.loadPlugin(cls.test_plugin_path)
		#END load plugin
