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
// File: BPTfty.cpp
//
// Node Factory: BPTfty
//
// Author: Sebastian Thiel
//



#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnComponentListData.h>
#include <maya/MIntArray.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshEdge.h>
#include <maya/MSelectionMask.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnNumericData.h>
#include <maya/MCommandResult.h>


#include <MACROS.h>

#include "BPTfty.h"
#include "BPT_BA.h"



//-----------------------------------------------------------------
BPTfty::BPTfty():	slide(0.5),
					ftyCreator(0),

					UVSlideStart(0),
					UVSlideEnd(0),
					UVSlideIndices(0),
					UVSlideDirections(0),	
					UVSlideScale(0),
/*
					Us(0),
					Vs(0),
					UVCounts(0),
					UVperVtx(0),
*/	

					maxStandardScale(1.0),

					normalScale(0),		
					slideScale(0),		
					slideDirections(0),	
					slideNormals(0),		
					slideStartPoints(0),
					slideEndPoints(0),
					slideIndices(0),
					maySlide(0)
//-----------------------------------------------------------------
{


}


//-----------------------------------------------------------------
BPTfty::~BPTfty()
//-----------------------------------------------------------------
{//MERKE: die fty mus auch damit klarkommen, dass keine variable richtig initialisiert wude (da zum Beispiel keine OutConenction existiert)
	

	//{//Alle Arrays loeschen
		if(UVSlideStart != 0)
		{
			delete UVSlideStart;	
			delete UVSlideEnd;
			delete UVSlideIndices;
			delete UVSlideDirections;	
			delete UVSlideScale;		
		}

		/*if(Us != 0)
		{
			delete Us;
			delete Vs;
			delete UVCounts;
			delete UVperVtx;
		}*/
	

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


//-----------------------------------------------------------------
void BPTfty::setOptions(const MIntArray o)
//-----------------------------------------------------------------
{
	options = o;
}

//-----------------------------------------------------------------
void BPTfty::setRelative(int value)
//-----------------------------------------------------------------
{
	isRelative = value;
}

//--------------------------------------------
void	BPTfty::setNormalRelative(int value)
//--------------------------------------------
{
	isNormalRelative = value;
}


//-----------------------------------------------------------------
void BPTfty::setSlide(double inSlide)
//-----------------------------------------------------------------
{
	slide = inSlide;
}

//-----------------------------------------------------------------
void BPTfty::setNormal(double value)
//-----------------------------------------------------------------
{
	normal = value;
}




//-----------------------------------------------------------------
void BPTfty::setSide(int value)
//-----------------------------------------------------------------
{
	options[3] = value;
}




//-----------------------------------------------------------------
void BPTfty::setMesh( MObject mesh )
//-----------------------------------------------------------------
{
	fMesh = mesh;
}


//-----------------------------------------------------------------
void BPTfty::setActionMode(int aMode)
//-----------------------------------------------------------------
{
	actionMode = aMode;
}



//-----------------------------------------------------------------
void	BPTfty::setVertIDs(const MIntArray array)
//-----------------------------------------------------------------
{
	MIntArray tmp = array;	//wegen bug in maya muss ich das so machen, denn obwohl hier eine konstante kopie übergeben wird, wird irgendwie doch das MObject des Attributes mitmanipuliert
	vertIDs = tmp;
}


//-----------------------------------------------------------------
void	BPTfty::setEdgeIDs(const  MIntArray array)
//-----------------------------------------------------------------
{
	MIntArray tmp = array;
	edgeIDs = tmp;
}


//-----------------------------------------------------------------
void	BPTfty::setPolyIDs(const  MIntArray array)
//-----------------------------------------------------------------
{
	MIntArray tmp = array;
	//polyIDs = array;
	polyIDs = tmp;
}

//-----------------------------------------------------------------
MObject	BPTfty::getMesh()
//-----------------------------------------------------------------
{
	if(actionMode == 6)
		return fMesh;
	else
		return newMeshData;
}

//******************************************************************
//*********************MESH-METHODEN********************************
//******************************************************************

//-----------------------------------------
void		BPTfty::switchComponentModeIfNeeded(bool mode,MSelectionMask::SelectionType type)
//-----------------------------------------
{
	

	if(mode)
	{
		if(options[5])
		{
			if(options[6] == 5)
			{
				type = MSelectionMask::kSelectMeshVerts;
			}
			else if(options[6] == 2)
			{
				type = MSelectionMask::kSelectMeshFaces;
			}


		}
	}



	if(options[5])
	{
		
		
		MSelectionMask mask(type);
		
		
		MGlobal:: setComponentSelectionMask(mask);
	}
	
}

//---------------------------------------
void BPTfty::initUVSlideArrays()
//---------------------------------------
{
	MFnMesh FnMesh(fMesh);
	FnMesh.getUVSetNames(UVSetNames);
	
	//Ptr erstmal mit DAten versehen, damit er hier nicht abkackt - normalerweise würde das vom creator gemact werden, dieser wird aber 
	//von SMC nicht verwendet

	UVSlideEnd = new list<MFloatVectorArray> ();
	UVSlideStart = new list<MFloatVectorArray> ();
	UVSlideDirections = new list<MFloatVectorArray> ();
	UVSlideScale = new list<MFloatArray> ();

	UVSlideIndices = new list<MIntArray> ();
	for(unsigned int i = 0; i < UVSetNames.length();i++)
	{
		//UVSlideArrays initialisieren
		UVSlideDirections->push_back(MFloatVectorArray());
		UVSlideStart->push_back(MFloatVectorArray());
		UVSlideEnd->push_back(MFloatVectorArray());
		UVSlideIndices->push_back(MIntArray());

		UVSlideScale->push_back(MFloatArray());
		

	}
}

//-----------------------------------------------------------------
void		BPTfty::initSlideArrays()
//-----------------------------------------------------------------
{
	normalScale = new MDoubleArray();		//hat eigene versionen der Arrays, die schlussendlich von der BBTfty 
	slideScale = new MDoubleArray();			//geholt werden
	slideDirections = new MVectorArray();
	slideNormals = new MVectorArray();		//wenn ein Vtx nicht slided, dann wird gegenwärtiger punkt als startPunkt eingetragen
	slideStartPoints = new MPointArray();	//und maySlide[n] wird auf 0 gesetzt
	slideEndPoints = new MPointArray();
	slideIndices = new MIntArray();
	maySlide = new MIntArray();			//synchron zu allen anderen Arrays, 0 wenn er sliden darf, 1 wenn nicht.

}



//******************************************************************
//*********************MESH-MODIFIKATOREN***************************
//******************************************************************


//-----------------------------------------------------------------
MIntArray		BPTfty::findOuterEdges(MIntArray& edges, MIntArray& verts)
//-----------------------------------------------------------------
{
	MIntArray	result;
	MItMeshEdge	edgeIter(fMesh);	
	int			tmp;
	MIntArray	IDs(2,0);

	int l =	edges.length();

	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(edges[i],tmp);
		IDs[0] = edgeIter.index(0);
		IDs[1]= edgeIter.index(1);

		if(helper.matchArray(IDs,verts).length() == 1)
			result.append(edges[i]);

	}
	
	return result;
}

//******************************************************************
//*********************ARRAY-METHODEN*******************************
//******************************************************************





//**************************************************************************
//************************CONVERSION_METHODEN*******************************
//**************************************************************************

//---------------------------------------------------------------------
void BPTfty::removeOverlappingFaces(MIntArray& faceIDs,MIntArray &vtxEdges)
//---------------------------------------------------------------------
{
		int tmp;
		MIntArray	tmpEdges;
		MIntArray	removals;
		MItMeshPolygon polyIter(fMesh);
		int l = faceIDs.length();

		for(int i = 0; i < l; i++)
		{
			polyIter.setIndex( faceIDs[i] ,tmp);
			polyIter.getEdges(tmpEdges);
			
			if(helper.matchArray(vtxEdges,tmpEdges).length() > 0)
				removals.append(faceIDs[i]);
		}

		helper.arrayRemove(faceIDs, removals);

}



//---------------------------------------------------------------------
void BPTfty::vertGetConnectedEdgeIndices(MIntArray& vertIndices,MIntArray& edgeIndices)
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
void BPTfty::faceGetConnectedEdgeIndices(MIntArray& faceIndices,MIntArray& edgeIndices)
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



//---------------------------------------------------------------------
void BPTfty::faceGetContainedEdgeIndices(MIntArray& faceIndices, MIntArray& edgeIndices)
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
		iter.getEdges(tmpArray);
		helper.addIntArrayToLHS(edgeIndices, tmpArray);
	}



}

//---------------------------------------------------------------------
void	BPTfty::faceGetContainedEdgeIndices(MIntArray& faceIndices, MIntArray& edgeIndices,MObject& mesh)
//---------------------------------------------------------------------
{

	MIntArray		tmpArray;
	MItMeshPolygon	iter(mesh);


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
void BPTfty::invertSelection(MIntArray& lhs,int maxElement, MIntArray& inverted)
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
//**************************************************************************
//****************************HELFER_METHODEN*******************************
//**************************************************************************



/*
//-----------------------------------------------------------------
void	BPTfty::getUVPositions(list<MFloatArray>*& inU, list<MFloatArray>*& inV)
//-----------------------------------------------------------------
{
	inU = Us;
	inV = Vs;
}

//-----------------------------------------------------------------
void	BPTfty::getUVIntArrays(list<MIntArray>*& inUVCounts,list<MIntArray>*& inUVperVtx )
//-----------------------------------------------------------------
{
	inUVCounts = UVCounts;
	inUVperVtx = UVperVtx;
}
*/
//-----------------------------------------------------------------
void		BPTfty::getAllEdgeIndices(MIntArray& allIndices)
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

//-----------------------------------------------------------------
void	BPTfty::getAllVertexIndices(MIntArray& allIndices)
//-----------------------------------------------------------------
{

	int l,tmp;
	MIntArray array;
	if(edgeIDs.length() > 0)
	{
		MItMeshEdge edgeIter(fMesh);
		l = edgeIDs.length();
		for(int i = 0; i < l; i++)
		{
			edgeIter.setIndex(edgeIDs[i],tmp);

			array.append(edgeIter.index(0));
			array.append(edgeIter.index(1));
		}

		helper.addIntArrayToLHS(allIndices,array);
	}

	array.clear();

	if(polyIDs.length() > 0)
	{
		MItMeshPolygon polyIter(fMesh);
		l = polyIDs.length();
		
		for(int i = 0; i < l; i++)
		{
			polyIter.setIndex(polyIDs[i],tmp);

			polyIter.getVertices(array);

			helper.addIntArrayToLHS(allIndices,array);
		}

	
	}
	
	helper.addIntArrayToLHS(allIndices, vertIDs);
	
	if(polyIDs.length() != 0 || edgeIDs.length() != 0)
		helper.memoryPrune(allIndices);

//	printArray(allIndices," = vertIndices");

}

//------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::addNormalSlide(int inVtx,MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//------------------------------------------------------------------------------------------------------------------------------------
{
	INVIS(cout<<"HABE VTX "<<inVtx<<" HINZUGEFUEGT (NORMAL)"<<endl;)

	int tmp;
	
	slideIndices->append(inVtx);


	vertIter.setIndex(inVtx,tmp);
	slideStartPoints->append(vertIter.position());

	//normalen erstellen
	MIntArray faces;
	vertIter.getConnectedFaces(faces);

	double scale = 0.0, tmpArea; MVector normal,tmpNormal;
	for(unsigned int i = 0; i < faces.length(); i++)
	{
		polyIter.setIndex(faces[i],tmp);
		polyIter.getArea(tmpArea);

		scale += tmpArea;

		polyIter.getNormal(tmpNormal);

		normal += tmpNormal;
	}

	scale /= faces.length();
	normal /= faces.length();

	normal.normalize();

	slideNormals->append(normal);
	normalScale->append(scale);
	
	slideEndPoints->setLength(slideEndPoints->length() + 1);			
	slideDirections->setLength(slideDirections->length() + 1);
	
	slideScale->append(-1);			

	maySlide->append(0);

	
	slideDirections2.setLength( (slideDirections2.length() + 1) );
	slideScale2.append(-1);
	normalScale2.setLength(normalScale2.length() + 1);
	slideNormals2.setLength(slideNormals2.length() +1);		
	hasSecondDirection.append(0);

}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::addSlideAndNormal(int inVtx,MPointArray& startEnd,MVectorArray& directions ,MVectorArray& normals, MDoubleArray& normalScaleFactors)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
{

	INVIS(cout<<"HABE VTX "<<inVtx<<" HINZUGEFUEGT (NORMAL&SLIDE)"<<endl;)

	slideIndices->append(inVtx);

	slideNormals->append(normals[0]);
	normalScale->append(normalScaleFactors[0]);

	
	slideNormals2.append(normals[1]);
	normalScale2.append(normalScaleFactors[1]);
	

	slideStartPoints->append(startEnd[0]);
	slideEndPoints->append(startEnd[1]);			
	slideDirections->append(directions[0]);
	slideScale->append(-1);
	slideDirections2.append(directions[1]);
	slideScale2.append(-1);

	maySlide->append(1);

	hasSecondDirection.append(1);



}

//----------------------------------------------------------------
void		BPTfty::SMC_deleteLastXSlideValues(int x, int offset)
//----------------------------------------------------------------
{
	int l = slideIndices->length();
	int bound = l - x - 1;
	l--;
	
	for(int i = l-offset; i > bound; i--)
	{
	//	slideIndices.setLength(slideIndices.length() - x);
		
	//	slideNormals.setLength(slideNormals.length() - x);
	//	normalScale.setLength(normalScale.length() - x);
		
		
	//	slideNormals2.setLength(slideNormals2.length() - x);
	//	normalScale2.setLength(normalScale2.length() - x);
		
		
		(*slideStartPoints)[i] = (*slideStartPoints)[i] + (*slideDirections)[i];

		
		(*maySlide)[i] = 0;
		
		hasSecondDirection[i] = 0;
	}

}

//----------------------------------------------------------------
int			BPTfty::SMC_getIndex(int vtxID, MIntArray& linedVtx)
//----------------------------------------------------------------
{
	int l = linedVtx.length();
	for(int i = 0; i < l; i++)
		if(linedVtx[i] == vtxID)
			return i;

	//forbiddenPath
	MPRINT("SMC_GETINDEX - WAR BEI FORBIDDEN PATH (ist Teil des Konzepts)")
	return -1;
}

//---------------------------------------------------------------------------------------------------------
void		BPTfty::SMC_createNormal(MIntArray& currentFaces,MIntArray& directionFaces, MVector& normal,double& scale, MItMeshPolygon& polyIter)
//---------------------------------------------------------------------------------------------------------
{
	MIntArray match;
	int tmp;
	normal.x = 0.0;normal.y = 0.0;normal.z = 0.0;
	scale = 0.0;
	double tmpScale;
	MVector	tmpVec;

	match = helper.matchArray(currentFaces,directionFaces);

	for(unsigned int i = 0; i < match.length(); i++)
	{
		polyIter.setIndex(match[i],tmp);

		polyIter.getNormal(tmpVec);
		polyIter.getArea(tmpScale);

		scale += tmpScale;
		normal += tmpVec;
	}

	scale /= match.length();
	normal /= match.length();
}

//----------------------------------------------------------------------------------------------------------------------//----------
int		BPTfty::SMC_getDirection(MIntArray& currentFaces, MPoint& startPos, MIntArray* connections, MItMeshVertex& vertIter)
//----------------------------------------------------------------------------------------------------------------------//----------
{
	//Okay, jetzt noch die neue direction bestimmen
	int tmp;

//	printArray(currentFaces," = currentFaces SMC");
	//im ersten durchlauf wird auf gemeinsame faces gecheckt
	MPointArray positionsSave;
	MIntArray directionFaces;

	unsigned int l = connections->length();
	
	UINT x; 	
	for(x = 0; x < l; x++)
	{
		if((*connections)[x] < 0)
		{
			vertIter.setIndex( -(*connections)[x] - 1,tmp);
			vertIter.getConnectedFaces(directionFaces);

		//	printArray(directionFaces," = directionFaces SMC");
			
			positionsSave.append(vertIter.position() ); //für den zweiten Pass, falls noetig
			
			if(helper.matchArray(currentFaces,directionFaces).length() > 0)
			{	
				MPRINT("Habe direction im ersten Pass gefunden")
				return -(*connections)[x] - 1;
			}
		}
	}
	
	
	//jetzt den nächstgelegenen Vtx als direction nehmen, weil im ersten pass kein vtx gefunden wurde
	MPRINT("Direction im ersten Pass nicht gefunden, starte zweiten Ansatz")
	
	double minDist = 16666666;
	int		minIndex = 0;
	for(x = 0; x < positionsSave.length(); x++)
	{
		if(positionsSave[x].distanceTo(startPos) < minDist)
		{
			minDist = positionsSave[x].distanceTo(startPos);
			minIndex = x;
		}
	}
	
	//jetzt noch den entsprechenden index bei den unselected Vtx im connectionsArray finden
	int counter = 0;
	for(x = 0; x < l; x++)
	{
		if((*connections)[x] < 0)
		{
			if(counter++ == minIndex)
			{
							
				MPRINT("Habe direction im 2. Pass gefunden")
				return -(*connections)[x]-1;
				
				break;
			}
		}
	}
	
	//forbidden path
	cout<<"WARNUNG: BIN BEI FORBIDDEN PATH VON SMC_GETDIRECTION"<<endl;
	return -1;

}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int		BPTfty::SMC_processLine(int currentVtx,
								int lastVtx, 
								int dVtx,
								int& counter,
								bool isFlipped,
								MIntArray& vtxRemovals, 
								MIntArray& linedVtx,
								MIntArray& isControlTwo,
								MIntArray& isFlippedArray,
								MIntArray& countArray,
								simpleLinkedList<MIntArray>& connectionsList, 
								MItMeshVertex& vertIter, 
								MItMeshPolygon& polyIter)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	MIntArray		currentFaces,directionFaces;
	MVectorArray	normals, directions;
	MPointArray		startEnd;
	MDoubleArray	normalScales;

	double		scale;

	MPoint		startPos, endPos, currentPos;
	MVector		normal;

	
	int index = SMC_getIndex(currentVtx,linedVtx);

	int tmp;

	//isFlipped bearbeiten, also directions vertauschen
	if(isFlipped)
	{MPRINT("Will Direction flippen")
		int convertedDVtx = -(dVtx+1);
		int l2 = connectionsList[index]->length();
		//dVtx finden
		for(int i = 0; i < l2; i++)
		{
			if(convertedDVtx == (*connectionsList[index])[i] )
			{//jetzt den nächsten selVtx finden von hier aus
				bool foundLastVtxOnce = false;
				bool found;
				for(int x = i+1; ;x++)
				{	
					found = false;

					if((*connectionsList[index])[x%l2] >= 0 )
					{//checken, ob iteration zuerst den lastVtx erwischt hat
						if((*connectionsList[index])[x%l2] == lastVtx )
						{INVIS(cout<<"Iteration war zuerst bei LastVtx: "<<(*connectionsList[index])[x%l2]<<endl;)
							foundLastVtxOnce = true;
							found = true;
						}

						if(!found)
						{//ist der nächste Vtx der richtige oder der vorherige?
							INVIS(cout<<"Bin bei SelID "<<(*connectionsList[index])[x%l2]<<endl;)
							int operand = 1;

							operand = (foundLastVtxOnce)?-1:1;
							
							INVIS(cout<<"Operand == "<<operand<<endl;)

							dVtx = -(*connectionsList[index])[(x+operand+l2)%l2]-1;

							INVIS(cout<<"Habe direction geflipped, sie ist jetzt "<<dVtx<<endl;)
							
							break;
						}
					}		
				}

				break;
			}
		}

	}


	while(1)
	{
		startEnd.clear();
		normals.clear();
		normalScales.clear();
		directions.clear();
		//erstmal gegenwärtigen Vtx eintragen und mit slide versehen
		vtxRemovals.append(currentVtx);
		
		//debug:
		INVIS(cout<<"-------------------------------------------"<<endl;)
		INVIS(cout<<"CurrentVtx ist : "<<currentVtx<<endl;)
		INVIS(cout<<"LastVtx ist : "<<lastVtx<<endl;)
		INVIS(cout<<"dVtx ist : "<<dVtx<<endl;)
		INVIS(cout<<"-------------------------------------------"<<endl;)
		
		vertIter.setIndex(currentVtx,tmp);
		vertIter.getConnectedFaces(currentFaces);
		
		currentPos = vertIter.position();
		
		//jetzt die Position des StartVtx holen
		vertIter.setIndex(dVtx,tmp);
		vertIter.getConnectedFaces(directionFaces);
		
		startPos = vertIter.position();
		
		SMC_createNormal(currentFaces,directionFaces, normal,scale,polyIter);	//normale/scale wird in der methode 0 gesetzt
		normals.append(normal);
		normalScales.append(scale);
		
		directions.append(currentPos - startPos);
		//jetzt die Position des endVtx holen
		//index = SMC_getIndex(currentVtx,linedVtx); //index ist noch vom letzten durchlauf gesetzt
		
		int endVtx;

		int l = connectionsList[index]->length();
		
		int x;
		for(x = 0; x < l;x++)
		{
			int dVtxConverted = -(dVtx+1);
			if( (*connectionsList[index])[x] == dVtxConverted )
			{//bei quadMeshes ist die sache klar, bei anderen nicht so
				
				if( (*connectionsList[index])[(x+2)%l] < 0)
				{
					endVtx = -((*connectionsList[index])[(x+2)%l] + 1);
					vertIter.setIndex( endVtx,tmp);				
				}
				else
				{
					MPRINT("PROCESS UNCONTROLLED LINED VTX: Unerwarteter Wert bei endVtx, starte erweiterte Suche")
					for(int c = x+3; ;c++)
					{
						if((*connectionsList[index])[c%l] < 0 && (*connectionsList[index])[c%l] != dVtxConverted )
						{
							endVtx = -((*connectionsList[index])[c%l] + 1);
							vertIter.setIndex( endVtx,tmp);
							break;
						}	
					}
				}
				
				break;
			}
		}
		
		//jetzt ist endVtx definitiv gesetzt
		endPos = vertIter.position();
		
		vertIter.getConnectedFaces(directionFaces);
		
		SMC_createNormal(currentFaces,directionFaces,normal,scale,polyIter);
		
		normals.append(normal);
		normalScales.append(scale);
		
		directions.append(endPos - currentPos);
		
		startEnd.append(startPos);
		startEnd.append(endPos);
		
		//slide erstellen 
		addSlideAndNormal(currentVtx,startEnd,directions,normals,normalScales); //momentan wird der firstVtx doppelt gemoppelt
		counter++;

		//UVs erstellen:
		SMC_createUVSlide(currentVtx,endVtx,dVtx,vertIter,polyIter);

		
		//-------------DataGather End---------------------------------
		//jetzt den nächsten Vtx finden
		
		if(countArray[index] > 1)
		{
			//durch connectionsArray parsen und Vtx finden, der nicht lastVtx ist
			//printArray((*connectionsList[index])," = Connetions bei GetNewCurrent");
			
			for(x = 0; x < l; x++)
			{
				if( (*connectionsList[index])[x] >= 0 && (*connectionsList[index])[x] != lastVtx )
				{
					lastVtx = currentVtx;
					currentVtx = (*connectionsList[index])[x];
					break;
				}
			}
			

			
			//index aktualisieren
			index = SMC_getIndex(currentVtx,linedVtx);
			
			if(index == -1)
			{
				MPRINT("Bin an Type3 || Type4 gestossen, beende Loop")
				
				return -2;
			}

			if(isControlTwo[index])
			{
				MPRINT("Bin an Typ2 gestossen, beende Loop")
				//Diese Methode behandelt diesen Fall, checkt ob flipped oder nicht
				//selectedVtx eintagen
				MIntArray selectedVtx;
				l = connectionsList[index]->length();
				for(x = 0; x < l; x++)
				{
					if( (*connectionsList[index])[x] >= 0 )
						selectedVtx.append((*connectionsList[index])[x]);
				}
				int faceID; MPoint tmpPoint;
				int controlDir = SMC_getTypeTwoControlDirection(currentVtx,(lastVtx == selectedVtx[0])?0:1,faceID,tmpPoint,selectedVtx,vertIter,polyIter);
				

				if(controlDir != dVtx)
				{MPRINT("PROCESS LINE: Musste direction flippen")
					isFlippedArray.append(currentVtx);
				}

				return index;
			}
			
			//Okay, jetzt noch die neue direction bestimmen
			
			vertIter.setIndex(dVtx,tmp);
			vertIter.getConnectedFaces(currentFaces);
			
			dVtx = SMC_getDirection(currentFaces,startPos,connectionsList[index],vertIter);
			//FERTIG
			
		}
		else
		{
			MPRINT("Bin an EndStück gestossen, beende Loop")
			return -1;
		}
		
	}

}

