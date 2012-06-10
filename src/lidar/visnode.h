/* Copyright (c) 2012, Sebastian Thiel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIDAR_VISUALIZATION_NODE
#define LIDAR_VISUALIZATION_NODE

#include <maya/MPxLocatorNode.h>
#include <maya/MGLdefinitions.h>



//! Node helping to visualize lidar data
class LidarVisNode : public MPxLocatorNode
{
	public:
	
	enum DisplayMode
	{
		DMColor = 0,				//!< display color as RGB of GLpoints
		DMReturnNumber = 1,		//!< display return number as RGB
		DMIntensity = 2			//!< display intensity as RGB
	};
	
	public:
		LidarVisNode();
		virtual ~LidarVisNode();

		virtual MStatus compute(const MPlug&, MDataBlock&);
		virtual bool	setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx);
		virtual void    postConstructor();
		virtual void	draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus);

		static  void*   creator();
		static  MStatus initialize();

		static const MTypeId typeId;				//!< binary file type id
		static const MString typeName;				//!< node type name

	protected:
		
		
	protected:
		// Input attributes
		static MObject aLidarFileName;			//!< path to lidard file to use
		static MObject aGlPointSize;			//!< size of a point when drawing
		static MObject aUseMMap;				//!< if true, we should use memory mapping (non-windows only !)
		static MObject aUsePointCache;			//!< if true, all data will be cached on the gpu
		static MObject aDisplayMode;			//!< display mode enumeration
		
		// output attributes
		static MObject aOutSystemIdentifier;	//!< creator's system id
		static MObject aOutGeneratingSoftware;	//!< Id of the software which created the file
		static MObject aOutCreationDate;		//!< string of month and year in which the file was created
		static MObject aOutVersionString;		//!< string in the format "major.minor"
		static MObject aOutNumVariableRecords;	//!< amount of additional data streams
		static MObject aOutPointDataFormat;		//!< Format id of the point data
		static MObject aOutNumPointRecords;		//!< amount of point records stored in the file
		static MObject aOutPointScale;			//!< vector of overall point scale factor
		static MObject aOutPointOffset;			//!< vector of point offset
		static MObject aOutPointBBox;			//!< bounding box which fits all points

		// other attributes
		static MObject aNeedsCompute;			//!< dummy output (for now) to check if we need to compute
		

	protected:
		MString			m_error;				//!< error string shown if non-empty
		
		MGLfloat		m_gl_point_size;		//!< size of a point when drawing (cache)	
};

#endif
