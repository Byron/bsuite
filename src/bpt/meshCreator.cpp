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

// meshCreator.cpp: implementation of the meshCreator class.
//
//////////////////////////////////////////////////////////////////////

#include "meshCreator.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------------
meshCreator::meshCreator(	MObject&		fMesh,
							double			inSlide,
							MDagPath&		meshPath
															)
						
						 :

																	edgeIter(fMesh),
																	polyIter(fMesh),
																	meshFn(fMesh),
																	vertIter(fMesh),
																	slide(inSlide),
																	minID(999999999),
																	maxID(0),
																	nMinID(999999999),
																	nMaxID(0),
																	deallocUVs(true),
																	numUVSets(0)

																	
{
//-----------------------------------------------------------------------------------------------------------------
//----------------------
//MESHARRAYS ERZEUGEN
//----------------------


	vtxPoints = new MPointArray();
	offsets = new MIntArray();
	offsetsAbsolute = new MIntArray();
	faceVtxIDs = new MIntArray();

	offsetsLUT = new MIntArray();
	offsetsLUTValues = new MIntArray();


//----------------------
//SLIDE ARRAYS ERZEUGEN
//----------------------

	normalScale = new MDoubleArray();		//hat eigene versionen der Arrays, die schlussendlich von der BBTfty 
	normalScale->setSizeIncrement(25);
	
	slideScale = new MDoubleArray();
	slideScale->setSizeIncrement(25);		//geholt werden
	
	slideDirections = new MVectorArray();
	slideDirections->setSizeIncrement(25);

	slideNormals = new MVectorArray();		//wenn ein Vtx nicht slided, dann wird gegenwärtiger punkt als startPunkt eingetragen
	slideNormals->setSizeIncrement(25);

	slideStartPoints = new MPointArray();	//und maySlide[n] wird auf 0 gesetzt
	slideStartPoints->setSizeIncrement(25);

	slideEndPoints = new MPointArray();
	slideEndPoints->setSizeIncrement(25);

	slideIndices = new MIntArray();
	slideIndices->setSizeIncrement(25);

	maySlide = new MIntArray();			//synchron zu allen anderen Arrays, 0 wenn er sliden darf, 1 wenn nicht.
	maySlide->setSizeIncrement(25);

//----------------------
//DATEN EXTRAHIEREN
//----------------------

	/*
	//SHADER HOLEN
	MStatus	stat;
	
	MObjectArray	sets, comps;
	MFnMesh myMeshFn(meshPath);
	
	stat = myMeshFn.getConnectedSetsAndMembers(0, sets, comps,true);
	
	stat.perror(" Problem beim Setauslesen");
	cout<<sets.length()<<" == Num Sets"<<endl;
	cout<<comps.length()<<" == Num comps"<<endl;

	//->Funzt nict - invalid MeshData in der Shape
	*/



	UINT				count = 0;
	int					tmp = 0;
	MIntArray			vertIDs;
	

//	alle Punkte holen	
	meshFn.getPoints(*vtxPoints);
	

	
	//-1, da Vtx count 0 based ist
	initialVtxCount = vtxPoints->length() - 1;

	ULONG incrementSize = ULONG(initialVtxCount / 8);	//1/8 der vtxZahl ist das default Increment, um das ständige umkopieren der Arrays zu vermeiden
														//bei append
	vtxPoints->setSizeIncrement(incrementSize);
	
	//ARRAYS müssen nicht zurückgesetzt werden, da meshCreator eh nicht dauerhaft existiert
	


	int polyCount = polyIter.count();
	
	//arrayGröße anpassen, um effizienter zu sein
	offsets->setLength(polyCount);
	offsetsAbsolute->setLength(polyCount);

	//damit nicht wegen  jedem append eine Reallocation vorgenommen werden muss
	faceVtxIDs->setSizeIncrement(incrementSize);
	
	unsigned int x = 0;	//counter für zuweisung der Daten zu offsetArrays

	for(;!polyIter.isDone();polyIter.next())
	{
		polyIter.getVertices(vertIDs);

		tmp = vertIDs.length();

		(*offsets)[x] = tmp;
		(*offsetsAbsolute)[x++] = count;

		count += tmp;
		
		helper.addIntArrayToLHS(*faceVtxIDs,vertIDs);

	}
	
	offsets->setSizeIncrement(incrementSize);
	offsetsAbsolute->setSizeIncrement(incrementSize);
	//UVs und normalen extrahieren
	

	//Arrays müssen mit listen versehen werden, aber da der meshcreatoe eh nur temporär ist, müssen sie nicht gesäubert werden vor der benutzung


		//initialisieren der Pointer
	
	//zuerst standardFloats machen, deren kopien nen vernünftigen sizeIncrement haben
	MIntArray standardInt;
	MFloatArray standardFloat;
	MFloatVectorArray standardVec;
	standardInt.setSizeIncrement(incrementSize);
	standardFloat.setSizeIncrement(incrementSize);
	standardVec.setSizeIncrement(incrementSize);
	
	Us = new list<MFloatArray>();
	Vs = new list<MFloatArray>();
	
	UVCounts =  new list<MIntArray>();
	UVperVtx = new list<MIntArray>();
	UVoffsetsLUT = new list<MIntArray>();
	UVoffsetsLUTValues = new list<MIntArray>();
	
	UVAbsOffsets = new list<MIntArray>();
	
	
	
	UVSlideDirections = new list<MFloatVectorArray>();
	UVSlideStart = new list<MFloatVectorArray>();
	UVSlideEnd = new list<MFloatVectorArray>();
	UVSlideIndices = new list<MIntArray>();
	
	UVSlideScale = new list<MFloatArray>();
	
	
	//UVs holen
	meshFn.getUVSetNames(UVSetNames);
	
	numUVSets = UVSetNames.length();
	
	
	//DEBUG !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	numUVSets = (numUVSets == 0) ? 0 : 1 ;
	UVSetNames.setLength(numUVSets);
	//DEBUG !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	
	UVMin.setLength(numUVSets);	//werden während der initialisierung gesetzt
	UVMax.setLength(numUVSets);
	//initialisieren
	UINT i;
	for(i = 0; i < numUVSets; i++)
	{
		UVMin[i] = 999999999;
		UVMax[i] = 0;
	}
	
	
	for(i = 0; i < numUVSets;i++)
	{
		MFloatArray tUs;
		MFloatArray tVs;				//temp Arrays
		
		tUs.setSizeIncrement(incrementSize);
		tVs.setSizeIncrement(incrementSize);
		
		MIntArray tUVCounts,tUVs;
		tUVs.setSizeIncrement(incrementSize);
		tUVCounts.setSizeIncrement(incrementSize);
		
		MPRINT("-------------------------------");
		INVIS(cout<<"Bearbeite UVSet: "<<UVSetNames[i].asChar()<<endl;);
		
		meshFn.getUVs(tUs,tVs,&UVSetNames[i]);
		
		INVIS(cout<<"Num UVs: "<<tUs.length()<<endl;);
		
		
		Us->push_back(tUs);
		Vs->push_back(tVs);
		
		
		
		
		meshFn.getAssignedUVs(tUVCounts,tUVs);
		
		
		UVCounts->push_back(tUVCounts);
		UVperVtx->push_back(tUVs);
		
		UVoffsetsLUT->push_back(standardInt);
		UVoffsetsLUTValues->push_back(standardInt);
		
		
		
		//UVSlideArrays initialisieren
		UVSlideDirections->push_back(standardVec);
		UVSlideStart->push_back(standardVec);
		UVSlideEnd->push_back(standardVec);
		UVSlideIndices->push_back(standardInt);
		
		UVSlideScale->push_back(standardFloat);
		
		
		UVBackup.push_back(standardInt);
		
		
		//------------------------------
		//jetzt die UVAbsOffsets erzeugenZ
		
		
		unsigned int tl = tUVCounts.length();
		
		
		MIntArray tAbsOffsets(tl);
		tAbsOffsets.setSizeIncrement(incrementSize);
		
		int tmp = 0;
		for(unsigned int x = 0; x < tl; x++)
		{
			tAbsOffsets[x] = tmp;
			tmp += tUVCounts[x];
		}
		
		UVAbsOffsets->push_back(tAbsOffsets);
		
		
		
	}
	
	
	
	
	
	
	
	
	//dieser Algo ist so angelegt, dass wirklich nur die vertizen gespeichert werden, die geteilte normalen haben (pro face)
	//dies bedeutet, dass alle anderen edges danach auf smooth gestellt sein werden
	
	//jetzt den SizeIncrement umstellen, so dass nicht bei jedem Append das Array umkopiert werden muss
	normalCount.setSizeIncrement(incrementSize);
	normalVertexIndices.setSizeIncrement(incrementSize);
	
	normalAbsOffsets.setSizeIncrement(incrementSize);	
	
	//diese Beiden Arrays bleiben erstmal 0
	normalOffsetsLUT.setSizeIncrement(incrementSize);	
	normalOffsetsLUTValues.setSizeIncrement(incrementSize);
	
	
	
	
	MIntArray containedEdges;	//tmpArray
	MIntArray faceVerts;		//tmpArray
	int2		edgeIndices;	//hält die 2 vertizen pro edge
	
	
	UINT absCount = 0; //hält den Gegenwärtigen abs offset
	count = 0;		//hält relativen Offset
	UINT l;
	
	
	//jetzt die NormalArrays aufbauen
	polyIter.reset();
	for(;!polyIter.isDone(); polyIter.next())
	{
		polyIter.getEdges(containedEdges);
		l = containedEdges.length();
		
		INVIS(UINT test = polyIter.index(););
		
		faceVerts.clear();
		
		count = 0;
		
		
		for(UINT i = 0; i < l; i++)
		{
			//!!!!MAN könnte dies beschleunigen durch wegfall des isFlipped lookups indem man einfach auf die faceVerts direkt zugreift.
			//Man müsste nur herausfinden, ob die contained edges in reihenfolge gegeben werden, also o, wie sie auch i face auftauchen
			
			edgeIter.setIndex(containedEdges[i],tmp);
			
			
			//if( !edgeIter.isSmooth() && !edgeIter.onBoundary() )
			if( !edgeIter.isSmooth()  )
			{
				
				if(faceVerts.length() == 0)
				{//entweder man checkt das vorher, holt aber nur dir vertizen, wenn wiklich nötig, oder man 
					//lässt den check aus, aber holt standardMäßig pro fce alle Daten - diese Variante ist wahrscheinlich besser 
					//in den meisten Fällen
					polyIter.getVertices(faceVerts);
				}
				
				//indices holen
				edgeIndices[0] = edgeIter.index(0);
				edgeIndices[1] = edgeIter.index(1);
				
				//checken, ob sie umgedreht werden müssen (edgeIndices müssen konform zum Face verlaufen)
				if(isEdgeFlipped(edgeIndices[0],edgeIndices[1],faceVerts))
				{
					tmp = edgeIndices[0];
					edgeIndices[0] = edgeIndices[1];
					edgeIndices[1] = tmp;
				}
				
				
				
				count += 2;
				
				normalVertexIndices.append(edgeIndices[0]);
				normalVertexIndices.append(edgeIndices[1]);
				
				
			}
			
		}
		
		
		
		//es werden nur die normalen gespeichert, die aus harten edges entstehen (unshared)
		normalCount.append(count);
		normalAbsOffsets.append(absCount);
		
		absCount += count;
		
		
	}
	
	INVIS(cout<<"LÄNGE DES NORMALVertexIndices: "<<normalVertexIndices.length()<<endl;);
	INVIS(cout<<"LÄNGE DES NORMALSCounts "<<normalCount.length()<<endl;);
	INVIS(cout<<"LÄNGE DES NORMALAbsOffset: "<<normalAbsOffsets.length()<<endl;);

	
}

