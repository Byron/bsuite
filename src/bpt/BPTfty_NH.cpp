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

#include "BPTfty_NH.h"

//test
#include "BPT_BA.h"





BPTfty_NH::BPTfty_NH(void)
{
}

BPTfty_NH::~BPTfty_NH(void)
{
}

//------------------------
//METHODEN
//------------------------

MStatus	BPTfty_NH::doIt()
{

	SPEED(" MACHE NON- TOPO - CHANGE AKTION ");	

	switch(mode)
	{
	case 1:
		{
			//erstmal alle Edges holen
			MIntArray	allEdges;
			MIntArray	edgeLoops;		//enthält die fertige EL auswahl zum anwählen


			//	cout<<"MAIN: "<<"hole alle edgeIndices"<<endl;
			getAllEdgeIndices(allEdges);

			//	cout<<"MAIN: "<<"finde edgeLoops"<<endl;
			findEdgeLoops(allEdges,edgeLoops);


			//	cout<<"MAIN: "<<"Wähle Komponenten an"<<endl;
			selectComponents(edgeLoops,"edges");
			switchComponentModeIfNeeded();	

			break;

		}

		//edgeRing auswählen
	case 2:
		{
			MIntArray	allEdges;
			MIntArray	edgeRings;

			//	cout<<"MAIN: "<<"hole alle edgeIndices"<<endl;
			getAllEdgeIndices(allEdges);

			//	cout<<"MAIN: "<<"finde edgeRings"<<endl;
			findEdgeRings(allEdges,edgeRings);


			//	cout<<"MAIN: "<<"Wähle Komponenten an"<<endl;
			selectComponents(edgeRings,"edges");
			switchComponentModeIfNeeded();

			break;
		}
		//boundary erstellen
	case 3:
		{
			

			MStatus	status;
			MItMeshPolygon	polyIter(fMesh,&status);

			convertAllToFaces(polyIDs,vertIDs,edgeIDs);

			INVIS(if(status == MS::kFailure))
				INVIS(cout <<"FEHLER, fMesh ist nicht angekommen in fty"<<endl;)


			MIntArray		EdgeTmp;



			//invertSelection(polyIDs, polyIter.count(),inverted);

			faceGetContainedEdgeIndices(polyIDs, EdgeTmp);
			//faceGetContainedEdgeIndices(inverted, Edge2Tmp);


			MIntArray outline;
			helper.memoryPrune(EdgeTmp,outline);

			//outline finalisieren durch : allEdges - inner edges (in outline momentan)
			helper.memoryArrayRemove(EdgeTmp,outline);

			selectComponents(EdgeTmp,"edges");
			//selectComponents(memoryMatch(EdgeTmp,Edge2Tmp),"edges");

			switchComponentModeIfNeeded();

			break;

		}



	case 7:
		{
			MPRINT("WILL GROWEN")
			
			SPEED(" GROWING ");
			
			MItMeshVertex	vertIter(fMesh);
			MItMeshPolygon	polyIter(fMesh);
			MItMeshEdge		edgeIter(fMesh);

			if(vertIDs.length() != 0)
			{	
				growVtxSelection(vertIDs,vertIter,polyIter, true);
			}
			else if(edgeIDs.length() != 0)
			{
				growEdgeSelection(edgeIDs,edgeIter,0);
			}
			else if(polyIDs.length() != 0)
			{//jetzt muss faceIDs an der reihe sein, da ja eine auswahl vorhanden sein muss, wenn er in fty ist
				//->diese auswahl koennen aber auch UVs sein
				growFaceSelection(polyIDs,edgeIter,polyIter.count());
			}
			break;
		}
	case 8:
		{
			MPRINT("will shrinken")

			SPEED(" SHRINKING ");
				
			MItMeshVertex	vertIter(fMesh);
			MItMeshPolygon	polyIter(fMesh);
			MItMeshEdge		edgeIter(fMesh);
			MIntArray	allVtx;


			if(vertIDs.length() != 0)
			{	
				schrinkVtxSelection(vertIDs,vertIter,polyIter, true);
			}
			else if(edgeIDs.length() != 0)
			{
				shrinkEdgeSelection(edgeIDs,edgeIter,polyIter.count());
			}
			else if(polyIDs.length() != 0)
			{//jetzt muss faceIDs an der reihe sein, da ja eine auswahl vorhanden sein muss, wenn er in fty ist
				shrinkFaceSelection(polyIDs,edgeIter,polyIter.count());
			}


			break;
		}
	}

	return MS::kSuccess;

}


//---------------------------------------------------------------------
void	BPTfty_NH::externalBoundary(MIntArray& partEdges, MIntArray& polys ,MObject& mesh)
//---------------------------------------------------------------------
{

		MItMeshPolygon	polyIter(mesh);


		MIntArray		EdgeTmp;

		

		faceGetContainedEdgeIndices(polys, EdgeTmp, &mesh);

		
		helper.memoryPrune(EdgeTmp,partEdges);
		
		//outline finalisieren durch : allEdges - inner edges (in outline momentan)
		helper.arrayRemove(EdgeTmp,partEdges);
		

		partEdges = EdgeTmp;
		
}

//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************
//ALLGEMEINE PROCEDURES
//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************

//-----------------------------------------------------------------
void		BPTfty_NH::getAllEdgeIndices(MIntArray& allIndices)
//-----------------------------------------------------------------
{
//	erstmal edgeIDs hinzufügen, dann die Ids aller anderen Componenten 
//	holen
	MIntArray	tmp;

	allIndices = edgeIDs;

	vertGetConnectedEdgeIndices(vertIDs,tmp);
	helper.addIntArrayToLHS(allIndices,tmp);
	faceGetContainedEdgeIndices(polyIDs,tmp);
	helper.addIntArrayToLHS(allIndices,tmp);

	if(allIndices.length() != 0)
		helper.memoryPrune(allIndices);

}


//---------------------------------------------------------------------
void		BPTfty_NH::vertGetConnectedEdgeIndices(MIntArray& vertIndices,
												   MIntArray& edgeIndices)
