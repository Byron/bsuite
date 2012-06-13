
from util import *

class TestLidar(TestLidarVisNodeBase):
	
	def test_main(self):
		n = self.setupScene("galveston_EPSG_32615_4326.las")
		assert n.isValid()
		
		# query header info
		assert not n.outSystemIdentifier.asString()
		assert n.outGeneratingSoftware.asString() == "TerraScan"
		assert n.outCreationDate.asString() == "0/0"
		assert n.outVersionString.asString() == "1.2"
		assert n.outNumVariableRecords.asInt() == 0
		assert n.outNumPointRecords.asInt() == 99660
		assert n.outPointDataFormat.asInt() == 1
		assert n.outPointScaleX.asFloat() <= 0.01
		assert n.outPointOffsetX.asFloat() == 0.0
		
		assert n.compute.asInt() == 0, "Computation should have been successful"
		

		# TODO: header info without valid file 
