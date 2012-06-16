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

// componentConverter.cpp: implementation of the componentConverter class.
//
//////////////////////////////////////////////////////////////////////

#include "componentConverter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------
componentConverter::componentConverter(MObject& inMesh):mesh(inMesh)
//---------------------------------------------------------------------
{
}

//---------------------------------------------------------------------
componentConverter::~componentConverter()
//---------------------------------------------------------------------
{

}


//---------------------------------------------------------------------
void	componentConverter::getContainedEdges(	MIntArray& vtxIDs,
												MIntArray& result	)
//---------------------------------------------------------------------
{
//	result.clear();

	MFnMesh meshFn(mesh);
	MItMeshVertex vertIter(mesh);

	BPT_BA searchArray(meshFn.numEdges());
	
	MIntArray conEdges;
	

	
	unsigned int l = vtxIDs.length();
	int indexValue;
	for(unsigned i = 0;i < l;i++)
	{
		vertIter.setIndex(vtxIDs[i],tmp);
		vertIter.getConnectedEdges(conEdges);
		
		
		
		for(unsigned int x = 0; x < conEdges.length(); x++)
		{
			indexValue = conEdges[x];
			if( searchArray[indexValue] )
			{
				result.append(indexValue);
			}
			else
				searchArray.setBitTrue(indexValue);
		}
	}




}

//---------------------------------------------------------------------------
void		componentConverter::getConnectedFaces(MIntArray& edgeIDs,
												  MIntArray& result		)
//---------------------------------------------------------------------------
{
	
	MItMeshEdge edgeIter(mesh);
	MIntArray connectedFaces;
	
	result.clear();
	
	unsigned int l = edgeIDs.length();


	for(unsigned int i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeIDs[i],tmp);
		edgeIter.getConnectedFaces(connectedFaces);
		
		for(unsigned int x = 0; x < connectedFaces.length();x++)
		{
			result.append(connectedFaces[x]);
		}		
	}

}



//--------------------------------------------------------------------------------------
void		componentConverter::vtxToConnectedFaceVtx(const	MIntArray&		vtxIDs, 
															MIntArray&		outVtxIDs)	
//--------------------------------------------------------------------------------------
{
	// Wandelt die vtxSelection in die connecteten faceVtx um (die Vertizen der verbundenen Faces)
	// Die gegebenen Vtx werden nicht mit hinzugefuegt

	outVtxIDs.setLength(0);
	outVtxIDs.setSizeIncrement(vtxIDs.length() / 4);

	MItMeshVertex	vertIter(mesh);
	MItMeshPolygon	polyIter(mesh);

	BPT_BA			allFaces(polyIter.count(), true);
	BPT_BA			allVtx(vertIter.count(), true);	// BA mit den vtxIDs initialisieren

	// Die vtxIds bereits jetzt false setzen
	allVtx.setBits(vtxIDs, false);


	MIntArray		conFaces;		// hlt die verbundenen Faces
	MIntArray		conVtx;			// Im face enthaltene Vtx
	
	uint i, x, y , l2,l3, l = vtxIDs.length();
	
	
	for(i = 0; i < l; i++)
	{
		vertIter .setIndex(vtxIDs[i], tmp);
		
		vertIter.getConnectedFaces(conFaces);
		
		// Jetzt die gueltigen conFaces holen
		conFaces = allFaces & conFaces;
		
		// Jetzt die conFaces false setzen, danit diese nicht wieder  bearbeitet werden
		allFaces.setBits(conFaces, false);
		
		
		l2 = conFaces.length();
		
		// jetzt die restlichen Faces on the fly in Vtx umwandeln
		for(x = 0; x < l2; x++)
		{
			
			// Jetzt die vertizen des Faces holen und auf Array packen, wenn sie einzigartig sind
			polyIter.setIndex(conFaces[x], tmp);
			
			polyIter.getVertices(conVtx);
			
			// Checken, ob Vtx einzigartig sind
			conVtx = allVtx & conVtx;

			// Das was uebrig bleibt im BitArray deaktivieren und zum outArray hinzufuegen 
			allVtx.setBits(conVtx, false);


			l3 = conVtx.length();
			for(y = 0; y < l3; y++)
			{
				outVtxIDs.append(conVtx[y]);
			}
		}
				
	}
	
}