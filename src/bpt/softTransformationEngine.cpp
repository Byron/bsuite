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

// softTransformationEngine.cpp: implementation of the softTransformationEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "softTransformationEngine.h"
#include <maya/MFnDagNode.h>

//////////////////////////////////////////////////////////////////////
// Statische Objekte
//////////////////////////////////////////////////////////////////////
/*
MObject	softTransformationEngine::distanceObj;		// Attribut fuer die TweakDistance	
MObject	softTransformationEngine::vfObj;			// VisualizeFalloffObject
MObject	softTransformationEngine::ftObj;			// FalloffTypeObj; Enum: Linear, Smooth, Spike - idealerweise - wird eventuell durch rampAttribute ersetzt, sobald die Mathematik dahinter klar ist
		

// SPEZIELL: MATRIX ATTRIBUT
// MObject softTransformationEngine::inMatrixObj;		// Huelt die TransformationsMatrix, welche im Fall der SoftTransformationNode fuer die Transformation sorgt


MObject softTransformationEngine::vtxOrigPosObj;

MObject softTransformationEngine::vtxSetObj;

MObject softTransformationEngine::outWeightsObj;

MObject softTransformationEngine::customSl;	
*/


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

softTransformationEngine::softTransformationEngine()
: mayCreateIdleEvent(true)
, animFnInitialized(false)
, lastDistance(-1.0)
, lastFType(50000U)
, lastMaxWeights(-1)
, lastSmoothen(-1)
, lastEdgeDistance(0)
{

}

softTransformationEngine::~softTransformationEngine()
{

}



//--------------------------------------------------------------------------------------
void	softTransformationEngine::setVtxSet(MDataBlock& data)
//--------------------------------------------------------------------------------------
{

	
	// Wenn sich die Topologie des Meshes veruendert, dann kann diese Methode verwendet werden
	
	uint numNewVtx = 0;
	uint initialLastID;

	MFnMesh meshFn(data.outputValue(outMesh).asMesh());	
				
	numNewVtx = meshFn.numVertices();
				
	meshFn.setObject(data.inputValue(inMesh).asMesh());
	initialLastID = meshFn.numVertices();

	numNewVtx = numNewVtx - initialLastID;


	// Array erzeugen
	MIntArray newIDs(numNewVtx);

	for(uint i = 0; i < numNewVtx; i++)
		newIDs[i] = initialLastID + i;


	setVtxSet(newIDs, data);
}


//--------------------------------------------------------------------------------
void	softTransformationEngine::gatherAttributeObjects(const MObject& inThisNode)
//--------------------------------------------------------------------------------
{
	// Wenn die attrObjects noch nicht gesetzt sind, dann dies jetzt nachholen
	if( outMesh.isNull() )
	{

		// Eigene NodeHandle setzen
		thisNode = inThisNode;


		MFnDependencyNode depFn( thisNode );
		
		inMesh = depFn.attribute("inMesh");
		outMesh = depFn.attribute("outMesh");

		inMatrix = depFn.attribute("matrix");

		vtxSetObj = depFn.attribute("vertexList");

		distanceObj = depFn.attribute("distance");

		ftObj = depFn.attribute("falloffType");

		vfObj = depFn.attribute("showWeights");

		vtxOrigPosObj = depFn.attribute("cachedVertexPositions");

		maxWeightsObj = depFn.attribute("maximumWeights");

		smoothenObj	= depFn.attribute("smoothen");

		edgeDistanceObj = depFn.attribute("edgeDistance");

		useMatrixRotationObj = depFn.attribute("useMatrixRotation");

		pushObj = depFn.attribute("push");

		pushRelativeObj = depFn.attribute("pushRelativeToFaceSize");

		meshMatrixObj = depFn.attribute("meshMatrix");

		vtxColorObj = depFn.attribute("aColor1");

		vtxColorObj2 = depFn.attribute("aColor2");

		pointSizeObj = depFn.attribute("pointSize");


	//	customSlO = depFn.attribute("customSlFalloff");
	}
	


}


//--------------------------------------------------------------------------------------
void	softTransformationEngine::setVtxSet(const MIntArray& newIDs, MDataBlock& data)
//--------------------------------------------------------------------------------------
{


		// CompList aufbauen
	MFnSingleIndexedComponent	compFn;
	MObject compfnObj = compFn.create(MFn::kMeshVertComponent);

	compFn.addElements( *(const_cast<MIntArray *>(&newIDs)) );	// Gemein - die AddElements Methode veruendert das MIntArray nicht, ist aber nicht const !


	// compData erzeugen
	MFnComponentListData compDataFn;
	MObject dataObj = compDataFn.create();

	
	compDataFn.add( compfnObj );



	// Jetzt das attribute setzen
	data.inputValue(vtxSetObj).set(dataObj);


}

//--------------------------------------------------------------------------------------
MStatus		softTransformationEngine::extractNonstaticData(const MPlug& plug, 
														   MDataBlock& data )
//--------------------------------------------------------------------------------------
{
	// Diese Methode extrahiert die Daten aus dem Datablock und speichert Sie lokal
	// Die Daten sind nur jene, welche der User veruendern kann.

	MStatus status;

	
	// Distance holen
	nd.distance = data.inputValue(distanceObj).asDouble();

 	// VisFlag wird direkt mit der VisNode verbunden

	// falloffType holen
	nd.fType = data.inputValue(ftObj).asShort();


	nd.fVis = data.inputValue(vfObj).asBool();



	// MaxWeights holen
	nd.maxWeights = data.inputValue(maxWeightsObj).asLong();



	// smoothen holen
	nd.smoothen = data.inputValue(smoothenObj).asFloat();

	// edgeDistance holen
	nd.edgeDistance = data.inputValue(edgeDistanceObj).asLong();


	// Bool useMatrixRotation holen
	nd.useMatrixRotation = data.inputValue(useMatrixRotationObj).asBool();

	// push holen
	nd.push = data.inputValue(pushObj).asDouble();

	// pushRelative holen
	nd.pushSizeRelative = data.inputValue(pushRelativeObj).asBool();

	// Matrix holen
	MFnMatrixData	matFn(data.inputValue(inMatrix).data());

	nd.matrix = matFn.matrix();

	// Checken ob die matrix ne connection hat

	// Man muss diese Methode verwenden, da das Statische Object wohl nur noch zur BPTIV node gehuert irgendwie
	MPlug matrixPlug( thisNode, inMatrix );
	
	
	isMatrixMode = matrixPlug.isConnected();

	INVIS(if(isMatrixMode){ );
	INVIS(cout<<"----------------------------------------- MATRIX ----------------------------------------------"<<endl);
	INVIS(cout<<"-----------------------------------------------------------------------------------------------"<<endl);
	INVIS(cout<<nd.matrix(0,0)<<"\t \t"<<nd.matrix(0,1)<<"\t \t"<<nd.matrix(0,2)<<"\t \t"<<nd.matrix(0,3)<<endl);
	INVIS(cout<<nd.matrix(1,0)<<"\t \t"<<nd.matrix(1,1)<<"\t \t"<<nd.matrix(1,2)<<"\t \t"<<nd.matrix(1,3)<<endl);
	INVIS(cout<<nd.matrix(2,0)<<"\t \t"<<nd.matrix(2,1)<<"\t \t"<<nd.matrix(2,2)<<"\t \t"<<nd.matrix(2,3)<<endl);
	INVIS(cout<<nd.matrix(3,0)<<"\t \t"<<nd.matrix(3,1)<<"\t \t"<<nd.matrix(3,2)<<"\t \t"<<nd.matrix(3,3)<<endl);
	INVIS(cout<<"----------------------------------------- MATRIX ----------------------------------------------"<<endl);
	INVIS(cout<<"-----------------------------------------------------------------------------------------------"<<endl);
	INVIS( } );
	// Jetzt noch das Mesh speichern
	nd.outMesh = data.outputValue(outMesh).asMesh();


	return status;
}



