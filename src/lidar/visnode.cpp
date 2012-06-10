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

#include "assert.h"



/////////////////////////////////////////////////////////////////////

const MTypeId LidarVisNode::typeId(0x00108bde);
const MString LidarVisNode::typeName("LidarVisNode");


// input attributes
MObject LidarVisNode::aLidarFileName;
MObject LidarVisNode::aGlPointSize;
MObject LidarVisNode::aUseMMap;
MObject LidarVisNode::aUsePointCache;
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
MObject LidarVisNode::aOutPointBBox;

// other attributes
MObject LidarVisNode::aNeedsCompute;




LidarVisNode::LidarVisNode()
    : m_gl_point_size(1.0)
{}

LidarVisNode::~LidarVisNode()
{}

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
	aLidarFileName = typFn.create("ltexFilePath", "lfp", MFnData::kString, &status);
	CHECK_MSTATUS(status);
	typFn.setInternal(true);
	
	aUseMMap = numFn.create("useMMap", "umm", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS(status);
	typFn.setInternal(true);
	
	aUsePointCache = numFn.create("useDisplaytCache", "udc", MFnNumericData::kBoolean, 0, &status);
	CHECK_MSTATUS(status);
	typFn.setInternal(true);
	
	aDisplayMode = mfnEnum.create("displayMode", "dm");
	mfnEnum.addField("Color", (int)DMColor);
	mfnEnum.addField("ReturnNumber", (int)DMReturnNumber);
	mfnEnum.addField("Intensity", (int)DMIntensity);
	
	mfnEnum.setDefault((short)DMColor);
	mfnEnum.setKeyable(true);
	
	aGlPointSize = numFn.create("glPointSize", "glps", MFnNumericData::kFloat, 1.0);
	numFn.setMin(0.0);
	numFn.setKeyable(true);
	numFn.setInternal(true);
	
	// Output attributes
	/////////////////////
	aNeedsCompute = numFn.create("needsComputation", "nc", MFnNumericData::kInt);
	setup_as_output(numFn);
	
	
	

	// Add attributes
	/////////////////
	CHECK_MSTATUS(addAttribute(aLidarFileName));
	CHECK_MSTATUS(addAttribute(aGlPointSize));
	CHECK_MSTATUS(addAttribute(aUseMMap));
	CHECK_MSTATUS(addAttribute(aUsePointCache));
	CHECK_MSTATUS(addAttribute(aDisplayMode));
	
	CHECK_MSTATUS(addAttribute(aNeedsCompute));
	
	CHECK_MSTATUS(addAttribute(aOutSystemIdentifier));
	CHECK_MSTATUS(addAttribute(aOutGeneratingSoftware));
	CHECK_MSTATUS(addAttribute(aOutCreationDate));
	CHECK_MSTATUS(addAttribute(aOutVersionString));
	CHECK_MSTATUS(addAttribute(aOutNumVariableRecords));
	CHECK_MSTATUS(addAttribute(aOutPointDataFormat));
	CHECK_MSTATUS(addAttribute(aOutNumPointRecords));
	CHECK_MSTATUS(addAttribute(aOutPointScale));
	CHECK_MSTATUS(addAttribute(aOutPointOffset));
	CHECK_MSTATUS(addAttribute(aOutPointBBox));
	
	

	// ATTRIBUTE AFFECTS
	/////////////////////
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutCreationDate));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutGeneratingSoftware));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutNumPointRecords));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutNumVariableRecords));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutPointBBox));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutPointDataFormat));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutPointOffset));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutPointScale));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutSystemIdentifier));
	CHECK_MSTATUS(attributeAffects(aLidarFileName,	aOutVersionString));
	
	CHECK_MSTATUS(attributeAffects(aLidarFileName,  aNeedsCompute));
	CHECK_MSTATUS(attributeAffects(aDisplayMode,	aNeedsCompute));
	CHECK_MSTATUS(attributeAffects(aUseMMap,		aNeedsCompute));
	CHECK_MSTATUS(attributeAffects(aUsePointCache,	aNeedsCompute));

	return MS::kSuccess;
}

bool LidarVisNode::setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx)
{
	if (plug == aLidarFileName) {
		// update lidear stream
	} else if (plug == aGlPointSize) {
		m_gl_point_size = dataHandle.asFloat();
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
		// we are successfull, even if there is no valid texture
		// We only use error codes if the sampling fails
		// indicate error
		ncHandle.setInt(1);
		
		return MS::kSuccess;
	} else {
		// Assume its an output plug
		
		return MS::kSuccess;
	}
}

void LidarVisNode::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus)
{
	// make sure we are uptodate - trigger compute
	MPlug(thisMObject(), aNeedsCompute).asInt();
	
	view.beginGL();
	if (m_error.length()) {
		view.drawText(MString("Error: ") + m_error, MPoint());
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
		
		glf->glPushClientAttrib(MGL_CLIENT_VERTEX_ARRAY_BIT);
		glf->glPushAttrib(MGL_ALL_ATTRIB_BITS);
		/*{
			glf->glEnableClientState(MGL_VERTEX_ARRAY);
			glf->glVertexPointer(3, MGL_FLOAT, 0, m_sample_pos.data());
			glf->glEnableClientState(MGL_COLOR_ARRAY);
			glf->glColorPointer(3, MGL_FLOAT, 0, m_sample_col.data());
			
			glf->glPointSize(m_gl_point_size);
			glf->glDrawArrays(MGL_POINTS, 0, m_sample_pos.size());
		}*/
		glf->glPopAttrib();
		glf->glPopClientAttrib();
	}
	
finish_drawing:
	view.endGL();
}
