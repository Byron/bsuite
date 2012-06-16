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

#ifndef _ByronsPolyToolsNode
#define _ByronsPolyToolsNode
//  Copyright (C) 2003 Sebastian Thiel
// 
// File: splitUVNode.h
//
// Dependency Graph Node: splitUVNode
//
// Authors: Lonnie Li, Jeyprakash Michaelraj
//

#include <polyModifierNode.h>
#include <BPTfty.h>
#include <BPTfty_NH.h>
#include <BPT_Helpers.h>
#include <BPT_BA.h>
#include <componentConverter.h>
#include <maya/MPxManipContainer.h>
// General Includes
//
#include <maya/MTypeId.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnSingleIndexedComponent.h>

//#include <maya/MCallbackId.h>
#include <maya/MModelMessage.h>

// Function Sets
//
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnComponentListData.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MEventMessage.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnMatrixData.h>

// General Includes
//

#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <iostream>
#include <softTransformationEngine.h>
 


namespace BPT
{

/** Hat die Fhigkeit, einen smartSplit, Chamfer oder solifChamfer auszufuehren

  @note
	Die eigentliche arbeit wird von Factories erledigt. Die Node selbst extrahiert im Grunde
	nur die ntigen Daten und leitet diese an die Factories weiter

*/

class ByronsPolyToolsNode : public MPxNode, public softTransformationEngine
{
public:
						ByronsPolyToolsNode();
	virtual				~ByronsPolyToolsNode(); 
	

	//Overrides
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	
	virtual bool		setInternalValue ( const MPlug & plug, const MDataHandle & dataHandle);
	
	virtual MStatus		connectionBroken( const MPlug& plug, const MPlug& otherPlug, bool asSrc );
	virtual MStatus		connectionMade( const MPlug& plug, const MPlug& otherPlug, bool asSrc );
	

	static  void*		creator();
	static  MStatus		initialize();


// ------------------
// PRIVATE METHODEN
private:
// ------------------
	MStatus				doCompleteCompute(MDataBlock& data);


// ------------------
// OEFFENTLICHE ATTRIBUTE
public:
// ------------------


	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// polyModifierNode has predefined the standard inMesh and outMesh attributes.
	//
	// We define an input attribute for our componentList Attribute
	//
	
	static	MObject		mode;
	static	MObject		slide;		//!<Wert fuer slide
	static	MObject		normal;		//!<wert für bewegung entlang der normale
	
	static	MObject		selEdgeIDs;	//!<arrayAttribute
	static	MObject		selVertIDs;	//!<arrayAttribute
	static	MObject		selFaceIDs;	//!<arrayAttribute
	
	static	MObject		slideRelative;	//!<bool ob relativer Slide oder absolut
	static	MObject		normalRelative;

	static	MObject		whichSide;	//!<von welcher Seite soll der Slide ausgene?
	static	MObject		options;	//!<Festes Array
	static	MObject		slideLimitedObj;//!< True, wenn slide nur zwischen 0 und 1 erlaubt sein soll

	static	MObject		BPTinMesh;	//!<inMeshObj fuer BPTNode
	static	MObject		BPToutMesh;	//!<inMeshObj fuer BPTNode
	
	STE_DEFINE_STATIC_PARAMETERS(BPT)

	//options[0] == 0: nix				//options[1] == 0: nix				
	//options[0] == 1: help				//options[1] == 1: connectEnd			//dies eventuell rausnehmen		
										//options[1] == 2: connectAllEnds
	
	//options[2] == MaxEdgeCount	//Wieviele Edges darf ein EL/ER lang sein?
	//options[3] == whichSide		//von welcher Seite aus soll geslided werden? 
		

// ------------------
// OEFFENTLICHE VARIABLEN
public:
// ------------------

	static	MTypeId		id;
	
	bool				check;
	bool				isAutoSelect;
	bool				isSCMode;

	MIntArray			validIndices;


	MCallbackId			eID;



// ------------------
// PRIVATE VARIABLEN
private:
// ------------------

	struct	parameters
	{
		long mode;
		
		long normalRelative;
		long slideRelative;
		long whichSide;

		double slide;
		double alongNormal;
		MMatrix	matrix;
		
		
		bool operator == (const parameters& rhs)
		{
			return	mode == rhs.mode 
				&&
					normalRelative == rhs.normalRelative
				&&
					slideRelative == rhs.slideRelative
				&& 
					whichSide == rhs.whichSide
				&&
					slide == rhs.slide
				&&
					alongNormal == rhs.alongNormal
				&&
					matrix == rhs.matrix;
		};


		void operator = (const parameters& rhs)
		{
				mode = rhs.mode;
				
				normalRelative = rhs.normalRelative;
				
				slideRelative = rhs.slideRelative;
				
				whichSide = rhs.whichSide;
				
				slide = rhs.slide;
				
				alongNormal = rhs.alongNormal;

				matrix = rhs.matrix;
		}


	};

	
	parameters			cp;		//!< currentParameters
	parameters			op;		//!< oldParameters - um vergleichn zu knnen, ob sich parameter verndert haben

	BPT_Helpers			helper; 
	BPTfty				fBPTfty;

	MPlug				otherOutMeshPlug;

	MDGModifier*		dgMod;

	bool				meshDirty;
	bool				oldShowMesh;
	bool				undoItCalled;


	uint				initialVtxCount;		//!< Hlt den VtxCount bei der ersten echten Berechnung (meshDirty = true)		

	

	

	
};

}

#endif
