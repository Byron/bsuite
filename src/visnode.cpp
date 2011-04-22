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
#include <maya/MFnMesh.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFloatVector.h>

#include <maya/MHardwareRenderer.h>
#include <maya/MGLFunctionTable.h>

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
MObject PtexVisNode::aGlPointSize;


PtexVisNode::PtexVisNode()
	: m_ptex_num_channels(0)
    , m_gl_point_size(1.0)
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
	MFnNumericAttribute numFn;
	MFnTypedAttribute typFn;
	MFnStringArrayData stringArrayFn;

	// Input attributes
	////////////////////
	aPtexFileName = typFn.create("ptexFilePath", "ptfp", MFnData::kString, &status);
	CHECK_MSTATUS(status);
	typFn.setInternal(true);
	
	aInMesh = typFn.create("inMesh", "i", MFnData::kMesh, &status);
	typFn.setDefault(MObject::kNullObj);
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

	aPtexFilterSize = numFn.create("ptexFilterSize", "ptfs", MFnNumericData::kFloat, 1.0);
	numFn.setKeyable(true);
	
	aGlPointSize = numFn.create("glPointSize", "glps", MFnNumericData::kFloat, 1.0);
	numFn.setKeyable(true);
	numFn.setInternal(true);
	
	// Output attributes
	/////////////////////
	aNeedsCompute = numFn.create("needsComputation", "nc", MFnNumericData::kInt);
	setup_output(numFn);
	
	aOutMetaDataKeys = typFn.create("outMetaDataKeys", "omdk", MFnData::kStringArray, &status);
	CHECK_MSTATUS(status);
	typFn.setDefault(stringArrayFn.create());
	setup_output(typFn);
	
	aOutNumChannels = numFn.create("outNumChannels", "onc", MFnNumericData::kInt);
	setup_output(numFn);
	
	aOutNumFaces = numFn.create("outNumFaces", "onf", MFnNumericData::kInt);
	setup_output(numFn);
	
	aOutAlphaChannel = numFn.create("outAlphaChannel", "oac", MFnNumericData::kInt);
	setup_output(numFn);
	
	aOutHasEdits = numFn.create("outHasEdits", "ohe", MFnNumericData::kBoolean);
	setup_output(numFn);
	
	aOutHasMipMaps = numFn.create("outHasMipMaps", "ohm", MFnNumericData::kBoolean);
	setup_output(numFn);
	
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
	CHECK_MSTATUS(addAttribute(aGlPointSize));
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

void PtexVisNode::release_cache()
{
	m_sample_col.clear();
	m_sample_pos.clear();
}

bool PtexVisNode::setInternalValueInContext(const MPlug &plug, const MDataHandle &dataHandle, MDGContext &ctx)
{
	if (plug == aPtexFileName) {
		release_texture_and_filter();
		release_cache();
	} else if (plug == aGlPointSize) {
		m_gl_point_size = dataHandle.asFloat();
	}
	
	return false;
}

bool PtexVisNode::update_sample_buffer(MDataBlock& data)
{
	PtexTexture* tex = m_ptex_texture.get();
	PtexFilter* filter = m_ptex_filter.get();
	
	if (!tex | !filter) {
		m_error = "Cannot sample ptex without a valid texture and filter";
		return false;
	}
	
	if (tex->numChannels() < 3 || tex->numChannels() > 4) {
		m_error = "Can only handle 3 or 4 channels currently";
		return false;
	}
	
	if (tex->meshType() != Ptex::mt_triangle)  {
		m_error = "Cannot visualize non-triangle meshes for now";
		return false;
	}
	
	MStatus stat;
	MFnMesh meshFn(data.inputValue(aInMesh).data(), &stat);
	if (stat.error()) {
		m_error = "no mesh provided";
		return false;
	}
	
	// For now, lets support one-on-one mappings without sub-face support
	if (meshFn.numPolygons() != tex->numFaces()) {
		m_error = "Face count of texture does not match polygon count of connected mesh. Currently these must match one on one";
		return false;
	}
	
	
	// lets just read the samples for now
	const int numFaces = tex->numFaces();
	const int numChannels = tex->numChannels();
	
	// count memory we require for preallocation
	size_t numTexels = 0;
	for (int i = 0; i < numFaces; ++i) {
		numTexels += tex->getFaceInfo(i).res.size();
	}// for each face
	
	release_cache();
	m_sample_pos.reserve(numTexels);
	m_sample_col.reserve(numTexels);
	
	const float step = 0.01f;
	Float4 p;								// one pixel
	Float3 pos = {0.f, 0.f, 0.f};			// position
	// const Float3* vtx = reinterpret_cast<const Float3*>(meshFn.getRawPoints());
	
	for (int i = 0; i < numFaces; ++i) {
		const Ptex::FaceInfo& fi = tex->getFaceInfo(i);
		for (int u = 0; u < fi.res.u(); u++) {
			for (int v = 0; v < fi.res.v(); ++v) {
				tex->getPixel(i, u, v, &p.x, 0, numChannels);
				m_sample_col.push_back(p);
				m_sample_pos.push_back(pos);
				pos.y += step;
			}// for each v texel
			pos.x += step;
			pos.y = 0.0f;
		}// for each u texel
	}// for each face
	
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
		MDataHandle ncHandle = data.outputValue(aNeedsCompute);
		// we are successfull, even if there is no valid texture
		// We only use error codes if the sampling fails
		ncHandle.setInt(1);
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
		
		// We cache the samples for faster drawing, and won't support live-drawing for now
		ncHandle.asInt() = update_sample_buffer(data);
		
		return MS::kSuccess;
	} else if (plug == aOutMetaDataKeys || plug == aOutNumChannels || plug == aOutNumFaces ||
	           plug == aOutAlphaChannel || plug == aOutHasEdits || plug == aOutHasMipMaps  ||
	           plug == aOutMeshType || plug == aOutDataType || 
	           plug == aOutUBorderMode || plug == aOutVBorderMode) {
		if (!assure_texture(data)) {
			return MS::kSuccess;
		}
		
		// Update all file information
		MFnStringArrayData arrayData(data.outputValue(aOutMetaDataKeys).data());
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
		
		data.outputValue(aOutNumChannels).asInt() = tex->numChannels();
		data.outputValue(aOutNumFaces).asInt() = tex->numFaces();
		data.outputValue(aOutHasEdits).asBool() = tex->hasEdits();
		data.outputValue(aOutHasMipMaps).asBool() = tex->hasMipMaps();
		data.outputValue(aOutAlphaChannel).asInt() = tex->alphaChannel();
		data.outputValue(aOutMeshType).asInt() = (int)tex->meshType();
		data.outputValue(aOutDataType).asInt() = (int)tex->dataType();
		data.outputValue(aOutUBorderMode).asInt() = (int)tex->uBorderMode();
		data.outputValue(aOutVBorderMode).asInt() = (int)tex->vBorderMode();
		
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
		view.drawText(MString("Error: ") + m_error, MPoint());
	}
		
	if (!m_sample_pos.size()) {
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
	}
	
finish_drawing:
	view.endGL();
}
