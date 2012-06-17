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

// edge.h: interface for the edge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDGE_H__E39F61B5_DD63_4B72_95AA_019563424641__INCLUDED_)
#define AFX_EDGE_H__E39F61B5_DD63_4B72_95AA_019563424641__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <maya/MVectorArray.h>
#include <maya/MIntArray.h>

#include "edgeFaceData.h"
#include "endFaceData.h"
#include "nSelEdgeData.h"
#include "edgeMeshCreator.h"
#include "BPT_BA.h"
#include <math.h>


namespace BPT
{

/** Diese Klasse stellt eine MeshEdge dar und wird verwendet, um Chamfer und solidchamfer zu berechnen
	
*/

class edge  
{
public:
	
	edge(int inID);
	//edge(MVectorArray inNormals, MIntArray inWingedVtx,MIntArray inFaceIDs,int inId);
	edge(MIntArray inFaceIDs,int inId);
	virtual ~edge();


public:
//----------------------------------
///ZUGRIFFSMETHODEN////////////
//----------------------------------

	void			setOnlyOneSelNachbar(bool* values){for(UINT i = 0; i < 4; i++) onlyOneNSelNachbar[i] = values[i];};
	void			setVtxIDs(int inVtxIDs[]);
	void			setNachbarn(edge* inNachbarn00,edge* inNachbarn01,edge* inNachbarn10,edge* inNachbarn11);
	void			setNachbar(int seite,int richtung, edge* nachbarPtr);
	void			setFaceIDs(MIntArray& inFaceIDs);



	void			setIsFlipped(bool inIsFlipped[]){isFlipped[0] = inIsFlipped[0]; isFlipped[1] = inIsFlipped[1]; };



	bool			isClean(){return clean;};

	MIntArray		getVtx() const;
	MIntArray		getFaces() const;
	int				getID() const{return id;};

	void			setNormal(MVector inNormal){normals.append(inNormal);};
	MVectorArray*	getNormals(){return &normals;};	



	void			setNSelNachbarn(edge* ptr1,edge* ptr2,edge* ptr3,edge* ptr4){	nSelNachbarn[0][0] = ptr1;
																					nSelNachbarn[0][1] = ptr2;
																					nSelNachbarn[1][0] = ptr3;
																					nSelNachbarn[1][1] = ptr4;	};

	void			setThisNachbarZero(edge* thisNachbar);

	void			getFaceData(edgeFaceData*& outData0,edgeFaceData*& outData1){ outData0 = faceData[0];outData1 = faceData[1];};

	void			setThisFaceData(int faceID, edgeFaceData* inFaceData);

	void			initFaceData(int extraDirection = -1);				//!< initialisiert eine faceData einheit mit allen notwendigen Daten																								
																		


	//HELFER

	int				C_fixNewVtxUV(int origVtx,int refVtx,  edgeFaceData* faceData);

	

	int				C_checkForEdgeVtx(	const edge* requester,		
										int UID,					
										bool isSeparated,			
										const MPoint& startPos,		
										const MPoint& endPos	);

	bool			borderThere(int requesterID,edge* lastEdge );


protected:
//*********************************
//HELFER				 **********
//*********************************

	bool			doUVsDiffer(int checkVtx,
								const edgeFaceData* faceData1,
								const edgeFaceData* faceData2, 
								MIntArray* outOne = NULL, 
								MIntArray* outTwo = NULL);

	void					findMatrixIndices(edge* origPtr,edge* localPtr,int& seite,int& richtung);
	
	edge*					findOppositeRichtung(edge* lastPtr);

	edge*					findOppositeRichtung(edge* lastPtr,int& seite,int& richtung);


	void					insertNewEdgeVtx(	int newVtx,
												bool edgeVtxExisted, 
												edge* insertIntoEdge );	//!< diese prozedur fuegt neue edgevertizes in faceDaten ein

	void					addVtxChange( int newVtx, edge* otherSelEdge);	//!< diese methode kuemmert sich um alles, was vtxChange betrifft - Vtx, normalIDs, UVs
																										

	void					getUVPos(	MPoint newPosition, 
										edgeFaceData* thisFace, 
										MFloatArray& newUVPos, 
										bool regatherData = true);



public:
//----------------------------------
///STATISCHE ELEMENTE
//----------------------------------
//werden bentigt, im das Mesh spter wieder korrekt aufzubauen, um Dataen zwischen zu speichern,die von jeder Edge aus erreichbar sind

	static edgeMeshCreator*		creator;
	static double				maxLength ;	//ist die kleinste unSelEdge und sorgt dafuer, dass die Faces nicht uebers ziel hinausschiessen
	