//--------------------------------------------------------------------------------------
MStatus		softTransformationEngine::extractStaticData(const MPlug& plug, MDataBlock& data)
//--------------------------------------------------------------------------------------
{
	// Diese Methode extrahiert die Daten aus dem Datablock und speichert Sie lokal
	// Die Daten sind nur jene, welche im Grunde nur nach der originalComputeMethod geholt werden muessen

	MStatus status;

	
	// vtxSet holen
	MFnSingleIndexedComponent compFn;
	MFnComponentListData	compDataFn(data.inputValue(vtxSetObj).data());

	if(compDataFn.length() == 1)
	{
		compFn.setObject(compDataFn[0]);
		compFn.getElements(vd.vtxSet);
	}

 	
	// Die gespeicherten InitialPositions der vtxParents holen
	MFnPointArrayData arrayFn(data.inputValue(vtxOrigPosObj).data());
	vd.vtxOrigPos = arrayFn.array(&status);

	
	INVIS(status.perror("Extrahiere VtxOrigPos") );
	INVIS(cout<<" HABE vtxORIGPos extrahier mit luenge von: "<<vd.vtxOrigPos.length()<<" vs Luenge vtxSet: "<<vd.vtxSet.length()<<endl);

	// und zuguterletzt das inMesh, welches nur zum lesen der origPos der TweakParents benutzt werden darf
	nd.inMesh = data.inputValue(inMesh).asMesh();
 

	
	return status;
}



//-----------------------------------------------------------------------------------------------------
bool	softTransformationEngine::rebuildTweakArrays()
//-----------------------------------------------------------------------------------------------------
{

	// MERKE


	// erstmal checken, ob alles legitim
								INVIS(cout<<endl;)
								INVIS(cout<<"------------------------------------------------"<<endl;)
								INVIS(cout<<"BAUE TWEAKARRAYS AUF"<<endl;)
								INVIS(cout<<"------------------------------------------------"<<endl;);



	MItMeshPolygon polyIter(nd.outMesh);
	MItMeshVertex vertIter(nd.outMesh);
	



	int  tmp;
	uint l,i;
	
	bool writeOutOrigPos = false;
	
	
	
	//erstmal alle arrays lschen

	if( nd.edgeDistance == 0 || lastEdgeDistance == 0 )	// Da edgeMode nen cache hat, werden die Daten nicht angeruehrt, wenn er aktiviert ist
		td.clearAll();

	


	//###########################################
	// POTENTIAL VTX BEARBEITEN
	//###########################################

	if(vd.potVtx.length() == 0 && nd.edgeDistance == 0)	// wenn eh der edgeMode verwendet wird, sind keine potentialVtx nuetig
	{

		// hier muss noch ne prozedur hin, die es ermueglicht, Elemente auszuschlieueen, 
		MItMeshVertex inVertIter(nd.inMesh);	
		
		l = inVertIter.count();		//da das inMesh verwendet wird, sind die neuen Vtx im Falle einer meshModifizierenden Node automatisch ausgeschlossen


		vd.potVtx.setLength(l);	//potentielle TweakVtx, 
		
		// array fuellen:
		for(i = 0; i < l; i++)
		{
			vd.potVtx[i] = i;
		}

		
		// Jetzt die vtxSet Vtx ausschlieueen, wenn die niedrigste ID im VtxSet kleiner inVertIter.count() ist - also die Topologie NICHT veruendert wurde
		// ( wie beim SMC )
		// Merke: vtxSet hat mindestens einen Eintrag, wenn er hier ist
		
		if( (uint)vd.vtxSet[0] < l )
		{
			helper.memoryArrayRemove(vd.potVtx, vd.vtxSet);
		}

	}



	
	//###########################################
	// VTX ORIG POS SETZEN
	//###########################################

	// Nun die originalPositionen der selektierten Vertizen holen, wenn nuetig
	// Wenn die SoftSelection vorher aus war oder die Zahl an origPos nicht mit der des vtxSets uebereinstimmt, dann  die origPos holen
	if(	 vd.vtxSet.length() != vd.vtxOrigPos.length() )
	{
		//einfach die gegenwuertigen Positionen der origVtx holen
		l = vd.vtxSet.length();

		vd.vtxOrigPos.setLength(l);

		for(i = 0; i < l; i++)
		{
			vertIter.setIndex(vd.vtxSet[i], tmp);

			vd.vtxOrigPos[i] = vertIter.position();
		}

		writeOutOrigPos = true;
	}




	INVIS(cout<<"PTVtx.length in Main = "<<vd.potVtx.length()<<endl;);


	 if(nd.edgeDistance == 0)
		createWeightsBasedOnDistance();
	else
		createWeightsBasedOnEdgeDistance();



	// Wenn push vorhanden ist, dann jetzt auf jeden Fall normalen neu erzeugen
	if(nd.push != 0.0)
		generateNormalVectors();


	INVIS(cout<<"ZAHL DER TWEAKS GESAMT: "<<td.vtx.length()<<endl;)
	

	return writeOutOrigPos;

}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::copyArrayPartial(MIntArray& copyTo, 
												   const MIntArray& copyFrom, 
												   uint inclMinID, 
												   uint exclMaxID)
//-----------------------------------------------------------------------------------------
{

	uint i, l = exclMaxID - inclMinID;

	copyTo.setLength(l);


	for( i = 0; i < l; i++, inclMinID++)
	{
		copyTo[i] = copyFrom[inclMinID];
	}

}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::resampleWeights()
//-----------------------------------------------------------------------------------------
{

	uint i, l, x, max = 0, min = 0;
	
	
	
	
	// Immer der kleinere von beiden
	uint myMaxCalculationLength = ( lastEdgeDistance > nd.edgeDistance ) ? nd.edgeDistance : lastEdgeDistance ;

	
	int add = (lastEdgeDistance < nd.edgeDistance) ? 1 : 0;	// nur wenn gegrowed wird, den letzten ring extra bearbeiten

	
	double	myMultiplier;		// cache fuer mutliplier

	// Auueerdem muessen die alten Weights angepasst werden an die neue edgeLength
	// a * b = c ---> c / b = a
	// - 1, weil der letzte ring immer 0 ist in diesem - diese Werte kann man nicht rekonstruieren, so dass man sie neu errechnen muss
	for(i = 0; i < myMaxCalculationLength - add; i++)
	{

		// Jetzt ring fuer ring die weights zurueckrechnen
		max += ec.ringCounts[i];

		// Der multiplikator rechnet auf 1 zurueck und multipliziert dann mit dem entsprechend neuen Wert
		myMultiplier = ( (double)lastEdgeDistance / (double)( lastEdgeDistance - i - 1) ) * ( (double) (nd.edgeDistance - i - 1)  / (double)nd.edgeDistance );


		for( ; min < max; min++ )
		{
			
			MDoubleArray&	weights = td.bWeights[min];
			
			
			l = weights.length();
			for(x = 0; x < l; x++)
			{
				
				weights[x] *= myMultiplier ;
				
			}
			
		}
		
	}
	

	// i hat jetzt den richtigen wert und zeigt auf den letzten (0) ring
	// min zeigt auf den ersten ringVtx, und max wird entsprechend gesetzt, spueter
	// Jetzt wie gewohnt die weights ausrechnen

	if( add == 1)
	{
		// vtxID array erzeugen:
		MIntArray outerRing;
		
		copyArrayPartial(outerRing, td.vtx, min, max + ec.ringCounts[i]);
		
		MItMeshVertex inVertIter(nd.inMesh);
		generateEdgeWeights(outerRing, i, inVertIter, min);
	}

}



