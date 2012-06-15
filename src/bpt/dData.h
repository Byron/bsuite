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

// dData.h: interface for the dData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDATA_H__1E5256C7_ACA4_4FD9_A34F_C62C888EC881__INCLUDED_)
#define AFX_DDATA_H__1E5256C7_ACA4_4FD9_A34F_C62C888EC881__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MACROS.h>

namespace BPT
{

/** Einfache KapselKlasse, welche directionalData beinhaltet. Genutzt von der face Klasse

*/

class dData  
{
public:
	dData();
	dData(int startLoc,int endLoc, int cStartLoc,int cEndLoc,int startAbs,int endAbs,int cStartAbs,int cEndAbs,int type,int requester,int inNachbarID);
	virtual ~dData();

	//reine ContainerKlasse, enthält nur einige Variablen

	int	startLocID;
	int	endLocID;
	
	int startLocCorner;
	int endLocCorner;
	
	int startAbsID;
	int endAbsID;
	
	int startAbsCorner;
	int	endAbsCorner;
	
	int typeInfo;	//enthält den Typ des originalRequesters, so dass DirectionControl Nodes entscheiden können, wie sie sich verhalten
	int	nachbarID;	//enthält die LocID des nachbarn, welcher um ne direction gefragt werden könnte. Wenn convertData/ToOpposite aufgerufen, verändern sie diesen Wert entsprechend
					//nachbarID sollte lokaler Wert sein (ob das überhaupt benötit wird ? JA - wenn mehrere Nachbarn pro CornerEdge)
	int requesterID;	//ist die ID des ursprünglichen Auftraggebers: in Loops muss dieser Vergleich dafür sorgen, dass Schleifen unmöglich werden
					//und ist absID
	bool isFlipped;

};

}

#endif // !defined(AFX_DDATA_H__1E5256C7_ACA4_4FD9_A34F_C62C888EC881__INCLUDED_)
