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

#ifndef _ByronsPolyTools
#define _ByronsPolyTools

#include "polyModifierCmd.h"
#include "BPTfty_NH.h"

// Function Sets
//
#include <maya/MFnComponentListData.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MSelectionList.h>
#include <maya/MIntArray.h>

#include <maya/M3dView.h>







namespace BPT
{

/** Diese Klasse wird direkt von maya gerufen und bereitet alles vor, um die Nodes erstellen zu knnen, welche fuer 
	operation bentigt werden
	
*/

class ByronsPolyTools : public polyModifierCmd
{

public:
		
	ByronsPolyTools();
	virtual						~ByronsPolyTools();

	static		void*			creator();
	static		MSyntax			newSyntax();


	virtual bool			isUndoable() const;

	virtual MStatus			doIt( const MArgList& );
	virtual MStatus			undoIt();
	virtual MStatus			redoIt();

	


	/////////////////////////////
	// polyModifierCmd Methods //
	/////////////////////////////

	virtual MStatus			initModifierNode( MObject modifierNode );
	virtual	MStatus			directModifier( MObject mesh );
	

	private:
	/////////////////////
	// Private Methods //
	/////////////////////

	//Noch Methoden hinzuzufügen

	//schreiben entsprechende IntArrays
				MStatus			setActionMode();
				void			getPolyComponents(MSelectionList &sList);
				void			getVertComponents(MSelectionList &sList);
				void			getEdgeComponents(MSelectionList &sList);

				void			convertToEdges(MSelectionList& sList, bool doNotConvertVertices);	//!< wandelt die verts und Faces in contained edges um
				

				virtual void	getModifierUINodeName(MString& newName);	//!< gibt passenden namen für die Node zurück, die gerade erstellt wurde
	//////////////////
	// Private Data //
	//////////////////

	
	//
	// Note: The MObject, fComponentList, is only ever accessed on a single call to the plugin.
	//		 It is never accessed between calls and is stored on the class for access in the
	//		 overriden initModifierNode() method.
	//
//	++++++++
//	+ARRAYS+
//	++++++++
				
MIntArray	vertIDs;
MIntArray	edgeIDs;
MIntArray	polyIDs;

	//undoInfo
	MSelectionMask	undoMask;

	MObject						components;

	//Flags für BPT
	//----------------
	bool	smartSplitFlagSet, 
			edgeLoopFlagSet, 
			edgeRingFlagSet, 
			boundaryFlagSet, 
			chamferFlagSet, 
			solidChamferFlagSet,
			smcFlagSet,
			growFlagSet,
			schrinkFlagSet;

	int slideIsRelative;
	int normalIsRelative;

	int CMDactionMode;

	///initialSettings
	bool avt, avm;


	
	//Flags für insert VTX
	//----------------

	unsigned int initialCount;

	//-----------------
	
	unsigned int operationMode;	//!< gibt an, welche node erstellt werden soll, und welche Flags gelesen werden
	
	double directSlide; //!< SlideWert, wenn er als Flag übergeben wird und keine Node erstellt wird

	MSelectionList	origList;

	MDagPath meshDagPath;

	MIntArray	options;


	
	MObject bptNode;	//!< beinhaltet den Namen der neuen BPTNode (für scripter zur Steuerung der node)
	
	// ByronsPolyTools Factory NO History - erledigt alle selection aufgaben, so dass der Command wirklich nur das hat was 
	//er wirklich benoetigt

	BPTfty_NH				fBPTfty;
};

}

#endif