//-----------------------------------------------------------------------------------------
void	softTransformationEngine::generateNormalVectors()
//-----------------------------------------------------------------------------------------
{
	// All normalen der tweakVtx cachen und die Grueuee des Faces kodieren als luenge der Normale selbst - sie sind also nicht normalisiert

	MItMeshPolygon	inPolyIter(nd.inMesh);
	MItMeshVertex	inVertIter(nd.inMesh);

	MIntArray	conFaces;

	uint i,x , l2, l = td.vtx.length();

	double	dTmp, aSize;				// aSize == AverageSize
	MVector tmpNormal, aNormal;			// aNormal == averageNormal

	int tmp;

	// Array einrichten
	td.normals.setLength(l);

	maxScale = 166666666;


	for( i = 0; i < l; i++ )
	{
		inVertIter.setIndex(td.vtx[i], tmp);

		inVertIter.getConnectedFaces(conFaces);


		// Alles 0 setzen fuer den nuechsten durchlauf
		aSize = 0.0;

		aNormal.x = 0.0;
		aNormal.y = 0.0;
		aNormal.z = 0.0;

		l2 = conFaces.length();
		for( x = 0; x < l2; x ++)
		{
			inPolyIter.setIndex(conFaces[x], tmp);

			inPolyIter.getArea(dTmp);
			aSize += dTmp;


			inPolyIter.getNormal(tmpNormal);
			aNormal += tmpNormal;
		}
		
		
		aSize /= l2;
		aNormal = aNormal.normal() * aSize;		// Die Normale wird erst normalisiert und dann skaliert mit size - so spart man sich einen extra normalScale Wert


		if(aSize < maxScale)
			maxScale = aSize;


		// zuweisen der Normale
		td.normals[i] = aNormal;

	}



	// Jetzt die ParentNormals holen

	l = vd.vtxSet.length();

	// Luenge anpassen, wenn nuetig
	if(l != td.pNormals.length() )
		td.pNormals.setLength(l);


	for( i = 0; i < l; i++ )
	{
		inVertIter.setIndex(vd.vtxSet[i], tmp);

		inVertIter.getConnectedFaces(conFaces);


		// Alles 0 setzen fuer den nuechsten durchlauf
		aSize = 0.0;

		aNormal.x = 0.0;
		aNormal.y = 0.0;
		aNormal.z = 0.0;

		l2 = conFaces.length();
		for( x = 0; x < l2; x ++)
		{
			inPolyIter.setIndex(conFaces[x], tmp);

			inPolyIter.getArea(dTmp);
			aSize += dTmp;


			inPolyIter.getNormal(tmpNormal);
			aNormal += tmpNormal;
		}
		
		
		aSize /= l2;
		aNormal = aNormal.normal() * aSize;		// Die Normale wird erst normalisiert und dann skaliert mit size - so spart man sich einen extra normalScale Wert


		if(aSize < maxScale)
			maxScale = aSize;


		// zuweisen der Normale
		td.pNormals[i] = aNormal;

	}


}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::generateEdgeWeights(	const MIntArray& vtxSet, 
														uint i, 
														MItMeshVertex& inVertIter, 
														int minID)
