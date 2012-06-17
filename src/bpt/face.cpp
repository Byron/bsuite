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

// face.cpp: implementation of the face class.
//
//////////////////////////////////////////////////////////////////////

#include "face.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------
face::face(MIntArray inVtx,int inID):vtx(inVtx),clean(false),id(inID)
//--------------------------------------------------------------------------------------------------------------------
{
	for(int i = 0; i < 20;i++)
	{
	//	slideDirectionsStart[i] = -1;
	//	slideDirectionsEnd[i] = -1;
		slideCodes[i] = -5;	
		isFlippedArray[i] = false;
		newVtx[i] = -1;
	}
}
//--------------------------------------------------------------------------------------------------------------------
face::face(MIntArray inVtx,MIntArray inCorners,MIntArray inNachbarIDs, int inID)
: vtx(inVtx)
, corners(inCorners)
, nachbarIDs(inNachbarIDs)
, clean(false)
, id(inID)
//--------------------------------------------------------------------------------------------------------------------
{
	for(int i = 0; i < 20;i++)
	{
	//	slideDirectionsStart[i] = -1;
	//	slideDirectionsEnd[i] = -1;
		slideCodes[i] = -5;
		isFlippedArray[i] = false;
		newVtx[i] = -1;
	}
}
//--------------------------------------------------------------------------------------------------------------------
face::~face()
//--------------------------------------------------------------------------------------------------------------------
{

}

meshCreator* face::creator = 0;


//--------------------------------------------------------------------------
bool	face::modifyType3(int& startCornerLocID,int& endCornerLocID)
//--------------------------------------------------------------------------
{
	//zuersteinmal die aloneEdge finden, welche hier durch CornerLocIDs representiert wird;

//	cout<<"!!!!Modifiziere ID -> "<<id<<" <-!!!!!!!!!!!"<<endl;

	startCornerLocID = 0;

	int nachbarCount;

	startCornerLocID = validateThisCornerLocID(0);
	endCornerLocID = findNextValidCornerLocID(startCornerLocID);
	nachbarCount = getNumCornerEdgeNachbarn(startCornerLocID,getCornerEdgeVtxCount(startCornerLocID,endCornerLocID));

	//wenn nachbarCount == 0, dann ist die gegenüberliegende die aloneEdge
	if(nachbarCount == 0)
	{
		findOppositeCorner(endCornerLocID,2,startCornerLocID,endCornerLocID);
	}
	else
	{//wenn nachbarCount !=0, dann checken, ob gegenüberliegende Edge nachbarn hat
		int newStartCornerLocID,newEndCornerLocID;
		findOppositeCorner(endCornerLocID,2,newStartCornerLocID,newEndCornerLocID);
		nachbarCount = getNumCornerEdgeNachbarn(newStartCornerLocID,getCornerEdgeVtxCount(newStartCornerLocID,newEndCornerLocID));

		//wenn nein, dann ist Ursprungsedge die richtige gewesen, ansonsten muss gecheckt werden , welche der anderen edges korrekt ist
		if(nachbarCount != 0)
		{
			//wenn cornerEdge nach newStartCornerLocID nachbarn hat, dann muss sie die aloneEdge sein ...
			int nextStartCornerLocID = newEndCornerLocID,nextEndCornerLocID;
			nextEndCornerLocID = findNextValidCornerLocID(nextStartCornerLocID);
			nachbarCount = getNumCornerEdgeNachbarn(nextStartCornerLocID,getCornerEdgeVtxCount(nextStartCornerLocID,nextEndCornerLocID));
				
			if(nachbarCount != 0)
			{
				startCornerLocID = nextStartCornerLocID;
				endCornerLocID = nextEndCornerLocID;
			
			}
			else
			{//in diesem Fall muss die CornerEdge nach originalStartCornerEdge die richtige sein
				startCornerLocID = endCornerLocID;
				endCornerLocID = findNextValidCornerLocID(startCornerLocID);
				
			}
		}
	}

//	cout<<"Start/EndCornerLocID = "<<vtx[startCornerLocID]<<" - "<<vtx[endCornerLocID]<<endl;

	//jetzt durch die Nachbarn parsen und Typ Checken
	MIntArray tmp,nLocIDs;
	
	nLocIDs = getCornerEdgeNachbarIDs(tmp,vtx[startCornerLocID],getCornerEdgeVtxCount(startCornerLocID,endCornerLocID)-1);
	//NICHT VERGESSEN: nLocIDs sind CornerEdgeRelative!!!

	
	//printArray(nLocIDs," = nLocIDs");
	for(unsigned int i = 0; i < nLocIDs.length();i++)
	{
		if(nachbarn[nLocIDs[i]]->whichType() == 3)
		{//checken, ob gemeinsame cEdge die aloneEdge vom gefundenen Typ3 Face ist
			//dData nur der einfachheit halber
			int startAbs = vtx[startCornerLocID],endAbs = vtx[endCornerLocID];
			dData data(startCornerLocID,endCornerLocID,startCornerLocID,endCornerLocID,startAbs,endAbs,startAbs,endAbs,3,id,nachbarIDs[nLocIDs[i]]);
			data.isFlipped = false;

			if(nachbarn[nLocIDs[i]]->modifyType3EdgeCheck(data))
			{
				return true;
			}

		}
	}

	return false;
	
}

//------------------------------------
void	face::convertType3ToType2(int startCornerLocID,int endCornerLocID)
//------------------------------------
{

	MIntArray tmp;

	MIntArray nLocIDs = getCornerEdgeNachbarIDs(tmp,vtx[startCornerLocID],getCornerEdgeVtxCount(startCornerLocID,endCornerLocID)-1);

			//	cout<<"WILL FACE VERÄNDERN ZU TYP "<<endl;
				
				type = 2;
				isControlTwo = false;
				
				//nachbarPtrs noch loeschen


				UINT a;
				for(a = 0; a < nLocIDs.length();a++)
				{
					nachbarn[nLocIDs[a]] = 0;
				}

				
				//jetzt das nachbarIDs array richtig aufbauen
				

				//nachbarIDs neu aufbauen zu testzwecken
				//MIntArray nachbarIDsTmp = nachbarIDs;

				

				int nl = nachbarIDs.length();
				for( a = 0; a < nLocIDs.length();a++)
				{
					int start;
					int end;
					//wenn vorheriges feld das ende des arrays ist, dann dieses feld -1 setzen und mit allen feldern weiter rechts
					//fortfahrem
					if((nLocIDs[a]+nl-1)%nl != nl - 1)
					{
						//letztes feld ist ID, dann muss dieses feld mit -888888 gesetzt werden, ansonsten ursprung des
						//counts zurückverfolgen.1. Wenn er -888888 ist, dann checken, wie viel Vtx dieser nachbar hat und count neu
						//machen;
						//2. Wenn er bis zum anfang reicht, dann dieses feld und alle folgenden(bis zum nächsten gültigen
						//vtx) felder entsprechend numerieren
						//3. wenn es ID ist, dann natürlich -888888 setzen 
						//4. wenn es -888888 ist, dann jetziges feld -1 setzen
						
						//ursprung des counts zurückverfolgen, wenn moeglich
						
						for(int x = nLocIDs[a] - 1;;x--)
						{
							//wenn x wirklich kleiner null, dann count bis zum ursprung zurückverfolgen
							if( nachbarIDs[x] >= 0) 
							{
								//end muss bis vor ende des endVtx des gegenwärtigen nachbarn gehen
								start = x+1;
								end = nLocIDs[a];	//endbound zeigt GENAU auf gegenwärtigen Vtx, endet aber einen davor

								nachbarIDs[nLocIDs[a]] = -888888;
								
								if( !(nLocIDs[start] == -888888 || nLocIDs[start] == -888888) )
									refillNachbarArray(start,end);

								break;
							}
							
							if(nachbarIDs[x] == -888888)
							{
								start = x+1;
								end = findLastNachbarVtx(nLocIDs[a]);

								if(nachbarIDs[end] == -888888)
								{
									end = findLastNachbarVtx(end);
								}

								refillNachbarArray(start,end);
								break;
							}
							
							if( x == 0)
							{
								start = x;

								end = findLastNachbarVtx(nLocIDs[a]);

								if(nachbarIDs[end] == -888888)
								{
									end = findLastNachbarVtx(end);
								}

								refillNachbarArray(start,end);

								break;
							}
							
						}
							

					}
					else
					{
						//hier zukunft bearbeiten
						
						start = nLocIDs[a];
						end = findLastNachbarVtx(nLocIDs[a]);
						
						if(nachbarIDs[end] == -888888)
						{
							end = findLastNachbarVtx(end);
						}
						
						refillNachbarArray(start,end);

					}

					//jetzt alles bearbeiten was nach gegenwärtigem feld kommt

				}

				

		//		printArray(nachbarIDs," = nachbarIDs gesamt");
				
}

//---------------------------------------------------
void	face::refillNachbarArray(int start,int end)
//---------------------------------------------------
{

//	cout<<"Start/EndID = "<<nachbarIDs[start]<<" - "<<nachbarTmpIDs[end]<<endl;

	int d = start + (end - start);
	int count = d;

	for(int i = start; i < d; i++)
	{
		nachbarIDs[i] = -count--;
	}
}
//---------------------------------------------------
bool	face::modifyType3EdgeCheck(dData& data)
//------------------------------------------------
{
	int tmp = 0;
	dData locData = convertData(data,tmp);

//	cout<<"Checke type3Edge, ID ist "<<id<<endl;
	
	//wenn gegenüberliegende CornerEdge keine nachbarn enthält, dann true zurückgeben, ansonsten false
	int start,end;
	
	findOppositeCorner(locData.endLocCorner,2,start,end);
	int nachbarCount = getNumCornerEdgeNachbarn(start,getCornerEdgeVtxCount(start,end));

//	cout<<"Start/EndCornerLocID der Opposite = "<<vtx[start]<<" - "<<vtx[end]<<endl;
//	cout<<"NachbarCount ist: "<<nachbarCount<<endl;

	if(nachbarCount == 0)
		return true;
	else
		return false;
}


//----------------------------------------------------------
bool	face::engage()
//----------------------------------------------------------
{
	
	//pro Typ eine Verhaltensweise implementieren
	switch(type)
	{
	case 4:	//solche Faces dürfen auf keinen Fall nen Slidewert erhalten und blockieren anfragen
		{
			//cout<<"ENGAGING TYPE 4: "<<"START ID: "<<id<<endl;
			
			//gleich daran machen, alles zu splitten
			splitType4();

			clean = true;
			return clean;
			break;
		}
	case 3:
		{
			//cout<<"ENGAGING TYPE 4: "<<"START ID: "<<id<<endl;		
			
			splitType3();

			clean = true;
			return clean;
			break;
		}
	case 2:
		{
			if(isControlTwo)
			{
				//cout<<"ENGAGING TYPE 2, CONTROL: "<<"START ID: "<<id<<endl;
				splitControlTwo();
			}
			else
			{
				//cout<<"ENGAGING TYPE 2: "<<"START ID: "<<id<<endl;
				splitNormalTwo();
			}
			clean = true;
			return clean;
			break;
		}
	case 1:
		{
			//<<cout<<"ENGAGING TYPE 1: START ID: "<<id<<endl;

			SplitTypeOne();

			clean = true;
			return clean;
			break;
		}
	case 0:
		{
		//	cout<<"ENGAGING TYPE 0: "<<"START ID: "<<id<<endl;
			pokeFace();
			
			clean = true;
			return clean;
			break;
		}
	case -1:
		{
			//Hier extraProcedur machen für Faces Ohne cornerEdges oder mit einer CornerEdge
		//	cout<<"ENGAGING TYPE MINUS ONE: "<<"START ID: "<<id<<endl;
			splitTypeMinusOne();

			clean = true;
			return clean;
			break;
		}
	}

	//forbidden path
	cout<<"FACE:ENGAGE: FORBIDDEN PATH"<<endl;
	return clean;
}


//-------------------------------
void	face::splitTypeMinusOne()
//-------------------------------
{

	//FUNKTIONSWEISE: Face wird datentechnisch zu face Typ 4 gemacht und dann bearbeitet

	//erstmal werden die CornerEdges geloescht, für den Fall
	//printArray(corners," = corners");

	corners.clear();

	//jetzt werden 2 CornerEdges erstellt aus vorhandenen Vtx

	corners.append(vtx[0]);
	unsigned ID = (vtx.length() - (vtx.length() % 2)) / 2;

	//zwischenräume erstellen
	unsigned i;
	for(i = 1; i < ID;i++)
		corners.append( - static_cast<int>((ID - i)));

	corners.append(vtx[ID]);
	
	numCorners = 2;
	type = 4;

	//printArray(corners," = corners bearbeitet");

	//jetzt die üblichen Procs aufrufen
	
	MIntArray processedVtx, newVtxCount(4,0),cornerData;


	//erstmal Vtx bearbeiten
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner
	


	INVIS(printArray(processedVtx," = PROCESSEDVtx");)
	INVIS(printArray(cornerData," = CornerDATA");)
	

	//nun VtxArray erstellen
	MIntArray	newVtxOrder;
	MIntArray	begleiter;		//synchron zu vtxPerCorner[n],speichert info, ob vtx neu oder nicht
	MIntArray	vtxPerCorner[4];	//enthält alle 
	
	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;
	
	for(i = 0; i < numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		INVIS(printArray(vtxPerCorner[i]," = VtxPERCorner");)
	}



	unsigned first,second,third,last;
	first = 0;second = 1;third = 1;last = 1;

	int centerVtx;
	centerVtx = creator->createCenterVtx(id);


	creator->createUV(id,creator->getLastVtxID(), newUVs);
	

	
	creator->createNormal(id,creator->getLastVtxID());
	


	creator->addNormalSlide(centerVtx,id);
	//split genau wie Face 4
	//cout<<"Erstelle Faces"<<endl;
	//StartFace verändern
	newVtxOrder.append(vtxPerCorner[first][0]);
	newVtxOrder.append(vtxPerCorner[first][1]);
	newVtxOrder.append(centerVtx);
	newVtxOrder.append(vtxPerCorner[last][vtxPerCorner[last].length()-2]);
	
	int lBound = 1;

	
	creator->changePolyVtxIDs(id,newVtxOrder);


	creator->changeUVIds(id,newUVs,newVtxOrder,vtx);

	
	creator->changeNormals(id,newVtxOrder,false,true);

	



	//letzten Vtx von [3] runterschmeissen, damit kein Face doppelt erstellt wird
	vtxPerCorner[last].remove(vtxPerCorner[last].length()-1);
	newVtxOrder.clear();
	
	//EndFace
	newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-2]);
	newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-1]);
	newVtxOrder.append(vtxPerCorner[second][1]);
	newVtxOrder.append(centerVtx);
	
	creator->createPoly(newVtxOrder);


		creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);
	

		creator->changeNormals(id,newVtxOrder,true, true);
	

	int rBound = vtxPerCorner[first].length()-2;

	vtxPerCorner[second].remove(0);

	
	//Jetzt Faces zwischen den Bounds erstllen, falls noetig
	if(rBound-lBound > 0)
		createInnerCornerPolys(lBound,rBound,centerVtx,vtxPerCorner[first],newVtxOrder);
	
	if(numCorners != 2)
	{
		for(i = first + 1; i <= last;i++)
		{
			newVtxOrder.clear();
			//cout<<"WAR IN FOR_SCHLEIFEdd, i == "<<i<<endl;
			//hier werden die mittleren CornerEdges bearbeitet
			//zuerst das abschlussQuad erstellen und entsprechende 
			//printArray(vtxPerCorner[i]," = Vtx in schleife");
			if(i != last)
			{
				newVtxOrder.append(vtxPerCorner[i][vtxPerCorner[i].length()-2]);
				newVtxOrder.append(vtxPerCorner[i][vtxPerCorner[i].length()-1]);
				newVtxOrder.append(vtxPerCorner[i+1][1]);
				newVtxOrder.append(centerVtx);
				
				vtxPerCorner[i+1].remove(0);
				
				creator->createPoly(newVtxOrder);


					creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);

					creator->changeNormals(id,newVtxOrder,true, true);

				lBound = 0;			rBound = vtxPerCorner[i].length()-2;
			}
			else
			{
				lBound = 0;			rBound = vtxPerCorner[i].length()-1;	//ob minus 1 oder minus 0 ist hier eigentlich egal
			}
			
			//jetzt die übrigen Faces Splitten;
			if(rBound-lBound > 0)
				createInnerCornerPolys(lBound,rBound,centerVtx,vtxPerCorner[i],newVtxOrder);
			//else
				//cout<<"BOUNDSECTION IST NULL!!!!"<<endl;
			
		}
	}
	else
	//wenn corner two, etwas nachhelfen
	//if(numCorners == 2)
		createInnerCornerPolys(0,vtxPerCorner[last].length()-1,centerVtx,vtxPerCorner[last],newVtxOrder);
		
	


}

