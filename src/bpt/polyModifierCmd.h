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

#ifndef _polyModifierCmd
#define _polyModifierCmd

// General Includes
//
#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MPlug.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MPxCommand.h>
#include <MACROS.h>

// Proxies
//


class polyModifierCmd : public MPxCommand
{
public:

                        	polyModifierCmd();
	virtual					~polyModifierCmd();

// Restrict access to derived classes only
//
protected:

	////////////////////////////////////
	// polyModifierCmd Initialization //
	////////////////////////////////////
	
	// Target polyMesh to modify
	//
	void							setMeshNode( MDagPath mesh );
	MDagPath						getMeshNode() const;

	// Modifier Node Type
	//
	void							setModifierNodeType( MTypeId type );
	void							setModifierNodeName( MString name );
	MTypeId							getModifierNodeType() const;
	MString							getModifierNodeName() const;


	virtual void					getModifierUINodeName(MString& newName);	//wird von abgeleitetem Cmd neu definiert

	///////////////////////////////
	// polyModifierCmd Execution //
	///////////////////////////////

	// initModifierNode - Derived classes should override this method if
	//					  they wish to initialize input attributes on the 
	//					  modifierNode
	//
		virtual	MStatus					initModifierNode( MObject modifierNode );


	/**	see detaills
	directModifier - Derived classes should override this method to provide
						direct modifications on the meshNode in the case where
						no history exists and construction history is turned off.
						(ie. no DG operations desired)
	
						This method is called only if history ds not exist and
						history is turned off. At this point, a handle to the
						meshNode is passed in so a derived class may directly
						modify the mesh.
	*/
		virtual	MStatus					directModifier( MObject mesh );


			MStatus							doModifyPoly();
			MStatus							redoModifyPoly();
			MStatus							undoModifyPoly();

protected:
	void		setCreateAnimCurves(const bool value){createAnimCurves = value;};	

// -------------------------
//	PRIVATE METHODEN
private:
// -------------------------



private:

	//////////////////////////////////////////////
	// polyModifierCmd Internal Processing Data //
	//////////////////////////////////////////////

	/** see detaills

	 This structure is used to maintain the data vital to the modifyPoly method.
	 It is necessary to simplify parameter passing between the methods used inside
	 modifyPoly (specifically inside connectNodes()). The diagram below dictates
	 the naming style used:
	@par
	 NOTE: modifierNode is intentionally left out of this structure since it
			 is given protected access to derived classes.
	@par	
	 Before:
	
	@par
	 (upstreamNode) *src -> dest* (meshNode)
	
	@par
	 After:
	
	@par
	 (upstreamNode) *src -> dest* (modifierNode) *src -> dest* (meshNode)
	*/
	
	struct modifyPolyData
	{
		MObject	meshNodeTransform;
		MObject	meshNodeShape;
		MPlug	meshNodeDestPlug;
		MObject	meshNodeDestAttr;

		MObject	upstreamNodeTransform;
		MObject	upstreamNodeShape;
		MPlug	upstreamNodeSrcPlug;
		MObject	upstreamNodeSrcAttr;

		MObject	modifierNodeSrcAttr;
		MObject	modifierNodeDestAttr;

		MObject	tweakNode;
		MObject tweakNodeSrcAttr;
		MObject tweakNodeDestAttr;
	};

	//////////////////////////////////////
	// polyModifierCmd Internal Methods //
	//////////////////////////////////////

	bool					isCommandDataValid();
	void					collectNodeState();
	
	void					deleteTweaks();

	// Modifier node methods
	//
	MStatus					createModifierNode( MObject& modifierNode );

	// Node processing methods (need to be executed in this order)
	//
	MStatus					processMeshNode( modifyPolyData& data );
	MStatus					processUpstreamNode( modifyPolyData& data );
	MStatus					processModifierNode( MObject modifierNode,
												 modifyPolyData& data );
	MStatus					processTweaks( modifyPolyData& data );

	// Node connection method
	//
	MStatus					connectNodes( MObject modifierNode );

