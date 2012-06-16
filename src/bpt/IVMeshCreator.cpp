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

#include "IVMeshCreator.h"

//----------------------------------------------------------------------------------------------------------
IVMeshCreator::IVMeshCreator(		MObject&		fMesh,
									UINT			inCount,
									double			inSlide, 
									MDagPath&		meshPath)

								 :

																meshCreator(fMesh,
																			inSlide, 
																			meshPath),

																splitCount(inCount)
//----------------------------------------------------------------------------------------------------------														
{
}


//----------------------------------------------------------------------------------------------------------
IVMeshCreator::~IVMeshCreator(void)
//----------------------------------------------------------------------------------------------------------
{
}



//----------------------------------------------------------------------------------------------------------
void IVMeshCreator::doInsertVtx( int edgeID,
								 bool useSlide, 
								 int startVtxID, 
								 int endVtxID	)
//----------------------------------------------------------------------------------------------------------
{
	//diese Prozedur erstellt/macht folgendes:
	//		SlideDaten (nur NormalSlide wenn useSlide = false)
	//		Erstellt neue Vertizen
	//		Fügt diese Vertizen in die an die Edges angrenzenden Faces ein
	//		Errechnet Normalen und aktualisiert die internen Arrays
	//		Erstellt UVs und atualisiert damit die Arrays

	//-----------
	//VARIABLEN
	//-----------
	MIntArray connectedFaces;	//hält die Faces, die mit edge verbunden sind

	MVector	slideNormal;		//hält die entgltige Normale, in die die neuen Vertizen sliden
	double	normalScaleValue = 0;	//skaliert die Normale entsprechend der Groesse des dazugehoerigen Faces
	MVector	direction;			//Richtungsvektor von start - endPoint
	MPoint	startPoint;			//startPoint wird ebenfalls für die Vtxinsertion benoetigt
	unsigned insertionID;		//die ID des letzten Vtx im Array

	MIntArray vertices[2];		//hält die vertizen der faces, die mit der edge verbunden sind (maximal 2)

	UINT l;						//länge von allen moeglicen arrays

	//******************
	//LOS GEHT'S
	//******************


	
	//SLIDENORMALE HOLEN (DIESE WIRD AUF JEDEN FALL BENÖTIGT)
	//////////////////////////////////////////////////////////
	
	edgeIter.setIndex(edgeID,tmp);
	edgeIter.getConnectedFaces(connectedFaces);

	//gleich am Anfang gültige start/endVtxIDs holen, wenn sie nicht übergeben wurden
	if(!useSlide)
	{
		startVtxID = edgeIter.index(0);
		endVtxID = edgeIter.index(1);
	}


	//einfach die gemittelte normale nehmen (egal, ob borderedge oder nicht)
	l = connectedFaces.length();
	UINT i;
	for(i = 0; i < l; i++)
	{
		MVector		normal;
		double		tmpScale;

		polyIter.setIndex(connectedFaces[i],tmp);
		polyIter.getNormal(normal);

		//hier schon vorsorglich die vertizen holen
		//man darf hier nur eigene prozeduren verwenden, da zu einem Face auch mehrmals Vertizen hinzu-
		//gefügt werden koennen und der PolyIter dann natürlich nicht mehr aktuell ist 
		getFaceVtxIDs(connectedFaces[i],vertices[i]);

		slideNormal += normal;
		
		//normal scale anhand area des Faces
		polyIter.getArea(tmpScale);
		normalScaleValue += tmpScale;
	}



	//werte normalisieren
	slideNormal /= l;
	normalScaleValue /= l;

	//normals erstmal hinzufügen
	slideNormals->append(slideNormal);
	normalScale->append(normalScaleValue);



	//slideScale ist immer - 1, da es erst berechnet werden kann, wenn alle Werte eingetragen sind
	//-1 ist ausserdem Flag dafür, dass die Werte noch berechnet werden müssen
	slideScale->append(-1.0);

	//slideIndices[x] ist immer der erste der neuen Vertizen, ich antizipiere das hier: der erste neue Vtx hat
	//den Index von vtxPoints->length();
	insertionID = vtxPoints->length() - 1;	//insertionID setzen, später für InsertVtx benoetigt
	slideIndices->append( insertionID + 1 );


	//jetzt noch die SlideArrays aktualisieren
	if(useSlide)
	{//direction errechnen und start/End noch eintragen
		startPoint = (*vtxPoints)[startVtxID];
		slideStartPoints->append(startPoint);

		slideEndPoints->append((*vtxPoints)[endVtxID]);
		
		//direction definieren
		direction = (*vtxPoints)[endVtxID] - (*vtxPoints)[startVtxID];
		slideDirections->append(direction);


		maySlide->append(1);
	}
	else
	{//ansonsten leere elemente anfügen - nur slideStartPoint und direction muss gegeben sein, damit along normal
		//funktionieren kann
		//man kann hier einfach start/endVtxID Variablen verwenden, da sie glech am anfang mit gültigen werten
		//beschrieben wurden

		startPoint = (*vtxPoints)[ startVtxID ];
		slideStartPoints->append(startPoint);

		direction = (*vtxPoints)[ endVtxID] - (*vtxPoints)[startVtxID];
		slideDirections->append( direction );
		
		slideEndPoints->append(MPoint());

		maySlide->append(0);
	}



	//NOCH BEVOR DIE VERTIZEN AKTUALISIERT WERDEN, WERDEN DIE NEUEN UVS ERSTELLT (WEIL SIE NOCH DIE 
	//ORIGINALVTX DER FACES BRAUCHEN)
	/////////////////////////////////////////////////////////////////////////////////////////

	generateUVs(connectedFaces,&vertices[0],startVtxID, endVtxID, useSlide);


	//NUN NOCH DIE NORMALEN ERSTELLEN
	//////////////////////////////////////////////////////////


	generateNormals(connectedFaces,startVtxID, endVtxID);


	//JETZT NEUE VERTIZEN ERSTELLEN
	//////////////////////////////////////////////////////////

	double	factor = 1.0 / (splitCount + 1);
	double	mySlide = (useSlide) ? slide : 1.0;
	double  r = 1.0;
	
	//einfügen der neuen Vertizen:
	for( i = 0; i < splitCount; i++)
	{
		vtxPoints->append(startPoint + (r++ * factor) * mySlide * direction);
	}

	//DIE NEUEN INDICES IN FACEVERTICES EINFUEGEN UND DIE MESHDATEN DAMIT AKTUALISIEREN
	/////////////////////////////////////////////////////////////////////////////////////////

	//jetzt die angrenzenden Faces aktualisieren
	UINT		x,y,l2 = vtxPoints->length();			//für iteration, l2 wird später gebraucht

	
	UINT		offset;			//offset als startMarkierung für faceVtxIDs
	UINT		length;			//zum einfügen der Vertizen in faceVtxIDs

	for(i = 0; i < connectedFaces.length();i++)
	{//merke: FaceVertices wurden bereits geholt
			
		vertices[i].setSizeIncrement(splitCount);	//performancesteigerung (verhindert häufige reallocs)

		//jetzt die startID im Face finden
		l = vertices[i].length();
		for(x = 0; x < l; x++)
		{
			if(vertices[i][x] == startVtxID )
			{

			
				if(vertices[i][ (x+1) % l] == endVtxID )
				{//neue VertexIndices in Richtung des Faces einfügen; 
					
					x = (x+1) % l;	//damit x auf das richtige Element zum einfügen zeigt	(endID)

					for(y = insertionID + 1; y < l2; y++)
					{
						vertices[i].insert(y,x++);
					}

				}
				else
				{//neue Verticen entgegen der Richtung des Faces einfügen
					
					//x zeigt auf startID -> korrekt

					for(y = l2 - 1; y > insertionID; y--)
					{
						vertices[i].insert(y,x++);	//da die insertposition durch insert um 1 nach oben verschoben wird, muss x ebenfalls um 1 erhoeht werden
					}
				
				}


				//meshDaten aktualisieren
				offset = getValidAbsoluteOffset(connectedFaces[i]);
				length = offset + vertices[i].length();

				//zuerst faceVtxArray erweitern
				for(y = 0; y < splitCount; y++)
					faceVtxIDs->insert(-1,offset);

				//jetzt das verticesArray übertragen
				UINT r = 0;
				for(y = offset; y < length; y++)
					(*faceVtxIDs)[y] = vertices[i][r++];

				//offsetsLUT updaten
				updateOffsetLUT(connectedFaces[i], splitCount);
				(*offsets)[ connectedFaces[i] ] += splitCount;

				break;

			}//if(vertices[x] == startID )

		}//for(x = 0; x < l; x++)

	}//for(i = 0; i < connectedFaces.length();i++)




}