//-------------------------------
void	face::splitNormalTwo()
//-------------------------------
{
	MIntArray processedVtx, newVtxCount(4,0),cornerData;


	//erstmal Vtx bearbeiten
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner
	


	////printArray(processedVtx," = PROCESSEDVtx");
	////printArray(cornerData," = CornerDATA");
	

	//nun VtxArray erstellen
	MIntArray	vtxPerCorner[4];	//enthält alle 
	
	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;
	
	
	unsigned i;
	for(i = 0; i < numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		//printArray(vtxPerCorner[i]," = VtxPERCorner");	
	}




	//Nun kommt der eigentliche SplitAlgo. Er muss auch mit cornerCounts <> 4 klarkommen
	int startCornerEdgeID = -1,endCornerEdgeID = -1;
	int origLBound,origRBound,		lBound,rBound;
	origLBound = origRBound = lBound = rBound = -1;
	//1. herausfinden, welche der corners nachbarn haben, ids des VtxPerCornerArrays speichern
	for(i = 0; i < numCorners;i++)
	{
		//funzt nur, wenn wirklich nur 2 nachbarCornerEdges vorhanden, was hier als gegeben gilt
		if(newVtxCount[i] != 0)
		{
			if(startCornerEdgeID == -1)
				startCornerEdgeID = i;
			else
				endCornerEdgeID = i;
		}
	}



	//printArray(newVtxCount," = newVtxCount");

	//cout<<"SPLIT_NORMAL_TWO: "<<startCornerEdgeID<<" = startCornerEdgeID"<<endl;
	//cout<<"SPLIT_NORMAL_TWO: "<<endCornerEdgeID<<" = endCornerEdgeID"<<endl;

	//2. Durch CornerEdges steppen und origBounds bzw Bounds L und R (LocalID) setzen, wenn Vtx gefunden
	setBounds(vtxPerCorner[startCornerEdgeID],newVtxCount[startCornerEdgeID],origLBound,origRBound);
	setBounds(vtxPerCorner[endCornerEdgeID],newVtxCount[endCornerEdgeID],lBound,rBound);


	//cout<<"SPLIT_NORMAL_TWO: "<<origLBound<<" - "<<origRBound<<" = origBounds"<<endl;
	//cout<<"SPLIT_NORMAL_TWO: "<<lBound<<" - "<<rBound<<" = Bounds"<<endl;

	//dann faces erstellen
	//Change dürfte nur 0 oder +1 sein, wenn nicht, dann abruch
	int change = endCornerEdgeID - (startCornerEdgeID+1);

	//cout<<"SPLIT_NORMAL_TWO: "<<change<<" = numCornerEdges dazwischen"<<endl;

	bool doIt = true;

	if(change < 0 )//|| change > 1)
	{
		//cout<<"SPLIT_NORMAL_TWO: "<<"ERROR: CHANGE war "<<change<<" BRECHE SPLIT FUER DIESES FACE AB"<<endl;
		doIt = false;
	}
	if(doIt)
	{
		if(numCorners != 3)
		{
			createType2Corner4Faces(origLBound,origRBound,lBound,rBound,vtxPerCorner[startCornerEdgeID],vtxPerCorner[(startCornerEdgeID + numCorners - 1)%numCorners],
								vtxPerCorner[(endCornerEdgeID + numCorners - 1)%numCorners],vtxPerCorner[endCornerEdgeID]);

		}
		else
		{

			int lastID;

			if( (endCornerEdgeID + 1)%numCorners != (unsigned)startCornerEdgeID )
			{
				lastID = (endCornerEdgeID + 1)%numCorners;
				
				//alles vertauschen
				createType2Corner3Faces(lBound,rBound,origLBound,origRBound,vtxPerCorner[endCornerEdgeID],vtxPerCorner[startCornerEdgeID],vtxPerCorner[lastID]);
				
			}
			else
			{
				lastID = endCornerEdgeID - 1;


				createType2Corner3Faces(origLBound,origRBound,lBound,rBound,vtxPerCorner[startCornerEdgeID],vtxPerCorner[endCornerEdgeID],vtxPerCorner[lastID]);
	
			}
			
		}

	}


}

//-------------------------------------//-------------------------------------//-------------------------------------//-------------------------------------//-------------------------------------
void	face::createType2Corner4Faces(int origLBound,
									  int origRBound,
									  int lBound, 
									  int rBound, 
									  MIntArray& vtxPerCornerStart,
									  MIntArray& vtxPerCorner2,
									  MIntArray& vtxPerCorner3,
									  MIntArray& vtxPerCornerEnd)
//-------------------------------------//-------------------------------------//-------------------------------------//-------------------------------------//-------------------------------------
{
			//zuerst originalFace ändern
			//-------------------------ORIG_FACE_ÄNDERN--------------------------------
			MIntArray	newVtxOrder;
			
			//Wenn Edge dazwischen liegt, müssen alle ihre Vtx ausser 0 benutzt werden
			int l;
			
			
			
			l = vtxPerCorner2.length() - 1;
			//mindestens eine noNachbarCEdge liegt dazwischen
			int i;
			for(i = 0; i < l;i++)
			{
				newVtxOrder.append(vtxPerCorner2[i]);
			}
			
			
			//jetzt alle Verts dieser CEdge richtung 0 (ausser 0 selbst) von lBound aus hinzufügen
			for(i = 0; i <= origLBound; i++)
			{
				newVtxOrder.append(vtxPerCornerStart[i]);
			}
			
			//Jetzt noch alle Vtx von l-1 bis rBound anwählen
			l = vtxPerCornerEnd.length() - 1;
			for(i = rBound; i < l; i++)
			{
				newVtxOrder.append(vtxPerCornerEnd[i]);
			}
			



			creator->changePolyVtxIDs(id,newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


				creator->changeNormals(id,newVtxOrder);
			
			//-------------------------ORIG_FACE_ÄNDERN_ENDE--------------------------------

			newVtxOrder.clear();	

			//-------------------------FACE_2_ERSTELLEN--------------------------------
			
			
			//changeWert einfach weiter nutzen
			
			l = vtxPerCorner3.length() - 1;
			//mindestens eine noNachbarCEdge liegt dazwischen
			
			for(i = 0; i < l;i++)
			{
				newVtxOrder.append(vtxPerCorner3[i]);
			}
			
			
			for(i = 0; i <= lBound; i++)
			{
				newVtxOrder.append(vtxPerCornerEnd[i]);
			}
			
			
			l = vtxPerCornerStart.length() - 1;
			for(i = origRBound; i < l;i++)
			{
				newVtxOrder.append(vtxPerCornerStart[i]);
			}
			

			
			creator->createPoly(newVtxOrder);

				creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true);

			//-------------------------FACE_2_ERSTELLEN_ENDE--------------------------------
			
			
			newVtxOrder.clear();
			
			//-------------------------RESTLICHE FACES ZWISCHEN BOUNDS ERSTELLEN------------
			
			createFacesBetweenBounds(origLBound,origRBound,lBound,rBound,vtxPerCornerStart,vtxPerCornerEnd);


}
//-------------------------------------
void		face::SplitTypeOne()
//-------------------------------------
{
	
	MIntArray processedVtx, newVtxCount(4,0),cornerData;
	
	
	//erstmal Vtx bearbeiten
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner
	
	
	
	//rintArray(processedVtx," = PROCESSEDVtx");
	//printArray(cornerData," = CornerDATA");
	
	
	//nun VtxArray erstellen
	MIntArray	newVtxOrder;
	MIntArray	begleiter;		//synchron zu vtxPerCorner[n],speichert info, ob vtx neu oder nicht
	MIntArray	vtxPerCorner[4];	//enthält alle 
	
	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;

	//erstmal Array erstllen für allre CEdges
	unsigned i;
	for(i = 0; i < numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		//printArray(vtxPerCorner[i]," = VtxPERCorner");	
	}


	
	
	int startCornerEdgeID = -1,endCornerEdgeID = -1;

	//1. einzigste CornerEdge mit nachbarn finden und ID speichern
	for(i = 0; i < numCorners;i++)
	{
		//funzt nur, wenn wirklich nur 2 nachbarCornerEdges vorhanden, was hier als gegeben gilt
		if(newVtxCount[i] != 0)
		{
			if(startCornerEdgeID == -1)
			{
				startCornerEdgeID = i;
				break;
			}
		}
	}


	//Nun anhand des CornerCounts entsprechende lastEdge finden
	switch(numCorners)
	{
	case 4: {endCornerEdgeID = (startCornerEdgeID + 2)%numCorners;break;}
	case 3:
		{

				MIntArray tmp[2];
				tmp[0] = vtxPerCorner[startCornerEdgeID];
				
				
				unsigned  ID = (startCornerEdgeID + 1)%3;
				unsigned l = vtxPerCorner[ID].length()-1;
				
				for(i = 0; i < l; i++)
					tmp[1].append(vtxPerCorner[ID][i]);
				
				ID = (startCornerEdgeID + 2)%3;
				l = vtxPerCorner[ID].length();
				
				for(i = 0; i < l; i++)
					tmp[1].append(vtxPerCorner[ID][i]);
				
				vtxPerCorner[0] = tmp[0];
				vtxPerCorner[1] = tmp[1];
				vtxPerCorner[2].clear();
				vtxPerCorner[3].clear();
				
				//VtxPerCorner ist nun bereit, jetzt noch cornerData anpassen, es werden nur aktuelle Längen benoetigt
				cornerData[2] = vtxPerCorner[0].length();
				cornerData[4] = vtxPerCorner[1].length();
				numCorners = 2;
				
				newVtxCount[0] = newVtxCount[startCornerEdgeID];
				startCornerEdgeID = 0;
				endCornerEdgeID = 1;
	
			break;
		}

	case 2:
		{
			endCornerEdgeID = 1-startCornerEdgeID;
		}
	
	}
	
	/*		//Hat fehler verursacht - dehalb ausgeklammert
	//jetzt checken, ob EndCornerEdge Vtx auf Boundary enthält, wenn ja, dann Typ4 Face erstellen 
	//(und hoffen, das alles glatt geht :) )
	MIntArray	boundVtx;
	creator->filterBoundaryVtx(vtxPerCorner[endCornerEdgeID],boundVtx);

	if(boundVtx.length() != 0 && numCorners == 2)
	{
		MIntArray tmpVtx[4];
		
		//cout<<"erstelle typ 4 face"<<endl;
		tmpVtx[startCornerEdgeID] = vtxPerCorner[startCornerEdgeID];

		//die cornerEdge bis zum ersten Eintrag in boundVtx finden
		
		int newID = (startCornerEdgeID + 1)%4;
		int l = vtxPerCorner[endCornerEdgeID].length();
		for(i = 0; i < l; i++)
		{
			if(vtxPerCorner[endCornerEdgeID][i] != boundVtx[1])
				tmpVtx[newID].append(vtxPerCorner[endCornerEdgeID][i]);
			else
				break;
		}
		
		newID = (newID + 1)%4;
		tmpVtx[newID] = boundVtx;

		//jetzt letztes Array eintragen vom Letzten BoundEintrag bis inkl. l
		newID = (newID + 1)%4;
		int offset,lastBoundVtx = boundVtx[boundVtx.length()-1];
		//erstmal offset finden
		for(i = 0; i < l;i++)
		{
			if(vtxPerCorner[endCornerEdgeID][i] == lastBoundVtx)
			{
				offset = i;
				break;
			}
		}
		//jetzt von offset bis ende alles in tmpVtx[newID] eintragen
		for(i = offset;i < l;i++)
			tmpVtx[newID].append(vtxPerCorner[endCornerEdgeID][i]);

		//zum schluss noch alle vtxPerCorners aktualisieren
		for(i = 0; i < 4;i++)
			vtxPerCorner[i] = tmpVtx[i];

		numCorners = 4;
		endCornerEdgeID = (startCornerEdgeID + 2)%numCorners;
		//cornerData wird eigentlich nicht benoetigt

	}
	*/


//	if(vtx.length() == 4)
	//		for(i = 0; i < numCorners;i++)
		//		printArray(vtxPerCorner[i]," = vtxPerCorner i");

		//	printArray(vtxPerCorner[endCornerEdgeID]," = VtxPerCorner END");

	//jetzt die Ids der angrenzenden Nachbarn finden
	MIntArray	nachbarSave;
	getCornerEdgeNachbarIDs(nachbarSave,vtxPerCorner[startCornerEdgeID][0],vtxPerCorner[startCornerEdgeID].length() );
	//cout<<"HABE CORNEREDGE NACHBAR IDS GEHOLT"<<endl;
	//printArray(nachbarSave," = nachbarSave");

	//jetzt vtxPerCorner[EndCornerEdgeID] von Creator beabeiten lassen: Creator erstellt neue Vtx OHNE normalSlide
	//und changed die VtxNumerierung der nachbarFaces. Fügt neue Vtx in vPC Array ein und gibt lokale indices (array)
	//der neuen Vtx zurück zur verwendung in eigener Bounds procedur.
	
	int origLBound,origRBound,		lBound,rBound;
	origLBound = origRBound = lBound = rBound = -1;
	
	MIntArray	newVtxIndices;
	
	//cout<<"Bin vor creator"<<endl;
	//other nachbar ist -1, wenn face on border liegt
	int otherNachbar = -1;
	MIntArray otherNachbarUVs;
	int normalFlag = creator->doFaceOneSplit(id,vtxPerCorner[endCornerEdgeID],newVtxIndices,nachbarSave,otherNachbar,otherNachbarUVs);
	

	lBound = newVtxIndices[0];
	rBound = newVtxIndices[newVtxIndices.length() - 1];	
	
	

//	printArray(vtxPerCorner[startCornerEdgeID]," = VtxPerCorner[start]");
//	printArray(vtxPerCorner[endCornerEdgeID]," = bearbeitete VtxPerCorner");
//	printArray(newVtxIndices," = locID der neuen Vtx, sortet");

	//nun die eigentliche SplitArbeit machen, entsprechend des cornerCounts
	
	//jetzt Slide zu neuem Vtx hinzufügen
	//erst datenObject aufbauen, alles ausser start/endAbsIDs und nachbarID egal
	int middleID = (vtxPerCorner[endCornerEdgeID].length() - 1)/2;
	int fakeNachbarID = 0;
	//locID von erstem nachbarn finden

	unsigned l = nachbarIDs.length();
	for(i = 0; i < l;i++)
	{
		if(!(nachbarIDs[i] < 0))
		{
			fakeNachbarID = i;
			break;
		}
	}

//	cout<<"FakeNachbarID (LOC) = "<<fakeNachbarID<<" und ABS = "<<nachbarIDs[fakeNachbarID]<<endl;
	dData data(0,1,0,1,vtxPerCorner[endCornerEdgeID][middleID-1],vtxPerCorner[endCornerEdgeID][middleID+1],
				0,1,1,id,fakeNachbarID);

	//wenn returnCode 1 ist, dann slide erstellen
	//wenn dreiecke dabei sind, dürfen diese keinen slide erhalten (erstmal), da die slides dann fehlerhaft sind
	//Allderdings ist das nicht die eleganteste Loesung, da die Ursache füra problem ja in den Daten liegt
	//allerdings funzt das hier ganz gut :)
	if(isDirectionStored(data) == 1 && vtx.length() != 3)
	{
		creator->addSlide(vtxPerCorner[endCornerEdgeID][middleID],data.endAbsID,data.startAbsID,id,normalFlag);
	
		


			//da nur die abs IDs mit sicherheit korrekt sind, müssen jetzt anhand der abs IDs die relativen IDs bestimmt werden
			int correctStart = 0, correctEnd = 0;
			
			unsigned int l = vtx.length();
			UINT x;
			for(x = 0; x < l; x++)
			{		
				if(vtx[x] == data.startAbsID)
				{
					correctStart = x;
					break;
				}
			}
			for(x = 0; x < l; x++)
			{		
				if(vtx[x] == data.endAbsID)
				{
					correctEnd = x;
					break;
				}
			}
			
			
			
			if(otherNachbar < 0)
				creator->createUV(id,vtxPerCorner[endCornerEdgeID][middleID],newUVs,true,correctEnd,correctStart);
			else
			{
				creator->checkForUV(id, otherNachbar,newUVs ,otherNachbarUVs ,correctEnd,correctStart,vtxPerCorner[endCornerEdgeID][middleID],true);
			}
			
			//jetzt noch normale für neuen Vtx dieses Faces erzeugen
			
			creator->createNormal(id,vtxPerCorner[endCornerEdgeID][middleID],data.startAbsID,data.endAbsID);
			
			
	}		
	else
	{//UVs bearbeiten, aber garantiert ohne slide


			int correctStart = 0, correctEnd = 0;

			unsigned int l = vtx.length();
			UINT x;
			for(x = 0; x < l; x++)
			{		
				if(vtx[x] == data.startAbsID)
				{
					correctStart = x;
					break;
				}
			}
			for(x = 0; x < l; x++)
			{		
				if(vtx[x] == data.endAbsID)
				{
					correctEnd = x;
					break;
				}
			}



			if(otherNachbar < 0)
				creator->createUV(id,vtxPerCorner[endCornerEdgeID][middleID],newUVs,false,correctStart,correctEnd);
			else
				creator->checkForUV(id, otherNachbar,newUVs,otherNachbarUVs, correctStart,correctEnd,vtxPerCorner[endCornerEdgeID][middleID],false);

			//jetzt noch normale für neuen Vtx dieses Faces erzeugen

				creator->createNormal(id,vtxPerCorner[endCornerEdgeID][middleID],data.startAbsID,data.endAbsID);

		
	}


	//jetzt bounds finden
	//rBound = 1;
	
	//cout<<"Bin vor setBounds"<<endl;
	setBounds(vtxPerCorner[startCornerEdgeID],newVtxCount[startCornerEdgeID],origLBound,origRBound);

	//cout<<"SPLIT_NORMAL_One: "<<startCornerEdgeID<<" = startCornerEdgeID"<<endl;
	//cout<<"SPLIT_NORMAL_One: "<<endCornerEdgeID<<" = endCornerEdgeID"<<endl;

	//cout<<"SPLIT_NORMAL_One: "<<origLBound<<" - "<<origRBound<<" = origBounds"<<endl;
	//cout<<"SPLIT_NORMAL_One: "<<lBound<<" - "<<rBound<<" = Bounds"<<endl;


	if(numCorners == 4)
	{
		createType2Corner4Faces(origLBound,origRBound,lBound,rBound,vtxPerCorner[startCornerEdgeID],vtxPerCorner[(startCornerEdgeID + numCorners - 1)%numCorners],
								vtxPerCorner[(endCornerEdgeID + numCorners - 1)%numCorners],vtxPerCorner[endCornerEdgeID]);
		//Bounds finden mit neuer Routine und dann selben SplitAlgo verwenden wie für Typ 2

	}
	else
	{
		//wenn es dreieck ist, dann wird kein centerVtx erstellt, weil das dann bullshit ist :).
		if(vtx.length() > 3 || numNachbarn > 1)
		{
			int first = 0, last = 1,second = 1;
			int centerVtx = creator->createCenterVtx(id);


				creator->createUV(id,creator->getLastVtxID(), newUVs);


				creator->createNormal(id,creator->getLastVtxID());


			creator->addNormalSlide(centerVtx,id);
			//numCorners kann jetzt nur 2 sein, split durchführen mit face 4 SplitAlgo!!CenterVtx erstellen nicht vergessen!!
			//out<<"Erstelle Faces"<<endl;
			//StartFace verändern
			newVtxOrder.append(vtxPerCorner[first][0]);
			newVtxOrder.append(vtxPerCorner[first][1]);
			newVtxOrder.append(centerVtx);
			newVtxOrder.append(vtxPerCorner[last][vtxPerCorner[last].length()-2]);

			int lBound = 1;

			creator->changePolyVtxIDs(id,newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


				creator->changeNormals(id,newVtxOrder,false, true);


			//letzten Vtx von [3] runterschmeissen, damit kein Face doppelt erstellt wird
			vtxPerCorner[last].remove(vtxPerCorner[last].length()-1);
			newVtxOrder.clear();

			//EndFace
			newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-2]);
			newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-1]);
			newVtxOrder.append(vtxPerCorner[second][1]);
			newVtxOrder.append(centerVtx);

			int rBound = vtxPerCorner[first].length()-2;

			creator->createPoly(newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true, true);

			vtxPerCorner[second].remove(0);


			//Jetzt Faces zwischen den Bounds erstllen, falls noetig
			if(rBound-lBound > 0)
				createInnerCornerPolys(lBound,rBound,centerVtx,vtxPerCorner[first],newVtxOrder);

			//jetzt die übriggebliebenen Faes erstellen
			createInnerCornerPolys(0,vtxPerCorner[last].length()-1,centerVtx,vtxPerCorner[last],newVtxOrder);
		}
		else
		{
			int first = 0, last = 1,second = 1;

			
			//out<<"Erstelle Faces"<<endl;
			//StartFace verändern
			newVtxOrder.append(vtxPerCorner[first][0]);
			newVtxOrder.append(vtxPerCorner[first][1]);
			newVtxOrder.append(vtxPerCorner[last][vtxPerCorner[last].length()-2]);

			creator->changePolyVtxIDs(id,newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


				creator->changeNormals(id,newVtxOrder, false,true);


			//letzten Vtx von [3] runterschmeissen, damit kein Face doppelt erstellt wird
			vtxPerCorner[last].remove(vtxPerCorner[last].length()-1);
			newVtxOrder.clear();

			//EndFace
			newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-2]);
			newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-1]);
			newVtxOrder.append(vtxPerCorner[second][1]);

			creator->createPoly(newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true, true);

			vtxPerCorner[second].remove(0);


		}

	}

}