//---------------------------------------------------------------------
{
	
	MIntArray	tmpArray;
	MItMeshVertex	iter(fMesh);

	edgeIndices.clear();

	int tmp;

	int l = vertIndices.length();
	

	for(int i = 0; i < l; i++)
	{
		iter.setIndex(vertIndices[i],tmp);
		iter.getConnectedEdges(tmpArray);
		helper.addIntArrayToLHS(edgeIndices, tmpArray);
	}
		

}


//---------------------------------------------------------------------
void		BPTfty_NH::faceGetConnectedEdgeIndices(MIntArray& faceIndices,
												   MIntArray& edgeIndices)
//---------------------------------------------------------------------
{
	
	MIntArray		tmpArray;
	MItMeshPolygon	iter(fMesh);
	
	edgeIndices.clear();

	int tmp;

	int l = faceIndices.length();

	for(int i = 0; i < l; i++)
	{
		iter.setIndex(faceIndices[i],tmp);
		iter.getConnectedEdges(tmpArray);
		helper.addIntArrayToLHS(edgeIndices, tmpArray);
	}


	
}

//--------------------------------------------------------------------------------------------------------------------
void		BPTfty_NH::selectComponents(MIntArray& edgeIndices, MString mode, MString selectMode)
//--------------------------------------------------------------------------------------------------------------------
{
	SPEED("SELECTING COMPONENTS");

	MDagPath		meshPath;
	
	MPRINT("Selecting")

	MDagPath::getAPathTo(fMesh,meshPath);
	MFn::Type type;
	////cout<<"Bin in `selectComponents`"<<endl;
//	MString	type;
	if(mode == "edges")
		type = MFn::kMeshEdgeComponent;
	else if(mode == "polys")
		type = MFn::kMeshPolygonComponent;
	else if(mode == "vertices")
		type = MFn::kMeshVertComponent;


	MFnSingleIndexedComponent compFn;
	MObject	comps = compFn.create(type);
	compFn.addElements(edgeIndices);

	redoSelection.add(meshPath,comps);

	if(selectMode == "replace")
		MGlobal::setActiveSelectionList(redoSelection);
	else if(selectMode == "add")
	{
		MGlobal::setActiveSelectionList(redoSelection,MGlobal::kAddToList);
		MGlobal::getActiveSelectionList(redoSelection);
	}
	else if(selectMode == "remove")
	{
		MGlobal::setActiveSelectionList(redoSelection,MGlobal::kRemoveFromList);
		MGlobal::getActiveSelectionList(redoSelection);
	}
	else if(selectMode == "toggle")
	{
		MGlobal::setActiveSelectionList(redoSelection,MGlobal::kXORWithList);
		MGlobal::getActiveSelectionList(redoSelection);
	}

	MPRINT("Done")
}


//--------------------------------------------------------------------------------------------
void		BPTfty_NH::switchComponentModeIfNeeded(bool mode,MSelectionMask::SelectionType type)
//--------------------------------------------------------------------------------------------
{
	
	if(options[5])
	{
		
		
		MSelectionMask mask(type);
		
		
		MGlobal:: setComponentSelectionMask(mask);
	}
	

}


//--------------------------------------------------------------------------------------------
void	BPTfty_NH::convertAllToFaces(MIntArray& polyIDs,
									 MIntArray& vertIDs,
									 MIntArray& edgeIDs)
//--------------------------------------------------------------------------------------------
{
	SPEED("CONVERT ALL TO FACES");

	MItMeshVertex	vertIter(fMesh);
	MIntArray temp;
	int tmp,l;
	
	l = vertIDs.length();
	int i;
	for(i = 0; i < l;i++)
	{
		vertIter.setIndex(vertIDs[i],tmp);
		vertIter.getConnectedFaces(temp);
		helper.addIntArrayToLHS(polyIDs,temp);
	}

	MItMeshEdge	edgeIter(fMesh);
	
	l = edgeIDs.length();
	for(i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeIDs[i],tmp);
		edgeIter.getConnectedFaces(temp);
		helper.addIntArrayToLHS(polyIDs,temp);
	}

	helper.memoryPrune(polyIDs);

}

//---------------------------------------------------------------------
void	BPTfty_NH::faceGetContainedEdgeIndices(MIntArray& faceIndices, MIntArray& edgeIndices, MObject* mesh)
//---------------------------------------------------------------------
{
	SPEED("GET CONTAINED EDGES");

	MIntArray		tmpArray;

	MObject thisMesh;
	if(mesh == 0)
		thisMesh = fMesh;
	else
		thisMesh = *mesh;

	MItMeshPolygon	iter(thisMesh);

	edgeIndices.clear();

	int tmp;

	int l = faceIndices.length();

	for(int i = 0; i < l; i++)
	{
		iter.setIndex(faceIndices[i],tmp);
		iter.getEdges(tmpArray);
		helper.addIntArrayToLHS(edgeIndices, tmpArray);
	}

}



//---------------------------------------------------------------------
void	BPTfty_NH::invertSelection(MIntArray& lhs,int maxElement, MIntArray& inverted)
//---------------------------------------------------------------------
{

	//invertSelection auf basis von SelectionLists
	MSelectionList list;
	MFnSingleIndexedComponent	compAll,compPart;

	compAll.create(MFn::kMeshPolygonComponent);
	compAll.setCompleteData(maxElement);

	//jetzt die gewählten elemente eintragen
	compPart.create(MFn::kMeshPolygonComponent);
	compPart.addElements(lhs);

	//liste bearbeiten
	list.add(meshPath, compAll.object());
	list.toggle(meshPath, compPart.object());

	MObject	components;
	MDagPath	path;

	list.getDagPath(0,path, components);

	compPart.setObject(components);
	compPart.getElements(inverted);


}