//-----------------------------------------------------------------------------------------
{

	MPoint startPoint;
	
	uint	x, 
			l2, 
			y, 
			l, 
			maxWeights;

	int		tmp;
	
	double	sumWeights, 
			newSumWeights, 
			dTmp, 
			inverse;
	

	l = vd.vtxSet.length();
	l2 = vtxSet.length();
	for( x = 0; x < l2; x++)
	{
		
		inVertIter.setIndex( vtxSet[x] ,tmp);
		startPoint = inVertIter.position();
		
		map<double, int>	weights;			// Speichert die weights (als key) zusammen mit den dazugehuerigen localPosIDs
		
		
		// Jetzt die Distanzen zu allen vtxSetMitgliedern ausrechnen und auf jeden Fall aufs weightArray hauen
		for(y = 0; y < l; y++)
		{
			weights.insert( wp( ( vd.vtxOrigPos[y] - startPoint).length(), y ) );
		}
		
		
		// Nun die Weigths entsprechend des maxWeigthsParams
		maxWeights = (weights.size() < (uint)nd.maxWeights) ? weights.size() : (uint)nd.maxWeights ; 
		
		
		
		
		
		// jetzt die weights extrahieren die pVtxPosIDs
		MDoubleArray* dWeights;
		MIntArray*	pVtxPosIDs;

		if( minID == -1)
		{
			// Aha, also sollen die arrays angehuengt werden - wir erzeugen neue Arrays im heap
			dWeights = new MDoubleArray(maxWeights);

			pVtxPosIDs = new MIntArray(maxWeights);

			
			// Man muss hier nicht checken, ob es weights gibt - einVtxSet mitglied gibts mindestens
			// TweakArrays updatan, aber nur, wenn man auch wirklich im AppendMode ist
			td.vtx.append(vtxSet[x]);
	
		}
		else
		{
			// Existierende arrays modifiezieren - also die Ptr mit referenzen bestuecken

			// minID ist in diesem Fall der Offset
			dWeights = &(td.bWeights[minID + x]);

			pVtxPosIDs = &(td.pVtxPosID[minID + x]);

		}

		
		std::map<double, int>::iterator	iter = weights.begin();
		
		sumWeights = newSumWeights = 0.0;
		
		
		
		
		// Pass 1: Daten uebertragen in arbeitsArray
		for( y = 0; y < maxWeights; y++, iter++)
		{
			(*pVtxPosIDs)[y] = iter->second;
			
			
			(*dWeights)[y] = iter->first;
			
			
			sumWeights += (*dWeights)[y];
			
		}
		
		
		
		
		// Pass 2: Vtx entsprechend ihrer relativen Distanzen zueinander: Vtx nueher am Parent sollen stuerker transformieren
		//	und alles anhand des gegenwuertigen edgeCounts skalieren
		if( maxWeights > 1 )
		{
			
			for( y = 0; y < maxWeights; y++)
			{
				dTmp = (*dWeights)[y];
				
				inverse = sumWeights - dTmp;
				
				dTmp = sumWeights / (dTmp / (pow( (float) inverse, nd.smoothen) ) );
				
				newSumWeights += dTmp;
				
				(*dWeights)[y] = dTmp;
			}
			
			
			
			
			
			// Pass 3:Die weights wieder runterskalieren mit newSum und sie entsprechend ihrer edgeDistance abschwuechen
			// hier kommt auch noch der scaleParameter zum tragen - er skaliert die dustanz - je weiter weg, desto stuerker ist sein effekt
			for( y = 0; y < maxWeights; y++)
			{
				dTmp = (*dWeights)[y];
				
				(*dWeights)[y] = dTmp = ( dTmp / newSumWeights ) * ( (double)(nd.edgeDistance - i - 1) / (double) nd.edgeDistance ) ;
			}
			
		}
		else
		{
			
			(*dWeights)[0] = 1.0  * ( (double)(nd.edgeDistance - i - 1) / (double) nd.edgeDistance ) ;
			
		}
		
		
		
		// zuguterletzt die heapArrays  lueschen, wenn nicht auf referenzen gearbeitet wurd
		if(minID == -1)
		{
			// zuguterletzt alles hinzufuegen
			td.bWeights.push_back(*dWeights);
			
			td.pVtxPosID.push_back(*pVtxPosIDs);

			delete dWeights;

			delete pVtxPosIDs;
		}
	}

}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::createWeightsBasedOnEdgeDistance()
//-----------------------------------------------------------------------------------------
{
	// VARIABLEN
	uint l , i;	// Fuer Iterationen
	

	
	MPoint startPoint;	// cache fuer position des potVtx


	MIntArray	vtxSet[2];	// 2 IntArrays, welche abwechselnd die originalVtx halten und die mit ihnen verbundenen
	uint		cvs = 0;	// currentVtxSet - entweder 0 oder 1; 	
	uint		ocvs = 1;	// Other VtxSet - ist immer 1 - cvs, cache

	
	
	MItMeshVertex	inVertIter(nd.inMesh);	
	MItMeshVertex	vertIter(nd.outMesh);

	componentConverter	converter(nd.outMesh);				// convertiert die vtxSelection zu faces, und danach zu den ensprechenden Vtx, ohne doppelte Eintruege

	BPT_BA		forbiddenVtx(vertIter.count(), true);		// Huelt die Vtx, welche bereits bearbeitet wurden und folglich nicht nochmal konvertiert werden sollen


	// Wenn ec bereits initialisiert, dann arbeiten wir mit cache und die forbidden Vtx muessen alle TwerakVtx sein



	// ######################
	// Vtx WEIGHTS AUFBAUEN
	// ######################

	int difference =  (int)nd.edgeDistance - (int)lastEdgeDistance;


	if( difference == 0 )
//	if(true)		
	{
		// Da in diesem Falle die Zahl der Weights geuendert wurde oder die distance, muss 
		// alles neu aufgerollt werden -> jetzt den Weg dafuer ebnen
		ec.ringCounts.setLength(0);

		lastEdgeDistance = 0;

		td.clearAll();

		difference = 1;

	}


	// Die originalVtx muessen in jedem fall verboten werden
	forbiddenVtx.setBits(vd.vtxSet, false);
	
	
	if( ec.ringCounts.length() > 0 )
	{
		// Wenn wir nen cache haben, dann muessen auch noch alle tweakVtx auf die forbiddenList
		forbiddenVtx.setBits(td.vtx, false);
	}
	



	if( difference > 0 )
	{// Es wird gegrowed
		
		
		// Der erste Durchlauf erfolgt mit den konvertierten vtx des VtxSets
		if( ec.ringCounts.length() > 0 )
			copyArrayPartial(vtxSet[0], td.vtx, td.vtx.length() - ec.ringCounts[ec.ringCounts.length() - 1], td.vtx.length());
		else
			vtxSet[0] = vd.vtxSet;

	


		l = vd.vtxSet.length();
		

		
		// jetzt edgeLength mal iterieren
		for( i = lastEdgeDistance; i < nd.edgeDistance; i++)
		{
			converter.vtxToConnectedFaceVtx(vtxSet[cvs], vtxSet[ocvs]);
			
			// jetzt die forbiddenVtx ausschlieueen
			vtxSet[ocvs] = forbiddenVtx & vtxSet[ocvs];
			
			// jetzt die uebriggebliebenen aufs vorbiddenArray packen
			forbiddenVtx.setBits(vtxSet[ocvs], false);
			
			// Nun die Distanzen ermitteln und die Weights, pro gefundenem Vtx 
			
			generateEdgeWeights(vtxSet[ocvs], i, inVertIter);


			// Auueerdem noch den Cache updaten mit der Zahl an Eintruegen fuer diesen Ring
			ec.ringCounts.append(vtxSet[ocvs].length());

			
			
			// zuguterletzt die Arrays switchen, so dass der neue Startpunkt das letzte Ergebnis ist, 
			// sozusagen der letzte Ring, welcher aus der Selection gegrowed wurde
			cvs = 1 - cvs;
			ocvs = 1 - cvs;

			

		}

		// Weights anpassen
		if(lastEdgeDistance != 0)
			resampleWeights();

	}
	else if(difference < 0)
	{
		// Es muss geshrinked werden

		// Die bedeutet, dass die letzten, nun veralteten ringe geluescht werden muessen, danach resampled
		// Auueerdem muss man den ec updaten
		uint delIDRange = 0;			// zusammenfassung aller IDs,welche gelueschgt werden sollen
		for( i = lastEdgeDistance - 1; i > nd.edgeDistance - 1; i--)
		{
			 delIDRange += ec.ringCounts[i];

		}

		
		// jetzt die letzten eintruege lueschen aus baseWeigths und potVtxIDs
		td.bWeights.resize(td.bWeights.size() - delIDRange);

		td.pVtxPosID.resize(td.pVtxPosID.size() - delIDRange);

		// Die tweakVtx selbst muessen natuerlich auch noch geluescht werden 
		td.vtx.setLength(td.vtx.length() - delIDRange);
		
		// Luenge anpassen
		ec.ringCounts.setLength(ec.ringCounts.length() - (lastEdgeDistance - nd.edgeDistance) );


		
		// Noch die weights anpassen - diese methode geht korrekt damit um
		if(nd.edgeDistance != 0)
			resampleWeights();

	}



}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::createWeightsBasedOnDistance()
//-----------------------------------------------------------------------------------------
{
	
	uint x, l, l2, i;
	int tmp;

	// VARIABLEN
	double  dTmp; 
	MPoint startPoint;
	
	MItMeshVertex inVertIter(nd.inMesh);
	
	unsigned maxWeights;		// Speichert die tatsuechlich zahl an maxWeights zwischen . mindestens td.maxWeights, oder weniger
	double	sumWeights;		// Tmpvar fuer Summe der Weights;
	double	newSum;
	MDoubleArray distScales;		// DistanceScale - je weiter weg, desto schwuecher ist das weight - pro weight
	


	//#########################################################################################################################################
	// WEIGHTS AUSRECHNEN - MERKE: Weights hat immer numVtx eintruege - fuer die VisNode - und eventuell zum painten spueter
	//#########################################################################################################################################
	
	
	l = vd.potVtx.length();	  l2 = vd.vtxSet.length();
	
	
	
	INVIS(cout<<"REBUILT TWEAK ARRAYS: MUSS "<<l * l2<<" RECHNUNGEN AUSFueueHREN"<<endl);
	
	
	
	for( i = 0; i < l; i++)
	{
	
		// Hier ist wichtig, dass alle Berechnungen auf dem originalMesh bzw. mit gecachten Positionen erfolgen
		
		//vertIter.setIndex(vd.potVtx[i],tmp);
		inVertIter.setIndex(vd.potVtx[i],tmp);
		startPoint = inVertIter.position();
		
		map<double, int>	weights;			// Speichert die weights (als key) zusammen mit den dazugehuerigen localPosIDs
		
		sumWeights = 0.0;
		
		//distancen ausrechnen: hier eventuell noch ne adaptivitueuet einbauen: z.B nur jeden 2. vtx ind SlideIndices einbeziehen
		for(x = 0; x < l2; x++)
		{//nun ist die Frage, ob es schneller ist, den vertIter zu strapazieren, oder sich die Position einfach zu berechnen ... 
			//vertIter.setIndex(vd.vtxSet[x],tmp);
			
			
			
			dTmp = (vd.vtxOrigPos[x] - startPoint).length();
			
			
			// Bereits hier wird aussortiert: Wenn die Distance immernoch innerhalb der toleranz, dann in die Map packen
			if( dTmp <= nd.distance )
			{
				weights.insert( wp( dTmp , x) );	// Den wert hier nicht veruendern - die kleinsten Werte sollen zuerst kommen
			}
			
		}
		
		
		
		// Es soll die grueuetMuegliche zahl an weights genommen werden
		maxWeights = ((uint)nd.maxWeights < weights.size() ) ? (uint)nd.maxWeights : weights.size();
		
		
		if(maxWeights > 0 )
		{
			// erstmal diesen TweakVtx hinzufuegen
			td.vtx.append(vd.potVtx[i]);
			
			
			
			
			
			// jetzt die weights extrahieren die pVtxPosIDs
			MDoubleArray dWeights(maxWeights);
			MIntArray	pVtxPosIDs(maxWeights);
			
			std::map<double, int>::iterator	iter = weights.begin();
			
			
			
			sumWeights = newSum = 0.0;
			distScales.setLength(maxWeights);
			
			// Zuerstmal die benuetigten MaxScaleDaten holen
			for( x = 0; x < maxWeights; x++, iter++)
			{
				dWeights[x] =  nd.distance - iter->first;
				//	dWeights[x] /= nd.distance ;//* maxWeights;	// Umkehren
				distScales[x] = dWeights[x] / nd.distance;
				
				
				// newSum += iter->first;
				
				//distScale += dWeights[x] / nd.distance;
				
				sumWeights += dWeights[x];
				
				pVtxPosIDs[x] = iter->second;
				
			}
			
			
			// jetzt die weights entsprechend ihrer Nuehe zum parent modifizieren: Parents nah am TweakVtx werden so viel stuerker
			for( x = 0; x < maxWeights; x++)
			{
				
				// dWeights[x] = dWeights[x] / (sumWeights / distScales[x] );
				dTmp = dWeights[x];
			//	dWeights[x] =  ( sumWeights  / ( (nd.distance - dTmp ) / ( dTmp * dTmp) ) )  *  ( ( 1 / dTmp ) * nd.distance);
				dWeights[x] =  ( sumWeights  / ( ((nd.distance - dTmp ) / (pow((float)dTmp, nd.smoothen))) / ( pow((float)dTmp, nd.smoothen) ) ) )  ;
				newSum += dWeights[x];
				
			}
			
			// Normalisieren und mit distScale versehen
			for( x = 0; x < maxWeights; x++)
			{
				dWeights[x] = (dWeights[x] / newSum) * distScales[x];
			}
			
			
			// Die erzeugten Arrays jetzt aud die Vectoren hauen
			td.bWeights.push_back(dWeights);
			
			td.pVtxPosID.push_back(pVtxPosIDs);
			
		} // if( maxWeights > 1 ) ENDE
		
		
		
	}
	//for(element in ptVtx) ENDE


}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::initAnimFn()
//-----------------------------------------------------------------------------------------
{

	MObjectArray objects;
	
	
	MFnDependencyNode depNodeFn(thisNode);
	
	
	if(MAnimUtil::findAnimation(depNodeFn.findPlug("csl"),objects))
	{
		animFn.setObject(objects[0]);
	}

}


