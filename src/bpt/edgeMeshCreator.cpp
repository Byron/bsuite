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

// edgeMeshCreator.cpp: implementation of the edgeMeshCreator class.
//
//////////////////////////////////////////////////////////////////////

#include "edgeMeshCreator.h"
#include "edge.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


edgeMeshCreator::edgeMeshCreator(	MObject&		fMesh,
									double			inSlide, 
									MDagPath&		meshPath
																	)
								 
								 :

																meshCreator(fMesh,
																			inSlide, 
																			meshPath)
																			
														
{
}

edgeMeshCreator::~edgeMeshCreator()
{
}


//----------------------------------------------------------------------------------------------------------------
void		edgeMeshCreator::updateFaceData(edgeFaceData* faceData)
//----------------------------------------------------------------------------------------------------------------
{
	//stattet dieses FaceDataObjekt mit aktuellen Daten aus: UVs, verts, normals
	getFaceVtxIDs(faceData->id, faceData->faceVertices);


		//die UVs müssen aktualisiert werden, für den Fall, dass jetzt ein neuer RefVtx verwendet wird
		
		
	faceData->UVIndices.clear();
	getFaceUVs(faceData->id, faceData->UVIndices, faceData->UVRelOffsets);
	
	//die AbsOffsets müssen auch neu aufgebaut werden - ist erst bei mehreren UVSets noetig, UVSet 0 hat immer Offset von 0
	UINT l = faceData->faceVertices.length();
	UINT count = 0;
	
	for(UINT i = 1; i < numUVSets; i++)
	{
		faceData->UVAbsOffsets[i] = count;;
		count += (int)faceData->UVRelOffsets[i] * l;
	}
	
	

	//auf useNormals muss nicht gecheckt werden, da das von dieser prdur gemacht wird
	getFaceNormalIDs(faceData->id, faceData->normalIndices);



}


//----------------------------------------------------------------------------------------------------------------
void	edgeMeshCreator::getFaceTriangles(int faceID, MIntArray& outTriVtx, MPointArray& outTriPoints)
//----------------------------------------------------------------------------------------------------------------
{

	polyIter.setIndex(faceID, tmp);


	polyIter.getTriangles(outTriPoints, outTriVtx);

}