//-----------------------------------------------------------------
void	face::splitType3()
//-----------------------------------------------------------------
{

	MIntArray processedVtx, newVtxCount(4,0),cornerData;
	


	//erstmal Vtx bearbeiten
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner
	
	
	
	//printArray(processedVtx," = PROCESSEDVtx");
	//printArray(cornerData," = CornerDATA");
	
	
	//nun VtxArray erstellen
	MIntArray	vtxPerCorner[4];	//enthält alle 
	
	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;

	//erstmal Array erstllen für allre CEdges
	int i;
	for(i = 0; (uint)i < numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		//printArray(vtxPerCorner[i]," = VtxPERCorner");	
	}

	//Start/end/alone und freeCornerEdge herausfinden; routine  geht davon aus, dass 4 Corners vorhanden sind
	int startEdge = 0,endEdge = 0,aloneEdge = 0,freeEdge = 0;
	
	
	for(i = 0; i < 2;i++)
	{
		if( newVtxCount[ i ] != 0 )
		{
			if(newVtxCount[ i+2 ] != 0) 
			{
				startEdge = i;
				endEdge = i+2;
			}
			else
			{
				aloneEdge = i;
				freeEdge = i+2;
			}
		}
		else
		{
			aloneEdge = i+2;
			freeEdge = i;
		}
	}

	//cout<<"Edges: start/end/alone/free: "<<startEdge<<" "<<endEdge<<" "<<aloneEdge<<" "<<freeEdge<<endl;

	//jetzt den Split machen
	//zuerst nachbarIds holen
	MIntArray	nachbarSave;
	int lStart,rStart;		int lEnd,rEnd;		int lAlone,rAlone;


//	getCornerEdgeNachbarIDs(nachbarSave,vtxPerCorner[startCornerEdgeID][0],vtxPerCorner[startCornerEdgeID].length() );
	setBounds(vtxPerCorner[startEdge],newVtxCount[startEdge],lStart,rStart);
	setBounds(vtxPerCorner[endEdge],newVtxCount[endEdge],lEnd,rEnd);
	setBounds(vtxPerCorner[aloneEdge],newVtxCount[aloneEdge],lAlone,rAlone);

	//cout<<"StartBounds: "<<lStart<<" - "<<rStart<<endl; 
	//cout<<"EndBounds: "<<lEnd<<" - "<<rEnd<<endl;
	//cout<<"AloneBounds: "<<lAlone<<" - "<<rAlone<<endl;


	//jetzt das Face Splitten: Im gegensatz zu Face4 geht er immer bis zu den Bounds, was ev. nGons erzeugt, 
	//	aber wahscheinlich ist das bei diedem FaceTyp besser
	MIntArray	neu;
	MIntArray slideIDs;
	int centerVtx = creator->createCenterVtx(id);


		creator->createUV(id,creator->getLastVtxID(), newUVs);
	

		creator->createNormal(id,creator->getLastVtxID());


	//jetzt beide locIDs von SlideDirections finden, zwischen jeweiligen anfangs und endpunkten mittelStrecke erstellen

	creator->addNormalSlide(centerVtx,id);
	bool	isInverted = false;

	//erst herausfinden, ob alone/freeEdges vertauscht sind
	if( vtxPerCorner[startEdge][0] != vtxPerCorner[aloneEdge][vtxPerCorner[aloneEdge].length()-1])
		isInverted = true;

	
	//----------------------FACE 1 Start--------------------------------
	//vorhandenes Face ändern

	//cout<<"IS_INVERTED?!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<isInverted<<endl;

	if(!isInverted)
	{
		for(i = 1; i <= lStart;i++)
			neu.append(vtxPerCorner[startEdge][i]);
		
		neu.append(centerVtx);

		for(i = rAlone; i < (int)vtxPerCorner[aloneEdge].length();i++)
			neu.append(vtxPerCorner[aloneEdge][i]);
	}
	else
	{
		int l = vtxPerCorner[startEdge].length()-1;
		for(i = rStart; i < l;i++)
			neu.append(vtxPerCorner[startEdge][i]);

		for(i = 0; i <= lAlone;i++)
			neu.append(vtxPerCorner[aloneEdge][i]);

		neu.append(centerVtx);
	}

	creator->changePolyVtxIDs(id,neu);



		creator->changeUVIds(id,newUVs,neu,vtx);


		creator->changeNormals(id,neu, false, true);

	//printArray(neu," = face 1");
	
	//----------------------FACE 1 start ENDE--------------------------------
	neu.clear();
	//----------------------FACE 2 Start--------------------------------
	
	
	if(!isInverted)
	{

		int l = vtxPerCorner[startEdge].length() - 1;
		for(i = rStart; i < l;i++)
			neu.append(vtxPerCorner[startEdge][i]);
		
		l = vtxPerCorner[freeEdge].length() - 1;

		for(i = 0; i < l;i++)
			neu.append(vtxPerCorner[freeEdge][i]);

		for(i = 0; i <= lEnd;i++)
			neu.append(vtxPerCorner[endEdge][i]);
	}
	else
	{
		int l = vtxPerCorner[endEdge].length() -1;
		for(i = rEnd; i < l;i++)
			neu.append(vtxPerCorner[endEdge][i]);
		

		l = vtxPerCorner[freeEdge].length();
		for(i = 0; i < l;i++)
			neu.append(vtxPerCorner[freeEdge][i]);
		
		
		for(i = 1; i <= lStart;i++)
			neu.append(vtxPerCorner[startEdge][i]);
	}



	neu.append(centerVtx);

	creator->createPoly(neu);


		creator->changeUVIds(id,newUVs,neu,vtx,true);


		creator->changeNormals(id,neu,true, true);
	//printArray(neu," = face 2");

	//----------------------FACE 2 Start Ende--------------------------------
	

	//jetzt noch betweenBounds von StartEdge erstellen
	createInnerCornerPolys(lStart,rStart,centerVtx,vtxPerCorner[startEdge],neu);

	//----------------------END CORNER--------------------------------
	neu.clear();
	
	int l = vtxPerCorner[endEdge].length() - 1;
	if(!isInverted)
	{
		for(i = rEnd; i < l;i++)
			neu.append(vtxPerCorner[endEdge][i]);
		
		for(i = 0; i <= lAlone; i++)
			neu.append(vtxPerCorner[aloneEdge][i]);
		
		neu.append(centerVtx);
	}
	else
	{
		
		for(i = 0; i <= lEnd;i++)
			neu.append(vtxPerCorner[endEdge][i]);

		neu.append(centerVtx);
		
		l = vtxPerCorner[aloneEdge].length()-1;
		for(i = rAlone; i < l; i++)
			neu.append(vtxPerCorner[aloneEdge][i]);
	}

	

	creator->createPoly(neu);


		creator->changeUVIds(id,newUVs,neu,vtx,true);


				creator->changeNormals(id,neu,true, true);

	//printArray(neu," = face End");

	//innerCornerPolys erstellen
	createInnerCornerPolys(lEnd,rEnd,centerVtx,vtxPerCorner[endEdge],neu);

	//----------------------END CORNER ENDE--------------------------------
	//neu.clear();
	//----------------------ALONE CORNER--------------------------------
	//hier nur noch zwischen den Bounds erstellen
	createInnerCornerPolys(lAlone,rAlone,centerVtx,vtxPerCorner[aloneEdge],neu);
	//----------------------ALONE CORNER ENDE--------------------------------


	//jetzt noch Vtx auf normalSlide packen, wo angebracht
	//Ansatz: ist die aloneEdge mit typ 3 oder 4 verbunden, dann all ihre Vtx auf normalSlide packen

	MIntArray locIDs;

	locIDs = getCornerEdgeNachbarIDs(nachbarSave,vtxPerCorner[aloneEdge][0],vtxPerCorner[aloneEdge].length());

	for(i = 0; i < (int)locIDs.length();i++)
	{	
		if(nachbarn[locIDs[i]]->whichType() >= 3 && !nachbarn[locIDs[i]]->isClean())
		{
			//startVtx = locIDs[i];
			//endVtx = findLastNachbarVtx(startVtx) + 1;

			//alles relativ zur vtxPerCorner ... einfach alle der VtxPerCornerAlone edge hinzufügen
			l = vtxPerCorner[aloneEdge].length();
			//for(int a = startVtx; a < endVtx;a++)
			for(int a = 0; a < l;a++)
				creator->addNormalSlide(vtxPerCorner[aloneEdge][a],id);
		}
			
	}

}	


//-----------------------------------------------------------------//-----------------------------------------------------------------
MIntArray	face::getCornerEdgeNachbarIDs(MIntArray& nachbarSave,int FirstCornerID,int vtxPerCornerLength)
//-----------------------------------------------------------------//-----------------------------------------------------------------
{
	//Proc gibt auch lokale IDs der nachbarn zurück
	MIntArray locIDs;

	int nl = nachbarIDs.length()-1,offset = 0;
	int i;
	for(i = 0; i < nl; i++)
	{
		if(vtx[i] == FirstCornerID)
		{
			offset = i;
			break;
		}
	}
	////cout<<"GET_CORNER_EDGE_NACHBARN: "<<"OFFSET = "<<offset<<endl;
	
	int l = vtxPerCornerLength;
	for(i = 0; i < l; i++)
	{
		//hier koennte es fehler geben: wahrscheinlich muss flag am Ende abgezogen werden, damit's funzt
		if(nachbarIDs[(i+offset)%nl] >= 0)
		{
			nachbarSave.append(nachbarIDs[(i+offset)%nl]);
			locIDs.append((i+offset)%nl);
		}
	}	
	return locIDs;
	////printArray(nachbarSave," = !!!!NACHBARN von startEdgeCorner!!!!");

}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void		face::createType2Corner3Faces(int origLBound,int origRBound,int lBound, int rBound, MIntArray& vtxPerCornerStart,MIntArray& vtxPerCornerEnd,MIntArray& vtxPerCornerLast)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
			MIntArray newVtxOrder;

			
			newVtxOrder.clear();
			
			//-------------------------ORIG_FACE_ÄNDERN--------------------------------
			int l = vtxPerCornerStart.length() - 1;
			//von rB bis ende
			int i;
			for(i = origRBound; i < l; i++)
				newVtxOrder.append(vtxPerCornerStart[i]);
			
			//jetzt die nächste Edge hinzufügen
			l = vtxPerCornerLast.length()-1;
			
			for(i = 0; i < l; i++)
				newVtxOrder.append(vtxPerCornerLast[i]);
			
			//und nun noch die EndEdge hinzufügen
			for(i = 0; i <= lBound;i++)
				newVtxOrder.append(vtxPerCornerEnd[i]);
			
			//printArray(newVtxOrder," = newVtxOrder Face 1");
			
			creator->changePolyVtxIDs(id,newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx);

	
				creator->changeNormals(id,newVtxOrder);

			//-------------------------ORIG_FACE_ÄNDERN_ENDE--------------------------------
			

			//-------------------------FACE_2_ÄNDERN--------------------------------
			newVtxOrder.clear();

			l = vtxPerCornerEnd.length()-1;
			
			//die eigenen Vtx holen
			for(i = rBound; i < l; i++)
				newVtxOrder.append(vtxPerCornerEnd[i]);


			//jetzt die startEdge

			for(i = 0; i <= origLBound; i++)
				newVtxOrder.append(vtxPerCornerStart[i]);

				//printArray(newVtxOrder," = newVtxOrder Face 2");

				creator->createPoly(newVtxOrder);

					creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


					creator->changeNormals(id,newVtxOrder,true);

			//-------------------------FACE_2_ÄNDERN_FERTIG--------------------------------

			//HIER EVENTUELL NOCH DIE BOUNDS VERTAUSCHEN - ist geschehen!
			createFacesBetweenBounds(lBound,rBound,origLBound,origRBound,vtxPerCornerEnd,vtxPerCornerStart);


}

