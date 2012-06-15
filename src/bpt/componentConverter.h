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

// componentConverter.h: interface for the componentConverter class.
//
//////////////////////////////////////////////////////////////////////

#ifndef COMPONENTCONVERTER_INCLUDE
#define COMPONENTCONVERTER_INCLUDE

#include "BPT_BA.h"


#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>


namespace BPT
{

/** Einfache KapselKlasse, welche hilfsmethoden beinhaltet, die speziell f¸r die umwandlung von polygoncomponenten
	gemacht sind.

	Die Namen der Funktionen sind selbsterkl‰rend

*/

class componentConverter 
{

//------------------
//KONSTRUKTION
public:
//------------------

	componentConverter(MObject& mesh);
	~componentConverter();

//------------------
//ARBEITSMETHODEN
public:
//------------------

void		getContainedEdges(	MIntArray& vtxIDs,
								MIntArray& result	);

void		getConnectedFaces(MIntArray& edgeIDs,
							  MIntArray& result		);

void		vtxToConnectedFaceVtx(const	MIntArray&		vtxIDs, 
										MIntArray&		outFaceVtx);




//------------------
//PRIVATE DATENELEMENTE
private:
//------------------
MObject		mesh;			//h√§lt das mesh, auf dem gearbeitet wird
int			tmp;			//wird f√ºr.setIndex der iteratoren ben√∂tigt


};

}

#endif
