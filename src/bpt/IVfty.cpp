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

#include "IVfty.h"



//--------------------------------------------------------------
IVfty::IVfty(void):	maxStandardScale(1.0),

					UVSlideStart(0),	
					UVSlideEnd(0),
					UVSlideIndices(0),
					UVSlideDirections(0),	
					UVSlideScale(0),

					normalScale(0),
					slideScale(0),		
					slideDirections(0),
					slideNormals(0)	,
					slideStartPoints(0),
					slideEndPoints(0),
					slideIndices(0),
					maySlide(0),

					spin(0)	//nur zur Sicherheit, is aber eigentlich unnoetig
//---------------------------------------------------------------
{
}


//-------------------
IVfty::~IVfty(void)
//-------------------
{

	if(UVSlideStart != 0)
		{
			delete UVSlideStart;	
			delete UVSlideEnd;
			delete UVSlideIndices;
			delete UVSlideDirections;	
			delete UVSlideScale;		
		}

	if(slideDirections != 0)
	{
		delete normalScale;		
		delete slideScale;			
		delete slideDirections;	
		delete slideNormals;		
		delete slideStartPoints;
		delete slideEndPoints;
		delete slideIndices;
		delete maySlide;
	}

}

/*
//-------------------------------------------------------------------------------------------
void	IVfty::getUVIntArrays(list<MIntArray>*& inUVCounts,list<MIntArray>*& inUVperVtx )
//-------------------------------------------------------------------------------------------
{
	inUVCounts = UVCounts;
	inUVperVtx = UVperVtx;
}

//-----------------------------------------------------------------
void	IVfty::getUVPositions(list<MFloatArray>*& inU, list<MFloatArray>*& inV)
//-----------------------------------------------------------------
{
	inU = Us;
	inV = Vs;
}
*/

//-------------------------
void	IVfty::cleanUp()
//-------------------------
{



	//maxStandardScale = 1.0 --> Muss nicht zurückgesetzt werden, da sich die FaceNormalen natürlich nicht ändern

	if(UVSlideStart != 0)
	{
		delete UVSlideStart;	
		delete UVSlideEnd;
		delete UVSlideIndices;
		delete UVSlideDirections;	
		delete UVSlideScale;	

		UVSlideStart = 0;	
		UVSlideEnd = 0;
		UVSlideIndices = 0;
		UVSlideDirections = 0;	
		UVSlideScale = 0;	
	}

	if(slideDirections != 0)
	{
		delete normalScale;		
		delete slideScale;			
		delete slideDirections;	
		delete slideNormals;		
		delete slideStartPoints;
		delete slideEndPoints;
		delete slideIndices;
		delete maySlide;

		normalScale = 0;		
		slideScale = 0;			
		slideDirections = 0;	
		slideNormals = 0;		
		slideStartPoints = 0;
		slideEndPoints = 0;
		slideIndices = 0;
		maySlide = 0;
	}
}

//-------------------------
//::In - None
//
//::out - MStatus
//
//Desctiption:
//--------------
/*
	
	Führt die eigentliche aktion aus(Erstellung des neuen Meshes, UVs, Normals, initialisierung des Slides)

*/

