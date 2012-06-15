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

// nSelEdge.cpp: implementation of the nSelEdge class.
//
//////////////////////////////////////////////////////////////////////

#include "nSelEdge.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

nSelEdge::nSelEdge(int inID):edge(inID)
{

}



nSelEdge::nSelEdge(MIntArray inFaceIDs,int inID):edge(inFaceIDs,inID)
{

}

//-------------------------------------------------------
nSelEdge::~nSelEdge()
//-------------------------------------------------------
{
	//MUSS HIER ALL IHRE NACHBARN LÖSCHEN
	//MERKE: NSEL EDGES HABEN NIE PTS ZU SELEDGES; SO DASS MAN DIESE GETROST von "Oben" löschen kann

	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(nachbarn[i][a] != 0)
			{//zuerst sich selbst beim nachban 0 setzen, damit der nicht wiederum versucht, diesen hier zu löschen - feedbackschleife

				nachbarn[i][a]->setThisNachbarZero(this);
				//jetzt kann man diesen nachbarn ebenfalls löschen
				delete nachbarn[i][a];
			}
		}

	}

}


//MERKE: Die unselecteten müssen selbst darauf achten, ob ihre pointer gültig sind



//-------------------------------------------------------
edge*	nSelEdge::findNextSelEdge(edge* lastEdge)
//-------------------------------------------------------
{
//unSelEdges finden den gegenüberliegenden Ptr und rufen diese methode auf
	edge* opposite = findOppositeRichtung(lastEdge);

	//debug:
	//hier wird rekursiv gesucht nach der nächsten selEdge
	if(opposite != 0)
		return opposite->findNextSelEdge(this);
	else
		MPRINT("FEHLER IN FINDsELeDGE: OppositePtr war 0 !!!")
	
	return 0;
}

//-----------------------------------------------
void	nSelEdge::setGerade(MDoubleArray inGerade)
//-----------------------------------------------
{
	for(int i = 0; i < 6; i++)
		gerade[i] = inGerade[i];
}

//-----------------------------------------------------------------
MDoubleArray nSelEdge::getGerade(bool displace,const edge* requester,bool useUId, int uniqueID)
//-----------------------------------------------------------------
{
	return MDoubleArray(gerade,6);
}




//-------------------------------------------
int		nSelEdge::getRefVtxID(int endVtx, bool special)
//-------------------------------------------
{

	if(special)
	{
		//ptr durchsuchen nach der refEdge
		MIntArray verts;
		for(int i = 0; i < 2; i++)
		{
			for(int a = 0; a < 2; a++)
			{
				if(nachbarn[i][a] != 0)
				{
					verts = nachbarn[i][a]->getVtx();
					break;
				}
			}
		}

		int result = -1;
		if(verts[0] == endVtx)
			result = verts[1];
		else
			result = verts[0];
		
		return result;
	}
	else
	{
		//cout<<"RefVtx ist "<<result<<endl;
		
		
		//einfach den anderen Vtx der nSelEdge als RefVtx nehmen
		if(vtxIDs[0] == endVtx)
			return vtxIDs[1];
		else
			return vtxIDs[0];
	}

	INVIS(cout<<"getRefVtx: FORBIDDEN PATH"<<endl;)
	return -1;
}



//-------------------------------------------
int		nSelEdge::getRefDirection(int startVtx, int dirClue )
//-------------------------------------------
{
	//dirClue gibt das Face an, welches als referenz für die direction heranzuziehen ist


	//nun anhand der des refVtx die richtung ermitteln
	UINT a;
	for( a = 0; a < faceIDs.length(); a++)
		if(faceIDs[a] == dirClue)
			break;

	if(startVtx == vtxIDs[1])
		return (edge::isFlipped[a]) ? 1 : -1;
	else
		return (edge::isFlipped[a]) ? -1 : 1;

	
}