//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************
//EDGE LOOP PROCEDURES
//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------------------
void	BPTfty_NH::findEdgeLoops(MIntArray& allEdges, MIntArray& edgeLoops)
//--------------------------------------------------------------------------------------------------------------------
{
	MItMeshEdge		edgeIter(fMesh);
	MItMeshVertex	vertIter(fMesh);			//PS: alle hilfsprocs erhalten referenzen des iterators
	MItMeshPolygon	polyIter(fMesh);

	int				maxEdgeCount = options[2];	//wenn dieser Wert 0, dann unendlich edges findbar
	int				edgeCount	;				//counter für jeden edgeLoop, wird inkrementiert

	MIntArray		removed;					//enthält entfernte Kanten, wird später wieder auf die Selection getan (doppelte Kanten, kanten mit connection zu nEck)
	MIntArray		triEdges;					//hält gewählte Edges, die auf triangles liegen

	int				LOffset = 0;
	unsigned int	l = allEdges.length(); 
	int				tmp;

	MIntArray		connectedFaces;
	
	//int				whichType = 0;				//0 = nicht bearbeitet; 1 = ValidEdge(auf quad oder auf nEck); 2 = triEdge;
	int				triangleCount = 0;				//zahl Tris, wenn =2, dann vermutlich triEdge, wenn 1, eventuell auch, ansonsten nicht


	INVIS(cout<<"FIND_EDGE_LOOPS: "<<"Preprocessing"<<endl;)
	//preProcess: KantenTyp herausfinden und sie entsprechnenden containern zuordnen


	UINT i;
	for(i = 0; i < l; i++)
	{
		
		
		edgeIter.setIndex(allEdges[i],tmp);
		edgeIter.getConnectedFaces(connectedFaces);

		for(int x = 0; x < 2; x++)
		{
			polyIter.setIndex(connectedFaces[x],tmp);
			
			if(polyIter.polygonVertexCount() == 3)
				triangleCount++;
		}
		
		if(triangleCount == 2)
		{
			
			//es ist valid triangle, also in array tun
			triEdges.append(allEdges[i]);
			
						
		}
		
	}

	//triangles von allEdges abziehen, kopie übergeben, da proc mit 2 referenzen arbeitet
	//MIntArray	arri = triEdges;

	helper.arrayRemove(allEdges,triEdges,true);

	INVIS(cout<<"NUM VALID TRIANGLES: "<<triEdges.length()<<endl;)
	INVIS(helper.printArray(triEdges," = TRI_EDGES");)

	INVIS(cout<<"NUM ALL EDGES(Rest): "<<allEdges.length()<<endl;)
	INVIS(helper.printArray(allEdges," = RESTL. EDGES");)


	//jetzt die BorderEdges filtern
	MIntArray borderEdges;
	l = allEdges.length();

	for(i = 0; i < l; i++)
	{
		edgeIter.setIndex(allEdges[i],tmp);

		if(edgeIter.onBoundary())
			borderEdges.append(allEdges[i]);
	}

	//arri = borderEdges;
	helper.arrayRemove(allEdges,borderEdges,true);

	//jetzt die border suchen mit stopEdge functionality
	l = borderEdges.length();
	for(i = 0; i < l; i++)
	{
		if(borderEdges[i] != -1)
		{
			findBorderEdges(borderEdges[i],edgeIter, vertIter, borderEdges, edgeLoops);
		}
	}


	bool			go = true;
	
	//nun zuerst triangleArray bearbeiten
	
	
	edgeCount = 0;					//wird inkrementiert, startet mit 1 für erste kante


	MIntArray		edgeVtx(2,0);
	MIntArray		vtxEdges;		//speichert die mit vtx verbundenen EdgeIDs
	MIntArray		tmpLoop,tmpLoop2;		//speichert loopdaten zwischen, solange sie noch nicht zwangsweise korrekt sind
	MIntArray		stopEdgeLUT;

	MIntArray		match;			//speichert die kanten vom gefunden Loop, die bereits in triEdges enthalten sind
	

	//*****************
	//*****QUADS*******
	//*****************

	INVIS(cout<<"FIND_EDGE_LOOPS: "<<"STARTE QUAD_LOOP ROUTINE"<<endl;)
	int quadEdgeCount = 0;		//quads bekommen eigenen counter, haben gleiches edgeMax

	//Nun quads bearbeiten
	l = allEdges.length();
	for(i = 0; i < l; i++)
	{
		//Flag: wenn edge nicht schon ungültig
		if(allEdges[i] != -1)
		{
			
			edgeIter.setIndex(allEdges[i],tmp);

			allEdges[i] = -1;
			
			edgeVtx[0] = edgeIter.index(0);
			edgeVtx[1] = edgeIter.index(1);
			
			//jede Richtung bearbeiten
			for(int x = 0; x < 2; x++)
			{
				if(x == 0)
				{
				INVIS(cout<<"FIND_EDGE_LOOP: "<<"BEGIN"<<endl;)
				tmpLoop = findOneWayQuadLoop(edgeIter.index(),allEdges,quadEdgeCount,edgeVtx[x], stopEdgeLUT);
				
				//diese Kante gleich als bearbeitet markieren
				}
				else
				{
				INVIS(cout<<"FIND_EDGE_LOOP: "<<"BEGIN"<<endl;)
				tmpLoop2 = findOneWayQuadLoop(edgeIter.index(),allEdges,quadEdgeCount,edgeVtx[x], stopEdgeLUT);
				}
			}
			//letztes tmpLoopElement enthält flag, welche entfernt werden muss
			//-1 = loop, -2 = einmal stopEdge gefunden, -3 = in jeder richtung selbe stopEdge gefunden, was stopEdge auf Loop entspricht
			
			addToEdgeLoopArray(edgeLoops,tmpLoop,tmpLoop2);

			INVIS(helper.printArray(tmpLoop," = tmpLoop");)
			INVIS(helper.printArray(tmpLoop2," = tmpLoop2");)
			//damit es für die nächste Edge nicht unnoetig gross ist
			stopEdgeLUT.clear();
			
		}
	}

	//Die herausgefilterten TriEdges wieder auf selectionList packen bzw. in edgeLoop array

	helper.addIntArrayToLHS(edgeLoops, triEdges);

}

//---------------------------------------------------------------------------------------------------------------------------
void	BPTfty_NH::findBorderEdges(int startEdge,
								   MItMeshEdge& edgeIter,
								   MItMeshVertex& vertIter,
								   MIntArray& borderEdges,
								   MIntArray& edgeLoops)
