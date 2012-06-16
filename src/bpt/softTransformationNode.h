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

// softTransformationNode.h: interface for the softTransformationNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOFTTRANSFORMATIONNODE_H__32ED4A13_95F6_4866_91F7_4789D7DB7FF9__INCLUDED_)
#define AFX_SOFTTRANSFORMATIONNODE_H__32ED4A13_95F6_4866_91F7_4789D7DB7FF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <maya/MTypeId.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MDataHandle.h>
#include <maya/MPxNode.h>
#include <maya/MPlug.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnData.h>
#include <maya/MVector.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
#include <maya/MDistance.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnAnimCurve.h>

#include "BPT_Helpers.h"



#include "MACROS.h"


namespace BPT
{

/** Spezielle node zur Verwendung im  softTransformationTool - eventuell wird sie durch eine echte deformerNode ersetzt

  @todo
	Soll noch eine softTransformationEngine erhalten, und an einen matrixManipulator gekoppelt werden
	Diese node muss nochmal ueberarbeitet und auf den neuesten Stand gebracht werden

*/

class softTransformationNode  : public MPxNode
{
public:
	softTransformationNode();
	virtual ~softTransformationNode();


	//Overrides
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	virtual bool  setInternalValue ( const MPlug &, const MDataHandle &); 

	static  void*		creator();
	static  MStatus		initialize();

	//attribute
	static MObject	outPnts;		//!< plugArrayAttribut
	static MObject	inMesh;
	static MObject	inVertices;		//!<selektierte Vertizen
	static MObject	outMesh;

	static MObject	startPoint;
	static MObject	endPoint;

	static MObject	scaleObj;
	static MObject  alongNormalObj;
	
	static MObject	finalizeObj;
	static MObject	normalRelativeOBJ;

	static MObject	radius;
	static MObject	maskSizeOBJ;
	
	static MObject	fTypeOBJ;
	static MObject	inverseMatrix;	//!<hlt die inverseMAtrix des meshes, weil der Vektor in WorldSpace geliefert wird und umgerechnet werden muss

	static MObject	customFalloff;
	static MObject	falloffVis;

	static MObject	outWeights;

	static	MTypeId		id;

private:
//-------------------------------------
//PRIVATE METHODEN
//-------------------------------------
//erledigen allgemeine Aufgaben

	void		clearAnimCurve();

	bool		doTweak();
	
	void		recalculateTweakScaleFactors(MFnMesh& fnMesh, bool force = false);
	
	void		rebuildTweakArrays(MItMeshPolygon& polyIter, MItMeshVertex& vertIter);
	
	void		generateTweakMask(MItMeshVertex&	vertIter,MItMeshPolygon& polyIter);
	

	void		growVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter, MItMeshPolygon& polyIter);	
	void		schrinkVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter, MItMeshPolygon& polyIter);
	

	
	
//-----------------
//VARIABLEN
//----------------
	
	MObject			fMesh;
	BPT_Helpers		helper;


//TWEAK ARRAYS
//---------------

	MPointArray		tweakOrigPos;

	MVectorArray	scaleVectors;		//vectoren vom startPunkt zu OrigPos
	MVectorArray	tweakNormals;
	MDoubleArray	tweakNormalScale;
	
	MIntArray		tweakVtx;
	MDoubleArray	tweakVtxScale;
	MDoubleArray	recalTweakVtxScale;
	MIntArray		ptVtx;




	MIntArray		inVtx;
	
	

	MFnAnimCurve	animFn;

//FLAGS UND INPUT WERTE
//------------------------

	double			fDistance;
	double			scale;
	double			alongNormal;
	double			maxScale;	//scaleFactor, um poppen zu vermeiden beim umschalten zwischen normalRelative true oder nicht

	bool			isNormalRelative;

	int				maskSize;

	int				computeNumVtx;

	int				fType;
	
	MVector			vector;
	MPoint			startPos;
	MPoint			origin;

	int				oldMaskSize;
	int				oldFType;
	
	bool			vis;
	bool			oldVis;

	double			oldFDistance;

	bool			vtxColorsStored;

	bool			onStartup;

};

} //namespace BPT ende

#endif // !defined(AFX_SOFTTRANSFORMATIONNODE_H__32ED4A13_95F6_4866_91F7_4789D7DB7FF9__INCLUDED_)