//------------------------------------------------------------------------------------------------------------------------------------------------------
void	face::createFacesBetweenBounds(int origLBound,int origRBound,int lBound, int rBound, MIntArray& vtxPerCornerStart,MIntArray& vtxPerCornerEnd)
//------------------------------------------------------------------------------------------------------------------------------------------------------
{

			int origMarker	= origLBound;			int origL	= origRBound;
			int marker		= rBound;				int l			= lBound;
			MIntArray newVtxOrder;
			int		markerDecreased = 0;
			int i;
			for(i = origMarker; i < origL; )
			{
				
				markerDecreased = 0;
				
				newVtxOrder.clear();
				
				newVtxOrder.append(vtxPerCornerStart[i]);
				
				if(i+1 <= origL)
					newVtxOrder.append(vtxPerCornerStart[++i]);
				else
					++i;

				if(i+1 <= origL && marker == l)
					newVtxOrder.append(vtxPerCornerStart[++i]);

					
				
				if(marker - 1 >= l)
				{
					newVtxOrder.append(vtxPerCornerEnd[--marker]);
					markerDecreased = 1;
				}
				
				
				newVtxOrder.append(vtxPerCornerEnd[marker + markerDecreased]);
				
				
				
				
				//printArray(newVtxOrder," = newVtxOrder Face X");
				
				creator->createPoly(newVtxOrder);

					creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


					creator->changeNormals(id,newVtxOrder,true);
			}
			
			origMarker = i;
			//jetzt dasselbe nochmal aus sicht der 2. cornerEdge
			
			//i = marker;
			//for(int a = 0; a < 1;a++)
			for(i = marker; i > l; )
			{
				
				markerDecreased = 0;
				newVtxOrder.clear();
				
				if(i-2 >= l)
				{
					newVtxOrder.append(vtxPerCornerEnd[i-2]);
					markerDecreased = 2;
				}
				
				newVtxOrder.append(vtxPerCornerEnd[i-1]);
				if(markerDecreased == 0)
					markerDecreased = 1;
				
				newVtxOrder.append(vtxPerCornerEnd[i]);
				
				
				newVtxOrder.append(vtxPerCornerStart[origMarker]);
				
				i -= markerDecreased;
				
				//auf dieser Seite kann es jetzt keinen 2. Vtx merh geben, deshalb gleich neuen Vtx auf anderer edge erstellen
				
				//printArray(newVtxOrder," = newVtxOrder Face Y");
				
				creator->createPoly(newVtxOrder);


					creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);

					creator->changeNormals(id,newVtxOrder,true);
				
			}
			


}

//------------------------------------------------------------------------
void	face::setBounds(MIntArray& vtxPerCorner, int newVtxCount, int &lBound, int &rBound)
//------------------------------------------------------------------------
{

	//int l = vtxPerCorner.length();
	int l2 = vtx.length(),tmp;
	//bool found;

	MIntArray locIDs;
	int startLocID;
	//zuerst startLocID suchen
	int i;
	for(i = 0; i < l2;i++)
	{
		if(vtx[i] == vtxPerCorner[0])
		{
			startLocID = i;
			break;
		}
	}
	
	////cout<<"SET_BOUNDS: "<<startLocID<<" = startLocID"<<endl;
	
	int nl = nachbarIDs.length();

	////cout<<"SET_BOUNDS: "<<l<<" = maxLength"<<endl;
	//i = 0;
	while(newVtxCount != 0)
	{
		if(newVtx[i%nl] != -1)
		{
			locIDs.append(i%nl);
			newVtxCount--;
		}
		i++;
	}

	////printArray(locIDs," = locIDs");
	
	l2 = locIDs.length();
	if( l2 == 1)
	{
		tmp = newVtx[locIDs[0]];
		l2 = vtxPerCorner.length();
		for(i = 0; i < l2; i++)
			if(tmp == vtxPerCorner[i])
			{
				lBound = rBound = i;
				break;
			}
	}
	else
	{

		tmp = newVtx[locIDs[0]];
		int tmp2 = newVtx[locIDs[locIDs.length() - 1]];
		l2 = vtxPerCorner.length();
		
		for(i = 0; i < l2; i++)
			if(tmp == vtxPerCorner[i])
			{
				lBound = i;
				break;
			}

		for(i = 0; i < l2; i++)
			if(tmp2 == vtxPerCorner[i])
			{
				rBound = i;
				break;
			}
		
	}

}

//-------------------------------------------
void	face::splitControlTwo()
//-------------------------------------------
{
	
	MIntArray processedVtx, newVtxCount(4,0),cornerData;
	
	
	//erstmal Vtx bearbeiten
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner
	
	
	
	//printArray(processedVtx," = PROCESSEDVtx");
	//printArray(cornerData," = CornerDATA");
	
	
	//nun VtxArray erstellen
	MIntArray	vtxPerCorner[4];	//enthält alle 
	
	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;

	//erstmal Array erstllen für allre CEdges
	int i;
	for(i = 0; i < (int)numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		//printArray(vtxPerCorner[i]," = VtxPERCorner");	
	}


	//oneEdge und twoEdge bestimmen. Sie liegen immer aneinander. Zuerst kommt one, dann twoEdge
	int oneEdge = 0, twoEdge = 0;

	for(i = 0; i < 4;i++)
	{
		if(newVtxCount[i] != 0)
		{
			if(newVtxCount[ (i + 4 - 1)%4 ] != 0)
			{
				oneEdge = (i + 4 - 1)%4;
				twoEdge = i;
				break;
			}
			else if(newVtxCount[ (i + 1)%4 ] != 0)
			{
				oneEdge = i;
				twoEdge = (i + 1)%4;
				break;
			}
		}
	}

	//cout<<"ONE / TWO_EDGE: "<<oneEdge<<" und "<<twoEdge<<endl;

	//jetzt die Bounds erstellen lassen
	int lOne,rOne;					int lTwo,rTwo;

	setBounds(vtxPerCorner[oneEdge],newVtxCount[oneEdge],lOne,rOne);
	setBounds(vtxPerCorner[twoEdge],newVtxCount[twoEdge],lTwo,rTwo);

	//cout<<"ONE_BOUNDS: "<<lOne<<" - "<<rOne<<endl;
	//cout<<"TWO_BOUNDS: "<<lTwo<<" - "<<rTwo<<endl;


	//jetzt beginnt der eigentliche Split

	//centerVtx erstellen
	int centerVtx = creator->createCenterVtx(id);



		creator->createUV(id,creator->getLastVtxID(), newUVs);
	

		creator->createNormal(id,creator->getLastVtxID());


	//creator->addNormalSlide(centerVtx,id);
	creator->addSlide(centerVtx,vtxPerCorner[oneEdge][vtxPerCorner[oneEdge].length() - 1],
						vtxPerCorner[(oneEdge + 4 - 1)%4][0],id);



		creator->addUVSlide(centerVtx,
							vtxPerCorner[oneEdge][vtxPerCorner[oneEdge].length() - 1],
							vtxPerCorner[(oneEdge + 4 - 1)%4][0],
							id,
							newUVs);


	MIntArray	neu;
	//-----------------------Face 1 --------------------------------
	int l = vtxPerCorner[oneEdge].length() - 1;
	for(i = rOne; i < l;i++)
		neu.append(vtxPerCorner[oneEdge][i]);

	for(i = 0; i <= lTwo;i++)
		neu.append(vtxPerCorner[twoEdge][i]);

	neu.append(centerVtx);

	creator->changePolyVtxIDs(id,neu);


		creator->changeUVIds(id,newUVs,neu,vtx);


		creator->changeNormals(id,neu, false, true);

	//-----------------------Face 1 --------------------------------
	neu.clear();
	//-----------------------Face 2 --------------------------------
	
	neu.append(centerVtx);

	int ID = (oneEdge + 4 - 1)%4;
	l = vtxPerCorner[ID].length()-1;
	for(i = 0; i < l;i++)
		neu.append(vtxPerCorner[ID][i]);

	for(i = 0; i <= lOne;i++)
		neu.append(vtxPerCorner[oneEdge][i]);

	creator->createPoly(neu);


		creator->changeUVIds(id,newUVs,neu,vtx,true);


			creator->changeNormals(id,neu,true, true);

	//sämtliche ZwischenStücke erstellen
	createInnerCornerPolys(lOne,rOne,centerVtx,vtxPerCorner[oneEdge],neu);		
	//-----------------------Face 2 --------------------------------
	neu.clear();
	//-----------------------RestFaces 2 --------------------------------

	neu.append(centerVtx);

	l = vtxPerCorner[twoEdge].length() - 1;
	for(i = rTwo; i < l;i++)
		neu.append(vtxPerCorner[twoEdge][i]);

	ID = (twoEdge + 1)%4;
	l = vtxPerCorner[ID].length();
	
	for(i = 0; i < l;i++)
		neu.append(vtxPerCorner[ID][i]);

	creator->createPoly(neu);


 		creator->changeUVIds(id,newUVs,neu,vtx,true);


			creator->changeNormals(id,neu,true, true);

	//zwischenfaces erstellen
	createInnerCornerPolys(lTwo,rTwo,centerVtx,vtxPerCorner[twoEdge],neu);	
}

//----------------------------------------------------------
bool	face::isClean()
//----------------------------------------------------------
{return clean;}


//----------------------------------------------------------
void	face::setNachbarn(face*	inNachbarn,int index)
//----------------------------------------------------------
{
	nachbarn[index] = inNachbarn;
}


//-------------------------
int		face::whichType()
//-------------------------
{
	//wenn es controlTwo ist, wird es Typ 5
	if(type == 2 && isControlTwo)
		return 5;
	else
		return type;
}

//----------------------------------------------------------
int		face::getType()
//----------------------------------------------------------
{
	//zuerst die Nachbarn pro Corner finden
	int l = corners.length();
	int cornerCount=0,nachbarCount = 0;
	int lastID = 0;
	MIntArray tempNCount(l,0);
	isControlTwo = false;
	int index;

	//erstmal durchs CornerArray parsen und corners zählen
	int i;
	for(i = 0; i < l; i++)
	{
		if(corners[i] >= 0)
		{
			cornerCount++;
		}
	}

	int tmpCount = cornerCount;
	i = 0;

	while(tmpCount > 0)
	{
		i = validateThisCornerLocID(i);
		lastID = findNextValidCornerLocID(i);
		
		//index = l - tmpCount;
		index = tmpCount-1;
		//CORNERDETECTION FUNZT RICHTIG!
		////cout<<"GET_TYPE: "<<i<<" = startCornerID"<<endl;
		////cout<<"GET_TYPE: "<<lastID<<" = EndCornerID"<<endl;
		
		int numCornerVtx = getCornerEdgeVtxCount(i,lastID);
		tempNCount[index] = getNumCornerEdgeNachbarn(i,numCornerVtx);

		////cout<<tempNCount<<" = NumNachbarn"<<endl;

		if(tempNCount[index] > 0)
		{
			nachbarCount++;
		}



		i = lastID;
		tmpCount--;
	}

	for(i = 0; i < cornerCount; i++)
	{
		if(tempNCount[i] != 0)
		{
			if(tempNCount[(i+1)%cornerCount] != 0)
			{
				isControlTwo = true;
				break;
			}
			else if(tempNCount[(i+cornerCount-1)%cornerCount] != 0)
			{
				isControlTwo = true;
				break;
			}
		}
	}

	//Auswertung von nachbar und cornerCount
	//cout<<"minNachbarcount: "<<nachbarCount<<endl;
	//cout<<"CornerCount: "<<cornerCount<<endl;
	
	int realNachbarCount = 0;
	//erstmal alle Nachbarn erfassen
	for(i = 0; i < 20;i++)
	{
		if(nachbarn[i] != 0)
			realNachbarCount++;
	}
	
	if(realNachbarCount == 0)
		type = 0;
	else
	{
		if(cornerCount >= 1)
		{

			//Besser ist, wenn type nach auslastungsgrad ausgerichtet ist: ist jede CornerEdge mit Mindesens 
			//1 nachbarn besetzt, isses type 4, wenn weniger, dann ergibt sich type aus numNachbarn
			
			//??????????????????
			//faces mi mehr als 4 corners einfach nich bearbeiten? JAAAAA, diese Faces werden gepoked
			//??????????????????
			if(cornerCount > 4)
				type = -1;
			else
			{
				if(cornerCount == nachbarCount)
					type = 4;
				else
				{
					//Typ 2 sollen keine Control Zweier sein, wenn sie weniger als 4 cornerEdges haben 
					//(genaugenommen werden sie dann zurecht als ControlTwo eingetstuft
					if(cornerCount < 4)
						isControlTwo = false;
					
					type = nachbarCount;
				}
			}
			
		}
		else
		{
			type = -1;		//-1 Typ blockt alle slideAnfragen, muss extra bearbeitet werden, weil navigation hier nur ohne corneredges moeglich
		}
	}
	numNachbarn = realNachbarCount;
	numCorners = cornerCount;

	//cout<<"Type von "<<id<<" == "<<type<<endl;
	//cout<<"Ist es ControlTwo: "<<isControlTwo<<endl;

	if(type == 2 && isControlTwo)
		return 5;
	else
		return type;


}

//----------------------------------------------------------
int		face::dsVtxExist(int inID)
//----------------------------------------------------------
{
			////cout<<"BIN IN DOES VtX EXIST"<<endl;
			return newVtx[findNachbarIDIndex(inID)];
	//wenn newVtx nicht vorhanden, steht an dieser stelle -1
}

//----------------------------------------------------------
void	face::setVtxExists(int inID,int vtxID)
//----------------------------------------------------------
{
			newVtx[findNachbarIDIndex(inID)] = vtxID;	
}
/*
//----------------------------------------------------------
face*	face::findOppositeNachbar(int myID)
//----------------------------------------------------------
{
	//erstmal unterscheiden zwischen typ -1 und >=0
	if(type > 0)
	{
		



	}
	else
	{




	}


}
*/



//////////////////////////////////////////////////////////////////////
// PRIVATE METHODEN
//////////////////////////////////////////////////////////////////////
//----------------------------------------------------------
int		face::findLastNachbarVtx(int firstVtxLocID)
//----------------------------------------------------------
{
	int lVtx = vtx.length();	//damit auch 0 gegeben werden kann
	int iPlusOne = nachbarIDs[(firstVtxLocID + 1)];

	if(iPlusOne < 0)
	{
		if(iPlusOne == -888888)
			return (firstVtxLocID + 1)%lVtx;
		else
			return	firstVtxLocID + 1 - iPlusOne;
	}
	else
	{	
			return firstVtxLocID + 1;
	}
}

//----------------------------------------------------------
int		face::findNachbarIDIndex(int inID)
//----------------------------------------------------------
{
	////cout<<"BIN IN FIND NACHBAR ID INDEX"<<endl;
	int l = nachbarIDs.length();

	for(int i = 0; i < l;i++)
	{
		if(nachbarIDs[i] == inID)
		{
			////cout<<"Habe "<<i<<" gefunden!"<<endl;
			return i;
		}
	}
	return -1;	//führt dazu, dass prog beim Arrayzugriff abstürzt, was aber gut ist, da er hier auf jeden fall was finden muss 
}

//----------------------------------------------------------
int		face::findVtxIndex(int inID)
//----------------------------------------------------------
{
	////cout<<"BIN IN FIND NACHBAR ID INDEX"<<endl;
	int l = vtx.length();

	for(int i = 0; i < l;i++)
	{
		if(vtx[i] == inID)
		{
			////cout<<"Habe "<<i<<" gefunden!"<<endl;
			return i;
		}
	}
	return -1;	//führt dazu, dass prog beim Arrayzugriff abstürzt, was aber gut ist, da er hier auf jeden fall was finden muss 
}


