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

#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MStringArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatPointArray.h>

// Fix unholy c++ incompatibility - typedefs to void are not allowed in gcc greater 4.1.2
#include "mayabaselib/ogl_headers.h"

#include "mayabaselib/base.h"
#include "yalaslib/iter.h"
#include "visnode.h"
// whyever this gets defined ... may have something to do with the ogl headers 
#undef Success

#include <assert.h>
#undef max
#include <limits>



/////////////////////////////////////////////////////////////////////

const MTypeId LidarVisNode::typeId(0x00108bde);
const MString LidarVisNode::typeName("lidarVisNode");


// input attributes
MObject LidarVisNode::aLidarFileName;
MObject LidarVisNode::aGlPointSize;
MObject LidarVisNode::aIntensityScale;
MObject LidarVisNode::aTranslateToOrigin;
MObject LidarVisNode::aUseMMap;
MObject LidarVisNode::aDisplayCacheMode;
MObject LidarVisNode::aDisplayMode;
MObject LidarVisNode::aNormalizeStoredCols;

// output attributes
MObject LidarVisNode::aOutSystemIdentifier;
MObject LidarVisNode::aOutGeneratingSoftware;
MObject LidarVisNode::aOutCreationDate;
MObject LidarVisNode::aOutVersionString;
MObject LidarVisNode::aOutNumVariableRecords;
MObject LidarVisNode::aOutPointDataFormat;
MObject LidarVisNode::aOutNumPointRecords;
MObject LidarVisNode::aOutPointScale;
MObject LidarVisNode::aOutPointOffset;
MObject LidarVisNode::aOutPointBBoxMin;
MObject LidarVisNode::aOutPointBBoxMax;

// other attributes
MObject LidarVisNode::aNeedsCompute;