//----------------------------------------------------------------------------------------------------------------
void	edgeMeshCreator::changeEndFace(int origID,int newID[],int directions[], edgeFaceData* faceData[2])
//----------------------------------------------------------------------------------------------------------------
{


	MPRINT("CHANGE_END_FACE: QUADS")

	//DIESE PROZEDUR ERZEUGT EIN QUAD BEIM SOLIDCHAMFER ALS ENDFACE
	
	//der edgeFaceData* zeigt auf Array von 2 faces, welchea Daten über normalIndices und UVs enthält

	MIntArray newFaceIDs;



	int refVtx;

	int d[2] = {-directions[0], -directions[1]};
	
	int newRefVtx[2] = {0,0};	//als speicher für beide RefVtx

	//suchen nach vtxMatch
	//MERKE: Man muss 2 suchen durchführen -> Einmal bei insertVtx, und einmal hier, um sicherzugehen, dass man den richtigen RefVtx hat.
	//allerdings ist die Suche hier vereinfacht, da einige Parameter von der letzten Suche gespeichert wurden
	UINT l,i;

	for(UINT y = 0; y < 2; y++)
	{

		

		l = faceData[y]->faceVertices.length();
		for(i = 0; i < l; i++)
		{
			
			if(faceData[y]->faceVertices[i] == origID)
			{
				
				
				newRefVtx[y] = faceData[y]->faceVertices[(i+d[y]+l) % l];	
				//zuerst neues Face erstellen
				
				break;
			}
		}
	}
	

	
	//wenn die beiden RefVtx nicht gleich sind, dann muss trianguliert werden
	if(newRefVtx[0] != newRefVtx[1])
	{
		for(UINT i = 0; i < 2; i++)
		{//die direction muss hier wieder umgekehrt werden, weil change endface davon ausgeht, dass dir in originalRichtung zeigt
			changeEndFace(origID, newID[i], -d[i], faceData[i]);	//der Alte RefVtx muss übergeben werden, weil diese prozedur ihgn ja auch nochmal suchen und finden muss
		}
		
		return;
	}
	else
		refVtx = newRefVtx[0];
	
	
	//newFaceIDs.append(vtxIDs[(i+l+d*1)%l]); //vvtx davor hinzufügen 
	newFaceIDs.append(refVtx);	
	newFaceIDs.append(newID[0]);	//newID
	newFaceIDs.append(origID); //diesen Vtx hinzu
	newFaceIDs.append(newID[1]);
	
	
	//jetzt erstma das neue Face erstellen auf die herkoemmliche variante
	//merke:dies ist abhängig von der Art der UVs zwischen den Faces. Wenn also keine passeden UVs vorhanden, dann muss 
	//der triangulations algo gerufen werden, weshalb hier dann kein face erstellt werden darf

//Wenn die UVs der beiden Faces zusammenhängen, dann kann man das UVFace für das neue Quad direkt dazwischen platzieren
		//Wenn die Faces getrennt sind, oder eines der Beiden keine eigenen UVs hat, dann muss von hier aus die triangulate methode afgerufen
		//werden, um doch noch gültige UVs und Faces zu erstellen
		
	
		UINT u, r;
		
		
		
		
		MIntArray origUVs[2], refUVs[2];	//hält die UVs der entsprechenden Vertizen
		
		//jetzt erstmal herausfinden, ob die UVs in jedem UVSet geshared sind. Wenn nicht, dann triangulieren
		origUVs[0] = faceData[0]->getEdgeVtxUVs(origID);
		origUVs[1] = faceData[1]->getEdgeVtxUVs(origID);
		
		refUVs[0] = faceData[0]->getEdgeVtxUVs(refVtx);
		refUVs[1] = faceData[1]->getEdgeVtxUVs(refVtx);
		
		
		UINT x;
		for(x = 0; x < numUVSets; x++)
		{
			if(		(origUVs[0][x] != origUVs[1][x])
				||
				(refUVs[0][x] != refUVs[1][x] )		)
			{//sie stimmen in einem UVSet nicht überein, alo triangulieren
				
				for(u = 0; u < 2; u++)
					changeEndFace(origID, newID[u], -d[u], faceData[u]);	
				
				
				//abbruch
				return;
				
			}
			
		}
		
		//jetzt die UVs von jedem der neiu verwendeten Vertizen holen. die erstn 3 kann an von face0 nehmen, den letzten muss das face 1 haben
		MIntArray UVSave[4];
		
		UVSave[0] = faceData[0]->getEdgeVtxUVs(newFaceIDs[0]);
		UVSave[1] = faceData[0]->getEdgeVtxUVs(newFaceIDs[1]);
		UVSave[2] = faceData[0]->getEdgeVtxUVs(newFaceIDs[2]);
		UVSave[3] = faceData[1]->getEdgeVtxUVs(newFaceIDs[3]);
		
		
		MIntArray tNewUVs(4);	//tempSpeicher für die neuen UVs
		
		//jetzt müssen die erstellten UVs pro UVSet angehängt werden
		//ITERATOREN
		std::list<MIntArray>::iterator	UVCountsIter	=		UVCounts->begin();
		std::list<MIntArray>::iterator	UVAbsIter		= 		UVAbsOffsets->begin();		
		std::list<MIntArray>::iterator  UVIDsIter		=		UVperVtx->begin();
		
		//durch jedes UVSet gehen
		for(x = 0; x < numUVSets; x++,UVCountsIter++,UVAbsIter++,UVIDsIter++)
		{//wenn eine der beiden faceUVs nicht definiert, abbruch
			if( !(faceData[0]->UVRelOffsets[x] & faceData[1]->UVRelOffsets[x]) )
			{//keine UVs vorhanden, also die Arrays aktualisieren screiben und mit dem nächsten UVSet fortfahren
				
				(*UVCountsIter).append(0);
				(*UVAbsIter).append( (*UVAbsIter)[ (*UVAbsIter).length()-1 ] );	//hier eventuell ausschliessen, dass .length 0 ist
				
				
				continue;
			}
			
			
			r = 0;
			
			tNewUVs[r++] = UVSave[0][x];
			tNewUVs[r++] = UVSave[1][x];
			tNewUVs[r++] = UVSave[2][x];
			tNewUVs[r++] = UVSave[3][x];
			
			//tnewIDs umkehren, wenn noetig
			if(d[0] == -1)
				helper.invertArray(tNewUVs);
			
			
			//-------------------
			//noch als eintragen
			//-------------------
			
			r = (*UVIDsIter).length();	// = marker für anfang der neuen UVs
			
			(*UVIDsIter).setLength(r + 4);
			
			
			
			
			for(u = 0; u < 4; u++)
				(*UVIDsIter)[r++] = tNewUVs[u];
			
			
			//man kann hier absolute Zahlenwerte nehmen, da er hier auf jeden Fall für jedes Face die richtigen UVs ha
			(*UVCountsIter).append(4);
			(*UVAbsIter).append( (*UVAbsIter)[ (*UVAbsIter).length()-1 ] + 4);	//hier eventuell ausschliessen, dass .length 0 ist
			
			
			
			
		}//for(UINT x = 0; x < numUVSets; x++)
		
		
		
		//wenn er hier hin kommt, ist mit den UVs alles in ordnung, und man kann das Face erstellen
		if(d[0] == -1)
		{
			helper.invertArray(newFaceIDs);
		}
		
		meshCreator::createPoly(newFaceIDs);
		
		
		
		
	
	
//Vorgehen: Die normalen für de newVtx holen in jeweils dem korrekten faceDataObject - er ist als edgeVtx zu suchen.
		//dann die normaleninfo für die restlichen vtx holen, die in einem FDObjekt vorhanden sein sollten
		
		UINT locID;
		MIntArray normalIndices;	//speicher für die neuen normalIndices für`s face
		
		INVIS(helper.printArray(newFaceIDs, " = newFaceIDs für Normalen"));
		
		if(d[0] == 1)
		{//zuerst den RefVtx suchen in einem face suchen 
			if(helper.getLocIndexMod2(origID, faceData[0]->normalIndices,locID))
			{//origID bis refVtx ist hardEdge - also ist refVtx zu newIDs auch hardEdge
				normalIndices.append(refVtx);
				normalIndices.append(newID[0]);
				
				normalIndices.append(newID[1]);
				normalIndices.append(refVtx);
			}
			
			//nun die edges vom origVtx zu newVtx hinzufügen - dies ist legitim, wenn der newVtx im normalindicesArray zu finden ist
			//(das bedeutet implizit, dass die edge, auf dem der newVtx eingefügt wurde, hard gewesen ist
			if(helper.getLocIndex(newID[0], faceData[0]->normalIndices, locID))
			{
				normalIndices.append(newID[0]);
				normalIndices.append(origID);
			}
			
			if(helper.getLocIndex(newID[1], faceData[1]->normalIndices, locID))
			{
				normalIndices.append(origID);
				normalIndices.append(newID[1]);
			}
			//fertig
		}
		else
		{//face ist falsch herum
			if(helper.getLocIndexMod2(refVtx, faceData[0]->normalIndices,locID))
			{//origID bis refVtx ist hardEdge - also wie oben, nur vertauscht
				normalIndices.append(newID[0]);
				normalIndices.append(refVtx);
				
				normalIndices.append(refVtx);
				normalIndices.append(newID[1]);
			}
			
			//nun die edges vom origVtx zu newVtx hinzufügen - dies ist legitim, wenn der newVtx im normalindicesArray zu finden ist
			//(das bedeutet implizit, dass die edge, auf dem der newVtx eingefügt wurde, hard gewesen ist
			if(helper.getLocIndex(newID[0], faceData[0]->normalIndices, locID))
			{
				normalIndices.append(origID);
				normalIndices.append(newID[0]);
			}
			
			if(helper.getLocIndex(newID[1], faceData[1]->normalIndices, locID))
			{
				normalIndices.append(newID[1]);
				normalIndices.append(origID);
			}
			//fertg
			
		}
		
		
		INVIS(helper.printArray(normalIndices, " = NewnormalINdices"));
		//jetzt müssen die normalen natürlich noch eingetraen werden
		helper.addIntArrayToLHS(normalVertexIndices, normalIndices);
		normalCount.append(normalIndices.length());
		normalAbsOffsets.append(normalAbsOffsets[normalAbsOffsets.length()-1] + normalIndices.length());
		
		
		
	
	
	//jetzt die originalVtx aus den Faces entdernen
	removeVtx(origID, faceData[0]->id);
	removeVtx(origID, faceData[1]->id);

		
			
}



