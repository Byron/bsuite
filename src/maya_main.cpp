#include <PtexUtils.h>
#include <maya/MFnPlugin.h>

extern "C" {

	//! Initialize the plugin in maya
	MStatus initializePlugin(MObject obj)
	{
		MFnPlugin plugin(obj, "Sebastian Thiel", "0.1");
		MStatus stat;

		return stat;
	}

	//! Deinitialize the plugin from maya
	MStatus uninitializePlugin(MObject obj)
	{
		MFnPlugin plugin(obj);

		MStatus	stat;

		return stat;
	}

}// end extern "C"
