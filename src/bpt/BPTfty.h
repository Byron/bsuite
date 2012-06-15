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

#ifndef _BPTfty
#define _BPTfty
//
// Copyright (C) 2002 Alias | Wavefront 
// 
// File: BPTfty.h
//
// Node Factory: BPTfty
//
// Author: Sebastian Thiel
//
// Overview:
//
//		The BPTfty factory implements all the algorhithms necessary to create a polySplit Node with
//		the right parameters or to find an edgePath or an edgeLoop, the SelectionBoundary respectively
//
//			1) A polygonal mesh
//			2) A component List (??)
//
//		The algorithms work as follows:
//
//
//
//
#include "meshCreator.h"
#include "edgeMeshCreator.h"
#include "polyModifierFty.h"
#include "face.h"
#include "edge.h"
#include "selEdge.h"
#include "nSelEdge.h"
#include "BPT_Helpers.h"
// General Includes
//
#include <maya/MObject.h>
#include <maya/MFloatArray.h>
#include <maya/MString.h>

#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MColorArray.h>
#include <maya/MColor.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimUtil.h>
#include <maya/MAngle.h>

#include <list>

using std::list;

#include <maya/MFloatArray.h>

#include "simpleLinkedList.h"

namespace BPT
{

/** diese Klasse dient als FunctionsKapsel und arbeitet auf meshes ums sie zu splitten oder die zu chamfern
	
*/

class BPTfty : public polyModifierFty
{

public:
				BPTfty();
	virtual		~BPTfty();


//	*****************************
//	**ACTION METHODS **
//	*****************************

	MStatus		doIt();//int slideInt);
	
	void		doSlide(bool tweakEnabled);
	
	void		doSlideSMC(bool tweakEnabled);
	


//	*****************************
//	**ZUGRIFFS METHODEN	 ->SET **
//	*****************************

//	Parameter werden nicht als Referenz übergeben, weil diese MObjects nicht unbedingt
//	gültig sein müssen. Deshalb werden sie kopiert (denke ich)
	void		setMesh( MObject mesh );
	
	void		setActionMode(int aMode);
	void		setSlide(double );
	void		setNormal(double);

	void		setVertIDs(const MIntArray);
	void		setEdgeIDs(const MIntArray);
	void		setPolyIDs(const MIntArray);

	void		setOptions(const MIntArray);
	void		setRelative(int value);
	void		setNormalRelative(int value);
	void		setSide(int value);

	void		setMeshPath(MDagPath& inMeshPath){meshPath = inMeshPath;};
	void		setThisMObject(MObject& obj){thisNode = obj;};


	void		convertInputToVtxIndices( MIntArray& outVtxIDs ){ getAllVertexIndices(outVtxIDs); };
//	*****************************
//	**ZUGRIFFS METHODEN	 ->GET **
//	*****************************
	int						getSlideLimited()	{return options[4];};

	MObject					getMesh();
	
	MIntArray&				getSlideIndices()	{return *slideIndices;};
	
	MIntArray&				getMaySlide()		{return *maySlide;};
	
	

private:
	void	doUVSlide();	//wird von doSLide aufgerufen
	void	doUVSlideSMC();	//wird von doSlideSMC aufgerufene
	

private:

//	************************
//	**BEVEL METHODEN	  **
//	************************

	MDoubleArray	createGerade(int verts[],MItMeshVertex& vertIter);

	MDoubleArray	createPlane(MDoubleArray&	gerade, 
								int				face, 
								MFnMesh&		fnMesh,
								MItMeshPolygon& polyIter,
								MVector&		normal);



	void			initEdges(	MIntArray&				allEdges,
								MIntArray&				edgeIDs,
								simpleLinkedList<edge>& edgePtrs,
								bool					createOrigVtxLOT = false);

	bool			isDirConform(	const int index0, 
									const int index1, 
									const MIntArray& faceVerts	);