//-----------------------------------------------------------------------------------------
void	softTransformationEngine::recalculateTweakScaleFactors( )
//-----------------------------------------------------------------------------------------
{
	uint l = td.vtx.length(), numWeights;
	uint i,x;

	if(! animFnInitialized )
	{
		// Erstmal die animFn einrichten
		initAnimFn();
		animFnInitialized = true;
	}

	
	
	INVIS(cout<<"Berechne Falloff neu, fType ist "<<nd.fType<<endl;)
		//Scale neu ausrechnen
		switch(nd.fType)
		{
		case 0:
			{//linear
				td.weights = td.bWeights;
				break;
			}
		case 1:
			{
				//smooth		
				clearAnimCurve();
				
				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				
				animFn.setAngle(0,MAngle(0.0),false);
				animFn.setAngle(1,MAngle(0.0),true);
				
				
				break;
			}
		case 2:
			{
				//Spike
				
				
				clearAnimCurve();
				
				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				
				animFn.setAngle(0,MAngle(0.0),false);
				animFn.setAngle(1,MAngle(0.093349),true);
				
				
				
				break;
			}
		case 3:
			{
				//Dome
				
				
				clearAnimCurve();
				
				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				
				animFn.setAngle(0,MAngle(0.093349),false);
				animFn.setAngle(1,MAngle(0.0),true);
				
				
				break;
			}
			
			
			
	}


	// Hier wird nur weitergemacht, wenn auch wirklich resampled werden muss
	if(nd.fType != 0)
	{

		// Wir unterscheiden der effizienz wegen 2 Modi: Bei dem einen muss das WeightsArray vuellig neu aufgebaut werden, beim anderen
		// Kann die vorhanden eintruege extrahieren

		if(		lastDistance == nd.distance
			&&	
				lastMaxWeights == nd.maxWeights
			&&	
				lastEdgeDistance == nd.edgeDistance
			&&
				lastSmoothen == nd.smoothen)
		{	// Die zahl der Eintruege hat sich nicht veruendert, also auf vorhandenen Daten arbeiten
			// Wenn er hier hinkommt, dann hat sich nur der FType veruendert

			for( i = 0; i < l; i++)
			{
				MDoubleArray& bWeights = td.bWeights[i];
				MDoubleArray& weights = td.weights[i];
				
				
				numWeights = bWeights.length();

				// Die beiden Arrays sollten auch noch gleich groue sein
				
				for( x = 0; x < numWeights; x++)
				{
					animFn.evaluate(bWeights[x],weights[x]);
				}
				
				// Da wir auf einer Referenz gearbeiten haben, ist hier nichts mehr zu tun;
			}
		}
		else
		{
			// Weight erstmal leer machen
			td.weights.clear();
			
			for( i = 0; i < l; i++)
			{
				MDoubleArray& bWeights = td.bWeights[i];
				
				
				
				numWeights = bWeights.length();
				
				MDoubleArray weights(numWeights);		// Wird dann auf weightsVector gepushed
				
				for( x = 0; x < numWeights; x++)
				{
					animFn.evaluate(bWeights[x],weights[x]);
				}
				
				td.weights.push_back(weights);
			}
		}
	}
	

}


//---------------------------------------------------------
void		softTransformationEngine::clearAnimCurve()
//---------------------------------------------------------
{
	unsigned int count = animFn.numKeys();

	for(unsigned int i = 0; i < count; i++)
	{
		animFn.remove(0);
	}
}


