
from util import *

class TestMain(TestPtexVisNodeBase):
	
	def test_main(self):
		n, m = self.setupScene('triangle')
		assert n.isValid() and m.isValid()
		
		# query output info
		keys = n.outMetaDataKeys.masData().array()
		assert len(keys) == 0
		assert n.outNumChannels.asInt() == 3
		assert n.outNumFaces.asInt() == 9
		assert n.outHasEdits.asBool() == 0
		assert n.outHasMipMaps.asBool() == 0
		assert n.outAlphaChannel.asInt() == -1
		assert n.outDataType.asInt() == 3
		assert n.outMeshType.asInt() == 0
		assert n.outUBorderMode.asInt() == 0
		assert n.outVBorderMode.asInt() == 0
		
		
		
		# trigger computation
		n.needsComputation.asInt()
		
		
		# channels change if texture changes
		n.ptfp.setString(self.ptexturePath('nonquad'))
		keys = n.outMetaDataKeys.masData().array()
		assert(len(keys) == 4)
		assert n.outNumChannels.asInt() == 3
		assert n.outNumFaces.asInt() == 21
		assert n.outHasEdits.asBool() == 0
		assert n.outHasMipMaps.asBool() == 1
		assert n.outAlphaChannel.asInt() == -1
		assert n.outDataType.asInt() == 0
		assert n.outMeshType.asInt() == 1
		assert n.outUBorderMode.asInt() == 0
		assert n.outVBorderMode.asInt() == 0
		
		# change scene
		n, m = self.setupScene('nonquad')
		n.needsComputation.asInt()
