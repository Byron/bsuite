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
#include <maya/MFileObject.h>
#include <maya/MStringArray.h>
#include <maya/MFnMeshData.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFloatVector.h>

#include "util.h"
#include "visnode.h"

#include "assert.h"


/////////////////////////////////////////////////////////////////////

const MTypeId PtexVisNode::typeId(0x00108bdd);
const MString PtexVisNode::typeName("ptexVisNode");


// Attributes
MObject PtexVisNode::aPtexFileName;
MObject PtexVisNode::aPtexFilterType;
MObject PtexVisNode::aPtexFilterSize;
MObject PtexVisNode::aInMesh;

MObject PtexVisNode::aOutNumChannels;
MObject PtexVisNode::aOutNumFaces;
MObject PtexVisNode::aOutHasEdits;
MObject PtexVisNode::aOutHasMipMaps;
MObject PtexVisNode::aOutAlphaChannel;
MObject PtexVisNode::aNeedsCompute;
MObject PtexVisNode::aOutMetaDataKeys;
MObject PtexVisNode::aOutMeshType;
MObject PtexVisNode::aOutDataType;
MObject PtexVisNode::aOutUBorderMode;
MObject PtexVisNode::aOutVBorderMode;


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

void add_border_mode_fields(MFnEnumAttribute& mfnEnum)
{
	mfnEnum.addField("clamp", 0);
	mfnEnum.addField("black", 0);
	mfnEnum.addField("periodic", 0);
}

// DESCRIPTION:
//
MStatus PtexVisNode::initialize()
{
	MStatus status;
	MFnNumericAttribute mfnNum;
	MFnTypedAttribute mfnTyp;
	MFnStringArrayData mfnStringArray;

	// Input attributes
	////////////////////
	aPtexFileName = mfnTyp.create("ptexFilePath", "ptfp", MFnData::kString, &status);
	CHECK_MSTATUS(status);
	mfnTyp.setInternal(true);
	
	aInMesh = mfnTyp.create("inMesh", "i", MFnData::kMesh, &status);
	mfnTyp.setDefault(MObject::kNullObj);
	CHECK_MSTATUS(status);

	MFnEnumAttribute mfnEnum;
	aPtexFilterType = mfnEnum.create("ptexFilterType", "ptft", 0);
	mfnEnum.addField("Point",      0);
	mfnEnum.addField("Bilinear",   1);
	mfnEnum.addField("Box",        2);
	mfnEnum.addField("Gaussian",   3);
	mfnEnum.addField("Bicubic",    4);
	mfnEnum.addField("BSpline",    5);
	mfnEnum.addField("CatmullRom", 6);
	mfnEnum.addField("Mitchell",   7);
	mfnEnum.setKeyable(true);

	aPtexFilterSize = mfnNum.create("ptexFilterSize", "ptfs", MFnNumericData::kFloat, 1.0);
	mfnNum.setKeyable(true);
	
	// Output attributes
	/////////////////////
	aNeedsCompute = mfnNum.create("needsComputation", "nc", MFnNumericData::kFloat, 0.0);
	setup_output(mfnNum);
	
	aOutMetaDataKeys = mfnTyp.create("outMetaDataKeys", "omdk", MFnData::kStringArray, &status);
	CHECK_MSTATUS(status);
	mfnTyp.setDefault(mfnStringArray.create());
	setup_output(mfnTyp);
	
	aOutNumChannels = mfnNum.create("outNumChannels", "onc", MFnNumericData::kInt);
	setup_output(mfnNum);
	
	aOutNumFaces = mfnNum.create("outNumFaces", "onf", MFnNumericData::kInt);
	setup_output(mfnNum);
	
	aOutAlphaChannel = mfnNum.create("outAlphaChannel", "oac", MFnNumericData::kInt);
	setup_output(mfnNum);
	
	aOutHasEdits = mfnNum.create("outHasEdits", "ohe", MFnNumericData::kBoolean);
	setup_output(mfnNum);
	
	aOutHasMipMaps = mfnNum.create("outHasMipMaps", "ohm", MFnNumericData::kBoolean);
	setup_output(mfnNum);
	
	aOutMeshType = mfnEnum.create("outMeshType", "omt");
	setup_output(mfnEnum);
	mfnEnum.addField("triangle", 0);
	mfnEnum.addField("quad", 1);
	
	aOutDataType = mfnEnum.create("outDataType", "odt");
	setup_output(mfnEnum);
	mfnEnum.addField("int8", 0);
	mfnEnum.addField("int16", 1);
	mfnEnum.addField("half", 2);
	mfnEnum.addField("float", 3);
	
	aOutUBorderMode = mfnEnum.create("outUBorderMode", "oubm");
	setup_output(mfnEnum);
	add_border_mode_fields(mfnEnum);
	aOutVBorderMode = mfnEnum.create("outVBorderMode", "ovbm");
	setup_output(mfnEnum);
	add_border_mode_fields(mfnEnum);
	

	// Add attributes
	/////////////////
	CHECK_MSTATUS(addAttribute(aPtexFileName));
	CHECK_MSTATUS(addAttribute(aPtexFilterType));
	CHECK_MSTATUS(addAttribute(aPtexFilterSize));
	CHECK_MSTATUS(addAttribute(aInMesh));
	
	CHECK_MSTATUS(addAttribute(aOutMetaDataKeys));
	CHECK_MSTATUS(addAttribute(aNeedsCompute));
	CHECK_MSTATUS(addAttribute(aOutNumChannels));
	CHECK_MSTATUS(addAttribute(aOutNumFaces));
	CHECK_MSTATUS(addAttribute(aOutAlphaChannel));
	CHECK_MSTATUS(addAttribute(aOutHasEdits));
	CHECK_MSTATUS(addAttribute(aOutHasMipMaps));
	CHECK_MSTATUS(addAttribute(aOutMeshType));
	CHECK_MSTATUS(addAttribute(aOutDataType));
	CHECK_MSTATUS(addAttribute(aOutUBorderMode));
	CHECK_MSTATUS(addAttribute(aOutVBorderMode));
	
	

	// All input affect the output color
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutMetaDataKeys));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutNumChannels));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutNumFaces));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutHasEdits));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutHasMipMaps));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutAlphaChannel));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutMeshType));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutDataType));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutUBorderMode));
	CHECK_MSTATUS(attributeAffects(aPtexFileName,	aOutVBorderMode));
	CHECK_MSTATUS(attributeAffects(aInMesh,			aNeedsCompute));
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