//----------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::SMC_createControlTwoUVSlide(int commonVtx,
												int dVtx, 
												MIntArray endVerts,
												MItMeshVertex& vertIter, 
												MItMeshPolygon& polyIter,
												bool isFlipped)
//----------------------------------------------------------------------------------------------------------------------------------
{
	INVIS(cout<<endl<<endl;);
	MPRINT("---------------------------");
	MPRINT("CREATE SMC UV SLIDE(CONTROL TWO): BEGIN");
	MPRINT("---------------------------");
	INVIS(cout<<"ARBEITE MIT (COMMON/dVtx/END[0]) VTX: "<<commonVtx<<", "<<dVtx<<", "<<endVerts[0]<<endl<<endl;);

	
	int tmp;
	MIntArray connectedFaces;
	MIntArray startFaces,endFaces;
	//zuerst Start und endFaces sortieren: Faces mit dVtx sind StartFaces, alle mit endVtx sind endFaces
	//faces die zr debatte stehen sind die connected Faces von commonVtx

	vertIter.setIndex(commonVtx,tmp);
	vertIter.getConnectedFaces(connectedFaces);
	
	//tempArrays erstellen für match
	MIntArray startVtx,endVtx;
	startVtx.append(commonVtx);startVtx.append(dVtx);
	endVtx.append(commonVtx); endVtx.append(endVerts[0]);

	unsigned int l = connectedFaces.length();
	
	UINT i;
	for(i = 0; i < l; i++)
	{
		MIntArray faceVerts;
		polyIter.setIndex(connectedFaces[i],tmp);
		polyIter.getVertices(faceVerts);
		
		if(helper.matchArray(faceVerts,startVtx).length() == 2)
			startFaces.append(connectedFaces[i]);
		else if(helper.matchArray(faceVerts,endVtx).length() == 2)
			endFaces.append(connectedFaces[i]);

	}

	INVIS(if(startFaces.length() != endFaces.length())\
		cout<<"ERROR: StartFaces nicht so gross wie endFaces: "<<startFaces.length()<<" zu "<<endFaces.length()<<endl;);

	//jetzt durch die Faces durchgehen, checken ob sie UVs haben, und wenn ja, die zu start und endVtx passenden UVs finden und eintragen
	
	//erstmal sichergehen, dass die Arrays auch die selbe Groesse haben
	(startFaces.length() < endFaces.length())?endFaces.setLength(startFaces.length()):startFaces.setLength(endFaces.length());
	


	std::list<MIntArray>::iterator iterSlideIDs = UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();


	MFnMesh meshFn(fMesh);

	for(unsigned int u = 0; u < UVSetNames.length(); u++)
	{

		l = startFaces.length();
		for(i = 0; i < l; i++)
		{
			bool startHasNoUVs = false, endHasNoUVs = false;
			int startUV,commonUV_S,commonUV_E,endUV,endUV2;

			
			polyIter.setIndex(startFaces[i],tmp);
			if(polyIter.hasUVs(UVSetNames[u],0) )
			{
				//lokalen index des startVtx in Face finden
				MIntArray verts;
				polyIter.getVertices(verts);

				UINT h;
				for(h = 0; h < verts.length(); h++)
				{
					if(verts[h] == dVtx)
					{
							polyIter.getUVIndex(h,startUV,&UVSetNames[u]);
							break;
					}

				}


				for(h = 0; h < verts.length(); h++)
				{
					if(verts[h] == commonVtx)
					{
						polyIter.getUVIndex(h,commonUV_S,&UVSetNames[u]);
						break;
					}

				}


			}
			else
			{//einfach -1 Flag setzen und fortfahren
				startHasNoUVs = true;
			}


			
			polyIter.setIndex(endFaces[i],tmp);

			if( polyIter.hasUVs(UVSetNames[u],0) )
			{
				//lokalen index des startVtx in Face finden
				MIntArray verts;
				polyIter.getVertices(verts);
				
				UINT h;
				for(h = 0; h < verts.length(); h++)
				{
					if(verts[h] == endVerts[0])
					{
						polyIter.getUVIndex(h,endUV,&UVSetNames[u]);
						break;
					}

				}

				if(endVerts[1] > -1)
				{
					for(unsigned int h = 0; h < verts.length(); h++)
					{
						if(verts[h] == endVerts[1])
						{
							polyIter.getUVIndex(h,endUV2,&UVSetNames[u]);
							break;
						}

					}
				}


				for(h = 0; h < verts.length(); h++)
				{
					if(verts[h] == commonVtx)
					{
						polyIter.getUVIndex(h,commonUV_E,&UVSetNames[u]);
						break;
					}

				}


			}
			else
			{
				endHasNoUVs = true;
			}


			if(endHasNoUVs && startHasNoUVs)
				continue;	//in dem Fall nix hinzufügen


			//jetzt einfach noch die UVPos holen und hinzufügen, es muss immer die reihenfolge: startUVs - endUVs haben
			//wenn eine der UVs nicht vorhanden ist, wird der UVIndex im Array -1 gesetzt
			
			float2 commonPos_S,startPos;
			float2 commonPos_E,endPos, endPos2;

			if(!startHasNoUVs)
			{
				

				meshFn.getUV(startUV,startPos[0],startPos[1],&(UVSetNames[u]));
				meshFn.getUV(commonUV_S,commonPos_S[0],commonPos_S[1],&(UVSetNames[u]));

			}

			if(!endHasNoUVs)
			{

				meshFn.getUV(endUV,endPos[0],endPos[1],&(UVSetNames[u]));
				meshFn.getUV(commonUV_E,commonPos_E[0],commonPos_E[1],&(UVSetNames[u]));
				
				if(endVerts[1] > -1)
				{
					float2 finalEnd;

					meshFn.getUV(endUV2,endPos2[0],endPos2[1],&(UVSetNames[u]));
					finalEnd[0] = (endPos[0] + endPos2[0]) / 2;
					finalEnd[1] = (endPos[1] + endPos2[1]) / 2;

					endPos[0] = finalEnd[0];
					endPos[1] = finalEnd[1];
				}

			}

			if(!isFlipped)	//die Daten müssen standardMässig umgekehrt werden, damit's funzt
			{//die Punkte entsprechend umdrehen
				float2 tmp;

				tmp[0] = commonPos_S[0];
				tmp[1] = commonPos_S[1];
				
				commonPos_S[0] = commonPos_E[0];
				commonPos_S[1] = commonPos_E[1];

				commonPos_E[0] = tmp[0];
				commonPos_E[1] = tmp[1];

				tmp[0] = startPos[0];
				tmp[1] = startPos[1];

				startPos[0] = endPos[0];
				startPos[1] = endPos[1];

				endPos[0] = tmp[0];
				endPos[1] = tmp[1];

			}

			//nun noch die Daten hinzufügen

			if(!startHasNoUVs)
			{
				// UVPositionen Arrays hinzufügen
				(*iterSlideIDs).append(commonUV_S);
				(*iterSlideStart).append(MFloatVector(startPos[0],startPos[1],0.0));
				(*iterSlideDirs).append(MFloatVector(commonPos_S[0] - startPos[0],commonPos_S[1] - startPos[1],0.0));

			}
			else
			{
				(*iterSlideIDs).append(-1);
				(*iterSlideStart).append(MFloatVector());
				(*iterSlideDirs).append(MFloatVector());
			}


			if(!endHasNoUVs)
			{
				// UVPositionen Arrays hinzufügen
				(*iterSlideIDs).append(commonUV_E);
				(*iterSlideStart).append(MFloatVector(endPos[0],endPos[1],0.0));
				(*iterSlideDirs).append(MFloatVector(commonPos_E[0] - endPos[0],commonPos_E[1] - endPos[1],0.0));

			}
			else
			{
				(*iterSlideIDs).append(-1);
				(*iterSlideStart).append(MFloatVector());
				(*iterSlideDirs).append(MFloatVector());
			}




		}

	iterSlideIDs++;
	iterSlideStart++;
	iterSlideDirs++;

	}//for(unsigned int u = 0; u < UVSetNames.length(); u++) ENDE


}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::SMC_createUVSlide(int commonVtx, int startVtx, int endVtx, MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	//für Control2 Slides (bei denen die Vertizen nicht über edges verbunden sind, dann SMC_createUVSlideType2 verwenden
	int tmp;

	INVIS(cout<<endl<<endl;);
	MPRINT("---------------------------");
	MPRINT("CREATE SMC UV SLIDE: BEGIN");
	MPRINT("---------------------------");
	INVIS(cout<<"ARBEITE MIT (START/COMMON/END) VTX: "<<startVtx<<", "<<commonVtx<<", "<<endVtx<<endl<<endl;);

	MIntArray commonEdges, otherEdges;

	MIntArray startFaces;	//enthält die Faces links und rechts von der startEdge
	MIntArray endFaces;		//enthält die Faces links und rechts von der endEdge

	//zuersteinmal die KantenIDs holen
	vertIter.setIndex(commonVtx,tmp);
	vertIter.getConnectedEdges(commonEdges);

	vertIter.setIndex(startVtx,tmp);
	vertIter.getConnectedEdges(otherEdges);


	int startEdge = helper.matchArray(commonEdges,otherEdges)[0];

	//endEdgeID holen
	vertIter.setIndex(endVtx,tmp);
	vertIter.getConnectedEdges(otherEdges);

	int endEdge = helper.matchArray(commonEdges,otherEdges)[0];

	INVIS(cout<<"END EDGE == "<<endEdge<<endl;);
	INVIS(cout<<"Start EDGE == "<<startEdge<<endl<<endl;);

	//jetzt die Faces holen
	MItMeshEdge edgeIter(fMesh);

	edgeIter.setIndex(startEdge,tmp);
	edgeIter.getConnectedFaces(startFaces);
	
	edgeIter.setIndex(endEdge,tmp);
	edgeIter.getConnectedFaces(endFaces);

	//debug
	INVIS(if( (startFaces.length() != endFaces.length()) ){\
		cout<<"ERROR IN SMCCREATEUV: STARTFACES NICHT SO GROss WIE ENDFACES"<<endl;\
		return;\
		}\
		);
	
	//wenn im FaceArray nur 1 Face ist, dann kann man sich alles folgende sparen (goto funzt hier leideer nicht, weil er übr übersprungene initialisierungen meckert)
	unsigned int i = 0;
	unsigned int l = 0;
	/*if(startFaces.length() > 1)
	{
		//nun die Faces herausfinden, die in einer richtung übereinander liegen und endFacesArray neu ordnen
		MIntArray startFaceVtx;
		MIntArray otherFaceVtx;

		polyIter.setIndex(startFaces[0],tmp);
		polyIter.getVertices(startFaceVtx);
		MPoint startFaceCenter = polyIter.center();
		MDoubleArray	distances;	//hält die entfernungen der endFaces zum startFace;

		int faceMatchLocID = -1; //enthält die ID des Faces, welches mindestens 2 vertizen teilt mit startFace, relativ zum endFaces array
		l = endFaces.length();
		for(i = 0; i < l; i++)
		{
			polyIter.setIndex(endFaces[i],tmp);
			polyIter.getVertices(otherFaceVtx);

			distances.append(startFaceCenter.distanceTo(polyIter.center()));

			if(matchArray(startFaceVtx,otherFaceVtx).length() > 1)
			{
				faceMatchLocID = i;
				break;
			}
		}

		//Für den Fall, dass die Faces auch dreiecke beinhalteten muss eine geometrische erkennung vrwendet werden
		if(faceMatchLocID == -1)
		{
			
			MString str = "CreateUVSlide: Reference faces had unexpected topology. Aborted the operation for the following vertices \n";
			str += startVtx ;
			str += ", "; 
			str += commonVtx ;
			str += ", " ;
			str += endVtx ;
			MGlobal::displayError(str);
			//return;
			

			if(distances[0] > distances[1])
			{//endFacesArray flippen
				MPRINT("HABE ENDFACES UMGETAUSCHT, erkannt durch geometrie methode")
					tmp = endFaces[0];
				endFaces[0] = endFaces[1];
				endFaces[1] = tmp;
			}
		}
		else
		{//endFacesArray flippen wenn noetig
			if(faceMatchLocID == 1)
			{
				MPRINT("HABE ENDFACES UMGETAUSCHT")
					tmp = endFaces[0];
				endFaces[0] = endFaces[1];
				endFaces[1] = tmp;
			}
		}

	}
	
	*/
	//iteratoren für die UVSlideArrays initialisieren
	std::list<MIntArray>::iterator iterSlideIDs = UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();


	MFnMesh meshFn(fMesh);

	for(unsigned int u = 0; u < UVSetNames.length(); u++)
	{
		
		//nun überprüfen, ob die UVs dieser Faces horizontal nicht zusammenhängen(pro Richtung, pro UVSet).
		//wenn dies so ist, dann abbrechen, da UVSliding dann nichts bringen kannW


		l = startFaces.length(); //obwohl start/endFaces eigentlich immer die länge 2 haben müssten, mache ich es mal lieber so (-->sicherer)
		for(i = 0; i < l; i++)
		{		
			//checken, ob mindestens 1 UV gemein ist (zwar wird das ergebnis nicht so grandios sein, enn es wirklich nur eimgemeinsamer UV ist, allerdings
			//hat es jemand der so ein komisches UVSet macht es auch nicht besser verdient ;)

			//zuerst mal die lokale vertex ID zu den Vertizen finden
			int commonUV_S = -1,commonUV_E = -1,startUV = -1,endUV = -1;	//_E end Teil des commonUV, _S startTeil, dienen auch als Flags
			
			//flags:
			bool startFaceHasUVs = true, endFaceHasUVs = true;

			//locStart und CommonID_s holen,
			MIntArray faceVtx;

			polyIter.setIndex(startFaces[i],tmp);
			polyIter.getVertices(faceVtx);

			if(polyIter.hasUVs(UVSetNames[u]))
			{
				//commonUV holen
				int x;
				for(x = 0; x < faceVtx.length(); x++)
				{
					if(faceVtx[x] == commonVtx)
					{
						polyIter.getUVIndex(x,tmp,&(UVSetNames[u]));
						commonUV_S = tmp;
						break;
					}
				}

				//startUV holen
				for(x = 0; x < faceVtx.length(); x++)
				{
					if(faceVtx[x] == startVtx)
					{
						polyIter.getUVIndex(x,tmp,&(UVSetNames[u]));
						startUV = tmp;
						break;
					}
				}
			}
			else
			{
				startFaceHasUVs = false;
			}
			
			//localEnd und commonUV holen
			polyIter.setIndex(endFaces[i],tmp);
			polyIter.getVertices(faceVtx);

			if(polyIter.hasUVs(UVSetNames[u]))
			{
				//commonUV holen
				int x;
				for(x = 0; x < faceVtx.length(); x++)
				{
					if(faceVtx[x] == commonVtx)
					{
						polyIter.getUVIndex(x,tmp,&(UVSetNames[u]));
						commonUV_E = tmp;
						break;
					}
				}

				//startUV holen
				for(x = 0; x < faceVtx.length(); x++)
				{
					if(faceVtx[x] == endVtx)
					{
						polyIter.getUVIndex(x,tmp,&(UVSetNames[u]));
						endUV = tmp;
						break;
					}
				}
			}
			else
			{
				endFaceHasUVs = false;
			}

			//debug: darf nimals vorkommen
			INVIS(if(startUV == -1 || endUV == -1){cout<<"ERROR IN SMC_GETUV: START/ENDUV WAREN FALSCH (-1), ABBRUCH"<<endl;return;});

			INVIS(cout<<"START/COMMON/END UVS SIND: "<<startUV<<", "<<commonUV_S<<", "<<endUV<<endl<<endl;);

			/*
			//Ergebnisse überprüfen:
			//Nur wenn commonVtx gleich sind, darf fortgefahren werden
			if(commonUV_S != commonUV_E)
			{
				MString str = "Invalid UV SlidePath for along the following vertices: \n";
				str += startVtx;str += ", ";  str += commonVtx;str += ", "; str += endVtx; str += ".\n ";
				str += "in UVSet <" + UVSetNames[u] + "> \n";
				str += "The UVs are separated, and therefore invalid for proper UVSliding.";
				MGlobal::displayError(str);
				continue;
			}
			*/

			//ergebnis scheint richtig zu sein, also jetzt noch die UVPositionen auslesen, directions errechnen und ergebnisse
			//zu UVSlideArrays hinzufügen
			
			float2 commonPos_S,commonPos_E,startPos,endPos;
			
			if(startFaceHasUVs)
			{
				meshFn.getUV(startUV,startPos[0],startPos[1],&(UVSetNames[u]));
				meshFn.getUV(commonUV_S,commonPos_S[0],commonPos_S[1],&(UVSetNames[u]));
			}

			if(endFaceHasUVs)
			{
				meshFn.getUV(endUV,endPos[0],endPos[1],&(UVSetNames[u]));
				meshFn.getUV(commonUV_E,commonPos_E[0],commonPos_E[1],&(UVSetNames[u]));
			}
			
			//alles wird in 2er paketen eingelagert: ein Paket (startPos, Direction, SlideIndex) pro direction
			//wenn eine direction nicht gefüllt ist, dann slideIndex -1 setzen (= flag, der überprüft wird)
			//wenn beide directions nicht vorhanden, dann gar nichts hinzufügen
			//-->Merke!!: letzteres koennte zu fehlern führen, da nicht mehr überprüft wird, ob das start und endFace tatsächlich übereinanderliegen
			//unter bestimmten umständen kann es dannzu fehlverhalten kommen!!

			if(!startFaceHasUVs && !endFaceHasUVs)
				continue;

			//slideArrays erweitern
			if(startFaceHasUVs)
			{
				(*iterSlideIDs).append(commonUV_S);
				(*iterSlideStart).append(MFloatVector(startPos[0],startPos[1],0.0));
				//(*iterSlideEnd).append(MFloatVector(endPos[0],endPos[1],0.0));
				(*iterSlideDirs).append(MFloatVector(commonPos_S[0] - startPos[0],commonPos_S[1] - startPos[1],0.0));
			}
			else
			{
				(*iterSlideIDs).append(-1);
				(*iterSlideDirs).append(MFloatVector());
				(*iterSlideStart).append(MFloatVector());
			}

			if(endFaceHasUVs)
			{
				(*iterSlideIDs).append(commonUV_E);
				(*iterSlideStart).append(MFloatVector(endPos[0],endPos[1],0.0));
				//(*iterSlideEnd).append(MFloatVector(endPos[0],endPos[1],0.0));
				(*iterSlideDirs).append(MFloatVector(commonPos_E[0] - endPos[0],commonPos_E[1] - endPos[1],0.0));
			}
			else
			{
				(*iterSlideIDs).append(-1);
				(*iterSlideDirs).append(MFloatVector());
				(*iterSlideStart).append(MFloatVector());
			}

			//(*iterSlideDirs).append(MFloatVector(commonPos[0] - startPos[0],commonPos[1] - startPos[1],0.0));
			//(*iterSlideDirs2).append(MFloatVector(endPos[0] - commonPos[0],endPos[1] - commonPos[1],0.0));
			
			//cout<<"Direction 1 = "<<MFloatVector(commonPos[0] - startPos[0],commonPos[1] - startPos[1],0.0)<<endl;
			//cout<<"Direction 2 = "<<MFloatVector(endPos[0] - commonPos[0],endPos[1] - commonPos[1],0.0)<<endl;

			//cout<<"Start = "<<MFloatVector(startPos[0],startPos[1],0.0)<<endl;
			//cout<<"END = "<<MFloatVector(endPos[0],endPos[1] ,0.0)<<endl;

		}

		iterSlideIDs++;
		iterSlideStart++;
		iterSlideDirs++;

	}


}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::processUncontrolledLines(MIntArray& linedVtx, 
											simpleLinkedList<MIntArray>& connectionsList,
											MIntArray& countArray,
											MItMeshVertex& vertIter,
											MItMeshPolygon& polyIter)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{//wenna hier hinkommt sind nur noch normaleTypeTwo und type One Vtx 
	MIntArray	vtxRemovals;	//vtxRemovals: bei übereinstimmung werden die IDs -1 gesetzt
	int			currentVtx,lastVtx,firstVtx;
	int			dVtx,origDVtx;		//directionalVtx
	int			tmp,index;

	int			counter = 0;
	
	bool		go = true;
	

	MPoint		startPos, endPos, currentPos;
	MVector		normal;
	double		scale = 0.0;
	bool		wasStopped = false;

	MIntArray		currentFaces,directionFaces;
	MVectorArray	normals, directions;
	MPointArray		startEnd;
	MDoubleArray	normalScales;
	
	INVIS(cout<<"LinedVtxLength vs ConnectionsListLength = "<<linedVtx.length()<<" vs "<<connectionsList.length()<<endl;)

	//BEGIN
	for(unsigned int i = 0; i < linedVtx.length(); i++)
	{
		if(linedVtx[i] != -1)
		{

			//niemals bei einsern anfangen
			if(countArray[i] == 1)
			{
				MPRINT("Einser war am Anfang, nächste Iterations gestartet")
				//linedVtx[i] = -1;
				continue;
			}

			wasStopped = false;

			firstVtx = linedVtx[i];
			INVIS(cout<<"FirstVtx ist"<<firstVtx<<endl;)


			int l = connectionsList[i]->length();
			//direction herausfinden
			int x;			
			for(x = 0; x < l; x++)
			{
				if( (*connectionsList[i])[x] >= 0)
				{
					//SicherheitsCheck
					if((*connectionsList[i])[(x+1)%l] < 0)
					{
						origDVtx = -((*connectionsList[i])[(x+1)%l] + 1);
					}
					else
					{
						MPRINT("Unerwarteter wert in getDirection, weiche aus.")
						
						for(int c = x+2; ; c++)
						{
							if((*connectionsList[i])[c%l]<0)
							{
								origDVtx = -((*connectionsList[i])[c%l] + 1);
								break;
							}
						}
					}

					break;
				}
			}



			counter = 0; //zählt die Verts des gesamten Passes (beide richtungen))

			for(int a = 0; a < 2; a++)
			{

				l = connectionsList[i]->length();

				if(a == 0)
				{//seite wählen
					for(x = 0; x < l; x++)
					{
						if( (*connectionsList[i])[x] >= 0)
						{
							MPRINT("Starte Iteration mit Seite 1")
							lastVtx = (*connectionsList[i])[x];
							break;
						}
					}
				}
				else
				{
					//diesmal von der anderen Seite rangehen, um den anderen Vtx zu finden
					for(x = l-1; x >= 0; x--)
					{
						if( (*connectionsList[i])[x] >= 0)
						{
							MPRINT("Starte Iteration mit Seite 2")
							lastVtx = (*connectionsList[i])[x];
							break;
						}
					}
				}

				dVtx = origDVtx;
				currentVtx = firstVtx;
				go = true;
				index = i;
				


			//	for(int t = 0; t < 3; t++)
				while(go)
				{
					startEnd.clear();
					normals.clear();
					normalScales.clear();
					directions.clear();
					//erstmal gegenwärtigen Vtx eintragen und mit slide versehen
					vtxRemovals.append(currentVtx);

					//debug:
					INVIS(cout<<"-------------------------------------------"<<endl;)
					INVIS(cout<<"CurrentVtx ist : "<<currentVtx<<endl;)
					INVIS(cout<<"LastVtx ist : "<<lastVtx<<endl;)
					INVIS(cout<<"dVtx ist : "<<dVtx<<endl;)
					INVIS(cout<<"-------------------------------------------"<<endl;)

					vertIter.setIndex(currentVtx,tmp);
					vertIter.getConnectedFaces(currentFaces);

					currentPos = vertIter.position();

					//jetzt die Position des StartVtx holen
					vertIter.setIndex(dVtx,tmp);
					vertIter.getConnectedFaces(directionFaces);
					
					startPos = vertIter.position();

					SMC_createNormal(currentFaces,directionFaces, normal,scale,polyIter);	//normale/scale wird in der methode 0 gesetzt
					normals.append(normal);
					normalScales.append(scale);

					directions.append(currentPos - startPos);
					//jetzt die Position des endVtx holen
					//index = SMC_getIndex(currentVtx,linedVtx); //index ist noch vom letzten durchlauf gesetzt
					
					int endVtx;
					l = connectionsList[index]->length();
					for(x = 0; x < l;x++)
					{
						int dVtxConverted = -(dVtx+1);
						if( (*connectionsList[index])[x] == dVtxConverted )
						{//bei quadMeshes ist die sache klar, bei anderen nicht so
							
							if( (*connectionsList[index])[(x+2)%l] < 0)
							{
								endVtx = -((*connectionsList[index])[(x+2)%l] + 1);
								vertIter.setIndex( endVtx,tmp);
							}
							else
							{
								MPRINT("PROCESS UNCONTROLLED LINED VTX: Unerwarteter Wert bei endVtx, starte erweiterte Suche")
								for(int c = x+3; ;c++)
								{
									if((*connectionsList[index])[c%l] < 0 && (*connectionsList[index])[c%l] != dVtxConverted )
									{
										endVtx = -((*connectionsList[index])[c%l] + 1);
										vertIter.setIndex( endVtx,tmp);
										break;
									}	
								}
							}

							break;
						}
					}

					//jetzt ist endVtx definitiv gesetzt
					endPos = vertIter.position();

					vertIter.getConnectedFaces(directionFaces);
					
					SMC_createNormal(currentFaces,directionFaces,normal,scale,polyIter);
					
					normals.append(normal);
					normalScales.append(scale);
					
					directions.append(endPos - currentPos);

					startEnd.append(startPos);
					startEnd.append(endPos);

					//slide erstellen 
					addSlideAndNormal(currentVtx,startEnd,directions,normals,normalScales); //momentan wird der firstVtx doppelt gemoppelt
					counter++;



					//jetzt noch UVs hinzufügen
					SMC_createUVSlide(currentVtx,endVtx,dVtx,vertIter,polyIter);



					//-------------DataGather End---------------------------------
					//jetzt den nächsten Vtx finden

					if(countArray[index] > 1)
					{
						//durch connectionsArray parsen und Vtx finden, der nicht lastVtx ist
						//printArray((*connectionsList[index])," = Connetions bei GetNewCurrent");

						for(x = 0; x < l; x++)
						{
							if( (*connectionsList[index])[x] >= 0 && (*connectionsList[index])[x] != lastVtx )
							{
								lastVtx = currentVtx;
								currentVtx = (*connectionsList[index])[x];
								break;
							}
						}

						//LoopCheck:
						if(currentVtx == firstVtx)
						{
							MPRINT("CurrentVtx war firstVtx")
							go = false;
							continue;
						}

						//index aktualisieren
						index = SMC_getIndex(currentVtx,linedVtx);

						if(index == -1)
						{
							MPRINT("Bin an Type3 || Type4 gestossen, beende Loop")
							
							wasStopped = true;
							go = false;
							continue;
						}

						//Okay, jetzt noch die neue direction bestimmen
						
						vertIter.setIndex(dVtx,tmp);
						vertIter.getConnectedFaces(currentFaces);
						
						dVtx = SMC_getDirection(currentFaces,startPos,connectionsList[index],vertIter);
						//FERTIG

					}
					else
					{
						go = false;
						continue;
					}
		
				}
				//WHILE ENDE
				
				
			}
			//FOR(RICHTUNG) ENDE

			

			//zum schluss vtxRemovals markieren in linedVtx
			MPRINT("Will vtx entfernen")
			l = vtxRemovals.length();
			for(x = 0; x < l; x++)
			{
				for(unsigned int k = 0; k < linedVtx.length(); k++)
				{
					if(vtxRemovals[x] == linedVtx[k])
					{
						linedVtx[k] = -1;
					}
				}
			}

			if(wasStopped)
			{//sämtliche Slides entfernen
				SMC_deleteLastXSlideValues(counter);
			}

			vtxRemovals.clear();
		
		}
	}


}
//-------------------------------------------------------------------------------------------
int			BPTfty::SMC_getTypeTwoControlDirection(int vtxID,
												   int nextVtxLocID,
												   int& faceID,
												   MPoint& startPoint,
												   MIntArray& selectedVtx, 
												   MItMeshVertex& vertIter, 
												   MItMeshPolygon& polyIter,
												   bool	getStartPoint)
