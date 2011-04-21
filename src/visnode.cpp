// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================

#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFloatVector.h>

#include "util.h"
#include "visnode.h"


/////////////////////////////////////////////////////////////////////

const MTypeId PtexVisNode::typeId(0x00108bdd);
const MString PtexVisNode::typeName("PtexVisNode");


// Attributes
MObject PtexVisNode::aPtexFileName;
MObject PtexVisNode::aPtexFilterType;
MObject PtexVisNode::aPtexFilterSize;

MObject PtexVisNode::aNeedsCompute;
MObject PtexVisNode::aMetaDataKeys;


PtexVisNode::PtexVisNode()
	: m_ptex_num_channels(0)
{}

PtexVisNode::~PtexVisNode()
{}

void PtexVisNode::postConstructor()
{
	setMPSafe(false);
}

// DESCRIPTION:
// creates an instance of the node
void* PtexVisNode::creator()
{
	return new PtexVisNode();
}

// DESCRIPTION:
//
MStatus PtexVisNode::initialize()
{
	MStatus status;
	MFnNumericAttribute mfnNum;
	MFnTypedAttribute mfnTyp;

	// Input attributes
	////////////////////
	aPtexFileName = mfnTyp.create("ptexFileName", "f", MFnData::kString);
	setup_input(mfnTyp);
	mfnTyp.setConnectable(false);

	MFnEnumAttribute mfnEnum;
	mfnEnum.create("ptexFilterType", "t", 0);
	mfnEnum.addField("Point",      0);
	mfnEnum.addField("Bilinear",   1);
	mfnEnum.addField("Box",        2);
	mfnEnum.addField("Gaussian",   3);
	mfnEnum.addField("Bicubic",    4);
	mfnEnum.addField("BSpline",    5);
	mfnEnum.addField("CatmullRom", 6);
	mfnEnum.addField("Mitchell",   7);
	mfnEnum.setHidden(false);
	mfnEnum.setKeyable(true);
	mfnEnum.setConnectable(false);

	aPtexFilterSize = mfnNum.create("ptexFilterSize", "s", MFnNumericData::kFloat, 1.0);
	mfnNum.setKeyable(true);
	
	// Output attributes
	/////////////////////
	aNeedsCompute = mfnNum.create("needsComputation", "nc", MFnNumericData::kFloat, 0.0);
	setup_output(aNeedsCompute);
	
	aMetaDataKeys = mfnTyp.create("metaData", "md", MFnData::kStringArray);
	setup_output(aMetaDataKeys);

	// Add attributes
	/////////////////
	CHECK_MSTATUS(addAttribute(aPtexFileName));
	CHECK_MSTATUS(addAttribute(aPtexFilterType));
	CHECK_MSTATUS(addAttribute(aPtexFilterSize));
	
	CHECK_MSTATUS(addAttribute(aMetaDataKeys));
	CHECK_MSTATUS(addAttribute(aNeedsCompute));

	// All input affect the output color
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aMetaDataKeys));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,   aNeedsCompute));
	CHECK_MSTATUS(attributeAffects(aPtexFilterSize, aNeedsCompute));
	CHECK_MSTATUS(attributeAffects(aPtexFilterType, aNeedsCompute));

	return MS::kSuccess;
}

PtexFilter::FilterType PtexVisNode::to_filter_type(int type) 
{
	switch (type) {
	case 0: return PtexFilter::f_point;
	case 1: return PtexFilter::f_bilinear;
	case 2: return PtexFilter::f_box;
	case 3: return PtexFilter::f_gaussian;
	case 4: return PtexFilter::f_bicubic;
	case 5: return PtexFilter::f_bspline;
	case 6: return PtexFilter::f_catmullrom;
	case 7: return PtexFilter::f_mitchell;
	default: {
		// usually we would throw, but as long as we are intergated into the node tightly
		// we just handle it.
		cerr << "Invalid filter type: " << type << " - defaulting to point filtering" << endl;
		return PtexFilter::f_point;
	}
	}
}

bool PtexVisNode::assure_filter(MDataBlock& data)
{
	if (m_ptex_texture.get()) {
		return true;
	}
	
	const MString& filePath = data.inputValue(aPtexFileName).asString();
	if (filePath.length() == 0) {
		return false;
	}
	
	Ptex::String error;
	m_ptex_texture = gCache->get(filePath.asChar(), error);
	if (!m_ptex_texture.get()) {
		m_error = error.c_str();
		return false;
	}
	
	return true;
}

MStatus PtexVisNode::compute(const MPlug& plug, MDataBlock& data)
{
	// reset previous error, we only get called whenever something changes
	if (m_error.length()) {
		m_error = MString();
	}
	
	// in all cases, just set us clean - if we are not setup, we would be called
	// over and over again, lets limit this to just when something changes
	data.setClean(plug);
	if (plug == aNeedsCompute) {
		if (!assure_filter(data)) {
			return MS::kSuccess;
		}
		
		// just update our filter (it needs to be recreated as options are not adjustable
		// through the interface)
		const float fSize = data.inputValue(aPtexFilterSize).asFloat();
		const PtexFilter::FilterType fType = to_filter_type(data.inputValue(aPtexFilterType).asInt());
		
		PtexFilter::Options opts(fType, 0, fSize);
		m_ptex_filter = PtexFilter::getFilter(m_ptex_texture.get(), opts);
		
		// Now we are ready for computation, and can leave everything else to the drawing method (for now)
		
		return MS::kSuccess;
	} else if (plug == aMetaDataKeys) {
		if (!assure_filter(data)) {
			return MS::kSuccess;
		}
		return MS::kSuccess;
	} else {
		return MS::kUnknownParameter;
	}
}

void PtexVisNode::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus)
{
	view.beginGL();
	if (m_error.length()) {
		view.drawText(m_error, MPoint());
	}
	
	view.endGL();
}