//----------------------------------------------------------
int		face::findNextValidNachbarLocID(int index)
//----------------------------------------------------------
{
	//wenn beim gegenwärtigen Index schon einer ist, wird auch der zurückgegeben
	int nl = nachbarIDs.length();
	int tmp;

	if(index < nl)
		tmp = nachbarIDs[index];
	else
	{
		tmp = nachbarIDs[0];
		index = 0;
	}
	////cout<<tmp<<"= nachbarLocId"<<endl;

	if(tmp < 0)
	{
		if(tmp == -888888)
		{	//checken, ob nächster Slot was brauchbares enthält
			int newIndex = (index + 1)%nl;
			return findNextValidNachbarLocID(newIndex);

		}
		else
			return index - tmp;
	}
	else
	{
		return index;
	}
	//sollte er irgendwie bis hier kommen, einfach abbrechen
	//cout<<"findNextValidNachbarLocID FEHLGESCHLAGEN"<<endl;
	return -1;
}

//----------------------------------------------------------
void	face::pokeFace()
//----------------------------------------------------------
{
	//geht durch die VtxList und erstellt soviele Quads wie moeglich
	int centerVtx = creator->createCenterVtx(id);


		creator->createUV(id,creator->getLastVtxID(), newUVs);


		creator->createNormal(id,creator->getLastVtxID());


	creator->addNormalSlide(centerVtx,id);

	int l = vtx.length();
	int triangles = guessNumTriangles(type,l+1); //kann maximal 1 sein oder 0
	MIntArray	newVtxOrder;

	//cout<<"TRIANGLES: "<<triangles<<endl;

	int i = 0;
	if(l > 4)
	{
		if(triangles)
		{
			newVtxOrder.append(vtx[0]);	
			newVtxOrder.append(centerVtx);
			newVtxOrder.append(vtx[l-1]);
			l-=2;
		}
		else
		{	
			//cout<<"Bin in quads"<<endl;
			newVtxOrder.append(centerVtx);
			newVtxOrder.append(vtx[l - 2]);
			newVtxOrder.append(vtx[l - 1]);
			newVtxOrder.append(vtx[0]);
			
			l-=2;
		}
		//cout<<"Bin vor creator"<<endl;
		//erstes Face erstellen
		creator->changePolyVtxIDs(id,newVtxOrder);


			creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


			creator->changeNormals(id,newVtxOrder, false, true);

		//cout<<"Bin vor schleife"<<endl;
		
		//newVtxOrder.clear();
		for(i = 0; i < l;i+=2)
		{
			newVtxOrder.clear();
			
			newVtxOrder.append(vtx[i]);
			newVtxOrder.append(vtx[i+1]);
			newVtxOrder.append(vtx[i+2]);
			newVtxOrder.append(centerVtx);
			
			creator->createPoly(newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true, true);
		}
	}
	else
	{
		newVtxOrder.append(vtx[0]);
		newVtxOrder.append(vtx[1]);
		newVtxOrder.append(centerVtx);
		
		creator->changePolyVtxIDs(id,newVtxOrder);


			creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


			creator->changeNormals(id,newVtxOrder, false, true);


		for(i = 1; i < l;i++)
		{
			newVtxOrder.clear();
			
			newVtxOrder.append(vtx[i]);
			newVtxOrder.append(vtx[(i+1)%l]);
			newVtxOrder.append(centerVtx);
			
			creator->createPoly(newVtxOrder);


				creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true, true);
		}
		
	}

}
//-----------------------------------------------
int		face::validateThisCornerLocID(int index)
//-----------------------------------------------
{
	int tmp = corners[index];

	if(tmp < 0)
	{
		return index - tmp;
	}
	else
		return index;

}

//------------------------------------------------------------------------------
int		face::findNextValidCornerLocID(int currentIndex)
//------------------------------------------------------------------------------
{
	int l = corners.length();
	
	if(l == 0)
		return 0;

	int mod = (currentIndex + 1) % l;
	int tmp = corners[mod];

	if(tmp < 0)
	{
		//if(mod == 0)
		//{
		//	int add = vtx.length() - currentIndex - 1;
		//	return (currentIndex + 1) - tmp + add;
		//}
	//	else
			return (currentIndex + 1)%l - tmp; 
	}
	else
		return (currentIndex + 1)%l;

}
//--------------------------------------------------------------------------
int		face::getCornerEdgeVtxCount(int startCID,int endCID)
//--------------------------------------------------------------------------
{//funzt wahrscheinlich nicht ganz korrekt
	
	int difference = endCID - startCID;
	
	if(difference < 0)
	{
		int l = vtx.length();
		//return	(l - startCID+1) + endCID+1-1 ;	//letztes Minus 1 nurn Test
		return	(l - startCID) + endCID+1;
		//eigentlich (dies hier verwenden, wenn formel erprobt und sicher)
		//return	abs(l - (startCID + (l - 1 - endCID)));
	}
	else
	{
		return	(endCID - startCID)+1;
	}
	
	//int l = vtx.length();
	//return	abs(l - (startCID + (l - 1 - endCID)));

}

//--------------------------------------------------------------------------
int		face::getNachbarEdgeVtxCount(int startID,int endID)
//--------------------------------------------------------------------------
{	//funzt wahrscheinlich nicht ganuz 

		int l = vtx.length();
		//return	(l - startCID+1) + endCID+1-1 ;	//letztes Minus 1 nurn Test
		return	abs(l - (startID + (l - 1 - endID)));


}


//--------------------------------------------------------------------------
int		face::getNumCornerEdgeNachbarn(int startVtxID,int cornerLength)
//--------------------------------------------------------------------------
{
	int nachbarCount = 0;
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//nachbarn am EndCornerVtx werden nicht mitgezählt, deshalb MINUS 1
	int l = startVtxID + cornerLength - 1;

	int vL = vtx.length();
	//int nL = nachbarIDs.length() - 1;	//den letzten Slot nicht werten da Flag

	for(int i = startVtxID; i < l; i++)
	{
		if( nachbarn[i%vL] > 0 )
			nachbarCount++;
	}

	////cout<<nachbarCount<<" == nachbarCount"<<endl;
	return nachbarCount;

}


//-------------------------------------------------------------------------------
void	face::findOppositeCorner(int startCornerID,int iterCount,int& newStart, int& newEnd)
//-------------------------------------------------------------------------------
{
	//iterCount sollte der Abstand zwischen gegenwärtiger und opposite Edge sein in edges:
	//typ 4: 2
	//typ 3: 1
	//und bei Typ 2 kann es die übergeordnete Proc selbst machen
	int currentCorner = 0,nextCorner = 0;
	currentCorner = startCornerID;

	for(int i = 0; i < iterCount;i++)
	{	
		nextCorner = findNextValidCornerLocID(currentCorner);
		
		if(i != iterCount-1)
			currentCorner = nextCorner;
	}

	newStart = currentCorner;
	newEnd = nextCorner;
	
}

//-------------------------------------------------
void	face::createDirections(dData& data)
//-------------------------------------------------
{
	//Type in Data bedenken: wenn Data von gleichgestelltem Face, bleibt sie unverändert erhalten und Face nimmt sie an
	//CORNERS WERDEN NICHT VERTAUSCHT: SIE FUNGIEREN ALS BOUNDARY FUER DIE DIRECTION
	
	//WENN DIE DIRECTION GEGEN DIE GENERELLE RICHTUNG DER VTX IST; DANN <<isFlipped>> WERT in <<data>> true setzen

	//Directions koennen nur von Typ 1 und typ 2 Control erzeugt werden

	switch(type)
	{
	case 1:
		{
			//case One hat die Wahl zwischen 2 Richtungen: es nimmt daher einfach die durch data vorgegebene
			//und bricht ab, nachdem es seine TypeInfo hinzugegeben hat
			data.typeInfo = 1;
			break;
		}

	case 2:
		{
			//Typ 2 ist bei Directions komplizierter: Die Richtung, die gen nachbarFreier CornerEdge zeigt, ist die richtige
			
			//ANSATZ: Wenn die nächste CornerEdge nachbarn enthält , dann ist die gegenwärtige reihenfolge umzukehren 
			//(da diese Anfangs sowieso in reihenfolge der Vtx richtung ist). umgekehrt ist dann also keine Handlung
			//auszuführen
			int nextEndCorner,numCVtx;
			

			nextEndCorner = findNextValidCornerLocID(data.endLocCorner);
			numCVtx = getCornerEdgeVtxCount(data.endLocCorner,nextEndCorner);
			int nachbarCount = getNumCornerEdgeNachbarn(data.endLocCorner,numCVtx);

	//		cout<<"Gefundene CornerIDs: "<<corners[data.endLocCorner %corners.length()]<<" "<<corners[nextEndCorner%corners.length()]<<endl;

			if(nachbarCount != 0)
			{
				//data umdrehen
				if(!data.isFlipped)
					flipData(data);

				data.typeInfo = 2;
			}
			else
			{
				if(data.isFlipped)
					flipData(data);

				data.typeInfo = 2;
			}
			//wenn keine nachbarn, dann zeigt die direction schon in richtige richtung, allerdings nur, wenn data nicht von anderem face stammt
			

		}
	}


}

//-------------------------------------------------
void	face::flipData(dData& data)
//-------------------------------------------------
{
	int tmp;
	//tauscht sämtliche Daten um ->es wird reichen, nur die Absoluten Datan der Vtx zu ändern ->mal sehen ...
	
	tmp = data.startLocID;
	data.startLocID = data.endLocID;
	data.endLocID = tmp;

	/*
	tmp = data.startLocCorner;
	data.startLocCorner = data.endLocCorner;
	data.endLocCorner = tmp;
	*/
	
	tmp = data.startAbsID;
	data.startAbsID = data.endAbsID;
	data.endAbsID = tmp;

	data.isFlipped = ((1 - int(data.isFlipped)) != 0);

	/*
	tmp = data.startAbsCorner;
	data.startAbsCorner = data.endAbsCorner;
	data.endAbsCorner = tmp;
	*/
}

//-----------------------------------------------------------
void	face::storeDirection(dData& data, int code)
//-----------------------------------------------------------
{
	//slideDirectionsStart[data.nachbarID] = data.startAbsID;
	//slideDirectionsEnd[data.nachbarID] = data.endAbsID;
	slideCodes[data.nachbarID] = code;
	isFlippedArray[data.nachbarID] = data.isFlipped;	//auf diesem Wert kann 
}

//-----------------------------------------------------------
int		face::isDirectionStored(dData& data)
//-----------------------------------------------------------
{

//	int tmp = slideDirectionsStart[data.nachbarID];
	int code = slideCodes[data.nachbarID];

	if( code > 0 )
	{
		//if(tmp == data.startAbsID)
		if(isFlippedArray[data.nachbarID] == false)
			if(data.isFlipped != false)
			{
				flipData(data);
				return 1;
			}
			else
			//Daten unverändert übergeben, wenn Reihenfolge bereits richtig
				return 1;
		else
		{
			//ansonsten Daten umtauschen und true zurückgeben
			flipData(data);
			return 1;
		}
		
	}
	else if(code == -1)
	{
		return -1;	//minus 1 nur, wenn Code auf blocken gestellt ist

	}
	else
	{
		return 0;
	}




}


//------------------------------------------------------------------------------
void	face::processCornerEdges(MIntArray& processedVtx,MIntArray& newVtxCount,MIntArray& cornerData)
//------------------------------------------------------------------------------
{
	//Processed Vtx ist codiert wie folgt: nx     nx+1 ... l-y					l-1 
										//locID	  absID	  IndexVonAnfangCEdgeY	GesamtzahlCEdgesInArray
	//NEWVtxCount speichert pro CornerEdge einen Vert für Num NewVtx
	//newVtxCount.setLength(numCorners); //MUSS BEREITS MIT 4 SLOTS INITIALISIERT SEIN
	MIntArray	tmpCornerStarts(4,-1);
	int cornerCount = numCorners;							
	int nextCorner,numVtx,numNachbarn,l,a = 0,i = 0,lVtx = vtx.length(),count = 0,nVtxCount = 0;	//numVtx = zahl der Vtx in der COrnerEdge incl cornerVtx selbst
	//pro CornerEdge Boundarys festlegen und standard Processing starten

	int lastVtxLocID;

	////cout<<"PROCESS_CORNER_EDGES: "<<"BEGIN"<<endl;
	while(cornerCount != 0)
	{
		
		a = validateThisCornerLocID(a);	
		////cout<<"PROCESS_CORNER_EDGES: "<<a<<" = StartCorner"<<endl;
		nextCorner = findNextValidCornerLocID(a);
		////cout<<"PROCESS_CORNER_EDGES: "<<nextCorner<<" = NextCorner"<<endl;
		numVtx = getCornerEdgeVtxCount(a,nextCorner);
		
		cornerData.append(a);
		cornerData.append(numVtx);

		numNachbarn = getNumCornerEdgeNachbarn(a,numVtx);
		////cout<<"PROCESS_CORNER_EDGES: "<<numNachbarn<<" = Num CornerEdgeNachbarn"<<endl;

		l = a + numVtx-1;
		////cout<<"PROCESS_CORNER_EDGES: "<<l<<" = Maximum Length()"<<endl;

		////cout<<"PROCESS_CORNER_EDGES: "<<"Bin vor NachbarWhile"<<endl;
		
		i = a;
		while(numNachbarn != 0)
		{
			i = findNextValidNachbarLocID(i);

			////cout<<"PROCESS_CORNER_EDGES: "<<i<<" = ValidNachbarStartID"<<endl;

			//Wenn i über der gültigen CornerRange liegt oder wenn errorCorde ausgegeben, abbruch
			if( i>=l || i == -1)
			{
				cout<<"PROCESS_CORNER_EDGES: "<<"i > l, Abbruch"<<endl;
				break;
			}
			
			//erstmal die gültige IDs hinzufügen
			processedVtx.append(i);
			processedVtx.append(vtx[i]);
			
			lastVtxLocID = findLastNachbarVtx(i);
			
			int myType = 0; //extraTypeDef für Faces: nonControl = 0, typ 1 == 1, typ2Control == 2
			if(type == 0 || type == -1 || type == 3 || type == 4)
				myType = 0;
			else if(type == 1)
				myType = 1;
			else if(type == 2)
			{
				if(isControlTwo)
					myType = 2;
				else
					myType = 0;
			}
			//jetzt DataObj erstellen für SlideProcs
			dData data(i,lastVtxLocID,a,nextCorner,vtx[i],vtx[lastVtxLocID],corners[a],corners[nextCorner],myType,id,i);
			//PS: isFlipped ist automatisch false
			
			////cout<<"PROCESS_CORNER_EDGES: "<<lastVtxLocID<<" = letzte Gültige NachbarVtxLocID"<<endl;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//WICHTIG: BEDENKEN; DASS DURCH SLIDESYSTEM DIE REIHENFOLGE VON START/ENDVTX GEÄNDERT WERDEN KANN. DIESE DARF ABER NICHT IN
//VERTAUSCHTER FORM AUF PROCESSED VTX
			int tmp2;
			int tmp = lastVtxLocID - i;
			if( tmp <= 0)
			{
				//lastVtx liegt wieder am anfang des IdArrays
				tmp2 = (i + ( numVtx - (i - a) - (nextCorner - lastVtxLocID) ))%lVtx - 1;
				//tmp2 = (lVtx ) - (i + 1) + lastVtxLocID;
				tmp2 = int(sqrt(double(tmp2*tmp2)));
				////cout<<"PROCESS_CORNER_EDGES: "<<tmp2<<" = Tmp2"<<endl;
				
				if( tmp2 == 0)
				{
					//createVtx(processedVtx,i,lastVtxLocID,i,false,data);
					createVtx(processedVtx,i,lastVtxLocID,i,true,data);
					newVtxCount[nVtxCount]++;
				}
				else
				{
					
					//newVtxCount[nVtxCount] += processMultiVtxNachbar(processedVtx,tmp2,i,false,data);
					newVtxCount[nVtxCount] += processMultiVtxNachbar(processedVtx,tmp2,i,true,data);
				}
				
			}
			else
			{
				if(tmp == 1)
				{
					//kein Vtx liegt zwischen ihnen, newVtx erstellen
					createVtx(processedVtx,i,lastVtxLocID,i,true,data);
					newVtxCount[nVtxCount]++;
					
				}
				else
				{
					newVtxCount[nVtxCount] += processMultiVtxNachbar(processedVtx,tmp-1,i,true,data);
				}
			}
			
			//zum Schluss auf jeden Fall EndVtx hinzufügen
			processedVtx.append(lastVtxLocID);
			processedVtx.append(vtx[lastVtxLocID]);

			
			i++;
			numNachbarn--;
			
		}

		i = nextCorner;

		nVtxCount++;
		tmpCornerStarts[count++] = processedVtx.length();
			
		//////cout<<"PROCESS_CORNER_EDGES: "<<" Durchlauf für Nachbarn Ende"<<endl;
		//////cout<<"PROCESS_CORNER_EDGES: "<<processedVtx.length()<<" = Pos. für CornerStart"<<endl;

		a++;
		cornerCount--;

	}

	//Zum schluss noch den processedVtx Array richtigstellen mit hilfe tmpCornerStarts
	//letztes element ist anzahl der CornerEdges
	processedVtx.append(numCorners);
	//letztes Element von tmpcornerstarts ist unnoetig
	cornerCount = numCorners - 1;
	for(i = cornerCount; i != 0 ;i--)
	{
		processedVtx.insert(tmpCornerStarts[i-1],processedVtx.length()-1);
	}

	//////cout<<"PROCESS_CORNER_EDGES: "<<"ENDE"<<endl;

	//Hui, das wars =)
}

