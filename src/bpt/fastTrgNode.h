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

// fastTrgNode.h: interface for the fastTrgNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FASTTRGNODE_H__EF351480_0CA3_43C0_8839_FADDD3E0E50A__INCLUDED_)
#define AFX_FASTTRGNODE_H__EF351480_0CA3_43C0_8839_FADDD3E0E50A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000








#include <maya/MPxPolyTrg.h>
#include <maya/MFnMessageAttribute.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MString.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>

#include <MACROS.h>

/////////////////////////////////////////////////////////////////////////////
//
// testPolyTrgNode node declaration
// 
/////////////////////////////////////////////////////////////////////////////


namespace BPT
{

	/** Diese Klasse definiert eine Node, welche eine neue TriangulationFunction definiert
	
	  @remarks
		Die Funktion wird hauptsächlich von der softTransformationEngine verwendet, um die 
		Deformation der Meshes im shadedMode zu beschleunigen
	*/

//----------------------------------------------------------------------------------------------
class fastTrgNode : public MPxPolyTrg
//----------------------------------------------------------------------------------------------
{
public:
    fastTrgNode() {};
    virtual ~fastTrgNode();

    virtual MStatus 	compute(const MPlug& plug, MDataBlock& data);
    virtual void		postConstructor( void );
	virtual bool		isAbstractClass( void ) const;

    static  void* 		creator();
    static  MStatus		initialize();

	// User triangulation - this is a triangulation per face.
	static  void 		triangulateFace( const float    *vert, 
									 	 const float 	*norm, 
									 	 const int		*loopSizes,
									 	 const int		nbLoops,
									 	 const int 		nbTrg,
									 	 unsigned short *trg );

public:
    static  MTypeId id;


};

}

#endif // !defined(AFX_FASTTRGNODE_H__EF351480_0CA3_43C0_8839_FADDD3E0E50A__INCLUDED_)