//---------------------------------------------------------------------------------------------------------------------------
{
	int currentEdge,tmp,lastVertex;
	
	MIntArray removals;		//hält Kandidaten (edges), die nachher vom borderEdges Array runter müssen
	MIntArray result[2];	//loop pro seite
	MIntArray ces;			//connectedEdges

	//initializations
	edgeIter.setIndex(startEdge,tmp);
	
	bool foundStopEdge = false; //damit für den fall, dass die Pathes der Stopedges gleichlang sind, trotzdem der korrecte
	
	result[0].append(startEdge);
	result[1].append(startEdge);



	//pro Vertex path suchen
	UINT i;
	for(i = 0; i < 2; i++)
	{

		
		lastVertex = edgeIter.index(i);
		currentEdge = startEdge;
		

		MPRINT("--------------------");
		MPRINT("STARTE NEUE RICHTUNG");
		MPRINT("--------------------");

		while(true)
		{
			
			//connected edges holen und non-boundary edges rausschmeissen
			int validVertex;

			edgeIter.setIndex(currentEdge,tmp);

			if(edgeIter.index(0) == lastVertex)
				validVertex = edgeIter.index(1);
			else
				validVertex = edgeIter.index(0);


			INVIS(cout<<validVertex<<" = validVertex"<<endl;)


			vertIter.setIndex(validVertex,tmp);
			vertIter.getConnectedEdges(ces);


			uint x;
			for(x = 0; x < ces.length();x++)
			{
				edgeIter.setIndex(ces[x],tmp);
				
				if(!edgeIter.onBoundary())
				{
					ces.remove(x--);
				}
			}
			
			
			//currentEdge aussortieren
			for(x = 0; x < ces.length();x++)
			{

				if(ces[x] == currentEdge)
				{
					ces.remove(x);
					break;
				}

			}


			//wenn jetzt keine edges mehr übrig sind, dann abbrechen
			if(ces.length() == 0)
				break; 

			
			//eigentlich sollte nur eine Edge auf Array sein koennen
			INVIS(if(ces.length() > 1))
				INVIS(cout<<"WARNING: MEHR ALS EINE BOUNDARYEDGE AUF CES ARRAY!"<<endl;);

			int validEdge = ces[0];
			//jetzt die gefundene Edge auf abbruchbedingungen abchecken
			if(validEdge == startEdge)
				break;

			else if(helper.isOnArray(validEdge, borderEdges))
			{
				result[i].append(validEdge);

				//kodierungsregel: erster vertex ist negiert, anderer ist normal
				//um später zu checken, welche der unter umständen 2 stopEdges (1 je richtung)
				//nun wirklich entfernt werden soll. -1 damit auch 0 kodiert werden kann
				foundStopEdge = true;
				removals.append((i == 0)?-validEdge-1:validEdge);
				break;
			}
			else
			{
				//es ist eine ganz normale gültige boder edge, also auf resultArray packen und
				//currentEdge, lastVertex setzen
				result[i].append(validEdge);
				currentEdge = validEdge;
				lastVertex = validVertex;
			}
			
		}

	}


	MIntArray removeThis; //container für wirklich gültige edgeRemovals
	//results checken und entsprechend auf edgeLoops packen
	uint l = removals.length();
	

	if(result[0].length() < result[1].length())
	{
		helper.addIntArrayToLHS(edgeLoops, result[0]);
		
		//negative removals von borderEdges nehmen
		for(i = 0; i < l; i++)
		{
			if(removals[i] < 0)
			{
				removeThis.append(-(removals[i]+1));
			}
		}
	}
	else if(result[1].length() < result[0].length())
	{
		helper.addIntArrayToLHS(edgeLoops, result[1]);
		
		for(int i = 0; i < l; i++)
		{
			if(removals[i] > -1)
			{
				removeThis.append(removals[i]);
			}
		}
	}
	else	//es ist ein durchgängiger loop, also isses egal welches array rauf kommt
	{	//für den Fall das die results gleich lang sind trotz stopEdge
		if(foundStopEdge)
		{
			for(i = 0; i < l; i++)
			{
				if(removals[i] < 0)
				{
					removeThis.append(-(removals[i]+1));
				}
				else
				{
					removeThis.append(removals[i]);
				}
			}

		}

		helper.addIntArrayToLHS(edgeLoops, result[0]);
	}



	if(removeThis.length() != 0)
	{
		for(i = 0; i < borderEdges.length();i++)
		{
			for(unsigned int x = 0; x < removeThis.length(); x++)
				if(borderEdges[i] == removeThis[x])
				{		
					borderEdges[i] = -1;
					removeThis.remove(x);
				}
		}
	}
	

}

//--------------------------------------------------------------------------------------------------------------------
MIntArray		BPTfty_NH::findOneWayQuadLoop(int firstEdge,
											  MIntArray& allEdges,
											  int &edgeCount, 
											  int firstVertex, 
											  MIntArray& stopEdgeLUT)