bool PtexVisNode::assure_texture(MDataBlock& data)
{
	if (m_ptex_texture.get()) {
		return true;
	}
	
	const MString& filePath = data.inputValue(aPtexFileName).asString();
	if (filePath.length() == 0) {
		return false;
	}
	
	// ptex crashes ungracefully if the file doesn't exist ... great
	MFileObject file;
	file.setRawFullName(filePath);
	if (!file.exists()) {
		return false;
	}
	
	Ptex::String error;
	PtexTexturePtr ptex(gCache->get(file.resolvedFullName().asChar(), error));	// This pointer interface is ridiculous
	if (!ptex.get()) {
		m_error = error.c_str();
		return false;
	}
	m_ptex_texture.swap(ptex);
	return true;
}

void PtexVisNode::release_texture_and_filter()
{
	PtexTexturePtr ptex;
	PtexFilterPtr pfilter;
	m_ptex_texture.swap(ptex);
	m_ptex_filter.swap(pfilter);
}

bool PtexVisNode::setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx)
{
	if (plug == aPtexFileName) {
		release_texture_and_filter();
	}
	
	return false;
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
		if (!assure_texture(data)) {
			return MS::kSuccess;
		}
		
		// just update our filter (it needs to be recreated as options are not adjustable
		// through the interface)
		const float fSize = data.inputValue(aPtexFilterSize).asFloat();
		const PtexFilter::FilterType fType = to_filter_type(data.inputValue(aPtexFilterType).asInt());
		
		PtexFilter::Options opts(fType, 0, fSize);
		PtexFilterPtr pfilter(PtexFilter::getFilter(m_ptex_texture.get(), opts));
		m_ptex_filter.swap(pfilter);
		
		// Now we are ready for computation, and can leave everything else to the drawing method (for now)
		
		return MS::kSuccess;
	} else if (plug == aOutMetaDataKeys || plug == aOutNumChannels || plug == aOutNumFaces ||
	           plug == aOutAlphaChannel || plug == aOutHasEdits || plug == aOutHasMipMaps  ||
	           plug == aOutMeshType || plug == aOutDataType || 
	           plug == aOutUBorderMode || plug == aOutVBorderMode) {
		if (!assure_texture(data)) {
			return MS::kSuccess;
		}
		
		// Update all file information
		MFnStringArrayData arrayData(data.inputValue(aOutMetaDataKeys).data());
		MStringArray keys = arrayData.array();
		keys.clear();
		
		PtexTexture* tex = m_ptex_texture.get();
		assert(tex);
		
		PtexMetaData* mdata = tex->getMetaData();
		for (int i = 0; i < mdata->numKeys(); ++i) {
			const char* keyName = 0;
			Ptex::MetaDataType dt;
			mdata->getKey(i, keyName, dt	);
			keys.append(MString(keyName ? keyName : "unknown"));
		}
		
		data.inputValue(aOutNumChannels).asInt() = tex->numChannels();
		data.inputValue(aOutNumFaces).asInt() = tex->numFaces();
		data.inputValue(aOutHasEdits).asBool() = tex->hasEdits();
		data.inputValue(aOutHasMipMaps).asBool() = tex->hasMipMaps();
		data.inputValue(aOutAlphaChannel).asInt() = tex->alphaChannel();
		data.inputValue(aOutMeshType).asInt() = (int)tex->meshType();
		data.inputValue(aOutDataType).asInt() = (int)tex->dataType();
		data.inputValue(aOutUBorderMode).asInt() = (int)tex->uBorderMode();
		data.inputValue(aOutVBorderMode).asInt() = (int)tex->vBorderMode();
		
		// set all clean
		MObject* attrs[] = {&aOutMetaDataKeys, &aOutNumChannels, &aOutNumFaces, 
		                    &aOutAlphaChannel, &aOutHasEdits, &aOutHasMipMaps,
		                   &aOutMeshType, &aOutDataType, &aOutUBorderMode, &aOutVBorderMode};
		MObject** end = attrs + (sizeof(attrs) / sizeof(attrs[0]));
		for (MObject** i = attrs; i < end; ++i) {
			data.setClean(**i);
		}
		
		return MS::kSuccess;
	} else {
		return MS::kUnknownParameter;
	}
}

void PtexVisNode::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus)
{
	// make sure we are uptodate
	MPlug(thisMObject(), aNeedsCompute).asInt();
	
	view.beginGL();
	if (m_error.length()) {
		view.drawText(m_error, MPoint());
	}
		
	if (m_ptex_filter.get()) {
		
	}// end have filter
	
	view.endGL();
}