//-----------------------------------------------------------------------------------
void	edgeMeshCreator::changeSideFace(int origVtx, int newVtxID, int faceID, MIntArray& newUVIds)
//-----------------------------------------------------------------------------------
{

	INVIS(cout<<"BIN IN CHANGE SIDE FACE: ÄNDERE: "<<faceID<<endl<<endl);
	
	//BEARBEITE ORIGVTX

	int offset = getValidAbsoluteOffset(faceID);
	int count = offset + (*offsets)[faceID];
	
	int newVtxLocID;		//hält den faceRelativen index des newVtxID, wird von UVs benoetigt, um schneller arbeiten zu koennen
	//ZUERST DEN ORIGVTX IN FACEARRAY MIT DEM NEWVTX TAUSCHEN
	int i;
	for(i = offset; i < count; i++)	
	{
		if(	(*faceVtxIDs)[i] == origVtx)
		{
			(*faceVtxIDs)[i] = newVtxID;
			newVtxLocID = i - offset;
			break;	//Abbruch, da origVtx eh nur einmal im Face vorhanden ist
		}
	}


	//BEARBEITE NORMALEN
	
	
	offset = getValidAbsNormalOffset(faceID);
	count = offset + normalCount[faceID];
	
	//JEDES VORKOMMEN VON ORIGVTX DURCH NEWVTX ERSETZEN
	for(i = offset; i < count; i++)
	{
		if(normalVertexIndices[i] == origVtx)
			normalVertexIndices[i] = newVtxID;
		//hier kein abbruch, da origvtx im indices array mehrfach vorhanden sein kann je face
	}
	


	//UVS BEARBEITEN



	//newUVIDs wie folgt kodiert: es hat numUVSets einträge, und -1, wenn face dort kein UVs hat
	
	//jetzt jede UV in jedem UVSet verändern, wenn moeglich
	
	//iteratoren
	std::list<MIntArray>::iterator iterCounts			= UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset			= UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT				= UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues		= UVoffsetsLUTValues->begin();
	std::list<MIntArray>::iterator iterUVIDs			= UVperVtx->begin();
	
	
	
	
	for(i = 0; i < numUVSets; i++)
	{
		//wenn face überhaupt UVs hat
		if( (*iterCounts)[faceID] != 0)
		{
			
			//faceUVs verändern
			offset = getValidAbsoluteUVOffset(faceID,i,iterOffset,iterLUT,iterLUTValues);
			
			(*iterUVIDs)[offset + newVtxLocID] = newUVIds[i];
			
		}
		
		
		
		++iterCounts;
		++iterOffset;
		++iterLUT;
		++iterLUTValues;
		++iterUVIDs;
		
	}
	
	




}

//---------------------------------------------------------------------------------------------------------------------------------------------
void	edgeMeshCreator::changeEndFace(int origID,int newID, int direction, edgeFaceData* faceData)
//---------------------------------------------------------------------------------------------------------------------------------------------
{

	//der edgeFaceData* zeigt auf ein face, welches Daten über normalIndices und UVs enthält

	MIntArray newFaceIDs;



	MPRINT("CHANGE_END_FACE: Triangulate")



	int d = -direction;


	INVIS(cout<<"OrigID: "<<origID<<endl;)
	INVIS(cout<<"newVtx: "<<newID<<endl;)

	INVIS(cout<<"faceID: "<<faceData->id<<endl;)

	
	if(newID == origID)
	{
		return;
	}


	
	//referenzface holen um herauszufinden, ob sich der refVtx verschoben hat
	
	
	//suchen nach vtxMatch
	int refVtx;

	UINT l = faceData->faceVertices.length();
	

	for(UINT i = 0; i < l; i++)
	{
	//	if(faceData->faceVertices[i] == origID)
		if(faceData->faceVertices[i] == origID)
		{

			
			
			refVtx = faceData->faceVertices[(i+d+l) % l];
	
			

			break;
			

		}
	}

				
	
	
				newFaceIDs.append(newID);
				newFaceIDs.append(origID); //diesen Vtx hinzu
				newFaceIDs.append(refVtx); //RefVtx
				
				if(d == -1)
				{
					//array umkehren
					helper.invertArray(newFaceIDs);
				}
				
				
				
				createPoly(newFaceIDs, faceData);	//alles noetige bezüglich der normalen und UVs wird von dieser Methode übernommen
				
				
				//jetzt noch den originalVtx entfernen: Man muss bedenken, dasss der NewVtx bereits ins originalFace eingefügt wurde
				removeVtx(origID, faceData->id);
				


}


//----------------------------------------------------------------------------------------------
void	edgeMeshCreator::insertVtx(const edgeFaceData* faceData, int origID, int newID, int direction)
//------------------------------------------------------------------------------------------------
{
	//faceData dient im endEffekt nur als referenz für die eventuell vorhandenen UVs

	
	//im Falle von Chamfer kann es sein, das die neue ID == origID ist, also diesen Fall gar nicht erst bearbeiten
	if( !(newID > initialVtxCount) )
		return;
	

	//in diesem Fall müssen die aktuellen FaceDaten verwendet werden, und der neue Vtx muss richtig eingefügt werden
	edgeFaceData tmpFace(faceData->id);
				
	//jetzt die Daten setzen 
	getFaceVtxIDs(tmpFace.id,tmpFace.faceVertices); 
				
	//die prozedur checkt, ob normalen überhaupt gewünscht;
	getFaceNormalIDs(tmpFace.id, tmpFace.normalIndices);

	//aber zuerst mal die Richtung bestimmen und den RefVtx anpassen

	UINT l = faceData->faceVertices.length() ;

		
				//okay, tmpFace ist mit gegebwärtiger edgeData gefüllt. 
				//vtx einfügen -> zuerst den origVtx finden
				
				bool faceDir = (direction == 1) ? true : false;


								
				
				//arrays allokalisieren, wenn UVs verwendet werden
				tmpFace.UVRelOffsets = (bool*)malloc(numUVSets);
				memset(tmpFace.UVRelOffsets, 0, numUVSets);
				
				tmpFace.UVAbsOffsets = (unsigned long*)malloc(numUVSets * 4);
				memset(tmpFace.UVAbsOffsets, 0, numUVSets * 4);
				
				
				
				getFaceUVs(tmpFace.id, tmpFace.UVIndices, tmpFace.UVRelOffsets );
				
				l = tmpFace.faceVertices.length();
				
				
				UINT count = 0;
				
				for(UINT x = 0; x < numUVSets; x++)
				{
					tmpFace.UVAbsOffsets[x] = count;
					
					count += (int)tmpFace.UVRelOffsets[x] * l ;
				}
				
				//methode hat den nachteil, dass sie den ReVtx nochmal auf`s neue sucht, aber was solls
				
				//erst jetzt den Vtx hinzufügen, damit die UVs aktualisiert werden koennen
				//tmpFace.insertEdgeVtx(origID, refVtx, newID, inFaceDir);	//diese Prozedur kümmert sich auch um de Normalen 
				tmpFace.insertEdgeVtx(origID, newID, faceDir);	
				
				//die UVs für den neen Vtx aus faceData extrahieren und in tmpFaceData einfügen
				//jetzt die UVs hinzufügen zu tmpData
				MIntArray tmpArray = faceData->getEdgeVtxUVs(newID);
				tmpFace.assignUV(newID, tmpArray );

				
				//jetzt einfach die arbeit machen lassen
				changePolyVtxIDs(&tmpFace);	
				
				INVIS(helper.printArray(tmpFace.faceVertices, " == tmpFaceFACEVERTICES"));
}


