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

// meshCreator.h: interface for the meshCreator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESHCREATOR_H__847CAC89_2F15_4C0D_8109_3094B92707C6__INCLUDED_)
#define AFX_MESHCREATOR_H__847CAC89_2F15_4C0D_8109_3094B92707C6__INCLUDED_

#include "BPT_Helpers.h"
#include "MACROS.h"

#include <maya/MObject.h>
#include <maya/MFloatArray.h>
#include <maya/MString.h>

#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

#include <maya/MFnMeshData.h>

#include <iostream>

#include <list>
using std::list;


namespace BPT
{

/** Der MeshCreator implememtiert die grundlegende funktionalitt, um meshes verndern zu knnen und wird so  nur 
	von smartSplit genutzt

	Er hlt ausserdem smtliche meshDaten, um auf ihnen efizient arbeiten zu knnen

  @sa
	edgeMeshCreator, IVMeshCreator

*/

class meshCreator  
{
public:
	meshCreator(		MObject&		fMesh,
						double			inSlidem,
						MDagPath&		meshPath
																	);	
	virtual ~meshCreator();


public:
/////////////////////////////////////////////
//	MESH Extraction/Creation
/////////////////////////////////////////////
	//extrahiert wird das Mesh direkt im Konstruktor
	MStatus createMesh(MObject& newMeshData);


public:
/////////////////////////////////////////////
//	ZUGRIFFS METHODEN
/////////////////////////////////////////////
//Daten Zugriff, immer direkt durch Ptr, nie durch Kopie
//

	//*sorgt dafuer, das der initialValue neu definiert wird wird verwendet von der connectEngine
	void	applyCurrentState(){initialVtxCount = vtxPoints->length() - 1;}		

	void	getUVData(	list<MFloatArray>*&			inUs,
						list<MFloatArray>*&			inVs,
						list<MIntArray>*&			inUVCounts,
						list<MIntArray>*&			inUVperVtx,
						MStringArray&				inUVSetNames
																	);
																

	void	getSlideArrays(MDoubleArray*& inNormalScale,
							MDoubleArray*& inSlideScale,
							MVectorArray*& inSlideDirections,
							MVectorArray*& inSlideNormals,
							MPointArray*& inSlideStartPoints,
							MPointArray*& inSlideEndPoints,
							MIntArray*& inSlideInidices, 
							MIntArray*& inMaySlide				);

	void	getUVSlideArrays(	list<MFloatVectorArray>*&		inUVSlideStart,
								list<MFloatVectorArray>*&		inUVSlideEnd,
								list<MIntArray>*&				inUVSlideIndices,
								list<MFloatVectorArray>*&		inUVSlideDirections,
								list<MFloatArray>*&				inUVSlideScale);

	int		getLastFaceID() const {return offsets->length()-1;};
	int		getLastVtxID() const {return vtxPoints->length() -1;};
	int		getInitialVtxCount() const {return initialVtxCount;};

public:
/////////////////////////////////////////////
//	SLIDE METHODEN
/////////////////////////////////////////////
//Mit diesen Methoden werden die SlideDaten aktualisiert
//

	
	
	void	addNormalSlide(int vtxID, int currentFace,int nachbarFace = -1);
	void	addNormalSlide(int vtxID,MVector& normal,double normalScale);
	
	void	addSlide(int vtxID,int startID,int endID,int currentFace,int nachbarFace = -1);
	void	addSlide(int vtxID,MPoint startPos,MPoint endPos, MVector& direction, MVector& normal, double normalScale);
	

	void	addUVSlide(int slideAbsVtx, int startAbsVtxID,int endAbsVtxID, int faceID, MIntArray& newUVs);

public:
/////////////////////////////////////////////
//	MESH MODIFIKATOREN
/////////////////////////////////////////////
//Verndern die Topologie des Meshes
//
	

	int		newVtx(MPoint position);	

	int		newVtxBetweenIDs(	int startID,
								int endID,
								int currentFace,
								bool useSlide, 
								int nachbarFace = -1);
	
	int		createCenterVtx(int faceID);

	void	createPoly(MIntArray& vtxIDs);
	
	void	changePolyVtxIDs(int faceID,const MIntArray& localVtxChange);

	
	//*in resultUVs kommen die UVs, de neu von doFaceONeSplit erstellt wurden
	int		doFaceOneSplit(	int faceID,
							MIntArray& verts,
							MIntArray& newVtxIndices,
							MIntArray& excludeIDs, 
							int& otherNachbar,
							MIntArray& resultVs);	
	
	//*addVtx dient eher der Arbeit an nicht gewhlten Faces, die durch gewhlte beeinflusst werden 
	void	addVtx(	int vtxID,
					int edgeID, 
					int faceID);		

//------------------------
//NORMAL MODIFIKATOREN
//------------------------

	void	createNormal(	int faceID, 
							int newVtxID,
							int startAbsID = -1, 
							int endAbsID = -1 );

	void	changeNormals(	int faceID,
							MIntArray& localVtxChange,  
							bool append = false ,
							bool hasCenter = false );

//------------------------
//UV MODIFIKATOREN
//------------------------