//---------------------------
MStatus		IVfty::doIt()
//---------------------------
{


	MStatus stat;

	//grundsätzlich muss diese prozedur im SelectMode sein - um dies sicherzustellen, jedes mal in den selectModegehen
//	MGlobal::executeCommand("setToolTo selectSuperContext",false,false);

	cleanUp();		//wird gerufen, damit die Ptr geloescht werden für den Fall, dass nur count geändert wurde
	
	//slidePathes werden einzeln gepeichert, so dass vor der topologieänderung noch geprüft werden kann, wie sich separate pathes zueinander Verhalten

	list<MIntArray> validPathes;		//gültige Pfade bestehen aus edges, wo sich maximal 2 edges ein face teilen
	list<MIntArray> startEndIDs;		//gültige pfade verfügen über eine richtung - dieses Array erfasst die start/end vtxIDs
	MIntArray		invalidPathes;		//alle ungültgen EdgeIDs, die die o.g norm nicht erfüllen

	MFnMesh meshFn(fMesh);
	int initialVtxCount = meshFn.numVertices();



	//creator erstellen für MeshArbeiten
	IVMeshCreator creator(fMesh,count,slide, meshPath);
	INVIS(cout<<meshPath.fullPathName().asChar()<<" == meshPathName"<<endl);



	//wenn edgeIDs keine vorhanden sind, dann gleich mit connect weitermachen, da ansonsten eine methode abstürzen würde
	if(edgeIDs.length() != 0)
	{
		
		trenneEdgePathes(validPathes, startEndIDs,invalidPathes);
			
		
		insertVertices(creator,validPathes,startEndIDs,invalidPathes);
	
	}



	if(options[9])
	{//connectVertces benoetigt eine VtxListe, welche jetzt erstellt wird - es sind nur neue Vertizen
		
		UINT l = creator.getLastVtxID() + 1 - initialVtxCount;	//+1, da creator die letzte ID wiedergibt, wie aber die gegenwärtige VtxZahl benoetigen
		MIntArray vtxList(l);
		
		l += initialVtxCount;
		UINT r = 0;

		for(UINT i = initialVtxCount; i < l; i++)
			vtxList[r++] = i;


		//jetzt zur vertexList die eventuell gewählten Vertizen hinzutun
		//so das connect sie mit einbezieht
		helper.addIntArrayToLHS(vtxList,vertIDs);

		//bevor man beginnen kann, muss der gegenwärtige initialState im creator aktualisiert werden, 
		// so dass er die neuen vertizen als gegeben ansieht
		creator.applyCurrentState();


		//jetzt vertex fty initialisieren ... 
		vertexFty  vertexWorker(&vtxList, &creator,spin);
		
		vertexWorker.doIt();

	}

	//slideArrays holen
	creator.getSlideArrays(	normalScale, 
							slideScale,
							slideDirections,
							slideNormals,
							slideStartPoints,
							slideEndPoints,
							slideIndices,
							maySlide	);

	//an dieser Stelle muss unbedingt noch ein Eintrag zum slideIndices Array hinzugefügt werden, der den 
	//letzten index bezeichnet (dies wird in slideprocedur benoetigt)
	slideIndices->append(creator.getLastVtxID() + 1);	//PLUS 1: slideRoutine geht davon aus, das dies der Minindex einer 
														//anderen edge ist, und subtrahiert 1, damit se ihren maxIndex erhält



	//UVSlideArrays holen
	if(options[7])
		creator.getUVSlideArrays(	UVSlideStart,
									UVSlideEnd,
									UVSlideIndices,
									UVSlideDirections,
									UVSlideScale		);

	//mesh erzeugen (erzeugung eines neuen Datenblocks wird in createMesh gemacht)
	
	creator.createMesh(newMeshData);


	//zum schluss noch die Auswahl und die component mas modifizieren
	

	modifySelectionList();



	return stat;
}


//---------------------------------------------------------------------
void	IVfty::insertVertices(		IVMeshCreator&	 creator,
									list<MIntArray>& validPathes,
									list<MIntArray>& startEndIDs,
									MIntArray&		 invalidPathes	)
//---------------------------------------------------------------------
{
	//Ungültige pfade werden einfach gleichmässig gesplittet entsprechend des counts
	
	//gültige Pfade werden gesplittet unter Verwendung des count und slide attributes sowie der startEndIDs


//********************
//LOS GEHT'S!
//********************

//ZUERST DIE INVALID PATHES SPLITTEN
	UINT l = invalidPathes.length();


	UINT i;
	for(i = 0; i < l; i++)
	{

		creator.doInsertVtx( invalidPathes[i] );
	
	}


//jetzt die gültigen Pathes erstellen
	std::list<MIntArray>::iterator iter = validPathes.begin();
	std::list<MIntArray>::iterator iterSE = startEndIDs.begin();

	l = validPathes.size();
	UINT l2;
	for(i = 0; i < l; i++)
	{
		l2 = (*iter).length();
		for(UINT x = 0; x < l2; x++)
		{

			creator.doInsertVtx((*iter)[x],true,(*iterSE)[x*2],(*iterSE)[x*2+1] );
	
		}

		iter++;
		iterSE++;
	}

	
	//fertig

}


//---------------------------------------------------------------------
void	IVfty::trenneEdgePathes(	list<MIntArray>& validPathes, 
									list<MIntArray>& startEndIDs, 
									MIntArray& invalidPathes)
