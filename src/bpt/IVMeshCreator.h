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


#ifndef IVMESHCREATOR_INCLUDE
#define IVMESHCREATOR_INCLUDE


#include "meshCreator.h"

namespace BPT
{

/** Der IVCreator implememtiert spezielisiert funktionen, welche von der IVFactory bzw. 
	dem vertexWorker verwendet werden

*/

class IVMeshCreator :
	public meshCreator
{

/////////////////////////////////////////////
//	KONSTRUKTION/DESTRUKTION
/////////////////////////////////////////////
public:
	IVMeshCreator(			MObject&		fMesh,
							UINT			inCount,
							double			inSlide, 
							MDagPath&		meshPath);


	virtual ~IVMeshCreator(void);



public:
	//-----------------------------
	//MESH MODIFIKATOREN
	//-----------------------------


	//* fügt die Vertizen ein, kümmert sich um UVs und Normalen, sowie slide
	void doInsertVtx(	int edgeID,
						bool useSlide = false, 
						int startVtxID = -1, 
						int endVtxID = -1 );			



private:
	//-----------------------------
	//PRIVATE HAUPTMETHODEN
	//-----------------------------

	void generateUVs(  MIntArray& connectedFaces,
					   MIntArray* vertices,
					   int		  startVtxID,
					   int		  endVtxID,
					   bool		  useSlide);

	
	void generateNormals(	MIntArray&	connectedFaces, 
							int			startVtxID,
							int			endVtxID		);



	//-----------------------------
	//UV HELFER
	//-----------------------------

	void	createUVs(	int&		 outMinIndex,		//ist output
						MIntArray&	 faceUVs,
						MFloatArray& UArray, 
						MFloatArray& VArray,
						int			 startLocID,
						int			 endLocID,
						std::list<MIntArray>::iterator* iterSlideIDs = 0,
						std::list<MFloatVectorArray>::iterator* iterSlideStart = 0,
						std::list<MFloatVectorArray>::iterator* iterSlideEnd = 0,
						std::list<MFloatVectorArray>::iterator* iterSlideDirs = 0 );


	void	updateUVArrays( int	faceID,
							MIntArray& faceVertices,	
							int  startLocID,
							int  endLocID,
							int  index,					
							int	 minIndex,				
							std::list<MIntArray>::iterator* iterCounts,
							std::list<MIntArray>::iterator* iterOffset,
							std::list<MIntArray>::iterator* iterLUT,
							std::list<MIntArray>::iterator* iterLUTValues,
							std::list<MIntArray>::iterator* iterUVperVtx	
																				);


//************************
//PRIVATE DATENELEMENTE
private:
//************************

	UINT	splitCount;		//zahl der Splits bei multiSplit

};

}

#endif