	void			createNonWingedGeomtry(	edge*			thisEdge,
											int				verts[],
											MIntArray&		faces,
											MFnMesh&		fnMesh, 
											MItMeshPolygon& polyIter, 
											MItMeshVertex&	vertIter);



	void			createNSelGerade(	edge*			thisEdge,
										int				verts[], 
										MItMeshVertex&	vertIter); //nur für nSelEdges
	
	
	int				getUID(		int face, 
								MIntArray& nFaces,
								MItMeshPolygon& polyIter,
								int origID,
								int nID);
	

//****************************************************************************************************************************************
//****************************************************************************************************************************************


//	************************
//	**SMART MOVE COMPONENT**
//	************************

	int			SMC_getIndex(int vtxID, MIntArray& linedVtx);

	void		SMC_createNormal(	MIntArray&		currentFaces,
									MIntArray&		directionFaces, 
									MVector&		normal,
									double&			scale, 
									MItMeshPolygon& polyIter);

	int 		SMC_getDirection(	MIntArray&		currentFaces, 
									MPoint&			startPos, 
									MIntArray*		connections, 
									MItMeshVertex&	vertIter);

	int			SMC_getTypeTwoControlDirection(	int				vtxID,
												int				nextVtxLocID,
												int&			faceID,
												MPoint&			startPoint, 
												MIntArray&		selectedVtx, 
												MItMeshVertex&	vertIter, 
												MItMeshPolygon& polyIter, 
												bool			getStartPoint = false);

	MPoint		SMC_getTypTwoEndDirection(	int				vtxID, 
											int				startFaceID,
											int&			endFaceID, 
											MIntArray*		connections, 
											MItMeshPolygon& polyIter, 
											MItMeshVertex&	vertIter,
											MIntArray*		endVtxIDs = 0);

	void		SMC_deleteLastXSlideValues(int x, int offset = 0);
	
	void		SMC_createUVSlide(	int				commonVtx, 
									int				startVtx, 
									int				endVtx, 
									MItMeshVertex&	vertIter, 
									MItMeshPolygon& polyIter);

	void		SMC_createControlTwoUVSlide(int commonVtx,
											int directionalVtx, 
											MIntArray endVerts,
											MItMeshVertex& vertIter, 
											MItMeshPolygon& polyIter, 
											bool isFlipped);

	double		calculateArc(	int				commonVtxID, 
								int				firstEnd, 
								int				secondEnd, 
								MItMeshVertex&	vertIter);

	void		processType3(	int				vtxID, 
								MIntArray&		connections, 
								MItMeshVertex&	vertIter, 
								MItMeshPolygon& polyIter);

	void		processType4Vertex(	int				vtxID,
									MIntArray&		connections,
									MIntArray&		selVtx,
									MItMeshVertex&	vertIter, 
									MItMeshPolygon& polyIter);

	void		processLinedVtx(MIntArray						startIDs,
								MIntArray&						linedVtx,
								MIntArray&						isControlTwo,
								simpleLinkedList<MIntArray>&	connectionsList,
								MIntArray&						countArray,
								MIntArray&						isFlipped,
								MIntArray&						edgeRemovals,
								MItMeshVertex&					vertIter,
								MItMeshPolygon&					polyIter);

	int			SMC_processLine(int								startVtx,
								int								lastVtx, 
								int								direction,
								int&							counter,
								bool							isFlipped,
								MIntArray&						vtxRemovals, 
								MIntArray&						linedVtx,
								MIntArray&						isControlTwo,
								MIntArray&						isFlippedArray,
								MIntArray&						countArray, 
								simpleLinkedList<MIntArray>&	connectionsList, 
								MItMeshVertex&					vertIter, 
								MItMeshPolygon&					polyIter);

	void		processUncontrolledLines(	MIntArray&						linedVtx, 
											simpleLinkedList<MIntArray>&	connectionsList,
											MIntArray&						countArray,
											MItMeshVertex&					vertIter,
											MItMeshPolygon&					polyIter);

