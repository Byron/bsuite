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


#include "mayabaselib/base.h"
#include "util.h"
#include "crvnode.h"

#include "assert.h"


//********************************************************************
//**	Class Implementation
//********************************************************************
/////////////////////////////////////////////////////////////////////

const MTypeId MeshCurvatureHWShader::typeId(0x00108bf9);
const MString MeshCurvatureHWShader::typeName("MeshCurvatureHWShader");


// Attributes
MObject MeshCurvatureHWShader::aCurveMap;


MeshCurvatureHWShader::MeshCurvatureHWShader()
{}

MeshCurvatureHWShader::~MeshCurvatureHWShader()
{}

void MeshCurvatureHWShader::postConstructor()
{
	setMPSafe(false);
}

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

	MFnTypedAttribute fnType;

	aCurveMap = MRampAttribute::createColorRamp("curvatureMap", "cm", &status);
	CHECK_MSTATUS(status);

	// Add attributes
	/////////////////
	CHECK_MSTATUS(addAttribute(aCurveMap));

	return MS::kSuccess;
}

MStatus MeshCurvatureHWShader::bind(const MDrawRequest& request, M3dView& view)
{
    view.beginGL();

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    view.endGL();

    return MS::kSuccess;
}


MStatus MeshCurvatureHWShader::unbind(const MDrawRequest& request,
               M3dView& view)
{
    view.beginGL(); 

    glPopClientAttrib();
    glPopAttrib();

    view.endGL();

    return MS::kSuccess;
}

bool MeshCurvatureHWShader::setInternalValueInContext( const MPlug& plug,
												   const MDataHandle& handle,
												   MDGContext& ctx)
{
	return MPxHwShaderNode::setInternalValueInContext(plug, handle, ctx);
}

// Compute colors per vertex, and map them according to the given ramp attribute
MStatus recomputeCurvatureToMesh(MFnMesh& mesh)
{
	MStatus stat;
	return stat;
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
	// 
    // if( colorCount > 0 && colorArrays[ colorCount - 1] != NULL )
    // {
    //     glPushAttrib(GL_ALL_ATTRIB_BITS);
    //     glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    //     glDisable(GL_LIGHTING);

    //     glEnableClientState(GL_COLOR_ARRAY);
    //     glColorPointer( 4, GL_FLOAT, 0, &colorArrays[colorCount - 1][0]);

    //     glEnableClientState(GL_VERTEX_ARRAY);
    //     glVertexPointer ( 3, GL_FLOAT, 0, &vertexArray[0] );
    //     glDrawElements ( prim, indexCount, GL_UNSIGNED_INT, indexArray );

    //     glEnableClientState(GL_COLOR_ARRAY);

    //     glPopClientAttrib();
    //     glPopAttrib();

    //     return MS::kSuccess;
    // }
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