//-----------------------------------------------------------------
meshCreator::~meshCreator()
//-----------------------------------------------------------------
{
//memory cleanup

	//mesh löschen
	delete vtxPoints;
	delete offsets;
	delete offsetsAbsolute;
	delete faceVtxIDs;

	delete offsetsLUT;
	delete offsetsLUTValues;



		if(deallocUVs)
		{//diese Daten werden eventuell von außen benutzt, auch nachdem der meshcreatoer zerstört wurde
			//deshalb shutz durch flag
			delete Us;
			delete Vs;

			delete UVCounts;
			delete UVperVtx;
		}
		

		delete UVoffsetsLUT;
		delete UVoffsetsLUTValues;

		delete UVAbsOffsets;

		
		//slide Arrays werden nie hier zerstört, sondern von fty selbst

	



}



//-----------------------------------------------------------------
MStatus 	meshCreator::createMesh(MObject& newMeshData)
//-----------------------------------------------------------------
{
	MStatus status;

	MFnMesh		meshCreator;
	MFnMeshData	FnMeshData;

	newMeshData = FnMeshData.create();
	meshCreator.create(vtxPoints->length(),offsets->length(), *vtxPoints, *offsets, *faceVtxIDs,newMeshData,&status);
	
	
	//jetzt die UVs aufs mesh übertragen 
	//MERKE:UVSliding wird von Fty gemacht
	
	//UVSets erstellen
	meshCreator.setObject(newMeshData);
	
	//UVSets erstellen, ab index 1 anfangen, da ein UVSet immer vorhanden ist
	//->funzt eh nicht
	UINT i;
	for(i = 1; i < UVSetNames.length();i++)
	{
		status = meshCreator.createUVSet(UVSetNames[i]);
		INVIS(status.perror("UV-PROBLEM, CreateSet"););
	}	
	
	//Iteratoren für UVPositions holen
	std::list<MFloatArray>::iterator UIter = Us->begin();
	std::list<MFloatArray>::iterator VIter = Vs->begin();
	
	
	//Iteratoren für Counts && UVPerVtx holen
	std::list<MIntArray>::iterator iterCount = UVCounts->begin();
	std::list<MIntArray>::iterator iterUVIDs = UVperVtx->begin();
	
	
	
	//U und V positions setzen pro UVSet
	for( i = 0; i < UVSetNames.length();i++)
	{/*
	 stat = meshCreator.createUVSet(UVSets[i]);
	 stat.perror("PROBLEM");	
	 
	   stat = meshCreator.setCurrentUVSetName(UVSets[i]);
	   stat.perror("PROBLEM");
		*/
		status = meshCreator.setUVs(*UIter,*VIter);
		INVIS(status.perror("UV-PROBLEM: SetUV"););
		
		status = meshCreator.assignUVs(*iterCount,*iterUVIDs);
		INVIS(status.perror("UV-PROBLEM: assignUV"););
		
		
		UIter++;
		VIter++;
		iterCount++;
		iterUVIDs++;
		
	}
	
	//Löschen der UVDaten darf nict hier vorgenommen werden, da sie später vom destruktor der FTY gelöscht werden
	
	
	


	//normalen erstellen, wenn möglich
	
	MIntArray	mFaceList;	//man muss bedenken, dass der MeshCreator wirklich nur passende face - vertex Kombinationen erzeugt !!!
	
	int count ;
	UINT x,y;
	int offset = 0; 
	int l2 ;
	int l3 ;
	int indexOne, indexTwo;		//hält den ersten index einer Edge
	MIntArray vertices;
	MIntArray edges;
	MIntArray smoothEdgeList;
	smoothEdgeList.setSizeIncrement(normalOffsetsLUT.length() + 20);	//so dürfte das Array ohne realloc auskommen
	
	MFnMesh meshFn(newMeshData,&status);	//hier koennte man eigentlich auch den meshCreator nehmen
	MItMeshPolygon polyIter(newMeshData);
	MItMeshEdge		edgeIter(newMeshData);
	
	INVIS(status.perror("PROB1 = ");)
		
		UINT l = normalCount.length();
	
	
	
	//im endEffekt muss nur eine passende FaceListe aufgebaut werden
	for(i = 0; i < l; i++)
	{
		
		if(normalCount[i] > 0)	//Merke: normalCount ist pro face
		{
			
			
			
			//edges des gegenwärtgen Face holen
			polyIter.getEdges(edges);
			polyIter.getVertices(vertices);
			
			count = normalCount[i];
			l3 = offset + count;
			l2 = edges.length();
			
			//das jetzt erstellte FAce muss nicht zwangsweise dieselbe richtung haben we das Face aus meinen Aufzeichnungen - 
			//dies muss jetzt gecheckt werden, um angemessen darauf zu reagieren
			//PS: es sind immer mindestens 2 indices im normalVertexIndicesArray pro face gespeichert, deshalb keinen rangeCheck machen
			
			//		INVIS(cout<<"!!!!!!!!!!!!!!!!!!!!!!Bearbeite Face "<<i<<"!!!!!!!!!!!!!!!!!!!!!!"<<endl;);
			
			for(x = 0; x < l2; x++)
			{//wenn index1 der Edge mit index 1 in normalVertexIndices ist, dann muss sie smooth gesetzt werden
				edgeIter.setIndex(edges[x],tmp);
				
				indexOne = edgeIter.index(0);
				indexTwo = edgeIter.index(1);
				
				
				//edgeRichting korrigieren
				if( isEdgeFlipped(indexOne, indexTwo,vertices) )
				{
					tmp = indexOne;
					indexOne = indexTwo;
					indexTwo = tmp;
				}
				
				//		INVIS(cout<<"gegenwärtige edgeVtx1: "<<indexOne<<endl;);
				//		INVIS(cout<<"gegenwärtige edgeVtx2: "<<indexTwo<<endl<<"-----------------------------"<<endl;);
				
				
				for(y = offset; y < l3; y += 2)
				{
					//		INVIS(cout<<"Vergleich mit NIndex1: "<<normalVertexIndices[y]<<endl;);
					//		INVIS(cout<<"NIndex2 = "<<normalVertexIndices[y+1]<<endl;);
					
					if(  indexOne == normalVertexIndices[y]  )
					{
						smoothEdgeList.append(edges[x]);
						
						//		MPRINT("GEFUNDEN!");
						
						break;
					}
				}
				
				//		INVIS(cout<<endl<<endl;);
			}
			
			offset += normalCount[i];
			
		}
		
		polyIter.next();
		
	}
	
	
	
	l2 = smoothEdgeList.length();
	for(x = 0; x < l2; x++)
	{
		edgeIter.setIndex(smoothEdgeList[x],tmp);
		edgeIter.setSmoothing(false);
	}
	
	
	INVIS(status.perror("PROB2 = ");)
		
		
		INVIS(cout<<"FaceList Länge ist: "<<mFaceList.length()<<endl; );
	INVIS(cout<<"normalVertex Länge ist: "<<normalVertexIndices.length()<<endl; );
	

	//SHADERS
	//Nun die shaderAssignments neu aufbauen
	
	//aus dem polyShaderIDs Array entsprechende componentLists zusammenstellen und diese ins ensprechende Plug setzen

	/*
	//dies aber nur, wenn mehrere Shader vorhanden sind
	for(x = 0; x < compPlugs.length(); x++)
	{
		//einfach die aktualisierten SelectionLists setzen
		cout<<compPlugs[x].name()<<" == compPlug"<<endl;
		
		cout<<compLists[x].apiTypeStr()<<" == compListType"<<endl;

		compPlugs[x].setValue(compLists[x]);
	}
	*/

	
/*	INVIS(\
	if(useUVs)\
	{\
		//statistik für UVs drucken//DEBUG
		std::list<MFloatArray>::iterator UIter= Us->begin();\
		std::list<MIntArray>::iterator countIter = UVCounts->begin();\
		std::list<MIntArray>::iterator UVIDsIter = UVperVtx->begin();\

		cout<<(*UIter).length()<<" = Länge des UVArrays"<<endl;\
		cout<<(*countIter).length()<<" = Länge des CountArrays"<<endl;\
		cout<<UVSetNames.length()<<" = UVSetNames"<<endl;\
		cout<<faceVtxIDs->length()<<" = FaceVtxIDs"<<endl;\

		unsigned int l = (*countIter).length();\
		int counter = 0;\
		//Countsper face drucken
		for(unsigned int i = 0; i < l; i++)\
		{\
			cout<<"UVCount für Face "<<i<<" == "<<(*countIter)[i]<<endl;\

			int l2 = counter + (*countIter)[i];\
			//einzelnde UVS ausdrucken
			for(int x = counter; x  < l2; x++)\
				cout<<(*UVIDsIter)[x]<<" == UV ID "<<endl;\

			cout<<endl;\
			counter += (*countIter)[i];\

		}\
	}\
	)//INVIS ENDE*/
	
	return status;
}

//-----------------------------------------------------------------------------------------
bool	meshCreator::isEdgeFlipped(int index1,
								   int index2,
								   const MIntArray& faceVertexList)
//-----------------------------------------------------------------------------------------
{
	//diese Prozedur bestimmt, ob die übergebenen EdgePunkte in faceRichtung sind oder nicht

	UINT l = faceVertexList.length();
	
	for(UINT i = 0; i < l; i++)
	{
		if(index1 == faceVertexList[i])
		{
			if(index2 == faceVertexList[(i+1) % l])
				return false;
			else
				return true;

		}
	}

	MPRINT("IS_EDGE_FLIPPED: WAR IN FORBIDDEN PATH");
	return false;
}

//-----------------------------------------------------------------------------------------
void	meshCreator::getSlideArrays(MDoubleArray*& inNormalScale,
									 MDoubleArray*& inSlideScale,
									 MVectorArray*& inSlideDirections,
									 MVectorArray*& inSlideNormals,
									 MPointArray*& inSlideStartPoints,
									 MPointArray*& inSlideEndPoints,
									 MIntArray*& inSlideInidices, 
									 MIntArray*& inMaySlide
									 )
//-----------------------------------------------------------------------------------------
{
	inNormalScale = normalScale;
	inSlideScale = slideScale;
	inSlideDirections = slideDirections;
	inSlideNormals = slideNormals;
	inSlideStartPoints = slideStartPoints;
	inSlideEndPoints = slideEndPoints;
	inSlideInidices = slideIndices;
	inMaySlide = maySlide;
}

//-----------------------------------------------------------------------------------------
void	meshCreator::getUVSlideArrays(	list<MFloatVectorArray>*&		inUVSlideStart,
										list<MFloatVectorArray>*&		inUVSlideEnd,
										list<MIntArray>*&				inUVSlideIndices,
										list<MFloatVectorArray>*&		inUVSlideDirections,
										list<MFloatArray>*&				inUVSlideScale)
//-----------------------------------------------------------------------------------------
{

inUVSlideStart = UVSlideStart; 
inUVSlideEnd = UVSlideEnd;
inUVSlideIndices = UVSlideIndices;
inUVSlideDirections = UVSlideDirections;
inUVSlideScale = UVSlideScale;

}


//-----------------------------------------------------------------------------------------
void	meshCreator::getUVData(	list<MFloatArray>*&			inUs,
								list<MFloatArray>*&			inVs,
								list<MIntArray>*&			inUVCounts,
								list<MIntArray>*&			inUVperVtx,
								MStringArray&				inUVSetNames
																		)