//---------------------------------------------------------------------
{
//Iteratoren
	MItMeshPolygon	polyIter(fMesh);
	MItMeshEdge		edgeIter(fMesh);

//flags
	bool			isValidPath = true;	//neue Pfade sind solange gültig, bis das Gegenteil bewiesen wird
	bool			isTrue = true;		//hält eine interne WhileSchleife am laufen, sie wird durch break oder goto verlassen

//Speichervariablen
	int				tmp;				//um die iteratoren zu initialisieren auf index

	int				currentEdge;		//gegenwärtige Edge auf Path
	int				currentFace;		//gegenwärtiges, richtungsAnzeigendes Face
	int				lastFace;			//letztes Face in der Iteration - wird verwendet, um neues currentFace zu finden

	MIntArray		connectedFaces;		//temporärer Speicher für faces, die mit currentEdge verbunden sind
	MIntArray		containedEdges;		//temporärer Speicher für Edges, die mit currentFace verbunden sind
	MIntArray		faceVerts;			//hält die verts eines Faces, dient somit der bestimmung, ob edgeIDs conform zum Face laufen oder nicht
	MIntArray		match;				//temporärer Speicher für matchEdges

	
	MIntArray		startEdges;			//enthält die edgeIDs, von denen aus eine neue Pfadsuche gestartet werden soll
	MIntArray		startFaces;			//enthält eine FaceID pro startEdge als richtungsvorgabe, oder -1 wenn einfach alle richtungen genommen werden sollen


	MIntArray		faceDirection;		//hält die faces, die mit der startEdge verbunden sind bzw. das gewünschte lastFace
	MIntArray		partialPath;		//hält die edges des Pathes
	MIntArray		partialPathDirs;	//hält den startEndVtx zur entsprechenden edge X (X*2, X*2+1);
	
	partialPath.setSizeIncrement(UINT(edgeIDs.length() / 5));
	partialPathDirs.setSizeIncrement(UINT(edgeIDs.length() / 5));



//	MIntArray		theWholePath;		//enthält den ganzen Pfad von einer edge aus, von der aus alles begann
//	MIntArray		wholePathDirs;		//hält meshrelative VtxIDs, mit bereits richtig angeordneten direction-vertices, synchron zu theWholePath (x*2,x*2+1) == (startVtx, endVtx)

//sonstige Variablen
	BPT_BA			allEdges(edgeIDs,true,false,edgeIter.count());


	while(allEdges.getTrueCount() > 0)
	{
		//der äussere Loop sorgt dafür, dass garantiert jede gewählte Edge zugeordnet wird
		
		//Variablen zurücksetzen
		isValidPath = true;
		partialPath.clear();
		partialPathDirs.clear();

		//variablen initialisieren
		startEdges.append(allEdges.getFirstBitIndex());
		startFaces.append(-1);	//->suche in beide Richtungen


		for(unsigned int i = 0; i < startEdges.length(); i++)
		{

			if(!allEdges[ startEdges[i] ])
			{//wenn edge zu loop gehoerte, dann koennte sie bereits bearbeitet worden sein, und muss folglich nicht zweimal bearbeitet werden
				continue;
			}
			else
			{//ansonsten startEdge entfernen
				allEdges.setBitFalse(startEdges[i]);
			}

				
			//faceDirections herausfinden
			if(startFaces[i] == -1)
			{
				edgeIter.setIndex(startEdges[i],tmp);
				
				edgeIter.getConnectedFaces(faceDirection);

				//wenn startFace BorderEdge ist, dann sofot abbrechen (vorher aber in pathArrays eintragen
				if(faceDirection.length() == 1)
				{
					partialPath.append(startEdges[i]);

					if(isValidPath)
					{
						edgeIter.setIndex(startEdges[i], tmp);
						polyIter.setIndex(faceDirection[0],tmp);
						polyIter.getVertices(faceVerts);
						
						if(isDirConform(edgeIter.index(0),edgeIter.index(1), faceVerts))
						{//direction muss in diesem Fall conform sein
							partialPathDirs.append(edgeIter.index(0));
							partialPathDirs.append(edgeIter.index(1));
						}
						else
						{//ansonsten umkehren
							partialPathDirs.append(edgeIter.index(1));
							partialPathDirs.append(edgeIter.index(0));
						}
					}

					continue;
				}
			}
			else
			{
				faceDirection.clear();

				faceDirection.append(startFaces[i]);
			}

			//zuersteinmal die startEdge eintragen samt direction (invertiert)
			partialPath.append(startEdges[i]);
			
			edgeIter.setIndex(startEdges[i],tmp);
			polyIter.setIndex(faceDirection[0],tmp);
			polyIter.getVertices(faceVerts);

			if( isDirConform(edgeIter.index(0),edgeIter.index(1),faceVerts))
			{
				partialPathDirs.append(edgeIter.index(1));
				partialPathDirs.append(edgeIter.index(0));
			}
			else
			{
				partialPathDirs.append(edgeIter.index(0));
				partialPathDirs.append(edgeIter.index(1));
			}

			//jetzt Pfade in jeder (gültigen) richtung suchen
			for(unsigned int x = 0; x < faceDirection.length(); x++)
			{
				//Variablen für whileLoop initialisieren
				currentEdge = startEdges[i];
				lastFace = faceDirection[x];

				while(isTrue)
				{//gleich nächste gültige currentEdge finden
					
					edgeIter.setIndex(currentEdge, tmp);
					edgeIter.getConnectedFaces(connectedFaces);

					//currentFace finden
					if(connectedFaces.length() == 1)
					{//es ist borderEdge, also abbrechen
						break;
					}
					else
					{//ansonsten das currentFace holen, welches NICHT das lastFace ist
						currentFace = (connectedFaces[0] == lastFace) 
										? connectedFaces[1] 
										: connectedFaces[0];
						
					}

					//die beinhalteten edges des currentFace holen
					polyIter.setIndex(currentFace,tmp);

					polyIter.getEdges(containedEdges);

					//jetzt herausfinden, wie viele gewählte edges mit den faceEdges übereinstimmen
					//zwar ist auch die currentEdge in contained Edges vorhanden, allerdings wurde sie schon aus dem allEdges Array entfernt
					allEdges.findMatching(containedEdges, match);


					//wenn match groesser 1, dann wird der Pfad ungültig und es müssen keine directions mehr geholt werden
					if(match.length() == 1)
					{//es ist weiterhin gültiger pfad, also lastFace setzen, currentEdge und direction auf Arrays packen
					 //,currentEdge setzen und sie aus allEdges array entfernen
						
						lastFace = currentFace;
						currentEdge = match[0];
						allEdges.setBitFalse(currentEdge);

						partialPath.append(currentEdge);
						
						if(isValidPath)
						{//nur wenn der pfad gültig ist, wird überhaupt diretion benoetigt
							edgeIter.setIndex(currentEdge,tmp);
							polyIter.setIndex(currentFace,tmp);
							polyIter.getVertices(faceVerts);
							
					if( (x) ? !isDirConform(edgeIter.index(0),edgeIter.index(1),faceVerts)
							:  isDirConform(edgeIter.index(0),edgeIter.index(1),faceVerts)	)
							{
								partialPathDirs.append(edgeIter.index(1));
								partialPathDirs.append(edgeIter.index(0));
							}
							else
							{
								partialPathDirs.append(edgeIter.index(0));
								partialPathDirs.append(edgeIter.index(1));
							}
						}

					}
					else if(match.length() == 0)
					{//in dieser Richtung gibt es keine Edges mehr - abbruch
						break;
					}
					else
					{//also ist match groesser 1 - dieser macht alles folgende ungültig; die überschüssigen Edges sind neue StartEdges
					 //currentFace kommt in startFaces, pfad ist ungültig ,->abbruch
						for( UINT u = 0; u < match.length(); u++ )
						{
							startEdges.append(match[u]);
							startFaces.append(currentFace);
						}
						
						isValidPath = false;

						break;

					}


				}


			}//for(unsigned int x = 0; x < faceDirection.length(); x++)



		}//for(unsigned int i = 0; i < startEdges.length(); i++)

		
		

		if(isValidPath)
		{//nur wenn der Pfad gültig ist, muss auch der directionArray hinzugefügt werden
			//jetzt die gefunden Pfade zum ganzen Array hinzufügen
			validPathes.push_back(partialPath);
			startEndIDs.push_back(partialPathDirs);
		}
		else
		{//ansonsten isses ein ungültiger path, der also nicht sliden kann
			helper.addIntArrayToLHS(invalidPathes, partialPath);
		}
		

	}//while(allEdges.getTrueCount() > 0)

}