// Column-Major right away
const float _initializer[4][4] = {
	{1.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, -1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

const MMatrix LidarVisNode::convert_z_up_to_y_up_column_major(_initializer);


LidarVisNode::LidarVisNode()
    : m_gl_point_size(1.0f)
	, m_intensity_scale(1.0f)
	, m_normalize_stored_cols(false)
{}

LidarVisNode::~LidarVisNode()
{
	// clear open handles
	renew_las_reader(MString());
	reset_caches();
}

void LidarVisNode::postConstructor()
{
	setMPSafe(false);
}

void* LidarVisNode::creator()
{
	return new LidarVisNode();
}

MStatus LidarVisNode::initialize()
{
	MStatus status;
	MFnNumericAttribute numFn;
	MFnTypedAttribute typFn;
	MFnEnumAttribute mfnEnum;

	// Input attributes
	////////////////////
	aLidarFileName = typFn.create("lidarFilePath", "lfp", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	typFn.setInternal(true);
	
	aUseMMap = numFn.create("useMMap", "umm", MFnNumericData::kBoolean, 1, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	aTranslateToOrigin = numFn.create("translateToOrigin", "tto", MFnNumericData::kBoolean, 1, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numFn.setAffectsWorldSpace(true);
	numFn.setInternal(true);
	
	aDisplayCacheMode = mfnEnum.create("displayCacheMode", "dcm");
	CHECK_MSTATUS_AND_RETURN_IT(status);
	mfnEnum.addField("None", 0);
	mfnEnum.addField("SystemCache", (int)SystemMemory);
	mfnEnum.addField("GPUCache", (int)GPUMemory);
	
	aNormalizeStoredCols = numFn.create("normalizeStoredColors", "nscol", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numFn.setInternal(true);
	
	
	aDisplayMode = mfnEnum.create("displayMode", "dm");
	mfnEnum.addField("NoColor", (int)DMNoColor);
	mfnEnum.addField("Intensity", (int)DMIntensity);
	mfnEnum.addField("ReturnNumber", (int)DMReturnNumber);
	mfnEnum.addField("ReturnNumberIntensified", (int)DMReturnNumberIntensity);
	mfnEnum.addField("StoredColor", (int)DMStoredColor);
	
	mfnEnum.setDefault((short)DMNoColor);
	mfnEnum.setKeyable(true);
	
	aIntensityScale = numFn.create("intensityScale", "iscale", MFnNumericData::kFloat, 1.0);
	numFn.setMin(1.0);
	numFn.setKeyable(true);
	numFn.setInternal(true);
	
	aGlPointSize = numFn.create("glPointSize", "glps", MFnNumericData::kFloat, 1.0);
	numFn.setMin(0.0);
	numFn.setKeyable(true);
	numFn.setInternal(true);
	
	// Output attributes
	/////////////////////
	aNeedsCompute = numFn.create("compute", "com", MFnNumericData::kInt);
	setup_as_output(numFn);
	
	aOutSystemIdentifier = typFn.create("outSystemIdentifier", "osid", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(typFn);
	
	aOutGeneratingSoftware = typFn.create("outGeneratingSoftware", "ogs", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(typFn);
	
	aOutCreationDate = typFn.create("outCreationDate", "ocd", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(typFn);
	
	aOutVersionString = typFn.create("outVersionString", "ovstr", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(typFn);
	
	aOutNumVariableRecords = numFn.create("outNumVariableRecords", "onvr", MFnNumericData::kInt, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);
	
	aOutNumPointRecords = numFn.create("outNumPointRecords", "onpr", MFnNumericData::kInt, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);
	
	aOutPointDataFormat = numFn.create("outPointDataFormat", "opdf", MFnNumericData::kInt, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);
	
	aOutPointScale = numFn.createPoint("outPointScale", "ops", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);
	
	aOutPointOffset = numFn.createPoint("outPointOffset", "opo", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);

	aOutPointBBoxMin = numFn.createPoint("outBBoxMin", "obbmin", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);	
	
	aOutPointBBoxMax = numFn.createPoint("outBBoxMax", "obbmax", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setup_as_output(numFn);
	
	
	
	

	// Add attributes
	/////////////////
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aLidarFileName));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aGlPointSize));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aIntensityScale));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aTranslateToOrigin))
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aUseMMap));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aDisplayCacheMode));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aNormalizeStoredCols));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aDisplayMode));
	
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aNeedsCompute));
	
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutSystemIdentifier));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutGeneratingSoftware));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutCreationDate));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutVersionString));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutNumVariableRecords));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutPointDataFormat));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutNumPointRecords));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutPointScale));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutPointOffset));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutPointBBoxMin));
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aOutPointBBoxMax));
	
	

	// ATTRIBUTE AFFECTS
	/////////////////////
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutCreationDate));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutGeneratingSoftware));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutNumPointRecords));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutNumVariableRecords));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutPointBBoxMin));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutPointBBoxMax));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutPointDataFormat));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutPointOffset));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutPointScale));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutSystemIdentifier));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aOutVersionString));
	
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aLidarFileName,	aNeedsCompute));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aUseMMap,			aNeedsCompute));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aDisplayCacheMode,	aNeedsCompute));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aDisplayMode,		aNeedsCompute));
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aNormalizeStoredCols, aNeedsCompute));
	
	return MS::kSuccess;
}

void LidarVisNode::reset_output_attributes(MDataBlock& data)
{
	data.outputValue(aOutCreationDate).setString(MString());
	data.outputValue(aOutGeneratingSoftware).setString(MString());
	data.outputValue(aOutSystemIdentifier).setString(MString());
	data.outputValue(aOutVersionString).setString(MString());
	
	data.outputValue(aOutNumPointRecords).setInt(0);
	data.outputValue(aOutNumVariableRecords).setInt(0);
	data.outputValue(aOutPointBBoxMax).set3Double(0, 0, 0);
	data.outputValue(aOutPointBBoxMin).set3Double(0, 0, 0);
	data.outputValue(aOutPointDataFormat).setInt(0);
	data.outputValue(aOutPointOffset).set3Double(0, 0, 0);
	data.outputValue(aOutPointScale).set3Double(0, 0, 0);
}