//-------------------------------------------------------------------------------------------
{//gibt die ontroll direction für das entsprechende Type2 face zurück


	MIntArray currentFaces,faceVerts, match;
	
	int tmp;



	vertIter.setIndex(vtxID,tmp);
	vertIter.getConnectedFaces(currentFaces);

	UINT x;
	for(x = 0; x < currentFaces.length(); x++)
	{
		polyIter.setIndex(currentFaces[x],tmp);
		polyIter.getVertices(faceVerts);
		
		
		match =  helper.matchArray(selectedVtx,faceVerts);
		if(match.length() == 2)
		{
			faceID = currentFaces[x];
			break;
		}
	}
	

	//jetzt noch den StartVtx/startDirection herausfinden
	if(faceVerts.length() == 3)
	{
		MPRINT("FaceVertsSize in GETControlFaceDirection war 3")

		int validFaceID;
		MIntArray connectedFaces;
		polyIter.setIndex(currentFaces[x],tmp);

		polyIter.getConnectedFaces(connectedFaces);
		
		connectedFaces.append(currentFaces[x]);

		helper.arrayRemove(connectedFaces,currentFaces);

		UINT i;
		for(i = 0; i < connectedFaces.length(); i++)
		{
			MIntArray faceVertices;
			polyIter.setIndex(connectedFaces[i],tmp);
			
			if(!polyIter.isConnectedToVertex(vtxID) )
			{
				validFaceID = connectedFaces[i];
				break;
			}
		}

		//jetzt einfach nextVtx finden
		polyIter.getVertices(faceVerts);
		unsigned int l = faceVerts.length();
		for(i = 0; i < l;i++)
		{
			if(faceVerts[i] == selectedVtx[nextVtxLocID])
			{
				
				if(getStartPoint)
				{
					vertIter.setIndex(selectedVtx[nextVtxLocID],tmp);
					startPoint = vertIter.position();

					vertIter.setIndex(selectedVtx[1-nextVtxLocID],tmp);

					startPoint = startPoint + (vertIter.position() - startPoint);
				}

				if(faceVerts[(i+1)%l] == selectedVtx[1-nextVtxLocID])
				{
					return faceVerts[(i-1+l)%l];
				}
				else
				{
					return faceVerts[(i+1)%l];
				}


			}
		}
	
	}
	else
	{//nextVtxID suchen und dVtx zurückgeben
		

		int l = faceVerts.length();
		for(int i = 0; i < l; i++)
		{
			if(faceVerts[i] == selectedVtx[nextVtxLocID])
			{
				if(faceVerts[(i+1)%l] != vtxID)
				{
					if(getStartPoint)
					{
						vertIter.setIndex(faceVerts[(i+1)%l],tmp);
						startPoint = vertIter.position();
					}
					return faceVerts[(i+1)%l];
				}
				else
				{
					if(getStartPoint)
					{
						vertIter.setIndex(faceVerts[(i-1+l)%l],tmp);
						startPoint = vertIter.position();
					}
					return faceVerts[(i-1+l)%l];
				}
			}
		}
	}

	cout<<"GET_CONTROL_DIRECTION - forbidden path!!!"<<endl;
	return -1;

}

//--------------------------------------------------------------------------------------------------
MPoint		BPTfty::SMC_getTypTwoEndDirection(int vtxID, 
											  int startFaceID,
											  int& endFaceID,
											  MIntArray* connections, 
											  MItMeshPolygon& polyIter, 
											  MItMeshVertex& vertIter,
											  MIntArray* endVtxIDs)	