//----------------------------------------------------------------------------------------------------------
void IVMeshCreator::generateUVs(MIntArray& connectedFaces,
								MIntArray* vertices,
								int		   startVtxID,
								int		   endVtxID,
								bool	   useSlide)
//----------------------------------------------------------------------------------------------------------
{//erzeugt UVs für die Faces, wenn moeglich


	//iteratoren initialisieren
	
	MIntArray	faceUVs[2];		//hält dieUVS des entsprechenden Faces
	UINT		startLocID[2],endLocID[2];	//lokale VtxIDs, die auf start/End im UVArray zeigen
	UINT		x,i,l;				//für iterationen
	int		minMaxIDs[2];	//hält die MinMaxIDs der neu erstellten UVs

	std::list<MIntArray>::iterator iterCounts			= UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset			= UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT				= UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues		= UVoffsetsLUTValues->begin();
	std::list<MIntArray>::iterator iterUVperVtx			= UVperVtx->begin();

	
	std::list<MFloatArray>::iterator iterUPos			= Us->begin();
	std::list<MFloatArray>::iterator iterVPos			= Vs->begin();


	std::list<MIntArray>::iterator iterSlideIDs			= UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideEnd = UVSlideEnd->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();


	//die lokalen IDs im UVArray finden (sie sind für jedes UVSet gleich, müssen also nur einmal gefunden werden)
	//faceUVs sind synchron zu vertices[0], so dass man erstmal den lokalen UVIndex für den StarUV finden muss
	for(x = 0; x < connectedFaces.length(); x++)
	{
		l = vertices[x].length();
		for(i = 0; i < l; i++)
		{
			if( vertices[x][i] == startVtxID)
			{
				startLocID[x] = i;

				if( vertices[x][ (i+1)%l ] == endVtxID)
					endLocID[x] = (i+1)%l;
				else
					endLocID[x] = (i+l-1)%l;
				
				break; //nächstes Face bearbeiten
			}
		}

	}



	for(x = 0; x < numUVSets; x++)
	{
		MIntArray& counts = *iterCounts;

		MFloatArray& UArray = *iterUPos;
		MFloatArray& VArray = *iterVPos;

		if(counts[ connectedFaces[0] ])
		{//also erstmal UVs für face0 erstellen


			getFaceUVs(	connectedFaces[0],
						faceUVs[0],
						x,
						iterOffset,
						iterLUT,
						iterLUTValues,
						iterCounts,
						iterUVperVtx );

			
			if(useSlide)
				createUVs(	minMaxIDs[0],
							faceUVs[0],
							UArray,
							VArray,
							startLocID[0],
							endLocID[0],
							&iterSlideIDs,
							&iterSlideStart,
							&iterSlideEnd,
							&iterSlideDirs );
			else
				createUVs(	minMaxIDs[0],
							faceUVs[0],
							UArray,
							VArray,
							startLocID[0],
							endLocID[0]	);

			//wenn das andere Face UVs mit dem Face teilt (falls UVs vorhanden), dann 
			//müssen keine neuen UVs erstellt werden und die neuen Indices werden einfach 
			//in das Face2 übertragen

			if(connectedFaces.length() == 2 && counts[connectedFaces[1]] != 0)
			{//checken, ob UVs übereinstimmen (min. 2)


				getFaceUVs(	connectedFaces[1],
							faceUVs[1],
							x,
							iterOffset,
							iterLUT,
							iterLUTValues,
							iterCounts,
							iterUVperVtx );

				if(helper.matchArray(faceUVs[0],faceUVs[1]).length() > 1)
				{//sie teilen UVs, also die bereits erstellten in face 2 übertragen
					minMaxIDs[1] = minMaxIDs[0];
				}
				else
				{//sie teilen keine UVs, also müssen neue erstellt werden für dieses Face
					if(useSlide)
					{			
						createUVs(	minMaxIDs[1],
									faceUVs[1],
									UArray,
									VArray,
									startLocID[1],
									endLocID[1],
									&iterSlideIDs,
									&iterSlideStart,
									&iterSlideEnd,
									&iterSlideDirs );

					}
					else
					{
						createUVs(	minMaxIDs[1],
									faceUVs[1],
									UArray,
									VArray,
									startLocID[1],
									endLocID[1]		);

					}


				}//if(helper.matchArray(faceUVs[0],faceUVs[1]).length() > 1)

				//jetzt die neuen UVs für Face2 eintragen in die UVArrays
				updateUVArrays(	connectedFaces[1],
								vertices[1],
								startLocID[1],
								endLocID[1],
								x,
								minMaxIDs[1],
								&iterCounts,
								&iterOffset,
								&iterLUT,
								&iterLUTValues,
								&iterUVperVtx );

			}//if(connectedFaces.length() == 2 && counts[connectedFaces[1] != 0)
			//else -> nichts tun, wenn 2. Face ungültig hinsictlich der UVs

	
		//jetzt die neuen UVs für Face1 eintragen in die UVArrays
			updateUVArrays(	connectedFaces[0],
							vertices[0],
							startLocID[0],
							endLocID[0],
							x,
							minMaxIDs[0],
							&iterCounts,
							&iterOffset,
							&iterLUT,
							&iterLUTValues,
							&iterUVperVtx );

		
		}
		else if(connectedFaces.length() == 2 && counts[ connectedFaces[1] ] )
		{//also UVs nur für face 1 erstellen und dann sofort mit der nächsten Iteration fortfahren
			getFaceUVs(	connectedFaces[1],
							faceUVs[1],
							x,
							iterOffset,
							iterLUT,
							iterLUTValues,
							iterCounts,
							iterUVperVtx );

			//UVs erzeugen (und auch slide, wenn noetig)
			if(useSlide)
			{			
				createUVs(	minMaxIDs[1],
							faceUVs[1],
							UArray,
							VArray,
							startLocID[1],
							endLocID[1],
							&iterSlideIDs,
							&iterSlideStart,
							&iterSlideEnd,
							&iterSlideDirs );

			}
			else
			{
				createUVs(	minMaxIDs[1],
					faceUVs[1],
					UArray,
					VArray,
					startLocID[1],
					endLocID[1]		);

			}


			//jetzt die neuen UVs für Face2 eintragen in die UVArrays
			updateUVArrays(	connectedFaces[1],
							vertices[1],
							startLocID[1],
							endLocID[1],
							x,
							minMaxIDs[1],
							&iterCounts,
							&iterOffset,
							&iterLUT,
							&iterLUTValues,
							&iterUVperVtx );

		}

		++iterUPos;
		++iterVPos;
		++iterCounts;
		++iterOffset;
		++iterLUT;
		++iterLUTValues;
		++iterUVperVtx;

		++iterSlideIDs;
		++iterSlideStart;
		++iterSlideEnd;
		++iterSlideDirs;
	}//for(UINT x = 0; x < numUVSets; x++)



}

