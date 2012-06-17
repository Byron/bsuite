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

#ifndef _BPT_InsertVtx
#define _BPT_InsertVtx
//  Copyright (C) 2003 Sebastian Thiel
// 
// File: splitUVNode.h
//
// Dependency Graph Node: splitUVNode
//
// Authors: Lonnie Li, Jeyprakash Michaelraj
//


#include "IVfty.h"
#include "componentConverter.h"
#include "BPT_Helpers.h"

// General Includes
//
#include <maya/MTypeId.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnSingleIndexedComponent.h>

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
#include <maya/MString.h>

#include <maya/MDagPath.h>

#include <maya/MFnMesh.h>
#include <maya/MPxNode.h>


// Eigene
#include <softTransformationEngine.h>

#include <maya/M3dView.h>

#include <iostream>
using std::cout;
using std::endl;


namespace BPT
{

/** Node, welche Vertizen einfuegen und diese miteinander verbinden kann durch neue edges
	
*/

// ----------------------------------------------------
// ----------------------------------------------------
class BPT_InsertVtx : public MPxNode , public softTransformationEngine
// ----------------------------------------------------
// ----------------------------------------------------
{
public:
						BPT_InsertVtx();
	virtual				~BPT_InsertVtx(); 
	

	//Overrides
	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	
	virtual bool		setInternalValue ( const MPlug & plug, const MDataHandle & dataHandle);
	
	//virtual MStatus	connectionBroken( const MPlug& plug, const MPlug& otherPlug, bool asSrc );
	//virtual MStatus	connectionMade( const MPlug& plug, const MPlug& otherPlug, bool asSrc );
	

	static  void*		creator();
	static  MStatus		initialize();

private:

	MStatus doCompleteCompute( MDataBlock& data );

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// polyModifierNode has predefined the standard inMesh and outMesh attributes.
	//
	// We define an input attribute for our componentList Attribute
	//
	
	static	MObject		IVcount;		//!<wieviele Vtx werden pro kante erstellt



	static	MObject		IVslide;		
	static	MObject		IVnormal;		//!<wert fuer bewegung entlang der normale
	
	static	MObject		IVselEdgeIDs;	//!<speichert die gewhlten edges
	static	MObject		IVselVertIDs;	//!<speichert die Vertizen, die gewhlt waren
	
	
	static	MObject		IVslideRelative;	//!<bool ob relativer Slide oder absolut
	static	MObject		IVnormalRelative;	//!<normalslide relativ zur facegrsse oder nicht?

	static	MObject		IVwhichSide;	//!<von welcher Seite soll der Slide ausgene?
	
	static	MObject		IVspin;		//!<wie rum soll der connect gedreht sein?	->ist internal, damit der Wert immer 0 bleibt, wenn civ nicht aktiv
	
	
	static	MObject		IVoptions;	//!<Festes Array mit optionen

	static	MObject		IVSlideLimited; //!< Wenn true, dann ist slide auf 0 bis 1 limitiert
	
	/*
	options[0] == 0: nix				//options[1] == 0: nix				
	//options[0] == 1: help				//options[1] == 1: connectEnd			//dies eventuell rausnehmen		
										//options[1] == 2: connectAllEnds
	
	//options[2] == MaxEdgeCount	//Wieviele Edges darf ein EL/ER lang sein?
	//options[3] == whichSide		//von welcher Seite aus soll geslided werden? 
	*/

	static	MObject		IVinMesh;
	static	MObject		IVoutMesh;
	

	STE_DEFINE_STATIC_PARAMETERS(IV);



	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		IVid;

	MIntArray			validIndices;		//!<wird von factory zuwueckgegeben - IDs die ausgewhlt werden sollen


	MCallbackId			eID;				//!<callbackID fuer selectionCallback



	IVfty				fIVfty;

	bool				scriptJobInitated;	//!<flag: Solange er true ist, wird kein neuer sribtJob erstellt

private:
	bool meshDirty;
	
	int	lastCount;		//!<hlt den letzten count und verhindert, 
						//!<dass evaluiert wird mit dem seben countWert

	int lastSpin;
	
};



}

#endif