//-----------------------------------------------------------------
void	edgeMeshCreator::removeVtx(int vtxID, int faceID)
//-----------------------------------------------------------------
{
	int offset = getValidAbsoluteOffset(faceID);

	int aOffset = offset + (*offsets)[faceID];

	int vtxLocID;	// == der faceRelative index des Vtx

	bool found = false;

	//jetzt den vtx finden:
	UINT i;
	for(i = offset; i < aOffset; i++)
	{

		if((*faceVtxIDs)[i] == vtxID)
		{
			vtxLocID = i - offset;
			
			faceVtxIDs->remove(i);

			found = true;

			break;
		}

	}

	if(!found)
	{
		INVIS(cout<<"WARNUNG: HABE VTX "<<vtxID<<" NICHT GEFUNDEN. KONNTE IHN NICHT ENTFERNEN. ABBRUCH!"<<endl);
		return;
	}

	//aktualisieren der LUT Arrays
	(*offsets)[faceID]--;
	updateOffsetLUT(faceID, -1);


//wenn vtx eine Kante beginnt, wird die ganze Kante entfernt, 
		//wenn er nur teil einer Kante ist, dann wird er durch den nächsten Vtx aus normalIndices erstetz
	
	MIntArray normalIDs;
	getNormalsIndices(faceID, normalIDs);
	
	if(normalIDs.length() > 0)
	{
		
		UINT locID;
		
		//erstmal die durch vtx erzeugt hardEdge entfernen, wenn moeglich
		if(helper.getLocIndexMod2(vtxID,normalIDs, locID))
		{
			normalIDs.remove(locID);
			normalIDs.remove(locID % normalIDs.length());
		}
		
		//jetzt alle vorkommnisse innerhalb einer edge entfernen
		UINT l = normalIDs.length();
		for( i = 1; i < l; i+=2)
		{
			if(normalIDs[i] == vtxID)
			{
				normalIDs[i] = normalIDs[(i+1)%l];
			}
		}
		
		//jetzt die aktualisierten normalIDs ins grosse" array übertragen
		updateNormalArraysWith(normalIDs, faceID);
		
		//fertig
	}
	

	//UVs
	
	UINT l;

	//ITERATOREN
	std::list<MIntArray>::iterator iterCounts		=		UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset		=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT			=		UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues	=		UVoffsetsLUTValues->begin();
	
	std::list<MIntArray>::iterator iterUVIDs		=		UVperVtx->begin();
	
	
	for(i = 0; i < numUVSets;i++)
	{
		
		if((*iterCounts)[faceID] == 0)
			continue;
		
		l = (*iterCounts)[faceID];
		
		offset = getValidAbsoluteUVOffset(faceID, i, iterOffset, iterLUT, iterLUTValues);
		
		aOffset = offset + l;
		
		
		
		
		//UV entfernen (mithilfer der locID
		
		iterUVIDs->remove(offset + vtxLocID);
		
		
		
		updateOffsetUVLUT(faceID,-1,i,iterLUT,iterLUTValues);
		(*iterCounts)[faceID]--;
		
		
		//ITERATOREN ERHÖHEN
		++iterCounts;
		++iterUVIDs;
		
		++iterLUT;
		++iterLUTValues;
		++iterOffset;
		
	}
	
	
	
	

}

//-----------------------------------------------------------------
void	edgeMeshCreator::changePolyVtxIDs(const edgeFaceData* faceData)
//-----------------------------------------------------------------
{
	int faceID = faceData->id;

	UINT l = (*offsets)[faceID];

	int change = faceData->faceVertices.length() - l;	//change muss in jedem Fall bereits hier gesetzt werden, da es nach der changeVtx prozedur sowieso gleich faceVertices.length() ist


	//diesse Prozedur nimmt die Daten aus der FaceData und aktualisiert damit das Face im grossen DatenArray

	meshCreator::changePolyVtxIDs(faceData->id,faceData->faceVertices);

	//normalen aktualisieren

	meshCreator::updateNormalArraysWith(faceData->normalIndices, faceData->id);

	
	
	//jetzt noch die UVs aktualisieren
	
	//die UVs ebenfalls übernehmen
	
	int offset, aOffset, r, k;
	
	
	l = faceData->faceVertices.length();
	
	
	//ITERATOREN
	std::list<MIntArray>::iterator iterCounts		=		UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset		=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT			=		UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues	=		UVoffsetsLUTValues->begin();
	
	std::list<MIntArray>::iterator iterUVIDs		=		UVperVtx->begin();
	
	
	for(UINT i = 0; i < numUVSets;i++)
	{
		
		if((*iterCounts)[faceID] == 0)
			continue;
		
		
		
		offset = getValidAbsoluteUVOffset(faceID, i, iterOffset, iterLUT, iterLUTValues);
		
		aOffset = offset + l;
		
		
		
		//jetzt noch den Arraybereich anpassen
		if(change > 0)
		{
			for(int k = 0; k < change; k++)
				(*iterUVIDs).insert(-1,offset);
		}
		else if(change < 0)
		{
			for(int k = 0; k > change; k--)	
				(*iterUVIDs).remove(offset);
		}
		
		//neue ergebnisse übertragen
		r = faceData->UVAbsOffsets[i];							
		for(k =offset; k < aOffset;k++)
		{
			(*iterUVIDs)[k] = faceData->UVIndices[r++];
		}
		
		
		//LUT aktualisieren
		if(change != 0)
		{
			updateOffsetUVLUT(faceID,change,i,iterLUT,iterLUTValues);
			(*iterCounts)[faceID] += change;
		}
		
		//ITERATOREN ERHÖHEN
		++iterCounts;
		++iterUVIDs;
		
		++iterLUT;
		++iterLUTValues;
		++iterOffset;
		
	}
	
	INVIS(cout<<"CHANGE FACE ID: "<<faceData->id<<endl<<endl);

	INVIS(helper.printArray(faceData->faceVertices, " = VTX IN CHANGEPOLY"));
	INVIS(helper.printArray(faceData->UVIndices, " = UVINDICES IN CHANGEPOLY"));
	
	
	
}