bool LidarVisNode::renew_las_reader(const MString &filepath)
{
	m_las_stream.reset();
	if (m_ifstream.is_open()) {
		m_ifstream.close();
	}
	
	m_map.unmap_file();
	if (filepath.length() == 0) {
		return false;
	}
	
	const MString res_path = resolved_filepath(filepath);
	m_ifstream.open(res_path.asChar(), std::ios_base::in | std::ios_base::binary);
	if (m_ifstream.fail()) {
		m_error = "Could not open file " + res_path + " for reading";
		return false;
	}
	
	m_las_stream.reset(new yalas::IStream(m_ifstream));
	if (m_las_stream->status() != yalas::IStream::Success) {
		m_error = "Unsupported file format";
		return renew_las_reader(MString());
	}
	
	return true;
}

void LidarVisNode::reset_caches()
{
	m_bbox = MBoundingBox();
	m_compensation_column_major.setToIdentity();
	reset_draw_caches();
}

void LidarVisNode::reset_draw_caches()
{
	// totally clear even the reserved memory
	m_col_cache = ColCache();
	m_pos_cache = PosCache();
}

void LidarVisNode::update_draw_cache(MDataBlock &data)
{
	if (m_las_stream.get() == 0) {
		return;
	}
	assert(m_las_stream->status() == yalas::IStream::Success);
	
	if (m_las_stream->reset_point_iteration() != yalas::IStream::Success) {
		return;
	}
	
	DisplayMode mode = static_cast<const DisplayMode>(data.outputValue(aDisplayMode).asInt());
	const uint8_t fmt = m_las_stream->header().point_data_format_id;
	
	// Sanity check - if people try to use stored color in files that don't have it, reset the mode
	if (mode == DMStoredColor && fmt != 2 && fmt != 3 && fmt != 5) {
		data.outputValue(aDisplayMode).setInt(DMNoColor);
		mode = DMNoColor;
	}
	
	m_pos_cache.resize(m_las_stream->header().num_point_records);
	if (mode != DMNoColor ) {
		m_col_cache.resize(m_las_stream->header().num_point_records);
	} else {
		m_col_cache = ColCache();
	}
	
	switch(fmt)
	{
	case 0: update_point_cache<0>(mode); break;
	case 1: update_point_cache<1>(mode); break;
	case 2: update_point_cache<2>(mode); break;
	case 3: update_point_cache<3>(mode); break;
	case 4: update_point_cache<4>(mode); break;
	case 5: update_point_cache<5>(mode); break;
	};
}

template <uint8_t format_id>
inline void LidarVisNode::update_point_cache(const DisplayMode mode)
{
	if (m_map.is_mapped()) {
		const yalas::types::Header13& hdr = m_las_stream->header();
		yalas::MemoryIterator it(m_map.mem_at_ofs<uint8_t>(hdr.offset_to_point_data), m_map.mem_end<uint8_t>(), &hdr.x_offset, &hdr.x_scale);
		
		update_point_cache_with_iterator<format_id>(it, mode);
	} else {
		update_point_cache_with_iterator<format_id>(*m_las_stream, mode);
	}// END handle mmap
}

template <uint8_t format_id, typename IteratorType>
inline void LidarVisNode::update_point_cache_with_iterator(IteratorType& it, const DisplayMode mode)
{
	yalas::types::point_data_record<format_id> p;
	const PosCache::iterator pend = m_pos_cache.end();
	if (mode == DMNoColor) {
		for (PosCache::iterator pit = m_pos_cache.begin(); pit < pend && it.read_next_point(p); ++pit) {
			pit->init_from_point(p);
		}
	} else {
		const ColCache::iterator cend = m_col_cache.end();
		ColCache::iterator cit = m_col_cache.begin();
		for (PosCache::iterator pit = m_pos_cache.begin(); pit < pend && cit < cend && it.read_next_point(p); ++pit, ++cit) {
			pit->init_from_point(p);
			color_point<format_id>(p, *cit, mode);
		}
	}
}

