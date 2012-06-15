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

//
// Copyright (C) 2003 Sebastian Thiel
// 
// File: BPTftyAction.cpp
//
// Node Factory: BPTfty
//
// Author: Sebastian Thiel
//

#include "BPTfty.h"
#include "vertexFty.h"

// General Includes
//
#include <maya/MGlobal.h>


// Function Sets
//
#include <maya/MFnMesh.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MPoint.h>
#include <maya/MItMeshEdge.h>
#include <maya/MCommandResult.h>
#include <maya/MPlug.h>
#include <maya/MAnimMessage.h>

#include "simpleLinkedList.h"
#include "BPT_BA.h"
#include "edgeFaceData.h"
#include "nSelEdgeData.h"

// Iterators
//
#include <maya/MItMeshPolygon.h>



MStatus BPTfty::doIt()//int slideInt)

{
/*

	cout<<"!!Bin in FactoryAction!!!"<<endl;
	
//	jetzt mal checken, ob sämtliche attribute auch angekommen sind ...



	//cout <<"SlideWert: "<<slideInt<<endl;
	cout <<"actionMode: "<<actionMode <<endl;
	cout <<"isRelative "<<isRelative <<endl;

*/
	
	
	INVIS(cout	<<vertIDs.length() <<" == Zahl der Vertizen incl. Indizes" <<endl;)
	INVIS(cout	<<edgeIDs.length() <<" == Zahl der Edges incl. Indizes" <<endl;)
	INVIS(cout	<<polyIDs.length() <<" == Zahl der faces incl. Indizes" <<endl;)
	

	SPEED("TOPOLOGIEÄNDERNDE AKTION");

/*	

	if(actionMode == 0 || actionMode > 3 && actionMode < 7)
	{
	//ANIMCURVE SETUPPEN;
		
		MObjectArray objects;
		
		
		MFnDependencyNode depNodeFn(thisNode);
		
		
		if(MAnimUtil::findAnimation(depNodeFn.findPlug("csl"),objects))
		{
			animFn.setObject(objects[0]);
		}
		
		objects.clear();
		if(MAnimUtil::findAnimation(depNodeFn.findPlug("can"),objects))
		{
			animFn2.setObject(objects[0]);
		}
		

	}
	
*/
  

#ifdef SCHOOL
		cout<<endl;
		cout<<"---------------------------------------------------------------"<<endl;
		cout<<"This license of BPT is for educational purposes only, reselling"<<endl
			<<"and usage in comercial projects is prohibited!"<<endl;
		
		cout<<"This license is locally restricted to the campus of the this school and may not be used outside this area"<<endl;
		cout<<"If you have been sold this program, please report this to: "<<endl;
		cout<<"Sebastian.Thiel@byronimo.de"<<endl;
#endif


	bool isSC = true;	//flag für die Beveloperationen

	switch(actionMode)
	{
	//SmartSplit
	case 0:
		{
//			zuerst díe Vertizen bearbeiten
//			Regeln: vertizen mit nur einer edgeConnection zur gesamtauswahl gehören mit dazu, wenn sie mindestens eine connection zu 
//			nem vertex haben. Wenn diese ein face mit anderem Vertex teilen, kommen diese beiden auf die nonConnectableList, am besten zusammen mit dem Fae, was sie teilen.

//			Bei allen anderen werden die IDs des äusseren EdgeRings gefunden und mit der eventuell vorhandenen EdgeSelection gemerged, um dann dort
//			weiterverarbeitet zu werden

//			generell müssen faces und Edges, die mit den Faces/edges der Vertizen matchen, von ihren Listen entfernt werden
//			so dass ein OriginalFace nur von einer Selection (Vtx ODER Edges ODER Faces) bearbeitet wird.
		

//			Meine tollen pointer müssen doch zuerst kopiert werden ...
			

			//MeshControl erstellen, arbeitet direkt auf daten von Fty
			meshCreator*	creator;
			

			creator = new meshCreator(	fMesh,
										slide, 
										meshPath);

			
			ftyCreator = creator;

			if(vertIDs.length() != 0)
			{
				//erstmal die meshDaten verfügbar machen
				
				//die fty Instanz des creators initialisieren für vertexWorker
				
				/*
				MIntArray	included;	//enthält die Vtx, welche miteinander verbunden sind
				
				//beim entfernen der Überlappungen könnte man auch warten, bis die IncludedList feststeht, 
				//damit man im Fall der separierten Vertizes nich zuviel wegnimmt. Bei ihnen sollte man dann die Betroffenen
				//faces verwenden bzw. deren edges
				
				
			
				//finded included Vtx, added sie zu included, erstellt für alle nicht includeten bereits die Faces
				vertexWorker(vertIDs,included);
				
				*/

				vertexFty	vtxFty(&vertIDs, creator);

				vtxFty.doIt();

				
				//sich mit excludedConnectedEdges überschneidende auswahlen entfernen
				//vertIDs brauche ich jetzt ja eigentlich nicht mehr und kann deshalb darauf arbeiten
				MIntArray	connectedEdges;
				
				//helper.arrayRemove(vertIDs,helper.matchArray(vertIDs,included) );
				
			
				vertGetConnectedEdgeIndices(vertIDs,connectedEdges);
				
			
				//überschneidende Edges abwählen
				if(edgeIDs.length() != 0)
					helper.arrayRemove(edgeIDs, connectedEdges);
				
				
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//Allerdings ist dieser Algo etwas zu großzügig, soll nur die Faces entfernen, die von excludedVtx 
				//geteilt werden
			
				//überschneidende Faces abziehen
				if(polyIDs.length() != 0)
				{
					removeOverlappingFaces(polyIDs, connectedEdges);
				}
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				
				
				//splitAround - gibts nicht mehr
			
				//includedVtx in Kanten umformen, dann die äußersten kannten finden und diese dann zum
				//edgeIDs Array hinzufügen
				//vertGetConnectedEdgeIndices(included,connectedEdges);
				//helper.addIntArrayToLHS(edgeIDs,findOuterEdges(connectedEdges,included) );
				
				
		
				
			}
			
			
		//	face*			facePtrs[500];
			simpleLinkedList<face> facePtrs;
			MIntArray		faceIDs;
			faceIDs.setSizeIncrement( 50 );
			
			INVIS(cout<<"MAIN: "<<"EdgeIDs: "<<edgeIDs.length()<<endl;)
			INVIS(cout<<"MAIN: "<<"PolyIDs: "<<polyIDs.length()<<endl;)
			//nun den eigentlichen splitVorgang starten
			if( edgeIDs.length() != 0 || polyIDs.length() != 0)
			{
				//faceIDs werde hier bereits gepruned
				if(edgeIDs.length() != 0)
					getAllFaceIndices(edgeIDs,polyIDs);
				
			
				INVIS(cout<<"MAIN: "<<"PolyIDs ist jetzt: "<<polyIDs.length()<<endl;)
		// ist synchron zu facePtr und enthält reguläre IDs
				

				//Creator aller faces initialisiern
				face::creator = creator;
				
				//Nun das FaceNetwork aufbauen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				MIntArray		faceVerts,nFaceVerts;
				MIntArray		corners;
				MIntArray		faceMatch,vtxMatch,faceNachbarn;	//faceNachbarn enthält die ungeordneten IDs der Nachbarn
				MIntArray		nachbarFaces;			//ebthält die geordneten IDs der Nachbarn
				face*			realNachbarn[20];			//enthält pointer zu Nachbarn, syncron zu NachbarFaces
				face*			existingFacePtr;
				
				bool			alreadyDone = false;
				
				MIntArray		faceMatchVtxMatchCount;
				
				

				MIntArray		neighbors;
				int				realFaceCount = 0;					
				bool			faceMatchWasZero = false;
				
				MItMeshPolygon	polyIter(fMesh);
				MItMeshVertex	vertIter(fMesh);
				int				tmp,l,ID,lastID;

				BPT_BA			tmpFaceIDs(polyIter.count());

				int				faceCount = polyIDs.length();


				
				NEWSPEED("KONVERTIERUNG MESH SMARTSPLIT");

				//BPT_BA zum matchen der PolyIDs verwenden
				BPT_BA	polyIDBA(polyIDs);
				
				//while(polyIDs.length() != 0)
				while(polyIDBA.getTrueCount() != 0)
				{
					//neighbors.append(polyIDs[0]);
					neighbors.append(polyIDBA.getFirstBitIndex());
					
					while(neighbors.length() != 0)	
					{
						
					//	INVIS(cout<<"CREATE_NETWORK: "<<"Bearbeite ID: "<<neighbors[0]<<endl;)
						
						//vor jedem durchlauf realnachbarn auf 0 setzen
						//!!!!!!!!!!!Abhängig von NachbarGröße
						for(int h = 0; h < 20; h++)
							realNachbarn[h] = 0;
						

						existingFacePtr = 0;
						faceMatchWasZero = false;
						//check, ob Face schon erstellt wurde
						tmp = neighbors[0];
						l = faceIDs.length();
						
						
						for(int a = 0; a < l;a++)
						{
							if(tmp == faceIDs[a])
							{
								existingFacePtr = facePtrs[a];
								break;
							}
						}

					//	INVIS(cout<<"CREATE_NETWORK: "<<"FacePtr existed"<<existingFacePtr<<endl;)
						
						
						polyIter.setIndex(neighbors[0],tmp);
						polyIter.getVertices(faceVerts);
						
						polyIter.getConnectedFaces(faceNachbarn);
						

						faceMatch.clear();
						polyIDBA.findMatching(faceNachbarn,faceMatch);

						//jetzt erstmal CornerVerts erstellen.
						findCornerVtx(corners,faceVerts,vertIter,polyIter.onBoundary());
						
					//	INVIS(cout<<"CREATE_NETWORK: "<<"CornerVtx sind:"<<endl;)
					//	INVIS(helper.printArray(corners," = CORNER_VTX");)
						
						
						//wenn keine nachbarn gewählt, werden keine <faces> erstellt
						if(faceMatch.length() == 0)
						{
							faceMatch = faceNachbarn;
							faceMatchWasZero = true;
						}
						
					//	INVIS(cout<<"CREATE_NETWORK: "<<"Nachbarn NICHT gefunden: "<<faceMatchWasZero<<endl;)
						
						lastID = 0;
						
					//	INVIS(helper.printArray(faceMatch," = FACE_MATCH");)

						//Array speichet die zahl der matchVtx, um sie später wiederzuverwenden
						faceMatchVtxMatchCount.clear();
						//wenn Match, dann nachbarn Ordnen
						int matchL = faceMatch.length();
						for(int x = 0; x < matchL;x++)
						{
							
							polyIter.setIndex(faceMatch[x],tmp);
							polyIter.getVertices(nFaceVerts);
							

							vtxMatch = helper.matchArray(nFaceVerts,faceVerts);
							
							faceMatchVtxMatchCount.append(vtxMatch.length());
							
							if(vtxMatch.length() > 2)
								cycleVtxMatch(vtxMatch,faceVerts);
	
							//INVIS(helper.printArray(vtxMatch," = MATCH_VTX");)
							//!!!!!!!!!!!!HIER WERTE VERÄNDERN,WENN MAXFACES ERHÖHT WERDEN


							if( tmpFaceIDs.isFlagSet(faceMatch[x]) )
							{
								alreadyDone = true;
							}
										
							
							if(!faceMatchWasZero && !alreadyDone)
							{
								neighbors.append(faceMatch[x]);
								facePtrs.append(new face(nFaceVerts,faceMatch[x]) );
								faceIDs.append(faceMatch[x]);
								tmpFaceIDs.setBitTrue(faceMatch[x]);
							}
							
							
							
							ID = findID(vtxMatch,faceVerts);
		
							//INVIS(cout<<"GEFUNDENE ID = "<<ID<<endl;)
							if(nachbarFaces.length() < ID + 1)
							{	
								tmp = nachbarFaces.length();
								nachbarFaces.setLength(ID + 1);
								//nicht initialisierte Slots mit  -1 versehen
								for(unsigned int z = tmp;z < nachbarFaces.length();z++)
								{								
										nachbarFaces[z] = -1;
								}
								//lastID = ID+1;
								nachbarFaces[ID] = faceMatch[x];

								//Nun noch in Positiver Richtung -1 eintragen
								if(x == matchL - 1 )
								{
									appendNachbarIDFlags(faceVerts,polyIter,vtxMatch,nachbarFaces);
								}

							}
							else
							{
								nachbarFaces[ID] = faceMatch[x];
								//lastID = ID+1;

								if(x == matchL - 1 )	//&& matchL != 1 : damit bei size1Matches Flag nicht doppelt angehängt wird
								{
									appendNachbarIDFlags(faceVerts,polyIter,vtxMatch,nachbarFaces);
								}

							}
							alreadyDone = false;
							
						}//for(int x = 0; x < matchL;x++)ENDE



						//INVIS(helper.printArray(nachbarFaces," = Nachbar_ARRAY VOR insertInner");)
						//zuerst Flags ins innere des Arrays einfügen
						insertNachbarInnerFlags(faceMatch,faceMatchVtxMatchCount,nachbarFaces);
						
						//Nun offsets in Nachbarfaces einfügen
						insertNachbarOffsets(nachbarFaces);
						
						//INVIS(cout<<"CREATE_NETWORK: "<<"ID-Array nach insertOffsets:"<<endl;)
						//INVIS(helper.printArray(nachbarFaces," = Nachbar_ARRAY");)
						
						//INVIS(helper.printArray(faceVerts," = FACE_VERTS");)
						
						//!!!!!!!!!!!!!!HIER ZAHL VERÄNDERN,WENN MAX NACHBAR COUNT ERHÖHT WIRD
						//Nun bachbarArray erstellen, wenn NachbarGröße über 10, abbruch
						
						for(unsigned int z = 0; z < nachbarFaces.length();z++)
						{
							if(nachbarFaces.length() < (20 + 2))	//+2 wegen flag am ende und weil sie ja 20 groß seín dürfen
							{
								if(nachbarFaces[z] == -1 || faceMatchWasZero)
								{
									realNachbarn[z] = 0;
								}
								else
								{
									//FaceID im ftyFaceArray finden und ptr in realnachbarn kopieren
									for(unsigned int o = 0;o < faceIDs.length();o++)
									{
										if(nachbarFaces[z] == faceIDs[o] )
										{
											realNachbarn[z] = facePtrs[o];
											break;
										}
									}
								}
							}
							else
							{
								break;
							}
						}
						
						
						//jetzt das neues Face erstellen
						if(existingFacePtr == 0)
						{

								face* tmpPtr = new face(faceVerts,corners,nachbarFaces,neighbors[0]);

								facePtrs.append(tmpPtr);
								
								
								for(int i = 0;i < 20;i++)
									tmpPtr->setNachbarn(realNachbarn[i],i);
								
								faceIDs.append(neighbors[0]);
								//Nur beim behandelten Face kommt es auch auf die tmpIDs list
								tmpFaceIDs.setBitTrue(neighbors[0]);
								neighbors.remove(0);
								
						}
						else
						{
							tmpFaceIDs.setBitTrue(neighbors[0]);
							existingFacePtr->setNachbarIDs(nachbarFaces);
							existingFacePtr->setCorners(corners);
							
							for(int i = 0;i < 20;i++)
								existingFacePtr->setNachbarn(realNachbarn[i],i);	
							
							neighbors.remove(0);
						}

						nachbarFaces.clear();			
						corners.clear();
						
						//realNachbarn drucken
						//cout<<"ALLE ADRESSEN AUSDRUCKEN VON NACHBARN"<<endl;
						for(int k = 0; k < 20;k++)
						{
							//cout<<realNachbarn[k]<<endl;
							realNachbarn[k] = 0;
						}

					
					//	  INVIS(cout<<"NEIGHBORS"<<endl;)
						//INVIS(helper.printArray(neighbors," = NEIGHBORS");)
						
					}//while(neighbors.length() != 0)	


					//hier die tmpFaceIDs, welche bearbeitet wurden, von polyIDs abziehen, damit alle FaceCluster bearbeitet werden
					
					

					INVIS(unsigned int p = tmpFaceIDs.getTrueCount(););
					INVIS(unsigned int h = polyIDBA.getTrueCount(););

					polyIDBA = polyIDBA - tmpFaceIDs;
					//nach dieser BA operations muss die synchronizität für trueCount wiederhergestellt werden
					polyIDBA.syncNumTrue();

					tmpFaceIDs.setAllFalse();

					INVIS(p = polyIDBA.getTrueCount();)

					
				//	INVIS(cout<<"CREATE_NETWORK: "<<"Neue polyIDs:"<<endl;)

					
				}
				

				DELETESPEED;

			}//while(polyIDs.length() != 0) ENDE
			
			
	

//			INVIS(helper.printArray(faceIDs," = FACEIDs");)
			
			//jetzt erstmal die Faces Ordnen nach ControlType
			simpleLinkedList<face>	controlFaces;	//WARNUNG - MAXIMAL 200 ControlFaces und kein FencePost Check
			int		tmp = 0;
			
			uint i;
			for(i = 0; i < faceIDs.length();i++)
			{	

				tmp = facePtrs[i]->getType();
				

				if(tmp == 5 || tmp == 1 )	//5 ist controlTwo
				{
					controlFaces.append(facePtrs[i]);
				}
			}

			//nachdem alle Faces ne typisierung haben, sollten Typ3, die mit der aloneEdge an Typ3 grenzen,
			//sich verhalten wie Typ2 normal, sie werden zu diesem Typ umgewandelt
			MIntArray	convertQueue,cornerBoundsQueue;
			int startCornerLocID,endCornerLocID;
			for(i = 0; i < faceIDs.length();i++)
			{	
				if(facePtrs[i]->whichType() == 3)
				{
					if(facePtrs[i]->modifyType3(startCornerLocID,endCornerLocID))
					{
						convertQueue.append(i);
						cornerBoundsQueue.append(startCornerLocID);
						cornerBoundsQueue.append(endCornerLocID);
					}
				}
			}

			//jetzt noch die gefundenen gültigen Type3 faces zu type two konvertieren
			for(i = 0; i < convertQueue.length();i++)
			{
				facePtrs[convertQueue[i]]->convertType3ToType2(cornerBoundsQueue[i*2],cornerBoundsQueue[i*2+1]);
			}


			//erst controlTypes engangen auf dass sich die directions verbreiten, und dann ...
			
			for( i = 0; i < controlFaces.length();i++)
			{
				controlFaces[i]->engage();
			}
			
			//alle anderen engagen, wenn sie noch nicht clean sind
			for( i = 0; i < faceIDs.length();i++)
			{	
				if( !(facePtrs[i]->isClean()) )
					facePtrs[i]->engage();
			}
			

			
			
			//jetzt noch slideDaten holen
	
			//zum schluss Slidedaten holen (auf jeden Fall, damit Ptr auf etwas verweisen
			creator->getSlideArrays(normalScale,
									slideScale,
									slideDirections,
									slideNormals,
									slideStartPoints,
									slideEndPoints,
									slideIndices, 
									maySlide
													);
			

			


				creator->getUVSlideArrays(	UVSlideStart,
											UVSlideEnd,
											UVSlideIndices,
											UVSlideDirections,
											UVSlideScale);
			
			


			INVIS(cout<<"MAIN: "<<"Erstelle Mesh"<<endl;)
			MStatus stat;
			creator->createMesh(newMeshData);
			
			INVIS(if(stat == MS::kFailure))
				INVIS(cout<<"!!!!!!Erzeugtes Mesh fehlerhaft!!!!!!!"<<endl;);


			//zuguterletzt selectionList zurücksetzen, aber die ByronsPolyToolsNode drauflassen
			modifySelectionList();

			switchComponentModeIfNeeded(true);
			

			//meshCreator löschen
			delete creator;
			ftyCreator = 0;


			break;
			
			
	}
	
		//CHAMFER
	case 4:
		{//es wird einfach nur ein Flag umgesetzt, der standardmäßig auf true ist (->SC als standard)
			isSC = false;	//einfach weitermachen
		}

		//SOLIDCHAMFER
	case 5:
		{ 
			INVIS(if(isSC) cout<<"WILL SOLID CHAMFERN"<<endl; else cout<<"WILL CHAMFERN"<<endl;)
		
		//alle edges zusammensammeln
		MIntArray	allEdges;
		
		getAllEdgeIndices(allEdges);



		MItMeshVertex	vertIter(fMesh);
		
		
		MIntArray	edgeIDs;	//IDs der erzeugten edges
		simpleLinkedList<edge> edgePtrs; //ptrs zu erzeugen edges

		//------------------------------------------------
		//erstmal Mesh extrahieren und Creator aufbauen
		//------------------------------------------------
		
		
		edgeMeshCreator* edgeCreator = new edgeMeshCreator(	fMesh,
															slide, 
															meshPath
																				);
		edge::creator = edgeCreator;
		
		//*******************************************************
		
		
		//Edges initialisieren
		initEdges(allEdges,edgeIDs,edgePtrs, !isSC);
		
		//für alle Fälle
		if(edge::MAINFaceDataPtrs.size() != 0)	
			edge::MAINFaceDataPtrs.clear();	

		if(edge::endFacePtrs.size() != 0)
			edge::endFacePtrs.clear();

		if(edge::nSelEdgeDataArray.size() != 0)
			edge::nSelEdgeDataArray.clear();



		int l = edgeIDs.length();
		MItMeshEdge edgeIter(fMesh);

		double smallest = 166666666.0;
		
		//es kann auch vorkommen, dass nur wingedEg gewählt waren. In diesem Fall alles rückgängig machen und fehlrmeldung ausgben
		if(l == 0)
		{
			MGlobal::displayError("Your Selection was invalid. Please make sure you select at least one valid (nonWinged) edge");
			return MS::kFailure;
		}

		int i;
		for(i = 0; i < l; i++)
		{
			
				double tmpLen; int tmp;

				edgeIter.setIndex(edgeIDs[i],tmp);
				edgeIter.getLength(tmpLen);
				
				if(tmpLen < smallest)
					smallest = tmpLen;

			//	lengthArray.append(tmpLen);
			
		}


		edge::maxLength = smallest ;	
		
		edgeFaceData::numUVSets = edgeCreator->numUVSets;

		INVIS(cout<<"MAXIMALE VTX-ENTFERNUNG: "<<smallest<<endl;)



		//Chamfer beginnen, erst die SelectedEdges - clean sind sie an dieser Stelle alle
		for(i = 0; i < l;i++)
		{
			if( edgePtrs[i]->isSelected() )
				edgePtrs[i]->engageBevel(options[0],options[1], isSC);
		}

		
		if(!isSC)
		{
			//um ganz einfach zu bestimmen, ob das Face an einem Vtx schon erstellt wurde, verwende ich BA auf der edge, welches eh bereits mit allen
			//origVtx initialisiert wurde 
			//es jetzt zur�cksetzen
			edge::origVtxDoneBA.setAllFalse();
			
			//jetzt alle selEdges, die clean sind, dazu bewegen, ihre smallFaces zu erstellen, wenn nötig
			for(i = 0; i < l; i++)
			{
				if(edgePtrs[i]->isSelected() && edgePtrs[i]->isClean())
				{
					
					(static_cast<selEdge*>(edgePtrs[i]))->createSmallChamferFaces();
				}
			}
			
		}

		//die Edges löschen
		for(i = 0; i < l;i++)
		{
			delete edgePtrs[i];
		}

		
		//nun durch die FaceData gehen und diese OriginalFaces wiederherstellen und 
		//dann alle Faces löschen
		
		l = edge::MAINFaceDataPtrs.size();
		std::list<edgeFaceData*>::iterator fIter = edge::MAINFaceDataPtrs.begin();

//		edgeFaceData* fTmp;
		for(i = 0; i < l; i++)
		{
	//		fTmp = *fIter;
			edgeCreator->recreateOriginalFace(*fIter);

//			delete fTmp;
			fIter++;
		}
		

		//jetzt die nSelEdges wiederherstellen und löschen, genauso wie ihre edgePtr
		l = edge::nSelEdgeDataArray.size();
		std::list<nSelEdgeData*>::iterator nIter = edge::nSelEdgeDataArray.begin();

		//BA mit anzahl der Faces erstellen - wird an changeFace übrgeben als LUT

		UINT maxRec;	//gibt die maximale rrecusionstiefe an.	

		nSelEdgeData* dTmp;
		nSelEdge* edgeTmp;
		for(i = 0; i < l; i++)
		{
			dTmp = *nIter;
			
			edgeTmp = static_cast<nSelEdge*>(dTmp->nSelEdge);
			
			maxRec = 0;
			edgeTmp->getMaxRecurseCount(&maxRec, edgeTmp);
			
			if(maxRec > 1)
			--maxRec;	//die letzte edge soll nicht evaluieren, da sie nochmal von hier aus aufgerufen wird

			edgeTmp->changeFace(dTmp->origID, dTmp->newID, dTmp->faceData, maxRec );

			
			delete dTmp;
			delete edgeTmp;

			nIter++;
		}



		edge::nSelEdgeDataArray.clear();
	
		//jetzt die endFaces erstellen
		l = edge::endFacePtrs.size();
		std::list<endFaceData*>::iterator eIter = edge::endFacePtrs.begin();
		endFaceData* eTmp;	//tmpVariable

		//hier benötogt man 2 durchgänge: Im ersten werden alle nonConnectEnds faces ausgeführt
		//Hier werden auch alle endFaces angepasst, das heißt, neue Vtx werden ihnen hinzugefügt
		for(i = 0; i < l; i++)
		{
			eTmp = *eIter;
			

			//jetzt das face erstellen
			if(eTmp->connectEnds[0] == -2)
			{//es ist also Quad

	//			edgeCreator->changeEndFace(eTmp->origVtx, eTmp->newVtx, eTmp->refVtx[0], eTmp->faceIDs, eTmp->faceData);
				edgeCreator->insertVtx(eTmp->faceData[0], eTmp->origVtx, eTmp->newVtx[0], eTmp->directions[0]);
				edgeCreator->insertVtx(eTmp->faceData[1], eTmp->origVtx, eTmp->newVtx[1], eTmp->directions[1]);
			}
			else
			{//also triangulieren


				if(eTmp->connectEnds[0] > -1)
				{

					
					edgeCreator->insertVtx(eTmp->faceData[0], eTmp->origVtx, eTmp->newVtx[0],eTmp->directions[0] );
				}

				if(eTmp->connectEnds[1] > -1)
				{


					edgeCreator->insertVtx(eTmp->faceData[1], eTmp->origVtx, eTmp->newVtx[1], eTmp->directions[1]);
				}

			}

			eIter++;
		//	delete eTmp;
		}


		//jetzt im 2. durchlauf die eigentlichen prozeduren aufrufen, welche originalVtx wieder entfernen (wenn nötig
		eIter = edge::endFacePtrs.begin();
		for(i = 0; i < l; i++)
		{
			eTmp = *eIter;
			

			//jetzt das face erstellen
			if( eTmp->connectEnds[0] == -2)	//-> Die letzte Klausel setzt connectEnds auf Werte, die es erlauben, zu triangulieren
			{//es ist also Quad
				edgeCreator->updateFaceData(eTmp->faceData[0]);
				edgeCreator->updateFaceData(eTmp->faceData[1]);
				edgeCreator->changeEndFace(eTmp->origVtx, eTmp->newVtx, eTmp->directions, eTmp->faceData);

			}
			else
			{//also triangulieren, aber nur, wenn connectEnds an ist (die andere Variante wurde bereits erledigt
				if(eTmp->connectEnds[0] == 1)
				{
					edgeCreator->updateFaceData(eTmp->faceData[0]);
					edgeCreator->changeEndFace(	eTmp->origVtx,
												eTmp->newVtx[0],
												eTmp->directions[0],
												eTmp->faceData[0]		);
					
					
				}

				if(eTmp->connectEnds[1] == 1)
				{
					edgeCreator->updateFaceData(eTmp->faceData[1]);
					edgeCreator->changeEndFace(	eTmp->origVtx,
												eTmp->newVtx[1],
												eTmp->directions[1],
												eTmp->faceData[1]		);		
					
				}

			}

			eIter++;
			delete eTmp;

		}

		
		l = edge::MAINFaceDataPtrs.size();
		fIter = edge::MAINFaceDataPtrs.begin();
		for(i = 0; i < l; i++)
		{
			delete *fIter;
			fIter++;
		}
		//Speicher für die Faces freimachen
		edge::MAINFaceDataPtrs.clear();

		//speicher für endFaces freimachen
		edge::endFacePtrs.clear();




		//letztlich noch die originalFaces wiederherstellen
	//	edgeCreator->recreateOriginalFaces();

	//	edge::originalFaceIDs.clear();
	//	INVIS(helper.printArray(edge::faceVtxChange," = faceVtxChange");)
	//	edge::faceVtxChange.clear();
		

		if(edgeIDs.length() != 0)
		{
			//zum schluss Slidedaten holen und mesh erstellen
			edgeCreator->getSlideArrays(normalScale,
										slideScale,
										slideDirections,
										slideNormals,
										slideStartPoints,
										slideEndPoints,
										slideIndices, 
										maySlide
										);

			//UVDatenArrays holen	

				edgeCreator->getUVSlideArrays(	UVSlideStart,
											UVSlideEnd,
											UVSlideIndices,
											UVSlideDirections,
											UVSlideScale);
			
			
			
		}

		
		//zum schluss Mesh erzeugen
		MStatus stat;
		edgeCreator->createMesh(newMeshData);
		
		INVIS(if(stat == MS::kFailure))
		INVIS(	cout<<"!!!!!!Erzeugtes Mesh fehlerhaft!!!!!!!"<<endl;)
		
		
		//zuguterletzt selectionList zurücksetzen, aber die ByronsPolyToolsNode drauflassen
		modifySelectionList();
		
		//alle statischen vaiablen säubern

		edge::origVtxDoneBA.setLength(0);

//		edge::vtxChangedBA.setLength(0);



//		edge::refVtxLOT.clear();
		
		switchComponentModeIfNeeded(true);


		delete edgeCreator;
		
		break;
		}
	case 6:
		{	
			#include "BPT_SMC.cpp.inc"

				break;
			}

	}


	
	return MS::kSuccess;
}
