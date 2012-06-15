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

/////////////////////////////////////////////////////////////////////////////
//
// fastTrgNode.cpp
//
// Description:
//		This node register a user defined face triangulation function.
//		After the function is register it can be used by any mesh in 
//		the scene to do the triangulation (replace the mesh native 
//		triangulation). In order for the mesh to use this function,
//		an attribute on the mesh: 'userTrg' has to be set to the name
// 		of the function. 
//	
//		Different meshes may use different functions. Each of them 
//		needs to be register. The same node can provide more than 
//		one function.  
//
// Example:
//		createNode polyTrgNode -n ptrg;
// 
//		polyPlane -w 1 -h 1 -sx 10 -sy 10 -ax 0 1 0 -tx 0 -ch 1 -n pp1;
//
//		select  -r pp1Shape;
//		setAttr pp1Shape.userTrg  -type "string" "triangulate";
//
/////////////////////////////////////////////////////////////////////////////
#include "fastTrgNode.h"
/////////////////////////////////////////////////////////////////////////////
//
// polyTrgNode implementation
//
/////////////////////////////////////////////////////////////////////////////

MTypeId fastTrgNode::id(0x00108BC4);

//----------------------------------------------------------------------------------------------
fastTrgNode::~fastTrgNode() 
//----------------------------------------------------------------------------------------------
{
//
//	Description:
//		Destructor: unregister the triangulation function. 
//

	// Unregister the triangulation function.
    MStatus stat = unregisterTrgFunction( "fastTrg" );
}

//----------------------------------------------------------------------------------------------
void * fastTrgNode::creator()
//----------------------------------------------------------------------------------------------
{
    return new fastTrgNode();
}

//----------------------------------------------------------------------------------------------
void fastTrgNode::postConstructor()
//----------------------------------------------------------------------------------------------
{
//
//	Description:
//		Constructor: register the triangulation function. 
//

    // Register the triangulation function.
	// The string given as a first parameter has to be
	// the same as the name given when setting the usrTrg
	// attribute on the mesh. See example above.
	// 
    MStatus stat = registerTrgFunction( "fastTrg", fastTrgNode::triangulateFace );
}

//----------------------------------------------------------------------------------------------
bool fastTrgNode::isAbstractClass( void ) const
//----------------------------------------------------------------------------------------------
{
	return false;
}

//----------------------------------------------------------------------------------------------
MStatus fastTrgNode::initialize()
//----------------------------------------------------------------------------------------------
{
    return MS::kSuccess;
}

//----------------------------------------------------------------------------------------------
MStatus fastTrgNode::compute(const MPlug& plug, MDataBlock& data)
//----------------------------------------------------------------------------------------------
{
	return MS::kSuccess;
}

//----------------------------------------------------------------------------------------------
void  
fastTrgNode::triangulateFace( 
	const float 	*vert, 			// I: face vertex position
	const float 	*norm, 			// I: face normals per vertex
	const int		*loopSizes,		// I: number of vertices per loop 
	const int		nbLoops,		// I: number of loops in the face
	const int 		nbTrg,			// I: number of triangles to generate
	unsigned short *trg				// O: triangles - size = 3*nbTrg. 
									//    Note: this array is already allocated.
)
//----------------------------------------------------------------------------------------------
{
//
//  Description:
//		Triangulate a given face. Returns triangles given by 
//		the relative vertex ids. Example:
//	   		nbTrg = 2
//	   		trg: 0, 1, 2,  2, 3, 0
//
	int nbVert = 0;
	for (int i=0; i<nbLoops; i++ ) 
	{
		nbVert += loopSizes[i];

	}

	int v0 = 0;
	int v1 = 1;
	int v2 = 2;
	
	for (int k=0; k<nbTrg; k++)
	{
		trg[3*k] 	= v0;
		trg[3*k+1]  = v1;
		trg[3*k+2]  = v2;

		v1 = v2;
		v2 ++;
		if (v2 >= nbVert)
		{
			v2 = 0;
		}
	}
	
/*	Minimal langsamer als die obere variante

	int myNbTrg = nbTrg - 1;

	for (int k=0; k<nbTrg; k++)
	{
		trg[3*k] 	= v0;
		trg[3*k+1]  = v1;
		trg[3*k+2]  = v2;

		v1 = v2;
		v2 ++;
	}
	
	v1 = v2;
	v2 = 0;
	k++;

	trg[3*k] 	= v0;
	trg[3*k+1]  = v1;
	trg[3*k+2]  = v2;

*/
}