void LidarVisNode::update_compensation_matrix_and_bbox(bool translateToOrigin)
{
	m_compensation_column_major.setToIdentity();
	if (translateToOrigin && m_las_stream.get()) {
		const yalas::types::Header13& hdr = m_las_stream->header();
		// enter data column -major
		m_compensation_column_major.matrix[3][0] = -hdr.min_x;
		m_compensation_column_major.matrix[3][1] = -hdr.min_y;
		m_compensation_column_major.matrix[3][2] = -hdr.min_z;
	}
	m_compensation_column_major *= convert_z_up_to_y_up_column_major;
	
	const yalas::types::Header13& hdr = m_las_stream->header();
	m_bbox = MBoundingBox(
							MPoint(hdr.min_x, hdr.min_y, hdr.min_z) * m_compensation_column_major,
							MPoint(hdr.max_x, hdr.max_y, hdr.max_z) * m_compensation_column_major
				 );
}


// no rgb by default
template <uint8_t format_id>
void LidarVisNode::color_point(const yalas::types::point_data_record<format_id>& p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	color_point_no_rgb(p, dc, mode);
}

// format 2, 3 and 5 have rgb info !
template <>
void LidarVisNode::color_point<2>(const yalas::types::point_data_record<2>& p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	color_point_with_rgb_info(p, dc, mode);
}

template <>
void LidarVisNode::color_point<3>(const yalas::types::point_data_record<3>& p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	color_point_with_rgb_info(p, dc, mode);
}

template <>
void LidarVisNode::color_point<5>(const yalas::types::point_data_record<5>& p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	color_point_with_rgb_info(p, dc, mode);
}

void LidarVisNode::color_point_no_rgb(const yalas::types::PointDataRecord0 &p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	static const uint16_t scale_3_to_16 = std::numeric_limits<uint16_t>::max() / 0x07;
	switch(mode)
	{
	case DMStoredColor: break;	//! handle it like no color in no-rgb mode
	case DMNoColor: break;
	case DMIntensity:
	{
		const uint16_t intensity = p.intensity * m_intensity_scale; 
		dc.col[0] = intensity;
		dc.col[1] = intensity;
		dc.col[2] = intensity;
		break;
	}
	case DMReturnNumber:
	{
		dc.col[0] = p.return_number() * scale_3_to_16;
		dc.col[1] = p.num_returns() * scale_3_to_16;
		dc.col[2] = p.return_number() * scale_3_to_16;
		break;
	}
	case DMReturnNumberIntensity:
	{
		const uint16_t intensity = p.intensity * m_intensity_scale; 
		dc.col[0] = p.return_number() * scale_3_to_16 + intensity;
		dc.col[1] = p.num_returns() * scale_3_to_16 + intensity;
		dc.col[2] = p.return_number() * scale_3_to_16 + intensity;
		break;
	}
	};// end color handler
}

template <typename PointType>
void LidarVisNode::color_point_with_rgb_info(const PointType &p, DrawCol& dc, const LidarVisNode::DisplayMode mode) const
{
	switch(mode)
	{
	case DMStoredColor:
	{
		if (m_normalize_stored_cols) {
			// assume its normalized to 8 bit, instead of 16
			dc.col[0] = p.red * 256;
			dc.col[1] = p.green * 256;
			dc.col[2] = p.blue * 256;
		} else {
			dc.col[0] = p.red;
			dc.col[1] = p.green;
			dc.col[2] = p.blue;
		}
		break;
	}
	default:
	{
		color_point_no_rgb(p, dc, mode);
		break;
	}
	};
}

bool LidarVisNode::setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx)
{
	if (plug == aLidarFileName) {
		if (renew_las_reader(dataHandle.asString())) {
			update_compensation_matrix_and_bbox(MPlug(thisMObject(), aTranslateToOrigin).asBool());
		} else {
			reset_caches();
		}
	} 
	else if (plug == aGlPointSize) {
		m_gl_point_size = dataHandle.asFloat();
	} else if (plug == aIntensityScale) {
		m_intensity_scale = dataHandle.asFloat();
	} else if (plug == aTranslateToOrigin) {
		// Have to do the update right when it happens as otherwise, we might not realize soon enough
		// that there is a new bbox if the update is triggered by the drawing itself, which is heavily
		// affected by the bbox !
		update_compensation_matrix_and_bbox(dataHandle.asBool());
	} else if (plug == aNormalizeStoredCols) {
		m_normalize_stored_cols = dataHandle.asBool();
	}
	
	return false;
}