//-----------------------------------------------------------------
void	edgeMeshCreator::createPoly(const edgeFaceData* data)
//-----------------------------------------------------------------
{
	//diese prozedur erstellt faces direkt anhand der faceData - dies erspart den lookup

	INVIS(helper.printArray(data->faceVertices, " = FACE VERTICES CREATE POLY CSF"));

	INVIS(helper.printArray(data->UVIndices, " = UVIDS CREATE POLY CSF"));

	//VTXARRAY UPDATEN
	
	(*offsets).append(data->faceVertices.length());
	helper.addIntArrayToLHS(*faceVtxIDs,data->faceVertices);
	

	offsetsAbsolute->append( (*offsetsAbsolute)[ offsetsAbsolute->length() - 1 ] + data->faceVertices.length() );

	

	//UVS BEARBEITEN
	
	
	UINT l = data->faceVertices.length(), l2;
	
	
	//ITERATOREN
	std::list<MIntArray>::iterator UVCountsIter		=		UVCounts->begin();
	std::list<MIntArray>::iterator UVAOIter			=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator UVIDIter			=		UVperVtx->begin();			
	
	UINT r, x;
	
	for(UINT a = 0; a < numUVSets; a++)
	{
	
		
		//wenn das face dieses UVSets nicht gemapt ist, abbruch
		if(!data->UVRelOffsets[a])
		{//vorm abbruch aber boch die OffsetsArrays aktualisieren und erhoehen
			(*UVCountsIter).append(0);
			(*UVAOIter).append( (*UVAOIter)[((*UVAOIter).length()-1)] );
			
			UVCountsIter++;
			UVAOIter++;
			
			continue;
		}
		
		//die eigentlichen UVs für dieses Face extrahieren
		r = 0;
		MIntArray newUVs(l);
		l2 = data->UVAbsOffsets[a] + l;
		for(x = data->UVAbsOffsets[a]; x < l2; x++)
		{
			newUVs[r++] = data->UVIndices[x];
		}
			
		//jetzt noch die neuen UVs hinzufügen, gleichzeitig die Iters erhoehen
		helper.addIntArrayToLHS(*UVIDIter, newUVs);
		(*UVCountsIter).append(l);
		(*UVAOIter).append( (*UVAOIter)[((*UVAOIter).length()-1)] + l );
		
		UVIDIter++;
		UVCountsIter++;
		UVAOIter++;
		
	}

	//NORMALEN BEARBEITEN
	//da die arrays in sync bleiben müssen, hänge ich selbst an, wenn die normalIDs 0 sind
		
	helper.addIntArrayToLHS(normalVertexIndices, data->normalIndices);
	normalCount.append(data->normalIndices.length());
	normalAbsOffsets.append( normalAbsOffsets[(normalAbsOffsets.length() - 1 )] + data->normalIndices.length() );
	
	

}


//-----------------------------------------------------------------
void	edgeMeshCreator::createPoly(MIntArray& vtxIDs, edgeFaceData* faceData)
//-----------------------------------------------------------------
{
	//Diese Methode geht davon aus, dass das VtxArray richtig geordnet
	//ist und diese Vtx im VtxPointsArray existieren

	//isFlipped ist für normalindices wichtig

	//isSmallFace hilft bei der erzeugung guter hardEdges
	//faceData zeigt auf das entsprechende Face, in dem das neue liegen soll. Demnach darf hier kein Index verwendet werden

	//die faceData wird verwendet, um die entsprechenden normal und UVDaten zu erhalten (LUT mässig)

	//FACE ERSTELLEN

	(*offsets).append(vtxIDs.length());
	helper.addIntArrayToLHS(*faceVtxIDs,vtxIDs);
	

	offsetsAbsolute->append( (*offsetsAbsolute)[ offsetsAbsolute->length() - 1 ] + vtxIDs.length() );


	//UVS BEARBEITEN

	
	
	//durch VtxArray parsen, und zwischen neuen und alcten Vtx unterscheiden, dann die entsprechenden Daten heraussuchen
	UINT x, r = 0, vl = faceData->faceVertices.length();			//anderer counter
	int cVtx;		//tmpVariable für vtx aus vtxIDs -> currentVtx
	
	//ITERATOREN
	std::list<MIntArray>::iterator UVCountsIter		=		UVCounts->begin();
	std::list<MIntArray>::iterator UVAOIter			=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator UVIDIter			=		UVperVtx->begin();			
	
	
	
	for(UINT a = 0; a < numUVSets; a++)
	{
		r = 0;
		
		//wenn das face dieses UVSets nicht gemapt ist, abbruch
		if(!faceData->UVRelOffsets[a])
		{//vorm abbruch aber boch die OffsetsArrays aktualisieren und erhoehen
			(*UVCountsIter).append(0);
			(*UVAOIter).append( (*UVAOIter)[((*UVAOIter).length()-1)] );
			
			UVCountsIter++;
			UVAOIter++;
			
			continue;
		}
		
		MIntArray newUVs(vtxIDs.length());	//hält die neu erstellten UVs, initialisiert mit 0, was vorteile hat, da er nicht abkackt bei fehlern
		
		UINT l = vtxIDs.length();	
		for(UINT i = 0; i < l; i++)
		{
			cVtx = vtxIDs[i];
			
			//man muss jetzt beide Arrays nach dem entsprechenden Vtx durchsuchen.
			//man kann hier nicht direkt nach newVtx und edgeVtx unterteilen, da sie beide Ids haben
			//die groesser sind als der initialVtxCount
			
			
			
			
			for(x = 1; x < faceData->vtxChange.length(); x+=2)
			{//vtx im changeArray finden
				if(faceData->vtxChange[x] == cVtx)
				{//x muss gerade sein, damit es auch die UVIndices holen kann, und dann durch 2 geteilt
					newUVs[r++] = faceData->UVChange[--x * numUVSets + (a*2) + 1];	//muss wirklich getestet werden, ob das funzt
					goto outOfInnerLoop;
				}
				
			}
			
			
			
			for(x = 0; x < vl; x++)
			{
				if(faceData->faceVertices[x] == cVtx)
				{
					newUVs[r++] = faceData->UVIndices[faceData->UVAbsOffsets[a]+x];
					goto outOfInnerLoop;
				}
			}
			
outOfInnerLoop:;
			   
			   
		}
		
		INVIS(helper.printArray(newUVs, " == NEWUVS FUERS GERADE ERSTELLTE FACE"));
		//jetzt noch die neuen UVs hinzufügen, gleichzeitig die Iters erhoehen
		helper.addIntArrayToLHS(*UVIDIter, newUVs);
		(*UVCountsIter).append(l);
		(*UVAOIter).append( (*UVAOIter)[((*UVAOIter).length()-1)] + l );
		
		UVIDIter++;
		UVCountsIter++;
		UVAOIter++;
		
	}
	


	//NORMALEN BEARBEITEN
	if(faceData->normalIndices.length() > 0)
	{
		//jetzt mit helper die NormalIDs durchsuchen nach entsprechenden kanten, und diese dann ins
		//array übertragen und dieses letztendlich hinzufügen
		//MERKE: Die IDs müssen bereits in der richtigen Richtung sein (relativ zum original Face), was hier eigentlich gegeben ist
		MIntArray newNIDs;
		UINT locID;
		


		//man müsste hier noch die newVtxs herausfiltern, den entsprechenden edgeVtx finden, und dann schauen
		//ob die entsprechende angrenzende edge im originalFace hard ist. Wenn ja, dann sollen alle IDs, die mit dem newVtx
		//verbunden sind, auch hard sein
		/*
		MIntArray newVtxIDs;	//referenziert in das VtxIDs array und zeigt auf newVtxIDs (die, die durch intersection entstanden sind)
		UINT l = vtxIDs.length();
		for(UINT i = 0; i < l; l++)
		{

		}
		*/
		
		UINT l = vtxIDs.length();

		UINT i;
		for(i = 0; i < l; i++)
		{
			int cVtx = vtxIDs[i]; //==  currentVtx als tmpSpeicher
			//erstmal checken, ob dieser Vtx im vtxChangeArray verzeichnet ist
/*			UINT l2, x;
			l2 = faceData->vtxChange.length();
			for(x = 0; x < l2; x++)
			{
				if(faceData->vtxChange[x] == vtxIDs[i])
				{//er ist im vtxchangearray - jetzt also den originalVttx (falls er es nicht schon ist), im normalIndices array finden
					bool isOrigVtx = (x % 2 == 0) ? true : false;
					
					if(isOrigVtx && helper.getLocIndexMod2(cVtx, faceData->normalIndices, locID) )
					{//origVtx wurde gefunden - also die normalIDs hinzufügen
						newNIDs.append(cVtx);
						newNIDs.append(vtxIDs[(i+1)%l]);
					}
					else if(helper.getLocIndexMod2(faceData->vtxChange[x-1], faceData->normalIndices, locID))
					{//es ist newVtx 
						newNIDs.append(cVtx);
						newNIDs.append(vtxIDs[(i+1)%l]);
					}

					continue;
				}

			}
*/
			if(helper.getLocIndexMod2(cVtx, faceData->normalIndices, locID) && faceData->normalIndices[locID+1] == vtxIDs[(i+1)%l])
			{
				newNIDs.append(faceData->normalIndices[locID]);
				newNIDs.append(faceData->normalIndices[locID+1]);
			}

		}

		//jetzt das Resultat noch in Arrays packen
		helper.addIntArrayToLHS(normalVertexIndices, newNIDs);
		normalCount.append(newNIDs.length());
		normalAbsOffsets.append( normalAbsOffsets[(normalAbsOffsets.length() - 1 )] + newNIDs.length() );

	}
	else
	{
		//ansonsten muss eine nullrunde einglegt werden, damit die Arrays in sync bleiben
		
		normalCount.append(0);
		normalAbsOffsets.append( normalAbsOffsets[(normalAbsOffsets.length() - 1 )]);
	}

}


