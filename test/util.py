import os
from mrv.test.lib import *
from mrv.test.maya.util import save_for_debugging
from mrv.path import Path
import mrv.maya
import mrv.maya.nt as nt

import maya.cmds as cmds
import maya.OpenMaya as api

import nose

__all__ = (	'cmds', 'api', 'nt', 'TestMayaBase', 'TestPtexVisNodeBase', 'TestLidarVisNodeBase',
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
				raise AssertionError("Failed to load plugin at %s" % cls.test_plugin_path)
		#END load plugin
		
	@classmethod
	def openScene(cls, path):
		"""Open the given scene forcibly"""
		return mrv.maya.Scene.open(path, force=True)
		
	@classmethod
	def makeNode(cls, typename):
		"""Create a node of the given type name and return it.
		If the node type does not exist, the test will be skipped"""
		try:
			return getattr(nt, typename)()
		except AttributeError:
			raise nose.SkipTest("plugin containing node of type %s was not loaded" % typename)
		
	@classmethod
	def tearDownAll(cls):
		mrv.maya.Scene.new(force=True)
		if cmds.pluginInfo(cls.test_plugin_path, q=1, loaded=1):
			cmds.unloadPlugin(cls.test_plugin_path)
		#END unload plugin safely
	

class TestLidarVisNodeBase(TestMayaBase):
	"""provides utilities to setup a lidar file"""
	
	@classmethod
	def lidarPath(cls, name):
		""":return: full path the ptexture with the given name"""
		return fixture_path("lidar") / name

	@classmethod
	def setupScene(cls, lidar_filename):
		"""create a new scene with a single lidar visualization node.
		Make it load the given lidar_filepath.
		:return: newly created lidar node"""
		lf = cls.lidarPath(lidar_filename)
		assert lf.isfile(), "lidar file did not exist at %s" % lf
		mrv.maya.Scene.new(force=True)
		
		n = cls.makeNode("LidarVisNode")
		n.lidarFilePath.setString(str(lf))
		return n
		

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
		
		n = cls.makeNode("PtexVisNode")
		m.outMesh.mconnectTo(n.inMesh)
		n.ptfp.setString(str(cls.ptexturePath(ptexture_name)))
		return n, m
