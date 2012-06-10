
from util import *

class TestLidar(TestLidarVisNodeBase):
	
	def test_main(self):
		n = self.setupScene("galveston_EPSG_32615_4326.las")
		assert n.isValid()
		
		# query header info
		print n.outSystemIdentifier.asString()
		print n.outGeneratingSoftware.asString()
		print n.outCreationDate.asString()
		print n.outVersionString.asString()
		print n.outNumVariableRecords.asInt()
		print n.outNumPointRecords.asInt()
		print n.outPointDataFormat.asInt()
		print n.outPointScale.child(0).asFloat()
		
		assert False