//-----------------------------------------------------------------------------------------
{

	inUs = Us;
	inVs = Vs;

	inUVCounts = UVCounts;
	inUVperVtx = UVperVtx;

	inUVSetNames = UVSetNames;

	//in diesem Fall wird dann die procedur, die dir Ptr geholt hat, für die deallokalisiereung verantwortlich
	//flag wird auf false gesetzt, so dass der Creator das nicht mehr macht
	deallocUVs = false;
}
																		
//---------------------------------------------------------------------------------------------------------------------------------
void	meshCreator::addSlide(int vtxID,MPoint startPos,MPoint endPos, MVector& direction, MVector& normal, double normalScaleValue)
//---------------------------------------------------------------------------------------------------------------------------------
{
	slideIndices->append(vtxID);
	
	slideStartPoints->append(startPos);
	slideEndPoints->append(endPos);
	slideDirections->append(direction);

	slideNormals->append(normal);
	normalScale->append(normalScaleValue);
	slideScale->append(-1);
	maySlide->append(1);
}

//---------------------------------------------------------------------------------------------------------------------------------
void	meshCreator::addNormalSlide(int vtxID, MVector& normal,double normalScaleValue)
//---------------------------------------------------------------------------------------------------------------------------------
{
	slideIndices->append(vtxID);
	slideStartPoints->append((*vtxPoints)[vtxID]);
	slideNormals->append(normal);
	normalScale->append(normalScaleValue);
	
	slideEndPoints->setLength(slideEndPoints->length() + 1);			//werden ebenfalls vergrößert, damit die Arrays in Sync bleiben
	slideDirections->setLength(slideDirections->length() + 1);
	
	slideScale->append(-1);			//slideScale muss immer synchron zu allen anderen bleiben

	maySlide->append(0);
}
//**************************************************
//*****************MESH_MODIFIKATOREN***************
//**************************************************

//-----------------------------------------------------------
void	meshCreator::getFaceVtxIDs(int faceID, MIntArray& array) const
//-----------------------------------------------------------
{
	
	int offset = getValidAbsoluteOffset(faceID);
	int count = offset + (*offsets)[faceID];
	
	array.setLength((*offsets)[faceID]);	//nun hat das Array garantiert genug slots zum halten der Infos
											//so dass beim append kein realloc gemacht wird

	array.clear();							//vohandene Einträge logisch löschen
	
	for(int i = offset; i < count; i++)
		array.append( (*faceVtxIDs)[i]);


}



//-----------------------------------------------------------------
int		meshCreator::newVtx(MPoint position)
//-----------------------------------------------------------------
{
	vtxPoints->append(position);
	return (vtxPoints->length() - 1);
}


//-----------------------------------------------------------------
int		meshCreator::createCenterVtx(int faceID)
//-----------------------------------------------------------------
{
	//holt centerPos von von Face und fügt diese als neuen Vtx hinzu, gitb ID von neuem Vtx zurück
	
	//diese variante erzeugt falsche centers! Also manuell machen
	//polyIter.setIndex(faceID,tmp);
	//MPoint centerPos = polyIter.center();

	MPoint centerPos;
	MIntArray faceVtx;
	getFaceVtxIDs(faceID,faceVtx);

	unsigned int l = faceVtx.length();
	for(unsigned int i = 0; i < l; i++)
		centerPos += (*vtxPoints)[faceVtx[i]];

	centerPos = centerPos / l;
	
	vtxPoints->append(centerPos);
	

	return vtxPoints->length() - 1;
}