//--------------------------------------------------------------------------------------
void	softTransformationEngine::saveMeshPathes()
//--------------------------------------------------------------------------------------
{
	if(outMesh.isNull())
	{
		MFnDependencyNode depFn(thisNode);

		inMesh = depFn.attribute("inMesh");
		outMesh = depFn.attribute("outMesh");
		inMatrix = depFn.attribute("matrix");
	}
	

	MPlugArray connections;
	MPlug thisOutMesh(thisNode, outMesh);
	thisOutMesh.connectedTo(connections,false, true);

	// Da die nuechste Node die visMeshnode ist, muss man noch eine stufe weiter gehen
	if( connections.length() == 0 )
		return;
	
	connections[0].connectedTo(connections, false, true);
	if( connections.length() == 0 )
		return;
	
	MFnDagNode dagfn( connections[0].node() );
	dagfn.getPath( meshPath );
	
	meshTransform = meshPath;
	meshTransform.pop();	
}

//--------------------------------------------------------------------------------------
void	softTransformationEngine::doTweak(MDataBlock& data)
//--------------------------------------------------------------------------------------
{
	

	// Diese Methode kuemmert sich um die gesamte Transformation des Meshes

	// Momentan gibt es 2 Modi: - Transformation mithilfe einer Matrix - quasi wie ein cluser
	//							- Transformation mithilfe von BeFwegungsvektoren

	// Wenn sich die weights veruendern, werden diese sogleich in den Datablock geschrieben


	// Wenn aus irgendeinem Grund keine BaseVtx vorhanden, einfach raushier
	if( vd.vtxSet.length() == 0)
		return;


	// VARIABLEN
	MFnMesh	fnMesh(nd.outMesh);
	
	MItMeshVertex vertIter(nd.outMesh);
	MItMeshPolygon polyIter(nd.outMesh);

	MItMeshVertex inVertIter(nd.inMesh);
	MItMeshPolygon inPolyIter(nd.inMesh);

	uint  l,i,x;
	int tmp;


	// Wenn die neue Distance kleiner ist als die alte wird es weniger tweakVtx geben, 
	// so dass man erstmal alle TweakVtx zuruecksetzen muss	
	// Oder wenn die edges hinzugeschaltet werden
	if(		nd.distance < lastDistance 
		|| 
			( (nd.distance > 0) & (lastEdgeDistance == 0) & (nd.edgeDistance > 0)  ))
	{
		l = td.vtx.length();
		for( i = 0; i < l; i++)
		{
			vertIter.setIndex(td.vtx[i], tmp);
			inVertIter.setIndex(td.vtx[i], tmp);
			
			vertIter.setPosition(inVertIter.position());
		}
		
	}


	// -----------------------------
	// TweakArrays neu aufbauen/uendern
	// -----------------------------

	
	





	// Wenn sich distanz veruendert hat. dann die weights neu berechnen
	if(		lastDistance != nd.distance 
		||
			lastMaxWeights != nd.maxWeights
		||
			lastSmoothen != nd.smoothen
		||
			lastEdgeDistance != nd.edgeDistance
		||
			vd.vtxSet.length() != vd.vtxOrigPos.length() )
	{//Die TweakArrays mueuessen neu aufgebaut werden
		

		// Wenn die edgeDistance auf 0 gestellt wurde, dann den Cache lueschen
		if( (nd.edgeDistance == 0) & (lastEdgeDistance > 0) )
		{
			
			ec.ringCounts.clear();
		}


		//wenn die Distanz 0 ist, dann suemtliche TempArrays lueschen, um speicher zu sparen
		if(nd.distance == 0.0 && nd.edgeDistance == 0)
		{
			// ZUERST alle Vtx wieder auf ursprungort setzen

			td.clearAll();
			vd.potVtx.setLength(0);

			lastDistance = nd.distance;
			lastFType = nd.fType;
			lastMaxWeights = nd.maxWeights;
			lastSmoothen = nd.smoothen;
			lastEdgeDistance = nd.edgeDistance;

			// Damit beim nuechsten mal die neuen VtxPositionen geholt werden
			vd.vtxOrigPos.clear();

			nd.fVis = false;

			return;
		}
		else
		{
		
			if(	rebuildTweakArrays( ) )
			{
				// aha, also die vd.pos in datenBlock schreiben, damit auch alles ordentlich gespeichert werden kann
				MFnPointArrayData	arrayFn;

				data.inputValue(vtxOrigPosObj).set(arrayFn.create( vd.vtxOrigPos ) );

			}
		
		// schlussendlich die TweaksScales neuberechnen
			recalculateTweakScaleFactors( );

		}
		
		
		// Jetzt muessen die Aktualisieren outWeights noch gesetzt werden in Datablock
		// Sie dienen als input fuer die eventuell aktivierte VisNode

		// Man muss bedenken, dass pro tweakVtx mehrere Weights existieren kuennen, so dass diese erst gemittelt werden muessen
		
		// Alles 0 setzen
		//
		wa = MDoubleArray(vertIter.count(), 0.0 );
		uint numWeights;
		double aw;	// == averageWeight

		l = td.vtx.length();
		for( i = 0; i < l; i++)
		{
			MDoubleArray& weights = td.weights[i];

			numWeights = weights.length();
			aw = 0.0;

			for(x = 0; x < numWeights; x++)
			{
				aw += weights[x];
			}

			wa[ td.vtx[i] ] = aw ;/// numWeights;

		}




		
		lastDistance = nd.distance;
		lastFType = nd.fType;
		lastMaxWeights = nd.maxWeights;
		lastSmoothen = nd.smoothen;
		lastEdgeDistance = nd.edgeDistance;

	}
		
	// hat sich der fallofftype veruendert?
	if(lastFType != nd.fType)
	{
		recalculateTweakScaleFactors();

		lastFType = nd.fType;


	}
	

	// -------------------------
	// MATRIX TRANSFORMATION
	// -------------------------

	nd.matrix = nd.matrix.transpose();
	l = vd.vtxSet.length();
	if(isMatrixMode)
	{
		// Einfach mal die parentVtx anhand der Matrix transformieren
		for( i = 0; i < l; i++)
		{
			vertIter.setIndex(vd.vtxSet[i], tmp);

			vertIter.setPosition( ( nd.matrix * vd.vtxOrigPos[i] )  ); 
		}


		if( nd.useMatrixRotation )
		{
		
			
			
			l = td.vtx.length();
			for(i = 0; i < l; i++)
			{
				vertIter.setIndex(td.vtx[i], tmp);
				inVertIter.setIndex(td.vtx[i], tmp);
				
				vertIter.setPosition( inVertIter.position() + ((nd.matrix * inVertIter.position()) - inVertIter.position()) * wa[td.vtx[i]] );
			}

			return;
		}

	}
	



	// -----------------------------
	// Direction werden gecached
	// -----------------------------

	MVectorArray directions(l);
	

	for( i = 0; i < l; i++)
	{
		vertIter.setIndex(vd.vtxSet[i], tmp);

		directions[i] = vertIter.position() - vd.vtxOrigPos[i];
	}





	MVector direction;
	int tweakVtx;
	uint numWeights;




	// PUSH VARIABLEN
	void (softTransformationEngine::*careAboutPush)(MVector& direction, uint index) ;




	// -----------------------------
	// PUSH FUNKTIONEN SETZEN
	// -----------------------------

	// Wenn push vorhanden ist, wird die funktion entsprechend gesetzt, so dass nicht fuer jeden TweakVtx derselbe check gemacht werden muss
	if( nd.push != 0.0)
	{
		// Sichergehen, dass auch eintruege vorhanden sind, was nicht zwangsweise sein muss
		if( td.normals.length() != td.vtx.length() )
		{// Aha, also welche erzeugen
			generateNormalVectors();
		}


		// NormalMode festlegen, und gleich die methode setzen
		if(nd.pushSizeRelative)
		{
			careAboutPush = &softTransformationEngine::doSizeRelativePush;

			// jetzt die origVtx transformieren
			for(i = 0; i < l; i++)
			{
				vertIter.setIndex(vd.vtxSet[i], tmp);
				

				direction = MVector::zero;
				pDoSizeRelativePush(direction, i);
				vertIter.setPosition(vertIter.position() + direction);
			}
		}
		else
		{
			careAboutPush = &softTransformationEngine::doStandardPush;

			// jetzt die origVtx transformieren
			for(i = 0; i < l; i++)
			{
				vertIter.setIndex(vd.vtxSet[i], tmp);
				

				direction = MVector::zero;
				pDoStandardPush(direction, i);
				vertIter.setPosition(vertIter.position() + direction);
			}
		}

	}
	else
		careAboutPush = &softTransformationEngine::doNoPush;




	l = td.vtx.length();

	// -----------------------------
	// VERSCHIEBEN
	// -----------------------------

	for( i = 0; i < l; i++)
	{
		
		tweakVtx = td.vtx[i];

		inVertIter.setIndex(tweakVtx , tmp);
		vertIter.setIndex(tweakVtx , tmp);

		MDoubleArray&	weights = td.weights[i];
		MIntArray&		pVtxPosIDs = td.pVtxPosID[i];
		
		
		
		numWeights = weights.length();

		// Direction zuruecksetzen
		direction = MVector::zero;

		// MERKE: Die weights sind bereits so skaliert, dass man hier nicht mehr dividieren muss
		// Dieser abschnitt ist wirklich NUR fuer die erzeugung und die translation der direction anhand der weights zustuendig und fuer sonst nix
		// Jetzt fuer jedes weight die direction addieren ... 
		for( x = 0; x < numWeights; x++)
		{

			direction += weights[x]  * directions[ pVtxPosIDs[x] ];

		}

		
		(this->*careAboutPush)(direction , i);

		// Nun die Direction dividieren, um den entgueltigen wert zu erhalten
		vertIter.setPosition(inVertIter.position() + direction );
		
	}
	


}