//----------------------------------------------------------------------------------------------------------
void	IVMeshCreator::createUVs(	int&		 minIndex,
									MIntArray&	 faceUVs,
									MFloatArray& UArray, 
									MFloatArray& VArray,
									int			 startLocID,
									int			 endLocID,
									std::list<MIntArray>::iterator* iterSlideIDs,
									std::list<MFloatVectorArray>::iterator* iterSlideStart,
									std::list<MFloatVectorArray>::iterator* iterSlideEnd,
									std::list<MFloatVectorArray>::iterator* iterSlideDirs)
//----------------------------------------------------------------------------------------------------------
{
//wenn er hier ankomt, hat das Fae garantiert UVs, so dass das hier nicht mehr gecheckt werden muss

	float2 startPoint = {	UArray[ faceUVs[startLocID] ],
							VArray[ faceUVs[startLocID] ]
						};

	float2 endPoint =	{	UArray[ faceUVs[endLocID] ],
							VArray[ faceUVs[endLocID] ]
						};

	float2 direction = { endPoint[0] - startPoint[0],
						 endPoint[1] - startPoint[1] };
						 

	//resultierende minMaxUVIndices setzen
	minIndex = UArray.length();						//minID



	//jetzt die eigentlichen UVs erstellen
	double factor = 1.0 / double(splitCount + 1);
	double	mySlide = (iterSlideStart != 0) ? slide : 1.0;	//klar, man koennte auch darauf verzichten, dass
															//man den selben check weiter unten nochmal macht
															//allerdings ist es so übersichtlicher
	double r = 1.0;


	UINT i;
	for(i = 0; i < splitCount; i++)
	{
		UArray.append(startPoint[0] + (r++ * factor) * mySlide * direction[0]);
	}

	r = 1.0;
	for(i = 0; i < splitCount; i++)
	{
		VArray.append(startPoint[1] + (r++ * factor) * mySlide * direction[1]);
	}


	if(iterSlideStart != 0)
	{
		(*(*iterSlideStart)).append( MFloatVector(startPoint[0],startPoint[1]) );
		(*(*iterSlideEnd)).append(MFloatVector(endPoint[0],endPoint[1]));
		(*(*iterSlideDirs)).append(MFloatVector(direction[0],direction[1]));

		//slideIDs sind im Doppelpack codiert:
		//Xn*2 = StartSlideID, Xn*2 + 1 = endSlideID
		//dies muss sein, weil nicht davon auszugehen ist, dass das Array lückenlos sein wird
		//so wie das beim vtxSlideArray (welche ja immer zumindest along normal sliden koennen)
		(*(*iterSlideIDs)).append(minIndex);
		(*(*iterSlideIDs)).append(minIndex + splitCount);
	}

}