	// Mesh caching methods - Only used in the directModifier case
	//
	MStatus					cacheMeshData();
	MStatus					cacheMeshTweaks();

	// Undo methods
	//
	MStatus					undoCachedMesh();
	MStatus					undoTweakProcessing();
	MStatus					undoDirectModifier();

	/////////////////////////////////////
	// polyModifierCmd Utility Methods //
	/////////////////////////////////////

	MStatus					getFloat3PlugValue( MPlug plug, MFloatVector& value );
	MStatus					getFloat3asMObject( MFloatVector value, MObject& object );

	//////////////////////////
	// polyModifierCmd Data //
	//////////////////////////

	// polyMesh
	//
	bool				fDagPathInitialized;
	MDagPath			fDagPath;
	MDagPath			fDuplicateDagPath;

	// Modifier Node Type
	//
	bool				fModifierNodeTypeInitialized;
	bool				fModifierNodeNameInitialized;
	MTypeId				fModifierNodeType;
	MString				fModifierNodeName;

	// Node State Information
	//
	bool				fHasHistory;
	bool				fHasTweaks;
	bool				fHasRecordHistory;

	// Cached Tweak Data (for undo)
	//
	MIntArray			fTweakIndexArray;
	MFloatVectorArray	fTweakVectorArray;

	// Cached Mesh Data (for undo in the 'No History'/'History turned off' case)
	//
	MObject				fMeshData;

	
	MDGModifier			createSlideAnim;
	
	MDagModifier		createDuplicate;
	MDagModifier		reparentDuplicate;
	MDGModifier			deleteDuplicate;

	bool				createAnimCurves;	//!< wenn false(Im fall von insertVtx), dann keine AnimCurves erstellen, weil polyModifier dann base von ganz ander node ist

	MDagPath			myMeshPath;
	MObject				myModifierNode;		//!< Hlt die ModifierNode selbst


	/**DG and DAG Modifier (see detaills)
	
		  - We need both DAG and DG modifiers since the MDagModifier::createNode()
			method is overridden and specific to DAG nodes. So to keep
			the operations consistent we will only use the fDagModifier
			when dealing with the DAG.
	@par
		  - There is a limitation between the reparentNode() and deleteNode()
			methods on the MDagModifier. The deleteNode() method ds some
			preparation work before it enqueues itself in the MDagModifier list
			of operations, namely, it looks at it's parents and children and
			deletes them as well if they are the only parent/child of the node
			scheduled to be deleted.
	@par
			This conflicts with our call to MDagModifier::reparentNode(),
			since we want to reparent the shape of a duplicated node under
			another node and then delete the transform of that node. Now you 
			can see that since the reparentNode() dsn't execute until after
			the MDagModifier::doIt() call, the scheduled deleteNode() call
			still sees the child and marks it for delete. The subsequent
			doIt() call reparents the shape and then deletes both it and the
			transform.
	@par
			To avoid this conflict, we separate the calls individually and
			perform the reparenting (by calling a doIt()) before the deleteNode()
			method is enqueued on the modifier.
	*/

	MDagModifier		fDGModifier;	//!< ist nun DagModifier, da er ne visNode erstellen knnen muss und parenten unter die Transform
	MDagModifier		fDagModifier;

};

//
// Inlines
//

// polyMesh
//
inline void polyModifierCmd::setMeshNode( MDagPath mesh )
{
	fDagPath = mesh;
	fDagPathInitialized = true;
}

inline MDagPath polyModifierCmd::getMeshNode() const
{
	return fDagPath;
}

// Modifier Node Type
//
inline void polyModifierCmd::setModifierNodeType( MTypeId type )
{
	fModifierNodeType = type;
	fModifierNodeTypeInitialized = true;
}

inline void polyModifierCmd::setModifierNodeName( MString name )
{
	fModifierNodeName = name;
	fModifierNodeNameInitialized = true;
}

inline MTypeId polyModifierCmd::getModifierNodeType() const
{
	return fModifierNodeType;
}

inline MString polyModifierCmd::getModifierNodeName() const
{
	return fModifierNodeName;
}

#endif
