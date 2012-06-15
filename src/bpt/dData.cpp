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

// dData.cpp: implementation of the dData class.
//
//////////////////////////////////////////////////////////////////////

#include "dData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dData::dData()//:isFlipped(false)
{}

dData::dData(int startLoc,int endLoc, int cStartLoc,int cEndLoc,int startAbs,int endAbs,int cStartAbs,int cEndAbs,int type,int requester,int inNachbarID)
:startLocID(startLoc), endLocID(endLoc), startLocCorner(cStartLoc), endLocCorner(cEndLoc), startAbsID(startAbs),
endAbsID(endAbs),startAbsCorner(cStartAbs),endAbsCorner(cEndAbs), typeInfo(type),requesterID(requester),nachbarID(inNachbarID),
isFlipped(false)
{}

dData::~dData()
{

}
