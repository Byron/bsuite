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

// vertexFty.h: interface for the vertexFty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VTXFTY_INCLUDE
#define VTXFTY_INCLUDE

#include "meshCreator.h"
#include "BPT_Helpers.h"
#include "BPT_BA.h"

#include <maya/MIntArray.h>
#include <maya/MPointArray.h>


#include <list>

using std::list;


namespace BPT
{

/** Diese Klasse ist in der Lage, die selektierten Vertizen zu verbinden wenn mglich
	
*/

class vertexFty  
{
//------------------
//KONSTRUKTOREN
public:
//------------------

	vertexFty(	MIntArray* inSelVtxIDs, 
				meshCreator* inCreator,
				int	inSpin = 0);

	~vertexFty();

//------------------
//ARBEITSMETHODEN
public:
//------------------
	void	doIt();



//------------------
//PRIVATE HELFERMETHODEN
private:
//------------------
	
	//ConditionChecks für connect, koennen auf ptr gespeichert werden
	bool	isSelected				(UCHAR value);
	bool	gimmeFalse				(UCHAR value);

	//* erzeugt und verifiziert face
	bool	createFace				(	UINT tsb,					
										UINT teb,
										UCHAR* flags,
										MIntArray& faceVtx, 
										MIntArray& result	);		

	//* erzeugt und verifiziert Face anderen Typs
	bool	createExtraFace			(	UINT* bounds,				
										UCHAR* flags, 
										MIntArray& faceVtx, 
										MIntArray& result		);

	//* ueberprueft, ob das erzeugte Face in ordnung ist (also keine NullFläche hat)
	bool	checkFaceIntegrity		(	MIntArray& VtxList	);	
	
	
	void	splitFace(					
										MIntArray& faceVtx, 
										MIntArray& matchVtx	,
										int faceID				);


//------------------
//PRIVATE ELEMENTE
private:
//------------------

#ifdef DEBUG
	BPT_Helpers		helper;			//!< helper - wird nur für printArray benoetigt
#endif

	meshCreator*	ftyCreator;

	MIntArray*		selVtxIDs;

	int				spin;			//!< default ist 0

	double			refValue;		//!< hält die Groesse des Faces, welches gerade bearbeitet wird

//----------
//KONSTANTEN
private:
//----------
	static const UINT null; //!< null (0)
	static const UCHAR is;	//!< isSelected
	static const UCHAR iv;  //!< isValid

};

}

#endif