	void	checkForUV(	int thisFaceID, 
						int otherFaceID,
						MIntArray& newUVs,
						const MIntArray& newNachbarUVs,
						int startVtxLocID, 
						int endVtxLocID,
						int newVtx,
						bool useSlide = false) const;

	void	createUV(	int currentFace,
						int newVtxID,
						MIntArray&	newUVArray,
						bool useSlide = false, 
						int startLocID = -1, 
						int endLocID = -1, 
						bool isAbsolute = false);

	void	changeUVIds(	int faceID,
							MIntArray& newUVs,
							MIntArray& localVtxChange,  
							MIntArray& faceVtxIDs, 
							bool append = false);

	

public:
/////////////////////////////////////////////
//	HELFER METHODEN
/////////////////////////////////////////////
//WERDEN VON AUssERHALB BENUTZT
//
	int		getNumFaces() const {return offsets->length();}; 

	//* gibt die aktuellen IDs der Vtx
	void	getFaceVtxIDs(int faceID, MIntArray& array) const;	

	//*gibt offset zurueck, so dass die Procs ihn nicht doppelt holen muessen
	void	getFaceNormalIDs(	int			faceID,			
								MIntArray&	normalIndices) const 
																	{getNormalsIndices(faceID, normalIndices);};

	void	getFaceUVs(int faceID, MIntArray& UVIDs,bool UVRelOffsets[]) const;

	//* holt UVPos aller Sets der UVs, 
	void	getUVPositions(const MIntArray& UVIds, MFloatArray& UVPos) const;	


	//* holt die grsse des Faces - holt die FaceVtxList und uebergibt diese an die andere Methode
	double	getFaceSize(int faceID) const ;						

	//* macht die eigentlice arbeit
	double	getFaceSize(const MPointArray& pointList) const;			

	void	getVtxPositions(const MIntArray& vtxList, MPointArray& pointsResult) const;

	void	getConnectedFaces(UINT vtxID, MIntArray& connectedFaces) const;
	
	void	filterBoundaryVtx(const MIntArray& inVtx, MIntArray& filterVtx);



public:
/////////////////////////////////////////////
//	DATENELEMENTE (ffentlich)
/////////////////////////////////////////////
//WERDEN VON AUssERHALB BENUTZT
//


	unsigned short	numUVSets;

	BPT_Helpers			helper;



protected:

//---------------
//MESH HELFER
//---------------
//Halten die Topologie der MeshDaten korrekt

	int		getValidAbsoluteOffset(int faceID) const;
	void	updateOffsetLUT(int faceID, int offsetCount);

//---------------
//UV HELFER
//---------------

	int		getValidAbsoluteUVOffset(int faceID, int index, std::list<MIntArray>::iterator& iterOffset,
															std::list<MIntArray>::iterator& iterLUT,
															std::list<MIntArray>::iterator& iterLUTValues) const;

	void	updateOffsetUVLUT(int faceID, int offsetCount, int index, std::list<MIntArray>::iterator& iterLUT,
																	  std::list<MIntArray>::iterator& iterLUTValues);


	void	getFaceUVs(	int								faceID,
						MIntArray&						result,
						int								uvSetIndex,
						std::list<MIntArray>::iterator& iterOffset,
						std::list<MIntArray>::iterator& iterLUT,
						std::list<MIntArray>::iterator& iterLUTValues,
						std::list<MIntArray>::iterator& iterCounts,
						std::list<MIntArray>::iterator& iterUVIDs) const;
	
//---------------
//NORMAL HELFER
//---------------

	//* gibt offset zurueck, so dass die Procs ihn nicht doppelt holen muessen
	int		getNormalsIndices(	int			faceID,					
								MIntArray&	normalIndices) const;
	
	int		getValidAbsNormalOffset(int faceID) const;
	void	updateNormalOffsetLUT(int faceID, int offsetCount);

	void	updateNormalArraysWith(const MIntArray& normalIndicesChange, int faceID);

//---------------
//SONSTIGE HELFER
//---------------

	void		cycleVtxMatch(MIntArray& vtxMatch,MIntArray& faceVerts);
	bool		orderNachbarn(MIntArray&	faceIDs,MIntArray&	bounds, MIntArray& verts);

	bool		isEdgeFlipped(int index1, int index2,const MIntArray& faceVertexList);

//*****************
//DATENELEMENE
//*****************

protected:
	
	MItMeshVertex		vertIter;
	MItMeshEdge			edgeIter;
	MItMeshPolygon		polyIter;		//!< zum holen der NormalVectors
	MFnMesh				meshFn;

//----------------
//MESH ARRAYS
//----------------
//Arrays fuer MeshDaten - werden sicherheitshalber im Heap erstellt, um stackoverflows bei grossen meshes zu vermeiden

	MPointArray*		vtxPoints;		//!< VTXIds und deren Position							
	MIntArray*			faceVtxIDs;		//!< Die VtxIDs, so wie sie die Faces definieren		
	MIntArray*			offsets;		//!< relativer offset(zahl der vtx/face) pro faceID	
	MIntArray*			offsetsAbsolute;//!< absoluter offset zum finden des richtigen \
										   	 insertpoints fuer neue vtx/zum ndern der \
											 vtxNumerierung