//--------------------------------------------------------------------------------------------------------------------
{
	int				maxEdgeCount = options[2] + 1;

	MItMeshVertex	vertIter(fMesh);
	MItMeshEdge		edgeIter(fMesh);
	MItMeshPolygon	polyIter(fMesh);

	MIntArray		match;
	MIntArray		result;
	MIntArray		vtxEdges;
	MIntArray		currentFaces,faces;

	int				checkFrequency;

	if(allEdges.length() >= 5)
		checkFrequency = ( allEdges.length() - (allEdges.length() % 5) ) / 5;	//checkHäufigkeit abhängig von Groesse von auswahl
	else
		checkFrequency = 1;	//jeden durchlauf checken, wenn Auswahl kleiner 5

	////cout<<"ONE_WAY_QUAD:  "<<"CheckFrequenz = "<<checkFrequency<<endl;

	int				dirCount = 0;		//zählt, wieviele Edges in diese Richtung gefunden wuden und kann als timer für allEdgesMatches verwendet werden
	int				tmp,currentVtx,currentEdge;

	edgeIter.setIndex(firstEdge,tmp);
	currentEdge = edgeIter.index();
	currentVtx = firstVertex;
	
	stopEdgeLUT.setSizeIncrement( (allEdges.length() - (allEdges.length() % 2)) / 2 );
	
	for(;;)
	{
		
		vertIter.setIndex(currentVtx,tmp);
		
		edgeIter.getConnectedFaces(currentFaces);
		//gegenüberliegenden Vtx holen
		vertIter.getOppositeVertex(currentVtx,currentEdge);
		
		vertIter.setIndex(currentVtx,tmp);
		////cout<<"ONE_WAY_QUAD:  "<<"currentEdge = "<<currentEdge<<endl;
		////cout<<"ONE_WAY_QUAD:  "<<"currentVtx = "<<currentVtx<<endl;
		
		vertIter.getConnectedEdges(vtxEdges);
		
		result.append(currentEdge);
		edgeCount++;
		dirCount++;
		
		if(vtxEdges.length() < 4 || vtxEdges.length() >= 5)
		{
			////cout<<"ONE_WAY_QUAD:  "<<"ABBRUCH: zuviele Edges"<<endl;
			stopLoopSearch(result,allEdges, stopEdgeLUT,currentEdge);
			result.append(0);
			return result;
		}
		
	
		
		if(edgeCount < maxEdgeCount)
		{
			for(unsigned int x = 0; x < vtxEdges.length();x++)
			{
				if(vtxEdges[x] != currentEdge)
				{
					edgeIter.setIndex(vtxEdges[x],tmp);
					edgeIter.getConnectedFaces(faces);
					
					if( helper.matchArray(currentFaces,faces).length() == 0 )
					{
						currentEdge = edgeIter.index();
					//	vertIter.setIndex(currentVtx,tmp);
						break;
						
					}	
				}
			}
		
			//beim ersten durchlauf die eventuell vorhandene edge direkt neben der first edge beseitigen
			//so dass sie von STopSearch nicht mehr gefunden wird
			//-->Je oefter das durchgeführt wird, desto mehr edges koennen hintereinander liegen, ohne als stopEdge zu gelten
			if(dirCount == 1)
			{
				for(unsigned int i = 0; i < allEdges.length(); i++)
					if(allEdges[i] == currentEdge)
						allEdges[i] = -1;
			}

			//-----------------------
			//auf StopEdges Checken
			//-----------------------
			if( (dirCount % checkFrequency == 0) )
			{
				////cout<<"ONE_WAY_QUAD:  "<<"checke auf STOPEDGES"<<endl;
				//stopLoopSearch verändert stopEdges( = allEdges) entsprechend, damit diese nicht nochmal gesucht werden
				if(stopLoopSearch(result,allEdges, stopEdgeLUT,currentEdge))
					return result;
			 }
			
		
			if(currentEdge == firstEdge)
			{
				////cout<<"ONE_WAY_QUAD:  "<<"ABBRUCH: startEdge erreicht"<<endl;
				//wenn vor der startEdge nich noch ne stop Edge liegt, dann marker für startEdge einfühe
				if(!stopLoopSearch(result,allEdges, stopEdgeLUT,currentEdge))
					result.append(-1);

				return result;
			}
			
		
		}
		else
		{
			//vor jedem Abbruch checken, ob vielleicht edges aus all Edges auf dem loop liegen, so dass eventuell schon
			//schon vorher abgebrochen werden muss 
			////cout<<"ONE_WAY_QUAD:  "<<"EdgeCount erreicht"<<endl;
			stopLoopSearch(result,allEdges, stopEdgeLUT,currentEdge);
			return result;
		}
		
	}

}


//--------------------------------------------------------------------------------------------------------------------
bool		BPTfty_NH::stopLoopSearch(MIntArray& loopResult, MIntArray& stopEdges, MIntArray& stopEdgeLUT,int currentEdge)
//--------------------------------------------------------------------------------------------------------------------
{
	int			l; loopResult.length();
	int			d = 99999;		//entfernung einer gefundenen Edge im Array
	int			bestMatch;			//enthält den index vom nächstgelegenen match
	MIntArray	match;



		
		match = helper.matchArray(loopResult,stopEdges);



	//noch im stopEdgeLUT nachschlagen, ob bei diesem EdgeLoop dieselbe kante schonmal aufgetreten ist

	l = stopEdgeLUT.length();
	////cout<<"STOP_EDGE_SEARCH: "<<"Groesse LUT:  "<<l<<endl;

	if(l != 0)
	{
		for(int  a = 0; a < l; a++ )
			if( currentEdge == stopEdgeLUT[a] )
			{
				////cout<<"ABBRUCH durch STOP_EDGE_SEARCH-LUT"<<endl;
				loopResult.append(currentEdge);
				loopResult.append(-3);
				return true;	
			}
	}
			

	if(match.length() == 0)
		return false;
	else
	{
		l = loopResult.length();
		////cout<<"STOP_EDGE_SEARCH: "<<"Groesse LOOP_RESULT:  "<<l<<endl;
		//kleinsten index von match im loopresult finden
		UINT i;
		for(i = 0; i < match.length(); i++)
			for(int a = 0; a < l; a++ )
				if( (match[i] == loopResult[a]) && (a < d))
				{
					d = a;
					bestMatch = i;
				}
		
		//loopResultArray verkleinern
		loopResult.setLength(d + 1);

		//jetzt stopEdges nach match absuchen und sie entfernen mit -1
		
		l = stopEdges.length();
		
		for(i = 0; i < match.length(); i++)
			for(int a = 0; a < l; a++ )
				if( match[i] == stopEdges[a] )
					stopEdges[a] = -1;


		
		//wenn kein LUT eintrag, dann diese Edge hinzufügen
		stopEdgeLUT.append(match[bestMatch]);

		////cout<<"ABBRUCH durch STOP_EDGE_SEARCH"<<endl;
		loopResult.append(-2);
		return true;	

	}
	
	//wenn er aufgrund irgendeines Fehlers hierhgin gekommen ist, lieber abbrechen
	////cout<<"FEHLER IN STOPLOOPSEARCH -->BRECHE AB"<<endl;
	return true;
	

}



