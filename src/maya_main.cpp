#include "util.h"
#include "visnode.h"

#include <maya/MFnPlugin.h>

#include <Ptexture.h>

PtexCache* gCache = 0;

extern "C" {

	//! Initialize the plugin in maya
	MStatus initializePlugin(MObject obj)
	{
		gCache = PtexCache::create();
		
		MFnPlugin plugin(obj, "Sebastian Thiel", "0.1");
		MStatus stat;
		
		stat = plugin.registerNode(PtexVisNode::typeName, PtexVisNode::typeId, 
									PtexVisNode::creator, PtexVisNode::initialize,
									MPxNode::kLocatorNode);
		if (stat.error()){
			stat.perror("register visualization node");
			return stat;
		}

		return stat;
	}

	//! Deinitialize the plugin from maya
	MStatus uninitializePlugin(MObject obj)
	{
		MFnPlugin plugin(obj);
		MStatus stat;

		stat = plugin.deregisterNode(PtexVisNode::typeId);
		if (stat.error()){
			stat.perror("deregister PtexVisNode");
			return stat;
		}
		
		// Finally clear ptexture cache
		gCache->release();
		gCache = NULL;
		return stat;
	}

}// end extern "C"
