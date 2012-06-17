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

// edgeFaceData.cpp: implementation of the edgeFaceData class.
//
//////////////////////////////////////////////////////////////////////

#include "edgeFaceData.h"
#include <cstdlib>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned short	edgeFaceData::numUVSets;
BPT_Helpers		edgeFaceData::helper;

//---------------------------------------------
edgeFaceData::edgeFaceData(UINT inFaceID)
: UVAbsOffsets(0)
, UVRelOffsets(0)
, id(inFaceID)
//---------------------------------------------
{	
}

//---------------------------------------------
edgeFaceData::~edgeFaceData()
//---------------------------------------------
{



	if(UVAbsOffsets != 0)
		free(UVAbsOffsets);

	if(UVRelOffsets != 0)
		free(UVRelOffsets);


}



//----------------------------------------------------
MIntArray		edgeFaceData::getNewVtxUVs(int newVtx) const
//----------------------------------------------------
{
	//gibt nur die UVs zurück, die zu neuem Vtx passen

	//zuerst suchen in newVtxArray
	MIntArray UVs(numUVSets, -1);	

	//in diesem Fall sind definitiv keine UVs vorhanden
	if(numUVSets == 0 ||  (numUVSets == 1 && UVRelOffsets[0] == false))
		return UVs;


	UINT l = vtxChange.length();
	UINT r = 0;

	for(unsigned i = 1; i < l; i+=2)
	{
		if(vtxChange[i] == newVtx)
		{			

			unsigned l2 = --i*numUVSets + numUVSets*2;
			for(i = i*numUVSets+1; i < l2; i+=2)
			{
				UVs[r++] = UVChange[i];
			}

			return UVs;
		}
	}

	//forbidden Path
	return UVs;

}

//----------------------------------------------------
MIntArray		edgeFaceData::getEdgeVtxUVs(int thisVtx) const
//----------------------------------------------------
{

	//UVArray muss immer numUVSets einträge haben
	MIntArray UVs(numUVSets, -1);
	
	//in diesem Fall sind definitiv keine UVs vorhanden
	if(numUVSets == 0 ||  (numUVSets == 1 && UVRelOffsets[0] == false))
		return UVs;

	UINT l = faceVertices.length();

	for(UINT i = 0; i < l; i++)
	{
		if(faceVertices[i] == thisVtx)
		{
			
			//jetzt durch die UVs parsen und UVArray schreiben
			UINT r = 0;

			for(UINT a = 0; a < numUVSets; a++)
			{
				if( UVRelOffsets[a] )
				{
					UVs[r] = UVIndices[UVAbsOffsets[a] + i];
				}

				r++;
			}

			return UVs;
		}
	}

	//forbidden Path
	return UVs;	//dann eben -1 ausgeben, damit der aufrufer was zum vergleichen hat

}


//-----------------------------------------------------------------------------------
void		edgeFaceData::assignUV(int thisVtx, MIntArray& UVIDs)
//-----------------------------------------------------------------------------------
{
	
	UINT l = faceVertices.length();
	for(UINT i = 0; i < l; i++)
	{
		if(faceVertices[i] == thisVtx)
		{
			//da die UVs inSync sind, kann man jetzt einfach die neuen UVs zuweisen
			for(UINT x = 0; x < numUVSets; x++)
			{
				if(UVRelOffsets[x])
					UVIndices[UVAbsOffsets[x] + i] = UVIDs[x];
			}

			return;
		}

	}


}


//-----------------------------------------------------------------------------------
void			edgeFaceData::invert()
//-----------------------------------------------------------------------------------
{
	//kehrt alle Daten um - bei den facevertices und den normalindices ist das kein problem

	helper.invertArray(faceVertices);

	
	helper.invertArray(normalIndices);


	//für die UVs die erweiterte invertArray prozedur verwenden
	if(hasUVs())
	{
		UINT lb = 0;
		UINT rb = 0;
		UINT l = faceVertices.length() - 1;	//-1, weil man sich hier ja auf die 0 basierenden indices des arrays bezieht, l aber 1 basierend wäre

		for(UINT i = 0; i < numUVSets; i++)
		{
			rb = lb + l*(int)UVRelOffsets[i];
			helper.invertArray(UVIndices, lb, rb);
			lb = rb;
		}
			
	}
}



