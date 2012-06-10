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

#include "LAS_IStream.h"

#include <maya/MPxLocatorNode.h>
#include <maya/MGLdefinitions.h>
#include <maya/MMatrix.h>

#include <fstream>
#include <auto_ptr.h>


//! Node helping to visualize lidar data
class LidarVisNode : public MPxLocatorNode
{
	public:
	
	enum DisplayMode
	{
		DMIntensity = 0,			//!< display intensity as RGB
		DMReturnNumber = 1		//!< display return number as RGB
	};
	
	public:
		LidarVisNode();
		virtual ~LidarVisNode();

		virtual MStatus compute(const MPlug&, MDataBlock&);
		virtual bool	setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx);
		virtual void    postConstructor();
		virtual void	draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus);
		virtual MBoundingBox boundingBox() const;
		virtual bool	isBounded() const;

		static  void*   creator();
		static  MStatus initialize();

		static const MTypeId typeId;				//!< binary file type id
		static const MString typeName;				//!< node type name

	protected:
		void reset_output_attributes(MDataBlock &data);	//!< reset all output attributes to their initial values
		bool renew_las_reader(const MString& filepath);	//!< initialize our reader with a new file
		void reset_caches();							//!< clear all caches
		void update_compensation_matrix_and_bbox(bool translateToOrigin);	//!< update our compensation matrix
		
	protected:
		// Input attributes
		static MObject aLidarFileName;			//!< path to lidard file to use
		static MObject aGlPointSize;			//!< size of a point when drawing
		static MObject aIntensityScale;			//!< scales the intensity by the given amount
		static MObject aTranslateToOrigin;		//!< if true, the point samples will be translated back to the origin
		static MObject aUseMMap;				//!< if true, we should use memory mapping (non-windows only !)
		static MObject aUsePointCache;			//!< if true, all data will be cached on the gpu
		static MObject aDisplayMode;			//!< display mode enumeration
		
		// output attributes
		static MObject aOutSystemIdentifier;	//!< creator's system id
		static MObject aOutGeneratingSoftware;	//!< Id of the software which created the file
		static MObject aOutCreationDate;		//!< string of day of year and year in which the file was created
		static MObject aOutVersionString;		//!< string in the format "major.minor"
		static MObject aOutNumVariableRecords;	//!< amount of additional data streams
		static MObject aOutPointDataFormat;		//!< Format id of the point data
		static MObject aOutNumPointRecords;		//!< amount of point records stored in the file
		static MObject aOutPointScale;			//!< vector of overall point scale factor
		static MObject aOutPointOffset;			//!< vector of point offset
		static MObject aOutPointBBoxMin;		//!< min point of bounding box which fits all points
		static MObject aOutPointBBoxMax;		//!< max point of bounding box which fits all points

		// other attributes
		static MObject aNeedsCompute;			//!< dummy output (for now) to check if we need to compute
		

	protected:
		MString			m_error;				//!< error string shown if non-empty
		MGLfloat		m_gl_point_size;		//!< size of a point when drawing (cache)
		MBoundingBox	m_bbox;					//!< bounding box cache
		float			m_intensity_scale;		//!< value to scale the intensity with
		
		std::auto_ptr<LAS_IStream>		m_las_stream;	//!< pointer to las reader
		std::ifstream					m_ifstream;		//!< file for reading samples
		
		static const MMatrix	convert_z_up_to_y_up_column_major;	//!< matrix to convert z up to y up
		MMatrix			m_compensation_column_major;	//!< column major compensation matrix for use by ogl
};

#endif
