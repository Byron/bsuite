/*
 * Copyright (C) 2002 Sebastian Thiel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef _visMeshNode
#define _visMeshNode


#include <maya/MPxLocatorNode.h> 
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/M3dView.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFloatVector.h>
#include <maya/MGlobal.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MItMeshVertex.h> 
#include <maya/MItMeshPolygon.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>

#include <MACROS.h>
#include "baselib/math_util.h"

class MGLFunctionTable;

namespace BPT
{

/* Diese Node ist in der lage, Weights grafisch darzustellen mit direkten openGl calls

*/

// ***********************************************
class visualizeMeshNode : public MPxLocatorNode
// ***********************************************
{
public:
							visualizeMeshNode();
	virtual					~visualizeMeshNode(); 

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

	virtual void            draw( M3dView & view, const MDagPath & path, 
								  M3dView::DisplayStyle style,
								  M3dView::DisplayStatus status );

	virtual bool            isBounded() const;
	
	MBoundingBox			boundingBox() const;

	virtual bool			excludeAsLocator() const {return false;}

	static  void *          creator();
	static  MStatus         initialize();


	// ----------
	// ATTRIBUTE
	// ----------

	//Input
	static	MObject			drawingEnabled;
	static	MObject			pointSize;
	static  MObject         inputMesh;
	static	MObject			vtxWeights;
	static	MObject			vtxColorObj;
	static	MObject			vtxColorObj2;


//* Im DebugModus soll es auch noch Parameter fuer polygonoffset geben
#ifdef DEBUG
	static	MObject			pOffset1Obj;
	static	MObject			pOffset2Obj;
#endif
		//Output
		static	MObject			vtxLocations;


	static	MTypeId			id;


//--------------------
//PRIVATE METHODEN
private:
//--------------------
	inline
	Float3	getCalColor(const Float3& color1, const Float3& color2, double weight); 

	
	enum	meshStatus{	kNone, 
						kSelected, 
						kHilited } lastStat;

	meshStatus	getMeshStatus();


	void	drawPoints( MItMeshVertex& vertIter, float fPointSize, MGLFunctionTable* glf);
	
	void	drawShadedTriangles(MItMeshPolygon& polyIter, 
								MItMeshVertex& vertIter, 
								M3dView::DisplayStyle style, 
								meshStatus meshStat,
								MGLFunctionTable* glf);

//--------------------
//CACHE VARIABLEN
private:
//--------------------

	MDoubleArray	vtxWeightArray;


//--------------------
//DRAW DATA
private:
//--------------------

	GLuint	list;
	bool	listNeedsUpdate;
	bool	wasInCompute;	//!< wird bentigt, damit man nicht stndig neue displayLists aufbaut, whrend man etwas verschiebt - man soll dann einfach so zeichnen

	M3dView::DisplayStyle	lastDMode;

	Float3	vtxColor;
	Float3	vtxColor2;

	

};

} // namespace BPT ende

#endif