//----------------------------------------------------------------------------------------------------------
void	IVMeshCreator::updateUVArrays(  int faceID,
									    MIntArray& faceVertices,	
										int startLocID,
										int endLocID,
										int index,
										int	 minIndex,	
										std::list<MIntArray>::iterator* iterCounts,
										std::list<MIntArray>::iterator* iterOffset,
										std::list<MIntArray>::iterator* iterLUT,
										std::list<MIntArray>::iterator* iterLUTValues,
										std::list<MIntArray>::iterator* iterUVperVtx	
																						)
//----------------------------------------------------------------------------------------------------------
{
	//fügt die zuvor erstellten Vertizen in die UVArrays ein, so dass später ein gültiges
	//UVSet zurückgewonnen werden kann



	//erstmal gültiges offset ins UVPerVtxArray holen
	int offset = getValidAbsoluteUVOffset(	faceID,
											index,
											*iterOffset,
											*iterLUT,
											*iterLUTValues );

	//die neuen UVs werden direkt in das grosse UVperVtxArray eingefügt


	MIntArray& UVIDs = (*(*iterUVperVtx));
	
	UINT fl = faceVertices.length();


	if( faceVertices[(startLocID + 1) % fl ] == faceVertices[endLocID]  )
	{//Die neuen IDs müssen nach startLocID eingefügt werden (was der position von endLocID entspricht)
		
		int insertPos = offset + endLocID;	//insertPos ist quasi endLocID
		int	maxLength = insertPos + splitCount;

		for(; insertPos < maxLength; insertPos++)
		{
			UVIDs.insert(minIndex++,insertPos);
		}


	}
	else
	{//die neuen IDs müssen gegen den Urzeigersinn eingefügt werden, in umgekehrter Reihenfolge

		int insertPos = offset + startLocID;	//insertPos ist quasi endLocID
		int	maxLength = insertPos + splitCount;
		int maxIndex = minIndex + splitCount - 1;

		for(; insertPos < maxLength; insertPos++)
		{
			UVIDs.insert(maxIndex--,insertPos);
		}

	}

	//gut, jetzt noch die UVArrays updaten

	updateOffsetUVLUT(faceID, splitCount,index,*iterLUT,*iterLUTValues);
	(*(*iterCounts))[faceID] += splitCount;

	

}