	static std::list<edgeFaceData*> MAINFaceDataPtrs;	//!< enthlt Ptr zu allen faceDataEinheiten - wird verwendet, um die origFaces anzupassen und um die einheiten zu lschen
	static std::list<endFaceData*>	endFacePtrs;		//!< hlt die Ptr zu den EndFaces, welche ganz zum schluss evaluiert/erstellt werden
	static std::list<nSelEdgeData*> nSelEdgeDataArray;	//!< hlt infos ueber die zu ndernden Faces, welche an nSelEdges angrenzen. Diese werden spter rekonstruiert

	static BPT_BA					origVtxDoneBA;		//!< ist true, wenn vtx gewhlt ist (== selVtx);WIRD VON CHAMFER VERWENDET UND VON SOLIDCHAMFER, allerdings aus erschiedenen Gruenden




public:
//*********************************
//ABSTRAKTE METHODEN***************
//*********************************
//werden von Sel/nSel Edge definiert

	virtual	void			engageBevel(int connectEnds, int triangulateEnds, bool isSC) = 0;


	virtual int				getValidRichtung(int uniqueID) = 0;
	virtual	void			setUIDs(MIntArray& IDs) = 0; 

	virtual edge*			findNextSelEdge(edge* lastEdge) = 0;

	virtual	const int*		getUIDs() const = 0;			

	virtual	bool			isSelected() = 0; //!< gibt den Typ des faces zurueck: false = unSelEdge, true = selEdge

	virtual MDoubleArray	getGerade(bool displace,const edge* requester,bool useUId = false, int uniqueID = -1) = 0; //!< diese proc ist nur fuer selEdges definiert

	virtual	void			setNewVerts(const edge* requester, MIntArray& newVerts,bool useUID = false, int uniqueID = 0) = 0;	//!< defefniert nur fuer selEdge

	virtual void			getValidRichtung(const edge* requester, int& seite, int& richtung, int uniqueID = -1) = 0;

	virtual int				getNewGeradenVtx(const edge* requester) = 0;	//!< definiert nur fuer selEdge	

	virtual MIntArray		getNewVtx() const = 0;



//-------------------
//DEBUG METHODEN
//-------------------
#ifdef DEBUG

	void			printArray(MIntArray& array, MString message);
	void			printAllData();

#endif


//--------------------
//PRIVATE METHODEN
	


protected:

	
	

	////////////////////////////
	///PRIVATE DATENELEMENTE////
	////////////////////////////
	static			ULONG			side,dir;			/*!< wird fuer iteration verwendet und verhindert, dass man diese Parameter stndig mitgeben muss
														//vars sind statisch, da sie immer nur von einer Edge zu einem Zeitpunkt verwendet werden
														//MERKE: MAN DARF JETZT NICHTS MEHR REKURSIV LOESEN!!! ->GEHT KLAR ;) */

	int				vtxIDs[2];				//!< hlt die VtxIDs der beiden enden

	
	int				edgeVtx[2];				//!< enthlt die neuenVtx auf der origEdge pro seite


	edge*			nachbarn[2][2];			//!< hlt ptr zu nachbarEdges pro seite pro richtung, erster index ist fuer Seite, zweiter index ist fuer Richtung

	edge*			nSelNachbarn[2][2];		//!< hlt Ptr zu nSelEdges zwecks intersection wenn nurEinNachbar


	MIntArray		faceIDs;				//!< hlt IDs der Faces links und rechts von Edge, sync zu richtung der edgeNachbarn
	
	
	bool			isFlipped[2];			//!< sync zu Richtung, true, wenn edgeVtx GEGEN den fluss des faces dieser Richtung, false wenn die Vtx in Richtung der FaceVtx
											//!< wird nicht im konstruktor initialisert - die Werte werden entsprechend gesetzt ueber zugriffsmethode

	edgeFaceData*	faceData[2];			//!< hlt datenFace pro richtung



	MVectorArray	normals;				//!< normalen der facees, sync zu faceIDs

	
	int				id;						//!< wird im grund nur fuers debuggung gebraucht;

	//FLAGS
	bool			clean;					//!< wird am ende der engageMethode auf true gesetzt	

	bool			onlyOneNSelNachbar[4];	//!< true (pro seite, pro richtimg), wenn es nur einen nSelNachbarn gibt, welcher dann auf jeden Fall herangezogen werden muss


	bool			C_IChangedUV[2];		//!< True, wenn diese edge die UV vom origVtx verschoben hat
};

}

#endif // !defined(AFX_EDGE_H__E39F61B5_DD63_4B72_95AA_019563424641__INCLUDED_)