//-----------------------------------------------------------------
void		meshCreator::cycleVtxMatch(MIntArray& vtxMatch,MIntArray& faceVerts)
//-----------------------------------------------------------------
{
	int l = faceVerts.length();
	bool	done = false,cycle = false;

	

	if(vtxMatch[0] != faceVerts[0])
	{
		while(!done)
		{
			cycle = false;
			//erstmal match[0] in facewerts finden
			int i;
			for(i = 0; i < l;i++)
			{
				if(vtxMatch[0] == faceVerts[i])
				{
					//		////cout<<"Index in CycleMatch = "<<i<<endl;
					break;
				}
			}
			
			if(i+1 < l)
				tmp = faceVerts[i+1];
			else
				tmp = faceVerts[0];
			
			
			//wenn tmp nicht in matchVtx vorkommt, dann ist cycle richtig,ansonsten drehen
			int l2 = vtxMatch.length();
			for(i = 1;i<l2;i++)
			{
				if(vtxMatch[i] == tmp)
				{
					cycle = true;
					break;
				}
			}
			
			if(!cycle)
				done = true;
			else
			{
				vtxMatch.append(vtxMatch[0]);
				vtxMatch.remove(0);
			}
			
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
int 	meshCreator::doFaceOneSplit(int faceID,
									MIntArray& verts,
									MIntArray& newVtxIndices,
									MIntArray& excludeIDs, 
									int& otherNachbar,
									MIntArray& resultUVs)
//-------------------------------------------------------------------------------------------------------------
{
	
	MIntArray	connectedFaces,vtxMatch,nFaceVtx,dump;
	
	polyIter.setIndex(faceID,tmp);
	polyIter.getConnectedFaces(connectedFaces);

	
	//erst alle excludedIDs von ConnectedFaces abziehen
	UINT i;
	for(i = 0; i < excludeIDs.length();i++)
	{
		for(unsigned int x = 0; x < connectedFaces.length();x++)
		{
			if(excludeIDs[i] == connectedFaces[x])
			{
				connectedFaces.remove(x);
				break;
			}
		}
	}
	
	//jetzt alle nachbarn der CornerEdgeVerts herausfinden und Reihenfolge der Faces
	MIntArray	faceIDs,bounds;

	//for(i = 0; i < connectedFaces.length();i++)
//	cout<<"VALID CORNER FACES = "<<connectedFaces[i]<<endl;

	
	int returnNachbar = -1;	//hält die absID des nachbarn, welcher nicht im Face verzeichenet ist

	bool isBoundary = orderNachbarn(connectedFaces,bounds,verts);
//	cout<<"Habe nachbarn geordnet,is Boundary ist "<<isBoundary<<endl;
	//wenn face auf boundary, dann soll einfach die BoundaryEdge genommen werden
	if(isBoundary)
		connectedFaces.setLength(1);
	
	if(connectedFaces.length() == 1)
	{
		int l = connectedFaces.length();
		for(i = 0; i < l;i++)
		{
			if(!isBoundary)
			{
				//polyIter.setIndex(connectedFaces[i],tmp);
				//polyIter.getVertices(nFaceVtx);
				getFaceVtxIDs(connectedFaces[i],nFaceVtx);
				returnNachbar = otherNachbar = connectedFaces[i];
			}
			else
			{
				nFaceVtx = verts;
			}
			
			vtxMatch = helper.matchArray(verts,nFaceVtx);
			
			if(vtxMatch.length() > 1)
			{
				if(vtxMatch.length() > 2)
					cycleVtxMatch(vtxMatch,nFaceVtx);
				
				//cout<<"DO_FACE_ONE_SPLIT: "<<"BEARBEITE FACE "<<connectedFaces[i]<<endl;
				//wenn größe ungerade, dann vorhandenen Vtx nutzen, ansonsten neuen erstellen, generell ohne jeden Slide
				if(vtxMatch.length() % 2 == 1)
				{
					//ist ungerade, also mittleren Vtx in newVtxIndices eintragen
					tmp = (vtxMatch.length() - 1)/2 ; // == locID von centerVtx
					
					tmp = vtxMatch[tmp];
					//jetzt diese VtxID finden und in NewIndices eintragen
					int l2 = verts.length();
					for(int a = 0; a < l2; a++)
					{
						if(tmp == verts[a])
						{
							newVtxIndices.append(a);
							break;
						}
					}
					
					
					
				}
				else
				{
					//neuer Vtx muss erstellt werden, anderes Face muss angepasst werden
					int middleLeft = vtxMatch.length() / 2 - 1;
					int middleRight = middleLeft + 1;		//alles locIDs
					
					MVector dir = (*vtxPoints)[vtxMatch[middleRight]] - (*vtxPoints)[vtxMatch[middleLeft]];
					
					vtxPoints->append( (*vtxPoints)[vtxMatch[middleLeft]] + slide * dir);
					int newID = vtxPoints->length() - 1;
					
					//jetzt newID in verts eintragen und locID in newIDs
					int l2 = verts.length();
					tmp = vtxMatch[middleLeft];
					int a;
					for(a = 0; a < l2; a++)
					{
						if(tmp == verts[a])
						{
							//Reihenfolge abchecken
							if(verts[(a+1)%l2] == vtxMatch[middleRight])
							{
								verts.insert(newID,a+1);
								dump.append(newID);
							}
							else
							{
								verts.insert(newID,a);
								dump.append(newID);
							}
							break;
						}
					}

					//UV für neuen Vtx im eigentlichen Face Type One wird in der aufrufenden Procedur erstellt, da 
					//erst bestimmt werden muss, ob slide erlaubt oder nicht.
					
					//jetzt noch das originalFace anpassen
					l2 = nFaceVtx.length();
					//tmp ist weiterhin middleLeft
					for(a = 0; a < l2; a++)
					{
						if(tmp == nFaceVtx[a])
						{
							if(nFaceVtx[(a+1)%l2] == vtxMatch[middleRight])
								nFaceVtx.insert(newID,a+1);
							
							else
								nFaceVtx.insert(newID,a);
							break;
							
						}
						
					}
					//poly  ändern, wenn vorhanden (wenn es also nicht auf boundary ist
					if(!isBoundary)
					{
						//Neue UV muss VOR dem change Poly erstellt werden, da diese dann gleich changeUV aufruft, und diese dann schon entsprechend viele 
						//UVs finden muss
						//neue UV erstellen für den neuen Vtx - slide ist erstmal false, wird dann aber true, sobald 
						//die sliedetermination gelaufen ist (wird also nachträglich ins slideArray hinzugefügt)
						MIntArray origVtx;	//hält die originalVertizen vor changePolyFace
						MIntArray thisUVs;	//hält neue UVs
						int thisFace = connectedFaces[i];

						getFaceVtxIDs(thisFace,origVtx);
						createUV(thisFace, newID, thisUVs, false,vtxMatch[middleLeft],vtxMatch[middleRight],true);
						


						createNormal(thisFace, newID,vtxMatch[middleLeft],vtxMatch[middleRight]);


						changePolyVtxIDs(thisFace,nFaceVtx);


						//jetzt noch die UVs anpassen und normalen fürs neue Face (diese methoden wurden früher von changePolyVtxIDs gerufen)

						changeUVIds(thisFace,thisUVs,nFaceVtx,origVtx);


						changeNormals(thisFace,nFaceVtx);

						//resultUVs schreiben
						resultUVs = thisUVs;

					}
				}
				
			}
			
		}
		

	//zum schluss alle newIDs aus dump holen, sie im bearbeiteten vertsArray finden und die indices speichern
		
		l = dump.length();
		int l2 = verts.length();
		for(i = 0; i < l; i++)
		{
			tmp = dump[i];
			
			for(int a = 0; a < l2; a++)
			{
				if(tmp == verts[a])
				{
					newVtxIndices.append(a);
					break;
				}
			}
		}
	}
	else
	{
		
		newVtxIndices = bounds;

	}

	if(!isBoundary)
		returnNachbar = -2;

	return returnNachbar;
	//jetzt auf jeden Fall sortieren -> jetzt eigentlich nicht mehr nötig
	//sortArray(newVtxIndices);
	
}

//---------------------------------------------------------//---------------------------------------------------------
bool	meshCreator::orderNachbarn(MIntArray&	faceIDs,MIntArray&	bounds, MIntArray& verts)
//---------------------------------------------------------//---------------------------------------------------------
{
	//faceIDs sind erst mit connected Faces gefüllt, und werden dann mit den gültigen Nachbarn neu geschrieben
	MIntArray	tmp,matchVtx,nVerts,tempIDs;
	int temp;
	int l = verts.length();
	MIntArray	save(20,-1);
	
	
//	for(int a = 0; a < faceIDs.length();a++)
		//cout<<faceIDs[a]<<" = connectedFace"<<endl; 

	
	for(unsigned int x = 0; x < faceIDs.length();x++)
	{
		polyIter.setIndex(faceIDs[x],temp);
		polyIter.getVertices(nVerts);
		
		matchVtx = helper.matchArray(verts,nVerts);
		
		int mL = matchVtx.length();

		if(mL > 2 )	
			cycleVtxMatch(matchVtx,verts);
		
		//Diese Variante gibt den Index für den StartVtx für jeden Nachbarn zurück, egal wie groß MatchVtx ist
		if(mL > 1)
		{
			for(int i = 0; i < l; i++)
			{
				
				if(verts[i] == matchVtx[0])
				{
					//wenn nach vorne und nach hinten dasselbe rauskommt, das von hinten nehmen
					if(verts[(i + mL-1)%l] == matchVtx[mL - 1])
					{
						if(verts[((i + l) - mL+1)%l] == matchVtx[mL - 1] )
							save[((i + l) - mL+1)%l] = faceIDs[x];
						else
							save[i] = faceIDs[x];
					}
					else
						save[( ((i + l) - mL+1)%l )] = faceIDs[x];;
					
					//return ( ((i + l + 1) - mL)%l );	
				}			
			}
		}
	}
	
	//durch saveArray gehen und IDs eintragen

	for(int i = 0; i < 20; i++)
	{
		if(save[i] != -1)
		{
			tmp.append(save[i]);
			tempIDs.append(i);
		}

	}
	
	if(tmp.length() == 0)
		return true;
	else
	{
		bounds.setLength(2);
		bounds[0] = tempIDs[1];
		bounds[1] = tempIDs[tempIDs.length() - 1];

		faceIDs = tmp;
	}
	return false;
}

//-------------------------------------------------------------------------------
void	meshCreator::filterBoundaryVtx(const MIntArray& inVtx, MIntArray& filterVtx)
//-------------------------------------------------------------------------------
{
	//FilterVtx sollte leer sein
	int l = inVtx.length();

	for(int i = 0; i < l; i++)
	{
		vertIter.setIndex(inVtx[i],tmp);
		
		if(vertIter.onBoundary())
			filterVtx.append(inVtx[i]);
	}
}


//-------------------------------------------------------------------------------------------------------------
int		meshCreator::newVtxBetweenIDs(int startID,int endID,int currentFace,bool useSlide,int nachbarFace)
//-------------------------------------------------------------------------------------------------------------
{
	
	MVector			normal,normalZwo;
	double			scale;
	
	
	
	polyIter.setIndex(currentFace,tmp);
	
	//es wird generell nur die gemittelte normale der beiden vtx gefunden, ungeachtet des Slides, weil sie meistens OK sein wird
	meshFn.getPolygonNormal(currentFace,normal);
	
	if(nachbarFace < 0)
	{
		polyIter.getArea(scale);
	}
	else
	{
		double temp;
		polyIter.getArea(scale);
		
		polyIter.setIndex(nachbarFace,tmp);
		polyIter.getArea(temp);

		meshFn.getPolygonNormal(nachbarFace,normalZwo);
		
		normal = (normal + normalZwo)/2;
		scale = (scale + temp) / 2;
	}

	//--------normal und scale berechnet

	int newID;
	double mySlide = 0.5;
	if(useSlide)
		mySlide = slide;
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Slide ist immer relativ zum startVtx. Wenn das anders sein soll, muss man bei der AgumentÜbergabe Start/EndVtxID vertauschen
	MPoint start = (*vtxPoints)[startID];
	MPoint end = (*vtxPoints)[endID];
	MVector direction = end - start;

	vtxPoints->append( start + mySlide * direction );

	newID = vtxPoints->length() - 1;


	
	//SlideDaten hinzufügen
	if(useSlide)
	{
		addSlide(newID,start,end,direction,normal,scale);
	}
	else
	{

		addNormalSlide(newID,normal,scale);
	}




	return newID;
}

//-------------------------------------------------------------------------------------------
int	meshCreator::getNormalsIndices(	int			faceID, 
										MIntArray&	inIndices) const
//-------------------------------------------------------------------------------------------
{
	//eventuell wurde vor dieser procedur bereits ein changePoly ausgeführt, so dass *Offsets eventell 1 zu groß sein kann
	//deshalb wird sicherheitshalber für diesen Fall 

	//dieser Check muss vermutlich nicht sein, das schon vorher überprüft wird
	//if(normalCount[faceID] == 0)
	//	return;

	int offset = getValidAbsNormalOffset(faceID);
	int l = offset + normalCount[faceID];
	int r = 0;
	inIndices.setLength(normalCount[faceID]);

	for(int i = offset; i < l; i++)
	{
		inIndices[r++] = normalVertexIndices[i];
	}

	return offset;
}




//----------------------------------------------------------------------------------------
void	meshCreator::createNormal(int faceID,
								  int newVtxID, 
								  int startAbsID, 
								  int endAbsID )
//----------------------------------------------------------------------------------------
{

	//nur weitermachen, wenn das face auch ungeteilte normalen hat
	if( normalCount[faceID] == 0)
		return;
	

	MIntArray normalIndices;
	UINT locIndex = 0;				//tempVariable für lokalen Index
	//erstmal die Normalen des Faces holen
	int offset = getNormalsIndices(faceID, normalIndices);


	//jetzt die Normalen an der start- und endLocID mitteln
	MFloatVector result;
	
	if(startAbsID > -1)
		//da jede Kante jetzt 2 vertiizen hat, muss alles immer durch 2 teilbar sein für gültigkeit
		//ist dies nicht der Fall, dann ist diese Ede nicht hart
		if( !helper.getLocIndexMod2(startAbsID,normalIndices,locIndex) || normalIndices[locIndex+1] != endAbsID  )
			return;



	//jetzt noch die normalArrays selbst modifizieren


	if(startAbsID == -1)
	{//centerVtxMode: diese Vertizen werden immer hinten (relativ zum Face) angehängt in der Form: -(centerIndex + 1)
		//außerdem werden diese nur einmal eingefügt, da jetzt noch kein Zusammenhang zu irgendeiner edge besteht
		offset = offset + normalCount[faceID] ;		
		
		normalVertexIndices.insert(-newVtxID-1,offset);


		//die anderen Arrays updaten
		updateNormalOffsetLUT(faceID,1);
		normalCount[faceID] += 1;

		//test
	//	getNormalsIndices(faceID, normalIndices);
	//	helper.printArray(normalIndices," = normalIndices nach insert center");
	}	
	else
	{//	neuer vtx wird ins normalArray eingefügt, und zwar doppelt, da durchs einfügen 2 edges entstehen, und deshalb auch 4 vertizen hintereinander liegen müssen
		offset = offset + locIndex + 1;		//plus 1, damit der neue Index direkt nach dem locIndex eingefügt wird
		normalVertexIndices.insert(newVtxID,offset);


		normalVertexIndices.insert(newVtxID,offset);

		//die anderen Arrays updaten (+2)
		updateNormalOffsetLUT(faceID,2);
		normalCount[faceID] += 2;
	}

	
	


}

//------------------------------------------------------------------------------------------------------------------------
void	meshCreator::getVtxPositions(const MIntArray& vtxList, MPointArray& pointsResult) const
//------------------------------------------------------------------------------------------------------------------------
{
	UINT i = 0;
	UINT l = vtxList.length();
	pointsResult.setLength(l);

	

	for(; i < l; i++)
	{
		pointsResult[i] = (*vtxPoints)[vtxList[i]];
	}

}

//------------------------------------------------------------------------------------------------------------------------
void	meshCreator::changeNormals(	int faceID, 
									MIntArray& localVtxChange, 
									bool append, 
									bool hasCenter	)
//------------------------------------------------------------------------------------------------------------------------
{	//MERKE: (int)CHANGE WIRD NICHT MEHR BENUTZT; IST ABER NOCH DRIN DAMIT ICH NICHT 150 aufrufe ändern muss

	//nur wenn face geteilte normalen hat, anfangen
	//if(normalCount[faceID] == 0 && normalIndicesBak.length() == 0)
	if(normalCount[faceID] == 0 )
	{
		if(append)
		{//aber noch die Arrays synchron halten - es müssen immer soviele normalCounts und offsets da sein wie Faces
			normalCount.append(0);
			normalAbsOffsets.append( normalAbsOffsets[ normalAbsOffsets.length() - 1] );
		}
		return;
	}



	MIntArray	 normalIndices; //array mit absoluten vertexindices, sync zu faceNormals
	UINT		 locId;			//tmp variale für lokalen index, der in normalIndices bzw. faceNormals zeigt


	MIntArray	 normalIndicesChange;	//array mit geänderten normalIndices
	normalIndicesChange.setSizeIncrement(localVtxChange.length());

	int offset = getValidAbsoluteOffset(faceID);
	//erstmal normalen holen, um den alten Vtx ihre normale zuordnen zu können
	

	//backup machen, wenn nicht im append mode
	if(!append)
	{
		getNormalsIndices(faceID, normalIndices);
		normalIndicesBak = normalIndices;
	}
	else
	{
		normalIndices = normalIndicesBak;
	}
	
	int vtxID, vtxID2;
	
	
	unsigned int l = localVtxChange.length();


	INVIS(cout<<"INDICES FÜR FACE: "<<faceID<<endl;);
	INVIS(helper.printArray(normalIndices," = normalIndices");)


	//centerID suchen, wenn nötig
	if(hasCenter)
	{//Center wurde als -(centerID + 1) am ende des FaceVtxSlots gespeichert
		int centerID;
		centerID = -normalIndices[normalIndices.length()-1]-1;

		//debug
		INVIS(if(centerID < 0){cout<<"ERROR, CENTER ID WAR NICHT AN RICHTIGER STELLE, gehe in normalen modus"<<endl; goto standardMode;});

		//jetzt die cangeArrays erstellen
		for(unsigned int i = 0; i < l; i++)
		{
			
			//alles immer Edgeweise bearbeiten
			vtxID = localVtxChange[i];
			vtxID2 = localVtxChange[(i+1) % l];

			//wenn centerID involviert ist und die Kante mit einer OriginalKante verbunden ist, die hard ist, dann soll diese Edge auch hard sein
			if(vtxID == centerID)
			{
				if(helper.getLocIndexMod2(vtxID2, normalIndices,locId) )
				{//verbindung zu harter edge vorhanden, also diese edge hinzufügen

					normalIndicesChange.append(vtxID );

					//immer alles edgeweise (also 2 vertices) aufs array packen

					normalIndicesChange.append(vtxID2);

				}
				
				continue;
			}
			else if( vtxID2 == centerID)
			{
				if(helper.getLocIndexMod2(vtxID, normalIndices,locId) )
				{//verbindung zu harter edge vorhanden, also diese edge hinzufügen

					normalIndicesChange.append(vtxID );

					//immer alles edgeweise (also 2 vertices) aufs array packen
					normalIndicesChange.append(vtxID2);

				}

				continue;
			}

			//if( helper.getLocIndex(vtxID,normalIndices, locId) )
			if( helper.getLocIndexMod2(vtxID,normalIndices, locId) && normalIndices[locId+1] == vtxID2)
			{

				normalIndicesChange.append(vtxID );

				//immer alles edgeweise (also 2 vertices) aufs array packen

				normalIndicesChange.append(vtxID2);

			}

		}//for(vtx in vtxChange) ENDE
	
	}
	else
	{//nonCenterMode

		INVIS(standardMode:;);

		for(unsigned int i = 0; i < l; i++)
		{
			//alles immer Edgeweise bearbeiten
			vtxID = localVtxChange[i];
			vtxID2 = localVtxChange[(i+1) % l];

			//spezialCheck: Wenn beide Vertizen neu sind und beide verbundene Edges hart sind, dann soll diese auch hart sein
			if(vtxID > initialVtxCount && vtxID2 > initialVtxCount)
			{
				UINT locID2;
				if(helper.getLocIndex(vtxID,normalIndices,locId) && helper.getLocIndex(vtxID2, normalIndices, locID2) )
				{

					normalIndicesChange.append(vtxID);


					normalIndicesChange.append(vtxID2);
					
					continue;
				}
			}

			//if( helper.getLocIndex(vtxID,normalIndices, locId) )
			if( helper.getLocIndexMod2(vtxID,normalIndices, locId) && normalIndices[locId+1] == vtxID2)
			{

				normalIndicesChange.append(vtxID );

				//immer alles edgeweise (also 2 vertices) aufs array packen

				normalIndicesChange.append(vtxID2);

			}

		}//for(vtx in vtxChange) ENDE
	}


	if(append)
	{//neues Face wurde erstellt, die Normalen müssen also angehängt werden	
		
		normalCount.append( normalIndicesChange.length() );

		helper.addIntArrayToLHS(normalVertexIndices,normalIndicesChange);


		//nur zur Sicherheit, damit man später auch garantiert auf die aktualisierten Daten zugreifen kann
		normalAbsOffsets.append( normalAbsOffsets[ normalAbsOffsets.length() - 1] + normalIndicesChange.length() );

		
	}
	else
	{//vorhandenes Face muss verändert werden

		updateNormalArraysWith(normalIndicesChange, faceID);

	}


	//fertig


}

//------------------------------------------------------------------------------------------------------------------------
void	meshCreator::updateNormalArraysWith(const MIntArray& normalIndicesChange, int faceID)
//------------------------------------------------------------------------------------------------------------------------
{
	//fügt die übergebenen normalIndices in große normalArrays ein

 		int l = normalIndicesChange.length();


		int change = l - normalCount[faceID];
		
		int aOffset = getValidAbsNormalOffset(faceID) ;
		int i = 0;	//i muss signed sein, damit es negativ werden kann!

		if(change < 0)
		{//slots entfernen
			for( i = 0; i > change; i--)
			{
				normalVertexIndices.remove(aOffset);
			}
		}
		else if(change > 0)
		{//slots hinzufügen (wird wohl kaum vorkommen im Fall der Normalen)
			for( i = 0; i < change; i++)
			{
				normalVertexIndices.insert( -1, aOffset);
			}
		}

		//jetzt sind arrays garantiert angepasst, also noch die neuen Datn übertragen
		UINT l2 = aOffset + l;
		UINT r = 0;
		for(i = aOffset; i < l2; i++)
		{
			normalVertexIndices[i] = normalIndicesChange[r++];
		}

		//restliche Arrays anpassen oder aktualisieren
		normalCount[faceID] = l;
		if(change != 0)
			updateNormalOffsetLUT(faceID,change);
		

}

//------------------------------------------------------------------------------------------------------------------------
void	meshCreator::createUV(int			currentFace,
							  int			newVtxID,
							  MIntArray&	newUVArray,
							  bool			useSlide,
							  int			startLocID, 
							  int			endLocID, 
							  bool			isAbsolute	)
//------------------------------------------------------------------------------------------------------------------------
{
	//erzeugt neuen UV, wenn möglich
	//wenn vtxID nicht gegeben, dann wird neue CenterUV erstellt

	

	//is Absolute: wenn true, dann sind die start - und endLocIDs absolute Vtx IDs und müssen erst zu relativen gemacht werden (pro UVSet)
	//dies wird nur von doFaceOneSplit so benötigt

	//iteratoren
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
	


	INVIS(cout<<endl<<endl;);
	MPRINT("BIN IN CREATE UV");
	MPRINT("---------------------");
	INVIS(cout<<currentFace<<" = currentFace"<<endl;);
	INVIS(cout<<newVtxID<<" = newVtxID"<<endl;);


	//NEWUVARRAY VORBEREITEN
	/////////////////////////
	//erstmal den Sizeincrement auf die benötigte größe stellen, damit beim append nur einmal ein realloc durchgeführt wird
	newUVArray.setSizeIncrement(numUVSets + 1);

	//Sie sind nur einmal auf dem Array (am Anfang), gefolgt von einer UVID pro UVSet ( -1, wenn keine UV)
	if(startLocID == -1)
	{//neue centerVtx sind immer eindeutig negativ, damit man sie später im Array noch fndet
		newUVArray.append( -(newVtxID + 1));
	}
	else
	{
		newUVArray.append(newVtxID);
	}
	UINT firstElement = newUVArray.length();	//helfer, der auf erstes freies Element im UVArray zeigt


	//indizes einstellen
	for(UINT i = 0; i < numUVSets; i++)
	{
		//gleich Anfangs einen leeren slot einfügen, der dann mit newUV beschrieben wird oder auch nicht
		newUVArray.append(-1);


		MIntArray& counts = *iterCounts;
		
		if(isAbsolute)
		{//relative Indices erzeugen
			MIntArray faceVtx ;
			getFaceVtxIDs(currentFace,faceVtx);

			unsigned int l = faceVtx.length();
			UINT x;
			for(x = 0; x < l; x++)
			{
				if(startLocID == faceVtx[x])
				{
					startLocID = x;
					break;
				}
			}

			for(x = 0; x < l; x++)
			{
				if(endLocID == faceVtx[x])
				{
					endLocID = x;
					break;
				}
			}

		}


		if(counts[currentFace])
		{//wenn UVs, dann bearbeiten
			MIntArray faceUVs;
			getFaceUVs(currentFace,faceUVs,i,iterOffset,iterLUT,iterLUTValues,iterCounts,iterUVperVtx);

			INVIS(cout<<endl<<endl;);
			INVIS(for(unsigned int u = 0; u < faceUVs.length(); u++))
			INVIS(cout<<faceUVs[u]<<" == faceUV"<<endl;);
			INVIS(cout<<endl<<endl;);

			int newUVID;

			MFloatArray& UArray = *iterUPos;
			MFloatArray& VArray = *iterVPos;



			if(startLocID == -1)
			{//UV im Zentrum erstellen

				MPRINT("CREATE CENTER VTX");

				float2 UV = {0.0,0.0};




				unsigned int l = faceUVs.length();
				for(unsigned int x = 0; x < l; x++)
				{
					UV[0] += UArray[faceUVs[x]];
					UV[1] += VArray[faceUVs[x]];			
				}

				UV[0] /= l;
				UV[1] /= l;

				//jetzt neuen UV erstellen und ins newUV array hinzufügen
				UArray.append(UV[0]);
				VArray.append(UV[1]);


				newUVID = UArray.length()-1;

				
				newUVArray[firstElement + i] = newUVID;

				
				INVIS(cout<<"Habe neuen CENTER UV erstellt für Vtx "<<newVtxID<<" == "<<UArray.length()-1<<endl;);


			}
			else
			{//UV zwischen start/end erstellen
				float2 UVStart = {0.0,0.0};
				float2 UVEnd = {0.0,0.0};



				int startUV =	faceUVs[startLocID];
				int endUV =		faceUVs[endLocID];

				UVStart[0] += UArray[startUV];
				UVStart[1] += VArray[startUV];

				UVEnd[0] += UArray[endUV];
				UVEnd[1] += VArray[endUV];

				
				float2 resultUV = {UVStart[0] + ((UVEnd[0] - UVStart[0])*slide),UVStart[1] + ((UVEnd[1] - UVStart[1])*slide)};

				UArray.append(resultUV[0]);
				VArray.append(resultUV[1]);


				
				newUVID = UArray.length()-1;
				
				//hinzufügen des neuen UVs
				newUVArray[firstElement + i] = newUVID;

				INVIS(cout<<"Habe neue UV erstellt für Vtx "<<newVtxID<<" == "<<UArray.length()-1<<endl;);




				//UVSliding initialisieren
				if(useSlide)
				{
					(*iterSlideIDs).append(newUVID);


					MFloatVector vec(UArray[faceUVs[startLocID]],VArray[faceUVs[startLocID]],0.0);
					(*iterSlideStart).append(vec);

					MFloatVector vec2(UArray[faceUVs[endLocID]],VArray[faceUVs[endLocID]],0.0);
					(*iterSlideEnd).append(vec2);

					(*iterSlideDirs).append(vec2 - vec);

				}

			}



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
	}

	//TODO: UVSlide System

}


//--------------------------------------------------------------------------------
void	meshCreator::checkForUV(int thisFaceID,
								int otherFaceID, 
								MIntArray& newUVs,
								const MIntArray& newNachbarUVs,
								int startVtxLocID, 
								int endVtxLocID,
								int newVtx,
								bool useSlide) const
//--------------------------------------------------------------------------------
{//checkt, ob der bereits vorhandene Vtx eine UV hat
 //useSlide wird benötigt, wenn diese Prozedur doch einen Vtx erzeugen muss, welcher dann natürlich einen eigenen slide benötigt
 //wenn nötig

	std::list<MIntArray>::iterator iterCounts		=		UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset		=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT			=		UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues	=		UVoffsetsLUTValues->begin();
	
	std::list<MIntArray>::iterator iterUVperVtx		=		UVperVtx->begin();

	std::list<MFloatArray>::iterator iterUPos		=		Us->begin();
	std::list<MFloatArray>::iterator iterVPos		=		Vs->begin();




	std::list<MIntArray>::iterator iterSlideIDs = UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideEnd = UVSlideEnd->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();


	INVIS(cout<<endl<<endl;);
	MPRINT("BIN IN CHECK FOR UV");
	MPRINT("---------------------");
	INVIS(cout<<thisFaceID<<" = THisFaceID"<<endl;);
	INVIS(cout<<otherFaceID<<" = otherFaceID"<<endl;);
	INVIS(cout<<newVtx<<" = newVtx"<<endl;);

	//NachbarArray vorbereiten
	//////////////////////////
	//zuersteinmal den newUVArray des nachbarns cachen für die spätere benutzung

	//gemeinsamen Vtx in diesem Array finden und nUVOffset setzen, so dass es auf erste UV zeigt
	UINT length = newNachbarUVs.length();
	int nUVOffset = -1;	//-1 ist flag, wenn kein passender Vtx in Array gefunden wurde
	UINT i;
	for(i = 0; i < length; i += numUVSets + 1)
	{
		//nach centerVtx wird hier nicht gesucht, da diese eh immer nur einem Face gehören
		if(newNachbarUVs[i] == newVtx)
		{
			nUVOffset = i+1;	//zeigt auf ersten UV des gefundenen Vtx
			break;
		}
	}
	
	//jetzt den eigenen newUVArray vorbereiten
	//er muss auf jeden Fall numUVSets + 1 Einheiten aufnehmen
	//man kann allerdings nicht davon ausgehen, dass es bereits durch createUV gesetzt wurde (manche Faces rufen niemals createUV auf)
	newUVs.append(newVtx);
	UINT firstElement = newUVs.length();	//zeigt auf den ersten Slot für neue UVs hinter diesem Vtx (slot wird später erstellt)

	//indizes einstellen
	for(i = 0; i < numUVSets; i++)
	{
		//newUV slot erstellen
		newUVs.append(-1);

		MIntArray& counts = *iterCounts;

		MIntArray thisFaceUVs, otherFaceUVs;

		if(counts[thisFaceID])
		{//wenn UVs, dann bearbeiten
			
			getFaceUVs(thisFaceID,thisFaceUVs,i,iterOffset,iterLUT,iterLUTValues,iterCounts,iterUVperVtx);

		}
		else	//hat keine UVs, also nächste Iteration starten
			continue;

		
		if(counts[otherFaceID])
		{

			getFaceUVs(otherFaceID,otherFaceUVs,i,iterOffset,iterLUT,iterLUTValues,iterCounts,iterUVperVtx);

		}
		
		MIntArray UVMatch;
		UVMatch = helper.matchArray(thisFaceUVs,otherFaceUVs);

		INVIS(cout<<endl<<endl;);
		INVIS(for(unsigned int u = 0; u < thisFaceUVs.length(); u++))
		INVIS(cout<<thisFaceUVs[u]<<" == thisFaceUVs"<<endl;);

		INVIS(cout<<endl<<endl;);

		INVIS(for(u = 0; u < otherFaceUVs.length(); u++))
		INVIS(cout<<otherFaceUVs[u]<<" == otherFaceUVs"<<endl;);

		INVIS(cout<<endl<<endl;);

		//wenn UVMatch NUR 1 ist, dann reict dies nicht aus für einen gemeinsamen UV
		if(UVMatch.length() > 1)
		{//UVs stimmen überein, also den erstellten UV des Nachbarns ins eigene Array übertragen
			//man muss hier nicht checken, ob nUVOffset -1 ist, da an dieser Stelle eine UV gefunden worden sein muss

			newUVs[firstElement + i] = newNachbarUVs[nUVOffset + i];

			//UVSliding initialisieren //wenn er hier ist besteht sehr wohl die möglichkeit, dass der gefundene UV noch keinen Slide hat, 
			//da er von einem typeOne (fake) face erstellt wurde (über meshCreator)
			if(useSlide)
			{
				MFloatArray& UArray = *iterUPos;
				MFloatArray& VArray = *iterVPos;
			
				(*iterSlideIDs).append(newUVs[firstElement + i]);


				MFloatVector vec(UArray[thisFaceUVs[startVtxLocID]],VArray[thisFaceUVs[startVtxLocID]],0.0);
				(*iterSlideStart).append(vec);

				MFloatVector vec2(UArray[thisFaceUVs[endVtxLocID]],VArray[thisFaceUVs[endVtxLocID]],0.0);
				(*iterSlideEnd).append(vec2);

				(*iterSlideDirs).append(vec2 - vec);

			}
		}
		else
		{//UVs ohne übereinstimmung, also einfach neue, eigene UV erstellen

			MPRINT("MUSS NEUEN UV ERSTELLEN");
			
			float2 UVStart = {0.0,0.0};
			float2 UVEnd = {0.0,0.0};
			
			
			MFloatArray& UArray = *iterUPos;
			MFloatArray& VArray = *iterVPos;
			
			int startUV =	thisFaceUVs[startVtxLocID];
			int endUV =		thisFaceUVs[endVtxLocID];
			
			UVStart[0] += UArray[startUV];
			UVStart[1] += VArray[startUV];
			
			UVEnd[0] += UArray[endUV];
			UVEnd[1] += VArray[endUV];
			
			
			float2 resultUV = {UVStart[0] + ((UVEnd[0] - UVStart[0])*slide),UVStart[1] + ((UVEnd[1] - UVStart[1])*slide)};
			
			UArray.append(resultUV[0]);
			VArray.append(resultUV[1]);
			


			int newUVID = UArray.length()-1;


			//hinzufügen des neuen UVs
			newUVs[firstElement + i] = newUVID;


			INVIS(cout<<"Habe neue UV erstellt für Vtx "<<newVtx<<" == "<<UArray.length()-1<<endl;);
			


			//UVSliding initialisieren
			if(useSlide)
			{
				(*iterSlideIDs).append(newUVID);


				MFloatVector vec(UArray[thisFaceUVs[startVtxLocID]],VArray[thisFaceUVs[startVtxLocID]],0.0);
				(*iterSlideStart).append(vec);

				MFloatVector vec2(UArray[thisFaceUVs[endVtxLocID]],VArray[thisFaceUVs[endVtxLocID]],0.0);
				(*iterSlideEnd).append(vec2);

				(*iterSlideDirs).append(vec2 - vec);

			}


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
	}

	bool hasNewUVs = false;
	length = firstElement + numUVSets;
	//jetzt noch letzte Überprüfung machen, ob überhaupt neue UVs eingefügt wurden
	for(i = firstElement; i < length; i++)
	{
		if(newUVs[i] > 0)
		{
			hasNewUVs = true;
		}
	}
	
	if(!hasNewUVs)
	{//die neuen indices entfernen
		length = numUVSets + 1;
		firstElement--;	//erstmal auf Vtx zeigen
		for(i = 0; i < length; i++)
			newUVs.remove(firstElement);

	}

}


//----------------------------------------------------------------------------------------------
void	meshCreator::addUVSlide(int slideAbsVtx, int startAbsVtxID,int endAbsVtxID, int faceID,MIntArray& newUVs)
//----------------------------------------------------------------------------------------------
{

	//diese procedur fügt sliding entsprechend der übergebenen parameter zu den UVS hinzu.
	
	std::list<MIntArray>::iterator iterCounts		=		UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset		=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT			=		UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues	=		UVoffsetsLUTValues->begin();
	
	std::list<MIntArray>::iterator iterUVperVtx		=		UVperVtx->begin();

	std::list<MFloatArray>::iterator iterUPos		=		Us->begin();
	std::list<MFloatArray>::iterator iterVPos		=		Vs->begin();




	std::list<MIntArray>::iterator iterSlideIDs =			UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideEnd =	UVSlideEnd->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs =	UVSlideDirections->begin();


	for(unsigned int i = 0; i < numUVSets; i++)
	{
		MIntArray& counts = *iterCounts;

		if(counts[faceID])
		{
			MIntArray faceUVs;
			getFaceUVs(faceID,faceUVs,i,iterOffset,iterLUT,iterLUTValues,iterCounts,iterUVperVtx);

			MIntArray faceVtx;
			getFaceVtxIDs(faceID,faceVtx);

			//jetzt dir absoluten IDs in face-relative umwandeln

			int startUV = -1, endUV = -1;

			unsigned int l = faceVtx.length();
			UINT x;
			for(x = 0; x < l; x++)
			{
				if(faceVtx[x] == startAbsVtxID)
				{
					startUV = faceUVs[x];
					break;
				}
			}

			for(x = 0; x < l; x++)
			{
				if(faceVtx[x] == endAbsVtxID)
				{
					endUV = faceUVs[x];
					break;
				}
			}

			//jetzt noch die centerUV finden im newVtxArray
			
			
			int centerUV = -1;

			bool found = false;
			int vtx = slideAbsVtx;
			unsigned int length = newUVs.length();


			for(unsigned int k  = 0; k < length;k+=numUVSets + 1)
			{//erst abbrechen, wenn neuer UV zu passendem Face gefunden ist
				if(newUVs[k] == vtx)
				{
					centerUV = newUVs[k + i + 1];	//vom gegenwärtigen Vertex i UVSets nach vorn
					found = true; 
					break;
				}
			}

			if(!found)
			{//wenn die obrige Suche nicht erfolgreich war, dann muss nach einem centerVtx gesucht werden, welcher sich wie folgt kodiert
				// centerVtx = -(newUVs + 1) bzw -newUVs - 1;
				for(unsigned int k  = 0; k < length;k+=numUVSets + 1)
				{//erst abbrechen, wenn neuer UV zu passendem Face gefunden ist

					if(- newUVs[k] - 1 == vtx)
					{
						centerUV = newUVs[k + i + 1];	//vom gegenwärtigen Vertex i UVSets nach vorn
						found = true; 
						break;
					}
				}
			}

			//debugtest
			INVIS( if(centerUV == -1 || startUV == -1 || endUV == -1) {cout<<"ERROR in ADDUVSLIDE : eineder UVs wurde nicht gefunden: ABBRUCH"<<endl;return; });

			//jetzt nochslideDaten extrahieren und in arrays hinzufügen
			MFloatArray& UArray = *iterUPos, VArray = *iterVPos;
			
			(*iterSlideIDs).append(centerUV);

			MFloatVector vec(UArray[startUV],VArray[startUV],0.0);
			(*iterSlideStart).append(vec);

			MFloatVector vec2(UArray[endUV],VArray[endUV],0.0);
			(*iterSlideEnd).append(vec2);

			(*iterSlideDirs).append(vec2 - vec);

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

	}

}

//--------------------------------------------------------------------------------
void	meshCreator::getFaceUVs(int faceID, MIntArray& UVIDs,bool UVRelOffsets[]) const
//--------------------------------------------------------------------------------
{

	//MERKE: UVRelOffsets muss bereits initialisiert sein mit numUVsets elementen!
	
	
	UVIDs.clear();
	
	//iteratoren
	std::list<MIntArray>::iterator iterCounts			= UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset			= UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT				= UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues		= UVoffsetsLUTValues->begin();
	std::list<MIntArray>::iterator iterUVIDs			= UVperVtx->begin();
	
	
	MIntArray resultTmp;
	
	for(UINT i = 0; i < numUVSets; i++)
	{
		//wenn face überhaupt UVs hat
		
		UVRelOffsets[i] = ((*iterCounts)[faceID] != 0) ? true : false;
		
		if( UVRelOffsets[i] )
		{
			
			getFaceUVs(faceID, resultTmp, i, iterOffset, iterLUT, iterLUTValues, iterCounts, iterUVIDs);
			
			helper.addIntArrayToLHS(UVIDs, resultTmp);
			
		}
		
		
		++iterCounts;
		++iterOffset;
		++iterLUT;
		++iterLUTValues;
		++iterUVIDs;
		
		
	}
	
	
	

	//ansonsten nichts tun


}

//--------------------------------------------------------------------------------
void	meshCreator::getUVPositions(const MIntArray& UVIds, MFloatArray& UVPos) const
//--------------------------------------------------------------------------------
{
	//UVPos ist wie folgt kodiert: Xn+0 = U, Xn + 1 = V, je UVSet, ; wenn UVs in diesem Set nicht vorhanden, dann werden dummyelemente angehängt
	
	//UVIDs: Hält die entsprechende UVId des UVSets, wenn in einem Set keine UVs vorhanden sind für das betreffende Face, dann ist UVIDs[x] == -1

	//diese Prozedur funktioniert mit beliebiger Zahl von UVs, allerdings nur, wenn deren Zahl pro UVSet nicht vari

	//ITERATOREN
	std::list<MFloatArray>::iterator UIter = Us->begin();
	std::list<MFloatArray>::iterator VIter = Vs->begin();

	UINT l = UVIds.length() / numUVSets;


	UVPos.setLength(l*2*numUVSets);
	UINT r = 0;
	UINT x = 0;

	for(UINT u = 0; u < numUVSets; u++)
	{
		for(UINT i = 0; i < l; i++)
		{
			UVPos[r++] = 0.0;
			UVPos[r++] = 0.0;
			
			if(UVIds[x] > -1)
			{
				UVPos[r-2] = (*UIter)[UVIds[x]];
				UVPos[r-1] = (*VIter)[UVIds[x++]];
			}
			
			
		}

		UIter++;
		VIter++;
	}

	//Da dummy elemente angehängt wurden, kann man die UVs sehr einfach dereferenzieren


}


//--------------------------------------------------------------------------------
void	meshCreator::getFaceUVs(int faceID,
								MIntArray& result,
								int uvSetIndex,
								std::list<MIntArray>::iterator& iterOffset,
								std::list<MIntArray>::iterator& iterLUT,
								std::list<MIntArray>::iterator& iterLUTValues,
								std::list<MIntArray>::iterator& iterCounts,
								std::list<MIntArray>::iterator& iterUVIDs) const
//--------------------------------------------------------------------------------
{

	int offset = getValidAbsoluteUVOffset(faceID,uvSetIndex,iterOffset,iterLUT,iterLUTValues);
	
	int length = offset + (*iterCounts)[faceID];


	result.setLength((*iterCounts)[faceID]);
	

	UINT r = 0;

	

	for(int i = offset; i < length; i++)
		result[r++] = ( (*iterUVIDs)[i] );


}

//--------------------------------------------------------------------------------
void	meshCreator::addNormalSlide(int vtxID,
									int currentFace,
									int nachbarFace)
//--------------------------------------------------------------------------------
{
	
	
	MVector			normal,normalZwo;
	double			scale;
	
	
	
	polyIter.setIndex(currentFace,tmp);
	
	


	meshFn.getPolygonNormal(currentFace,normal);
	
	//es wird generell nur die gemittelte normale der beiden vtx gefunden, ungeachtet des Slides, weil sie meistens OK sein wird
	
	
	if(nachbarFace < 0)
	{
		polyIter.getArea(scale);
	}
	else
	{
		double temp;
		polyIter.getArea(scale);
		
		meshFn.getPolygonNormal(nachbarFace,normalZwo);

		polyIter.setIndex(nachbarFace,tmp);
		polyIter.getArea(temp);
		
		scale = (scale + temp) / 2;
		normal = (normal + normalZwo)/2;
	}



	addNormalSlide(vtxID,normal,scale);

}

//---------------------------------------------------------------------------------------------------
void	meshCreator::addSlide(int vtxID,
							  int startID,
							  int endID,
							  int currentFace,
							  int nachbarFace)
//---------------------------------------------------------------------------------------------------
{

	MVector			normal,normalZwo;
	double			scale;
	
	
	
	//erst die normale holen
	polyIter.setIndex(currentFace,tmp);


	meshFn.getPolygonNormal(currentFace,normal);
	
	//es wird generell nur die gemittelte normale der beiden vtx gefunden, ungeachtet des Slides, weil sie meistens OK sein wird
	
	
	if(nachbarFace < 0)
	{
		polyIter.getArea(scale);
	}
	else
	{
		double temp;
		polyIter.getArea(scale);
		
		meshFn.getPolygonNormal(nachbarFace,normalZwo);

		polyIter.setIndex(nachbarFace,tmp);
		polyIter.getArea(temp);
		
		scale = (scale + temp) / 2;
		normal = (normal + normalZwo)/2;
	}

	
	//extra: wenn nachbarID -2(wird so nur von doFaceOneSplit zurückgegeben), dann normalSlide deaktivieren durch nullSetzen:
	if(nachbarFace == -2)
		normal *= 0;

	//dann die SlideDirection holen
	MPoint start = (*vtxPoints)[startID];
	MPoint end = (*vtxPoints)[endID];
	MVector direction = end - start;



	addSlide(vtxID,start,end,direction,normal,scale);

}


//-------------------------------------------------------------------------------------------------------------
void	meshCreator::addVtx(int vtxID,
							int edgeID, 
							int faceID)
//-------------------------------------------------------------------------------------------------------------
{

	
	
	edgeIter.setIndex(edgeID,tmp);

	int startID = edgeIter.index(0);
	int endID = edgeIter.index(1);
	
	//cout<<"startID: "<<startID<<endl;
	
	int aOffset = getValidAbsoluteOffset(faceID);
	//cout <<"arbeite mit OFfset: "<<aOffset<<endl;

//	jetzt richtige Stelle für neuen Vtx finden und ihn einfügen
	int l = aOffset + (*offsets)[faceID];



	
	//korrekte insertPosition finden
	for(int i = aOffset; i < l; i++)
		if((*faceVtxIDs)[i] == startID)
			if((*faceVtxIDs)[i+1] == endID)
			{
				(*faceVtxIDs).insert(vtxID,i+1);
			}
			else
				(*faceVtxIDs).insert(vtxID,i-1);


//	faceVtxCount(relativ) inkrementieren
	(*offsets)[faceID] = (*offsets)[faceID]++;

	updateOffsetLUT(faceID,1);

//	zum Test alles ausdrucken
//	for(i = aOffset; i < l+1; i++)
//		//cout<<faceVtxIDs[i]<<" = id nach bearbeitung"<<endl;

}



//-----------------------------------------------------------------
void	meshCreator::createPoly(MIntArray& vtxIDs)
//-----------------------------------------------------------------
{
	//Diese Methode geht davon aus, dass das VtxArray richtig geordnet
	//ist und diese Vtx im VtxPointsArray existieren

	(*offsets).append(vtxIDs.length());
	helper.addIntArrayToLHS(*faceVtxIDs,vtxIDs);
	
	//offsetsAbsolute muss nicht verändert werden, da er 
	//für die MeshCreation nicht benötigt wird.
	//er ist nur für reele faces nötig
	//->DIE ABSOLUTEN OFFSETS WERDEN AUCH FÜR NEUE FACES BENÖTIGT, WENN NORMALEN ERSTELLT WERDEN
	//-->ALSO AUCH ABSOFFSETS AKTUALISIEREN
	offsetsAbsolute->append( (*offsetsAbsolute)[ offsetsAbsolute->length() - 1 ] + vtxIDs.length() );

	//UVs werden von der aufrufenden Procedur in "Face" erzeugt bzw. die entsprechende methode wird aufgerufen
	//weil sie unbedingt die ID des original Faces benötigt
}


//----------------------------------------------------------------------------------------------------------------------------
void	meshCreator::changePolyVtxIDs(int faceID,const MIntArray& localVtxChange)
//----------------------------------------------------------------------------------------------------------------------------
{
	//Jede ID des Faces muss im Array sein
	int change;
	MIntArray	a,faceVtx;
	

	int aOffset = getValidAbsoluteOffset(faceID);
	

	int l = aOffset + localVtxChange.length();


	change =  localVtxChange.length() - (*offsets)[faceID];

	//erstmal neue element erstellen bzw. alte rausschmeissen wenn nötig/möglich
	if(change > 0)
		for(int i = 0; i < change; i++)
			faceVtxIDs->insert(-1,aOffset);
	else if(change < 0)
		for(int i = 0; i > change; i--)	
			faceVtxIDs->remove(aOffset);

	//jetzt stimmt das VtxArray auf jeden Fall mit der Zahl von VtxChange überein, un man kann die Daten 
	//übertragen
	int r = 0;
	for(int i = aOffset; i < l;i++)
		(*faceVtxIDs)[i] = localVtxChange[r++];

	//wenn vtcChangeArray größer/kleiner als original faceVtxCount
	//dann wird auch der OffsetArray verändert

	if(change != 0)
	{
		updateOffsetLUT(faceID,change);
		(*offsets)[faceID] += change;
	}

	//UVs werden von der aufrufenden prozedur in Face erstellt über meshCreator

}





//-----------------------------------------------------------------
void	meshCreator::changeUVIds(int faceID,
								 MIntArray& newUVs, 
								 MIntArray& localVtxChange, 
								 MIntArray& faceVtx,	//de original FaceVtx, ohne jede Änderung
								 bool append )
//-----------------------------------------------------------------
{

	//diese Procedur dient dem Aufbau der UVs des UVSets
	//sie wird nur einkmal pro original-face aufgerufen (also NICHT im AppendMode), weshalb man getrost auf die OriginalUVs zugreifen kann

	//wenn append wahr ist, ist ein neues poly erstellt worden und die gefundenen UVs werden angehängt, gearbeitet wird dann allerding nur auf UVBackup
	//da das originalFace schon verändert wurde

	std::list<MIntArray>::iterator iterCounts		=		UVCounts->begin();
	std::list<MIntArray>::iterator iterOffset		=		UVAbsOffsets->begin();
	std::list<MIntArray>::iterator iterLUT			=		UVoffsetsLUT->begin();
	std::list<MIntArray>::iterator iterLUTValues	=		UVoffsetsLUTValues->begin();
	
	std::list<MIntArray>::iterator iterUVIDs		=		UVperVtx->begin();


	
	std::list<MIntArray>::iterator iterBak			=		UVBackup.begin();

	//DEBUG
	INVIS(cout<<endl<<endl;);
	MPRINT("BIN IN CHANGE UVs");
	INVIS(cout<<"FaceID: "<<faceID<<endl;);
	INVIS(cout<<"append?: "<<append<<endl;);
	INVIS(cout<<"--------------------------"<<endl;);
	

	INVIS(helper.printArray(localVtxChange," = LocalVtxChange "););


	//DEBUG
	INVIS(helper.printArray(newUVs," = newUVs"));


	unsigned int l = localVtxChange.length();

	for(int i = 0; i < numUVSets; i++)
	{
		MIntArray& UVCountArray = *iterCounts;
		
		//nur wenn es in diesem UVSet überhaupt UVs gibt für dieses Face, weitermachen
		if(UVCountArray[faceID] != 0)
		{
			
			int offset = getValidAbsoluteUVOffset(faceID,i,iterOffset,iterLUT,iterLUTValues);
			 
			MIntArray faceUVs;
			

			if(!append)
			{
				getFaceUVs(faceID,faceUVs,i,iterOffset,iterLUT,iterLUTValues,iterCounts,iterUVIDs);
				*iterBak = faceUVs;
			}
			else
				faceUVs = *iterBak;	//= die Original FaceUVs, ohne jede Änderung und synchron zu &faceVtx

			INVIS(for(unsigned int f = 0; f < faceUVs.length(); f++))
				INVIS(cout<<"FaceUV == "<<faceUVs[f]<<endl;);


			MIntArray UVChange;

			for(unsigned int x = 0; x < l; x++)
			{
				if(localVtxChange[x] > initialVtxCount)
				{//ist ein neuer Vtx - UV muss in newUVsArray gesucht werden
				 //zwei suchmodi: Suche nach standardVertizen und nach centerVtx

					bool found = false;
					int vtx = localVtxChange[x];
					unsigned int length = newUVs.length();
					

					for(unsigned int k  = 0; k < length;k+=numUVSets + 1)
					{//erst abbrechen, wenn neuer UV zu passendem Face gefunden ist
						if(newUVs[k] == vtx)
						{
							UVChange.append(newUVs[k + i + 1]);	//vom gegenwärtigen Vertex i UVSets nach vorn
							found = true; 
							goto startNextCycle;
						}
					}

					if(!found)
					{//wenn die obrige Suche nicht erfolgreich war, dann muss nach einem centerVtx gesucht werden, welcher sich wie folgt kodiert
						// centerVtx = -(newUVs + 1) bzw -newUVs - 1;
						for(unsigned int k  = 0; k < length;k+=numUVSets + 1)
						{//erst abbrechen, wenn neuer UV zu passendem Face gefunden ist

							if(- newUVs[k] - 1 == vtx)
							{
								UVChange.append(newUVs[k + i + 1]);	//vom gegenwärtigen Vertex i UVSets nach vorn
								found = true; 
								goto startNextCycle;
							}
						}
					}

					//letzter Durchlauf: Wenn er hier noch nichts gefunden hat, dann hat diese methode mit doFaceOneSplit zu tun, so dass die neue UV
					//nur mit der standardSuche gefunden werden kann, auch wenn der Vtx eigentlich ein neu hinzugefügter ist.
					//das geht, da das Face bereits aktualisiert wurde (auch die UVs)
					if(!found)
						goto alternativeSearch;

					//letzte Kontrolle
					//INVIS(if (!found){ cout<<"WARNUNG, NEW VTX VON FACE "<<faceID<<" WURDE WEDER MIT DER STANDARD, NOCH MIT DER ERWEITERTEN SUCHE GEFUNDEN!!! ->ABBRUCH"<<endl; return; });

				
				}
				else
				{//UV suchen in vorhandenem Mesh
					
alternativeSearch:	//wird von newVtx search angesprungen

					//suchen nach Vtx ID, um die LocVtx ID zu erhalten
					int vtx = localVtxChange[x];
					for(unsigned int k = 0; k < faceVtx.length(); k++)
					{
						if(faceVtx[k] == vtx)
						{
							INVIS(int tmp = faceUVs[k];);
							UVChange.append(faceUVs[k]);
							break;
						}
					}
				}
				
startNextCycle:;

			}

			MIntArray& UVIDs = *iterUVIDs;

			INVIS( \
				if(UVChange.length() != localVtxChange.length()) \
				{													\
				cout<<"FEHLER: UVCount stimmt nicht mit VtxCount überein!! Müsste "<<localVtxChange.length()<<"sein, ist aber "<<UVChange.length()<<endl; \
				//0 an UVCount hängen, um die synchronität zu wahren 
				UVCountArray.append(0); \
				return; \
				} \
				)
			


			if(append)
			{//neues UVFace erstellen
				UINT length = UVChange.length();
				helper.addIntArrayToLHS(UVIDs,UVChange);

				INVIS(helper.printArray(UVChange," = UVCHANGE"));
				//UVCounts synchronisieren (per Face)
				UVCountArray.append(length);
						
				
			}
			else
			{

				int aOffset = offset + UVChange.length();
				
				int change = localVtxChange.length() - faceVtx.length();

				//jetzt noch den Arraybereich anpassen
				if(change > 0)
					for(int k = 0; k < change; k++)
						UVIDs.insert(-1,offset);
				else if(change < 0)
					for(int k = 0; k > change; k--)	
						UVIDs.remove(offset);

				//neue ergebnisse übertragen
				int r = 0;
				for(int k =offset; k < aOffset;k++)
				{
					INVIS(cout<<"UVChange ist: "<<UVChange[r]<<endl;);
					UVIDs[k] = UVChange[r++];
				}

				//LUT aktualisieren
				if(change != 0)
				{
					updateOffsetUVLUT(faceID,change,i,iterLUT,iterLUTValues);
					(*iterCounts)[faceID] += change;
				}
			}
			
			
		}
		else
		{//UVCounts aktualisieren wenn im append modus; 
		 //da offset (absolut) nicht aktualisiert wird, kann man im nachhinein nicht auf neue Faces zugreifen, was allerdings auch nicht nötig ist
			if(append)
				UVCountArray.append(0);

		}

		++iterCounts;
		++iterUVIDs;
		
		++iterLUT;
		++iterLUTValues;
		++iterOffset;

		++iterBak;
	}
}


//-----------------------------------------------------------------
double		meshCreator::getFaceSize(int faceID) const
//-----------------------------------------------------------------
{
	MIntArray faceVtxList;
	MPointArray positions;

	getFaceVtxIDs(faceID,faceVtxList);

	getVtxPositions(faceVtxList,positions);

	return getFaceSize(positions);

}


//-----------------------------------------------------------------
double		meshCreator::getFaceSize( const MPointArray& pointList) const
//-----------------------------------------------------------------
{

	//!!!GRUNDSÄTZLICH muss die methode so schnell wie möglich routinen erhalten, die die Triangulation des Faces errechnen, so dass ich nicht Maya
	//objecte bemühen muss, die ich nicht wirklich unter kontrolle habe!!

	UINT l = pointList.length();

	double size ;
	MIntArray vtxList(l);

	//Sequenz von vtx IDs aufbauen
	for(UINT i = 0; i < l; i++)
		vtxList[i] = i;

	//jetzt mesh erzeugen, und dann alles damit machen
	MFnMeshData mDataFn;									//erzeugt ein meshDatenObject
	MFnMesh meshFn;	//man muss lokale instanz erzeugen, da die Instanz des MeshCreators auf dem OriginalMesh stehen sollte (vielleicht später noch ändern)
	MObject tmpMesh = mDataFn.create();


	meshFn.create(l, 1, pointList, MIntArray(1,l), vtxList, tmpMesh);


	//jetzt die größe des Faces holen und zurückgeben
	MItMeshPolygon polyIter(tmpMesh);

	polyIter.getArea(size);

	return size;
}

//-----------------------------------------------------------------
int		meshCreator::getValidAbsNormalOffset(int faceID) const
//-----------------------------------------------------------------
{

	int offset = normalAbsOffsets[faceID];
	int l = normalOffsetsLUT.length();


	if(nMinID > faceID)
		return offset;
	

	for(int i = 0;i < l ;i++)
	{
		if( normalOffsetsLUT[i] < faceID)
			offset += normalOffsetsLUTValues[i];			
	}

	

	return offset;


}

//-----------------------------------------------------------------
int		meshCreator::getValidAbsoluteOffset(int faceID) const
//-----------------------------------------------------------------
{
	int offset = (*offsetsAbsolute)[faceID];
	int l = offsetsLUT->length();


/*	//das hier lohnt sich im grunde nicht
	if(l == 0)
	{
		return offset;
	}
	else
	{*/
		//wenn faceID vor allen offsets, dann gleich abrechen
		if(minID > faceID)
			return offset;
		

		for(int i = 0;i < l ;i++)
		{
			if((*offsetsLUT)[i] < faceID)
				offset += (*offsetsLUTValues)[i];
			//else ... hier nicht abbrechen, da die IDs ungeordnet im array sind und später
			//nochmal eine passende ID kommen kann
			
		}
		
//	}


	return offset;
}

//-----------------------------------------------------------------
int		meshCreator::getValidAbsoluteUVOffset(int faceID,
											  int index,
											  std::list<MIntArray>::iterator& iterOffset,
											  std::list<MIntArray>::iterator& iterLUT,
											  std::list<MIntArray>::iterator& iterLUTValues) const
//-----------------------------------------------------------------
{


	
	MIntArray& absOffset = *iterOffset;
	

	int offset = absOffset[faceID];
	
	MIntArray& LUT = *iterLUT;
	unsigned int l = LUT.length();

/*	//lohnt sich nicht
	if(l == 0)
	{	
		return offset;
	}
	else
	{*/
		if(UVMin[index] > faceID)
		{
			return offset;
		}
		else
		{
			
			MIntArray& LUTValues = *iterLUTValues;

			for(int i = 0;i < l ;i++)
			{
				if(LUT[i] < faceID)
					offset += LUTValues[i];
				//else ... hier nicht abbrechen, da die IDs ungeordnet im array sind und später
				//nochmal eine passende ID kommen kann
			}
			
			return offset;
			
		}
		
//	}
		
		
	
	
	return offset;
}


//-----------------------------------------------------------------
void	 meshCreator::updateOffsetLUT(int faceID, int offsetCount)
//-----------------------------------------------------------------
{

	//wenn er hier hin kommt, dann sind UVs für faceID vorhanden - dies wird von master Proc gecheckt

	int l = offsetsLUT->length();


	
	//IDBounds setzen
	if(faceID < minID)
		minID = faceID;
	if(faceID > maxID)
		maxID = faceID;
	

	
//	if( (l == 0) || (faceID > maxID))
	if( (faceID > maxID) )
	{
		offsetsLUT->append(faceID);
		offsetsLUTValues->append(offsetCount);
		//cout <<"!!Habe erstes LUT element angehängt!!"<<endl;
		return;
	}
	
	
	//Eintrag im LUT suchen und offset aktualisieren
	for(int i = 0; i < l; i++)
	{
		if((*offsetsLUT)[i] == faceID)
		{
			(*offsetsLUTValues)[i] +=offsetCount;


			return;
		}
	}	

	offsetsLUT->append(faceID);
	offsetsLUTValues->append(offsetCount);
	//cout <<"!!Habe OFFset nich gefunden und deshalb angehängt!!"<<endl;


	

}

//-----------------------------------------------------------------
void	meshCreator::updateNormalOffsetLUT(int faceID, int offsetCount)
//-----------------------------------------------------------------
{
	int l = normalOffsetsLUT.length();


	if(faceID < nMinID)
		nMinID = faceID;
	if(faceID > nMaxID)
		nMaxID = faceID;
	
	
	if(faceID > nMaxID)
	{
		normalOffsetsLUT.append(faceID);
		normalOffsetsLUTValues.append(offsetCount);

		return;
	}

	//Eintrag im LUT suchen und offset aktualisieren
	for(int i = 0; i < l; i++)
	{
		if( normalOffsetsLUT[i] == faceID)
		{
			normalOffsetsLUTValues[i] +=offsetCount;
			return;
		}
	}	

	//ansonsten offset anhängen
	normalOffsetsLUT.append(faceID);
	normalOffsetsLUTValues.append(offsetCount);
	

	

}

//--------------------------------------------------------------------------------
void	meshCreator::getConnectedFaces(UINT vtxID, MIntArray& connectedFaces) const
//--------------------------------------------------------------------------------
{
	//die verbundenen Vtx herausfinden, indem man durchs CountArray parsed und 
	//im FaceVtxIndicesArray checkt, ob die übergebene VtxID vorhanden ist

	connectedFaces.clear();

	UINT l = offsets->length();
	UINT absOffset = 0;
	UINT maxLength,x;

	for(UINT i = 0; i < l; i++)
	{
		maxLength = absOffset + (*offsets)[i];

		for(x = absOffset; x < maxLength;x++)
		{
			if((*faceVtxIDs)[x] == vtxID)
			{//face i beinhaltet vtx, also auf liste setzen und mit nächster iteration in huptschleife weitermachen
				connectedFaces.append(i);
				break;
			}

		}

		absOffset += (*offsets)[i];
	}

	return;

}

//-----------------------------------------------------------------
void	 meshCreator::updateOffsetUVLUT(int faceID, 
										int offsetCount, 
										int index,
										std::list<MIntArray>::iterator& iterLUT,
										std::list<MIntArray>::iterator& iterLUTValues)
//-----------------------------------------------------------------
{
	//wenn er hier hin kommt, dann sind UVs für faceID vorhanden - dies wird von master Proc gecheckt



	MIntArray* LUT = &(*iterLUT);
	MIntArray* LUTValues = &(*iterLUTValues);

	int l = LUT->length();


	
	//IDBounds setzen
	if(faceID < UVMin[index])
		UVMin[index] = faceID;
	if(faceID > UVMax[index])
		UVMax[index] = faceID;
	

	
//	if( (l == 0) || (faceID > maxID))
	if(  faceID > maxID )
	{
		LUT->append(faceID);
		LUTValues->append(offsetCount);
		
		return;
		//cout <<"!!Habe erstes LUT element angehängt!!"<<endl;
	}
	else
	{
		//Eintrag im LUT suchen und offset aktualisieren
		for(int i = 0; i < l; i++)
		{
			if((*LUT)[i] == faceID)
			{
				(*LUTValues)[i] +=offsetCount;
				//cout <<"!!Habe Offset im array gefunden!!"<<endl;
				return;
			}
		}
		
		//wenn nichts gefunden wurde, dann einfach offset anhängen
		LUT->append(faceID);
		LUTValues->append(offsetCount);
			//cout <<"!!Habe OFFset nich gefunden und deshalb angehängt!!"<<endl;
	
	
	}

}