//--------------------------------------------------------------------------------
void			nSelEdge::getMaxRecurseCount(UINT* zeroNumber, const edge* startPtr) const
//--------------------------------------------------------------------------------
{
	for(UINT i = 0; i < 2; i++)
	{
		for(UINT a = 0; a < faceIDs.length(); a++)
		{
			if(nachbarn[i][a] != 0 && nachbarn[i][a] != startPtr)
			{
				*zeroNumber += 1;
				(static_cast<nSelEdge*>(nachbarn[i][a]))->getMaxRecurseCount(zeroNumber, this);
				return;
			}

		}
	}


}


//--------------------------------------------------------------------------------
void		nSelEdge::changeFace(int origVtx, int newVtx,const edgeFaceData* faceData, UINT recurseCount )
//--------------------------------------------------------------------------------
{
	//UVChanges hat X * 2 Einträge, X = numUVSets, nX+0 = origUVID, nX+1 = newUVID
	
	//newVtxExisted: true, wenn dies der 2. durchlauf ist. newVtx muss dann neigentlich ncht nochmal gesetzt werden, allerdings muss man dann die UVs neu berechnen
	//und mit den bereits vorhandenen UVs Mitteln

	if(recurseCount == 0 )//&& faceIDs.length() == 2)	//wenn es eine borderEdge ist, dann muss sie nicht abbrechen - 
		return;

	--recurseCount;	//auf diese Weise kann diese Variable nicht negativ (und als UINT), riesengroß werden

	UINT i = 0;
	
	edgeFaceData*	thisFace = 0;

	clean = true;

	//Nur jede Richtung checken - Seite spielt keine Rolle
	
	for(i = 0; i < faceIDs.length(); i++)
	{
		//wenn nachbar dieser Seite nicht schon evaluiert wurde (und folglich face nicht schon geändert wurde), dann loslegen
		//MERKE: man muss hier nicht auf isSelected() prüfen, da diese Art edges sowieso NUR mit edges ihres Typs verbunden sind, und demnach nicht selected sind
		for(UINT a = 0; a < 2; a++)	//->hetzt die Seiten abarbeiten
		{
			if(  nachbarn[a][i] != 0 && !nachbarn[a][i]->isClean() ) 
			{

				/*
				if(faceDone[faceIDs[i]])	//faces nie zweimal bearbeiten
					return;
				else
					faceDone.setBitTrue(faceIDs[i]);
				*/

				//wenn der originalVtx nicht mehr auf dem Gegenwärtigen Face  zu finden ist, dann , dann wurde das Face schon bearbeitet und man muss in dieser richtung abbrechen
				
				
				
				//das Face, welches übergeben wurde ist ein referenzface, welches auf jeden Fall in der anderen Richtung der edge angrenzt
				//FACE AUFBAUEN
				thisFace = new edgeFaceData(faceIDs[i]);

				creator->getFaceVtxIDs(faceIDs[i], thisFace->faceVertices);

				//CHECK: OrigVtx noch drauf?
				UINT l = thisFace->faceVertices.length();
				bool found = false;
				UINT x;
				for(x = 0; x < l; x++)
				{
					if(thisFace->faceVertices[x] == origVtx)
					{
						found = true;
						break;
					}
				}

				if(!found)
				{
					//abbruch!
					delete thisFace;
					return;
				}


				MIntArray newUVs(faceData->getNewVtxUVs(newVtx));		//hält die entgültigen UVs für den newVtx, wird mit newVtxUV initialisiert

				
				//UVs
				thisFace->UVRelOffsets = (bool*)malloc(creator->numUVSets);
				thisFace->UVAbsOffsets = (unsigned long*)malloc(creator->numUVSets*4);
				
				creator->getFaceUVs(faceIDs[i], thisFace->UVIndices, thisFace->UVRelOffsets);
				
				//nun das AbsOffsetsArray aufbauen - es wird von der getEdgeUVMethode gebraucht
				UINT count = 0;
				for(x = 0; x < creator->numUVSets; x++)
				{
					thisFace->UVAbsOffsets[x] = count;
					
					count += (int)thisFace->UVRelOffsets[x] * thisFace->faceVertices.length();
				}
				
				//jetzt schauen, ob die Faces an den UVs der edge, die sie teilen (this) zusammenhängen und je nach bedarf neue UVPositionen errechnen
				
				MIntArray notShareUVs(creator->numUVSets, -1);		//hält die UVs, die geteilt werden zwischen den Faces
				found = false;								//flag, ist true, wenn mindesten ein geteilter UV vorhanden
				
				MIntArray vtxUVs[2] = {faceData->getEdgeVtxUVs(origVtx), thisFace->getEdgeVtxUVs(origVtx) };
				
				
				
				
				//jetzt die Werte pro UVSet vergleichen
				for(x = 0; x < creator->numUVSets; x++)
				{
					if(		( vtxUVs[0][x] > -1 && vtxUVs[1][x] > -1 )
						&&
						( vtxUVs[0][x] != vtxUVs[1][x]  )	)	//es reicht, wenn sie nicht am origVtx verbunden sind
					{
						found = true;
						
						notShareUVs[x] = 1;
					}
					
				}
				
				
				//Okay, jetzt die UVPositionen erstellen, wenn überhaupt ungeteilte UVs vorhanden
				if(found /*&& newVtx > creator->getInitialVtxCount()*/)	//das dürfte auch keine endgültige Lösung darstellen
				{
					MFloatArray newUVPos;
					getUVPos(creator->getVtxPoint(newVtx), thisFace, newUVPos, false);
					
					//MERKE: Diese SideFaces dürfen keinen neuenUVs erzeugen - sie dürfen wenn überhaupt die vorhanenen UVs umsetzen
					
					
					
					//die neuen UVs müssen auch gleich noch slide bekommen
					//WICHTIG: Eventuell kommt es vor, dass die vorhandenen Vtx bereits einen Slide hatten, welcher dann sozusagen überschrieben wird
					//mit einem ganz anderen Slide (ein Slide basierend auf einer UVposition relativ zu diesem Face).
					//wahrscheinlich müsste man das vorher checken und nur dann nen slide hinzufügen, wenn die UV noch keinen slide hat
					
					MFloatArray UVPos;
					
					creator->getUVPositions(vtxUVs[1], UVPos);
					
					creator->addUVSlide(newUVPos, UVPos, vtxUVs[1]);
					
					notShareUVs = vtxUVs[1] ;
				}
				
				
				//zuguterletzt durch die shareUVs parsen und alle ins entgültige UVArray eintragen
				for(x = 0; x < creator->numUVSets; x++)
				{
					if(notShareUVs[x] > -1)
					{
						newUVs[x] = notShareUVs[x];
					}
				}
				
				
				creator->changeSideFace(origVtx, newVtx, faceIDs[i], newUVs);
				
				
				//jetzt muss thisFace noch aufgerüstet werden mit vtx/ UVChange informationen, damit die nächst edge auch weiß, wonach sie suchen soll
				thisFace->vtxChange.append(origVtx);
				thisFace->vtxChange.append(newVtx);

				//jetzt die UVs aktualisieren
				MIntArray origUVs;
				origUVs = thisFace->getEdgeVtxUVs(origVtx);
	
				//entgültiges array aufbauen
				MIntArray UVIDs(creator->numUVSets * 2);
				for(x = 0; x < creator->numUVSets; x++)
				{
					UVIDs[x] = origUVs[x];
					UVIDs[x+1] = newUVs[x];
				}

				//hinzufügen
				creator->helper.addIntArrayToLHS(thisFace->UVChange, UVIDs);
				//FERTIG
				
				//der nachbar ist gültig - ihn also auch zwingen, sein unbearbeitetes face - so vorhanden - zu ändern
				nSelEdge* thisNachbar = (nSelEdge*)nachbarn[a][i];
				thisNachbar->changeFace(origVtx, newVtx, thisFace, recurseCount);

				//in der anderen Richtung (falls es sie gibt) kann nichts mehr sein, also abbrechen

				//zuguterletzt kann thisFace getrost gelöscht werden - es gibt immer nur ein thisFace, da immer nur in eine Richtung ein gültiger nachbar ist			
				delete thisFace;
				return;
				
			}
		}
	}

	

		

}