//--------------------------------------------------------------------------------------------------
{
	int tmp;
	MIntArray vertFaces,faceVerts;


	vertIter.setIndex(vtxID,tmp);
	vertIter.getConnectedFaces(vertFaces);

	int offset = 0;

	

	//jetzt offset erstellen
	UINT i;
	for(i = 0; i < vertFaces.length(); i++)
	{
		if( vertFaces[i] == startFaceID )
		{
			offset = i;
			vertFaces.remove(i);
			break;
		}
	}


	unsigned int l = vertFaces.length();
	int middleLocID = ( l - (l%2) ) / 2;

	int middleFace = vertFaces[ (offset + middleLocID)%l ];

	INVIS(cout<<"MIDDLEFACE von Vtx "<<vtxID<<" ist "<<middleFace<<endl;)

	endFaceID = middleFace;

	polyIter.setIndex(middleFace,tmp);
	polyIter.getVertices(faceVerts);

	

	l = faceVerts.length();
	if(l == 3)
	{//triangleMode
		//erstmal sämtliche unselVtx aus connections holen
		MIntArray unselVtx;
		l = connections->length();
		for(i = 0; i < l; i++)
		{
			if((*connections)[i] < 0)
			{
				unselVtx.append( -(*connections)[i]-1 );
			}
		}

	//	connections = 0; //damit er den Ptr nicht loeschen kann, falls er das macht ->machta nicht :)

		if(helper.matchArray(faceVerts,unselVtx).length() == 3)
		{//triangle liegt quer, so dass punkt auf Kante zurückgegeben wird
			//TmpArray erzeugen, damit man das als Referenz übergeben kann - LINUX only

			MIntArray tmpArray(1,vtxID);
			helper.arrayRemove(faceVerts,tmpArray );

			
			vertIter.setIndex(faceVerts[0],tmp);
			MPoint start = vertIter.position();
			
			vertIter.setIndex(faceVerts[1],tmp);
			MPoint end 	= vertIter.position();

			//endVtx übertragen wenn noetig
			if(endVtxIDs != 0)
			{
				(*endVtxIDs)[0] = faceVerts[0];
				(*endVtxIDs)[1] = faceVerts[1];
			}

			return start + ( 0.5 * (end - start));

		}
		else
		{//triangle liegt ordentlich, also einfach vtxID+2 zurückgeben
			l = faceVerts.length();
			
			for(i = 0; i < l; i++)
			{
				if(faceVerts[i] == vtxID)
				{
					vertIter.setIndex(faceVerts[ (i+2)%l ],tmp );

					//endVtx übertragen wenn noetig
					if(endVtxIDs != 0)
					{
						(*endVtxIDs)[0] = faceVerts[ (i+2)%l ];
						(*endVtxIDs)[1] = -1;	//flag, das diese position nicht besetzt ist
					}
					
					return vertIter.position();
				}
			}
		}

	}
	else
	{//jetzigen Vtx finden und DirectionalVtx zurückgeben
		int offset = (l - (l%2))/2 ;
		for(i = 0; i < l; i++)
		{
			if(faceVerts[i] == vtxID)
			{
				vertIter.setIndex(faceVerts[(i+offset)%l],tmp);

				//endVtx übertragen wenn noetig
				if(endVtxIDs != 0)
				{
					(*endVtxIDs)[0] = faceVerts[(i+offset)%l];
					(*endVtxIDs)[1] = -1;	//flag, das diese position nicht besetzt ist
				}

				return vertIter.position();
			}
		}

	}

	//forbiddenPath, absturz provoziern
	cout<<"SMC_GET_TYPE_TWO_END_VTX: FORBIDDEN PATH"<<endl;
	return MPoint(0.0,0.0,0.0);


}

//----------------------------------------------------------------------------------------------------------------------//----------------------------------------------------------------------------------------------------------------------
void		BPTfty::processLinedVtx(MIntArray startIDs,
									MIntArray& linedVtx,
									MIntArray& isControlTwo,
									simpleLinkedList<MIntArray>& connectionsList,
									MIntArray& countArray,
									MIntArray& isFlippedArray,
									MIntArray& bigVtxRemovals,
									MItMeshVertex& vertIter,
									MItMeshPolygon& polyIter)
//----------------------------------------------------------------------------------------------------------------------//----------------------------------------------------------------------------------------------------------------------
{//startID ist lokal	
	MPRINT("BIN IN PROCESS LINED VTX")
	
	MIntArray	vtxRemovals;	//vtxRemovals: bei übereinstimmung werden die IDs -1 gesetzt, nur lokal gültig/per pathSection
	int			currentVtx,lastVtx;
	int			dVtx;		//directionalVtx
	int			tmp,index,faceID;

	bool		isFlipped = false;
	int			countTmp = 0;

	MIntArray	counter;
	
	
	MIntArray	results;

	MPoint		startPos, endPos, currentPos;
	MVector		normal, tmpNormal;
	double		scale = 0.0;
	
	
	int			directionDoneCount = 0;	//counter für die richtungen, die bereits erledigt waren
	
	MVectorArray	normals, directions;
	MPointArray		startEnd;
	MDoubleArray	normalScales;

	MIntArray	selectedVtx;
	MIntArray newStartIDs;	//hält die neuen StartIds, die dann recursiv für loesung des problems sorgen
	

	

	//wenn startIDs gleich sind, einen entfernen
	if(startIDs.length() == 2)
		if(startIDs[0] == startIDs[1])
			startIDs.remove(0);

	INVIS(helper.printArray(startIDs," = startIDs");)

	//BEGIN
	for(unsigned int i = 0; i < startIDs.length(); i++)
	{

		counter.clear();
		results.clear();
		normals.clear(); 
		directions.clear();
		startEnd.clear();
		normalScales.clear();
		selectedVtx.clear();
		directionDoneCount = 0;
		//scale = 0.0;
		


		unsigned int l = connectionsList[startIDs[i]]->length();
		//direction herausfinden
		UINT x;
		for( x = 0; x < l; x++)
		{
			if( (*connectionsList[startIDs[i]])[x] >= 0)
			{//selected finden
				selectedVtx.append((*connectionsList[startIDs[i]])[x]);
			}
		}



		lastVtx = linedVtx[startIDs[i]];
		



		INVIS(cout<<"------------------------------------------------------"<<endl;)
		INVIS(cout<<"Bearbeite TWO CONTROL VTX mit ID   "<<lastVtx<<endl;)
		//checken, ob dieser Vtx geflipped ist
		isFlipped = false;

	

		for(x = 0; x < isFlippedArray.length(); x++)
		{
			if(isFlippedArray[x] == lastVtx)
			{
				MPRINT("Dieser ControlVtx ist flipped")
				isFlipped = true;
			}
		}

		


		INVIS(if(selectedVtx.length() != 2))
			INVIS(cout<<"FEHLER, SELECTEDVTX WAR "<<selectedVtx.length()<<endl;)
		
		


		for(int a = 0; a < 2; a++)
		{
			
			//wenn die gewünschte richtung nicht mehr im linedArray ist, weil sie schon bearbeitet wurde, abbruch
			index = SMC_getIndex(selectedVtx[a],linedVtx);

			if(index == -1)
			{
				MPRINT("DIESE DIRECTION WURDE BEREITS BEARBEITET")
				directionDoneCount++;
				continue;
			}
			

			
			currentVtx = selectedVtx[a];

			

			dVtx = SMC_getTypeTwoControlDirection(	linedVtx[startIDs[i]],
													a,
													faceID,
													startPos,
													selectedVtx,
													vertIter,
													polyIter,
													true);


			
			
			INVIS(cout<<"Gefundener dVtx ist "<<dVtx<<endl;)
			
			countTmp = 0;
	
			if(!isControlTwo[index])
			{MPRINT("WILL NORMALE LINE BEARBEITEN")
				//eigentlichen Path abarbeiten
				results.append(SMC_processLine(	currentVtx,
												lastVtx,
												dVtx,
												countTmp,
												isFlipped,
												vtxRemovals,
												linedVtx,
												isControlTwo,
												isFlippedArray,
												countArray,
												connectionsList,
												vertIter,
												polyIter ) );
				
				counter.append(countTmp);
			}
			else
			{//an diesen TwoVtx schliesst sich gleich wieder twoVtx an, SonderFall
				//wenn direction von currentVtx ein selectedVtx von der anderen Edge ist, dann ist die direction nicht flipped
				//der eigene status wird mit einbezogen
				MPRINT("CONTROLZWO GRENZT AN")

				bool	otherVtxFlipped;
				l = connectionsList[index]->length();

				if(isFlipped)
				{//wenn CVtx geflipped, dann sollte dVtx nicht selVtx von otherVtx sein
					
					otherVtxFlipped = false;
					
					for(x = 0; x < l; x++)
					{
						if((*connectionsList[index])[x] == dVtx)
						{MPRINT("Habe direction geflipped")
							otherVtxFlipped = true;
							break;
						}
					}

				}
				else
				{//wenn CVtx nicht geflipped, dann sollte dVtx selVtx von otherVtx sein
	
					otherVtxFlipped = true;

					for(x = 0; x < l; x++)
					{
						if((*connectionsList[index])[x] == dVtx)
						{
							otherVtxFlipped = false;
							break;
						}
					}
				}

				//da das geklärt ist, den TwoControlVtx noch aufs flippedArray hauen, falls noetig
				if(otherVtxFlipped)
				{MPRINT("Habe angrenzenden ControlTwo geflipped")
					isFlippedArray.append(linedVtx[index]);
				}

				results.append(index);
				
			}
				
			


		}
		//FOR(RICHTUNG) ENDE
	
		//bevor die Vtx entfernt werden, noch schnell den gegenwärtigen index vom vtx zu holen
		index = SMC_getIndex(lastVtx,linedVtx);

		int startVtx;	//wird benoetigt für die Erstellung des UVSlides
		//startPositionen holen, wenn beide Pfade bereits erledigt
		if(directionDoneCount == 2)
		{
			MPRINT("MUSSTE STARTPOS EXTRA HOLEN")
			startVtx = SMC_getTypeTwoControlDirection(	linedVtx[startIDs[i]],
											0,
											faceID,
											startPos,
											selectedVtx,
											vertIter,
											polyIter,
											true);
		}
		else
			startVtx = dVtx;

		//zum schluss vtxRemovals markieren in linedVtx
		MPRINT("Will vtx entfernen")
		
		vtxRemovals.append(linedVtx[startIDs[i]]);
		

		l = vtxRemovals.length();
		for(x = 0; x < l; x++)
		{
			for(unsigned int k = 0; k < linedVtx.length(); k++)
			{
				if(vtxRemovals[x] == linedVtx[k])
				{
					linedVtx[k] = -1;
				}
			}
		}
		


		helper.addIntArrayToLHS(bigVtxRemovals,vtxRemovals);	//vtxRemovals an aussenwelt weitergeben
		vtxRemovals.clear();

				//ReturnValues analysieren
		for( x = 0; x < results.length(); x++)
		{
			if(results[x] == -2)
			{	//slides entfernen
				MPRINT("Bearbeite Blockierten Path")
				if(x == 0)
					SMC_deleteLastXSlideValues(counter[0],counter[1]);
				else
					SMC_deleteLastXSlideValues(counter[0]);
			}
			else if(results[x] >= 0)
			{//anderer typeTwo vtx gefunden: auf array packen
				newStartIDs.append(results[x]);
			}
		}
		
			

		//jetzt die SlideDaten für dieses TwoFace erstellen
		//startPos ist vorhanden

		
		int endFaceID;

		MIntArray* endVertices = new MIntArray(2,-1);		//enthält 2 slots für die endVertices des control faces. der 2. Slot ist noetig für den Fall
											//dass rückgabePosition zwischen zwei verticen liegt (triangle Sonderfall)

		endPos = SMC_getTypTwoEndDirection(lastVtx,
											faceID,endFaceID,
											connectionsList[index],
											polyIter,
											vertIter,
											endVertices);

	
		vertIter.setIndex(lastVtx,tmp);
		currentPos = vertIter.position();

		//wenn flipped, startEnd vertauschen
		if(isFlipped)
		{MPRINT("Habe direction tatsächlich umgetauscht")
			MPoint tmpPos = startPos;
			startPos = endPos;
			endPos = tmpPos;
		}

		startEnd.append(startPos);
		startEnd.append(endPos);
		
		directions.append(currentPos - startPos);
		directions.append(endPos - currentPos);

		polyIter.setIndex(faceID,tmp);
		polyIter.getNormal(tmpNormal);
		polyIter.getArea(scale);

		normalScales.append(scale);
		normals.append(tmpNormal);



		polyIter.setIndex(endFaceID,tmp);
		polyIter.getNormal(tmpNormal);
		polyIter.getArea(scale);

		normalScales.append(scale);
		normals.append(tmpNormal);
		
		addSlideAndNormal(lastVtx,startEnd,directions,normals,normalScales);

		//UVSlide hinzufügen
		SMC_createControlTwoUVSlide(lastVtx,startVtx,*endVertices,vertIter,polyIter,isFlipped);
		
	
	}
	//for(controlTwoVtx) ENDE


	//jetzt in die rekursion eintauchen, wenn moeglich
	if(newStartIDs.length() > 0)
	{
		MPRINT("------------------------------------------------------")
		MPRINT("GEHE IN NEUE REKURSION")
		processLinedVtx(	newStartIDs,
							linedVtx,
							isControlTwo,
							connectionsList,
							countArray,
							isFlippedArray,
							bigVtxRemovals,
							vertIter,
							polyIter);

		INVIS(cout<<"HABE REKURSION BEENDET, ID "<<startIDs[0]<<endl;)
	}
	

	
	//ENDE
	INVIS(cout<<"ID "<<startIDs[0]<<" ENDE"<<endl;)
	

}

//----------------------------------------------------------------------------------------------------------------------
double		BPTfty::calculateArc(int commonVtxID, int firstEnd, int secondEnd, MItMeshVertex& vertIter)
//----------------------------------------------------------------------------------------------------------------------
{
	int tmp;

	vertIter.setIndex(commonVtxID,tmp);
	MPoint cp = vertIter.position();

	vertIter.setIndex(firstEnd,tmp);
	MPoint ep1 = vertIter.position();

	vertIter.setIndex(secondEnd,tmp);
	MPoint ep2 = vertIter.position();


	//winkel berechnen
	return	(ep1 - cp).angle(ep2 - cp);

}