MStatus LidarVisNode::compute(const MPlug& plug, MDataBlock& data)
{
	// in all cases, just set us clean - if we are not setup, we would be called
	// over and over again, lets limit this to just when something changes
	data.setClean(plug);
	
	if (plug == aNeedsCompute) {
		MDataHandle ncHandle = data.outputValue(aNeedsCompute);
		// indicate error right awway
		ncHandle.setInt(1);
		
		// UPDATE MEMORY MAP
		/////////////////////
		// Do so first, as display cache generation is affected by this
		if (data.inputValue(aUseMMap).asBool()) {
			if (!m_map.is_mapped()) {
				m_map.map_file(resolved_filepath(data.inputValue(aLidarFileName).asString()).asChar());
			}
		} else {
			m_map.unmap_file();
		}
		
		// Update caches which would be relevant for drawing !
		if (data.inputValue(aDisplayCacheMode).asBool()) {
			if (m_las_stream.get() == 0) {
				reset_caches();
				return MS::kSuccess;
			}
			// NOTE: could only update colors if required, and release the stream
			// Currently it will re-read the positions as well, which is not required !
			// However, it probably doesn't matter in terms of IO, except that we would 
			// save a little MemoryIO
			update_draw_cache(data);
		} else {
			// free all data
			reset_draw_caches();
		}
		
		// indicate everything is fine
		m_error.clear();
		ncHandle.setInt(0);
		return MS::kSuccess;
	} else {
		// Assume its an output plug
		if (!m_las_stream.get()) {
			reset_output_attributes(data);
			return MS::kSuccess;
		}
		assert(m_las_stream->status() == yalas::IStream::Success);
		
		const yalas::types::Header13& hdr = m_las_stream->header();
		
		data.outputValue(aOutSystemIdentifier).setString(hdr.system_identifier);
		data.outputValue(aOutGeneratingSoftware).setString(hdr.generating_software);
		{
			MString date;
			date += hdr.creation_day_of_year;
			date += "/";
			date += hdr.creation_year;
			data.outputValue(aOutCreationDate).setString(date);
		}
		{
			MString version;
			version += hdr.version_major;
			version += ".";
			version += hdr.version_minor;
			data.outputValue(aOutVersionString).setString(version);
		}
		data.outputValue(aOutNumVariableRecords).setInt(hdr.num_variable_length_records);
		data.outputValue(aOutPointDataFormat).setInt(hdr.point_data_format_id);
		data.outputValue(aOutNumPointRecords).setInt(hdr.num_point_records);
		
		data.outputValue(aOutPointScale).set3Float(hdr.x_scale, hdr.y_scale, hdr.z_scale);
		data.outputValue(aOutPointOffset).set3Float(hdr.x_offset, hdr.y_offset, hdr.z_offset);
		data.outputValue(aOutPointBBoxMin).set3Float(hdr.min_x, hdr.min_y, hdr.min_z);
		data.outputValue(aOutPointBBoxMax).set3Float(hdr.max_x, hdr.max_y, hdr.max_z);
		
		return MS::kSuccess;
	}
}