//------------------------------------------------------------------------------------------------
MIntArray	face::createCompleteCornerEdgeArray(int cornerStartID,int l,int i,MIntArray& processedVtx)
//------------------------------------------------------------------------------------------------
{
	//<l> ist die länge der CornerEdge, <i> ist der Index des vtxPerFace[4] Arrays der Hauptprocedur
	MIntArray	vtxPerCorner;
	MIntArray	begleiter;
	int vtxL = vtx.length();

	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<"START"<<endl;
	
	
	int a;
	for(a = cornerStartID; a < l; a++)
	{
		vtxPerCorner.append(a%vtxL);
		begleiter.append(0);
	}
	
	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<"Bin vor BOUNDProcessing"<<endl;
	
	int rBound,lBound;
	//Nun ProcessedVtx hinzufügen
	if(i != (int)numCorners-1)
		rBound = processedVtx[processedVtx.length() - 1 - (i+1)];
	else
		rBound = processedVtx.length() - 1 - (i+1);
	
	if(i == 0)
		lBound = 0;
	else
		lBound = processedVtx[processedVtx.length() - 1 - i];

	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<lBound<<" = LBOUND"<<endl;
	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<rBound<<" = RBound"<<endl;
	
	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<"Trage newVtxAbsIDs  ein ..."<<endl;
	
	for(a = lBound; a < rBound; a+=2)
	{
		//wenn newVtx, dann lokalePos von vorgänger in vtxpercornerArray finden und newVtx eintragen
		if(processedVtx[a] == -1)
		{
			l = vtxPerCorner.length();
			
			for(int x = 0; x < l;x++)
			{
				if(processedVtx[a+2] == vtxPerCorner[x])
				{
					vtxPerCorner.insert(processedVtx[a+1],x);
					begleiter.insert(-1,x);
					break;
				}
			}
		}
	}
	
	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<"Finde absolute IDs"<<endl;
	//und jetzt noch all die lokalen IDs ersetzen durch Absolute VtxIDs aus VtxArray
	l = vtxPerCorner.length();
	for(a = 0; a < l;a++)
	{
		if(begleiter[a] != -1)
			vtxPerCorner[a] = vtx[vtxPerCorner[a]];
	}
	
	////cout<<"CREATE_COMPLETE_CORNEREDGE_ARRAY: "<<"BEENDET"<<endl;
	return	vtxPerCorner;
}

//----------------------------------------------------------
void	face::splitType4()
//----------------------------------------------------------
{
	//!!!!!!!BEACHTEN: DIESE PROCEDUR (FACESPLIT) AUFRUESTEN; SO DASS SIE NICHT NUR MIT 4CORNERCOUNTS UMGEHEN KANN
	 // durch nachbarIDs iterieren und Punkte sammeln

							//!!!!!!!BEACHTEN: Nur in zweierschritten durchsteppen: {lokaleID,AbsoluteID}{L,A}usw
	MIntArray	processedVtx;	// hier werden echte IDs der Vtx gespeichert zur späteren Erstellung von Faces, erst lokale id, danach absolute id!!!!
	MIntArray	cornerData;		//enthält in n den die startLocID der Corner, in n+1 die Zahl der Vtx auf CornerEdge inkl. cornerVtx PRO CORNER

	int centerVtx = creator->createCenterVtx(id);


		creator->createUV(id,creator->getLastVtxID(), newUVs);


		creator->createNormal(id,creator->getLastVtxID());


	creator->addNormalSlide(centerVtx,id);

	MIntArray newVtxCount(4,0);	//muss initialiseirt werden mit 4,0
	
	//cout<<"Bin vor ProcessCornerEdges"<<endl;
	//Macht die ganze Arbeit und erstellt Vertizen, wenn noetig
	processCornerEdges(processedVtx,newVtxCount,cornerData);	//newVtxCount pro Corner

	//printArray(processedVtx," = PROCESSEDVtx");
	//printArray(cornerData," = CornerDATA");
	

	//jetzt durchs temparray steppen und Faces erstellen, moeglichst quads
	//zuerst Schätzung einholen, wieviele Quads und triangles erstellt werden koennen, dies ermäglicht eine gewisse Einteilung der Vtx

	
	//l = processedVtx.length();
	MIntArray	newVtxOrder;
	MIntArray	vtxPerCorner[4];	//enthält alle 

	//Ziel: Alle Vtx samt NewVtx pro corner in ein Array packen in richtiger Reihenfolge 
	//cout<<"Bin vor CreateCompleteCornerEdgeArray"<<endl;


	int i;
	for(i = 0; i < (int)numCorners;i++)
	{
		int l = cornerData[i*2] + cornerData[2*i+1];
		
		vtxPerCorner[i] = createCompleteCornerEdgeArray(cornerData[i*2],l,i,processedVtx);
		//printArray(vtxPerCorner[i]," = VtxPERCorner");	
	}

	
	int first = 0,last = 0,second = 0,third = 0;
	switch(numCorners)
	{
	case 4:
		{
			first = 0;second = 1;third =2;last = 3;
			break;
		}
	case 3:
		{
			first = 0;second = 1;third = 1;last = 2;
			break;
		}
	case 2:
		{
			first = 0;second = 1;third = 1;last = 1;
			break;
		}
	}



		//cout<<"Erstelle Faces"<<endl;
	//StartFace verändern
	newVtxOrder.append(vtxPerCorner[first][0]);
	newVtxOrder.append(vtxPerCorner[first][1]);
	newVtxOrder.append(centerVtx);
	newVtxOrder.append(vtxPerCorner[last][vtxPerCorner[last].length()-2]);
	
	int lBound = 1;

	creator->changePolyVtxIDs(id,newVtxOrder);


		creator->changeUVIds(id,newUVs,newVtxOrder,vtx);


		creator->changeNormals(id,newVtxOrder, false, true);

	//letzten Vtx von [3] runterschmeissen, damit kein Face doppelt erstellt wird
	vtxPerCorner[last].remove(vtxPerCorner[last].length()-1);
	newVtxOrder.clear();
	
	//EndFace
	newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-2]);
	newVtxOrder.append(vtxPerCorner[first][vtxPerCorner[first].length()-1]);
	newVtxOrder.append(vtxPerCorner[second][1]);
	newVtxOrder.append(centerVtx);

	int rBound = vtxPerCorner[first].length()-2;

	creator->createPoly(newVtxOrder);


		creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


 				creator->changeNormals(id,newVtxOrder,true, true);
	
	vtxPerCorner[second].remove(0);

	
	//Jetzt Faces zwischen den Bounds erstllen, falls noetig
	if(rBound-lBound > 0)
		createInnerCornerPolys(lBound,rBound,centerVtx,vtxPerCorner[first],newVtxOrder);
	
	if(numCorners != 2)
	{
		for(i = first + 1; i <= last;i++)
		{
			newVtxOrder.clear();
			//cout<<"WAR IN FOR_SCHLEIFEdd, i == "<<i<<endl;
			//hier werden die mittleren CornerEdges bearbeitet
			//zuerst das abschlussQuad erstellen und entsprechende 
			//printArray(vtxPerCorner[i]," = Vtx in schleife");
			if(i != last)
			{
				newVtxOrder.append(vtxPerCorner[i][vtxPerCorner[i].length()-2]);
				newVtxOrder.append(vtxPerCorner[i][vtxPerCorner[i].length()-1]);
				newVtxOrder.append(vtxPerCorner[i+1][1]);
				newVtxOrder.append(centerVtx);
				
				vtxPerCorner[i+1].remove(0);
				
				creator->createPoly(newVtxOrder);


					creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


					creator->changeNormals(id,newVtxOrder,true, true);

				lBound = 0;			rBound = vtxPerCorner[i].length()-2;
			}
			else
			{
				lBound = 0;			rBound = vtxPerCorner[i].length()-1;	//ob minus 1 oder minus 0 ist hier eigentlich egal
			}
			
			//jetzt die übrigen Faces Splitten;
			if(rBound-lBound > 0)
				createInnerCornerPolys(lBound,rBound,centerVtx,vtxPerCorner[i],newVtxOrder);
			//else
				//cout<<"BOUNDSECTION IST NULL!!!!"<<endl;
			
		}
	}
	else
	//wenn corner two, etwas nachhelfen
	//if(numCorners == 2)
		createInnerCornerPolys(0,vtxPerCorner[last].length()+1,centerVtx,vtxPerCorner[last],newVtxOrder);

	


	//GENERELL COOL: FinalVersion darf dass nur machen, wenn nachbar ebenfalls Typ 4 ist
	//testweise alle Vtx von Face mit NormalSlide versehen
	MIntArray	nachbarSave,locIDs;
	int startLocVtx,endLocVtx,l2 = vtx.length(),type4Count = 0;
	for(i = 0; i < (int)numCorners; i++)
	{
		//if(i != numCorners - 1)
		//	l--;
		//nachbarIDs finden
		
		locIDs = getCornerEdgeNachbarIDs(nachbarSave,vtxPerCorner[i][0],vtxPerCorner[i].length());
	
		for(unsigned int x = 0; x < locIDs.length();x++)
		{//WARNUNG: ZU TESTZWECKEN GEBE ICH HIER ALS NACHBAR ID EIN FACE AN; WAS MIT SICHERHEIT! NUR IND STANDARDTYPE 4 VORKOMMT
			if(nachbarn[locIDs[x]]->whichType() == 4 && !nachbarn[locIDs[x]]->isClean())
			{
				//jetzt pro 4'er nachbar die geteilten Vtx hinzufügen
				type4Count++;
			}
		}
	}

	//wenn mindestens 2 type 4 dran sind, werden alle Punkte mit along normal geschleppt
	if(type4Count >= 2)
	{
		for(i = 0; i < (int)numCorners; i++)
		{
			int l = vtxPerCorner[i].length();
			
			for(int a = 0; a < l; a++)
				creator->addNormalSlide(vtxPerCorner[i][a],id);

		}

	}
	//wenn nur ein t4 face, dann nur die geteilten Vtx hinzufügen
	else if(type4Count == 1)
	{
		for(i = 0; i < (int)locIDs.length();i++)
		{
			if(nachbarn[locIDs[i]]->whichType() == 4)
			{
				startLocVtx = locIDs[i];
				endLocVtx = findLastNachbarVtx(startLocVtx)+1;
				
				for(int a = startLocVtx;a < endLocVtx;a++)
					creator->addNormalSlide(vtx[a%l2],id);
			}
		}
	}
	  

	
}
#ifdef DEBUG

//-----------------------------------------------------------------
void		face::printArray(MIntArray& array, MString message)
//-----------------------------------------------------------------
{
	for(unsigned int i = 0; i < array.length(); i++)
		cout <<array[i]<<message.asChar()<<endl;

	cout <<"//printArray Ende"<<endl;
	cout <<endl;
	cout <<endl;
	cout <<endl;
	
}

#endif

//--------------------------------------------------------------------------------------------------------------------
void	face::createInnerCornerPolys(int lBound,int rBound,int centerVtx,MIntArray& vtxPerCorner,MIntArray& newVtxOrder)
//--------------------------------------------------------------------------------------------------------------------
{

	int	add = 2;
	//Jetzt Faces zwischen den Bounds erstllen, falls noetig
	for(int i = lBound;i < rBound; )
	{
		newVtxOrder.clear();
		
		//errechnen, ob dreieck oder quad erstellen
		add = 2;
		add -= (rBound - i + 2 )%2;
		
		newVtxOrder.append(vtxPerCorner[i]);
		//entweder 1 oder 2 weitere faces erstellen
		for(int a = 0;a < add;a++)
			newVtxOrder.append(vtxPerCorner[i+a+1]);
		i += add; 
		//noch mittelpunkt hinzufügen
		newVtxOrder.append(centerVtx);
		//face erstellen

		creator->createPoly(newVtxOrder);


			creator->changeUVIds(id,newUVs,newVtxOrder,vtx,true);


				creator->changeNormals(id,newVtxOrder,true, true);
			
	}

}
//--------------------------------------------------------------------------------------------------------------------
void 	face::createVtx(MIntArray& tmpFaceVtx,int startLocID, int endLocID,int realStartLocID,bool useNachbar,dData& data)
//--------------------------------------------------------------------------------------------------------------------
{
	//>data< enthält alle Infos für GatherSlideData

	//trägt neue VtzxID in tmpFaceVtx und in newVtx ein 
	//erstmal Checken, ob nachbar schon Vtx erstellt hat oder den eigenen Slot gesetzt hat -> im Grunde braucht man den Nachbarn gar nicht zu fragen, da er den eigenen Slot setzt
	int newVtxID;
	////cout<<"CREATE_VTX: "<<"Existiert Vtx Check: startLocID = "<<realStartLocID<<endl;
	//int existingVtx = nachbarn[realStartLocID]->dsVtxExist(id);
	bool	useSlide = false; //useSlide wird von folgender GatherSlideInfo gesetzt
	//createVtx sucht automatisch nach geeigneten SlideWerten und gibt entsprechende werte an "newVtxBetweenIDs"
//	bool tmpSlide;	//nur zum testen, später muss dann <<useSlide>> benutzt werden

	int existingVtx = newVtx[realStartLocID];
	if(existingVtx == -1)
	{
		dData	bakDat = data;
	//	printData("PreSlideDATA: ",data);
		useSlide = gatherSlideData(data);
	//	cout<<"SLIDEDATEN FUER FACE "<<id<<endl;
	//	cout<<"SLIDE ERLAUBT? "<<useSlide<<endl;
	//	printData("FinalSlideDATA: ",data);
		if(!useSlide)
			data = bakDat;

		//kein Vtx liegt zwischen ihnen, newVtx erstellen; hier wird kein nachbarface übergeben, da nicht eindeutig
		tmpFaceVtx.append(-1);	//neue Vtx liegen nicht im Array, haben keine LocalID = -1 Flag

		if(useNachbar)
		{
			//newVtxID = creator->newVtxBetweenIDs(vtx[startLocID],vtx[endLocID],id,useSlide,nachbarIDs[realStartLocID]);
			newVtxID = creator->newVtxBetweenIDs(data.startAbsID,data.endAbsID,id,useSlide,nachbarIDs[realStartLocID]);
			tmpFaceVtx.append(newVtxID);
		}
		else
		{
			//newVtxID = creator->newVtxBetweenIDs(vtx[startLocID],vtx[endLocID],id,useSlide);
			newVtxID = creator->newVtxBetweenIDs(data.startAbsID,data.endAbsID,id,useSlide);
			tmpFaceVtx.append(newVtxID);
		}

		//UVs werden auch von dieser Prozedur behandelt

			creator->createUV(id,newVtxID,newUVs,useSlide,data.startLocID,data.endLocID);

			//normalen müssen immer die standardIDs (nicht die duch ne direction veränderten bekommen, damit der Lookup ordentlich klappt
			creator->createNormal(id,newVtxID,vtx[startLocID],vtx[endLocID]);

		//jetzt dem Nachbarn an <startLocID> über neuen Vtx informieren
		newVtx[realStartLocID] = newVtxID;
		nachbarn[realStartLocID]->setVtxExists(id,newVtxID);
	}
	else
	{
		//wenn Vtx schon existiert, wurden auch schon slideWerte hinzugefügt, wenn moeglich, dies hier also nicht machen
		
		
		//Um UVs zu erzeugen, muss hier extra der creator aufgerufen werden

		
			useSlide = (isDirectionStored(data) > 0)?true:false;
			creator->checkForUV(id,nachbarIDs[realStartLocID], newUVs, nachbarn[realStartLocID]->getNachbarUVArray() ,data.startLocID, data.endLocID,existingVtx,useSlide);
		

		//für die normalen muss immer eine normale pro face vorhanden sein, diese also neu erzeugen (selbst wenn sie in die selbe Richtung zeigt
		//wir die des anderen faces)

			creator->createNormal(id,existingVtx,vtx[startLocID],vtx[endLocID]);

		tmpFaceVtx.append(-1);
		tmpFaceVtx.append(existingVtx);
		newVtx[realStartLocID] = existingVtx;
	}
	
}

