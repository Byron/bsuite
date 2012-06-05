
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
		assert n.outNumSamples.asInt() == 0	# not computed yet
		
		# trigger computation
		# This can still work as its a one-on-one mapping
		assert n.needsComputation.asInt() == True
		assert n.outNumSamples.asInt() == 144
		
		# trigger resampling
		n.sampleMultiplier.setFloat(0.0)	# 0 should be fine as well, maybe restrict it
		assert n.needsComputation.asInt() == True
		n.sampleMultiplier.setFloat(1.5)
		assert n.needsComputation.asInt() == True
		for dm in range(3):
			n.displayMode.setFloat(dm)
			assert n.needsComputation.asInt() == True
		#END set display mode
		
		
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
		
		# not a one-on-one mapping
		assert n.needsComputation.asInt() == False
		assert n.outNumSamples.asInt() == 0
		
		# change scene
		n, m = self.setupScene('nonquad')
		n.needsComputation.asInt()
		
		# empty string is fine
		n.ptfp.setString("")
		assert n.needsComputation.asInt() == True
		
		# non-exinsting path as well
		n.ptfp.setString("helloworld")
		assert n.needsComputation.asInt() == True
		
		# if there is no valid texture, we don't have texture information
		assert n.outNumChannels.asInt() == 0
		assert n.outNumFaces.asInt() == 0
		assert n.outHasEdits.asBool() == 0
		assert n.outHasMipMaps.asBool() == 0
		assert n.outAlphaChannel.asInt() == 0
		assert len(n.outMetaDataKeys.masData().array()) == 0