//-----------------------------------------------------------------------------------
bool				edgeFaceData::verifyStartEndVtx(int& startID, int& endID) const
//-----------------------------------------------------------------------------------
{
	bool isFlipped = false;	//wurde der input vertauscht?

	UINT l = faceVertices.length();
	
	
	//startID finden
	for(UINT i = 0; i < l; i++)
	{
		if(faceVertices[i] == startID)
		{//den Punkt finden, an dem eingefügt werden muss
			if(faceVertices[(i+1)%l] == endID )
			{
				return isFlipped;
			}
			else if(faceVertices[(i+l - 1)%l] == endID)	
			{

				//dies bedeutet auch, dass die Vtx geflipped sind, so dass dies jetzt korrigiert werden muss, damit dir normalen korrekt bearbeitet werden koennen
				int tmp = startID;
				startID = endID;
				endID = tmp;

				isFlipped = true;

			}
			else
			{//jetzt ist ein besonderer Fall aufgetreten, und es muss eine erweiterte Suche gestartet werden
				//im endeffekt muss hier die Richtung gefunden werden, in der der passende endVtx liegt: i+1 oder i-1
				UINT x = (i+2)%l;

				//das geht in besonderen Fällen leider nicht! Nämlich dann, wenn 2 selEdges so liegen, dass sie nichts voneinander wissen, und die eine über change endFace die topologie 
				//des Faces ändert, welches sich die andere dann holt. Diese sucht dann bei insertEdgeVtx nach einer endID, die einfach nicht mehr vorhanden ist
				//da sie ersetzt wurde durch andere ID. In diesem Fall ist es dann reines Glück, wenn das face trotzdem richtig aussieht
				
				UINT l2 = l - 3;	//-4, damit der letzte zu suchende Vtx entspricht: (i+l-2)%l
				l2 = (l2 - (l2 % 2)) / 2;	//jetzt l2 so umrechnen, das es bis zur geraden Hälfte des Faces zählt.
											//wenn bis dorthin nichts gefunden wurde, dann muss der gesuchte endVtx hinter dem StartVtx liegen
				bool found = false;
				for(UINT r = 0; r < l2  ; x = (x+1)%l, r++)
				{
					if(faceVertices[x] == endID)
					{//in faceRichtung wurde endVtx gefunden -  also endFaceID entsprechend anpassen und Vtx einfügen
						endID = faceVertices[(i+1)%l];
						

						found = true;
						break;
					}
				}

				if(!found)
				{//der endVtx muss hinter dem statVtx sein - also alles entsprecend anpassen
					int tmp = startID;
					startID = faceVertices[(i+l - 1)%l];
					endID = tmp;
					
					isFlipped = true;
				}

			}
					

			//nochmal wird der startVtx nicht gefunden werden ;)
			return isFlipped;
		}
	}

return false;
}


//-----------------------------------------------------------------------------------
void	edgeFaceData::updateUVArrays(int i, int offset)
//-----------------------------------------------------------------------------------
{
	UINT l = faceVertices.length();
	
	if(UVIndices.length() > 0)
	{
		UINT locOff = 0;
		UINT x;
		for(x = 0; x < numUVSets; x++)
		{
			UVIndices.insert(0, UVAbsOffsets[x] + locOff++ + ((i+l+offset) % l) );
		}
		
		//jetzt das Absoffsets Array aktualisieren
		for(x = 1; x < numUVSets; x++)
		{
			UVAbsOffsets[i]++;
		}
	}
}