//--------------------------------------------------------------------------------------------------------------------
int		face::processMultiVtxNachbar(MIntArray& tmpFaceVtx,int vertsZwischenIDs,int startLocID,bool useNachbar,dData& data)
//--------------------------------------------------------------------------------------------------------------------
{
	//Gibt 1 zurück, wenn er neuen Vtx erstellt hat
	//!!!!!!!!!!!!!!!!!!!AUFRUESTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Diese Methode kann momentan noch nicht mit SLIDE umgehen
	//!!!!!!!!!!!!!!!!!!!AUFRUESTEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	int		nIDLength = vtx.length() ;//- 1;	//hier gleich letzten Flag abziehen
	int		madeNewVtx = 0;

	bool	useSlide = false; //wird von gatherSlideData gesetzt
	//mindestens 1 Vtx liegt zwischen ihnen
	//!!!!!!!!Eventuell vorher noch die Wahre Zahl an Vtx zwischen den IDs erreichen ...!!!!!!!
	//ist die Zahl ungerade, mittleren Vtx nehmen, ansonsten zwischen n/2 und  n/2+1 neuen Vtx erstellen
	if(vertsZwischenIDs%2 == 1)
	{
		//erst ab dem nächsten index beginnen, da startLocID schon hinzugefügt wurde
		int myL = startLocID + vertsZwischenIDs + 1;
		int	mA;		// startLocID modulo a
		int middleVtx = (vertsZwischenIDs - 1)/2 + 1 + startLocID;

		//Data modifizieren, damit es die mittleren Vtx representiert
		data.startLocID = middleVtx-1;
		data.endLocID = (middleVtx + 1)%nIDLength;
		data.startAbsID = vtx[middleVtx - 1];
		data.endAbsID = vtx[(middleVtx + 1)%nIDLength];

		useSlide = gatherSlideData(data);
	//	cout<<"SLIDEDATEN FUER FACE(MULTI_VTX) "<<id<<endl;
	//	cout<<"SLIDE ERLAUBT? "<<useSlide<<endl;
	//	printData("FinalSlideDATA: ",data);

		//es ist ungerade, also mittelsten Vtx hinzufügen, aber die davor erstmal auf Array tun, dann newVtx, dann die Danach
		for(int a = startLocID+1; a < myL; a++)
		{
			//jedes mal auf bereichsüberschreitungen achten und Flags ausschliessen
			mA = a % nIDLength;
			
			
			if(a != middleVtx)
			{//check auf MiddleVtx
				tmpFaceVtx.append(mA);
				tmpFaceVtx.append(vtx[mA]);
			}
			else
			{
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//HIER NOCH USE SLIDE EINBAUEN!!!
				//es ist der MiddleVtx
				if(useSlide)
				{
					if(useNachbar)
						creator->addSlide(vtx[mA],data.startAbsID,data.endAbsID,id,nachbarIDs[data.startLocID]);
					else
						creator->addSlide(vtx[mA],data.startAbsID,data.endAbsID,id);


						creator->addUVSlide(vtx[mA],data.startAbsID,data.endAbsID,id,newUVs);
				}
				else
				{
					if(useNachbar)
						creator->addNormalSlide(vtx[mA],id,nachbarIDs[startLocID]);
					else
						creator->addNormalSlide(vtx[mA],id);
				}
				
				//cout<<"PROCESS_Multi_Vtx_Nachbar: "<<"Will info hinzufügen"<<endl;
				tmpFaceVtx.append(mA);
				tmpFaceVtx.append(vtx[mA]);
				
				newVtx[startLocID] = vtx[mA];
				madeNewVtx++; //SELBST WENN VTX SCHON VOHANDEN WIRD ER IN NEWVTXCOUNTarray AUFGENOMMEN, damit SplitNormalTwo nicht versagt
			}
			
		}
	}
	else
	{
		//durch array steppen,alle Vtx hinzufügem und zwischen den mittleren Vtx neuen Vtx erstellen
		int myL = startLocID + vertsZwischenIDs + 1;
		int	mA;		// startLocID modulo a
		int middleLeft = startLocID+vertsZwischenIDs/2;
		//es ist ungerade, also mittelsten Vtx hinzufügen, aber die davor erstmal auf Array tun, dann newVtx, dann die Danach
		//!!!!!!!!!!!!!!!!
		//!!!!!DATA-OBJ zuerst verändern, so dass die entsprechenden StartEnds (middleLeft/right) respektiert werden
		//!!!!!!!!!!!!!!!!
		data.startLocID = middleLeft;
		data.endLocID = (middleLeft + 1)%nIDLength;
		data.startAbsID = vtx[middleLeft];
		data.endAbsID = vtx[(middleLeft + 1)%nIDLength];

		//gatherSlideData(data);  GatherSlideData wird von >>createVtx<< gerufen

		for(int a = startLocID+1; a < myL; a++)
		{
			//jedes mal auf bereichsüberschreitungen achten und Flags ausschliessen
			mA = a % nIDLength;
			
			
			//check auf MiddleVtx
			if(a == middleLeft)
			{
				
				tmpFaceVtx.append(mA);
				tmpFaceVtx.append(vtx[mA]);
				

				createVtx(tmpFaceVtx,mA,mA+1,startLocID,useNachbar,data);


				madeNewVtx++;
			}
			else
			{
				tmpFaceVtx.append(mA);
				tmpFaceVtx.append(vtx[mA]);
			}
				
		}
	}
	return madeNewVtx;			
}



//-------------------------------------------------
bool	face::gatherSlideData(dData& data)
//-------------------------------------------------
{
	//Generell sind 2 Dinge zu beachten: Man müsste den IdentifikationsCode für die SlideDirektinosquelle speichern, 
	//um im Nachhinein unberechtigt erstellte Slides (weil z.B die andere Seite blockiert ist (code -1) ) entfernen zu koennen
	//Zweitens muss die fty zuerst die ControlFaces herausfiltern und diese zuerst Engagen, damit nicht die eine Seite von Face
	//direction von anderer quelle erhält als die andere Seite
	dData bakData,tmpData;
	//Zuerst checken, was für Typ Face ist und danach entsprechendes einleiten
	int tmp;
	int code;	//-1 ist keinSlide, 1 ist slide von Type1Face, 2 von Type3 und 3 von Type 2, hirarchisch geordnet.
	int errCode;
	switch(type)
	{
	case -1:	//gibt nur false zurück, da slide nicht moeglich ist
		{
			return false;
		}
	case 4:
		{
		//Typ 4 gibt einfach false zurück, ausserdem verändert es dData nicht
		return false;
		}
	case 3:
		{
			// wenn edge gegenüber hat, dann einfach requesten, ansonten blocken

			tmp = isDirectionStored(data);

			if(tmp == 1)
			{
				return true;
			}
			else if(tmp == -1)
				return false;


			//nur wenn tmp  == 0, also nix gespeichert ist, checken, ob opposite edge vorhanden
			code = convertToOpposite(data); //wenn CTO fehlschlägt, verändert er die Daten nicht
			
			if(code == -1)
				return false;

			//check, ob bereits was gespeichert
			tmp = isDirectionStored(data);

			if(tmp == 1)
			{
				//wenn für diese Seite speicherung vorliegt, dann auch für die andere ( siehe requestCode )
				convertToOpposite(data);
				return true;
			}
			else if(tmp == -1)
			{
				//daten wieder auf ursprungsseite konvertieren
				convertToOpposite(data);
				return false;
			}

			//jetzt einfach von opposite edge ausgehen und die erhaltene direction auf beide seiten kopieren
			code = nachbarn[data.nachbarID]->requestDirections(data);

			//cout<<"BEENDE TYP 3, ID : "<<id<<endl;
			//cout<<"------------------------------------------------------------------"<<endl;
			//cout<<"------------------------------------------------------------------"<<endl;

			
			data = convertData(data,errCode);
			//printData("DATA NACH REQUEST+CONVERT: ",data);
			

			storeDirection(data,code);
			convertToOpposite(data);
			storeDirection(data,code);

			if(code == -1)
				return false;
			else
				return true;
			
		}
	case 2:
		{



			if(isControlTwo)
			{
				//wenn nichts gespeichert, selbst erstellen und validieren lassen

				tmp = isDirectionStored(data);
		
				if(tmp == 1)
				{
				//	cout<<"GATHER SLIDE DATA TYPE TWO CONTROL: "<<"Direction war bereits gespeichert"<<endl;
					return true;
				}
				else if(tmp == -1)
					return false;
				
				createDirections(data);

				code = nachbarn[data.nachbarID]->requestDirections(data);
				data = convertData(data,errCode);

			//	printData("Data (Typ2Control) nach request: ",data);

				storeDirection(data,code);
				if(code == -1)
				{
					return false;
				}
				else			//für alle anderen Codes ( -2 und +x) dasselbe machen
				{
					return true;
				}
			}
			else//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			{	//IM GRUNDE SOLLTE ES NICHT PASSIEREN KÖNNEN; DASS ZWEIERFACE SELBST NACH SLIDEIRECTIONS FRAGT
				//gültige dData holen für normales "DurchlaufSplitFace"
				//erstmal schauen, ob Richtungen nicht schon bereits in DirectionArray abgelegt sind
				
				//erstmal Backup machen von der Data:
				bakData = data;

				tmp = isDirectionStored(data);
		
				if(tmp == 1)
				{
					//cout<<"GATHER SLIDE DATA TYPE TWO: "<<"Direction war bereits gespeichert"<<endl;
					return true;
				}
				else if(tmp == -1)
					return false;
				//erstnmal nachbarn fragen, ob er slideDirections hat, Wenn nachbar andere SlideRichtung vorsieht, dann flippt er die AbsIDs der Data
				//hier nicht andere Seite nehmen, da von hier alles losgeht
				//printData("DATA vor REQUEST: ",data);
				code = nachbarn[data.nachbarID]->requestDirections(data);
				//nach Request muss Data immer wieder convertiert werden
				tmpData = convertData(data,errCode);
				data = tmpData;
				if(errCode == -1)
					return errCode != 0;

				//cout<<"GATHER SLIDE DATA: AKTUELLE ID IST: "<<id<<endl;
				//printData("DATA nach REQUEST+CONVERT: ",data);
				//cout<<"GATHER SLIDE DATA: CODE IST: "<<code<<endl;
				//egal was geschieht, daten müssen gespeichert werden, damit das Face im Grunde nie auf die gezwungen wird
				//selbst zu requesten, obgleich es das kann
				storeDirection(data,code);
				//für die andere Seite auch speichern
				//tmpData = data;		//tmpData ist bereits = data
				convertToOpposite(tmpData);
				//printData("Opposite Save Data: ",tmpData);
				storeDirection(tmpData,code);

				

				if(code == -1)
				{	
					return false;
				}
				else			//für alle anderen Codes ( -2 und +x) dasselbe machen
				{
					return true;
				}
				
			}
		}
	case 1:
		{
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//MERKE: TYP 1 SPLIT IST ETWAS AUFWENDIGER; DA GEGENUEBERLIEGENDE SEITE NICHT IN "NACHBARN" AUFGEFUEHRT IST
			//		SPLIT MUSS EXTRAREGEL ERHALTEN
			//ist grundsätzlich befugt, SlideDirections zu erstellen, wird aber von jedem anderen controlFace überstimmt
			tmp = isDirectionStored(data);
			
			if(tmp == 1)
			{
				//cout<<"GATHER SLIDE DATA: Direction gefunden"<<endl;
				return true;
			}
			else if(tmp == -1)
				return false;

			//directions erstellen und diese dann validieren lassen
			createDirections(data);
			
		//	printData("DIRECTIONS NACH CREATE TYPE 1: ",data);

			code = nachbarn[data.nachbarID]->requestDirections(data);
		//	cout<<"BEENDE TYP 1, ID : "<<id<<endl;
		//	cout<<"------------------------------------------------------------------"<<endl;
		//	cout<<"------------------------------------------------------------------"<<endl;
			data = convertData(data,errCode);


			storeDirection(data,code);

			if(code == -1)
				return false;
			else
				return true;

			//erhaltene Daten auf jeden Fall speichern und entsprechend ErrCode handlen

			
		}

	}

	//forbidden path
	cout<<"FACE:GATHER SLIDE DATA: FORBIDDEN PATH"<<endl;
	return false;

}