//----------------------------------------------------------------------------------------------------
void		BPTfty_NH::addToEdgeLoopArray(MIntArray& edgeLoops,MIntArray& tmpLoop,MIntArray& tmpLoop2)
//----------------------------------------------------------------------------------------------------
{

	if( (tmpLoop[tmpLoop.length() - 1] == -3) )
	{
		tmpLoop.setLength(tmpLoop.length() - 1);
		tmpLoop2.setLength(tmpLoop2.length() - 1);
		
		if(tmpLoop.length() < tmpLoop2.length())
			helper.addIntArrayToLHS(edgeLoops,tmpLoop);
		else
			helper.addIntArrayToLHS(edgeLoops,tmpLoop2);
	}
	
	
	else if( (tmpLoop2[tmpLoop2.length() - 1] == -3) )
	{
		tmpLoop2.setLength(tmpLoop2.length() - 1);
		tmpLoop.setLength(tmpLoop.length() - 1);
		
		if(tmpLoop.length() < tmpLoop2.length())
			helper.addIntArrayToLHS(edgeLoops,tmpLoop);
		else
			helper.addIntArrayToLHS(edgeLoops,tmpLoop2);
	}
	
	else if((tmpLoop[tmpLoop.length() - 1] == -2) && (tmpLoop2[tmpLoop2.length() - 1] == -2))
	{
		tmpLoop.setLength(tmpLoop.length() - 1);
		tmpLoop2.setLength(tmpLoop2.length() - 1);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop2);
	}
	
	else if( (tmpLoop2[tmpLoop2.length() - 1] == -2))
	{
		tmpLoop2.setLength(tmpLoop2.length() - 1);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop2);
	}
	
	
	else if( (tmpLoop[tmpLoop.length() - 1] == -2) )
	{
		tmpLoop.setLength(tmpLoop.length() - 1);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop);
	}
	
	
	else
	{
		tmpLoop.setLength(tmpLoop.length() - 1);
		tmpLoop2.setLength(tmpLoop2.length() - 1);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop);
		helper.addIntArrayToLHS(edgeLoops,tmpLoop2);
	}
	
}


//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************
//EDGE RING PROCEDURES
//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------------------
void		BPTfty_NH::findEdgeRings(MIntArray& allEdges,MIntArray& edgeRings)
//--------------------------------------------------------------------------------------------------------------------
{
	//diese Methode wählt rekursiv edgeRings an. Als neuerung befindet sich der >addToEdgeRingArray<Algo direkt
	//in der Methode, so dass sie beim eintauchen in die rekursion ihren vorgang wirklich abschliessen kann
	int l = allEdges.length();

	int edgeCount = 0;
	MIntArray stopEdgeLUT;
	

	for(int i = 0; i < l; i++)
	{
		if(allEdges[i] != -1)
		{
		//proc starten
			findTwoWayRing(allEdges[i],allEdges,edgeRings,edgeCount,stopEdgeLUT);

		}

		stopEdgeLUT.clear();

	}

}


//--------------------------------------------------------------------------------------------------------------------
void	BPTfty_NH::findTwoWayRing(int startEdge,
								  MIntArray& startEdges,
								  MIntArray& edgeRings,
								  int& edgeCount,
								  MIntArray& stopEdgeLUT, 
								  int startFace)
