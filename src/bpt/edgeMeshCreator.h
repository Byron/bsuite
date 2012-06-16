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

// edgeMeshCreator.h: interface for the edgeMeshCreator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDGEMESHCREATOR_H__0FC9C2BD_E36E_4E59_BB37_85D73A318D17__INCLUDED_)
#define AFX_EDGEMESHCREATOR_H__0FC9C2BD_E36E_4E59_BB37_85D73A318D17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "meshCreator.h"
#include "edgeFaceData.h"



namespace BPT
{

/** Der EdgeMeshCreator implememtiert spezialisierte funktionen, welche von der BPTfty verwendet werden, er
	wird also insbesondere zum beveln bentigt

*/

class edgeMeshCreator : public meshCreator  
{
public:
	edgeMeshCreator(		MObject&		fMesh,
							double			inSlide, 
							MDagPath&		meshPath
															 );
	
	virtual ~edgeMeshCreator();

//---------------
//SLIDE METHODEN
//---------------

	void	addNormalSlide(int vtxID);										//!< speziell fuer vtxIDs der Edges
	void	addSlideAndNormal(int startID,int edgeVtx,MIntArray&  faces);	//!< nur fuer die neuen Vtx auf Edges

	void	addSlideAndNormal(int startID, int edgeVtx,MPoint& position, bool usePositionInstead = false);	//!< speziell fuer chamfer;

	void	addSlide(int startID,int endID);

	void	addUVSlide(const MFloatArray& startPositions,const MFloatArray& endPositions,const MIntArray& newUVIds);

	MPoint	getSlideStartPoint(int vtxID);
	
	void	getUVSlideStartPoint(const MIntArray& UVIDs, MFloatArray& outPos);

//--------------------------
//ZUGRIFFS METHODEN
//--------------------------

	MPoint	getWingedDisplacedPoint(int wingedVtx, MVector& preMultipliedNormal);
	
	MPoint	getVtxPoint(int ID){return (*vtxPoints)[ID];};

	void	setVtxPoint(int ID,MPoint& inPoint){ (*vtxPoints)[ID] = inPoint;};

	void	getFaceNormal(int faceID, MVector& inNormal);

	MPoint	getFaceCenter( int faceID ){polyIter.setIndex(faceID,tmp); return polyIter.center();}

	void	getFaceTriangles(int faceID, MIntArray& outTriVtx, MPointArray& outTriPoints);


//-------------------------------------
//MESH MODIFIKATOREN (EDGE SPEZIFISCH)
//-------------------------------------

	void	insertVtx(const edgeFaceData* faceData, int startID, int newID,int direction );
	
	void	removeVtx(int vtxID, int faceID);


	void	changePolyVtxIDs(const edgeFaceData* faceData);


	void	createPoly(MIntArray& vtxIDs, edgeFaceData* faceData);		//!< spezielisierte Variante von create Poly

	void	createPoly(const edgeFaceData* data);
	

	void	changeSideFace(int origVtxID, int newVtxID, int faceID, MIntArray& newUVIds );

	void	changeEndFace(int origVtxID,int newVtxID[],int directions[], edgeFaceData* faceData[2]);

	void	changeEndFace(int origVtxID,int newVtxID, int direction, edgeFaceData* faceData); //!< speziell fuer die endbereiche des chamfers
	


	int		addWingedDisplacedVtx(int wingedVtx, MVector& preMultipliedNormal);

	void	removeLastVtxPoint(){vtxPoints->remove(vtxPoints->length() - 1);};


	void	updateFaceData(edgeFaceData* faceData);
	

	void	recreateOriginalFace(edgeFaceData* origFace);




//-------------------------------------
//UV MESH
//-------------------------------------

	void		setUVPositions(MIntArray& UVIds, MFloatArray& UVPos);

	MIntArray	createUVs(MIntArray& validator, MFloatArray& UVPos);




//PRIVATE METHODEN
private:
	
	


};

}

#endif // !defined(AFX_EDGEMESHCREATOR_H__0FC9C2BD_E36E_4E59_BB37_85D73A318D17__INCLUDED_)
