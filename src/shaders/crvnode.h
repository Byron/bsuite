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

#ifndef CURVATURE_VISUALIZATION_SHADER
#define CURVATURE_VISUALIZATION_SHADER

#include <maya/MPxHwShaderNode.h>

#include <fstream>
#include <memory>
#include <vector>



//! Node helping to visualize lidar data
class MeshCurvatureHWShader : public MPxHwShaderNode
{
	public:
	
	public:
		MeshCurvatureHWShader();
		virtual ~MeshCurvatureHWShader();
		
		virtual void postConstructor() { setMPSafe(true); }
		virtual int normalsPerVertex() { return 1; }

		MStatus bind(const MDrawRequest&, M3dView&);
		MStatus unbind(const MDrawRequest&, M3dView&);
		virtual MStatus     geometry( const MDrawRequest& request,
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
								const float ** texCoordArrays);

		static  void*   creator();
		static  MStatus initialize();
		MStatus compute( const MPlug&, MDataBlock&);
		
		static const MTypeId typeId;				//!< binary file type id
		static const MString typeName;				//!< node type name

	protected:
		// Input attributes
		static MObject aUseMap;					//!< a switch to enable expensive color remapping
		static MObject aCurveMap;				//!< a ramp attribute to allow mapping 
		static MObject aFlatShading;			//!< if set, we will also light the surface
		static MObject aScale;					//!< a scale factor, to handle differently scaled scenes

};

#endif