	void		getAllVertexIndices(MIntArray& allIndices);	//verwendet von SMC

	void		addNormalSlide(	int inVtx,
								MItMeshVertex& vertIter, 
								MItMeshPolygon& polyIter);

	void		addSlideAndNormal(	int				inVtx,
									MPointArray&	startEnd,
									MVectorArray&	directions,
									MVectorArray&	normals, 
									MDoubleArray&	normalScaleFactors);

	bool		getVtxConnectInfo(	int				vtxID, 
									MIntArray&		connections, 
									unsigned int&	connectCount, 
									MIntArray&		selVtx, 
									MItMeshVertex&	vertIter, 
									MItMeshEdge&	edgeIter);



//****************************************************************************************************************************************
//****************************************************************************************************************************************

	

//	************************
//	**SLIDING			  **
//	************************

	void		createAbsoluteVectors(	MFloatVectorArray&	slideDirections,
										MFloatArray&		slideScale, 
										bool				isSMC = false, 
										MIntArray*			IDs = 0);

	void		createAbsoluteVectors(MVectorArray& slideDirections,MDoubleArray& slideScale);


//---------	************************
//---------	**SMC SLIDING		  **
//---------	************************

	double				clampX(double value);
	double				clampY(double value);

	float				clampX(float value);
	float				clampY(float value);

	double				clampNormalX(double value);
	double				clampNormalY(double value);

	void				initUVSlideArrays();
	void				initSlideArrays();


//****************************************************************************************************************************************
//****************************************************************************************************************************************

//	************************
//	**ALLG. HELFER		  **
//	************************

	void		switchComponentModeIfNeeded(bool mode = false,MSelectionMask::SelectionType type = MSelectionMask::kSelectMeshEdges);
	

	void		vertGetConnectedEdgeIndices(MIntArray& vertIndices,MIntArray& edgeIndices);
	void		faceGetConnectedEdgeIndices(MIntArray& faceIndices,MIntArray& edgeIndices);
	void		faceGetContainedEdgeIndices(MIntArray& faceIndices, MIntArray& edgeIndices);
	
	void		faceGetContainedEdgeIndices(MIntArray& faceIndices,
											MIntArray& edgeIndices,
											MObject& mesh);	//für die externe BoundaryMethode

	int			getContainedFaces(MIntArray faceStorage,MIntArray& containedFaces);
	void		invertSelection(MIntArray& lhs, int maxElement, MIntArray& );
	
	void		convertAllToFaces(	MIntArray& polyIDs,
									MIntArray& vertIDs,
									MIntArray& edgeIDs	);

	void		removeOverlappingFaces(MIntArray& faceIDs, MIntArray& vtxEdges);


	void		getAllEdgeIndices(MIntArray& );
	void		getAllFaceIndices(MIntArray& edgeIDs,MIntArray& faceIDs);


	

	void		selectComponents(	MIntArray& edgeIndices,
									MString mode, 
									MString selectMode = "replace");

	void		findEdgeNachbarMatrixIndices(	edge* origPtr,
												edge* localPtr,
												int& index1,
												int& index2);


	void		growVtxSelection(	MIntArray& vtxArray,
									MItMeshVertex& vertIter, 
									MItMeshPolygon& polyIter, 
									bool doSelect = false);

	void		schrinkVtxSelection(MIntArray& vtxArray, 
									MItMeshVertex& vertIter,
									MItMeshPolygon& polyIter, 
									bool doSelect = false);


	void		modifySelectionList();


//---------	************************
//---------	**FACE HELFER		  **
//---------	************************


	int					findID(MIntArray& matchVtx,MIntArray& faceVerts);
	
	void				findCornerVtx(	MIntArray& corners,
										MIntArray& faceVerts,
										MItMeshVertex& vertIter,
										int onBound);
	
	void				insertNachbarOffsets(MIntArray& nachbarFaces);