//----------------------------------------------------------------------------------------------------------------------
void		BPTfty::processType3(int vtxID, MIntArray& connections, MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//----------------------------------------------------------------------------------------------------------------------
{
	MPoint startPos, endPos, currentPos;
	int startID = -1, endID = -1;
	MVector tmpNormal,normal;
	double scale = 0.0,tmpScale;

	int tmp;

	MVectorArray	directions,normals;
	MDoubleArray	normalScale;
	MPointArray		startEnd;

	
	//durch connections parsen und startVtx finden, in quadMesh dürfte es hier nur einen gültigen anfangspunkt finden
	unsigned int l = connections.length();
	
	//2 Fälle unterscheiden: entweder ist es ideales QuadMesh (nur 4 connections und 3 counts) oder nicht
	//wenn nicht, werden die start ud end poiints mathematisch bestimmt (dot)
	unsigned int i = 0;
	
	if(l > 4)
	{
		//zuerst eine gültige startID finden. StartID ist die, deren Winkel zu den anderen beiden selVtx am keinsten ist
		//da sie mit er groessten wahrscheinlichkeit in der echten "Mitte" liegt
		MIntArray localSelIDs;	//enthält die IDs der selIDs in reihenfolge
		MDoubleArray angles;		//enthält die winkel der entprechenden localSeID zu ihren nachbarn(Summe)

		for(;i < l;i++)
		{
			if(connections[i] > -1)
				localSelIDs.append(connections[i]);
		}

		angles.setLength(localSelIDs.length()); //length ist hier sowieso 3, aber was solls :)
		//jetzt die Winkel ausrechnen
		unsigned int l2 = localSelIDs.length();
		for(i = 0; i < l2; i++)
		{
			angles[i] += calculateArc(vtxID,localSelIDs[i],localSelIDs[(i+1)%l2],vertIter);
			angles[i] += calculateArc(vtxID,localSelIDs[i],localSelIDs[(i-1+l2)%l2],vertIter);
		
			INVIS(cout<<"GESAMTWINKEL bei Vtx "<<vtxID<<" mit Vtx "<<localSelIDs[i]<<" == "<<angles[i]<<endl;);
		}

		//jetzt localSelID als startID wählen, die den kleinesten anglewert hat,
		double smallestAngle = 1666666666;
		int smallestID = -1; 

		for(i = 0; i < l2; i++)
		{
			if(angles[i] < smallestAngle)
			{
				smallestAngle = angles[i];
				smallestID = i;
			}
		}

		startID = localSelIDs[smallestID];
		INVIS(cout<<"StartID für diesen Vtx ist: "<<startID<<endl;);


		//jetzt winkel mit allen anderen edges bilden und den groesstn auswählen
		//eigentlich müsste man hier den 2d angle nehmen ... 

		angles.setLength(l);
		for(i = 0; i < l; i++)
		{
			int tmp = connections[(i+1)%l];
			if(tmp < 0)
			{
				angles[i] = calculateArc(vtxID,startID,-tmp-1,vertIter);
			}
			else
			{
				angles[i] = calculateArc(vtxID,startID,tmp,vertIter);
			}
		}

		//jetzt den groessten winkel finden, dessen dazugehoeriger Vtx dann der endpoint ist
		double biggestAngle = 0.000000001;
		int biggestID = -1; 

		for(i = 0; i < l; i++)
		{
			INVIS(cout<<"Angle for Vtx = "<<connections[i]<<" == "<<biggestAngle<<endl;);
			if(angles[i] > biggestAngle)
			{
				biggestAngle = angles[i];
				biggestID = i;
			}
		}

		endID = (connections[(biggestID+1)%l] < 0)?-(connections[(biggestID+1)%l]+1):connections[(biggestID+1)%l];
		INVIS(cout<<"EndID für diesen Vtx ist: "<<endID<<endl;);

		//positionen holen - namen müssen aber vertauscht sein (warum auch immer)
		vertIter.setIndex(startID,tmp);
		endPos = vertIter.position();
		
		vertIter.setIndex(endID,tmp);
		startPos = vertIter.position();

		//IDs jetzt auch noch vertauschen
		int tmp = startID;
		startID = endID;
		endID = tmp;

	}
	else
	{
		for(; i < l;i++)
		{
			if(connections[i] < 0)
			{
				vertIter.setIndex(-(connections[i]+1),tmp);
				startPos = vertIter.position();
				startID = -(connections[i]+1);

				if(connections[(i+2)%l] >= 0 )
				{
					vertIter.setIndex(connections[(i+2)%l],tmp);
					endPos = vertIter.position();
					endID = connections[(i+2)%l];
				}
				else if(connections[(i-2+l)%l] >= 0)
				{
					vertIter.setIndex(connections[(i-2+l)%l],tmp);
					endPos = vertIter.position();
					endID = connections[(i-2+l)%l];
				}
				else
				{//wenna bis hier kommt, hat der vtx ziemlích viele edges, also nächstbesten Vtx finden
					INVIS(cout<<"Bin in Type3SpecialCase, id ist "<<vtxID<<endl;)
						for(int a = i+1; ; a++)
						{
							if(connections[a%l] >= 0)
							{
								vertIter.setIndex(connections[a%l],tmp);
								endPos = vertIter.position();
								endID = connections[a%l];

								break;
							}
						}	
				}

				break;

			}
		}
	}

	//directions erstellen
	vertIter.setIndex(vtxID,tmp);	
	currentPos = vertIter.position();

	//directions umkehren
	MPoint tmpPos;
	tmpPos = startPos;
	startPos = endPos;
	endPos = tmpPos;

	directions.append(currentPos - startPos);
	directions.append(endPos - currentPos);

	//pointArray erstellen
	startEnd.append(startPos);
	startEnd.append(endPos);

	//normalen erstellen
	MIntArray currentFaces, otherFaces,match;
	
	
	vertIter.getConnectedFaces(currentFaces);


	vertIter.setIndex(startID,tmp);
	vertIter.getConnectedFaces(otherFaces);

 	match = helper.matchArray(currentFaces,otherFaces);

	//jetzt die normalen sammeln
	l = match.length();
	for(i = 0; i < l; i++)
	{
		polyIter.setIndex(match[i],tmp);
		polyIter.getNormal(tmpNormal);

		polyIter.getArea(tmpScale);
		
		scale += tmpScale;
		normal += tmpNormal;
	}

	scale /= l;
	normal /= l;
	normal.normalize();

	normalScale.append(scale);
	normals.append(normal);


	
	scale = 0.0;
	normal = MVector(0.0,0.0,0.0);

	//normalen vom anderen face holen
	vertIter.setIndex(endID,tmp);
	vertIter.getConnectedFaces(otherFaces);

	match = helper.matchArray(currentFaces,otherFaces);

	l = match.length();
	for(i = 0; i < l; i++)
	{
		polyIter.setIndex(match[i],tmp);
		polyIter.getNormal(tmpNormal);
		
		polyIter.getArea(tmpScale);
		
		scale += tmpScale;
		normal += tmpNormal;
	}

	scale /= l;
	normal /= l; 
	normal.normalize();

	
	normalScale.append(scale);
	normals.append(normal);

	//jetzt endgültig slide hinzufügen
	addSlideAndNormal(vtxID,startEnd,directions,normals,normalScale);

	//zuguterletzt UVSlide hinzufügen (und zwar auf jeden Fall)
	SMC_createUVSlide(vtxID,startID,endID,vertIter,polyIter);


}

//---------------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::processType4Vertex(int vtxID,MIntArray& connections,MIntArray& selVtx,MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//---------------------------------------------------------------------------------------------------------------------------------------------
{
	//checken, ob irgendwo ein face existiert, bei dem nicht alle faceVtx auch zu selVtx gehoeren
	int tmp,startFaceID = -1,startVtx;
	double scale = 0.0;
	bool	isSpecialCase = false;

	

	vertIter.setIndex(vtxID,tmp);

	MIntArray faces,vertices,match;
	vertIter.getConnectedFaces(faces);

	

	
	

	for(unsigned int i = 0; i < faces.length(); i++)
	{
		polyIter.setIndex(faces[i],tmp);
		
					
		polyIter.getVertices(vertices);
		
		//jetzt matchen mit selVtx
		match = helper.matchArray(vertices,selVtx);

		if(match.length() != vertices.length())
		{
			INVIS(cout<<"Habe specialCase bei Type4 gefunden, ID ist "<<vtxID<<endl;)

			startFaceID = i;

			isSpecialCase = true;

			//jetzt den startVtx finden
			helper.arrayRemove(vertices,match);

			startVtx = vertices[0];		//dies hier wird zu unerwarteten resultaten führen, wenn face nGon ist

			break;
		}

	}

	if(isSpecialCase)
	{
		MVectorArray	normals,directions;
		MDoubleArray	normalScaleFactors;
		MPoint startPos, endPos, currentPos = vertIter.position();
		MPointArray startEnd;
		MVector tmpVec;

		polyIter.getNormal(tmpVec);
		polyIter.getArea(scale);

		normalScaleFactors.append(scale);
		tmpVec.normalize();
		normals.append(tmpVec);

		vertIter.setIndex(startVtx,tmp);
		startPos = vertIter.position();

		
		
		//Face 2 bearbeiten
		
		//bei standardQuadMesh ist das gesuchte Face 2 faces weiter, bzw. die hälfte der facesLänge weiter
		tmp = (faces.length() - (faces.length() % 2)) / 2;
		int endFace = faces[(startFaceID + tmp)%faces.length()];
		
	//	printArray(faces, " = faces Type4Special");
	//	cout<<"endFace == "<<endFace<<endl;
	//	cout<<"StartFace == "<<faces[startFaceID]<<endl;
		
		polyIter.setIndex(endFace,tmp);

		polyIter.getNormal(tmpVec);
		polyIter.getArea(scale);

		normalScaleFactors.append(scale);
		tmpVec.normalize();
		normals.append(tmpVec);

		//jetzt den endVtx finden, directions und slide erstellen

		polyIter.getVertices(vertices);
		
		connections.append(vtxID);	//damit das funzen kann
		
		match = helper.matchArray(vertices,connections);

		helper.arrayRemove(vertices,match);

		//wenn alles richtig war, dann ist vertices mindestens eins gross
		if(vertices.length() > 0)
		{
			vertIter.setIndex(vertices[0],tmp);

			endPos = vertIter.position();
		}
		else
		{
			MPRINT("Fehler in ProcessType4, vertices hat unerwartete groesse, weiche aus")
			//einfach einen matchVtx als endVtx nehmen, obwohl das dann zu unerwarteten resultaten führen kann

			endPos = polyIter.center();
		}

		//direction erstellen
		MPoint tmpPos = startPos;
		startPos = endPos;
		endPos = tmpPos;

		directions.append(currentPos - startPos);
		directions.append( endPos - currentPos );

		startEnd.append(startPos);
		startEnd.append(endPos);
		


		addSlideAndNormal(vtxID,startEnd,directions,normals,normalScaleFactors);

	}
	else
	{
		addNormalSlide(vtxID,vertIter,polyIter);
	}



}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool		BPTfty::getVtxConnectInfo(int vtxID,MIntArray& connections, unsigned int& connectCount, MIntArray& selVtx, MItMeshVertex& vertIter, MItMeshEdge& edgeIter)
//------------------------------------------------------------------------------------------------------------------------------------
{
	//speichert im connectionsArray per edge die ID des selVtx, wenn edge keinen selVtx teilt, dann ist ID -1,
	//connectCount ist zahl der selVtx, die mit ihm verbunden sind

	int tmp;
	MIntArray connectedEdges;

	connectCount = 0;


	vertIter.setIndex(vtxID,tmp);
	

	
	vertIter.getConnectedEdges(connectedEdges);



	//Zahl der connectedVtx finden und in connectionsArray eintragen
	int l = connectedEdges.length();
	connections.clear();
	connections.setLength(l);

	int validVtx;
	MIntArray match;
	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(connectedEdges[i],tmp);

		if(edgeIter.index(0) == vtxID)
			validVtx = edgeIter.index(1);
		else
			validVtx = edgeIter.index(0);

		match = helper.matchArray(selVtx,MIntArray(1,validVtx));
		
		if(match.length() > 0)
		{
			connections[i] = validVtx;
			connectCount++;
		}
		else
		{
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			connections[i] = -(validVtx+1);	//auf diese Weise wird zugriff auch auf unselected Vertex ermoeglicht
											//+1, damit die 0 nur für selected vtx steht und eindeutig ist
			//merke: connections müssen zurückgerechnet werden, wenn man auf unselected zugreifen will!!!
		}
	}
	
	if(vertIter.onBoundary())
		return true;

	return false;
	//fertig
	
}

//--------------------------------------------------------------------------------------------
void	BPTfty::convertAllToFaces(MIntArray& polyIDs,MIntArray& vertIDs,MIntArray& edgeIDs)
//--------------------------------------------------------------------------------------------
{
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
//-----------------------------------------------------------------
void  BPTfty::getAllFaceIndices(MIntArray& edgeIDs, MIntArray& faceIDs)
//-----------------------------------------------------------------
{
	//diese Procedur wählt entweder contained Faces an oder connectedFaces, je nachdem, ob mehr containedFaces oder connectedFaces vorhanden
	MItMeshEdge		edgeIter(fMesh);
	MIntArray		tmp;

	int l = edgeIDs.length();
	
	UINT z;


	faceIDs.setSizeIncrement(l*2);//jede edge hat maximal 2 faces - den speicher hole ich im vorraus, um reallocs zu vermeiden

	MIntArray		containedFaces;	
	int				temp;

	////cout<<"Bin in getAllFaceIndices"<<endl;
	
	for(int i = 0; i < l; i++)
	{
		edgeIter.setIndex(edgeIDs[i],temp);
		edgeIter.getConnectedFaces(tmp);
		
		for(z = 0; z < tmp.length(); z++)
			faceIDs.append(tmp[z]);
	}

	
	int containedCount = getContainedFaces(faceIDs,containedFaces);
	
	helper.memoryPrune(faceIDs);
	
	
	////printArray(containedFaces," = containedFaces");

	//wenn containedFaces mindesten 70 % der gesamtauswahl, dann wird diese genommen, ansonsten die nichtContained faces
	if(faceIDs.length() != 0 && containedFaces.length() != 1)
	{
		if(double(containedCount) / double(faceIDs.length()) > 0.45)
		{
			faceIDs = containedFaces;
		}
		else
		{
			helper.arrayRemove(faceIDs,containedFaces);
		}
	}
	else
	{
		faceIDs = containedFaces;
			
	}

	
}

//------------------------------------------------------------------------------------------------
int			BPTfty::getContainedFaces(MIntArray faceStorage,MIntArray& containedFaces)
//------------------------------------------------------------------------------------------------
{
	/*
	int l = faceStorage.length(),tmp;
	bool found = false;
	int lOffset = 0;
	
	////cout<<"Bin in getContainedFaces"<<endl;

	for(int i = 0; i < l-lOffset; i++)
	{
		tmp = faceStorage[i];
		for(int a = 0; a < l-lOffset;a++)
		{
			if(a != i && tmp == faceStorage[a])
			{
				found = true;
				containedFaces.append(tmp);
				faceStorage.remove(a);
				lOffset++;
				//hier kein abbruch, damit alle faces gefunden und entfernt werden
			}
		}

		if(found)
		{
			found = false;
			faceStorage.remove(i--);
			lOffset++;
		}
	}


	////cout<<"Bin vor memoryPrune"<<endl;
	if(containedFaces.length() != 0)
		helper.memoryPrune(containedFaces);
*/
	helper.memoryPrune(faceStorage,containedFaces);
	//if(containedFaces.length() != 0)
	//	helper.memoryPrune(containedFaces);

	return containedFaces.length();
}


//--------------------------------------------------------------------------------------------------------------------
void		BPTfty::selectComponents(MIntArray& edgeIndices, MString mode, MString selectMode)
//--------------------------------------------------------------------------------------------------------------------
{

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



//------------------------------------------------------------------------------------------------------
void		BPTfty::findCornerVtx(MIntArray& corners,MIntArray& faceVerts,MItMeshVertex& vertIter,int onBoundary)
//------------------------------------------------------------------------------------------------------
{
	int tmp,l,count = -1,edgeCount;
	l = faceVerts.length();
	MIntArray	offsets;
	

	////cout<<"FIND_CORNER_VTX: "<<"Bin in CornerVtxBEGIN"<<endl;
	if(faceVerts.length() != 4 )//|| onBoundary)
	{
		for(int i = 0; i < l;i++)
		{
			vertIter.setIndex(faceVerts[i],tmp);
			vertIter.numConnectedEdges(edgeCount);
			
			if(edgeCount >= 4)
			{
				////cout<<"FIND_CORNER_VTX: "<<"EdgeCount ist groesserGleich 4"<<endl;
				
				corners.setLength(i+1);
				corners[i] = faceVerts[i];
				
				//gefundene Corner war gleich am Anfang oder davor war schon eine
				if(offsets.length() == 0)
				{
					//offsets.clear();
					count = -1;
				}
				else
				{
					//LOffsets = offsets.length();
					
					while(offsets.length() != 0)
					{
						corners[i + offsets[0]] = offsets[0];
						offsets.remove(0);
					}
					/*
					for(int a =  offsets[offsets.length()-1]; a < 0;a++)
					{
					corners[i + offsets[a]] = offsets[a];
					}
					
					  offsets.clear();
					*/
					count = -1;
					
				}
			}
			else
			{
				////cout<<"FIND_CORNER_VTX: "<<"EdgeCount ist KLEINER 4"<<endl;
				offsets.append(count--);
				////cout<<"FIND_CORNER_VTX: "<<"OffsetsLAST_ELEMENT = "<<offsets[offsets.length()-1]<<endl;
			}
			
		}
	}
	else
	{
		corners = faceVerts;
	}

	//Diese Procedur gibt Array der Form BSP. zurück

	//0  1  2  3  4  5  6  7	-->index
	//1 -2 -1  8 -1  5 -1  6	-->VtxId, welche auch Corner ist
}
//-----------------------------------------------------------------------------------------------------
int			BPTfty::findID(MIntArray& matchVtx,MIntArray& faceVerts)
//-----------------------------------------------------------------------------------------------------
{
	int l = faceVerts.length();
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int mL = matchVtx.length(); //!!!Ml wird bereits hier um 1 dekrementiert!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	
	//Diese Variante gibt den Index für den StartVtx für jeden Nachbarn zurück, egal wie gross MatchVtx ist
	for(int i = 0; i < l; i++)
	{
		if(faceVerts[i] == matchVtx[0])
		{
			//wenn nach vorne und nach hinten dasselbe rauskommt, das von hinten nehmen
			if(faceVerts[(i + mL-1)%l] == matchVtx[mL - 1])
				if(faceVerts[((i + l) - mL+1)%l] == matchVtx[mL - 1] )
					return ((i + l) - mL+1)%l;
				else
					return i;
			else
				return ( ((i + l) - mL+1)%l );
				
					//return ( ((i + l + 1) - mL)%l );	
		}			
	}

	INVIS(cout<<"FIND_ID: WAR BEI FOBIDDEN PATH"<<endl;)
	return -1;
}

//-----------------------------------------------------------------
void		BPTfty::cycleVtxMatch(MIntArray& vtxMatch,MIntArray& faceVerts)
//-----------------------------------------------------------------
{
	int l = faceVerts.length();
	bool	done = false,cycle = false;
	int tmp;
	int er = 0; //emergency stop
	MIntArray backup = vtxMatch;

	while(!done)
	{
		if(er++ == 30)
		{
			vtxMatch = backup;
			break;
		}

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
//-----------------------------------------------------------------
void		BPTfty::insertNachbarOffsets(MIntArray& nachbarFaces)
//-----------------------------------------------------------------
{
	int counter = 0,l = nachbarFaces.length();

	for(int i = 0;i < l; i++)
	{
		if(nachbarFaces[i] == -1)
			counter++;
		else
		{
			for(int x = i-counter; counter != 0;x++)
			{
				nachbarFaces[x] = -counter--;
			}
		}
	}



}

//------------------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::appendNachbarIDFlags(MIntArray& faceVtx,MItMeshPolygon& polyIter,MIntArray& vtxMatch,MIntArray& nachbarFaces)
//------------------------------------------------------------------------------------------------------------------------------------------------
{
	int tmp;
	MIntArray	nFaceVerts;
	//erstmal aktuelle VtxMatch bekommen
	polyIter.setIndex(nachbarFaces[nachbarFaces.length()-1],tmp);
	polyIter.getVertices(nFaceVerts);
	vtxMatch = helper.matchArray(faceVtx,nFaceVerts);

	
 
	//tmp =  faceVtx.length() - vtxMatch.length()+1 - nachbarFaces.length();
	//tmp =  ( nachbarFaces.length() + vtxMatch.length() - 1 ) % faceVtx.length();

	////cout<<"APPEND_FLAG: "<<tmp<<" = tmp"<<endl;

	
		//ansonsten einfach mit -1 auffüllen und am endpunkt flag reinpacken
	tmp = vtxMatch.length() - 2;
	for(int m = 0; m < tmp; m++)
	nachbarFaces.append(-1);
		
	nachbarFaces.append(-888888);	//-888888 = flag für FaceEndVtx
	


}

//-------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::insertNachbarInnerFlags(MIntArray& faceMatch,MIntArray& faceMatchVtxMatchCount,MIntArray& nachbarFaces)
//-------------------------------------------------------------------------------------------------------------------------------
{
	int fmL = faceMatch.length();
	int nL	= nachbarFaces.length();
	int tmp;

	for(int i = 0; i < fmL;i++)
	{
		//absFaceID im nachbarArray finden
		for(int x = 0; x < nL; x++)
		{
			if(nachbarFaces[x] == faceMatch[i])
			{
				//letzen eintrag nicht bearbeiten
				if(x != nL - 1)
				{
					tmp = x + faceMatchVtxMatchCount[i] - 1;
					//dann Flag an notwendige Position eintragen, wenn nicht schon was gültiges da
					if( nachbarFaces[tmp] < 0 )
						nachbarFaces[tmp] = -888888;

					break;
				}
			}
		}
	}

}


//**************************************************************************
//****************************SLIDE_METHODEN********************************
//**************************************************************************
//-----------------------------------------------------------------
void	BPTfty::doSlide(bool tweakEnabled)
//-----------------------------------------------------------------
{
	MStatus	stat;
	



//	MFnMesh	FnMesh(newMeshData,&stat);
	MFnMesh	FnMesh(fMesh,&stat);

	
	int l = slideIndices->length();
	
	//dafür sorgen, dass scaling immer den korrekten wert hat
	if(maxStandardScale == 1.0)
	{	
		maxStandardScale = 0.0;

		for(int i = 0; i < l; i++)
		{
			if((*normalScale)[i] > maxStandardScale)
				maxStandardScale = (*normalScale)[i];
		}
	}

	if(!isRelative)
	{
		//sichergehen, dass auch schon ScaleWerte erzeugt wurden, wenn er hier ankommt 
		if((*slideScale)[0] == -1)
			createAbsoluteVectors(*slideDirections, *slideScale);
	}

	//checken, welche seite gewünscht ist
	for(int i = 0; i < l; i++)
	{
		
		if((*maySlide)[i])
		{
			if(isRelative)
			{
				
				if(options[3])
				{
					if(isNormalRelative)
						FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideDirections)[i]) + (normal * (*slideNormals)[i] * maxStandardScale ));
					else
						FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideDirections)[i]) + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
				}
				else
				{
					if(isNormalRelative)
						FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * slide * (*slideDirections)[i] + (normal * (*slideNormals)[i] * maxStandardScale));
					else
						FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * slide * (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
				}
			}
			else
			{

				
				////cout<<"Bin nach CREATEABSOLUTE VECTORS"<<endl;
				if(options[3])
				{
					if(isNormalRelative)
						FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideScale)[i])* (*slideDirections)[i] + ( normal  * (*slideNormals)[i] * maxStandardScale) );
					else
						FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideScale)[i])* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
				}
				else
				{
					if(isNormalRelative)
						FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] + ( normal * (*slideNormals)[i] * maxStandardScale) );
					else
						FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
				}	
			}
			
			
		}
		else
		{
			if(isNormalRelative)
			{
				FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (normal * (*slideNormals)[i] * maxStandardScale) );
			}
			else
			{
				FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (normal * (*normalScale)[i] *(*slideNormals)[i]) );
			}
		}
	}


	if( !tweakEnabled )	//nur wenn keine Tweaks vorhanden sind dürfen die UVs verändert werden, weil es ansonsten zu ungewollten verzerrungen kommen kann
		doUVSlide();
	

	if(!tweakEnabled)
		stat = FnMesh.updateSurface();
	
	
}