//----------------------------------------------------------------------------------------------------------
void IVMeshCreator::generateNormals(	MIntArray&	connectedFaces,
										int			startVtxID,
										int			endVtxID		)
//----------------------------------------------------------------------------------------------------------
{//aktualisiert die NormalenArrays und fügt hardEdges hinzu, wenn moeglich



	UINT i, l = connectedFaces.length();
	UINT locIndex1,locIndex2;
	int offset, highBound;
	int thisFace;

	for(i = 0; i < l; i++)
	{
		//nur weitermachen, wenn überhaupt harte edges vorhanden sind
		thisFace = connectedFaces[i];

		if(normalCount[thisFace] == 0)
			continue;


		offset = getValidAbsNormalOffset(thisFace);
		highBound = offset + normalCount[thisFace];

		//jetzt schauen ob die edge, die durch start/endVtx definiert wird, hart ist
		if(		helper.getLocIndexMod2(startVtxID,normalVertexIndices,locIndex1, offset, highBound) && 
				helper.getLocIndex(endVtxID,normalVertexIndices,locIndex2, offset, highBound)     )

		{//edge ist hart, jetzt also alle neuen edges bestehend aus den neuen vertizen
		 //zwischen diese vorhandenen edges einfügen (direkt in normalVertexIndices)
			

			int minIndex = vtxPoints->length();

			//erst Kante definieren zwischen start und erstem neuen Vertex - Anfangpositions wird definert durch logIndex
			//normalVertexIndices.insert(minIndex,++offset);
			normalVertexIndices.insert(minIndex,++locIndex1);

			int mySplitCount = minIndex + splitCount - 1;	//minus 1, damit ich in der schleife sicher i + 1 holen kann
			//jetzt alle Kanten zwischen den neuen Vertizen definieren
			for(; minIndex < mySplitCount; minIndex++)
			{
				//erst den endPunkt der Kante hinzufügen
				normalVertexIndices.insert(minIndex + 1, ++locIndex1);
				
				//dann den startPunkt auf der selben position wie endpunkt einfügen
				//und somit den endpunkt an die richtige position schieben;
				//dann offset 1 nach vorn auf endPunkt setzen, damit die nächste Iteration richtige 
				//Startvorraussetzungen hat
				normalVertexIndices.insert(minIndex, locIndex1++);
			}

			//jetzt noch den letzten neuen Vtx mit dem letzten Vtx der original edge (welcher die ganze Zeit
			//nach oben geschoben wurde) verbinden
			//dafür einfach mySplitCount and die position einfügen, wo der offset marker gerade steht
			normalVertexIndices.insert(mySplitCount,locIndex1);


			//jetzt noch die Arrays richtig aktualisieren
			normalCount[thisFace] += splitCount * 2;	//insgesamt wurden numSplitCount * 2 vertizen hinzugefügt
														//jede edge besteht ja immer aus zwei vertizen
			updateNormalOffsetLUT(thisFace,splitCount * 2);

		}//if(helper. ... )ende;

	}

}

