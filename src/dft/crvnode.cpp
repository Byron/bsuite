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

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MFnMesh.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MRampAttribute.h>
#include <maya/MColorArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MDrawRequest.h>
#include <maya/MFloatVector.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MGLFunctionTable.h>


#include "mayabaselib/base.h"
#include "baselib/math_util.h"

#include "crvnode.h"

#include <assert.h>
#include <math.h>
#include <float.h>


#ifdef WIN32
float fminf(float a, float b) {
	return (a < b) ? a : b;
}
#endif

inline
MGLFunctionTable* getGL(MStatus& stat){
	MHardwareRenderer* renderer = MHardwareRenderer::theRenderer();
	if (!renderer) {
		stat.perror("There was no hardware renderer");
		return NULL;
	}
	return renderer->glFunctionTable();
}

//********************************************************************
//**	Class Implementation
//********************************************************************
/////////////////////////////////////////////////////////////////////

const MTypeId MeshCurvatureHWShader::typeId(0x00108bf9);
const MString MeshCurvatureHWShader::typeName("MeshCurvatureHWShader");

const double pi_2 = 1.57079632679489661923132169163975144;


// Attributes
MObject MeshCurvatureHWShader::aCurveMap;
MObject MeshCurvatureHWShader::aUseMap;
MObject MeshCurvatureHWShader::aFlatShading;


MeshCurvatureHWShader::MeshCurvatureHWShader()
{}

MeshCurvatureHWShader::~MeshCurvatureHWShader()
{}

// DESCRIPTION:
// creates an instance of the node
void* MeshCurvatureHWShader::creator()
{
	return new MeshCurvatureHWShader();
}

// DESCRIPTION:
//
MStatus MeshCurvatureHWShader::initialize()
{
	MStatus status;

	MFnNumericAttribute fnNum;

	aCurveMap = MRampAttribute::createColorRamp("curvatureMap", "cm", &status);
	CHECK_MSTATUS(status);
	aUseMap = fnNum.create("useCurvatureMap", "ucm", MFnNumericData::kBoolean, 0.0, &status);
	CHECK_MSTATUS(status);
	aFlatShading = fnNum.create("flatShading", "fs", MFnNumericData::kBoolean, 1.0, &status);
	CHECK_MSTATUS(status);

	// Add attributes
	/////////////////
	CHECK_MSTATUS(addAttribute(aUseMap));
	CHECK_MSTATUS(addAttribute(aCurveMap));
	CHECK_MSTATUS(addAttribute(aFlatShading));

	return MS::kSuccess;
}

MStatus MeshCurvatureHWShader::bind(const MDrawRequest& request, M3dView& view)
{
	if (request.displayStyle() == M3dView::kPoints ||
		request.displayStyle() == M3dView::kWireFrame) {
		return MS::kFailure;
	}

	MStatus stat;
	MGLFunctionTable* gl = getGL(stat);
	if (!gl) {
		return stat;
	}

	view.beginGL();
	gl->glPushAttrib( GL_ALL_ATTRIB_BITS );
	view.endGL();

    return MS::kSuccess;
}


MStatus MeshCurvatureHWShader::unbind(const MDrawRequest& request,
               M3dView& view)
{
	MStatus stat;
	MGLFunctionTable* gl = getGL(stat);
	if (!gl) {
		return stat;
	}

	view.beginGL();
	gl->glPopAttrib();
	view.endGL();

    return MS::kSuccess;
}

inline 
float dot(const float*const l, const float*const r) {
	return l[0]*r[0] + l[1]*r[1] + l[2]*r[2];
}

