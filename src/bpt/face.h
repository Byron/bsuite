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

// face.h: interface for the face class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FACE_H__48A1D89B_9DAF_451D_B09D_3F2D6FC7D513__INCLUDED_)
#define AFX_FACE_H__48A1D89B_9DAF_451D_B09D_3F2D6FC7D513__INCLUDED_


#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnComponentListData.h>
#include <maya/MIntArray.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshEdge.h>
#include <iostream>
#include <math.h>
#include "dData.h"
#include "meshCreator.h"


namespace BPT
{

/** diese Klasse dient als DatenKapsel und stellt ein face dar, welches mit anderen verbunden sein kann
	
*/

class face  
{
public:
	face(){};
	face(MIntArray inVtx,int inID);
	face(MIntArray inVtx,MIntArray inCorners,MIntArray inNachbarIDs,int inID);
	virtual ~face();


public:
//-------------------------------------
//HAUPT METHODEN	
//-------------------------------------
//
	bool	engage();	//!< startet den eigentlichen splitVorgang, gibt true zurueck, wenn alles geklappt hat


public:
//-------------------------------------
//ZUGRIFFS METHODEN	
//-------------------------------------
//
	MIntArray	getVtx(){return vtx;};

	void		setNachbarIDs(MIntArray& inNachbarn){nachbarIDs = inNachbarn;};
	void		setCorners(MIntArray& inCorners){corners = inCorners;};
	void		setNachbarn(face*	inNachbarn,int index);
	
	bool		isClean();
	int			whichType();								//!< gibt einfach den Typ des Faces zurueck.

	int			getType();									//!< analysiert die NodeDaten und erstellt daraus seine TypeID, gibt TypeID zurueck


	int			dsVtxExist(int myID);						//!< Fragendes Face gibt seine ID und erhlt die ID des vorhanden Vtx zurueck bzw -1
	void		setVtxExists(int myID,int absVtxID);		//!< nachdem Face Vtx erstellt hat, wird das Nachbarface davon in Kentnis gesetzt
	
	const MIntArray	getNachbarUVArray() const {return newUVs;};


public:
//-------------------------------------
//MODIFIKATOR METHODEN	
//-------------------------------------
//
	bool		modifyType3(int& startCornerLocID,int& endCornerLocID);		//!< macht aus typ3 nen typ2normal, wenn an die alone edge auch typ3 grenzt
	void		convertType3ToType2(int startCornerLocID,int endCornerLocID);
	bool		modifyType3EdgeCheck(dData& data);							//!< gibt true zurueck, wenn aloneEdge gemeinsam ist, ansonsten false;




public:
//-------------------------------------
//STATISCHE ELEMENTE
//-------------------------------------
//
	static	meshCreator*	creator;	//!< ist bei allen Faces gleich, wird mit 0 initialisiert





//*********************************
//PRIVATE METHODEN
private:
//*********************************

private:
//-------------------------------------
//MESH MODIFIKATOREN
//-------------------------------------
//erledigen allgemeine Aufgaben
	void		createVtx(	MIntArray& tmpFaceVtx,
							int startLocID, 
							int endLocID,
							int realStartLocID,
							bool useNachbar,
							dData& data
													);

	int			processMultiVtxNachbar(	MIntArray& tmpFaceVtx,
										int vertsZwischenIDs,
										int startLocID,
										bool useNachbar,
										dData& data
														);


//>>>>>>>>>>-------------------------------------
//>>>>>>>>>>>>MESH HELFER
//>>>>>>>>>>-------------------------------------
//konvertieren daten, erstellen Faces usw.

	void		pokeFace();
	void		splitType4();	//!< splittet type 4 faces
	void		splitControlTwo();
	void		splitNormalTwo();
	void		SplitTypeOne();
	void		splitType3();
	void		splitTypeMinusOne();
	
	void		createFacesBetweenBounds(	int origLBound,
											int origRBound,
											int lBound, 
											int rBound, 
											MIntArray& vtxPerCornerStart,
											MIntArray& vtxPerCornerEnd
																			);

	void		createType2Corner3Faces(int origLBound,
										int origRBound,
										int lBound, 
										int rBound, 
										MIntArray& vtxPerCornerStart,
										MIntArray& vtxPerCornerEnd,
										MIntArray& vtxPerCornerLast
																		);

	void		createType2Corner4Faces(int origLBound,
										int origRBound,
										int lBound, 
										int rBound, 
										MIntArray& vtxPerCornerStart,
										MIntArray& vtxPerCorner2,
										MIntArray& vtxPerCorner3,
										MIntArray& vtxPerCornerEnd
																	);

	void		setBounds(MIntArray& vtxPerCorner,
							int newVtxCount,
							int &lBound,
							int &rBound
														);	//!< findet die Bounds eines VtxPerCornerArrays in Reihenfolge
	