void LidarVisNode::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus)
{
	// make sure we are uptodate - trigger compute
	MPlug(thisMObject(), aNeedsCompute).asInt();
	const DisplayMode mode = static_cast<const DisplayMode>(MPlug(thisMObject(), aDisplayMode).asInt());
	
	view.beginGL();
	if (m_error.length()) {
		view.drawText(MString("Error: ") + m_error, MPoint());
		goto finish_drawing;
	}
	
	{ // start drawing
		
		MHardwareRenderer* renderer = MHardwareRenderer::theRenderer();
		if (!renderer) {
			m_error = "No hardware renderer";
			goto finish_drawing;
		}
		
		MGLFunctionTable* glf = renderer->glFunctionTable();
		if (!glf) {
			m_error = "No function table";
			goto finish_drawing;
		}
		
		glf->glPointSize(m_gl_point_size);
		glf->glPushMatrix();
		glf->glMultMatrixd(&m_compensation_column_major.matrix[0][0]);
		{
			if (m_pos_cache.size()) {
				glf->glPushClientAttrib(MGL_CLIENT_VERTEX_ARRAY_BIT);
				glf->glPushAttrib(MGL_ALL_ATTRIB_BITS);
				{
					glf->glEnableClientState(MGL_VERTEX_ARRAY);
					glf->glVertexPointer(3, MGL_INT, 0, m_pos_cache.data());
					if (m_col_cache.size()) {
						glf->glEnableClientState(MGL_COLOR_ARRAY);
						glf->glColorPointer(3, MGL_UNSIGNED_SHORT, 0, m_col_cache.data());
					}
					
					glf->glDrawArrays(MGL_POINTS, 0, m_pos_cache.size());
					if (glf->glGetError() != 0) {
						m_error = "display cache not supported";
						MPlug(thisMObject(), aDisplayCacheMode).setBool(false);
					}
				}
				glf->glPopAttrib();
				glf->glPopClientAttrib();
			} else {
				assert(m_las_stream.get());
				yalas::IStream& las_stream = *m_las_stream.get();
				if (las_stream.reset_point_iteration() != yalas::IStream::Success) {
					m_error = "could not initialize LAS stream for iteration";
					goto finish_drawing;
				}
				
				glf->glBegin(MGL_POINTS);
				{
					// PERFORM DRAWING
					///////////////////
					switch(las_stream.header().point_data_format_id)
					{
					case 0: draw_point_records<0>(glf, las_stream, mode); break;
					case 1: draw_point_records<1>(glf, las_stream, mode); break;
					case 2: draw_point_records<2>(glf, las_stream, mode); break;
					case 3: draw_point_records<3>(glf, las_stream, mode); break;
					case 4: draw_point_records<4>(glf, las_stream, mode); break;
					case 5: draw_point_records<5>(glf, las_stream, mode); break;
					default: {
						m_error = "Unknown point format: ";
						m_error += las_stream.header().point_data_format_id;
						break;
					}
					}
				}
				glf->glEnd();
			}// end handle caching
		}
		glf->glPopMatrix();
	}
	
finish_drawing:
	view.endGL();
	if (m_las_stream.get() && m_las_stream->status() != yalas::IStream::Success) {
		// can happen if something is not okay with the file. Remove it if its in an invalid state.
		renew_las_reader(MString());
	}
}

template <uint8_t format_id>
void LidarVisNode::draw_point_records(MGLFunctionTable* glf, yalas::IStream& las_stream, const DisplayMode mode) const
{
	if (m_map.is_mapped()) {
		const yalas::types::Header13& hdr = las_stream.header();
		yalas::MemoryIterator it(m_map.mem_at_ofs<uint8_t>(hdr.offset_to_point_data), m_map.mem_end<uint8_t>(), &hdr.x_offset, &hdr.x_scale);
		
		draw_piont_records_with_iterator<format_id>(it, glf, mode);
	} else {
		draw_piont_records_with_iterator<format_id>(las_stream, glf, mode);
	}// END handle mmap
}

template <uint8_t format_id, typename IteratorType>
inline void LidarVisNode::draw_piont_records_with_iterator(IteratorType& it, MGLFunctionTable* glf, const DisplayMode mode) const
{
	DrawCol dc;
	yalas::types::point_data_record<format_id> p;
	if (mode == DMNoColor) {
		while(it.read_next_point(p)) {
			glf->glVertex3iv(static_cast<const MGLint*>(&p.x));
		}
	} else {
		while (it.read_next_point(p)) {
			color_point<format_id>(p ,dc, mode);
			glf->glColor3usv(dc.col);
			glf->glVertex3iv(static_cast<const MGLint*>(&p.x));
		}// end while iterating points
	}
}

MBoundingBox LidarVisNode::boundingBox() const
{
	return m_bbox;
}

bool LidarVisNode::isBounded() const
{
	return m_las_stream.get() != 0;
}