//--------------------------------------------------------------------------------------------
bool	IVfty::isDirConform(const int index0, 
							const int index1, 
							const MIntArray& faceVerts)
//--------------------------------------------------------------------------------------------
{
	UINT l = faceVerts.length();

	for(UINT i = 0; i < l; i++)
	{
		if(faceVerts[i] == index0)
			if(faceVerts[(i+1) % l] == index1)
			{
				INVIS(cout<<"Vertizen "<<index0<<" und "<<index1<<" sind faceConform."<<endl;);
				return true;
			}
			else
			{
				INVIS(cout<<"Vertizen "<<index0<<" und "<<index1<<" sind NICHT faceConform."<<endl;);
				return false;
			}
	}

	MPRINT("IS DIR CONFORM: FORBIDDEN PATH");
	return false;
}


//----------------------------------------------------
void		IVfty::doSlide(MObject& slideMesh)
//------------------------------------------------------
{

MStatus	stat;

	int minIndex, maxIndex;		//wird später verwendet für schleife (for)
	UINT x;						//für die iteration
	double r = 1.0;				//für iteration
	double	factor = 1.0 / (count + 1);	//factor für positionsberechnung

	MFnMesh	FnMesh(slideMesh,&stat);

	
	int l = slideIndices->length() - 1;	//MINUS 1, da der letzt eintrag von slideIndices ein dummy ist, welcher
										//die info über den letzten erzeugten vertex erhält, ohne dass dazu noch
										//noch irgendwelche vektoren gespeichert wären
	
	//dafür sorgen, dass scaling immer den korrekten wert hat
	if(maxStandardScale == 1.0)
	{//findet den groessten normalscalewert und setzt damit maxStandardScale	
		maxStandardScale = 0.0;

		for(int i = 0; i < l; i++)
		{
			if((*normalScale)[i] > maxStandardScale)
				maxStandardScale = (*normalScale)[i];
		}
	}

	if(!isSlideRelative)
	{
		//sichergehen, dass auch schon ScaleWerte erzeugt wurden, wenn er hier ankommt 
		if((*slideScale)[0] == -1)
			createAbsoluteVectors(*slideDirections, *slideScale);
	}

	//checken, welche seite gewünscht ist
	for(int i = 0; i < l; i++)
	{
		r = 1.0;

		if((*maySlide)[i])
		{
			if(isSlideRelative)
			{
				
				if(side)
				{
					minIndex = (*slideIndices)[i];
					maxIndex = (*slideIndices)[i+1];

					if(isNormalRelative)
					{//r wurd oben initialisiert (1.0)
						for(x = minIndex; x < maxIndex; x++)
							FnMesh.setPoint(x,(*slideStartPoints)[i] + (slide * (*slideDirections)[i] * factor * r++) + (normal * (*slideNormals)[i] * maxStandardScale ));
					}
					else
					{
						for(x = minIndex; x < maxIndex; x++)
							FnMesh.setPoint(x,(*slideStartPoints)[i] + (slide * factor * r++ * (*slideDirections)[i]) + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
					}
				}
				else
				{
					minIndex = (*slideIndices)[i] - 1;	//MINUS 1, damit ich als bedingung x *>* minIndex sagen kann
					maxIndex = (*slideIndices)[i+1] - 1;	//hier genauso
					
					if(isNormalRelative)
					{
						for(x = maxIndex; x > minIndex; x--)
							FnMesh.setPoint(x,(*slideEndPoints)[i] + -1 * slide * factor * r++ * (*slideDirections)[i] + (normal * (*slideNormals)[i] * maxStandardScale));
					}
					else
					{
						for(x = maxIndex; x > minIndex; x--)
							FnMesh.setPoint(x,(*slideEndPoints)[i] + -1 * slide * factor * r++ * (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
					}
				}
			}
			else
			{

				
				////cout<<"Bin nach CREATEABSOLUTE VECTORS"<<endl;
				if(side)
				{
					minIndex = (*slideIndices)[i];
					maxIndex = (*slideIndices)[i+1];

					if(isNormalRelative)
					{
						for(x = minIndex; x < maxIndex; x++)
							FnMesh.setPoint(x,(*slideStartPoints)[i] + (slide * factor * r++ * (*slideScale)[i])* (*slideDirections)[i] + ( normal  * (*slideNormals)[i] * maxStandardScale) );
					}
					else
					{
						for(x = minIndex; x < maxIndex; x++)
							FnMesh.setPoint( x,(*slideStartPoints)[i] + (slide * factor * r++ * (*slideScale)[i])* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
					}
				}
				else
				{
					minIndex = (*slideIndices)[i] - 1;	//MINUS 1, damit ich als bedingung x *>* minIndex sagen kann
					maxIndex = (*slideIndices)[i+1] - 1;	//hier genauso

					if(isNormalRelative)
					{
						for(x = maxIndex; x > minIndex; x--)
							FnMesh.setPoint(x,(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] * factor * r++ + ( normal * (*slideNormals)[i] * maxStandardScale) );
					}
					else
					{
						for(x = maxIndex; x > minIndex; x--)
							FnMesh.setPoint(x,(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] * factor * r++ + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
					}
				}	
			}
			
			
		}
		else
		{

			minIndex = (*slideIndices)[i];
			maxIndex = (*slideIndices)[i+1];

			if(isNormalRelative)
			{
				for(x = minIndex; x < maxIndex; x++)
					FnMesh.setPoint(x,(*slideStartPoints)[i] + (*slideDirections)[i] * factor * r++ + (normal * (*slideNormals)[i] * maxStandardScale) );
			}
			else
			{
				for(x = minIndex; x < maxIndex; x++)
					FnMesh.setPoint(x,(*slideStartPoints)[i] + (*slideDirections)[i] * factor * r++ + (normal * (*normalScale)[i] *(*slideNormals)[i]) );
			}
		}
	}


	if(options[7])	//nur wenn keine Tweaks vorhanden sind dürfen die UVs verändert werden, weil es ansonsten zu ungewollten verzerrungen kommen kann
		doUVSlide(slideMesh);
	

	FnMesh.updateSurface();

}


//-----------------------------------------------------------------
void	IVfty::createAbsoluteVectors(MVectorArray& slideDirections,MDoubleArray& slideScale)
//-----------------------------------------------------------------
{
	//index mit dem kleinsten enthaltenen vector finden und einzelne Längen zwischenspeichern
	////cout<<"Bin in createAbsolute Vectors"<<endl;
	//ScaleFaktoren für die Normale werden bereits bei der Erstellung des Meshes erzeugt und sind vorhanden
	MDoubleArray		tmp;
	
	int  l = slideDirections.length();
	tmp.setLength(l);

	double x = 16666666;


	int i;
	for(i = 0;i < l;i++)
	{
		tmp[i] = (slideDirections[i].length());
		if( (*maySlide)[i]  && tmp[i] < x )
		{
			x = tmp[i];
		}

		
	
	}


	for(i = 0; i < l;i++)
	{
		slideScale[i] = x / tmp[i];
	}

	
}


//-----------------------------------------------------------------
void	IVfty::createAbsoluteVectors(MFloatVectorArray& slideDirections,MFloatArray& slideScale)
//-----------------------------------------------------------------
{//überladene Funktion - arbeitet mit float präzision

	//index mit dem kleinsten enthaltenen vector finden und einzelne Längen zwischenspeichern
	////cout<<"Bin in createAbsolute Vectors"<<endl;
	//ScaleFaktoren für die Normale werden bereits bei der Erstellung des Meshes erzeugt und sind vorhanden
	MFloatArray		tmp;
	
	int  l = slideDirections.length();
	float x = 16666666;

	tmp.setLength(l);




	x = 16666666;


	int i;
	for(i = 0;i < l;i++)
	{
		tmp[i] = (slideDirections[i].length());
		if( tmp[i] < x)
		{
			x = tmp[i];
		}

	}


	for(i = 0; i < l;i++)
	{
		slideScale[i] = x / tmp[i];
	}

	
}


//---------------------------
void	IVfty::doUVSlide(MObject& mesh)
//---------------------------
{

	MFnMesh	FnMesh(mesh);
	//unsigned int numUVSets = UVSetNames.length();
	unsigned int numUVSets = 1;		//da er e nur auf einem UVSet arbeiten kann ... 

	//lokaoe#le float kopie von slide, um ständiges konvertieren zu vermeiden;
	float fSlide = slide;

	//iteratoren
	std::list<MIntArray>::iterator iterSlideIDs = UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideEnd = UVSlideEnd->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();

	std::list<MFloatArray>::iterator iterScale = UVSlideScale->begin();

	//merke: IDs sind im doppelPack angeordnet: Xn*2 = minUVId , Xn*2+1 = maxID+1 (damit x < maxID funzt

	UINT minUVID, maxUVID;		//tmpVariablen
	UINT	c2n = 0, c2n1 = 1;	//counter für i*2 und i*2+n (verhindert, dass man das jedes mal ausrechnen muss
								//und wird ausschliesslich mit IDsArray verwendet
	
	double	tf;				//tmpValue für factor * r, damt das nicht zweimal berechnet werden muss
	double	factor = 1.0 / (count + 1);	//factor für positionsberechnung
	double r = 1.0;

	for(unsigned int i = 0; i < numUVSets; i++)
	{
		MFloatVectorArray&	start = *iterSlideStart;
		MFloatVectorArray&	end	  = *iterSlideEnd;
		MFloatVectorArray&	dirs = *iterSlideDirs;

		MIntArray&			IDs = *iterSlideIDs;

		MFloatArray& scale = *iterScale;

		c2n = 0;
		c2n1 = 1;

		//unsigned int l = IDs.length();	//da IDs im doppelpack sind, sollte man lieber andere array für länge nehmen :)
		unsigned int l = start.length();

		if(!isSlideRelative)
		{
			//erstmal den ScaleFactor errechnen, wenn noetig
			

			if(scale.length() == 0)
			{
				scale.setLength(l);
				createAbsoluteVectors(dirs,scale);
			}
		}


		


		if(isSlideRelative)
		{
			if(side)
				for(unsigned int x = 0; x < l;x++)
				{
					minUVID = IDs[c2n];
					maxUVID = IDs[c2n1];
					r = 1.0;

					for(UINT y = minUVID; y < maxUVID; y++)
					{
						tf = factor * r++;
						FnMesh.setUV(y,start[x].x + (fSlide * dirs[x].x * tf), start[x].y + (fSlide * dirs[x].y * tf));
					}

					c2n  +=  2;
					c2n1 +=  2;
				}
			else
				for(unsigned int x = 0; x < l;x++)
				{
					minUVID = IDs[c2n] - 1;	//minUVID ist niemals 0, weshalb man hier getrost UINT benutzen kann
					maxUVID = IDs[c2n1] - 1;
					r = 1.0;

					for(UINT y = maxUVID; y > minUVID; y--)
					{
						tf = factor * r++;
						FnMesh.setUV(y,end[x].x - (fSlide * dirs[x].x * tf), end[x].y - (fSlide * dirs[x].y * tf));
					}
					c2n  +=  2;
					c2n1 +=  2;
				}

		}
		else
		{
			if(side)
				for(unsigned int x = 0; x < l;x++)
				{
					minUVID = IDs[c2n];
					maxUVID = IDs[c2n1];
					r = 1.0;

					for(UINT y = minUVID; y < maxUVID; y++)
					{
						tf = factor * r++;
						FnMesh.setUV(y,start[x].x + (fSlide * scale[x] * dirs[x].x * tf), start[x].y + (fSlide * scale[x] * dirs[x].y * tf));
					}

					c2n  +=  2;
					c2n1 +=  2;
				}
			else
				for(unsigned int x = 0; x < l;x++)
				{
					minUVID = IDs[c2n] - 1;	//minUVID ist niemals 0, weshalb man hier getrost UINT benutzen kann
					maxUVID = IDs[c2n1] - 1;
					r = 1.0;

					for(UINT y = maxUVID; y > minUVID; y--)
					{
						tf = factor * r++;
						FnMesh.setUV(y,end[x].x - (fSlide * scale[x] * dirs[x].x * tf), end[x].y - (fSlide * scale[x] * dirs[x].y * tf));
					}
					c2n  +=  2;
					c2n1 +=  2;
				}
		}

		
		

		++iterSlideStart;
		++iterSlideEnd;
		++iterSlideDirs;
		++iterSlideIDs;
		++iterScale;
		
	}
}




//-------------------------------------------
void		IVfty::modifySelectionList()
//-------------------------------------------
{
	//modifiziert die endgültige selectionList dahingehend, dass sie nur die BPTNode angewählt lässt

	MSelectionList	newList;	MObject	modifierNode;
	MGlobal::getActiveSelectionList(newList);
	MItSelectionList	selIter(newList,MFn::kPluginDependNode);
	for(;!selIter.isDone();selIter.next())
	{
		//eigentlich dürfte nur eine auf der Liste sein, nämlich die letzte BPTNode
		
		selIter.getDependNode(modifierNode);
		break;
	}
	
	MSelectionList	nullList;	nullList.add(modifierNode);
	MGlobal::setActiveSelectionList(nullList);

}



