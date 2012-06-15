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

// endFaceData.cpp: implementation of the endFaceData class.
//
//////////////////////////////////////////////////////////////////////

#include "endFaceData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

endFaceData::endFaceData(	int inOrigVtx,
							int inNewVtx[2], 
							edgeFaceData* inFaceData[2], 
							int inConnectEnds[2], 
							int inDirections[2],
							int edgeID)
														:
																origVtx(inOrigVtx)
																
{
	newVtx[0] = inNewVtx[0];		newVtx[1] = inNewVtx[1];


	branding = edgeID;


	faceData[0] = inFaceData[0];	faceData[1] = inFaceData[1];

	directions[0] = inDirections[0];	directions[1] = inDirections[1];

	if(inConnectEnds[0] == -2)
	{//es ist quad mode
		connectEnds[0] = -2;
	}
	else
	{// es ist trimode
		connectEnds[0] = inConnectEnds[0];
		connectEnds[1] = inConnectEnds[1];
	}

}

endFaceData::~endFaceData()
{

}