	void		processCornerEdges(	MIntArray& processedVtx,
									MIntArray& newVtxCount,
									MIntArray& cornerData);

	MIntArray	createCompleteCornerEdgeArray(	int cornerStartID,
												int l,
												int i,
												MIntArray& processedVtx
																		);

	void		createInnerCornerPolys(	int lBound,
										int rBound,
										int centerVtx,
										MIntArray& vtxPerCorner,
										MIntArray& newVtxOrder
																);



private:
//-------------------------------------
//SLIDING
//-------------------------------------
//alle methoden, die sich um das Slding kuemmern

	bool		gatherSlideData(dData&	data);	//!< gibt true zurueck, wenn slide aktiviert werden kann
	void		flipData(dData& data);			//!< flipt abs VtxIDs
	int			isDirectionStored(dData& data);	//!< check auf gespeicherte Directions, wenn ja, gibts true zurueck, flipped data automatisch, wenn ntig
	void		storeDirection(dData& data,int code);	//!< speichert die uebergebene Daten
	void		createDirections(dData& data);	//!< erzeugt directionData fuer diesen Nachbarn, je nach controlFace


	int			convertToOpposite(dData& data);	//!< schreibt data so um, dass sie auf der gegenueberliegenden Edge liegt, die Direction bleibt erhalten
	dData		convertData(dData& data,int& errCode);		//!< konvertiert Abs VtxIDs zu eigenen Lokalen VtxIDs und gibt data Obj zurueck
	int			requestDirections(dData& data);	//!< gibt Code zurueck, der die Quelle der Direction definiert
	face*		findOppositeNachbar(int myID);	//!< gibt facePtr zu gegenueberliegendem Nachbarn zurueck, wenn vorhanden ansonsten NULL,
	void		findOppositeCorner(int startCornerID,int iterCount,int& newStart, int& newEnd);


private:
//-------------------------------------
//HELFER
//-------------------------------------
//erledigen allgemeine Aufgaben

	void		refillNachbarArray(int start,int end);

	int			findNachbarIDIndex(int inID);	//!< gibt die lokale ArrayID von der gegebenen ID zurueck
	int			findLastNachbarVtx(int firstVtx);	//!< gibt den letzten NachbarFaceVtx zurueck
	int			findNextValidNachbarLocID(int index);	//!< input ist index der uebergeordneten iteration durchs nachbarArray
	
	int			validateThisCornerLocID(int index);
	int			findNextValidCornerLocID(int index);
	int			getCornerEdgeVtxCount(int startCID,int endCID);
	int			getNumCornerEdgeNachbarn(int startVtxID,int cornerLength);
	MIntArray	getCornerEdgeNachbarIDs(MIntArray& nachbarSave,int FirstCornerID,int vtxPerCornerLength);


	int			guessNumTriangles(int type,int vtxCount);


	int			getNachbarEdgeVtxCount(int startID,int endID);
	int			findVtxIndex(int inID);



//-------------------------------------
//DEBUG
//-------------------------------------

#ifdef DEBUG
	void		printData(MString prefix,dData& data);	//!< Debug Methode, druckt gesamtes DatenObjekt aus
	void		printArray(MIntArray& array, MString message);
#endif

	

private:
	//////////////////////
	//DATENELEMENTE
	//////////////////////
	MIntArray	vtx;			//!< hlt alle FaceVtx in Reihenfolge
	MIntArray	corners;
	int			newVtx[20];		//!< neu angelegte Vertizen pro Nachbar
	
	/** neue UVs pro Nacbar, pro UVSet: Codiert:
	
	Xn*numUVSets = lokale ID des neuen Vtx (relativ zu newVtx[20])
	Xn*numUVSets +(1 bis numUVSets) =	neue UVIndices oder -1, wenn face in diesem UVSet keine UVs hatte - fuer jedes UVSet muss aber
	ein Eintrag sein
	
	@par
		centerVtx hat in Slot Xn*numUVSets den Flag -2, da der CenterVtx nicht in newVtx zu finden sein wird
	*/
	MIntArray	newUVs;			


	
	MIntArray	nachbarIDs;		//!< werden erstmal gespeichert, um dann spter die nachbarPointer von der fty zu erhalten
	face*		nachbarn[20];	//!< syncron zu NachbarIDs
	bool		clean;
	bool		isControlTwo;


	int			slideCodes[20];
	bool		isFlippedArray[20];		//!< hier wird gespeichert, ob die Daten relativ zur richtung des faces vertauscht sind

	int			numNachbarn;	//!< speichert die maximale zahl der nachbarn pro corner(ev. unntig, aber vielleicht relevant fuer Type -1)
	unsigned	numCorners;		//!< wird fuer PokeSingleFace benutzt
	int			type;
	int			id;				//!< die eigene FaceID



};

}

#endif // !defined(AFX_FACE_H__48A1D89B_9DAF_451D_B09D_3F2D6FC7D513__INCLUDED_)
