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

// edgeFaceData.h: interface for the edgeFaceData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef EDGEFACEDATA
#define EDGEFACEDATA

#include <maya/MIntArray.h>
#include "BPT_Helpers.h"


#include <list>

using std::list;

namespace BPT
{

/** diese Klasse dient als DatenKapsel für Daten, die ein Face so bekommt, wenn es von edges während des Chamfers verändert wird
	
	  Diese Einheit kann dann an edgeMeshCreator übergeben werden, um alles notwendige einzuleiten
*/


class edgeFaceData  
{
//----------------------------
//KONSTRUKTION - DESTRUKTION
public:
//----------------------------
	edgeFaceData(UINT inFaceID);
	~edgeFaceData();


//----------------------------
//ZUGRIFFSMETHODEN
public:
//----------------------------
	
	MIntArray				getNewVtxUVs(int thisVtx) const;			//!< holt die UVs des übergebenen Vtx, gibt IntArray zurück, welches leer ist, wennn UVHandling aus
	
	MIntArray				getEdgeVtxUVs(int thisVtx) const;

	MIntArray				insertEdgeVtx(int startID, int endID, int edgeVtxID,bool inFaceRichtung = true);					//!< fügt edgeVtxID zwischen start und end ein

	void					insertEdgeVtx(int startID, int edgeVtxID, bool inFaceRichtung);

	void					assignUV(int thisVtx, MIntArray& UVIDs);

	bool					verifyStartEndVtx(int& startID, int& endID) const;	//!< Rückgabe: Wenn start/end vertauscht wurden, true, ansonsten false

	bool					hasUVs() const {return !(numUVSets == 1 && !UVRelOffsets[0]);};

	void					invert();

//----------------------------
//PRIVATE METHODEN
private:
//----------------------------

void						updateUVArrays(int localUVID, int offset);


//----------------------------
//DATENELEMENTE - Das face selbst betreffend, hält nur die neuen Vertizen, die auch dieses Face direkt betreffen
public:
//----------------------------

	MIntArray				vtxChange;		//!< hält die original und die newVtx IDs; ist kodiert: N*2 = origVtxID, N*2+1 = newVtxID
	
	/** hält alle UVs; ist sync zu vtxChange; ist kodiert (siehe detaills)
	
	N = numUVSets, X = vtxChangeIndex / 2; Hält immer origUV gefolgt von newUV -> *2
	->> X*N*2 + N-N*2		= origUv des X'ten Vtx des N'ten UVSets
	->> X*N*2 + N-N*2+1	= NewUv des X'ten Vtx des N'ten UVSets, zugehörig zu origUV
	
	 @par
		wenn eine UV nicht vorhanden ist, dann ist im Array -1 !!!
	*/
	MIntArray				UVChange;		

//----------------------------
//DATENELEMENTE - Als Zwischenspeicher für optimalen Zugriff auf die Facedaten
public:
//----------------------------
//MERKE : DIESE DATEN HIER MÜSSEN DURCH DIE EDGES AKTUALISIERT WERDEN!

	MIntArray				faceVertices;	//!< hält die Vertizen des Faces

	MIntArray				UVIndices;		//!< Hält die UVs des Faces, sync zu faceVertices, kodiert: x = UV zu vtx, n = numUVsets, l = länge vtxArray, 
											//!< ->UVIDs[l*n+x]


	unsigned long*			UVAbsOffsets;	//!< hält absoluten offset pro UVSet, dient der indizierung des UVIndices arrays
											//!< Beispiel: Face mit 5 vertizen und 3 UVSets, wobei das mittlere keine UVs hat, sieht so aus: {5,5,10}
											//!< wird während creation mit array initialisiert

	bool*					UVRelOffsets;	//!< hat faceUVSs oder nicht?, pro UVSet;	//wird während creation mit array initialisiert


	MIntArray				normalIndices;	//!< Hält die VtxIndices, die zusammen eine hardEdge bilden: 1 - 2, 2 - 3, 3 - 1;


//----------------------------
//Sonstige Variablen
public:
//----------------------------

	static unsigned short	numUVSets;		//!< wird in ftyAction gesetzt

	const UINT				id;				//!< hält die FaceID

	

//----------------------------
//PRIVATE DATENELEMENTE 
private:
//----------------------------
	//HelperObject
	static BPT_Helpers		helper;

	
};

}

#endif