//--------------------------------------------------------------------------------------------------------------------
{
	MIntArray		tmpLoop,tmpLoop2,result,origConnectedFaces,connectedFaces,containedEdges,connectedVtx(2,0),match,edgeVtx(2,0),edgeFaces;


	int				maxEdgeCount = options[2];


	MItMeshPolygon	polyIter(fMesh);
	MItMeshEdge		edgeIter(fMesh);

	bool			go = true;

	int				currentFace,currentEdge,tmp,dirCount = 0,isConnected;


	if(startFace != -1)
		origConnectedFaces.append(startFace);
	else
	{
		edgeIter.setIndex(startEdge,tmp);
		edgeIter.getConnectedFaces(origConnectedFaces);
	}

	stopEdgeLUT.append(startEdge);

	
	//nach eintrag in startEdges suchen und wenn vorhanden -1 setzen
	for(unsigned int t = 0; t < startEdges.length(); t++)
		if(startEdges[t] == startEdge)
			startEdges[t] = -1;
		
	//BitArray für StartEdges aufbauen

	////cout<<"FIND_TWO_WAY_RING: "<<" START START START START START START"<<endl;

	for(unsigned int i = 0; i < origConnectedFaces.length(); i++)
	{
		go = true;
		dirCount =  0;
		currentFace = origConnectedFaces[i];
		currentEdge = startEdge;
		result.clear();
		////cout<<"FIND_TWO_WAY_RING: "<<" BEGINNE NEUEN DURCHLAUF"<<endl;


		while(go)
		//for(int o = 0; o < 12; o++)
		{
			polyIter.setIndex(currentFace,tmp);

			//gleich am Anfang nen EdgeCheck
			if(edgeCount >= maxEdgeCount)
				break;

			////cout<<"FIND_TWO_WAY_RING: "<<" CurrentEdge = "<<currentEdge<<endl;
			////cout<<"FIND_TWO_WAY_RING: "<<" CurrentFace = "<<currentFace<<endl;
			//in stopEdgeLUT nachschauen, ob bereits vorhanden
			if(dirCount >= 1)
			{
				for(unsigned int t = 0; t < stopEdgeLUT.length();t++)
				{
					if(currentEdge == stopEdgeLUT[t])
					{
						////cout<<"FIND_TWO_WAY_RING: "<<" EMERGENCY STOP - LUTLength = "<<stopEdgeLUT.length()<<endl;
						
						result.insert(startEdge,0);
						result.append(currentEdge);
						result.append(-3);
						
						goto whileEnd;
						
					}
				}
			}



			//Hier war früher das entfernen der currentEdge aus selectedEdges - jetzt weiter unten
	
			if(polyIter.polygonVertexCount() != 4)
			{
				////cout<<"FIND_TWO_WAY_RING: "<<"STOP: Kein currentFace kein Quad"<<endl;
				result.insert(startEdge,0);
				result.append(currentEdge);
				result.append(-10);
				go = false;
				break;
			}
			else
			{
				//erstmal schauen, ob faceEdges ausgewählt sind
				polyIter.getEdges(containedEdges);
				
				match = helper.matchArray(containedEdges,startEdges);
				//erstmal die CurrentEdge rausschmeissen
				for(unsigned int a = 0; a < match.length(); a++)
					if(match[a] == currentEdge)
					{
						match.remove(a);
						break;
					}
				
				if(match.length() != 0 && dirCount >= 1)
				{
					////cout<<"FIND_TWO_WAY_RING: "<<" Habe bin an gewählter/en Edge(s) angekommen"<<endl;
					edgeIter.setIndex(currentEdge,tmp);
					connectedVtx[0] = edgeIter.index(0);
					connectedVtx[1] = edgeIter.index(1);

					for(unsigned int a = 0; a < match.length(); a++)
					{	
						edgeIter.setIndex(match[a],tmp);
						edgeVtx[0] = edgeIter.index(0);
						edgeVtx[1] = edgeIter.index(1);

						stopEdgeLUT.append(match[a]);

						for(int x = 0; x < 2;x++)
						{
							for(int y = 0; y < 2; y++)
							{
								if(connectedVtx[x] == edgeVtx[y])
								{
									//erstmal alle rausschmeissen, die direkt mit currentEdge verbunden sind
									match.remove(a--);
								}

							}
	
						}
						
					}

					stopEdgeLUT.append(currentEdge);
					
					//wenn immernoch match vorhanden ist, dann ist es stopEdge
					if(match.length() >= 1)
					{
						result.insert(startEdge,0);
						for(unsigned int a = 0; a < match.length(); a++)
							result.append(match[a]);
						result.append(-2);
						go = false; //zur sicherheit, nur für den fall, dass ich nicht in der whileschleife bin
						break;						
					}
					else
					{
						//andernfalls einfach weitermachen mit loopfinder
						//hier einfach nur eine Edge nehmen, auch wenn mehrere noch vorhanden sein sollten dank wingendVerts
						edgeIter.setIndex(match[0],tmp);
						edgeIter.getConnectedFaces(edgeFaces);

						if(edgeFaces.length() == 2)
						{
							
							if(edgeFaces[0] == currentFace)
								currentFace = edgeFaces[1];
							else
								currentFace = edgeFaces[0];
							
							currentEdge = match[0];
							result.append(currentEdge);
							dirCount++;
							edgeCount++;
							//Eventuell GEFAHR!!!!!!		//müsste nur aus der ForSchleife Raus
							break;
						}
						else
						{
							result.insert(startEdge,0);
							result.append(match[0]);
							result.append(-10);
							go = false; //zur sicherheit, nur für den fall, dass ich nicht in der whileschleife bin
							break;
						}
					
					}
					
					////cout<<"FIND_TWO_WAY_RING: "<<" habe gültige stopEdge gefunden "<<endl;
					

				}
				else
				{

					edgeIter.setIndex(currentEdge,tmp);
					connectedVtx[0] = edgeIter.index(0);
					connectedVtx[1] = edgeIter.index(1);

					for(unsigned int d = 0; d < containedEdges.length();d++)
					{
						//hier ist die eigen edge enthalten, was aber nicht macht, da sie eh connected ist
						edgeIter.setIndex(containedEdges[d],tmp);
						edgeVtx[0] = edgeIter.index(0);
						edgeVtx[1] = edgeIter.index(1);
						
						isConnected = 0;

						for(int x = 0; x < 2; x++)
						{
							for(int y = 0; y < 2; y++)
							{
								if(connectedVtx[x] == edgeVtx[y])
								{
									isConnected = 1;
								}
							}
						}

						if(!isConnected)
						{
							////cout<<"FIND_TWO_WAY_RING: "<<" Habe gegenüberliegende Kante gefunden = "<<containedEdges[d]<<endl;
							//edgeIter müsste noch gültig sein
							edgeIter.getConnectedFaces(edgeFaces);

							if(edgeFaces.length() == 2)
							{
								
								if(edgeFaces[0] == currentFace)
									currentFace = edgeFaces[1];
								else
									currentFace = edgeFaces[0];
								
								currentEdge = containedEdges[d];
								result.append(currentEdge);
								dirCount++;
								edgeCount++;
//Eventuell GEFAHR!!!!!!		//müsste nur aus der ForSchleife Raus
								break;
							}
							else
							{
								////cout<<"FIND_TWO_WAY_RING: "<<" STOP - Keine gültige Edge gefunden"<<endl;
								result.insert(startEdge,0);
								result.append(containedEdges[d]);
								result.append(-10);
								go = false; //zur sicherheit, nur für den fall, dass ich nicht in der whileschleife bin
								break;
							}
						}
					}
					
					isConnected = 0;
					if(go == false)
						break;

				}
			}
		}


whileEnd:

		////cout<<"FIND_TWO_WAY_RING: "<<" Setze tmpLoop(2) auf Result"<<endl;
		//Hier die Resultate bearbeiten (edges anwählen usw.)
		
		tmp = result[result.length() - 2];
		for(unsigned int a = 0; a < startEdges.length(); a++)
		{
			if(startEdges[a] == tmp)
			{
				startEdges[a] = -1;
				break;
			}
		}
		
		
		if(i == 0)
			tmpLoop = result;
		else
			tmpLoop2 = result;

		

	}

	////printArray(tmpLoop," = tmpLoop");
	////printArray(tmpLoop2," = tmpLoop2");
	if(tmpLoop2.length() == 0)
		helper.addIntArrayToLHS(edgeRings,tmpLoop);
	else
	{
		//wenn beide Arrays gefüllt, dann die startEdge noch einfügen
		tmpLoop.insert(startEdge,0);
		//auch die EdgeLoopArrayMethode kann man benutzen
		addToEdgeLoopArray(edgeRings,tmpLoop,tmpLoop2);
	}


}


//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************
//GROW/SHRINK PROCEDURES
//*********************************************************************************************************************************************************************
//*********************************************************************************************************************************************************************


//---------------------------------------------------------------------------------
void		BPTfty_NH::schrinkVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter,MItMeshPolygon& polyIter, bool doSelect)
//---------------------------------------------------------------------------------
{
	MIntArray removals;
	MIntArray	match,conVtx;
	
	

	int l = vtxArray.length(), tmp;


	//versuch 3, speed 2, aber ist reversibel durch grow
	MIntArray conFaces,allFaces;
	int i;
	for(i = 0; i < l; i++)
	{
		vertIter.setIndex(vtxArray[i],tmp);
		vertIter.getConnectedFaces(conFaces);
		helper.addIntArrayToLHS(allFaces,conFaces);
	}
	helper.memoryPrune(allFaces);
	
	MIntArray conVerts;
	l = allFaces.length();
	BPT_BA memArray(vtxArray,true);

	for(i = 0; i < l; i++)
	{
		polyIter.setIndex(allFaces[i],tmp);
		
	

		polyIter.getVertices(conVerts);
		
		
		MIntArray match;
		memArray.findMatching(conVerts,match);

		//if(helper.matchArray(vtxArray,conVerts).length() != conVerts.length())
		if(match.length() != conVerts.length())
		{
			helper.addIntArrayToLHS(removals,conVerts);
		}

	}


	if(!doSelect)
		helper.arrayRemove(vtxArray,removals);
	else
		selectComponents(removals,"vertices","remove");
	
	MPRINT("Habe Vtx geschrinkt")

}