//-----------------------------------------------------------------------------------
MIntArray			edgeFaceData::insertEdgeVtx(int startID, int endID, int edgeVtxID, bool inFaceRichtung)
//-----------------------------------------------------------------------------------
{

	//MERKE: Diese prozedur verfügt nur über minimale fehlerkorrektur

//VTX BEARBEITEN

	UINT l = faceVertices.length();
	

	bool isFlipped = verifyStartEndVtx(startID, endID);

	//jetzt noch die startID finden und den edgeVtx einfach einfügen
	
	
	UINT i;
	for(i = 0; i < l; i++)
	{
		if(faceVertices[i] == startID)
		{
			//aleso den EdgeVtx an i+i einfügen
			//jetzt auch gleich UVs aktualisieren, was die Arraygroesse angeht

			if(inFaceRichtung)
			{
				
				updateUVArrays(i, 1);	//diese Prozedur immer vorher ausführen, damit l noch korrkt ist für UVIDs
				
				faceVertices.insert(edgeVtxID, (i+1) % l);
				
				
			}
			else
			{
				if(isFlipped)
				{
					updateUVArrays(i, 2);

					faceVertices.insert(edgeVtxID, (i+2) % l);
				}
				else
				{
					updateUVArrays(i, 0);

					//also gegen die FaceRichtung
					faceVertices.insert(edgeVtxID, i );
				}
			}

			break;
		}

	}


	
//NORMALEN BEARBEITEN
	//jetzt um die normalen kümmern (in dem Moment, wo keine nomalen vorhanden sind, ist das array leer und es passiert eh nix)
	//hier müssen die vtx conform zur faceDir sein

	//nur jeden 2. Vtx checken, da die vertizen hier immer paarweise abgelegt sind
	if(inFaceRichtung)
	{
		if(helper.getLocIndexMod2(startID, normalIndices, i))
		{
			i = (i+1) % normalIndices.length();
			
			normalIndices.insert(edgeVtxID, i);
			normalIndices.insert(edgeVtxID, i);
		}
	}
	else
	{//gegen face Richtung: l anpassen
		l++;
		if(isFlipped)
		{
			if(helper.getLocIndexMod2(faceVertices[(i+1)%l], normalIndices, i))	//->ob das hier korrekt ist muss noch getestet werden!!!!
			{
				i = (i+1) % normalIndices.length();
				
				normalIndices.insert(edgeVtxID, i);
				normalIndices.insert(edgeVtxID, i);
			}

		}
		else
		{
			if(helper.getLocIndexMod2(faceVertices[(i+l-1)%l], normalIndices, i))
			{
				i = (i+1) % normalIndices.length();
				
				normalIndices.insert(edgeVtxID, i);
				normalIndices.insert(edgeVtxID, i);
			}
		}


	}


	//jetzt noch die unter umständen aktualisierten startEndIDs zurückgeben
	MIntArray returnValue(2);
	returnValue[0] = startID;
	returnValue[1] = endID;

	return returnValue;
}




//-----------------------------------------------------------------------------------
void			edgeFaceData::insertEdgeVtx(int startID, int edgeVtxID, bool inFaceRichtung)
//-----------------------------------------------------------------------------------
{

	//MERKE: Diese prozedur verfügt nur über minimale fehlerkorrektur

//VTX BEARBEITEN

	UINT l = faceVertices.length();
	
	//jetzt noch die startID finden und den edgeVtx einfach einfügen
	
	
	UINT i;
	for(i = 0; i < l; i++)
	{
		if(faceVertices[i] == startID)
		{
			//aleso den EdgeVtx an i+i einfügen
			//jetzt auch gleich UVs aktualisieren, was die Arraygroesse angeht

			if(inFaceRichtung)
			{
				
				updateUVArrays(i, 1);	//diese Prozedur immer vorher ausführen, damit l noch korrkt ist für UVIDs
				
				faceVertices.insert(edgeVtxID, (i+1) % l);
				
				
			}
			else
			{
				
				updateUVArrays(i, 0);
				
				//also gegen die FaceRichtung
				faceVertices.insert(edgeVtxID, i );
			
			}

			break;
		}

	}


	
//NORMALEN BEARBEITEN
	//jetzt um die normalen kümmern (in dem Moment, wo keine nomalen vorhanden sind, ist das array leer und es passiert eh nix)
	//hier müssen die vtx conform zur faceDir sein

	//nur jeden 2. Vtx checken, da die vertizen hier immer paarweise abgelegt sind
	if(inFaceRichtung)
	{
		if(helper.getLocIndexMod2(startID, normalIndices, i))
		{
			i = (i+1) % normalIndices.length();
			
			normalIndices.insert(edgeVtxID, i);
			normalIndices.insert(edgeVtxID, i);
		}
	}
	else
	{//gegen face Richtung: l anpassen
		l++;
		
		if(helper.getLocIndexMod2(faceVertices[(i+l-1)%l], normalIndices, i))
		{
			i = (i+1) % normalIndices.length();
			
			normalIndices.insert(edgeVtxID, i);
			normalIndices.insert(edgeVtxID, i);
		}
		


	}


}



