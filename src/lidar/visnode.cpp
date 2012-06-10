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
#include <maya/MFileObject.h>
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
#include "ogl_headers.h"

#include "base.h"
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
MObject LidarVisNode::aUseDisplayList;
MObject LidarVisNode::aDisplayMode;


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
	, m_display_list(0)
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
	
	aUseMMap = numFn.create("useMMap", "umm", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	aTranslateToOrigin = numFn.create("translateToOrigin", "tto", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numFn.setAffectsWorldSpace(true);
	numFn.setInternal(true);
	
	aUseDisplayList = numFn.create("useDisplayCache", "udc", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	numFn.setInternal(true);
	
	aDisplayMode = mfnEnum.create("displayMode", "dm");
	mfnEnum.addField("Intensity", (int)DMIntensity);
	mfnEnum.addField("ReturnNumber", (int)DMReturnNumber);
	mfnEnum.addField("ReturnNumberIntensified", (int)DMReturnNumberIntensity);
	
	mfnEnum.setDefault((short)DMReturnNumberIntensity);
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
	CHECK_MSTATUS_AND_RETURN_IT(addAttribute(aUseDisplayList));
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
	CHECK_MSTATUS_AND_RETURN_IT(attributeAffects(aTranslateToOrigin,	aNeedsCompute));
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
	
	if (filepath.length() == 0) {
		return false;
	}
	
	MFileObject fobj;
	
	fobj.setRawFullName(filepath);
	const MString res_path = fobj.resolvedFullName();
	m_ifstream.open(res_path.asChar(), std::ios_base::in | std::ios_base::binary);
	if (m_ifstream.fail()) {
		m_error = "Could not open file " + res_path + " for reading";
		return false;
	}
	
	m_las_stream.reset(new LAS_IStream(m_ifstream));
	if (m_las_stream->status() != LAS_IStream::Success) {
		m_error = "Unsupported file format";
		return renew_las_reader(MString());
	}
	
	return true;
}

void LidarVisNode::reset_caches()
{
	m_bbox = MBoundingBox();
	m_compensation_column_major.setToIdentity();
}

void LidarVisNode::update_compensation_matrix_and_bbox(bool translateToOrigin)
{
	m_compensation_column_major.setToIdentity();
	if (translateToOrigin && m_las_stream.get()) {
		const LAS_Types::Header13& hdr = m_las_stream->header();
		// enter data column -major
		m_compensation_column_major.matrix[3][0] = -hdr.min_x;
		m_compensation_column_major.matrix[3][1] = -hdr.min_y;
		m_compensation_column_major.matrix[3][2] = -hdr.min_z;
	}
	m_compensation_column_major *= convert_z_up_to_y_up_column_major;
	
	const LAS_Types::Header13& hdr = m_las_stream->header();
	m_bbox = MBoundingBox(
							MPoint(hdr.min_x, hdr.min_y, hdr.min_z) * m_compensation_column_major,
							MPoint(hdr.max_x, hdr.max_y, hdr.max_z) * m_compensation_column_major
						);
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
		update_compensation_matrix_and_bbox(dataHandle.asBool());
	} else if (plug == aUseDisplayList) {
		m_use_display_list = dataHandle.asBool();
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
		if (m_las_stream.get() == 0) {
			reset_caches();
			return MS::kSuccess;
		}
		
		// Update caches which would be relevant for drawing !
		assert(m_las_stream->status() == LAS_IStream::Success);
		
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
		assert(m_las_stream->status() == LAS_IStream::Success);
		
		const LAS_Types::Header13& hdr = m_las_stream->header();
		
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
	const DisplayMode mode = static_cast<DisplayMode>(MPlug(thisMObject(), aDisplayMode).asInt());
	
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
		
		
		assert(m_las_stream.get());
		LAS_IStream& las_stream = *m_las_stream.get();
		if (las_stream.reset_point_iteration() != LAS_IStream::Success) {
			m_error = "could not initialize LAS stream for iteration";
			goto finish_drawing;
		}
		
		LAS_Types::PointDataRecord1 p;
		
		glf->glPointSize(m_gl_point_size);
		glf->glPushMatrix();
		glf->glMultMatrixd(&m_compensation_column_major.matrix[0][0]);
		
		// HANDLE DISPLAY LIST
		//////////////////////
		bool use_draw_list = false;
		if (m_use_display_list) {
			if (m_display_list == 0) {
				// genLists must not be between glBegin and glEnd
				m_display_list = glf->glGenLists(1);
				// if this doesn't work, we will just keep drawing as usual ... and retry and retry ...
				if (m_display_list == 0) {
					// so we inform about it at least
					MPlug(thisMObject(), aUseDisplayList).setBool(false);
				} else {
					glf->glNewList(m_display_list, MGL_COMPILE_AND_EXECUTE);
					// just continue executing the drawing code to fill the list
				}
			} else {
				use_draw_list = true;
			}
		} else {
			if (m_display_list != 0) {
				glf->glDeleteLists(m_display_list, 1);
				m_display_list = 0;
			}
		}
		
		glf->glBegin(MGL_POINTS);
		{
			static const uint16_t scale_3_to_16 = std::numeric_limits<uint16_t>::max() / 0x07;
			if (use_draw_list) {
				assert(m_display_list);
				glf->glCallList(m_display_list);
			} else {
				// PERFORM DRAWING
				///////////////////
				while (las_stream.read_next_point(p) == LAS_IStream::Success) {
					// setup color
					switch(mode)
					{
					case DMIntensity:
					{
						const uint16_t intensity = p.intensity * m_intensity_scale; 
						glf->glColor3us(intensity, intensity, intensity);
						break;
					}
					case DMReturnNumber:
					{
						glf->glColor3us(p.return_number() * scale_3_to_16, 
										p.num_returns() * scale_3_to_16, 
										p.return_number() * scale_3_to_16);
						break;
					}
					case DMReturnNumberIntensity:
					{
						const uint16_t intensity = p.intensity * m_intensity_scale; 
						glf->glColor3us(p.return_number() * scale_3_to_16 + intensity, 
										p.num_returns() * scale_3_to_16 + intensity, 
										p.return_number() * scale_3_to_16 + intensity);
						break;
					}
					};// end color handler
					
					glf->glVertex3iv(reinterpret_cast<const MGLint*>(&p.x));
				}// end while iterating points
				
				if (m_display_list) {
					glf->glEndList();
				}
			}// end handle use draw list
		}
		glf->glEnd();
		glf->glPopMatrix();
		
		
		
		/*
		glf->glPushClientAttrib(MGL_CLIENT_VERTEX_ARRAY_BIT);
		glf->glPushAttrib(MGL_ALL_ATTRIB_BITS);
		{
			glf->glEnableClientState(MGL_VERTEX_ARRAY);
			glf->glVertexPointer(3, MGL_FLOAT, 0, m_sample_pos.data());
			glf->glEnableClientState(MGL_COLOR_ARRAY);
			glf->glColorPointer(3, MGL_FLOAT, 0, m_sample_col.data());
			
			glf->glPointSize(m_gl_point_size);
			glf->glDrawArrays(MGL_POINTS, 0, m_sample_pos.size());
		}
		glf->glPopAttrib();
		glf->glPopClientAttrib();
		*/
	}
	
finish_drawing:
	view.endGL();
	if (m_las_stream.get() && m_las_stream->status() != LAS_IStream::Success) {
		// can happen if something is not okay with the file. Remove it if its in an invalid state.
		renew_las_reader(MString());
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