//-----------------------------------------------------------------------------------------------------------
void		BPTfty_NH::growVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter, MItMeshPolygon& polyIter,bool doSelect)
//-----------------------------------------------------------------------------------------------------------
{


	int l = vtxArray.length(), tmp;

	MIntArray conFaces,allFaces;
	int i;
	for(i = 0; i < l; i++)
	{
		vertIter.setIndex(vtxArray[i],tmp);
		vertIter.getConnectedFaces(conFaces);
		helper.addIntArrayToLHS(allFaces,conFaces);
	}
	helper.memoryPrune(allFaces);

	l = allFaces.length();

	MIntArray	growedSelection,vtxTmp;
	for(i = 0; i < l; i++)
	{
		polyIter.setIndex(allFaces[i],tmp);
		polyIter.getVertices(vtxTmp);

		helper.addIntArrayToLHS(growedSelection,vtxTmp);
	}
	helper.memoryArrayRemove(growedSelection,vtxArray);


	if(!doSelect)
		helper.addIntArrayToLHS(vtxArray,growedSelection);
	else
	//	selectComponents(adds,"vertices","add");
		selectComponents(growedSelection,"vertices","add");

	MPRINT("Habe Vtx gegrowed")



}

//--------------------------------------------------------------------------
void		BPTfty_NH::growFaceSelection(MIntArray& faceArray,MItMeshEdge& edgeIter, int numFaces )
//--------------------------------------------------------------------------
{
	/*	//Veruch 1: kein wirkliches grow
	MIntArray		inverted;
	MIntArray		EdgeTmp;
	MIntArray		Edge2Tmp;
	
	invertSelection(faceArray, numFaces,inverted);
	
	faceGetContainedEdgeIndices(faceArray, EdgeTmp);
	faceGetContainedEdgeIndices(inverted, Edge2Tmp);

	MIntArray	boundEdges = memoryMatch(EdgeTmp,Edge2Tmp);

	MIntArray faceTmp, faceAdd;
	int l = boundEdges.length(),tmp;
	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(boundEdges[i],tmp);
		edgeIter.getConnectedFaces(faceTmp);
		addIntArrayToLHS(faceAdd,faceTmp);
	}

	//auswahl erstellen
	selectComponents(faceAdd,"polys","add");
	*/
	
	MIntArray conEdges,conFaces, faceTmp;

	faceGetConnectedEdgeIndices(faceArray,conEdges);

	helper.memoryPrune(conEdges);

	int l = conEdges.length(),tmp;

	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(conEdges[i],tmp);
		edgeIter.getConnectedFaces(faceTmp);		

		helper.addIntArrayToLHS(conFaces,faceTmp);
	}

	helper.memoryPrune(conFaces);

	selectComponents(conFaces,"polys","add");

}

//--------------------------------------------------------------------------
void		BPTfty_NH::shrinkFaceSelection(MIntArray& faceArray,MItMeshEdge& edgeIter, int numFaces )
//--------------------------------------------------------------------------
{
	
	//MIntArray		inverted;
	MIntArray		EdgeTmp;
	//MIntArray		Edge2Tmp;
	
	//invertSelection(faceArray, numFaces,inverted);
	
	faceGetContainedEdgeIndices(faceArray, EdgeTmp);
	//faceGetContainedEdgeIndices(inverted, Edge2Tmp);

	//MIntArray	boundEdges = helper.memoryMatch(EdgeTmp,Edge2Tmp);
	MIntArray	boundEdges;
	helper.memoryPrune(EdgeTmp,boundEdges);

	helper.memoryArrayRemove(EdgeTmp,boundEdges);

	MIntArray faceTmp, faceAdd;
	int l = EdgeTmp.length(),tmp;
	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(EdgeTmp[i],tmp);
		edgeIter.getConnectedFaces(faceTmp);
		helper.addIntArrayToLHS(faceAdd,faceTmp);
	}

	//auswahl erstellen
	selectComponents(faceAdd,"polys","remove");


}

//-----------------------------------------------------------------------------------------------
void		BPTfty_NH::growEdgeSelection(MIntArray& edgeArray, MItMeshEdge& edgeIter, int numFaces)
//-----------------------------------------------------------------------------------------------
{
	MIntArray conEdges,eTmp;

	int l = edgeArray.length(),tmp;
	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeArray[i],tmp);
		edgeIter.getConnectedEdges(eTmp);
		helper.addIntArrayToLHS(conEdges,eTmp);
	}

	helper.memoryPrune(conEdges);
	

	selectComponents(conEdges,"edges","add");

}

//-----------------------------------------------------------------------------------------------
void		BPTfty_NH::shrinkEdgeSelection(MIntArray& edgeArray,MItMeshEdge& edgeIter, int numFaces)
//-----------------------------------------------------------------------------------------------
{
	MIntArray	conFaces, faceTmp, inverted, edgeTmp, edgeTmp2,edgeBound;

	int l = edgeArray.length(),tmp, i;
	
	for(i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeArray[i],tmp);
		edgeIter.getConnectedFaces(faceTmp);

		helper.addIntArrayToLHS(conFaces,faceTmp);
	}

	invertSelection(conFaces,numFaces,inverted);

	faceGetContainedEdgeIndices(conFaces, edgeTmp);
	faceGetContainedEdgeIndices(inverted, edgeTmp2);

	edgeBound = helper.memoryMatch(edgeTmp,edgeTmp2);

	l = edgeBound.length();
	conFaces.clear();

	for(i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeBound[i],tmp);
		edgeIter.getConnectedFaces(faceTmp);

		helper.addIntArrayToLHS(conFaces,faceTmp);
	}

	edgeTmp.clear();

	faceGetContainedEdgeIndices(conFaces, edgeTmp);



	MIntArray tmpArray = helper.memoryMatch(edgeTmp,edgeArray);
	selectComponents(tmpArray,"edges","remove");

	

}
