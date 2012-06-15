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

// selEdge.h: interface for the selEdge class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SELEDGE_H
#define SELEDGE_H



#include <edge.h>
#include <nSelEdge.h>
#include <edgeFaceData.h>
#include <endFaceData.h>


#include <maya/MGlobal.h>



namespace BPT
{

/** diese Klasse dient als DatenKapsel und stellt ein gewählte edge dar 
	
*/


class selEdge : public edge  
{

public:
	selEdge(int inID);
	selEdge(MIntArray inFaceIDs,int inID);
	virtual ~selEdge();


public:
//-------------------------------------
//HAUPT METHODEN	
//-------------------------------------
//BEGINNEN DES BEVELS


	virtual void	engageBevel(int connectEnds, int triangulateEnds, bool isSC);

	void			createSmallChamferFaces();	//!< chamfer


public:
//-------------------------------------
//ZUGRIFFS METHODEN	
//-------------------------------------
//Setzen und erhalten von Werten

	void			setSingleGerade(MDoubleArray inGerade);
	void			setSinglePlane(int richtungsIndex, MDoubleArray inPlane);




	virtual void	setUIDs(MIntArray& IDs);		//!< setzt einfach die uniqueID auf übergebenen wert, dient der eindeutigen identifikation von nurEinNachbarEdges

	/** intArray hat zwei einträge: [0] ist schnittVtxID der verschobenen Requesterplane und der versch. nachbarGeraden, [1] ist
	schnittpunkt mit nicht verschobener geraden
	*/
	virtual	void	setNewVerts(	const edge* requester, 
									MIntArray& newVerts,
									bool useUID = false, 
									int uniqueID = 0);					
	

	virtual	bool			isSelected(){return true;};

	//* eigentlich könnte man den UseID flag sparen und einfach alles anhand unique id machen (if(uniqueID != -1))
	virtual MDoubleArray	getGerade(	bool displace,
										const edge* requester, 
										bool useUId = false, 
										int uniqueID = -1);			

	virtual MIntArray		getNewVtx() const{return MIntArray(newVtx,4);};

	virtual	int				getValidRichtung(int uniqueID);

	virtual void			getValidRichtung(const edge* requester, int& seite, int& richtung, int uniqueID = -1);

	virtual int				getNewGeradenVtx(const edge* requester);	//!< gibt Vtx zurück, der auf der edge liegt


	virtual	const int*		getUIDs() const {return uID;}


	


private:
//-------------------------------------
//HELFER METHODEN	
//-------------------------------------
//

//---------------ALLGEMEINE HELFER
//-------------------------------------

	
	virtual edge*	findNextSelEdge(edge* lastEdge);

	bool			isParallel(double threshold, edge* nachbar, double& value, int richtung = -1, int uniqueID = -1);




//---------------CHAMFER HELFER
//--------------------------------------------
	void			C_initFaceData(edgeFaceData* thisFace, int thisVtx, edgeFaceData* refFace, bool isNew);

	UINT			C_addVtx(int requesterID,const selEdge* requester, edgeFaceData* fData, int& centerVtx, bool needsEdgeVtx);

	bool			C_queryCenterVtx(int requesterID,edge* lastEdge, MIntArray& newVtxIDs, int& centerVtxID );	
	
	void			C_dataAddVtx(edgeFaceData* fData,const edgeFaceData* nRefFace, int thisVtx, bool isNew);


//---------------MESH MODIFIKATOR HELFER
//--------------------------------------------

	
	
	int				intersect(bool isEdgeDisplaced,bool displacePlane,edge* nachbar, bool isNurEinNachbar = false);

	//* chamfer ist nur ein dummyParameter, damit ich die gunktion überladen kann
	MPoint			intersect(bool isEdgeDisplaced,bool displacePlane,edge* nachbar, bool isNurEinNachbar, bool chamfer);	



	void			insertNewVtx(MPoint& inPoint, bool specialMode);
	
	MIntArray		checkNewFaceIntegrity(MIntArray& newFaces);

	double			get3dArcRatio(edge* nachbar,int seite);

	MPoint			getDisplacePoint(int startID, edge* otherEdge);


private:
//**************************************
//DATENELEMENTE
//**************************************
//
	
	//-----------------
	//GEOMETRIE-DATEN
	//-----------------
	//MERKE:	wenn die Edge nonwinged, ist der erste index von plane und gerade immer 0.
	//			wenn winged Vertex nicht größer 1 ist winged normals leer und die normale des punktes ergibt sich aus
	//			den beiden normalen der planes
	
	/**		plane[richtung][koordinaten]

	hält planes der Edge in normalenForm: (x - a) * n = 0 -> {ax,ay,az,nx,ny,nz}
	eine plane pro richtung, jeweils für jede endEdge falls vorhanden 
	ansonsten wird nur ein index (plane[0][x][y] genutzt, sync. zu richtung

	*/
	double	plane[2][6];	
	
	//*hält die gerade der Edge, in parameterform {x,y,z,u,v,w}
	double	gerade[6];	

	/**enthält die IDs der neuen Vtx(=schnittpunkte der verschobenen planes/geraden)
	
	  index ergibt sich aus Seite*2+Richtung wenn kein Vtx drin, sind die werte -1
	*/
	int		newVtx[4];	

	bool	smallFaceClean[2]; //!< sync zu vtx, wird von chamfer verwendet
	

	//*enthält den für jede richtung einzigartigen identifier (die EdgeID, die das face dieser richtung mit	dem face der anderen edge teilt)
	int		uID[4];	

	
	
};

}

#endif 