//-----------------------------------------------------------------------------------
softTransformationEngine::meshStatus	
softTransformationEngine::getMeshStatus()
//-----------------------------------------------------------------------------------
{


		MDagPath visNodePath = meshPath, transform;
		


		transform = visNodePath;
		transform.pop();	//transform

		
		MSelectionList activeList;
		//momentan checken wir nur, ob das Mesh selected ist oder nicht
		
		MGlobal::getActiveSelectionList(activeList);


		if( ( activeList.hasItem(transform.node()) || activeList.hasItem( transform ) ) )
			return kSelected;
		


		MGlobal::getHiliteList(activeList);

		if( ( activeList.hasItem(transform.node()) || activeList.hasItem( transform ) ) )
			return kHilited;



 		return kNone;

	//	Nur zum testen
	//	return kSelected;

}


// --------------------------------------------------
bool		
softTransformationEngine::nodeStatusAllowsDrawing()
// --------------------------------------------------
{
	
	// Checken, ob diese Node auf der selectionList ist
	//
	MSelectionList activeList;
		//momentan checken wir nur, ob das Mesh selected ist oder nicht
		
	MGlobal::getActiveSelectionList(activeList);



	if( activeList.hasItem(thisNode) )
	{
		// die Node ist gewuehlt - also auf jeden fall zeichnen
		//
		return true;
	}


	// Die node ist nicht gewuehlt - ist sie denn ganz oben in der history ?
	//
	MPlug plug ( thisNode, outMesh );

	MPlugArray	connections;

	plug.connectedTo(connections, false, true);

	if(connections.length() )
	{
		// Auueerdem muss das mesh gehilited sein, damit das funktioniert
		//
		MSelectionList	hl;
		MGlobal::getHiliteList( hl );
		MDagPath tPath = meshPath;
		
		// Wir brauchen die Transform der shape als path
		//
		tPath.pop();

		if( connections[0].node() == meshPath.node()  && hl.hasItem( tPath ) )
			return true;
		
	}


	return false;


}



//-----------------------------------------------------------------------------------
MColor 
softTransformationEngine::getCalColor(const MColor& color1, 
									  const MColor& color2, 
									  double weight)
//-----------------------------------------------------------------------------------
{

	return ( color1 * weight + (1.0 -  weight)  * color2 );

}



