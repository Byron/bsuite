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

// endFaceData.h: interface for the endFaceData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	ENDFACEDATA
#define ENDFACEDATA




#include "edgeFaceData.h"


namespace BPT
{
	
	/** diese Klasse dient als DatenKapsel für Daten, die ein Face so bekommt, wenn es von edges während des Chamfers verändert wird
		
	*/
	class endFaceData  
	{
	//----------------------------
	//KONSTRUKTION - DESTRUKTION
	public:
	//----------------------------
		
		endFaceData(	int origVtx, 
						int newVtx[2], 
						edgeFaceData* faceData[2], 
						int connectEnds[2], 
						int inDirections[2],
						int edgeID);			//!< wenn für connectEnds ein nullPtr gegeben wird heisstdas, dass quads erstellt werden sollen
		~endFaceData();
	
	//----------------------------
	//VARIABLEN
	public:
	//----------------------------
	
		int origVtx;
		int newVtx[2];
		edgeFaceData* faceData[2];
		int connectEnds[2];			//!< werte bestimmen modus; ce[0] == -2 -> createQuads, ansonsten 0, wenn CE false, 1 wenn true -> in diesen Fällen wird sowieso trianguliert
		
		int directions[2];			//!< besagt, ob der refVtx in faceRictung zeigt (true), oder nicht
	
		int branding;				//!< hält die edgeID der edge, die diese instanz erstellt hat. Wird von chamfer benötigt, um später seine faces wiederzufinden, irrelevant für SC
	};


}

#endif // ENDFACEDATA