//--------------------------------------------------------------------
void	edgeMeshCreator::recreateOriginalFace(edgeFaceData* origFace)
//--------------------------------------------------------------------
{
	//in dieser Prozedur wird das OriginalFace (siehe FaceID der FaceData) anhand der newVtx und der UVs in UVChange wiederhergestellt
	
	//man koennte auch die changeSideFace prozedur verwenden, allerdings kann diese pro aufruf nur einen Vtx ändern, und würde deshalb bei jedem ruf
	// die getValidAbs offset prozeduren verwenden, was Zeit verschwendet
	INVIS(cout<<"ÄNDERE ENDFACE: "<<origFace->id<<endl);
	INVIS(helper.printArray(origFace->vtxChange, " = vtxChange"));
	INVIS(helper.printArray(origFace->UVChange, " == UV CHANGE RECREATEFACES"));

	MIntArray UVTmp;		//speichert die UVÄnderungen eines newVtx
	
	int k = origFace->vtxChange.length();
	UINT l = origFace->vtxChange.length();

	UINT vOffset = getValidAbsoluteOffset(origFace->id);	//gültiges offset in faceVtxArray
	UINT vCount =  vOffset + (*offsets)[origFace->id];
	UINT newVtxLocID;										//ID, die drekt auf die position vom OrigVtx zeigt, und somit auch die entsprechende UV referenziert
	int	 origVtx;											//speicher für den originalVtx
	int	 newVtxID;											//speicher für den newVtx



	UINT nOffset;											//gültiges offset in NormalIndicesArray
	UINT nCount;


		nOffset = getValidAbsNormalOffset(origFace->id);
		nCount = nOffset + normalCount[origFace->id];
	


	MIntArray uOffset(numUVSets, -2);								//gültiges Offset in jedes der UVSets, wird nur einmal initialisiert (-> wenn uOffset[x] == -2)


	UINT a;
	for(a = 0; a < l; a+=2)
	{
		
		//ZUERST DEN ORIGVTX IN FACEARRAY MIT DEM NEWVTX TAUSCHEN
		origVtx = origFace->vtxChange[a];
		
		bool notFound = true;


		int i;
		for(i = vOffset; i < vCount; i++)	
		{
			if(	(*faceVtxIDs)[i] == origVtx)
			{
				newVtxID = origFace->vtxChange[a+1];	

				(*faceVtxIDs)[i] = newVtxID;
				
				newVtxLocID = i - vOffset;				//wird später für schnellen UVLookup benoetigt

				notFound = false;

				break;	//Abbruch, da origVtx eh nur einmal im Face vorhanden ist
			}
		}
		
		if(notFound)
		{
			//wenn er diesen Vtx nicht finden konnte, dann wurde er wahrscheinlich vorher schon verändert von einer FaceData instanz, die das selbe Face bezeichnete
			//dies kann vorkommen bei bestimmten konstellationen von endEdges uns selEdges, die sich ein grosses nGon teilen, ohne dabei miteinander verbunden zu sein
			continue;		//um weiteren overhead zu vermeiden, mit dem nächsten Vtx weitermachen
		}
		
		//BEARBEITE NORMALEN
		

		
		//JEDES VORKOMMEN VON ORIGVTX DURCH NEWVTX ERSETZEN
		for(i = nOffset; i < nCount; i++)
		{
			if(normalVertexIndices[i] == origVtx)
				normalVertexIndices[i] = newVtxID;
			//hier kein abbruch, da origvtx im indices array mehrfach vorhanden sein kann je face
		}
		

		
		//UVS BEARBEITEN
		

			
			//jetzt jede UV in jedem UVSet verändern, wenn moeglich
			
			//iteratoren
			std::list<MIntArray>::iterator iterCounts			= UVCounts->begin();
			std::list<MIntArray>::iterator iterOffset			= UVAbsOffsets->begin();
			std::list<MIntArray>::iterator iterLUT				= UVoffsetsLUT->begin();
			std::list<MIntArray>::iterator iterLUTValues		= UVoffsetsLUTValues->begin();
			std::list<MIntArray>::iterator iterUVIDs			= UVperVtx->begin();
			
			
			
			
			for(i = 0; i < numUVSets; i++)
			{
				//wenn face überhaupt UVs hat
				if( (*iterCounts)[origFace->id] != 0)
				{
					
					//faceUVs verändern
					
					if(uOffset[i] == -2)
					{	//es muss also noch ein passender Offset erstellt werden
						uOffset[i] = getValidAbsoluteUVOffset(origFace->id,i,iterOffset,iterLUT,iterLUTValues);
					}
					
					
					
					(*iterUVIDs)[uOffset[i] + newVtxLocID] = origFace->UVChange[i*numUVSets*2 + a + 1];	//a hat schon vielfaches von 2, weshalb hier nicht noceinmal mit 2 mulipliziert wird
					
				}
				
				
				
				++iterCounts;
				++iterOffset;
				++iterLUT;
				++iterLUTValues;
				++iterUVIDs;
				
			}
			
			



	}


}