// --------------------------------------------
void	
softTransformationEngine::draw(M3dView& view)
// --------------------------------------------
{

	// Sollen wir ueberhaupt zeichnen ?
	// 
	if( !nd.fVis || ( (nd.distance == 0.0) & (nd.edgeDistance == 0)  ) )
		return;


	// Duerfen wir zeichnen ?
	//
	if( ! nodeStatusAllowsDrawing() )
		return ;




	bool listNeedsUpdate = false;


	softTransformationEngine::meshStatus mStat = getMeshStatus();

	// Das mesh muss gewuehlt sein, damit man was zeuchnen darf
	//
	if( mStat == kNone )
		return;

	// Wenn sich der Anezigstatus des Meshes uendert, dann muss die DisplayList neu erstellt werden
	// Nur um den PolygonOffset zu aktualisieren
	// Diese procedur muss im endeffekt nur herausfinden, ob das mesh gehilited ist oder selected, weil der
	// PolyOffset dann entsprechend angepasst werden muss
	//
	if(mStat != dd.mstat)
	{
		lastStat = mStat;

		listNeedsUpdate = true;
	}


	dd.mstat = mStat;




	//Okay, also die restlichen Daten extrahieren	

	//Lokale Variablen
	//
	float	fPointSize;


	//PointSize holen
	//
	MPlug	plug(thisNode, pointSizeObj);
	plug.getValue(fPointSize);
	

	bool	allowBeautyMode = false;

	// Jetzt das Plug auslesen um herauszufinden, ob man schuen zeichnen darf oder nicht
	// TODO

	

	//Farbe holen	-> Diese Schreibweise hat den (hier unbedeutenden) Vorteil, dass Objekte automatisch zerstuert werden - sie existieren nur innerhalb der Klammern
	{
		MColor tmpColor(MColor::kRGB, 0.0f, 0.0f, 0.0f);
		

		plug.setAttribute(vtxColorObj);
		MObject colorObj;
		plug.getValue(colorObj);
		
		
		MFnNumericData	numDataFn(colorObj);
		numDataFn.getData(tmpColor.r, tmpColor.g, tmpColor.b);

		//wenn sich die Farben veruendert haben, dann die liste updaten - und alles neuzeichnen
		if(tmpColor != dd.vtxColor1 )
		{
			dd.vtxColor1 = tmpColor;

			listNeedsUpdate = true;
		}

		plug.setAttribute(vtxColorObj2);
		plug.getValue(colorObj);
		numDataFn.setObject(colorObj);

		numDataFn.getData(tmpColor.r, tmpColor.g, tmpColor.b);

		if(tmpColor != dd.vtxColor2)
		{
			dd.vtxColor2 = tmpColor;
			//dd.vtxColor2 = view.colorAtIndex(9);
			
			listNeedsUpdate = true;
		}

	}


	
	// Erstmal die meshMatrix holen, damit die Points auch an der richtigen Stelle erscheinen
	//

	glMatrixMode( GL_MODELVIEW );

	// CameraMatrix holen
	//
	double	meshMatrix[4][4];
	MDagPath	cam;
	view.getCamera(cam);

	cam.inclusiveMatrix().inverse().get(meshMatrix);

	
	// Neue Matrix holen
	//
	glPushMatrix();

	glLoadMatrixd( (GLdouble*) meshMatrix);


	// Jetzt die matrix des meshes selbst holen
	//
	MObject mmatrix;
	plug.setAttribute(meshMatrixObj);	

	plug.getValue(mmatrix);

	MFnMatrixData matrixFn(mmatrix);

	
	matrixFn.matrix().get( meshMatrix );

	glMultMatrixd( (GLdouble* ) meshMatrix );





	M3dView::DisplayStyle style = view.displayStyle();

//	view.beginGL(); 
//	view.beginOverlayDrawing(); 
	
	
	//Im PointMode werden PointsGezeigt, und wenn das Objekt selected ist (dann funzt shaded nicht mehr)
	if( style == M3dView::kPoints || (mStat == kSelected ) || !allowBeautyMode)
	{

		//VertiIter initialisieren
		MItMeshVertex vertIter(nd.outMesh);
		
		drawPoints(vertIter, fPointSize);
		

	}
	else
	{//in diesem Modus werden Faces gezeichnet, mit entsprechenden alphawerten als zeichen ihrer Farbe
		
		MItMeshPolygon polyIter(nd.outMesh);
		MItMeshVertex	vertIter(nd.outMesh);
		


		//Die displayList pruefen
		if( listNeedsUpdate )//|| lastDMode != style )
		{	//neue Liste erzeugen - wird eigentlich nur gemacht, wenn sich was veruendert hat
			//
			
		//	lastDMode = style;

			if( dd.list != 450000 ) 
				//alte liste lueschen
				glDeleteLists(dd.list, 1);
			
			dd.list = glGenLists(1);

			//if(glIsList(dd.list))
			if( dd.list != 0 )
			{
				listNeedsUpdate = false;


				//#######################
				//NEUE LISTE AUFZEICHNEN
				//#######################
				glNewList(dd.list, GL_COMPILE_AND_EXECUTE);

		
				//drawShaded(polyIter, vertIter, style, mStat);
				drawShadedTriangles(polyIter, vertIter, style, mStat);


				//#######################
				//ENDE LISTE AUFZEICHNEN
				//#######################

				glEndList();

			}
			else
			{
				//fehler, also alles ohne displayList zeichnen
				//drawShaded(polyIter, vertIter, style, mStat);
				drawShadedTriangles(polyIter, vertIter, style, mStat);
			}

		}
		else 
		{
			glCallList(dd.list);

		}

	}

	glPopMatrix();



}




//-----------------------------------------------------------------------------------
void	
softTransformationEngine::drawPoints( MItMeshVertex& vertIter, float fPointSize)
//-----------------------------------------------------------------------------------
{

// Push the color settings
			// 
			glPushAttrib( GL_CURRENT_BIT | GL_POINT_BIT );
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glEnable ( GL_POINT_SMOOTH );  // Draw square "points"
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			


			MColor tmpColor(MColor::kRGB, 0.0f, 0.0f, 0.0f);
			double dTmp;		// huelt das weight, cache

			//weights verwenden um Farbe zu skalieren
			uint numCVs = vertIter.count();
			for (uint i = 0; i < numCVs; i++, vertIter.next()) 
			{

				dTmp = wa[i];

				glPointSize( (fPointSize * dTmp) + 3.0 );

				glBegin( GL_POINTS );	

			//	glPointSize( fPointSize );
				MPoint		point( vertIter.position() );
				
				
			
				
	

				tmpColor = getCalColor(dd.vtxColor1, dd.vtxColor2, dTmp );
				glColor4f(tmpColor.r,tmpColor.g,tmpColor.b, dTmp );
				glVertex3f( (float)point.x, (float)point.y, (float)point.z);

				glEnd();
			} 
			
			
			
			glPopAttrib();

}



//-------------------------------------------------------------------
void	
softTransformationEngine::drawShadedTriangles(	MItMeshPolygon& polyIter, 
												MItMeshVertex& vertIter, 
												M3dView::DisplayStyle style, 
												meshStatus meshStat)
//-------------------------------------------------------------------
{

				//alles zeichnen
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				
				
				glEnable(GL_POLYGON_OFFSET_FILL);
				
				
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);
				
				glPolygonMode(GL_BACK, GL_FILL);
				glShadeModel(GL_SMOOTH);
				
				
				
				//Dass muss ausueerhalb der displayList bleiben, weil dieser Wert nicht precompiliert werden darf
				float param1 = 0.45, param2 = 0.55;

				// im DebugMode werden die Params anhand der NodeParameter gesetzt

				if(style == M3dView::kWireFrame)
				{

						param1 = 0.45; param2 = 0.55;

				}
				else
				{
					switch(meshStat)
					{
					case kNone:
						{
							param1 = -0.5; param2 = -0.6;

							break;
						}
					case kHilited:
						{
							param1 = 0.45; param2 = 0.55;
							
							break;
						}
					default: break;
					}
				}



				glPolygonOffset( param1, param2 );
				
				//jedes Poly zeichnen
				
				uint numPolys = polyIter.count();
				uint i, x , l;
				MPoint	point;
				MColor	tmpCol(MColor::kRGB, 0.0f, 0.0f, 0.0f);
				MPointArray triPoints;
				MIntArray	triVtx;
				//glColor4f(0.0f, 0.0f, 1.0f, 0.2);
				
				for(i = 0; i < numPolys; i++, polyIter.next())
				{
					

					polyIter.getTriangles(triPoints, triVtx);
					
					
					l = triVtx.length();
					
					
					glBegin(GL_TRIANGLES);
					
					for(x = 0; x < l ; x+=3)
					{
						//view.setDrawColor( vtxColor *  vtxWeightArray[ polyVtx[x] ]);
						//glColor4f(0.0f, 0.0f, 1.0f, vtxWeightArray[ polyVtx[x] ]);
						
						tmpCol = getCalColor(dd.vtxColor1, dd.vtxColor2 ,  wa[ triVtx[x] ]);
						glColor4f(tmpCol.r, tmpCol.g,tmpCol.b, wa[ triVtx[x] ]);
						glVertex3d(triPoints[x].x, triPoints[x].y, triPoints[x].z);
						
			
						tmpCol = getCalColor(dd.vtxColor1, dd.vtxColor2 ,  wa[ triVtx[x+1] ]);
						glColor4f(tmpCol.r, tmpCol.g,tmpCol.b, wa[ triVtx[x+1] ]);
						glVertex3d(triPoints[x+1].x, triPoints[x+1].y, triPoints[x+1].z);

						tmpCol = getCalColor(dd.vtxColor1, dd.vtxColor2 ,  wa[ triVtx[x+2] ]);
						glColor4f(tmpCol.r, tmpCol.g,tmpCol.b, wa[ triVtx[x+2] ]);
						glVertex3d(triPoints[x+2].x, triPoints[x+2].y, triPoints[x+2].z);

					}
					
					glEnd();
					
				}
				
				
			//	glDisable(GL_POLYGON_OFFSET_FILL);

				glPopAttrib();


}