//--------------------------------------
void BPTfty::doUVSlide()
//--------------------------------------
{

//	MFnMesh	FnMesh(newMeshData);
	MFnMesh	FnMesh(fMesh);
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



	for(unsigned int i = 0; i < numUVSets; i++)
	{
		MFloatVectorArray&	start = *iterSlideStart;
		MFloatVectorArray&	end	  = *iterSlideEnd;
		MFloatVectorArray&	dirs = *iterSlideDirs;

		MIntArray&			IDs = *iterSlideIDs;

		MFloatArray& scale = *iterScale;

		unsigned int l = IDs.length();

		if(!isRelative)
		{
			//erstmal den ScaleFactor errechnen, wenn noetig
			

			if(scale.length() == 0)
			{
				scale.setLength(l);
				createAbsoluteVectors(dirs,scale);
			}
		}


		


		if(isRelative)
		{
			if(options[3])
				for(unsigned int x = 0; x < l;x++)
					FnMesh.setUV(IDs[x],start[x].x + (fSlide * dirs[x].x), start[x].y + (fSlide * dirs[x].y));
			else
				for(unsigned int x = 0; x < l;x++)
					FnMesh.setUV(IDs[x],end[x].x - (fSlide * dirs[x].x), end[x].y - (fSlide * dirs[x].y));

		}
		else
		{
			if(options[3])
				for(unsigned int x = 0; x < l;x++)
					FnMesh.setUV(IDs[x],start[x].x + (fSlide * scale[x] * dirs[x].x), start[x].y + (fSlide * scale[x] * dirs[x].y));
			else
				for(unsigned int x = 0; x < l;x++)
					FnMesh.setUV(IDs[x],end[x].x - (fSlide * scale[x] * dirs[x].x), end[x].y - (fSlide * scale[x] * dirs[x].y));
		}

		
		

		++iterSlideStart;
		++iterSlideEnd;
		++iterSlideDirs;
		++iterSlideIDs;
		++iterScale;
		
	}
}



//--------------------------------------
double		BPTfty::clampX(double value)
//--------------------------------------
{
	if(value <= 0.5)
		return value * 2;
	else
		return 1.0;
}

//--------------------------------------
float		BPTfty::clampX(float value)
//--------------------------------------
{//für UVs überladene Funktion, die auf floats arbeitet
	if(value <= 0.5)
		return value * 2;
	else
		return 1.0;
}


//--------------------------------------
double		BPTfty::clampY(double value)
//--------------------------------------
{
	if(value >= 0.5)
		return value * 2 - 1;
	else
		return 0.0;
}


//--------------------------------------
float		BPTfty::clampY(float value)
//--------------------------------------
{//für UVs überladene Funktion, die auf floats arbeitet

	if(value >= 0.5)
		return value * 2 - 1;
	else
		return 0.0;
}


//----------------------------------------------
double		BPTfty::clampNormalX(double value)
//----------------------------------------------
{
	if(value < 0.4) 
		return 1.0;
	
	if(value > 0.6)
		value = 0.6;


	return (0.6 - value) * 5 ; 
}

//----------------------------------------------
double		BPTfty::clampNormalY(double value)
//----------------------------------------------
{
	if(value >= 0.6) 
		return 1.0; 
	
	if(value < 0.4)
		value = 0.4;
	
	return ( fabs(0.4 - value) ) * 5;
}

/*
//-----------------------------------------------------------------
bool		BPTfty::controlVtxNotDone(int ID,MIntArray& isDone)
//-----------------------------------------------------------------
{
	int l = isDone.length();

	for(int i = 0; i < l; i++)
	{
		if(ID == isDone[i])
			return false;
	}

	return true;
}
*/

//-----------------------------------------------------------------
void		BPTfty::doSlideSMC(bool tweakEnabled)
//-----------------------------------------------------------------
{
	MStatus	stat;


	MFnMesh	FnMesh(fMesh,&stat);
	
	int l = slideIndices->length();
	

	//dafür sorgen, dass scaling immer den korrekten wert hat
	if(maxStandardScale == 1.0)
	{	
		maxStandardScale = 0.0;

		int i;
		for(i = 0; i < l; i++)
		{
			if((*normalScale)[i] > maxStandardScale)
				maxStandardScale = (*normalScale)[i];
		}
	
		for(i = 0; i < l; i++)
		{
			if(normalScale2[i] > maxStandardScale)
				maxStandardScale = normalScale2[i];
		}
	}



	//checken, welche seite gewünscht ist
	for(int i = 0; i < l; i++)
	{
		
		if((*maySlide)[i])
		{
			if(isRelative)
			{
				
				if(options[3])
				{
					if(hasSecondDirection[i])
					{
						if(isNormalRelative)
							
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * ( (clampX(slide) * (*slideDirections)[i]) + clampY(slide)*slideDirections2[i] ) + normal * clampNormalX(slide) * maxStandardScale * (*slideNormals)[i] + normal * clampNormalY(slide) * maxStandardScale * slideNormals2[i]);
						else
							//FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * ( (clampX(slide) * (*slideDirections)[i]) + clampY(slide)*slideDirections2[i] ) + normal * clampNormalX(slide) * (*normalScale)[i] * (*slideNormals)[i] + normal * normalScale2[i] * clampNormalY(slide) * slideNormals2[i]);
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * ( (clampY(slide) * (*slideDirections)[i]) + clampX(slide)*slideDirections2[i] ) + normal * clampNormalY(slide) * (*normalScale)[i] * (*slideNormals)[i] + normal * normalScale2[i] * clampNormalX(slide) * slideNormals2[i]);
						
					}
					else
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideDirections)[i]) + (normal * maxStandardScale * (*slideNormals)[i] ));
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideDirections)[i]) + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
					}
				
				}
				else
				{
					if(hasSecondDirection[i])
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (clampX(slide) * (*slideDirections)[i]) + clampY(slide)*slideDirections2[i] + normal * clampNormalX(slide) * maxStandardScale * (*slideNormals)[i] + normal * clampNormalY(slide) * maxStandardScale * slideNormals2[i]);
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (clampX(slide) * (*slideDirections)[i]) + clampY(slide)*slideDirections2[i] + normal * (*normalScale)[i] * clampNormalX(slide) * (*slideNormals)[i] + normal * normalScale2[i] * clampNormalY(slide) * slideNormals2[i]);
						
					}
					else
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * slide * (*slideDirections)[i] + (normal * (*slideNormals)[i] * maxStandardScale));
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + -1 * slide * (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i] ));
					}
				}
			}
			else
			{
				//sichergehen, dass auch schon ScaleWerte erzeugt wurden, wenn er hier ankommt 


				if(hasSecondDirection[i])
				{

					if((*slideScale)[0] == -1)
						createAbsoluteVectors(*slideDirections,*slideScale);


					if(slideScale2[0] == -1)
						createAbsoluteVectors(slideDirections2,slideScale2);


					if(options[3])
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + slide * (*slideScale)[i]* (*slideDirections)[i] + ( normal  * (*slideNormals)[i] * maxStandardScale) );
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + slide * (*slideScale)[i]* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
					}
					else
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * slideScale2[i])* slideDirections2[i] + ( normal * (*slideNormals)[i] * maxStandardScale) );
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * slideScale2[i])* slideDirections2[i] + ( (normal * normalScale2[i]) * slideNormals2[i]) );
					}
					


				}
				else
				{
					
					////cout<<"Bin nach CREATEABSOLUTE VECTORS"<<endl;

					if((*slideScale)[0] == -1)
						createAbsoluteVectors(*slideDirections,*slideScale);


					if(options[3])
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideScale)[i])* (*slideDirections)[i] + ( normal  * (*slideNormals)[i] * maxStandardScale ) );
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (slide * (*slideScale)[i])* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
					}
					else
					{
						if(isNormalRelative)
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] + ( normal * (*slideNormals)[i] * maxStandardScale) );
						else
							FnMesh.setPoint((*slideIndices)[i],(*slideEndPoints)[i] + (-1 * slide * (*slideScale)[i])* (*slideDirections)[i] + ( (normal * (*normalScale)[i]) * (*slideNormals)[i]) );
					}
				}
			}
			
			
		}
		else
		{
			if(isNormalRelative)
			{
				FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (normal * (*slideNormals)[i] * maxStandardScale) );
			}
			else
			{
				FnMesh.setPoint((*slideIndices)[i],(*slideStartPoints)[i] + (normal * (*normalScale)[i] *(*slideNormals)[i]) );
			}
		}
	}
	
	//UVSlide wird immer gemacht, wenn tweak nicht aktiviert ist
	if(!tweakEnabled)
	{
		doUVSlideSMC();	
	}


			

}

//-----------------------------------------------------------------
void	BPTfty::doUVSlideSMC()
//-----------------------------------------------------------------
{
	MFnMesh	FnMesh(fMesh);
	//unsigned int numUVSets = UVSetNames.length();
	
	//da mutiUVSets eh nicht funzen
	unsigned int numUVSets = 1;	

	//lokaoe#le float kopie von slide, um ständiges konvertieren zu vermeiden;
	float fSlide = slide;

	//iteratoren
	std::list<MIntArray>::iterator iterSlideIDs = UVSlideIndices->begin();
	std::list<MFloatVectorArray>::iterator iterSlideStart = UVSlideStart->begin();
	std::list<MFloatVectorArray>::iterator iterSlideDirs = UVSlideDirections->begin();

	std::list<MFloatArray>::iterator iterScale = UVSlideScale->begin();




	for(unsigned int i = 0; i < numUVSets; i++)
	{
		MFloatVectorArray&	start = *iterSlideStart;
		MFloatVectorArray&	dirs = *iterSlideDirs;


		MIntArray&			IDs = *iterSlideIDs;

		MFloatArray& scale = *iterScale;


		unsigned int l = IDs.length();

		if(!isRelative)
		{
			//erstmal den ScaleFactor errechnen, wenn noetig
			if(scale.length() == 0)
			{
				scale.setLength(l);
				createAbsoluteVectors(dirs,scale,true,&IDs);
			}

		}



		if(isRelative)
		{
			if(options[3])
				for(unsigned int x = 0; x < l;x+=2)
				{
					if(IDs[x+1] > -1)
					{
						if(fSlide > 0.5)
							FnMesh.setUV(IDs[x],start[x+1].x + ( 1 - (fSlide * 2 - 1) ) * dirs[x+1].x ,start[x+1].y + ( 1 - (fSlide * 2 - 1) ) * dirs[x+1].y ,&(UVSetNames[i]) );
						else if(IDs[x] > -1)
						{
							FnMesh.setUV(IDs[x],start[x].x + (fSlide * 2 * dirs[x].x),start[x].y + (fSlide * 2 * dirs[x].y),&(UVSetNames[i]) );
						}
					}
				}
			else
				for(unsigned int x = 0; x < l;x+=2)
				{
					if(IDs[x] > -1)
					{
						if(fSlide > 0.5)
							FnMesh.setUV(IDs[x],start[x].x + ( 1 - (fSlide * 2 - 1) ) * dirs[x].x ,start[x].y + ( 1 - (fSlide * 2 - 1) ) * dirs[x].y ,&(UVSetNames[i]) );
						else if(IDs[x+1] > -1)
						{
							FnMesh.setUV(IDs[x+1],start[x+1].x + (fSlide * 2 * dirs[x+1].x),start[x+1].y + (fSlide * 2 * dirs[x+1].y),&(UVSetNames[i]) );
						}
					}
				}
		}
		else
		{
		if(options[3])
				for(unsigned int x = 0; x < l;x+=2)
				{
					if(IDs[x+1] > -1 && IDs[x] > -1)
					{
							FnMesh.setUV(IDs[x],start[x+1].x + fSlide * scale[x+1] * dirs[x+1].x ,start[x+1].y + fSlide * scale[x+1] * dirs[x+1].y ,&(UVSetNames[i]) );
					}
				}
			else
				for(unsigned int x = 0; x < l;x+=2)
				{
					if(IDs[x] > -1)
					{
							FnMesh.setUV(IDs[x],start[x].x + fSlide * scale[x] * dirs[x].x ,start[x].y + fSlide * scale[x] * dirs[x].y ,&(UVSetNames[i]) );
					}
				}
		}

		
		

		++iterSlideStart;
		++iterSlideDirs;
		++iterSlideIDs;
		++iterScale;

		
	}


}

//-----------------------------------------------------------------
void	BPTfty::createAbsoluteVectors(MVectorArray& slideDirections,MDoubleArray& slideScale)
//-----------------------------------------------------------------
{
	//index mit dem kleinsten enthaltenen vector finden und einzelne Längen zwischenspeichern
	////cout<<"Bin in createAbsolute Vectors"<<endl;
	//ScaleFaktoren für die Normale werden bereits bei der Erstellung des Meshes erzeugt und sind vorhanden
	MDoubleArray		tmp;
	int  l = slideIndices->length();
	
	tmp.setLength(l);	//um reallocs zu vermeiden

	double x = 16666666;

	int i;
	for(i = 0;i < l;i++)
	{
		tmp[i] = slideDirections[i].length();
		if( (*maySlide)[i]  && tmp[i] < x )
		{
			x = tmp[i];
		}
	//////cout<<smallestIndex
		
	
	}



	for(i = 0; i < l;i++)
	{
		slideScale[i] = x / tmp[i];
	}

//	//cout<<"ENDE createAbsolute Vectors ENDE"<<endl;
	
}

//-----------------------------------------------------------------
void	BPTfty::createAbsoluteVectors(MFloatVectorArray& slideDirections,MFloatArray& slideScale, bool isSMC,MIntArray* IDs)
//-----------------------------------------------------------------
{//überladene Funktion - arbeitet mit float präzision

	//index mit dem kleinsten enthaltenen vector finden und einzelne Längen zwischenspeichern
	////cout<<"Bin in createAbsolute Vectors"<<endl;
	//ScaleFaktoren für die Normale werden bereits bei der Erstellung des Meshes erzeugt und sind vorhanden
	MFloatArray		tmp;
	
	int  l = slideDirections.length();
	float x = 16666666;

	tmp.setLength(l);

	int factor = 1;

	if(isSMC)
	{
		factor = 2;
	}

	
	for(int c = 0; c < factor; c++)
	{
		x = 16666666;
		tmp.clear();

		if(IDs == 0)
		{
			for(int i = 0;i < l;i+=factor)
			{
				tmp[i] = (slideDirections[i+c].length());
				if( tmp[i/factor] < x)
				{
					x = tmp[i/factor];
				}
				//////cout<<smallestIndex
			}
		}
		else
		{
			for(int i = 0;i < l;i+=factor)
			{
				tmp[i] = (slideDirections[i+c].length());
				if( tmp[i/factor] < x && (*IDs)[i+c] != -1)
				{
					x = tmp[i/factor];
				}
				//////cout<<smallestIndex
			}
		}


		for(int i = 0; i < l;i+=factor)
		{
			slideScale[i+c] = x / tmp[i/factor];
		}

		
	}
//	//cout<<"ENDE createAbsolute Vectors ENDE"<<endl;
	
}

//-----------------------------------------------------------------------------------------------------------------------
void		BPTfty::findEdgeNachbarMatrixIndices(edge* origPtr,edge* localPtr,int& seite,int& richtung)
//------------------------------------------------------------------------------------------------------------------------
{

	//hier im grunde nur RIchtung ( durch faces ) und seite (durch Vtx) miteinander vergleichen
	MIntArray otherFaces = localPtr->getFaces();
	MIntArray otherVtx = localPtr->getVtx();

	MIntArray origFaces = origPtr->getFaces();
	MIntArray origVerts = origPtr->getVtx();

	//Richtung finden
	UINT i;
	for(i = 0;i < origFaces.length();i++)
	{
		for(unsigned int a = 0; a < otherFaces.length();a++)
		{
			if(origFaces[i] == otherFaces[a])
			{
				richtung = i;
				break;
			}
		}
	}

	// nun die richtige Seite finden
	for(i = 0; i < origVerts.length();i++)
	{
		for(unsigned int a = 0;a < otherVtx.length();a++)
		{
			if(origVerts[i] == otherVtx[a])
			{
				seite = i;
				break;
			}
		}
	}

	//DEBUG: Checken, ob jeder index innerhalb zulässiger parameter
//	if(seite > 1 || richtung > 1)
		//cout<<"Fehler in FindEdgeBachbarMAtrixIndices: seite/2 = "<<seite<<" - "<<richtung<<endl;

	
}



