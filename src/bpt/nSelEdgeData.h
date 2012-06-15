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

// nSelEdgeData.h: interface for the nSelEdgeData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	NSELEDGEDATA
#define NSELEDGEDATA



#include "edgeFaceData.h"



namespace BPT
{

/** diese Klasse dient als DatenKapsel
	
*/

class nSelEdgeData  
{
//----------------------------
//KONSTRUKTION - DESTRUKTION
public:
//----------------------------
	nSelEdgeData(void* nSelEdge, int origID, int newID, edgeFaceData* faceData);
	~nSelEdgeData();


//----------------------------
//VARIABLEM
public:
//----------------------------
	void* nSelEdge;		

	int origID;
	int newID;

	const edgeFaceData* faceData;

};

}

#endif 