inline
float vlen(const float*const v) {
	return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

inline
void vnorm(const float*const v, float* out) {
	const float l = vlen(v);
	out[0] = v[0] / l;
	out[1] = v[1] / l;
	out[2] = v[2] / l;
}

inline
void computeVertexCurvature(const float triNormal[3], const float vtxNormal[3], MRampAttribute* map, const float normalizer, float outColor[3])
{
	// We remap the value to be 1.0 at 90DEG, and 2.0 at 180DEG
	const float angle = acosf(fminf(dot(vtxNormal, triNormal), 1.0f-FLT_EPSILON)) / ((float)pi_2 * 2 * normalizer);

	if (map) {
		MColor col;
		map->getColorAtPosition(angle, col);
		outColor[0] = (float)col.r;
		outColor[1] = (float)col.g;
		outColor[2] = (float)col.b;
	} else {
		outColor[0] = angle;
		outColor[1] = (1.0f-angle) * 0.75f;
		outColor[2] = 0.0f;
	}
}

MStatus MeshCurvatureHWShader::geometry(const MDrawRequest& request,
			                            M3dView& view,
			                            int prim,
			                            unsigned int writable,
			                            int indexCount,
			                            const unsigned int * indexArray,
			                            int vertexCount,
			                            const int * vertexIDs,
			                            const float * vertexArray,
			                            int normalCount,
			                            const float ** normalArrays,
			                            int colorCount,
			                            const float ** colorArrays,
			                            int texCoordCount,
			                            const float ** texCoordArrays)
{

	// For now, only do triangles, our cache is somewhat
	if (prim != GL_TRIANGLES || normalArrays == NULL || normalArrays[0] == NULL) {
		return MS::kInvalidParameter;
	}

	MStatus stat;
	MGLFunctionTable* gl = getGL(stat);
	if (!gl) {
		return stat;
	}
	const bool flatShading = MPlug(thisMObject(), aFlatShading).asBool();

	if (flatShading) {
		gl->glDisable(GL_LIGHTING);
	} else {
		gl->glEnable(GL_LIGHTING);
		gl->glEnable(GL_COLOR_MATERIAL);
		gl->glColorMaterial(GL_FRONT, GL_DIFFUSE);
	}
	
	float vtxColor[3];
	MRampAttribute mapper(thisMObject(), aCurveMap, &stat);
	CHECK_MSTATUS(stat);
	MRampAttribute* mapPtr = MPlug(thisMObject(), aUseMap).asBool() ? &mapper : NULL;

	const float*const normals = normalArrays[0];
	const unsigned int*const indexArrayEnd = indexArray + indexCount;

	for (const unsigned int* cIndex = indexArray; cIndex < indexArrayEnd;) {
		gl->glBegin(prim);
		{
			// I expect this to be unrolled when optimized

			float triNormal[3] = {0.0f, 0.0f, 0.0f};

			// compute face normal, as average of all given normals
			for (uint i = 0; i < 3; ++i) {
				const unsigned id = cIndex[i]*3;
				triNormal[0] += normals[id+0] / 3.0f;
				triNormal[1] += normals[id+1] / 3.0f;
				triNormal[2] += normals[id+2] / 3.0f;
			}

			float extend = 0.0f;
			float edge[3];
			for (uint i = 0; i < 3; ++i) {
				int a; int b;
				switch(i) {
					case 0: {
						a = 0;
						b = 1;
						break;
					}
					case 1: {
						a = 0;
						b = 2;
						break;
					}
					case 2: {
						a = 1;
						b = 2;
						break;
					}
				}
				const float* va = &vertexArray[cIndex[a] * 3];
				const float* vb = &vertexArray[cIndex[b] * 3];

				edge[0] = va[0] - vb[0];
				edge[1] = va[1] - vb[1];
				edge[2] = va[2] - vb[2];

				extend += vlen(edge);
			}


			for (unsigned int i = 0; i < 3; ++i, ++cIndex) {
				const unsigned drawIndex = *cIndex * 3;
				computeVertexCurvature(triNormal, &normals[drawIndex], mapPtr, extend, vtxColor);

				gl->glColor3fv(vtxColor);
				if (!flatShading) {
					gl->glNormal3fv(&normals[drawIndex]);
				}
				gl->glVertex3fv(&vertexArray[drawIndex]);
			}
		}
		gl->glEnd();
	}// for each triangle

	return MS::kSuccess;
}


MStatus MeshCurvatureHWShader::compute(
const MPlug&      plug,
      MDataBlock& block ) 
{ 
    MDataHandle outColorHandle = block.outputValue( outColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor.x = 0.0f;
	outColor.y = 0.0f;
	outColor.z = 0.0f;
    outColorHandle.setClean();

    return MS::kSuccess;
}