//--------------------------------------------------------------------------------------------
bool	BPTfty::isDirConform(	const int index0, 
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



//----------------------------------------------------------------------------------------------------------
void		BPTfty::initEdges(MIntArray& allEdges,MIntArray& edgeIDs,simpleLinkedList<edge>& edgePtrs, bool	createOrigVtxLOT)
//----------------------------------------------------------------------------------------------------------
{
		SPEED("KONVERTIERE MESH IN (BEVEL)");


	


		MItMeshEdge		edgeIter(fMesh);
		MItMeshPolygon	polyIter(fMesh);
		MItMeshVertex	vertIter(fMesh);
		MFnMesh			meshFn(fMesh);

									//standardBA mit offset


		int tmp;
		unsigned int l = 0, i, a, x;

		bool	hasPtr = false;
		bool	connectedHasPtr = false;
		bool	isEndEdge = false;
		edge*	existingPtr = 0;	//nimmt ptr auf, wenn edge bereits in ptrArray vorhanden

		MVectorArray normals;
		
		int			verts[2];
		MIntArray	connectedEdges[2];
		MIntArray	nFaces;
		MIntArray	faceEdges[2];
		edge*		nachbarn[2][2];
		MIntArray	matchEdges;


		edge*		nSelEdgeArray[4];	//sync zu seite/richtung, hält die unselEdges rechts und links von selEdge zwecks intersection wenn nurEinNachbar true

		
		MIntArray	uIDs(4);		//enthält im falle von nurEinNachbarEdges eine ID zur späteren eindeutigen identifikation von richtung
		bool		onlyOneNSelEdge[4];	//hält true, wenn nur eine nSeledge pro seite pro richtung
		//BEGIN

				//erstmal edgeArrays loeschen, für den fall, dass vorher abgestürzt ist

		edge::origVtxDoneBA.setLength(0);

		edge::endFacePtrs.clear();

		edge::nSelEdgeDataArray.clear();
		
//		edge::vtxChangedBA.setLength(0);
		
		//initialisierung mit der gesamtZahl an Vertizen
//		edge::vtxChangedBA = BPT_BA(vertIter.count());

//		edge::refVtxLOT.clear();
//		edge::refVtxLOT.setSizeIncrement(100);	// == performanceZY
		
		

		//debug
		//printArray(allEdges," = allEdges am Anfang");
		BPT_BA selEdgesBA(allEdges,true,false,meshFn.numEdges());	//erstellt komplettes BA, damit man später problemlos "append" operationen simulieren kann


		//zuerst alle winged edges entfernen
		l = allEdges.length();

		if(createOrigVtxLOT)
		{//diese Variante baut ausserdem noch ORIG VTX LOT auf, wird von camfer verwendet -> dies vermeidet redundanz
			MIntArray edgeVertices(l*2);	//jede edge hat 2 vertizen
			a = 0;
			int VID;	// = vtxID
			
			for(i = 0; i < l; i++)
			{
				edgeIter.setIndex(allEdges[i], tmp);
				
				for(x = 0; x < 2; x++)
				{

					VID = edgeIter.index(x);
					edgeVertices[a++] = VID;


					vertIter.setIndex(VID,tmp);
					
					
					vertIter.getConnectedEdges(matchEdges);
					//vertIter.numConnectedEdges(tmp);	//funzt nicht richtig unter 4.5, 5.0 ->Absturz bei wingedVtx
					
					//if(tmp < 3)
					if(matchEdges.length() < 3)
					{//vertex ist mit 2 oder weniger edges verbunden == er ist winged und edge wird entfernt
						
						selEdgesBA.setBitFalse(allEdges[i]);
						//break;	//dieses Break koennte verhindern, dass ein benoetigter Vtx nicht aufs array kommt
					}
					
				}
				
			}

			
			//jetzt die edgeVerts in BA setzen
			edge::origVtxDoneBA.add(edgeVertices, 0,0);
	
		}
		else
		{
			for(i = 0; i < l; i++)
			{
				edgeIter.setIndex(allEdges[i], tmp);
				
				for(x = 0; x < 2; x++)
				{
					vertIter.setIndex(edgeIter.index(x),tmp);
					
					vertIter.getConnectedEdges(matchEdges);
					//vertIter.numConnectedEdges(tmp);	//funzt nicht richtig unter 4.5, 5.0
					
					//if(tmp < 3)
					if(matchEdges.length() < 3)
					{//vertex ist mit 2 oder weniger edges verbunden == er ist winged und edge wird entfernt
						
						selEdgesBA.setBitFalse(allEdges[i]);
						break;
					}
					
				}
				
			}

		}


		//BAs erstellen:
		
		BPT_BA allEdgesBA(selEdgesBA);	
		//zuerst sämtliche winged edges entfernen


	
		ULONG allEdgesZero;
	//while(allEdges.length() != 0)
		while(allEdgesBA.getTrueCount() != 0)
		{
			MPRINT("Bin in while")
			//allEdges[0] Cachen, damit sich der BALookup auf ein minimum beschränkt
			allEdgesZero = allEdgesBA.getFirstBitIndex();

			hasPtr = false;
	

			nachbarn[0][0] = nachbarn[0][1] = nachbarn[1][0] = nachbarn[1][1] = 0;
			nSelEdgeArray[0] = nSelEdgeArray[1] = nSelEdgeArray[2] = nSelEdgeArray[3] = 0;
			onlyOneNSelEdge[0] = onlyOneNSelEdge[1] = onlyOneNSelEdge[2] = onlyOneNSelEdge[3] = false;
		
			
			//zuerst checken, ob edgeID bereits pointer hat
			l = edgeIDs.length();
			for(i = 0; i < l;i++)
			{
				if(edgeIDs[i] == allEdgesZero)
				{
					hasPtr = true;
					existingPtr = edgePtrs[i];
					break;
				}
			}
			
			edgeIter.setIndex(allEdgesZero,tmp);
			
			
			
			verts[0] = edgeIter.index(0);verts[1] = edgeIter.index(1);
			
			
			
			edgeIter.getConnectedFaces(nFaces);
			
			
			
			normals.clear();
			normals.setLength(nFaces.length());
			for(i = 0; i < nFaces.length();i++)
			{
				polyIter.setIndex(nFaces[i],tmp);
				polyIter.getNormal(normals[i]);		//holt normale in ObjectSpace
				polyIter.getEdges(faceEdges[i]);
			}
			
			
			//nachbarn erstellen
			MPRINT("----------------------------------------------")
			INVIS(cout<<"BEARBEITE EDGE: "<<allEdgesZero<<endl;)
			MPRINT("----------------------------------------------")
			//erst connectedEdges holen
			for(i = 0; i < 2; i++)
			{
				vertIter.setIndex(verts[i],tmp);
				vertIter.getConnectedEdges(connectedEdges[i]);
				
				//gegenwärtige edge entfernen
				l = connectedEdges[i].length();
				for(a = 0; a < l; a++)
				{
					if(connectedEdges[i][a] == allEdgesZero)
					{
						connectedEdges[i].remove(a);
						break;
					}
				}
			}
			

			


			INVIS(cout<<verts[0]<<" - "<<verts[1]<<" = verts"<<endl;)

			//nun weitermachen mit den 2 CornerSeiten
			for(i = 0; i < 2;i++)
			{
				isEndEdge = false;
				//UEBERPRUEFEN; OB von restlichen connected Edges noch welche selected sind. Wenn nicht, dann is diese edge
				// ne endEdge und man muss den ganzen aufwand mit den nonSelectedEdges nicht betreiben
				//if(helper.matchArray(connectedEdges[i],selEdges).length() == 0)

				if( (selEdgesBA & connectedEdges[i]).length() == 0 )
				{
					INVIS(cout<<"Seite mit Vtx "<<verts[i]<<" ist endEdge ( EDGE: "<<allEdgesZero<<" )"<<endl;)
					isEndEdge = true;
				}
				
				
				
				//edgePtr erstellen und für die angrenzenden edges pro richtung
				for(a = 0; a < nFaces.length(); a++)
				{
					connectedHasPtr = false;

					matchEdges = helper.matchArray(connectedEdges[i],faceEdges[a]);
					
					//matchEdges dürfte nur immer 1 sein. Check, ob das wirklich so ist: SPÄTER RAUSNEHMEN
					INVIS(if(matchEdges.length() != 1))
						INVIS(cout<<"FEHLER IN SOLID CHAMFER: MATCH EDGES HAT GRÖssE VON "<<matchEdges.length()<<endl;)
					
					//zuerst prüfen, ob edge selected ist
					if(!isEndEdge)
					{
						
						if( (selEdgesBA & matchEdges).length() == 1)
						{
							//checken, ob matchEdge bereits ptr hat
							l = edgeIDs.length();
							for(x = 0; x < l; x++)
							{
								if(edgeIDs[x] == matchEdges[0])
								{
									connectedHasPtr = true;
									nachbarn[i][a] = edgePtrs[x];

									//printArray(nFaces," = nFaces");

									MIntArray tmpArray = edgePtrs[x]->getFaces();
									uIDs[i*2+a] = getUID(nFaces[a],tmpArray,polyIter,(int)allEdgesZero,matchEdges[0]);

									//debug
									
									INVIS(cout<<"Habe selected mit Ptr gefunden: "<<nachbarn[i][a]->getID()<<" eigentl. ID: "<<matchEdges[0]<<endl;)

									break;
								}
							}
							
							if(!connectedHasPtr)
							{
								edgeIDs.append(matchEdges[0]);
							
								//hier werden keine Daten gesammelt, da diese später sowieso für selected edges 
								//erzeugt werden, wenn sie an der reihe sind
								//VERMUTLICH gibt es situationen, wo doch eine mit Vtx und Faces initialisierte Edge benoetigt 
								//wird, also lieber noch einbauen (obwohl es dann doppelt gemoppelt wird)
								
								INVIS(cout<<"Habe selected OHNE ptr gefunden: "<<matchEdges[0]<<endl;)
								//Vtx und FaceDaten holen
								edgeIter.setIndex(matchEdges[0],tmp);
								
								MIntArray seFaces;
								int seVtx[2];
								edgeIter.getConnectedFaces(seFaces);
								seVtx[0] = edgeIter.index(0);
								seVtx[1] = edgeIter.index(1);
								
								edge* selEdgeTmp = new selEdge(matchEdges[0]);
								
								edgePtrs.append(selEdgeTmp); 
								
								nachbarn[i][a] = selEdgeTmp;
								
								selEdgeTmp->setVtxIDs(seVtx);
								selEdgeTmp->setFaceIDs(seFaces);




								MIntArray tmpFaceVtx;
								bool	flipped[2] = {false,false};
								//jetzt bei den seFaces checken, ob die direction conform ist oder nicht - diese Werte dann der edge übergeben
								for(x = 0; x < seFaces.length(); x++)
								{
									polyIter.setIndex(seFaces[x],tmp);
									polyIter.getVertices(tmpFaceVtx);

									flipped[x] = !isDirConform(seVtx[0],seVtx[1],tmpFaceVtx);
								}

								selEdgeTmp->setIsFlipped(flipped);


								uIDs[i*2+a] = getUID(nFaces[a],seFaces,polyIter,allEdgesZero,matchEdges[0]);
								
								
								
							}
							
						}
						else
						{//UNSELECTED EDGES
							//checken, ob matchEdge bereits ptr hat

							
							//bearbeiten aller unselected edges
							//Nun neue unselected edges erstellen an diesem Vtx und nur mit
							//einer initialisierten seite, bis sie auf selected edge stoesst oder edge mit ptr
							if(!connectedHasPtr)
							{
								MPRINT("Bearbeite unselected ohne Ptr")

								MIntArray vertexEdges = connectedEdges[i];
								MIntArray ceFaces;
								int ceVerts[2];
								
								
								MIntArray					localEdgeIDs;
								simpleLinkedList<edge>		localEdgePtrs;	//kann in diesem Fall verwendet werden, da sie ihre einheiten nicht loescht, wenn sie selbst geloescht wird
	
								//Wegen UVs geht das nicht
							//	MDoubleArray				arcRatios;		//sync zu localEdgeIDs/Ptrs, hält werte zwischen 0 und 1 (1 = parallel, 0 = orthogonal zu OriginalEdge)
							//	MPoint						currentPoint = edge::creator->getVtxPoint(verts[i]);
							//	MVector						currentDir = currentPoint - edge::creator->getVtxPoint(verts[1-i]);
							//	currentDir.normalize();

								int currentEdge ,lastEdge;
								

								lastEdge = allEdgesZero;
								

								currentEdge = matchEdges[0];


								int count;
								count = 0;
								
								bool stopSearch = false;
								
								localEdgeIDs.clear();
								//und locEdgePtrs werden einfach überschrieben

								MPRINT("!!!!!!!!!!!!!!!!!!!!!")
								
								while(!stopSearch)
								{
									//erst richtungsdaten sammeln
									edgeIter.setIndex(currentEdge,tmp);
									edgeIter.getConnectedFaces(ceFaces);
									
									
									
									if(selEdgesBA[currentEdge])
									{
										INVIS(cout<<"Habe SELECTED Edge gefunden im "<<count<<"'ten durchlauf,Edge ist"<<currentEdge<<endl;)
											//wenn er hier selected edge findet, kann diese bereits einen Ptr haben, wenn sie 
											//zuvor von dem durchgang, welcher die andere richtung behandelt, mit ptr versehen wurde
											stopSearch = true;
										//Diese Edge jetzt mit nem MiniPtr ausstatten und nachbarArray von alledges 0 damit
										//auffüllen, ausserdem die Lastedge damit bestücken
										//erst wenn diese selEdge hier an der reihe ist, holt sie sich einen ptr zur ihrem
										//nachbarn
										bool ptrAvailable = false;
										edge* selPtr = 0;
										
										//hat edge ptr?
										int miles2 = edgeIDs.length();
										for(x = 0; x < miles2;x++)
										{
											if(currentEdge == edgeIDs[x])
											{
												ptrAvailable = true;
												selPtr = edgePtrs[x];
												
												break;
											}
										}
										
										if(!ptrAvailable)
										{
											edgeIDs.append(currentEdge);
											selPtr = new selEdge(currentEdge);
											edgePtrs.append(selPtr);
											
											int selVerts[2]; //(Faces sind schon in ceFaces)
											//edgeIter ist immer noch auf currentEdge gesetzt
											selVerts[0] = edgeIter.index(0);
											selVerts[1] = edgeIter.index(1);
											
											
											//lastEdge ist an dieser Stelle auf jeden fall eine nSelEdge
											selPtr->setVtxIDs(selVerts);
											selPtr->setFaceIDs(ceFaces);
											
											MIntArray tmpFaceVtx;
											bool	flipped[2] = {false,false};
											//jetzt bei den seFaces checken, ob die direction conform ist oder nicht - diese Werte dann der edge übergeben
											for(x = 0; x < ceFaces.length(); x++)
											{
												polyIter.setIndex(ceFaces[x],tmp);
												polyIter.getVertices(tmpFaceVtx);
												
												flipped[x] = !isDirConform(selVerts[0],selVerts[1],tmpFaceVtx);
											}
											
											selPtr->setIsFlipped(flipped);
											
										}
										
										
										
										//in diesem Fall liegen die selEdges weiter voneinander entfernt und ihre
										//faces teilen keine Kanten. Deshalb ist die UId die Summe von (nFaces[a] + lastFace)
										MIntArray lastEdgeFaces;
										if(count == 0)
										{	
											lastEdgeFaces = nFaces;
										}
										else
										{
											
											lastEdgeFaces = localEdgePtrs[localEdgeIDs.length()-1]->getFaces();
											//ptr von der (noch) currentEdge führt
										}
										
										
										uIDs[i*2+a] = nFaces[a]*3.75 + 4.75*helper.matchArray(ceFaces,lastEdgeFaces)[0];
										INVIS(cout<<"HABE UID ADDIERT! ( "<<nFaces[a]*1.25<<" + "<<helper.matchArray(ceFaces,lastEdgeFaces)[0]*4.75<<endl;)
											
											
											
											//!!!!//	//nachbarArray aktualisieren mit dieser gültigen selEdge
										nachbarn[i][a] = selPtr;
										
										//ptr von lastEdge holen (ist natürlich der letzte ptr im LocPtrArray
										//diese Arrayposition ist auf jeden Fall gefüllt
										edge* lastPtr = localEdgePtrs[localEdgeIDs.length() -1];
										
										
										int seite, richtung;
									
										//UNNÖTIG; DA DIE NACHBARN EH WIEDER UEBERSCHRIEBEN WERDEN; SOBALD DIESE EDGE HIER AN DER REIHE IST
										findEdgeNachbarMatrixIndices(selPtr,lastPtr,seite, richtung);
										//selPtr->setNachbar(seite, richtung,lastPtr);
										
										
										//jetzt noch die uID für die Neue Edge setzen
										MIntArray tmpUID(4,-1);
										tmpUID[seite*2+richtung] = uIDs[i*2+a];
										////printArray(tmpUID," = tmpUID");
										selPtr->setUIDs(tmpUID);
										
										
										//UNNÖTIG: UNSELEDGES SOLLEN KEINE VERBINDUNG ZU SEL EDGES HABEN
										/*
										findEdgeNachbarMatrixIndices(lastPtr,selPtr,seite, richtung);
										lastPtr->setNachbar(seite,richtung,selPtr);
										*/
										
										break;
										
									}
									
									
									//if(stopSearch)	//eigentlich unnoetig 
									//	break;
									
									//edge ist gültig, also daten sammeln und ptr erstellen
									//unSelEdges benoetigen immer nur ihre Faces und ihre Vtx
									
									edgeIter.setIndex(currentEdge,tmp);
									ceVerts[0] = edgeIter.index(0);
									ceVerts[1] = edgeIter.index(1);
									
									//edgeIDs.append(currentEdge);
									edge* nSelEdgeTmp = new nSelEdge(currentEdge);
									//edgePtrs.append(nSelEdgeTmp);
									
									nSelEdgeTmp->setVtxIDs(ceVerts);
									nSelEdgeTmp->setFaceIDs(ceFaces);

									//nSelEdges benoetigen die isFlipped information nicht
									
									/*	//Dies darf nicht mehr gemacht werden, da bei der Erzeugung der UVs wiklich die nächstgelegene Edge verwendet werden muss
									//jetzt die EdgeRatio ausrechnen (origEdge zu current unselEdge)
									MVector thisDir = currentPoint - edge::creator->getVtxPoint( (ceVerts[0] == verts[i]) ? ceVerts[1] : ceVerts[0] );
									thisDir.normalize();
									arcRatios.append( fabs(currentDir * thisDir) );
									*/									


									//LocalArrays aktualisieren
									localEdgeIDs.append(currentEdge);
									localEdgePtrs.append(nSelEdgeTmp);
									
									if(ceFaces.length() == 1)
									{
										//edge liegt auf border, und man kann nicht weiter um den Vtx herum in diese richtung
										//->Abbruch
										//allerdings muss diese edge noch als nachbar eingetragen werden, da ein anderer ja 
										//wohl nicht mehr kommen kann
										nachbarn[i][a] = nSelEdgeTmp;
										//diese Edge braucht selbst keinen Ptr zu irgendwas, die sie eh nichts machen muss
									
										//gerade erzeugen
										createNSelGerade(nSelEdgeTmp,ceVerts,vertIter);
										
										stopSearch = true;
										continue;	//hier koennte eigentlich auch break hin
									}
									
		
									
									//jetzt noch die neue currentEdge holen und lastEdge aktualisieren
									
									MIntArray lastEdgeFaces;
									if(count == 0)
									{
										//diesen Ptr vormerken, so dass er später mit Ptr versehen werden kann
										
										//DEAKTIVIERT - UNSELEDGES SOLLEN KEINE VERBINDUNG ZU SEL EDGES HABEN (AUssER ENDEDGES)
									//	needsPtr = true;
									//	needsPtrArray[i*2+a] = nSelEdgeTmp;

										//nun wird den hier gespeicherten Ptr später der richtige Ptr ihrer selecteten
										//vorgängerEdge zugewiesen

									//	count++;
										//wenns erster durchlauf ist, muss darauf geachtet werden, dass lastEdge
										//selected edge ist bis dato ohne ptr
								
										//SPÄTER WIRD ENTSCHIEDEN; WER IN NSELEDGEARRAY KOMMT -> nu doch nich wegen UVs!
										nSelEdgeArray[i*2+a] = nSelEdgeTmp;

										createNSelGerade(nSelEdgeTmp,ceVerts,vertIter);
										
										//hier einfach die Faces der dafür geschaffenen variable zuweisen, damit sich der
										//code weiter unten um den rest kümmern kann
										
										lastEdgeFaces = nFaces;
										////cout<<"Habe lastEdgeFaces nFaces zugewiesen"<<endl;
									}
									else
									{
										edge*	lastEdge = localEdgePtrs[localEdgeIDs.length()-2];

										////cout<<"Will faces von lastEdge holen: ID "<<lastEdge->getID()<<endl;
										lastEdgeFaces = lastEdge->getFaces(); //-2, da der letzte eintrag nun zum 
										//ptr von der (noch) currentEdge führt
										////cout<<"Habe lastFaces von id "<<lastEdge->getID()<<endl;
						

										edge* thisEdge = nSelEdgeTmp;
										//jetzt müssen sich die edge noch gegenseitig die Ptr zuweisen 
										int seite, richtung;
										findEdgeNachbarMatrixIndices(thisEdge,lastEdge,seite, richtung);
										thisEdge->setNachbar(seite, richtung,lastEdge);
										
										findEdgeNachbarMatrixIndices(lastEdge,thisEdge,seite, richtung);
										lastEdge->setNachbar(seite,richtung,thisEdge);
									}

									
									//ansatz, wenn face0 nicht von lastEdge geteilt wird, kann man von ihm die nächste edgeableiten
									MPRINT("Will neue current/StartEdge festlegen")

									MIntArray tmpArray = helper.matchArray(ceFaces,lastEdgeFaces);
									helper.arrayRemove(ceFaces,tmpArray);


									MIntArray currentFaceEdges;
									polyIter.setIndex(ceFaces[0],tmp);
									
									polyIter.getEdges(currentFaceEdges);
									
									//es dürfte hier nur 2 matches geben: einer ist die neue current, einer die neue
									//lastEdge
									
									MIntArray match = helper.matchArray(connectedEdges[i],currentFaceEdges);
									

									if(match[0] == currentEdge)
									{
										currentEdge = match[1];
										lastEdge = match[0];
									}
									else
									{
										currentEdge = match[0];
										lastEdge = match[1];
									}

									//LASTEDGE: wird eigentlich nieee verwendet, aber ich lasses mal drin (gibt mir irgendwie
									//ein gefühl von sicherheit ;) )

									
									count++;
									
									
							}
							
							//jetzt noch festhalten, ob count == eins war
							if(count == 1)
								onlyOneNSelEdge[i*2+a] = true;
/*
								//wenn count groesser 1, dann ist nSelNachbar ungültig
								if(count > 1)
									nSelEdgeArray[i*2+a] = 0;
*/
								/*	//-> würde nicht funzen mit UVs
								//jetzt durch arcRatios parsen und die nSelEdge mit dem kleinsten Eintrag als nSelEdge eintragen
								//die Edge soll moeglichst orthogonal sein
								double smallest = 166666666;
								UINT smallestID = 0;
								for(x = 0; x < arcRatios.length(); x++)
								{
									INVIS(cout<<"ArcRatio für edge: "<<localEdgePtrs[x]->getID()<<" == "<<arcRatios[x]<<endl;);

									if(arcRatios[x] < smallest)
									{
										
										smallest = arcRatios[x];
										smallestID = x;
									}
								}
								*/

								//gültige nSelEdge eintragen in richtigen Slot
								//nSelEdgeArray[i*2+a] = localEdgePtrs[smallestID];

								//INVIS(cout<<" Gefundenen nSelEdge ist: "<<localEdgePtrs[smallestID]->getID()<<endl;);
							}		
						
						}
						
					}
					else
					{
						//HIER EDGE BEARBEITEN; WENN ES DIE ENDSEITE IST( In diesem Fall spielt es keine rolle  ob die 
						//nachbarEdges selected sind oder nicht, hier werden nur unselected edges erstellt
						MPRINT("Bearbeite EndEdge")
						//checken, ob matchEdge bereits ptr hat

						//jetzt bekomme ich zwar doppelt soviele edges am ende, allerdings handelt es sich um nSelEdges
						//die benoetigt werden für den fall das mehrere Edges eines faces angewählt sind
						/*
						l = edgeIDs.length();
						for(int x = 0; x < l; x++)
						{
							if(edgeIDs[x] == matchEdges[0])
							{
								connectedHasPtr = true;
								nachbarn[i][a] = edgePtrs[x];
								break;
							}
						}
						*/
						
					//	if(!connectedHasPtr)	//im gegenwärtigen Stadium ist der Flag eh immer false, da darauf garnicht getestet wird
					//	{
							MPRINT("ERZEUGE END EDGES")

							
							onlyOneNSelEdge[i*2+a] = true;

							//Daten sammeln (nur faces und vtx, normals werden nicht benoetigt)
							edgeIter.setIndex(matchEdges[0],tmp);
							
							MIntArray nSelFaces;
							edgeIter.getConnectedFaces(nSelFaces);
							
							int nSelVtx[2];
							nSelVtx[0] = edgeIter.index(0);
							nSelVtx[1] = edgeIter.index(1);
							
							//nur noch de endEdges dürfen auf array
							edge* thisEdge = new nSelEdge(matchEdges[0]);


							//auch diese endEdges benoetigen isFlippedInfos
							MIntArray tmpFaceVtx;
							bool	flipped[2] = {false,false};
							//jetzt bei den seFaces checken, ob die direction conform ist oder nicht - diese Werte dann der edge übergeben
							for(x = 0; x < nSelFaces.length(); x++)
							{
								polyIter.setIndex(nSelFaces[x],tmp);
								polyIter.getVertices(tmpFaceVtx);
								
								flipped[x] = !isDirConform(nSelVtx[0],nSelVtx[1],tmpFaceVtx);
							}
							
							thisEdge->setIsFlipped(flipped);
						
							//endEdges werden mit aufgenommen, damit sie später mit geloescht werden koennen
							edgePtrs.append(thisEdge);
							edgeIDs.append(matchEdges[0]);
							
							//Daten setzen
							thisEdge->setVtxIDs(nSelVtx);
							thisEdge->setFaceIDs(nSelFaces);
							
							nachbarn[i][a] = thisEdge;

							//gerade erzeugen, was es einer selectedEdge erlaubt, mit ihr zu intersecten
							createNSelGerade(thisEdge,nSelVtx,vertIter);

							//jetzt noch die nächste Edge in diese NSelEdge einfügen als referenz für die spätere 
							//faceErstellung
							if(nSelFaces.length() == 2)
							{
								for(unsigned int h = 0; h < nSelFaces.length(); h++)
								{
									for(unsigned int j = 0; j < nFaces.length();j++)
									{
										if(nSelFaces[h] == nFaces[j])
										{
											polyIter.setIndex(nSelFaces[1-h],tmp);
											break;
										}
									}
								}
								
								MIntArray containedEdges;
								polyIter.getEdges(containedEdges);
								
								MIntArray match = helper.matchArray(containedEdges,connectedEdges[i]);
								
								//array dürfte jetzt genau 2 sein
								//debug
								//if(match.length() != 2)
								////cout<<"MatchEdges war NICHT 2, sondern "<<match.length()<<endl;
								
								int refEdge;
								if(match[0] == matchEdges[0])
									refEdge = match[1];
								else
									refEdge = match[0];
								
								//neuen Ptr für refEdge erstellen
								edge* refEdgePtr = new nSelEdge(refEdge);
								
								int refVtx[2]; MIntArray refFaces;
								edgeIter.setIndex(refEdge,tmp);
								edgeIter.getConnectedFaces(refFaces);
								
								refVtx[0] = edgeIter.index(0);
								refVtx[1] = edgeIter.index(1);
								
								refEdgePtr->setVtxIDs(refVtx);
								refEdgePtr->setFaceIDs(refFaces);
								
								//jetzt den neuen Ptr zur letzten nSelEdge hinzufügen
								int s, r;
								findEdgeNachbarMatrixIndices(thisEdge,refEdgePtr,s,r);
								thisEdge->setNachbar(s,r,refEdgePtr);
								
								//diese nSelEdge bekommt keinen ptr zu irgendwem. Dies ist auch nicht noetig, da sie selbst
								//keine aktion ausführen wird
							}
					//	}
						
					}
					
			}
			
			
		}
		//debug
		/*
		//cout<<"Im nachbarnArray sind: "<<endl;
		//cout<<endl;
		//cout<<"Nachbarn Richtung 1, Face "<<nFaces[0]<<endl;
		if(nachbarn[0][0] != 0)//cout<<nachbarn[0][0]->getID()<<endl;
		if(nachbarn[1][0] != 0)//cout<<nachbarn[1][0]->getID()<<endl;
		//cout<<endl;
		if(nFaces.length() ==2)
			//cout<<"Nachbarn Richtung 2, Face "<<nFaces[1]<<endl;
		else
			//cout<<"Nachbarn Richtung 2, BorderEdge"<<endl;
		if(nachbarn[0][1] != 0)//cout<<nachbarn[0][1]->getID()<<endl;
		if(nachbarn[1][1] != 0)//cout<<nachbarn[1][1]->getID()<<endl;
		*/
		//cout<<"BIN VOR FINIHING"<<endl;
		//nun sind nachbarn erstellt, und die eigentliche edge kann erstellt/vervollständigt werden
		edge* thisEdge = 0;
		if(hasPtr)
		{
			//hier edges bearbeiten, die wingedVtx und Ptr haben
				//ANSATZ: wenn er hier hinkommt, ist bereits minimalPtr für diese edge erstellt wurden.
				//ausserdem wurde eine edge der wingedVtxedge in alledges belassen, damit der Ptr dank 
				//edgeIDs und allEdges gefunden und als "selected" identifiziert werden konnte.
				//hier ist jetzt nichts weiter zu tun, als die werte zu setzen.
				//die edgeRemovals wurden bereits von allEdges abgezogen
				//An dieser Stelle muss nicht unterschieden werden zwischen Winged und non winged
				


				thisEdge = existingPtr;
				
				//debug
				//if(thisEdge == 0)
				//	////cout<<"Warnung: Ptr zu ThisEdge wurde nicht gefunden"<<endl;

//				thisEdge->setWingedVtx(wingedVtx);
				thisEdge->setFaceIDs(nFaces);
				thisEdge->setVtxIDs(verts);
				thisEdge->setNachbarn(nachbarn[0][0],nachbarn[0][1],nachbarn[1][0],nachbarn[1][1]);
				
				thisEdge->setUIDs(uIDs);

				allEdgesBA.setBitFalse(allEdgesZero);

	
				createNonWingedGeomtry(thisEdge,verts,nFaces,meshFn, polyIter, vertIter);
				
			
		}
		else
		{
			
	
				
				edgeIDs.append(allEdgesZero);

				thisEdge =  new selEdge(nFaces,allEdgesZero);
				edgePtrs.append(thisEdge); 



				MIntArray tmpFaceVtx;
				bool	flipped[2] = {false,false};
				//jetzt bei den seFaces checken, ob die direction conform ist oder nicht - diese Werte dann der edge übergeben
				for(UINT p = 0; p < nFaces.length(); p++)
				{
					polyIter.setIndex(nFaces[p],tmp);
					polyIter.getVertices(tmpFaceVtx);
					
					flipped[p] = !isDirConform(verts[0],verts[1],tmpFaceVtx);
				}
				
				thisEdge->setIsFlipped(flipped);

				
				
				//restl Werte setzen
				
				thisEdge->setVtxIDs(verts);
				thisEdge->setNachbarn(nachbarn[0][0],nachbarn[0][1],nachbarn[1][0],nachbarn[1][1]);
				

				thisEdge->setUIDs(uIDs);

				createNonWingedGeomtry(thisEdge,verts,nFaces, meshFn,polyIter, vertIter);
				


				//allEdges.remove(0);
				allEdgesBA.setBitFalse(allEdgesZero);
		

		}

/*		
		//jetzt checken, ob einigen unselEdges noch ein Ptr zugewiesen werden muss
		if(needsPtr)
		{
			MPRINT("Will ptr für unSelEdge ergänzen")
			for(int g = 0; g < 4;g++)
			{
				if(needsPtrArray[g] != 0)
				{
					
					int seite, richtung;

					
					INVIS(cout<<"Habe eintrag gefunden: "<<needsPtrArray[g]->getID()<<" ThisSelEdge ist "<<thisEdge->getID()<<endl;)
					findEdgeNachbarMatrixIndices(needsPtrArray[g],thisEdge,seite, richtung);
					needsPtrArray[g]->setNachbar(seite,richtung,thisEdge);

				}

			}


		}
*/

		if((bool)options[8])
		{//wennnormalen verwendet werden, benoetigen die NSelEdges auch noch VtxInfos, damit sie als referenz dienen koennen
			//jetzt noch die VtxIDs setzen
			int nVtx[2];

			for(UINT t = 0; t < 4; t++)
			{
				if(nSelEdgeArray[t] != 0)
				{
					edgeIter.setIndex(nSelEdgeArray[t]->getID(), tmp);
					nVtx[0] = edgeIter.index(0);
					nVtx[1] = edgeIter.index(1);
					
					nSelEdgeArray[t]->setVtxIDs(nVtx);
				}
			}
		}



		thisEdge->setNSelNachbarn(nSelEdgeArray[0],nSelEdgeArray[1],nSelEdgeArray[2],nSelEdgeArray[3]);
		thisEdge->setOnlyOneSelNachbar(&onlyOneNSelEdge[0]);
		//ENDE While
		
		}


}


//-------------------------------------------
void		BPTfty::modifySelectionList()
//--------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------------------------------------
MDoubleArray	BPTfty::createGerade(int verts[],MItMeshVertex& vertIter)
//-----------------------------------------------------------------------------------------------------------------
{
	MDoubleArray result;
	int tmp;

	vertIter.setIndex(verts[0],tmp);
	MVector start = vertIter.position();

	vertIter.setIndex(verts[1],tmp);
	MVector end = vertIter.position();



	

	result.append(start.x);result.append(start.y);result.append(start.z);

	end = end - start;

	result.append(end.x);result.append(end.y);result.append(end.z);

	return result;
}

//wieder loeschen wenn fertig

//-----------------------------------------------------------------------------------------------------------------
MDoubleArray	BPTfty::createPlane(MDoubleArray& gerade, int face, MFnMesh& fnMesh, MItMeshPolygon& polyIter, MVector& normal)
//-----------------------------------------------------------------------------------------------------------------
{
	MDoubleArray result(6);
	int tmp;
	MPoint center;

	
	polyIter.setIndex(face,tmp);

	fnMesh.getPolygonNormal(face,normal);
	center = polyIter.center();
	
	MPoint start(gerade[0],gerade[1],gerade[2]);
	MVector direction(gerade[3],gerade[4],gerade[5]);
	
	//direction.normalize();
	MVector planeNormal = normal ^ direction;	//erstellt das crossProduct dieser RictungsVectoren

	planeNormal.normalize();



	//check, ob normal richtig herum
	if( (start + planeNormal).distanceTo(center) > (start - planeNormal).distanceTo(center))
	{
		//wenn end-normal näher dran als end+normal, muss normal gewendet werden
		planeNormal *= -1;
		//cout<<"habe planeNormal umgekehrt!"<<endl;
	}
	


	result[0] = start.x;	result[1] = start.y;	result[2] = start.z;
	result[3] = planeNormal.x;	result[4] = planeNormal.y;	result[5] = planeNormal.z;

	

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::createNonWingedGeomtry(edge* thisEdge,int verts[],MIntArray& faces,MFnMesh& fnMesh, MItMeshPolygon& polyIter, MItMeshVertex& vertIter)
//-----------------------------------------------------------------------------------------------------------------------------------------
{
	MVector normal;



	selEdge* realSelEdge = static_cast <selEdge*> (thisEdge);
	MDoubleArray gerade = createGerade(verts,vertIter);
	
	//damit man später nicht ständig checken muss, ob die gerade nun vorhanden (da winged) oder nicht, werden einfach alle
	// slots von geraden und plane mit identischen daten gefüllt. Reserviert wird der speicher ja sowieso
	realSelEdge->setSingleGerade(gerade);

	
	
	for(unsigned int a = 0; a < faces.length();a++)
	{
		MDoubleArray planeTmp = createPlane(gerade,faces[a],fnMesh,polyIter,normal);
		realSelEdge->setSinglePlane(a,planeTmp);
		realSelEdge->setNormal(normal);	
	}
	
	
}

//-----------------------------------------------------------------------------------------------------------------------------------------
void		BPTfty::createNSelGerade(edge* thisEdge,int verts[], MItMeshVertex& vertIter)
//-----------------------------------------------------------------------------------------------------------------------------------------
{
	
	nSelEdge* realnSelEdge = static_cast <nSelEdge* > (thisEdge);
	MDoubleArray gerade = createGerade(verts,vertIter);
				
	realnSelEdge->setGerade(gerade);

}



//---------------------------------------------------------
int			BPTfty::getUID(int face, MIntArray& nFaces,MItMeshPolygon& polyIter,int origID,int nID)
//---------------------------------------------------------
{
	//die beiden ids dienen der eindeutigen generierung der uID, besonders im fall, dass mehrere kanten an einer edge liegen
	MIntArray match[2];
	int tmp;
	


	MIntArray edges,nEdges;
	
	polyIter.setIndex(face,tmp);
	polyIter.getEdges(edges);
	
	//es kann bei nFacesLänge zu fehlern kommen, so dass es mehr als 2 sind
	//also sicherheitshalber cutten
	//->allerdings tritt dieser fehler nur suuuper selten auf, weshalb ich die rechenzeit lieber spare
	//(jetzt mal abgesehen davon dass dieser fehlercheck hier sowieso den fehler nur verschiebt, aber nicht behebt
	nFaces.setLength(2);

	for(unsigned int i = 0; i < nFaces.length(); i++)
	{
		polyIter.setIndex(nFaces[i],tmp);

		polyIter.getEdges(nEdges);

		match[i] = helper.matchArray(edges,nEdges);	//wenn match mindestens 2 ist, muss match sicherlich noch geordnet werden von klein nach gross
		

	}
	

	if(match[0].length() < match[1].length())
	{
		match[0] = match[1];
	}
		
	if(match[0].length() > 1)
	{
		////cout<<"HABE ARRAY SORTIERT"<<endl;
		helper.sortArray(match[0]);
	}
	
	//cout<<"RECHNUNG IST: "<<match[0][0]<<" + "<<origID<<" + "<<nID<<endl;
	
	return match[0][0]+origID+nID;
	


	//hierhin sollte er nie kommen
	//cout<<"BIN BEI GETUID - FORBIDDEN PATH!!!"<<endl;
	return -1;

}





//---------------------------------------------------------------------------------
void		BPTfty::schrinkVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter,MItMeshPolygon& polyIter, bool doSelect)
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
void		BPTfty::growVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter, MItMeshPolygon& polyIter,bool doSelect)
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
	helper.memoryArrayRemove(vtxArray,growedSelection);

	if(!doSelect)
		helper.addIntArrayToLHS(vtxArray,growedSelection);
	else
	//	selectComponents(adds,"vertices","add");
		selectComponents(growedSelection,"vertices","add");

	MPRINT("Habe Vtx gegrowed")



}
