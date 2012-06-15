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

#include "yalaslib/IStream.h"
#include "baselib/typ.h"
#include "mayabaselib/ogl_buffer.hpp"

#include <maya/MPxLocatorNode.h>
#include <maya/MGLdefinitions.h>
#include <maya/MMatrix.h>

#include <fstream>
#include <memory>
#include <vector>


class MGLFunctionTable;


//! Node helping to visualize lidar data
class LidarVisNode : public MPxLocatorNode
{
	public:
	
	enum DisplayMode
	{
		DMNoColor = 0,				//!< performance mode whcih saves individual color calls to ogl
		DMIntensity,				//!< display intensity as RGB
		DMReturnNumber,				//!< display return number as RGB
		DMReturnNumberIntensity,	//!< mix return number with intensity as RGB
		DMStoredColor				//!< display the stored color if possible
	};
	
	enum CacheMode
	{
		CMNone = 0,
		CMSystem = SystemMemory,
		CMGPU = GPUMemory
	};
	
	struct VtxPrimitive : public draw_primitive<MGLint, 3, VertexArray>
	{
		inline
		void	init_from_point(const yalas::types::PointDataRecord0& p) {
			field[0] = p.x;
			field[1] = p.y;
			field[2] = p.z;
		}
	};
	
	typedef draw_primitive<MGLushort, 3, ColorArray>		ColPrimitive;
	
	typedef ogl_system_buffer<VtxPrimitive, ColPrimitive>	OGLSysBuf;
	typedef ogl_gpu_buffer<VtxPrimitive, ColPrimitive>		OGLGPUBuf;
	
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
		void reset_caches();								//!< clear all caches
		void reset_draw_caches(MGLFunctionTable* glf = 0);	//!< clear draw caches only
		void update_compensation_matrix_and_bbox(bool translateToOrigin);	//!< update our compensation matrix
		
		template <uint8_t format_id>
		inline void color_point(const yalas::types::point_data_record<format_id>& p, ColPrimitive &dc, const DisplayMode mode) const;
		inline void color_point_no_rgb(const yalas::types::PointDataRecord0& p, ColPrimitive &dc, const DisplayMode mode) const;
		template <typename PointType>
		inline void color_point_with_rgb_info(const PointType& p, ColPrimitive &dc, const DisplayMode mode) const;
		
		template <uint8_t format_id>
		inline void draw_point_records(MGLFunctionTable& glf, yalas::IStream& las_stream, const DisplayMode mode) const;
		
		template <uint8_t format_id, typename IteratorType>
		inline void draw_point_records_with_iterator(IteratorType& it, MGLFunctionTable& glf, const DisplayMode mode) const;
		
		template <typename Buffer>
		void update_draw_cache(Buffer& buf, DisplayMode mode, MGLFunctionTable &glf);		//!< fill in the draw cache
		
		template <uint8_t format_id, typename Buffer>
		inline void update_point_cache(Buffer& buf, const DisplayMode mode, MGLFunctionTable& glf);
		
		template <uint8_t format_id, typename IteratorType, typename Buffer>
		inline void update_point_cache_with_iterator(Buffer& buf, IteratorType& it, const DisplayMode mode, MGLFunctionTable& glf);
		
	protected:
		// Input attributes
		static MObject aLidarFileName;			//!< path to lidard file to use
		static MObject aGlPointSize;			//!< size of a point when drawing
		static MObject aIntensityScale;			//!< scales the intensity by the given amount
		static MObject aTranslateToOrigin;		//!< if true, the point samples will be translated back to the origin
		static MObject aUseMMap;				//!< if true, we should use memory mapping (non-windows only !)
		static MObject aDisplayCacheMode;		//!< Identify the type of display cache to use
		static MObject aNormalizeStoredCols;	//!< if true, stored colors will be upscaled to 16 bit - only necessary if stored normalized to 8 bit
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
		bool			m_normalize_stored_cols;//!< if true, we will normalize stored colors which is not the case in all files !
		bool			m_cache_needs_refresh;	//!< refresh the cache when drawing the next time
		
		std::auto_ptr<yalas::IStream>	m_las_stream;	//!< pointer to las reader
		std::ifstream					m_ifstream;		//!< file for reading samples
		
		static const MMatrix	convert_z_up_to_y_up_column_major;	//!< matrix to convert z up to y up
		MMatrix					m_compensation_column_major;	//!< column major compensation matrix for use by ogl
		
		OGLSysBuf				m_sysbuf;		//!< Systembased buffer for our data
		OGLGPUBuf				m_gpubuf;		//!< buffer directly on the graphics-card
		
		ROMappedFile			m_map;			//!< may contain a memory map of our lidar file
};

#endif