//-------------------------------------------------
void	edgeMeshCreator::addNormalSlide(int vtxID)
//-------------------------------------------------
{
	//diese Methode mittelt die umgebenden Facenormalen und fügt slide hinzu
	//MERKE: es wird nirgendwo gecheckt, ob dieser Vtx schon hinzugefügt wurde, 
	//weshalb bei "doSlide" vernachlässigbarer mehraufwand entsteht

	int l;
	MIntArray connectedFaces;
	MVector normal,tmpNormal;
	double scale = 0,tmpScale;

	vertIter.setIndex(vtxID,tmp);
	vertIter.getConnectedFaces(connectedFaces);

	l = connectedFaces.length();
	for(int i = 0; i < l; i++)
	{
		meshFn.getPolygonNormal(connectedFaces[i],tmpNormal);
	
		normal += tmpNormal;

		polyIter.setIndex(connectedFaces[i],tmp);
		polyIter.getArea(tmpScale);

		scale += tmpScale;
	}

	//mitteln
	normal /= l;
	scale /= l;

	normal.normalize();

	meshCreator::addNormalSlide(vtxID,normal,scale);


}

//--------------------------------------------------------
void	edgeMeshCreator::addSlide(int startID,int endID)
//--------------------------------------------------------
{
	MPoint start = (*vtxPoints)[endID];
	MPoint end = (*vtxPoints)[startID];

	MVector direction = end - start;

//	->MUSS DEAKTIVIERT SEIN; WENN UVS FUER SIDEFACES RICHTIG FUNZEN SOLLEN ->geh jetzt auch so, da posTmp verwendet wird - muss aber immer noch nicht an sein
//	(*vtxPoints)[endID] = end - slide*direction;


	slideIndices->append(endID);	//hier ist die EndID die ID des slidenden Vtx
	
	slideStartPoints->append(start);
	slideEndPoints->append(end);
	slideDirections->append(direction);

	slideNormals->append(MVector(0.0,0.0,0.0));
	normalScale->append(0.0);
	slideScale->append(-1);
	maySlide->append(1);

	

}


//------------------------------------------------------------------
void	edgeMeshCreator::addSlideAndNormal(int startID, int newVtx, MPoint& position, bool usePositionInstead)
//------------------------------------------------------------------
{

	MVector normal,tmpNormal;
	double scale = 0,tmpScale;
	MIntArray conFaces;

	int tmp;

	vertIter.setIndex(startID,tmp);
	vertIter.getConnectedFaces(conFaces);
	
	for(unsigned int i = 0; i < conFaces.length(); i++)
	{
		meshFn.getPolygonNormal(conFaces[i],tmpNormal);
	
		normal += tmpNormal;

		polyIter.setIndex(conFaces[i],tmp);
		polyIter.getArea(tmpScale);

		scale += tmpScale;
	}

	//mitteln
	normal /= conFaces.length();
	scale /= conFaces.length();

	normal.normalize();

	MPoint end;
	MPoint start;
	if(usePositionInstead)
		start = position;
	else
		start = (*vtxPoints)[newVtx];

	end = (*vtxPoints)[startID];

	MVector direction = end - start;

//habe ich rausgenommen, um bug zu beheben ->MUSS DEAKTIVIERT SEIN; WENN UVS FUER SIDEFACES RICHTIG FUNZEN SOLLEN -> siehe oben
//	if(newVtx < initialVtxCount)	//"alte" neue Vtx müssen sofort gesetzt werden
//        		(*vtxPoints)[newVtx] = position;	//->problem, wenn origVtx zuerst gesetzt wird, da dann der newVtx verschoben ist
													//origVtx werden jetzt zuletzt verschoben

	slideIndices->append(newVtx);	//hier ist die EndID die ID des slidenden Vtx
	
	slideStartPoints->append(start);
	slideEndPoints->append(end);
	slideDirections->append(direction);

	slideNormals->append(normal);
	normalScale->append(scale);
	slideScale->append(-1);
	maySlide->append(1);

}


//-----------------------------------------------------------------------
MPoint		edgeMeshCreator::getSlideStartPoint(int vtxID)
//-----------------------------------------------------------------------
{
	//sucht im slideIndicesArray nach dem passende Vtx, und gibt die dazugehoerige startposition zurück, was der endposition entspricht

	UINT l = slideIndices->length();
	for(UINT i = 0; i < l; i++)
	{
		if((*slideIndices)[i] == vtxID)
			return (*slideStartPoints)[i];

	}

	//forbidden
	return MPoint();
}