//------------------------------------------------------------------------------
int		face::requestDirections(dData& data)
//------------------------------------------------------------------------------
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Zum schluss von Request muss noch die direction selbst aus den gewonnenen daten extrahiert werden
	//für den fall, dass zurückgegebene Vtx nicht mit den gegebenen übereinstimmen (WESHALB AM ANFANG EINES REQUESTS
	//AUCH ERST NOCH EIN BACKUP VON DATA GEMACHT WERDEN MUSS FUER DEN SPÄTEREN VERGLEICH
	dData locData,tmpData;		//bakData = BackupData für späteren vergleich
	int tmp;
	int code;
	int errCode;
	switch(type)
	{
	case -1:	//== Face ohne cornerEdge: blockt alle slideAnfragen
		{
			data = convertData(data,errCode);
			return -1;
		}
	case 4:
		{
			//Typ 4 blockt alle anfragen und gibt -1 zurück getreu nach dem Motto: Nicht bewegen

			//daten kovertieren, und -1 zurückgeben
			data = convertData(data,errCode); //jeNachdem wie der Algo von convertData funzt, kann man diese Konvertierung dann villeicht auch sei lassen
			return -1;
		}
	case 3:
		{
			//cout<<"REQUEST DATA TYPE THREE: "<<"Beginne Request für ID: "<<id<<endl;
			//cout<<"----------------------------------------------------------------"<<endl;
			//cout<<"----------------------------------------------------------------"<<endl;
			//schutz vor Loops
			if(data.requesterID == id)
			{
				//dann einfach die vorhandene Direction mit Code 1 zurückschicken, ohne vorher ein anderes Faces
				//zu konsultieren
				//cout<<"REQUEST DIRECTION TYPE THREE: Bin an Ursprungface angekommen"<<endl;
				//data = locData;
				return 1;
			}

			//darf selbst entscheiden in Richtung mit beiden nachbarn. convertToOpposite muss ERCode ausgeben koennen
			locData = convertData(data,errCode);
			
			//printData("DATA CONVERTED TYPE 3: ",locData);

			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if(isDirectionStored(locData))
			{
				data = locData;
				return slideCodes[locData.startLocID];	//Hier mal lieber den Code der NachbarID geben
			}


			code = convertToOpposite(locData);


			if(code == -1)
			{
				//wenn requester ein face 2 ist, dann darf er doch sliden - >besser ausschalten, da bei manchen Combis das damm wieder scheisse ist
				if( locData.typeInfo == 2)
				{
					storeDirection(locData,2);	//wenn code -1, wurde data nicht verändert
					data = convertData(data,errCode);
					return 1; //minimalCode zurückgeben

				}
				else
				{
					storeDirection(locData,code);	//wenn code -1, wurde data nicht verändert
					data = convertData(data,errCode);
					return code; //Data muss sicherheitshalber immer angepasst werden, weil empfangsMehtode diese dann konvertieren will
				}			
			}

			
			//ansonsten directions von nachbarn requesten
			code = nachbarn[locData.nachbarID]->requestDirections(locData);
			locData = convertData(locData,errCode);

			//cout<<"REQUEST DATA TYPE THREE: "<<"BEENDE Request für ID: "<<id<<endl;
			//cout<<"----------------------------------------------------------------"<<endl;
			//cout<<"----------------------------------------------------------------"<<endl;

			//printData("DATA TYPE 3 NACH REQUEST+CONVERT: ",locData);

			//wenn egal welcher code folgt,face nimmt immer die erhaltenen Directions an
			if(code != -1)
				storeDirection(locData,code);
			//wenn convert to opposite fehlschlägt, dann die daten nicht speichern sondern unverändert zurück
			tmp = convertToOpposite(locData);
			if(tmp != -1)
				storeDirection(locData,code);
			else
				code = tmp;
			
			data = locData;
			
			return code;
		
			//soll doch nicht selbst directions erstellen
			//createDirections(locData);	//die Speicherung der Directions für beide Seiten wird in createDirections übernommen, 
										//locData muss danach wieder konvertiert werden

		}
	case 2:
		{
			if(isControlTwo)
			{
			//	cout<<"REQUEST DATA TYPE TWO CONTROL: "<<"BEGINNE Request für ID: "<<id<<endl;
			//	cout<<"----------------------------------------------------------------"<<endl;
			//	cout<<"----------------------------------------------------------------"<<endl;

				//erst Daten lokalisieren, dann checken, ob Direction bereits vorhanden, wenn nicht, dann erstellen
				locData = convertData(data,errCode);
			//	printData("Nach convertData, TYP 2 CONTROL: ",locData);
				

				//zuerst checken, ob data von typ 3 controlFace kommt
				if(locData.typeInfo >= 2)
				{
					//alle Daten annehmen und speichern, minimalReturnCode geben
				//	cout<<"TYPE 2 CONTROL, HABE DATEN ANGENOMMEN UND GESPEICHERT"<<endl;
					//wenn richtung andersherum als die eigene, dann blocken!
					dData test = locData;
					createDirections(test);

					if(test.isFlipped != locData.isFlipped)
					{
						//richtungen unterscheiden sich,also blocken
						storeDirection(locData,-1);
						return -1;
					}
					else
					{
						storeDirection(locData,2);
						return 1;
					}
				}
				else
				{
					tmp = isDirectionStored(locData);	//hat die daten bereits geflipped, wenn noetig
					if(tmp)
					{
						//data wurde bereits entsprechend angepasst
				//		cout<<"DATEN (TYP 2 CONTROL) GEFUNDEN"<<endl;
						data = locData;
						return slideCodes[locData.nachbarID];
					}
					else
					{
						//Directions erstellen
						
				//		cout<<"TYP 2 CONTROL: ERSTELLE DIRECTIONS"<<endl;
						createDirections(locData);
						storeDirection(locData,2);
				//		printData("ERSTELLTE TYPE TWO DIRECTIONS: ",locData);
						data = locData;
						return 2;
						
					}
				}

			}
			else
			{
				//bevor hier irgendwas passiert muss gecheckt werden, ob der Requester vielleicht dieses Face ist
				
				//cout<<"REQUEST DATA TYPE TWO: "<<"Beginne Request für ID: "<<id<<endl;
				//cout<<"----------------------------------------------------------------"<<endl;
				//cout<<"----------------------------------------------------------------"<<endl;
				locData = convertData(data,errCode);

				if(data.requesterID == id)
				{
					//dann einfach die vorhandene Direction mit Code 1 zurückschicken, ohne vorher ein anderes Faces
					//zu konsultieren
					//cout<<"REQUEST DIRECTION TYPE TWO: Bin an Ursprungface angekommen"<<endl;
					data = locData;
					return 1;
				}

				//es handelt sich um einen normalen Zweier: gleich an opposite weiterreichen, store ist zweiseitig
				
				if(errCode == -1)
					return errCode;

				
				////printData("DATA nach CONVERT: ",locData);
				code = convertToOpposite(locData);	//hier einmal checken, ob code  = -2
				//cout<<"REQUEST DATA TYPE TWO: "<<"Habe Daten auf andere Seite gebracht"<<endl;
				/*
				if(code == -2)
				{
					if(locData.typeInfo == 2)
					{
						//dann errcode für oppositeSide speichern, und gültige Type3Data ebenfalls speichern
						storeDirection(locData,code);
						storeDirection(convertData(data,errCode),2);
						return code;
					}
				}
				*/
				if(code == -1)
				{
					//converToOpposite ist fehlgeschlagen, daten werden mit errCode versehen und unverändert zurückgegeben
					//cout<<"REQUEST DATA TYPE TWO: "<<"FEHLER IN CONVERT TO OPPOSITE"<<endl;
					return -1;
				}
				
				//printData("REQUEST DATA VOR nächstem REQUEST: ",locData);
				code = nachbarn[locData.nachbarID]->requestDirections(locData);
				
				//cout<<"REQUEST DATA TYPE TWO: "<<"BEENDE Request für ID: "<<id<<endl;
				//cout<<"----------------------------------------------------------------"<<endl;
				//cout<<"----------------------------------------------------------------"<<endl;

				tmpData = convertData(locData,errCode);
				locData = tmpData;

				//printData("REQUEST DATA NACH REQUEST+CONVERT IM REQUEST: ",locData);

				storeDirection(locData,code);

				//convertToOpposite(locData);
				
				tmp = convertToOpposite(locData);
				if(tmp != -1)
				{
					//printData("DATA NACH CONVERT TO OPPOSITE: ",locData);
					storeDirection(locData,code);
				}
				else
					code = tmp;

				
				//storeDirection(locData,code);

				data = locData;
				return code;
			}
		}
	case 1:
		{
		//	cout<<"REQUEST DATA TYPE ONE: "<<"Beginne Request für ID: "<<id<<endl;
		//	cout<<"----------------------------------------------------------------"<<endl;
		//	cout<<"----------------------------------------------------------------"<<endl;
//			if(vtx.length() == 3)
//				return 0;
			
			locData = convertData(data,errCode);
				
		//	printData("DATA NACH CONVERT (TYP ONE): ",locData);

			tmp = isDirectionStored(locData);

			data = locData;
			if(tmp)
			{
		//		cout<<"TYP ONE: HABE DIRECTION GEFUNDEN !"<<endl;
				return 1;
			}
			else if(tmp == -1)
			{
				return -1;
			}
			else
			{
				//wenn direction von TypeInfo 1 oder 2, dann einfach annehmen und zurückschicken, ansonsten direction erstellen
				if( !locData.typeInfo )
				{
					createDirections(locData);
					storeDirection(locData,1);
					data = locData;
			//		printData("REQUEST: MUSSTE DIRECTION ERSTELLEN",data);
					return 1;
				}
				else
				{
					storeDirection(locData,locData.typeInfo);
					data = locData;
					return 1;
				}
			}
		}
	}

	//forbidden path
	cout<<"FACE:REQUEST DIRECTION: FORBIDDEN PATH"<<endl;
	return 0;

}

//----------------------------------------
dData	face::convertData(dData& data, int& errCode)
//----------------------------------------
{	
	//errCode: -1 = fehler und InputData zurückgeben, ansonsten 0
	//nimmt die Absoluten IDs der inputData und findet die entsprechenden LocIDs dieses Faces
	//wandelt nachbarID nicht um: dies wird nur von Convert to opposite gemacht
	int tmp;
	dData myData = data;

	//locIDs müsste er eigentlich immer finden
	myData.startLocID = findVtxIndex(myData.startAbsID)%vtx.length();
	myData.endLocID = findVtxIndex(myData.endAbsID)%vtx.length();;
	
	//da corners zu Vtx synchron sind, kann ich auch für corners LocnachbarIds nehmen
	
	//muss viel Errorchecking machen, und bei fehler abbruchCode ausgeben!
	//zB wenn cornerEdge nicht gefunden, oder ähnliche

	//die Corners müssen noch in die richtige Reihenfolge gebracht werden, damit convert to opposite 
	//richtig funzt
	//ausserdem muss gecheckt werden, ob <<isFlipped>> relativ zu diesem Face korrekt ist, wenn ja, ändern

	myData.startLocCorner = findVtxIndex(myData.startAbsCorner);

		

	myData.endLocCorner = findVtxIndex(myData.endAbsCorner);


	//Jetzt Reihenfolge der Corners überprüfen: Start muss vor End sein!
	//Ansatz: Die nächstgelegene Corner von start aus muss endcorneredgeID sein
	if( findNextValidCornerLocID(myData.startLocCorner) != myData.endLocCorner )//ev. probleme bei numCorners 2
	{
		//corners drehen
		tmp = myData.startLocCorner;
		myData.startLocCorner = myData.endLocCorner;
		myData.endLocCorner = tmp;

		tmp = myData.startAbsCorner;
		myData.startAbsCorner = myData.endAbsCorner;
		myData.endAbsCorner = tmp;

		//test: wenn corners geflipped werden, muss flippedWert umgekehrt werden
		myData.isFlipped = (1 - int(myData.isFlipped)) != 0;

		//cout<<"CONVERT DATA: "<<"Habe CornerEdges geflippt"<<endl;
		//cout<<"CONVERT DATA: "<<"StartCornerID ist jetzt "<<myData.startAbsCorner<<endl;
		//cout<<"CONVERT DATA: "<<"EndCornerID ist jetzt "<<myData.endAbsCorner<<endl;
	}

	/*
	//nun noch checken, ob flippedStatus für NachbarVtx gerechtfertigt
	//Ansatz: nächster nachbarVtx von startVtxAus 
	tmp = findLastNachbarVtx(myData.startLocID);
	if(tmp == myData.endLocID)
	{
		//also ist die Riehenfolge richtig , flipped muss false sein
		myData.isFlipped = false;
	}
	else
	{
		myData.isFlipped = true;
	}
	*/

	if(myData.isFlipped)
	{
		//wenn Punkte vertauscht sind, muss endLocId die StartPosition von nachbar sein (lokal)
		myData.nachbarID = myData.endLocID;
	}
	else
	{	//ansonsten ist nachbar bei startLocID
		myData.nachbarID = myData.startLocID;
	}

	errCode = 0;


	return myData;
}

//-------------------------------------------------
int		face::convertToOpposite(dData& dataa)
//-------------------------------------------------
{
	//Rückgabewert ist ErrCode: -2 == ploetzlich mindestens 2 Nachbarn pro CornerEdge -->OBSOLETE
							//	in dem Fall müssten die gefundenen Nachbarn analysiert und eventuell engaged werden, so dass
							//	sie nen slidewert sich selbst erstellen, alles von dieser Procedur aus

							//	-1 == keinen Nachbarn gefunden, daten bleiben unverändert, slideBlock
	//!Problematisch wirds generell, wennŽs mehrere Nachbarn an der gefundenen CornerEdge gibt
	//DIESE METHODE GEHT DAVON AUS; DAS SICH DER <<isFlipped>> WERT AKTUELL IST; SICH ALSO AUF DIESES FACES BEZIEHT 

	//WENN DATA VON MULTIVTX FACE, DANN IST DAS FUER CONVERT TO OPPPOSITE KEIN PROBLEM, DA DIESE METHODE HAUPTSÄCHLICH
	//VON CORNEREDGES AUSGEHT; WELCHE JA AUCH BEI MULTIVTX FACES KORREKT IST
	int tmp;
	int nStartVtx,nEndVtx;		//start/endVtx des gefundenen Nachbarn
	int currentCorner,nextCorner;		//alles LocIDs
	int numCNachbarn,numCVtx,numNVtx;					//zahl nachbarn der CEdge,num cornerEdgeVtx und num nachbarEdgeVtx
	//Convert to opposite entscheidet sich entsprechend der NumCorners
	switch(numCorners)
	{
		//wenn nur 2 Corners, dann ist's entweder typ 4 oder typ 1, aber typ 1 konvertiert nicht zu opposite
	case 3:
		{
			
			findOppositeCorner(dataa.endLocCorner,1,currentCorner,nextCorner);
			//wenn die gegenüberliegende Edge keinen nachbarn enthält, dann die übriggebliebene Edge nehmen
			numCVtx = getCornerEdgeVtxCount(currentCorner,nextCorner);
			numCNachbarn = getNumCornerEdgeNachbarn(currentCorner,numCVtx); 

			if(numCNachbarn == 0)
			{
				findOppositeCorner(nextCorner,1,currentCorner,nextCorner);
			}
			
			break;
		}
	case 4:
		{
			//erst die StartIDs der UEbernächsten Corners finden
			findOppositeCorner(dataa.endLocCorner,2,currentCorner,nextCorner);
			break;
		}

		//wenna bis hier kommt, dann ist's Face mit 0 oder mehr als 4 cornerEdges und muss somit sowieso geblockt werden
	default:{return -1;}
		
	}

//cout<<"CONVERT TO OPPOSITE: "<<currentCorner<<" - "<<nextCorner<<" = NewCurrent/NextCorner LocIDs"<<endl;
//cout<<"CONVERT TO OPPOSITE: "<<vtx[currentCorner]<<" - "<<vtx[nextCorner]<<" = NewCurrent/NextCorner ABSIDs"<<endl;

		numCVtx = getCornerEdgeVtxCount(currentCorner,nextCorner);
		numCNachbarn = getNumCornerEdgeNachbarn(currentCorner,numCVtx); 

//wenn mehr als 1 nachbar oder kein nachbar (bei typ 3 moeglich), dann blocken, ansonsten Start/EndVtx herausfinden
			if(numCNachbarn > 1 || numCNachbarn == 0)
			{
				//cout<<"CONVERT TO OPPOSITE: KEINE GUELTIGE OPPOSITE! ABBRUCH!"<<endl;
				return -1;
			}
			else
			{
				//erstmal start/end nachbarVtx finden
				nStartVtx = findNextValidNachbarLocID(currentCorner);
				nEndVtx = findLastNachbarVtx(nStartVtx)%vtx.length();
				numNVtx = getNachbarEdgeVtxCount(nStartVtx,nEndVtx);
				
				//cout<<"CONVERT TO OPPOSITE: "<<nStartVtx<<" - "<<nEndVtx<<" = NewStart/EndVtx LocIDs"<<endl;
				
				//zuerst  checken, ob dieser eine Nachbar genauso viele Vtx hat wie CEdge. Wenn nicht, cEdges für nachbarn anpassen
				//damit dieser die Convertierung ordentlich vornehmen kann
				//cout<<"CONVERT TO OPPOSITE: "<<numNVtx<<" - "<<numCVtx<<" = num Nachbar/Corner Vtx"<<endl;
				if(numNVtx != numCVtx)
				{
					
					currentCorner = nStartVtx;
					nextCorner = nEndVtx;
				}
				
				//wenn die direction gegen die generelle vtxRichtung giing zum Zeitpunkt der Conversion
				//dann ist sie nun richtig herum, ansonsten selbst flippen
				if(!dataa.isFlipped)
				{
					tmp = nStartVtx;
					nStartVtx = nEndVtx;
					nEndVtx = tmp;
					
					dataa.isFlipped = true;
				}
				else
				{
					dataa.isFlipped = false;
				}
				
				//jetzt noch alles dem DatenObjekt zuweisen ( WOBEI DIE LOKALEN IDS EIGENTLICH NICHT GESETZT WERDEN MUESSEN)
				//dataa.startLocID = nStartVtx;
				dataa.startAbsID = vtx[nStartVtx];
				
				
				//dataa.endLocID = nEndVtx;
				dataa.endAbsID = vtx[nEndVtx];
				//cout<<"CONVERT TO OPPOSITE: "<<vtx[nStartVtx]<<" - "<<vtx[nEndVtx]<<" = StartEndVtxABS"<<endl;
				//cout<<"CONVERT TO OPPOSITE: "<<dataa.startAbsID<<" - "<<dataa.endAbsID<<" = StartEndVtxABS laut DATA"<<endl;
				
				
				//dataa.startLocCorner = currentCorner;
				dataa.startAbsCorner = vtx[currentCorner];
				
				//dataa.startLocCorner = nextCorner;
				dataa.endAbsCorner = vtx[nextCorner];
				
				//cout<<"CONVERT TO OPPOSITE: "<<dataa.startAbsCorner<<" - "<<dataa.endAbsCorner<<" = NEUE Start/EndCorner Absolute"<<endl;
				
				
				
				//immer darauf achten, das die nachbarID die richtige lokale startID erhält, welche immer in reihenfolge sein muss
				if(dataa.isFlipped)
					dataa.nachbarID = nEndVtx;
				else
					dataa.nachbarID = nStartVtx;
				
				//cout<<"CONVERT TO OPPOSITE: "<<dataa.nachbarID<<" = Data.nachbarID(LOC) nach opposite"<<endl;
				
				return 0;
			}


}


//--------------------------------------------------------
int		face::guessNumTriangles(int type,int vtxCount)
//-------------------------------------------------------
{
	//alle teilen einen Vtx
	if(type == 4 || type == 0)
	{
		return	(vtxCount - 1) % 2;
	}
	//teilen zwei Vtx
	if(type - 2)
		return	(vtxCount - 2) % 2;

	//weitere Typen folgen ... doch nicht
	
	//forbidden path
	cout<<"GUESS NUM TRIANGLES: FORBIDDEN PATH"<<endl;
	return 0;
}

#ifdef DEBUG

//----------------------------------------------
void	face::printData(MString prefix,dData& data)
//----------------------------------------------
{
	cout<<prefix.asChar()<<data.startAbsID<<" = StartID"<<endl;
	cout<<prefix.asChar()<<data.endAbsID<<" = EndID"<<endl;
	cout<<endl;
	cout<<prefix.asChar()<<data.startAbsCorner<<" = StartCorner"<<endl;
	cout<<prefix.asChar()<<data.endAbsCorner<<" = EndCorner"<<endl;
	cout<<endl;
	cout<<prefix.asChar()<<data.isFlipped<<" = isFlipped"<<endl;
	cout<<prefix.asChar()<<nachbarIDs[data.nachbarID]<<" = NachbarID"<<endl;
	cout<<prefix.asChar()<<data.typeInfo<<" = TypeInfo"<<endl;
	cout<<"End of DataPrint"<<endl;
}

#endif
