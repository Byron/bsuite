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

#ifndef BPTFTY_NH_INCLUDE
#define BPTFTY_NH_INCLUDE

#include "BPT_Helpers.h"

#include <maya/MIntArray.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MSelectionMask.h>

#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>

#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <maya/MStatus.h>


#include "MACROS.h"



namespace BPT
{

/** Diese Klasse kapselt alles, was keine History erzeugt und direkt vom command aus ausgeführt wird
	
*/

class BPTfty_NH
{
public:
	BPTfty_NH(void);
	~BPTfty_NH(void);

public:	//ACTION METHOD

	MStatus	doIt();
	void	redoFty(){MGlobal::setActiveSelectionList(redoSelection);};
	
	
	void	externalBoundary(MIntArray& partEdges, 
							 MIntArray& polys ,
							 MObject& mesh);	//!< wird von BPT Node genutzt um boundarys zu finden

public:	//ZUGRIFFSMETHODEN

	void	setMesh(const MObject& inMesh){fMesh = inMesh;};
	void	setMeshPath(const MDagPath& inPath){meshPath = inPath;};
	void	setActionMode(const int inMode){mode = inMode;};

    
	void	setVertIDs(const MIntArray& inIDs){vertIDs = inIDs;};
	void	setEdgeIDs(const MIntArray& inIDs){edgeIDs = inIDs;};
	void	setPolyIDs(const MIntArray& inIDs){polyIDs = inIDs;};

	void	setOptions(const MIntArray& inOptions){options = inOptions;};


private: //PRIVATE METHODEN

	//-------------------
	//ALLGEMEINE HELFER
	//-------------------

	void	getAllEdgeIndices(MIntArray& allIndices);

	void	vertGetConnectedEdgeIndices(MIntArray& vertIndices,
										MIntArray& edgeIndices);

	void	faceGetConnectedEdgeIndices(MIntArray& faceIndices,
										MIntArray& edgeIndices);

	void	selectComponents(MIntArray& edgeIndices, 
										MString mode, 
										MString selectMode = "replace");

	void	switchComponentModeIfNeeded(bool mode = false,
										MSelectionMask::SelectionType type = MSelectionMask::kSelectMeshEdges);

	void	convertAllToFaces(	MIntArray& polyIDs,
								MIntArray& vertIDs,
								MIntArray& edgeIDs);

	void	faceGetContainedEdgeIndices(MIntArray& faceIndices, MIntArray& edgeIndices, MObject* mesh = 0);

	void	addToEdgeLoopArray(MIntArray&,
							   MIntArray&,
							   MIntArray&);

	void	invertSelection(MIntArray& lhs,
							int maxElement, 
							MIntArray& inverted);

	//-------------------
	//EDGE LOOP
	//-------------------

	void		findEdgeLoops(MIntArray& allEdges, 
							  MIntArray& edgeLoops);
	
	MIntArray	findOneWayQuadLoop(	int firstEdge,
									MIntArray& allEdges,
									int &edgeCount, 
									int firstVertex, 
									MIntArray& stopEdgeLUT);

	void		findBorderEdges(  int startEdge,
								  MItMeshEdge& edgeIter,
								  MItMeshVertex& vertIter,
								  MIntArray& borderEdges,
								  MIntArray& edgeLoops);

	bool		stopLoopSearch(	MIntArray& loopResult, 
								MIntArray& stopEdges, 
								MIntArray& stopEdgeLUT,
								int currentEdge);

	//-------------------
	//EDGE RING
	//-------------------
	
	void	findEdgeRings(MIntArray& allEdges,
						  MIntArray& edgeRings);

	void	findTwoWayRing(	int startEdge,
							MIntArray& startEdges,
							MIntArray& edgeRings,
							int& edgeCount,
							MIntArray& stopEdgeLUT, 
							int startFace = -1);


	//-------------------
	//Grow/Shrink
	//-------------------
	
	void		growVtxSelection(	MIntArray& vtxArray,
									MItMeshVertex& vertIter, 
									MItMeshPolygon& polyIter, 
									bool doSelect = false);

	void		schrinkVtxSelection(MIntArray& vtxArray, 
									MItMeshVertex& vertIter,
									MItMeshPolygon& polyIter, 
									bool doSelect = false);

	void		growFaceSelection(	MIntArray& faceArray, 
									MItMeshEdge& edgeIter, 
									int numFaces);

	void		shrinkFaceSelection(MIntArray& faceArray,
									MItMeshEdge& edgeIter, 
									int numFaces);

	void		growEdgeSelection(	MIntArray& edgeArray, 
									MItMeshEdge& edgeIter, 
									int numFaces);

	void		shrinkEdgeSelection(MIntArray& edgeArray,
									MItMeshEdge& edgeIter, 
									int numFaces);


private:	//DATENELEMENTE

	MObject		fMesh;	//!< mesh um darauf zuarbeiten
	
	MDagPath	meshPath;	//!< dagPath zum mesh, um die neuen kanten auswählen zu koennen


	int			mode;	//!< welche aktion soll ausgeführt werden?


	MIntArray	vertIDs;
	MIntArray	edgeIDs;
	MIntArray	polyIDs;
	

	MIntArray	options;

	MSelectionList	redoSelection;

	BPT_Helpers	helper;

};

}
#endif

