import os
from mrv.test.lib import *
from mrv.test.maya.util import save_for_debugging
from mrv.path import Path
import mrv.maya
import mrv.maya.nt as nt

import maya.cmds as cmds
import maya.OpenMaya as api

__all__ = (	'cmds', 'api', 'nt', 'TestMayaBase', 'TestPtexVisNodeBase',
			'fixture_path', 'save_for_debugging')


def fixture_path(name):
	""":return: full path to fixture with given name"""
	return Path(__file__).dirname() / "fixtures" / name 

class TestMayaBase(unittest.TestCase):
	env_var_plugin_path = "TEST_PLUGIN_PATH"
	test_plugin_path = None	# fill be set automatically
	
	@classmethod
	def setUpAll(cls):
		if cls.env_var_plugin_path not in os.environ:
			raise AssertionError("Missing configuration variable: %s" % cls.env_var_plugin_path)
		#END handle conf
		cls.test_plugin_path = os.environ[cls.env_var_plugin_path]
		if not cmds.pluginInfo(cls.test_plugin_path, q=1, loaded=1):
			cmds.loadPlugin(cls.test_plugin_path)
			# even though loadPlugin may fail, it doesn't throw a runtime error
			if not cmds.pluginInfo(cls.test_plugin_path, q=1, loaded=1):
				raise AssertionError("Failed to load plugin")
		#END load plugin
		
	@classmethod
	def openScene(cls, path):
		"""Open the given scene forcibly"""
		return mrv.maya.Scene.open(path, force=True)
	
	
class TestPtexVisNodeBase(TestMayaBase):
	"""provides utilities to setup a ptexture file"""
	
	@classmethod
	def ptexturePath(cls, name):
		""":return: full path the ptexture with the given name"""
		return fixture_path(name) / 'texture.ptx'
	
	@classmethod
	def setupScene(cls, ptexture_name):
		"""Open the given ptexture scene and set it up
		:return: tuple(ptexnode, mesh) tuple of ptex node and the connected mesh shape"""
		base = fixture_path(ptexture_name)
		cls.openScene(base / 'scene.ma')
		m = nt.Node("meshShape")
		assert(isinstance(m, nt.Mesh))
		
		n = nt.PtexVisNode()
		m.outMesh.mconnectTo(n.inMesh)
		n.ptfp.setString(str(cls.ptexturePath(ptexture_name)))
		return n, m