//-----------------------------------------------------------------------
void	edgeMeshCreator::getUVSlideStartPoint(const MIntArray& UVIDs, MFloatArray& outPos)
//-----------------------------------------------------------------------
{
	//sicherstellen, dass outPos die richtige lnge hat
	outPos.setLength(numUVSets * 2);

	//holt die UVPositionen aus dem UVSlideArray
	UINT l;
	
	std::list<MFloatVectorArray>::iterator	posIter = UVSlideStart->begin();
	std::list<MIntArray>::iterator			idIter	= UVSlideIndices->begin();

	for(UINT i = 0; i < numUVSets; i++)
	{//pro UVSet
		
		MIntArray& ids = *idIter;
		
		l = ids.length();

		for(UINT a = 0; a < l; a++)
		{
			if(UVIDs[i] == ids[a])
			{
				MFloatVector tmpVec = (*posIter)[a];
				outPos[i] = tmpVec.x;
				outPos[i+1] = tmpVec.y;

				break;
			}
		}

	}



}


//-----------------------------------------------------------------------
void	edgeMeshCreator::addSlideAndNormal(int startID, int edgeVtx,MIntArray& faces)
//-----------------------------------------------------------------------
{

	MVector normal,tmpNormal;
	double scale = 0,tmpScale;
	
	for(unsigned int i = 0; i < faces.length(); i++)
	{
		meshFn.getPolygonNormal(faces[i],tmpNormal);
	
		normal += tmpNormal;

		polyIter.setIndex(faces[i],tmp);
		polyIter.getArea(tmpScale);

		scale += tmpScale;
	}

	//mitteln
	normal /= faces.length();
	scale /= faces.length();

	normal.normalize();

	MPoint start = (*vtxPoints)[edgeVtx];
	MPoint end = (*vtxPoints)[startID];

	MVector direction = end - start;

	
//	(*vtxPoints)[edgeVtx] = end - slide*direction;


	slideIndices->append(edgeVtx);	//hier ist die EndID die ID des slidenden Vtx
	
	slideStartPoints->append(start);
	slideEndPoints->append(end);
	slideDirections->append(direction);

	slideNormals->append(normal);
	normalScale->append(scale);
	slideScale->append(-1);
	maySlide->append(1);

}


//-------------------------------------------------------------------------------------------
int		edgeMeshCreator::addWingedDisplacedVtx(int wingedVtx, MVector& preMultipliedNormal)
//-------------------------------------------------------------------------------------------
{
	return meshCreator::newVtx((*vtxPoints)[wingedVtx] + preMultipliedNormal);
}

//-------------------------------------------------------------------------------------------
MPoint		edgeMeshCreator::getWingedDisplacedPoint(int wingedVtx, MVector& preMultipliedNormal)
//-------------------------------------------------------------------------------------------
{
	return ((*vtxPoints)[wingedVtx] + preMultipliedNormal);
}




//-------------------------------------------------------------------------------------------
void	edgeMeshCreator::getFaceNormal(int faceID, MVector& inNormal)
//-------------------------------------------------------------------------------------------
{

	polyIter.setIndex(faceID,tmp);
	polyIter.getNormal(inNormal);

}


//--------------------------------------------------------------------------------
void	edgeMeshCreator::setUVPositions(MIntArray& UVIds, MFloatArray& UVPos)
//--------------------------------------------------------------------------------
{
	//UVPos hat pro UVSet 2 einträge (U,V), UVIDs sind negativ, wenn in diesem UVSet keine UV vorhanden ist für das entsprechende Face (welches hier keine rolle spielt)
	
	//UVIDs: Hält die entsprechende UVId des UVSets, wenn in einem Set keine UVs vorhanden sind für das betreffende Face, dann ist UVIDs[x] == -1

	//ITERATOREN
	std::list<MFloatArray>::iterator UIter = Us->begin();
	std::list<MFloatArray>::iterator VIter = Vs->begin();

	
	for(UINT i = 0; i < numUVSets; i++)
	{
		
		if(UVIds[i] > -1)
		{
			(*UIter)[UVIds[i]] = UVPos[i*2] ;
			(*VIter)[UVIds[i]] = UVPos[i*2+1] ;
		}

		UIter++;
		VIter++;
	}

	//Da dummy elemente angehängt wurden, kann man die UVs sehr einfach dereferenzieren


}

//--------------------------------------------------------------------------------
MIntArray		edgeMeshCreator::createUVs(MIntArray& validator, MFloatArray& UVPos)
//--------------------------------------------------------------------------------
{

	//erzeugt neue UVID, wenn der validator positiv ist

		//ITERATOREN
	std::list<MFloatArray>::iterator UIter = Us->begin();
	std::list<MFloatArray>::iterator VIter = Vs->begin();

	MIntArray returnValue(numUVSets, -1);

	for(UINT i = 0; i < numUVSets; i++)
	{
		
		if(validator[i] > -1)
		{
			(*UIter).append(UVPos[i*2]);
			(*VIter).append(UVPos[i*2+1]);

			//newUVID hinzufügen
			returnValue[i] = (*UIter).length() - 1;
		}

		UIter++;
		VIter++;
	}

	return returnValue;

}


//-----------------------------------------------------------------------------------------------------------------------------------
void	edgeMeshCreator::addUVSlide(const MFloatArray& startPositions,const MFloatArray& endPositions,const MIntArray& newUVIds)
//-----------------------------------------------------------------------------------------------------------------------------------
{

	//Diese Prozedur kann so erstmal nur einen UVIndex pro UVSt eintragem , koennte aber falls noetig leicht umgerüstet werden, um x pro UVSet verarbeiten zu koennen
	

//ITERATOREN

	std::list<MIntArray>::iterator iterSlideIDs =			UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideEnd =	UVSlideEnd->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs =	UVSlideDirections->begin();


	float cStart[2];	//startCache
	float cEnd[2];		//endCache;

	//jetzt einfach alles pro UVSet eintragen

	for(UINT i = 0; i < numUVSets; i++)
	{
		if(newUVIds[i] == -1)
			continue;

		(*iterSlideIDs).append(newUVIds[i]);

		cEnd[0] = endPositions[i*2];
		cEnd[1] = endPositions[i*2+1];

		(*iterSlideEnd).append( MFloatVector(cEnd[0], cEnd[1],0.0 ) );
		


		
		cStart[0] = startPositions[i*2];
		cStart[1] = startPositions[i*2+1];

		(*iterSlideStart).append( MFloatVector(cStart[0], cStart[1],0.0 ) );



		(*iterSlideDirs).append(MFloatVector( cEnd[0] - cStart[0], cEnd[1] - cStart[1] ) );
		
		//weiter mit nächstem UVSet
	}


}