	MPlugArray			compPlugs;		//!< speichert die Plugs, welche selectionLists enthalten
	MObjectArray		compLists;		//!< speichert ComponentListen, sync zu CompPlugs
	MFnSingleIndexedComponent	compFn;	//!< dient der Bearbeitung der compList Objekte
	

	MIntArray*		offsetsLUT;			//!< beinhaltet index, an dem vtx hinzufgefuegt/genommen wurde
	MIntArray*		offsetsLUTValues;	//!< Beinhaltet den Wert zum entsprechenden Slot in <offsetsALookUp>
	int				minID;				//!< die kleinste ID im LUT, efizienzsteigerung
	int				maxID;				//!< die gsste ID usw.


//----------------
//NORMAL ARRAYS
//----------------



	
	MIntArray					normalCount;	//!< Wenn mindestens eine Normale nicht geteilt ist, dann \
													hlt dieses Array die Zahl der Normalen /bzw. faceVtx \
													ansonsten 0; pro face
	
	MIntArray					normalVertexIndices; //!< hlt Indices (obj relativ) der vertizen, die geteilte normalen haben
													 

	MIntArray					normalAbsOffsets;	//!< hlt verweise direkt ins normalIndices Array (absolut), (im gegensatz dazu \
													ist normalCount relativ)
	
	MIntArray					normalOffsetsLUT;	//!< beinhaltet index, an dem normale hinzufgefuegt/genommen \
													wurde (was einhergeht mit faceVtxChanges)
	
	MIntArray					normalOffsetsLUTValues; //!< beinhaltet den Wert zum entsprechenden slot im OffsetsLUT (Wieviele Normalen wurden hinzugefuegt?)\
														synchron zu offsetsLUT


	MIntArray					normalIndicesBak;	//!< sie werden in den darauf folgenden Appends verwendet

	int							nMinID;			//!< minimale und maximale IDs im Offset LUT - verhindert unsinnige suchen
	int							nMaxID;


//----------------
//UV ARRAYS
//----------------

	//UVs
	list<MFloatArray>*			Us;
	list<MFloatArray>*			Vs;
	
	list<MIntArray>*			UVCounts;
	list<MIntArray>*			UVAbsOffsets;		//!< enthlt die absoluten offsets der UVs zum referenzieren der UVs im
	list<MIntArray>*			UVperVtx;			//!< eigentlich scheisse benannt: sollte UVIDs heissen, und definiert die UVs pro Face

	
	MIntArray					UVMin;				//!< verbessert die LUTPerformance im offsets Array
	MIntArray					UVMax;

	list<MIntArray>*			UVoffsetsLUT;
	list<MIntArray>*			UVoffsetsLUTValues;


	list<MIntArray>				UVBackup;			//!< enthlt die originalUVs des faces, welches gerade von changeUVs gendert wurde.\
													diese werden dann von der folgenden Append procedure verwendet, da diese die original UVs bentigt \
													pro UVSet, wird im konstruktor initialisiert											

	MStringArray				UVSetNames;			//!< strings mit UVSetNamen

	

	
//----------------
//SLIDE ARRAYS
//----------------

	double			slide;
	MDoubleArray*	normalScale;		//!< hat eigene versionen der Arrays, die schlussendlich von der BBTfty geholt werden
	MDoubleArray*	slideScale;			
	MVectorArray*	slideDirections;
	MVectorArray*	slideNormals;		//!< wenn ein Vtx nicht slided, dann wird gegenwrtiger punkt als startPunkt eingetragen und maySlide[n] wird auf 0 gesetzt
	MPointArray*	slideStartPoints;	
	MPointArray*	slideEndPoints;
	MIntArray*		slideIndices;
	MIntArray*		maySlide;			//!< synchron zu allen anderen Arrays, 0 wenn er sliden darf, 1 wenn nicht.

//----------------
//UV SLIDE ARRAYS
//----------------
	
	list<MFloatVectorArray>*		 UVSlideStart;
	list<MFloatVectorArray>*		 UVSlideEnd;
	list<MIntArray>*				 UVSlideIndices;
	list<MFloatVectorArray>*		 UVSlideDirections;	//!< zKomponente ist 0	

	list<MFloatArray>* UVSlideScale;	//!< wird nur mit leeren MFloatArrays initialisiert, wird hier aber nicht verwendet


//----------------
//DIVERSES DATEN
//----------------
	int					tmp;				//!< als dummy fuer setIndex(x,tmp);

	int					initialVtxCount;	//!< Anzahl der Vertizen bevor sie bearbeitet wurden.

	//BPT_Helpers			helper;	//hat hilfsprozeduren, und ist ffentlich (dies ist ein reminder )

	bool				deallocUVs;		//!< sollen die UVDaten mit der zerstrung des creators ebenfalls gelscht werden oder soll das jemand anders machen? \
										default = false;
};

}

#endif // !defined(AFX_MESHCREATOR_H__847CAC89_2F15_4C0D_8109_3094B92707C6__INCLUDED_)