	void				appendNachbarIDFlags(	MIntArray&		faceVtx,
												MItMeshPolygon& polyIter,
												MIntArray&		vtxMatch,
												MIntArray&		nachbarFaces);

	void				insertNachbarInnerFlags(MIntArray& faceMatch,
												MIntArray& faceMatchVtxMatchCount,
												MIntArray& nachbarFaces);

	void				cycleVtxMatch(MIntArray& vtxMatch,MIntArray& faceVerts);
	
	MIntArray			findOuterEdges(MIntArray&, MIntArray&);



//****************************************************************************************************************************************
//****************************************************************************************************************************************





//	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	++DATENELEMENTE++
//	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Mesh Node - Note: We only make use of this MObject during a single call of
	//					 the splitUV plugin. It is never maintained and used between
	//					 calls to the plugin as the MObject handle could be invalidated
	//					 between calls to the plugin.
	//
	MDagPath	meshPath;	//!< wird benötigt für selectMethoden, also für alles, was KEINE history erzeugt und wird am anfang auf jeden fall gesetzt 
	MObject		fMesh;
	MObject		newMeshData;

	int			actionMode;	//!< welche Aktion wird ausgeführt?



//SLIDE ELEMENTE
//---------------------------------
	int				whichSide;
	double			slide;
	double			normal;
	MDoubleArray*	normalScale;		//!< scaleFactor für absolute normalen 
	MDoubleArray*	slideScale;			//!< ScaleFactor für absoluten slide
	MVectorArray*	slideDirections;	//!< slideRichtungen
	MVectorArray*	slideNormals;		//!< slideNormalRichtungen
	MPointArray*	slideStartPoints;
	MPointArray*	slideEndPoints;
	MIntArray*		slideIndices;
	MIntArray*		maySlide;
	

	//hier keine Ptr verwenden - MArrays localisieren ihren Daten intern sowieso im heap
	MVectorArray	slideDirections2;	//!< ist die 2.slideDirection, die für SMC notwendig sein kann (bei edgeLines)
	MDoubleArray	slideScale2;
	MDoubleArray	normalScale2;
	MVectorArray	slideNormals2;		//!< ist die 2.normalDirection, die für die SMC notwendig sein kann (bei edgeLines)
	MIntArray		hasSecondDirection;	//!< bestimmt, ob slide ne 2. Direction hat oder nicht;



//UVSLIDE ARRAYS
//---------------------------------
	list<MFloatVectorArray>*		UVSlideStart;	//!< als Points speichern statt indices
	list<MFloatVectorArray>*		UVSlideEnd;
	list<MIntArray>*				UVSlideIndices;
	list<MFloatVectorArray>*		UVSlideDirections;	//!< zKomponente ist 0
	list<MFloatArray>*				UVSlideScale;		//!< werden initialisiert von extract mesh



//UV DATEN ARRAYS (zur weitergabe an die BPTNode, damit das irgendwie klappt mit der Übertragung der UVs aufs neue Mesh)
//---------------------------------
/*
	list<MFloatArray>*			Us;
	list<MFloatArray>*			Vs;
	list<MIntArray>*			UVCounts;
	list<MIntArray>*			UVperVtx;

*/
	MStringArray				UVSetNames;			//!< strings mit UVSetNamen, wird von SMC benötigt




	
	double			maxStandardScale;

	MObject			thisNode;




//SELECTION (DIE BEARBEITET WIRD)
//---------------------------------


	MIntArray	vertIDs;
	MIntArray	edgeIDs;
	MIntArray	polyIDs;

	MIntArray	options;			//!< 0 = relative, 1 = connectallEnds, 2 = maxEdgeCount, 3 = whichSide - 4slideNotLimited
	MSelectionList	redoSelection;
	int	isRelative,isNormalRelative;	//!< relative ist für normal und slide getrennt steuerbar;



//HELFER
//---------------------------------

	meshCreator*		ftyCreator;
	BPT_Helpers			helper;

	
};

}

#endif
