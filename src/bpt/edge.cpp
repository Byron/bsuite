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

// edge.cpp: implementation of the edge class.
//
//////////////////////////////////////////////////////////////////////

#include "edge.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

edge::edge(int inID)
	: id(inID)
	, clean(false)
{
	//sicherheitshalber alle nachbarn mit 0 initialisieren
	nachbarn[0][0] = 0;
	nachbarn[0][1] = 0;
	nachbarn[1][0] = 0;
	nachbarn[1][1] = 0;

		//faceDaten 0 setzen
	faceData[0] = 0;
	faceData[1] = 0;
}


edge::edge(MIntArray inFaceIDs,int inID)
	: faceIDs(inFaceIDs)
	, id(inID)
	, clean(false)
{
	nachbarn[0][0] = 0;
	nachbarn[0][1] = 0;
	nachbarn[1][0] = 0;
	nachbarn[1][1] = 0;

	//faceDaten 0 setzen
	faceData[0] = 0;
	faceData[1] = 0;
}


edge::~edge()
{

}


edgeMeshCreator*			edge::creator = 0;
double						edge::maxLength = (0.0);	//wird dann von InitEdges routine gesetzt



BPT_BA						edge::origVtxDoneBA;

std::list<edgeFaceData*>	edge::MAINFaceDataPtrs;
std::list<endFaceData*>		edge::endFacePtrs;
std::list<nSelEdgeData*>	edge::nSelEdgeDataArray;

ULONG						edge::side,edge::dir;



//////////////////////////////////////////////////////////////////////
//ZUGRIFFSMETHODEN
//////////////////////////////////////////////////////////////////////



//------------------------------------------
void	edge::setVtxIDs(int inVtxIDs[])
//------------------------------------------
{
	vtxIDs[0] = inVtxIDs[0];
	vtxIDs[1] = inVtxIDs[1];
}

//-------------------------------------------------
void	edge::setNachbarn(edge* inNachbarn00,edge* inNachbarn01,edge* inNachbarn10,edge* inNachbarn11)
//-------------------------------------------------
{
	nachbarn[0][0] = inNachbarn00;
	nachbarn[0][1] = inNachbarn01;
	nachbarn[1][0] = inNachbarn10;
	nachbarn[1][1] = inNachbarn11;
}

//---------------------------------------------------------
void edge::setNachbar(int seite,int richtung, edge* nachbarPtr)
//---------------------------------------------------------
{
	//ueueberschreibschutz test
	if(nachbarn[seite][richtung] == 0)
		nachbarn[seite][richtung] = nachbarPtr;
}





//---------------------------------------------------------
void edge::setThisNachbarZero(edge* thisNachbar)
//---------------------------------------------------------
{
	
	if(nachbarn[0][0] == thisNachbar)
	{
		nachbarn[0][0] = 0;
		return;
	}
	if(nachbarn[0][1] == thisNachbar)
	{
		nachbarn[0][1] = 0;
		return;
	}


	if(nachbarn[1][0] == thisNachbar)
	{
		nachbarn[1][0] = 0;
		return;
	}
	if(nachbarn[1][1] == thisNachbar)
	{
		nachbarn[1][1] = 0;
		return;
	}
}


//------------------------------------------
void	edge::setFaceIDs(MIntArray&	inFaceIDs)
//------------------------------------------
{
	faceIDs = inFaceIDs;
}



//-----------------------------
MIntArray	edge::getFaces() const
//-----------------------------
{
	return faceIDs;
}

//-----------------------------
MIntArray	edge::getVtx() const
//-----------------------------
{
	return MIntArray(vtxIDs,2);
}
	


//------------------------------------------------------
edge* edge::findOppositeRichtung(edge* lastPtr)
//------------------------------------------------------
{
	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(lastPtr == nachbarn[i][a])
			{
				return nachbarn[i][1-a];
			}
		}
	}

	//hierHin sollte er nie kommen!
	return 0;
}


//--------------------------------------------------------------------------------------------------------------
bool		edge::borderThere(int requesterID,edge* lastEdge )
//--------------------------------------------------------------------------------------------------------------
{

	//clean = false; //Ob sich eine Edge aktiviert oder nicht hueuengt jetzt vom smallFaceClean wert ab

	if(id == requesterID)
	{//wir sind wieder am anfang angekommen - normalerweise kueuennte man jetzt die eigentlich position ermitteln, allerdings
		//kueuennte es auch sein, dass man hierher nie kommt da der loop durch borders gestoppt wurde. Aus dem grund macht das
		//die aufrufende prozedur

		return false;
	}

	if(!isSelected() && faceIDs.length() == 2)
		return false;
	else if(faceIDs.length() == 1)
		return true;

	
	int s, r;
	edge* opposite = findOppositeRichtung(lastEdge, s, r);

	if(opposite == 0)
	{

		return true;

	}
	else
	{
		
		return opposite->borderThere(requesterID,this);
	}

	//forbiddenPath
	return false;

}

//------------------------------------------------------------------------------
edge*	edge::findOppositeRichtung(edge* lastPtr,int& seite,int& richtung)
//------------------------------------------------------------------------------
{
	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(lastPtr == nachbarn[i][a])
			{
				seite = i; richtung = 1-a;
				return nachbarn[i][1-a];
			}
		}
	}
	
	//hierHin sollte er nie kommen!
	seite = richtung = 0;
	return 0;

}


//-----------------------------------------------------------------------------------------------------------------------
void		edge::findMatrixIndices(edge* origPtr,edge* localPtr,int& seite,int& richtung)
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
	
}


#ifdef DEBUG

//-----------------------------------------------------------------
void		edge::printArray(MIntArray& array, MString message)
//-----------------------------------------------------------------
{
	for(unsigned int i = 0; i < array.length(); i++)
		cout <<array[i]<<message.asChar()<<endl;

	cout <<"//printArray Ende"<<endl;
	cout <<endl;
	cout <<endl;
	cout <<endl;
	
}

//--------------------------
void	edge::printAllData()
//--------------------------
{
	cout<<"------------------------------------------------------"<<endl;
	cout<<"Drucke Daten fueuer EDGE: <<<<   "<<id<<"   >>>>"<<endl;
	cout<<"------------------------------------------------------"<<endl;
	cout<<endl;
	cout<<"Vtx sind: "<<vtxIDs[0]<<" - "<<vtxIDs[1]<<endl;

	cout<<endl;
	cout<<endl;
//	cout<<"Winged Vtx sind: "<<endl;
//	for(int i = 0; i < wingedVtx.length();i++)
//		cout<<wingedVtx[i]<<endl;
	cout<<"NORMALEN ZAHL: "<<normals.length()<<endl;

	cout<<endl;
	cout<<"Nachbarn Richtung 1, Face "<<faceIDs[0]<<endl;
	if(nachbarn[0][0] != 0)cout<<nachbarn[0][0]->getID()<<endl;
	if(nachbarn[1][0] != 0)cout<<nachbarn[1][0]->getID()<<endl;
	cout<<endl;
	if(faceIDs.length() ==2)
		cout<<"Nachbarn Richtung 2, Face "<<faceIDs[1]<<endl;
	else
		cout<<"Nachbarn Richtung 2, BorderEdge"<<endl;
	if(nachbarn[0][1] != 0)cout<<nachbarn[0][1]->getID()<<endl;
	if(nachbarn[1][1] != 0)cout<<nachbarn[1][1]->getID()<<endl;
	cout<<"******************************************************"<<endl;
	cout<<endl;
}

#endif



//-------------------------------------
void		edge::initFaceData(int extraDirection)
//-------------------------------------
{

	//MERKE: es kann sein, das diese prozedur zur herstellung von faces fueuer nSel edges verwendet wird.
	//in dem fall darf "dir" nicht verueuendert werden, und man muss das die ueuebergebene direction nehmen
	

	int myDir;

	
	if(extraDirection == -1)
		myDir = dir;
	else
		myDir = extraDirection;

	INVIS(cout<<"INITIALISIERE FACE: "<<faceIDs[myDir]<<endl;);

	

	//jetzt noch faceData Erstellen
	int thisFaceID = faceIDs[myDir];
	faceData[myDir] = new edgeFaceData(thisFaceID);

	//neue Einheit auf die HauptDump packen
	MAINFaceDataPtrs.push_back(faceData[myDir]);


	
	


	creator->getFaceVtxIDs(thisFaceID, faceData[myDir]->faceVertices);

	INVIS(printArray(faceData[myDir]->faceVertices, " = faceDataVERTICES"));
	//creato checkt, ob normalen ueueberhaupt gewueuenscht
	creator->getFaceNormalIDs(thisFaceID, faceData[myDir]->normalIndices);




	//arrays allokalisieren, wenn UVs verwendet werden
	if(creator->numUVSets > 0)
	{
		faceData[myDir]->UVRelOffsets = (bool*)malloc(creator->numUVSets);
		memset(faceData[myDir]->UVRelOffsets, 0, creator->numUVSets);

		faceData[myDir]->UVAbsOffsets = (unsigned long*)malloc(creator->numUVSets * 4);
		memset(faceData[myDir]->UVAbsOffsets, 0, creator->numUVSets * 4);
	}
	
	
	creator->getFaceUVs(thisFaceID, faceData[myDir]->UVIndices, faceData[myDir]->UVRelOffsets );
	
	//jetzt noch die UVAbsOffsets erstellen
	UINT l = faceData[myDir]->faceVertices.length();
	UINT count = 0;
	
	UINT i;
	for( i = 0; i < creator->numUVSets; i++)
	{
		faceData[myDir]->UVAbsOffsets[i] = count;

		count += faceData[myDir]->UVRelOffsets[i] ? l : 0;
	}



	//zuguterletzt noch allen nachbarn dieser Richtung den Ptr abgeben, aber nur wenn dieses Face geteilt wird und der nachbar selected ist
	edge* nEdge;

	for( i = 0; i < 2; i++)
	{
		nEdge = nachbarn[i][myDir];

		if( nEdge != 0 )
		{
			//diese prozedur prueueft selbstueuendig, ob das ueuebergebene Face geteilt ist, und weist nur dann den Ptr zu
			//ergo: ich muss hier nichts prueuefen
			nEdge->setThisFaceData(thisFaceID, faceData[myDir]);

		}
	}

}

//--------------------------------------------------------------------------
void		edge::setThisFaceData(int faceID, edgeFaceData* inFaceData)	
//--------------------------------------------------------------------------
{
	for(UINT i = 0; i < faceIDs.length(); i++) 
	{
		//nach dieser Methode wird leider auch der auftraggeber noch mal aufgerufen, sich das face anzueignen
		//was unsinn ist ;)
		if(faceIDs[i] == faceID && faceData[i] == 0)
		{
			faceData[i] = inFaceData;
			
			//jetzt noch den eigenen Nachbarn dieser Seite bescheid sagen
			//hierbei wird auch der eigentliche auftraggeber angesprochen, 
			//dieser bricht jedoch ab wegen der faceDate[i] == 0 klausel
			//(sein faceData Slot ist dann schon ungleich 0)
			for(UINT r = 0; r < 2; r++)
			{
				if( nachbarn[r][i] != 0 )
					nachbarn[r][i]->setThisFaceData(faceID, inFaceData);
			}
			break;
		}
	}
}


//---------------------------------------------------------------------------------------
int			edge::C_fixNewVtxUV(int origVtx,int refVtx, edgeFaceData* faceData) 
//---------------------------------------------------------------------------------------
{
	if(!faceData->hasUVs())
	{
		return origVtx;
	}
	
	//es muss auch dafueuer gesorgt werden, dass die entsprechenden UVs eingetragen werden
	//diese mueuessen unbedingt vorn ins Array eingefueuegt werden, damit RecreateFaces richtig funktioniert (welches ja einfach durch vtxChange geht und IDs austauscht)
	MIntArray vtxChangeBak, UVChangeBak;
	vtxChangeBak = faceData->vtxChange;
	UVChangeBak = faceData->UVChange;

	//einen Unterschied ,machen, ob der fehlende Vtx durch einen neuen oder einen alten Vtx gestellt werden soll
	//ALLERDINGS : dies darf nur geschehen, wenn de entsprechende nachbar NICHT selected ist
	//UND: Das extraverhalten darf nicht gezeigt werden, wenn man hier von insertVtx aufgerufen wurde, da der refVtx dann -1 sein kann

	if(refVtx > creator->getInitialVtxCount() || !nachbarn[side][0]->isSelected())
	{	
		//jetzt vtx und UVChange neu schreiben
		faceData->vtxChange.setLength(2);
		faceData->vtxChange[0] = origVtx;
		faceData->vtxChange[1] = origVtx;
		
		//jetzt das Backup hinten ranhueuengen
		creator->helper.addIntArrayToLHS(faceData->vtxChange, vtxChangeBak);
		
		//nun dasselbe mit UVChange
		MIntArray UVIDs(creator->numUVSets * 2);
		MIntArray origUVs = faceData->getEdgeVtxUVs(origVtx);
		
		for(UINT i = 0; i < creator->numUVSets; i++)
		{
			UVIDs[i*2] = origUVs[i];
			UVIDs[i*2+1] = origUVs[i];
		}
		
		//jetzt das UVCahnge array ersetzen
		faceData->UVChange = UVIDs;
		
		//und nun noch das Backup hinten ranhueuengen
		creator->helper.addIntArrayToLHS(faceData->UVChange, UVChangeBak);
		
		return origVtx;
	}
	else
	{//es handelt sich um einen durch den Nachbarn verschobenen origVtx, so dass man jetzt einen neuen Vtx erzeugen muss an Stelle des origVtx
		//die neuen Datan muessen als newVtx eingetragen werden an den Anfang, weil ja ein tmpFace erzeugt werden muss mit den entsprechenden infos aus den 2
		//(normalerweilse vorhandenen faceDatas)
		
		int thisNewVtx = edge::creator->newVtx(creator->getVtxPoint(origVtx));

				//jetzt vtx und UVChange neu schreiben
		faceData->vtxChange.setLength(2);
		faceData->vtxChange[0] = thisNewVtx;
		faceData->vtxChange[1] = thisNewVtx;
		
		//jetzt das Backup hinten ranhueuengen
		creator->helper.addIntArrayToLHS(faceData->vtxChange, vtxChangeBak);
		
		//nun dasselbe mit UVChange
		MIntArray UVIDs(creator->numUVSets * 2);
		MIntArray origUVs = faceData->getEdgeVtxUVs(origVtx);
		
		MFloatArray origUVPos;
		edge::creator->getUVPositions(origUVs, origUVPos);

		MIntArray newUVs = edge::creator->createUVs(origUVs, origUVPos);
		

		for(UINT i = 0; i < creator->numUVSets; i++)
		{
			UVIDs[i*2] = newUVs[i];
			UVIDs[i*2+1] = newUVs[i];
		}
		
		//jetzt das UVCahnge array ersetzen
		faceData->UVChange = UVIDs;
		
		//und nun noch das Backup hinten ranhueuengen
		creator->helper.addIntArrayToLHS(faceData->UVChange, UVChangeBak);

		

		//fertig
		return thisNewVtx;
	}

}

//---------------------------------------------------------------------------------------
void		edge::addVtxChange( int newVtx, edge* orsl )
//---------------------------------------------------------------------------------------
{

	//gilt nur fueuer "newVtx"

	//Diese Methode wird nur gerufen, wenn 
	//a)									der Vtx wirklich Neu ist - also zum ersten mal erstellt wurde //dies wird aueueerhalb gecheckt
	//b)									die benachbarten edges kein Face teilen (so dass sie 2 separate datenfaces haben
	//c)									wenn die nur ein nachbar methode gerufen wird, muss interpoliert werden zwischen 2 samples fueuer newVtx

	// kueuemmert sich auch um die Berechnung der UVs

	//fueuegt die Vertizen, die aus Intersection zwischen zwei edges entstanden sind, in Array ein
	
	//erstmal eine echte SelEdge erstellen
	
	bool	shareFace = false;	//flag: Teilen sie ein Face oder nicht?

	MIntArray nFaces = orsl->getFaces();
	UINT i;
	for(i = 0; i < nFaces.length(); i++)
	{
		if(nFaces[i] == faceIDs[dir])
		{
			shareFace = true;
			break;
		}
	}


	bool existed = (newVtx == vtxIDs[side]) ? true : false; 


	//zuerst den originalVtx und den NewVtx verarbeiten

	//bei dieser prozedur ist orsl IMMER eine nachbarEdge(selected) - diese muss im Falle von !shareFace ebenfalls einen Vtx erhalten

	//VERTIZEN BEARBEITEN

	//zuersteinmal neuehn Vtx zu diesem Face hinzufueuegen
	faceData[dir]->vtxChange.append(vtxIDs[side]);
	faceData[dir]->vtxChange.append(newVtx);

	int nDir = -1;	//hueuelt die dir ueuequivalente Richtung (zu dir) auf der anderen edge (orsl), ist nur definiert, wenn shareFace = false

	if(!shareFace /*& !existed*/)	//Vtx auf keinen Fall doppelt hinzufueuegen //wenn der Vtx bereits exisiterte, muss einfach nur ein Slide errechnet werden
					//MERKE: der neue Vtx soll trotzdem zum face hinzugefueuegt werden, selbst wenn es eigentlich kein neuer Vtx ist (->Chamfer), damit man spueueter die UVs ohne extraChecks holen kann
	{
		int uid = (this->getUIDs())[side*2+dir];
		const int* nuID = orsl->getUIDs();
		//jetzt die korrekte richtung fueuer orsl herausfinden, indem man durchs nachbarArray parsed und ptr vergleicht
		for(int i = 0; i < 2; i++)
		{
			for(int a = 0; a < 2; a++)
			{
				if(orsl->nachbarn[i][a] == this && nuID[i*2+a] == uid )
				{//die gefundenen Richtung ist korrekt, also das face in dieser richtung aktualiesieren
					nDir = a;
					
					//man sollte hier davon ausgehen, dass die Ptr noch nicht initialisiert wurden
					if(orsl->faceData[nDir] == 0)
						orsl->initFaceData(nDir);

					orsl->faceData[nDir]->vtxChange.append(vtxIDs[side]);
					orsl->faceData[nDir]->vtxChange.append(newVtx);

					goto outOfIf;		
				}
			}
		}
	}
	
outOfIf:;


	
	//UVS BEARBEITEN
//	if( creator->numUVSets > 0 )
//	{
		

		
		//VORBEREITUNG DER GEOMETRIEDATEN

		int seite[2] = {side,-1},richtung[2] = {dir,-1};	//seite/richtung, von this und orsl aus gesehen
	
		int origVtx = vtxIDs[side];		//wird als referenz benueuetigt 


		edgeFaceData*	tmpFace[2] = {0, 0};

		if(!orsl->isSelected())
			shareFace = true;


		if(shareFace )	//kommt in seltenen Situationen vor, dass orsl nicht gewuehlt ist
		{//also wird nur this und orsl verwendet

			tmpFace[0] = tmpFace[1] = faceData[dir];
		}
		else
		{//folglich muss es auch mindestens eine unSelEdge zwischen ihnen (this/orsl) geben, welche dann stattessen verwendet werden muss
		

			orsl->getValidRichtung(this,seite[1], richtung[1], (getUIDs())[side*2+dir] );	//wenn es NICHT nurEinNachbar ist, dann ist UID eh -1 ueueberall, und es funzt trotzdem
			//hier wird mit der unSelEdge begonnen, von orsl aus


			tmpFace[0] = faceData[dir];
			tmpFace[1] = orsl->faceData[richtung[1]];


		}



		UINT faceCount = 2-(int)shareFace, f;	//facecount - wenn sie ein face teilen, dann ist er 1




		

		
		

		MIntArray validator[2];	//->jedes Face erhueuelt einen Validator, : hueuelt info pro UVSet: -1 == keine UVs, , 0 == hat UV, 1 == hat UV und teilt diese mit dem anderen Face
		MFloatArray newUVPos[2];//hueuelt de entsprechnenden UVPositionen pro Face ( U gefolgt von V) pro UVSet
		


		//diese Arrays sind immer vollstueuendig gefueuellt, weshalb man ihre Grueueueuee mit einem Mal setzen kann
		for(f = 0; f < faceCount; f++)
		{
			validator[f].setLength(creator->numUVSets);
			
			newUVPos[f].setLength(creator->numUVSets * 2);

		}




		bool hasUVs[2] = {false, false};

		for(f = 0; f < faceCount; f++)
		{
	
			
	
			UINT vr = 0;
			
			for(i = 0; i < creator->numUVSets; i++)
			{
				if(tmpFace[f]->UVRelOffsets[i])
				{
					validator[f][vr++] = 0;
					hasUVs[f] = true;		
				}
				else
				{
					validator[f][vr++] = -1;
				}
			}

			if(!hasUVs[f])
			{
				continue;
			}

			

			getUVPos((newVtx <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx) : creator->getVtxPoint(newVtx),
						tmpFace[f], 
							newUVPos[f]);

			
			

		}

		
		//jetzt die UVPos des der origUVs holen, pro face
		MIntArray origUVs[2];
		MFloatArray origUVPos[2];	//Hueuelt die Positionen des origUVs
		
		for(f = 0; f < faceCount; f++)
		{

			origUVs[f] = tmpFace[f]->getEdgeVtxUVs(origVtx);
			
			creator->getUVPositions(origUVs[f], origUVPos[f]);
		}
		


		//jetzt die eigentlichen Ergebnisse verarbeiten
		if(shareFace)
		{//sie teilen ein Face, also kann man jetzt einfach die neuen UVs erstellen und den Slide hinzufueuegen 
			
			//wenn dieses Face keine UVs hat, dann gleich abbrechen > in diesem Fall wird spueueter auch keiner darauf zugreifen
			if(!hasUVs[0])
				return;

			//Erstellung
			MIntArray newUVIds;

			if(!existed)
			{//dies ist der SC Fall - der neue Vtx ist anders als vtxIDs[side]
				newUVIds = creator->createUVs(validator[0], newUVPos[0]);
				
				//Slide
				creator->addUVSlide(newUVPos[0],origUVPos[0], newUVIds);
			}
			else
			{//dies KANN ein chamfer fall sein, wenn ein originalVtx verschoben wurde, anstatt einen neuen Vtx and dieser Position zu erstellen
				//einfach die alten UVs als neue UVs angeben, und slide hinzufueuegen
				newUVIds = origUVs[0];

				creator->addUVSlide(newUVPos[0],origUVPos[0], newUVIds);

				C_IChangedUV[side] = true;	//damit man bestimmen kann, ob neue UVs zu erstellen sind oder nicht
			}

			//jetzt das UVIDs Array neu aufbauen, so dass es den Formvorschriften von "face" genueuegt
			//[X] = origID, [X+1] = newID, pro UVSet
			MIntArray UVIDs(creator->numUVSets*2);
			for(i = 0; i < creator->numUVSets; i++)
			{
				UVIDs[i*2] = origUVs[0][i];
				UVIDs[i*2+1] = newUVIds[i];
			}

			//zum Face hinzufueuegen
			creator->helper.addIntArrayToLHS(tmpFace[0]->UVChange, UVIDs);


			if(!orsl->isSelected() && nSelNachbarn[side][dir] != 0)	//extrapfad fuer absolute ausnahmesituation
			{//jetzt  selbs das Face herausfinden, welches die nSelEdge exclusiv besitzt, und dieses veruendern ueber changeSideFace
				
				MIntArray nFaces = nSelNachbarn[side][dir]->getFaces();	//hat auf jeden Fall 2 Faces

				int validFace = (nFaces[0] == faceIDs[dir]) ? nFaces[1] : nFaces[0] ;
				
				//nun das Face veruendern
				edge::creator->changeSideFace(origVtx, newVtx, validFace, newUVIds);

			}

		}
		else
		{//also erstmal herausfinden, ob sich irgendw UVs teilen zwischen den faces. Wenn ja, dann diese UVs extra behandeln

			MIntArray newValidator(creator->numUVSets, -1);	//hueuelt infos ueueber geteilte UVs zwischen den Faces
			MIntArray newSharedUVIDs;

			//nur wenn zwischen this und orsl nur eine unsel edge ist, lohnt sich eine erweiterte Suche 
			//da die Faces ansonsten sowieso keine UVs teilen kueuennen

			if(onlyOneNSelNachbar[side*2+dir] && newVtx != vtxIDs[side] && (hasUVs[0] & hasUVs[1]) )	//nur wenn beide Face UVs haben, hat das hier ueueberhaupt nen sinn
		//	if( (hasUVs[0] & hasUVs[1]) )
			{
				
				UINT r = 0;
				bool found = false;	//nur wenn ueueberhaupt eine derartige UV gefunden wurde, soll der creator bemueueht werden
				
				MIntArray otherUVs[2];	//hueuelt die UVs des anderen Vtx, welcher der nSelGeraden von this gehueuert
				int otherVtx = (origVtx == nSelNachbarn[side][dir]->vtxIDs[0]) ? nSelNachbarn[side][dir]->vtxIDs[1] : nSelNachbarn[side][dir]->vtxIDs[0];

				otherUVs[0] = tmpFace[0]->getEdgeVtxUVs(otherVtx);
				otherUVs[1] = tmpFace[1]->getEdgeVtxUVs(otherVtx);

				for(i = 0; i < creator->numUVSets; i++)
				{
					//nur wenn beide Faces UVs haben, ueueberhaupt genaueren Vergleich machen
					if(  (validator[0][i] == 0 && validator[1][i] == 0) 
						&& 
						( origUVs[0][i] == origUVs[1][i]) //)	//es reicht, wenn dieser Typ Face am origVtx zusammenhueuengt ->doch nicht
						&& 
						( otherUVs[0][i] == otherUVs[1][i]) )
					//	|| !onlyOneNSelNachbar[side*2+dir] )	//wenn man das hier anmacht, dann werden wenigr UVs erstellt, aber diese reichen dann eventuell nicht, um verzerrungsfrei zu sein
					{//AHA, sie teilen sich die 2 UVs von bedeutung. Also sollen sie auch die neue UV teilen
												
						//erstmal Flag setzen in newValidator
						newValidator[r++] = 1;

						//dann die Flags im originalValidator -1 setzen, damit spueueter nicht nochma UVs dafueuer erzeugt werden
						validator[0][i] = validator[1][i] = -1;


						//zuguterletzt noch die Positionen der newUVs mitteln und in newUVPos[0] speichern
						newUVPos[0][i*2] = (newUVPos[0][i*2] + newUVPos[1][i*2]) / 2;
						newUVPos[0][i*2+1] = (newUVPos[0][i*2+1] + newUVPos[1][i*2+1]) / 2;


						found = true;

					}
				//	else
				//	{	// also nix gewesen - dies dokumentieren	->newValidator ist bereits  -1 (schon seit initialisierung)
				//		newValidator[r++] = -1;
				//	}

				}

				//wenn was gefunden wurde, dann jetzt die Arbeit machen (lassen ;) )
				if(found)
				{
					
					newSharedUVIDs = creator->createUVs(newValidator, newUVPos[0]);
					
					//Slide
					creator->addUVSlide(newUVPos[0],origUVPos[0],newSharedUVIDs);

					//Das hinzufueuegen zu den Faces wird dann spueueter gemacht

				}
			}//if(onlyOneNSelNachbar[side*2+dir])


			//vorbereitung der edges, damit man gut auf sie zugreifen kann
			edge* edgeTmp[2] = {this, orsl};
			
			//jetzt die ungeshareten UVs bearbeiten - pro face erstmal bestimmen, ob ueueberhaupt UVs vorhanden sind, und diese dann erstellen
			for(f = 0; f < 2; f++)
			{

				//gut: Bei bedarf jetzt die sideFaces auf Queue tun - wenn sie ein face teilen gibt es keine nSelEdges
				if( edgeTmp[f]->nSelNachbarn[seite[f]][richtung[f]] != 0)
				{//auch wenn die UVs hier noch nicht eingetragen sind, wird dies bei der benutzung des tmpFacePtr bereits geschehen sein
					edge::nSelEdgeDataArray.push_back(new nSelEdgeData(edgeTmp[f]->nSelNachbarn[seite[f]][richtung[f]], origVtx, newVtx, tmpFace[f] ) );
				}

				//erstmal checken, ob jetzt noch (nach der bearbeitung der geshareten UVs, UVs vorhanden sind zum erstellen
				//um overhead im creator zu vermeiden wegen eines unsinnigen aufrufs

				
				MIntArray newUVIDs(creator->numUVSets);	
				if(hasUVs[f])
				{
					
					
					if(!existed)
					{
					//UVs erzeugen - nur noch die ungeteilten werden erzeugt
						newUVIDs =  creator->createUVs(validator[f],newUVPos[f]);
					}
					else
					{//ChamferVariante - die ORigUVs erhalten slide
						newUVIDs = origUVs[f];
					}
		
					
					
					//jetzt die UVSlides hinzufueuegem
					creator->addUVSlide(newUVPos[f], origUVPos[f], newUVIDs);
					
				}
				else
					continue;

				
				//zuletzt noch die sharedUVIDs zusammentun mit den normalen newUVIDs, damit die Faces alle nueuetigen infos erhalten
				if(newSharedUVIDs.length() > 0)
				{
					for(i = 0; i < creator->numUVSets; i++)
					{
						if(newSharedUVIDs[i] > -1)
							newUVIDs[i] = newSharedUVIDs[i];
						
					}
				}
				
				//jetzt das UVIDs Array neu aufbauen entsprechend der Regeln und die UVs zum face hinzufueuegen
				MIntArray UVIDs(creator->numUVSets*2);
				for(i = 0; i < creator->numUVSets; i++)
				{
					UVIDs[i*2] = origUVs[f][i];
					UVIDs[i*2+1] = newUVIDs[i];
				}
				
				
				//und rauf damit auf's array
				creator->helper.addIntArrayToLHS(tmpFace[f]->UVChange, UVIDs);
				

			}//for(f = 0; f < 2; f++)


		}


//	}//if( creator->useUVs && creator->numUVSets > 0)


}



#define DELTA 0.000001	

//---------------------------------------------------------------------------------
void	edge::getUVPos(MPoint newVtxPos, edgeFaceData* thisFace, MFloatArray& newUVPos,bool regatherData)
//---------------------------------------------------------------------------------
{

	//regatherData = true;
	newUVPos.setLength(creator->numUVSets * 2);

	//ich baue mir mein eigenes Face anhand der OriginalDaten im Creator -> Diese sind unverueuendert (haben keine EdgeUVs, die direkt auf einer edge liegen, und mir dann die Werte versauen)
	if(regatherData)
	{//diese funktion ist deaktiviert, wenn diese methode von nSelEdge gerufen wird, da diese bereits gute Daten uebergibt
		edgeFaceData* tmpFace = new edgeFaceData(thisFace->id);
		
		creator->getFaceVtxIDs(tmpFace->id, tmpFace->faceVertices);
		tmpFace->UVRelOffsets = (bool*)malloc(creator->numUVSets);
		creator->getFaceUVs(tmpFace->id, tmpFace->UVIndices, tmpFace->UVRelOffsets);
		
		thisFace = tmpFace;
	}

	MPointArray positions;
	creator->getVtxPositions(thisFace->faceVertices, positions);

	MFloatArray			UVPositions;	//hueuelt die positionen aller UVs aller UVs aller UVSets dieses Faces
	edge::creator->getUVPositions(thisFace->UVIndices, UVPositions);

	UINT l = thisFace->faceVertices.length();


	INVIS(creator->helper.printArray(thisFace->UVIndices, " = UVIndices"));

	MDoubleArray		weights(l*creator->numUVSets);		//die gewichtungen der errechneten Vtx positionen ergeben sich aus der entfernung des neuen Vtx zur edge, relativ zur lueuenge der edge
	MPointArray			tNewUVPos(l*creator->numUVSets);	//hueuelt die errechneten Punkte fueuer UV passend zum newVtx
//	MFloatVectorArray	newVectors(l*creator->numUVSets);	
	MDoubleArray		maxWeight(creator->numUVSets,0.0);		//Summe aller weights
	

	MFloatVector		newVector;
	MFloatVector		UVVector;	//hueuelt die  UVVectors, die this und orsl entsprechen
	MVector				vectors[3];	//this, orsl, new
	MVector				normal;		//die Normale am Dreieck, zwischen vtx[i-1] - vtx[i] - vtx[1+1] aufgespannt ist, sollte immer vom face weg zeigen
	double rl;		//Nueuechstgelegener Punkt vom newVtx aus gesehen, relativ zur edge
	double h;		//die lueuenge des vektors der auf AB steht, und am newVtx endet, relativ zu AB
	bool inside;	//perpDotProdukt des neuen Vektors (AN) mit AB als Referenzwert fuer die Richtung
	
	double tmp, length;	// == newLength, newAngle
	
	
	

	//BEGIN>>>>>
	UINT i, n,x, r = 0;

	for(i = 0; i < l; i++)
	{
		INVIS(cout<<"Bin bei EDGE: "<<thisFace->faceVertices[i]<<" - "<<thisFace->faceVertices[(i+1)%l]<<endl);
		
		//jetzt zuerst die winkelRatios aufbauen
		vectors[0] = positions[(i+1)%l] - positions[i];
		vectors[1] = newVtxPos - positions[i];

		vectors[2] = positions[i] - positions[(i-1+l)%l];	// == die vorhergehende edge, in faceRichtung

		length = vectors[0].length();	//zwischenspichern der lueuenge, weil die noch oft gebraucht wird
		
		rl	= (cos(vectors[0].angle(vectors[1])) * vectors[1].length()) / length;
		h	= sqrt(( pow(vectors[1].length(),2.0) - pow((rl * length),2.0) + DELTA )) / length;	//Satz des pytagoras - Hypotenuse - kathete 1 == kathete 2
	
		//jetzt bestimmen ob neuer Punkt innerhalb von face, welches ich 3dimenional extrudiere
		
		//normale des dreiecks
		normal = vectors[0] ^ (vectors[2]^vectors[0]);

	//	normal.normalize();
		//ein Vector, der nach aussen zeigt, wieder in normal gespeichert, der einfachheit halber
	//	normal = vectors[0]^normal;


		//wenn der Winkel (cosGAMMA) zwischen dem newVektor und der normale, die nach aussen zeigt, negativ ist, dann ist neuer Vtx innen (relativ
		//zu dieser edge)
		if((normal * vectors[1]) / (normal.length() * vectors[1].length() +  DELTA) <= 0.0 )	//== 0 bedeutet auf der kante - eventuell dieses = rausnehmen, da es eh keine rolle spielt eigentlich
			inside = true;
		else
			inside = false;


		//GEO DATEN HOLEN ENDE


		
		
		for(n = 0; n < creator->numUVSets; n++)
		{
			//wenn ueueberhapt UVs vorhanden
			if(!thisFace->UVRelOffsets)
			{
				continue;
			}
			
			MPoint origUV(UVPositions[n*l*2 + i*2], UVPositions[n*l*2 + i*2+1]);

			//jetzt die UVDaten erstellen

			UVVector = MPoint(UVPositions[n*l*2 + ((i+1)%l)*2], UVPositions[n*l*2 + ((i+1)%l)*2+1]) - origUV;
				
			
			
			newVector = UVVector * rl;
			
			//lueuenge des vektors zischenspeichern
			length = UVVector.length();
			
			//UVVektor umstellen, so dass ein neuer senkrechter winkel entsteht. Die richtung ist abhueuengig von "inside"
			
			//erst CCW um 90ueue drehen
			tmp = UVVector.x;
			UVVector.x = -UVVector.y;
			UVVector.y = tmp;
			

			//wenn punkt NICHT inside, dann andere Richtung nehmen
			if(!inside)
			{//einfach die vorzeichen umkehren
				UVVector.x = -UVVector.x;
				UVVector.y = -UVVector.y;
			}

			
			//jetzt den vektor normalisieren, und ihn entsprechend verlueuengern anhand der lengthratio der hueuehe
			UVVector.normalize();
			UVVector = UVVector  * (length  * h);

			newVector = UVVector + newVector;
			//jetzt die gewichtung errechnen anhand der entfernung des newVtx zur currentEdge
			weights[n*l+i] = newVector.length() + DELTA;	//weight darf niemals 0 sein	->besser als die untere Variante
			

			
			maxWeight[n] += weights[n*l+i];
			
			
			//neuer Punkt ist der errchnete Parallele vector (newVector) + der orthogonale vektor
			tNewUVPos[n*l+i] = origUV + newVector;
			

			


		}//for(n = 0; n < creator->numUVSets; n++)ENDE


	}//for(i = 0; i < l; i+=2) ENDE

	

	//jetzt mueuessen die errechneten UVPositionen nur noch gewichtet werden
	MPoint tmpPos;
	
	double fWeight;	//hueuelt den entgueueltigen betrag des weights
	double nmw = 0.0;//hueuelt die Summe aller neuen Weights - wird zum schluss als divisor verwendet
	double inverse;
	for(i = 0; i < creator->numUVSets;i++)
	{
		r = i * l;	//schnellerer Zugriff

		tmpPos.x = 0.0;
		tmpPos.y = 0.0;
		
		nmw = 0.0;

		if(!thisFace->UVRelOffsets[i])
			continue;

		for(x = 0; x < l; x++)
		{	
			//ich will das inverse dieser weights - je nueueher das weight bei 0, desto estueuerker ist es. 
			//Diese klasulierung da untern sorgt dafueuer, dass alle invertierten weights nach der normalisierung ach wieder 1 ergeben

			weights[r+x] /= maxWeight[i];	//normalisieren auf 1

			inverse = (1 - weights[r+x]);
			fWeight = 1 / (weights[r+x] / inverse) * inverse ;	
			//fWeight = 1.0;
			tmpPos += tNewUVPos[r+x] *  fWeight;
			nmw += fWeight;
	
		}
		
		newUVPos[i*2] = tmpPos.x / nmw;	//zum schluss muss die ganze sache noch normalisiert werden
		newUVPos[i*2+1] = tmpPos.y / nmw;
	//	newUVPos[i*2] = tmpPos.x / (float)l;
	//	newUVPos[i*2+1] = tmpPos.y / (float)l;

	//	newUVPos[i*2] = tNewUVPos[3].x ;	//zum schluss muss die ganze sache noch normalisiert werden
	//	newUVPos[i*2+1] = tNewUVPos[3].y ;


	}

	if(regatherData)	//lueueschen ist nur erlaubt, wenn das Face auch wirklich lokal erstellt wurde
		delete thisFace;

}

//--------------------------------------------------------------
bool	edge::doUVsDiffer(int checkVtx,
							const edgeFaceData* faceData1,
							const edgeFaceData* faceData2, 
							MIntArray* outOne, 
							MIntArray* outTwo)
//--------------------------------------------------------------
{
	//vergleicht die UVs der Faces, und gibt true zurueck, wenn sie sich in irgendeinem Set unterscheiden
	
	MIntArray*  vtxUVs[2] = {(outOne == 0)? new MIntArray():outOne,(outTwo == 0)? new MIntArray():outTwo};

	//vtxUVs holen
	*vtxUVs[0] = faceData1->getEdgeVtxUVs(checkVtx);
	*vtxUVs[1] = faceData2->getEdgeVtxUVs(checkVtx);

	//jetzt vergleichen
	for(UINT i = 0; i < creator->numUVSets; i++)
	{
		if((*vtxUVs[0])[i] != (*vtxUVs[1])[i])
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------------------------
int			edge::C_checkForEdgeVtx( const edge* requester,		//ptr zum 
									 int UID,					//uniqueID - wird immer benueuetigt, da diese methode nur gerufen wird wenn nurEinNAchbar = true
									 bool	isSeparated,		/*Flag: ist true, wenn die methode von edge gerufen wird, derem UVs nicht zusammen hueuengen*/
									 const MPoint& startPos,			/*zwischen startPosition und endPosition wird der edgeVtx erstellt*/
									 const MPoint& endPos	)
//------------------------------------------------------------------------------------------------------------------------
{
	
	//zuerst mal die Richtige Seite finden
	int s, r;
	getValidRichtung(requester, s, r, UID);

	//wenn infaceData 0 ist, dann einfach die gegenwueuertige edgeIDs zurueueckgeben. Der Requester macht dann alles weitere
	//dies bedeutet auch, dass diese edge bereits evaluiert wurde
	if(isSeparated)
	{
		return edgeVtx[s];
	}
	else
	{//dieser Zweig ist aufwendiger: Eine edge, welche nicht ueueber getrennte UVs verfueuegt will wissen, ob diese edge eventuell edgeIDs erstellt hat/erstellen muss
		if(edgeVtx[s] == -1 && faceData[1] != 0)
		{//es sind keine edgeVtx vorhanden, also checken, ob man welche erstellen muss. Das aber auch nur, wenn diese edge noch nicht evaluiert hat
			if(!clean)
			{
				//checken, ob die UVs getrennt sind
				MIntArray vtxUVs[2][2];
				if( faceData[0]->hasUVs() || faceData[1]->hasUVs() )
				{
					bool notConnected =			doUVsDiffer(vtxIDs[0], faceData[0], faceData[1], &vtxUVs[0][0], &vtxUVs[0][1])
										|
												doUVsDiffer(vtxIDs[1], faceData[0], faceData[1], &vtxUVs[1][0], &vtxUVs[1][1]) ;
					

					if( notConnected )
					{
						//sie sind an einem UVSet an mindestens einem Vtx an den UVs getrennt. Flag setzen und abbruch ->
						MFloatArray newUVPos;
						MIntArray newUVs;
						
						//okay - jetzt einen neuen edgeVtx erzeugen mithilge der ueuebergebenen Positionen
						edgeVtx[s] = creator->newVtx( startPos + 0.5*(endPos - startPos) );
						
						//nun die UVs erzeugen fueuer`s eigene Face
						faceData[0]->insertEdgeVtx(vtxIDs[s], edgeVtx[s], (isFlipped[0]) ? ( (s == 0) ? false : true) : ((s == 0) ? true : false) ) ;
						faceData[1]->insertEdgeVtx(vtxIDs[s], edgeVtx[s], (isFlipped[1]) ? ( (s == 0) ? false : true) : ((s == 0) ? true : false));
						
						//neue UVs erzeugen
						
						creator->getUVPositions(vtxUVs[s][0], newUVPos);
						newUVs = creator->createUVs(vtxUVs[s][0], newUVPos);	//MAN MUSS HIER EIGENTLICH AUCH NOCH NACH SHARED UVS UNTERSCHEIDEN 
						
						
						//newUVs setzen: 
						faceData[0]->assignUV(edgeVtx[s], newUVs);
						
						
						
						creator->getUVPositions(vtxUVs[s][1], newUVPos);
						newUVs = creator->createUVs(vtxUVs[s][1], newUVPos);	//MAN MUSS HIER EIGENTLICH AUCH NOCH NACH SHARED UVS UNTERSCHEIDEN 
						
						
						//newUVs setzen: 
						faceData[1]->assignUV(edgeVtx[s], newUVs);
						
						
						//jetzt noch slide hinzufueuegen, damit die Position des edgeVtx wirklich immer in der Mitte 
						edge::creator->addSlideAndNormal(vtxIDs[s], edgeVtx[s], faceIDs);
						
						//FERTIG //MERKE: Um normalen kueuemmert sich das Face automatisch
						
						
					}
					
				}
			}
		}


		return edgeVtx[s];	//kann auch -1 sein ... wird vom requester gecheckt

	}

	//forbidden path
	return -1;

}
									

//-----------------------------------------------------------------------------------------------------------------
void		edge::insertNewEdgeVtx( int newVtx, bool edgeVtxExisted, edge* orsl)
//-----------------------------------------------------------------------------------------------------------------
{
	//UVs werden ebenfalls von dieser prozedur behandelt -  das ist effizienter
	//allerdings nur, wenn es explizit gesagt wird, was in den meisten Fueuellen so ist. Nur fueuer Normals werden ne Ausnahme gemacht
	
	

	UINT i,a;

	

	bool orslSelected = orsl->isSelected();	//gibt an, ob die andere edge selected ist

	//nun dafueuer sorgen, dass orsl auch wirklich jedes Face defniert - egal ob selected oder nicht
	//es kann nueuemlich auch sein, dass die faceDataPropagation nicht weit genug war
	
	for(i = 0; i < orsl->faceIDs.length(); i++)
	{
		if(orsl->faceData[i] == 0)
			orsl->initFaceData(i);
	}
	
	

	
	bool		isNGVtx = false;		//flag: isNachbargeradenVtx - wird ein NG Vtx auf der nachbargeraden erstellt?	
	

	unsigned int	selSharedFaceLocID = 0;
	
	//jetzt die locID herausfinden, die das face anzeigt, welches mit der selEdge geteilt wird (und zwar relativ zum array der unSelEdge)
	if(!orslSelected)
	{
		// in diesem Fall ist match auf jeden Fall gefueuellt, da es sich bei orsl dann um eine endEdge handeln muss
		for(i = 0; i < orsl->faceIDs.length(); i++)
		{
			for( a = 0; a < faceIDs.length(); a++)
			{
				if(orsl->faceIDs[i] == faceIDs[a])
				{
					selSharedFaceLocID = i;
					break;
				}
			}
		}
	}
	
	
	//Da die ueuebergebene edge nicht zwangsweise jene sein muss, zu der der statische "side" wert passt, muss erstmal die richtige seite des origVertizes
	//herausgefunden werden;

	
	//hier nur edgeVtx einfueuegen, wenn es sich wirkich um einen neuen Vtx handelt (kueuennte bei Chamfer der Fall sein).
	//Achtet man nicht darauf, wird in Vtx doppelt vorhanden sein, und das Face wueuere defekt
	bool notExists = (vtxIDs[side] == newVtx) ? false : true;
	

	
	//VERTIZEN BEARBETIEN
	int validStart;					//validStart enthueuelt die ID des ORIGVtx , welcher am nueuechsten an dem edgeVtx dran ist (laut MeshTopologie)
	int validEnd;
	int realValidEnd(0);				// ==  ValidEnd - Speicher fueuer den Originalwert - er wird fueuer UVs benueuetigt, da diese immer die eigentliche Edg als referenz nehmen mueuessen
	
	//jetzt mueuessen die Vertizen gefunden werden, die die edge bilden fueuer den neuen EdgeVtx, genannt frameVtx
	if(this == orsl)	//hier muss nicht auf nen bereits existenten Vtx gecheckt werden,da dieser Zweig NIE von Chamfer durchlaufen wird
	{//aha, der neue Vtx liegt auf der gegenwueuertigen edge, also ist side ausschlaggebend fueuer den origVtx
		//dieser neue Vtx muss dann in beide Faces eingefueuegt werden
		//MERKE: Wenn auf der anderen Seite bereits ein edgeVtx erstellt wurde, muss dieser als endVtx verwendet werden!


		validStart = vtxIDs[side];
		validEnd = vtxIDs[1-side];	// er sucht sich den richtigen endVtx dann automatisch
		
		realValidEnd = validEnd;

		if(orsl->faceData[0]->verifyStartEndVtx(validStart, validEnd))
		{//wenn diese Prozedur die Vtx geflipped hat, dises umkehren. Ihr eigentlicher Sinn besteht darin, validEnd zu aktualisieren, wenn nueuetig
			int tmp = validEnd;
			validEnd = validStart;
			validStart = tmp;
		}

		if(!edgeVtxExisted)	
		{ 
			//hier muss man nict aufs zurueueclgegebene IntArray achten, (denke ich)
		
			faceData[0]->insertEdgeVtx(validStart, validEnd ,newVtx);
			
			
			//borderEdges checken
			if(faceData[1] != 0)
				faceData[1]->insertEdgeVtx(validStart, validEnd, newVtx);
		}

			

	}
	else
	{//also ist edge ein nachbar, so dass man die gueueltige Seite (und damit den OrigVtx) ertmal finden muss
		//MERKE: Man muss hier nicht checken, ob "nurEinNachbar" zutrifft, da diese eh niemals edgeVtx bekommen
		
		isNGVtx = true;
		
		//jetzt validStart und end herausfinden

		for(i = 0; i < 2; i++)
		{
			for(UINT x = 0; x < 2; x++)
			{
				if(vtxIDs[i] == orsl->vtxIDs[x])
				{
					
					validStart = orsl->vtxIDs[x];
					
					validEnd =  orsl->vtxIDs[1-x] ;
					
					realValidEnd = validEnd;

					goto outOfLoops;
					
				}
			}
		}
		
outOfLoops:
		
		if(orsl->faceData[0]->verifyStartEndVtx(validStart, validEnd))
		{//wenn diese Prozedur die Vtx geflipped hat, dises umkehren. Ihr eigentlicher Sinn besteht darin, validEnd zu aktualisieren, wenn nueuetig
			int tmp = validEnd;
			validEnd = validStart;
			validStart = tmp;
		}


		if(!edgeVtxExisted )	//notExisted bezieht sich hier auf die ID es neuen Vtx
		{//MERKE: Wenn der Vtx bereits existierte, muss er nicht mehr hinzugefueuegt werden. Das einzig wichtige ist, dass die UVs einen Slide erhalten
			//->er muss sehr wohl hinzugefueuegt werden, damit bei der Erstellung der Faces (wo ja ein passendes FaceData objekt neu aufgebaut wird)
			//die UVs gefunden werden kueuennn mit getNewVtxUV
			
			
			if(orslSelected)
			{//wenn die andere Edge selected ist, dann kann der edgeVtx ganz normal hinzugefueuegt werden
				//der origVtx ist jetzt also  VtxIDs[i] - jetzt einfach fueuer alle faces den Vtx einfueuegen

				
				orsl->faceData[0]->insertEdgeVtx(validStart, validEnd, newVtx);
				
				
				//borders beachten
				if( orsl->faceData[1] != 0 )
					orsl->faceData[1]->insertEdgeVtx(validStart, validEnd, newVtx);
			}
			else
			{//in diesem Fall muss zwischen selSharedFace und dem anderen (falls voranden, unterschieden werden
				
				//in das Face, was der unselEdge gehueuert, muss der newVtx als edgeVtx eingefueuegt werden

				if( orsl->faceData[1 - selSharedFaceLocID] != 0 && notExists)
				{
					orsl->faceData[1 - selSharedFaceLocID]->insertEdgeVtx(validStart, validEnd, newVtx);
					
				}
				
				//Das Face welches mit selEdge geteilt wird, muss den "edgeVtx" als newVtx erhaltem - ebenso dann spueueter mit den UVs
				//hier mueuessen auf jeden Fall edgeRelative werte zugewiesen werden - deshalb wird immer ERValidStart verwendet

				
				orsl->faceData[selSharedFaceLocID]->vtxChange.append(validStart);
				orsl->faceData[selSharedFaceLocID]->vtxChange.append(newVtx);

				
			}
		}				

		
	}
	
	
	
	//UVs BEARBEITEN
	
	if(orsl->faceData[0]->hasUVs() || (orsl->faceData[1] != 0 && orsl->faceData[1]->hasUVs()) )
	{
		//jetzt die UVsPositionen fueuer jede Richtung errechnen und fueuer jedes UVSet
		//wenn der edgeVtxExisted Flag gesetzt wurde, dann mueuessen die Positionen mit den bereits vorhandenen gemittelt werden
		//MERKE: Ich greife einfach immer ueueber den "orsl" ptr auf die Daten zu, welcher sich ja entweder auf "this" bezieht oder auf die nachbar edge
		
		//vorher noch zusehen, dass die validStartIDs stimmen, fueuer den Fall, das !edgeVtxExisted und die insertEdgeVtx methode nicht gerufen wurde, die das ja eigentlich macht
	//	if(edgeVtxExisted)
	//		faceData[0]->verifyStartEndVtx(validStart, validEnd);
		//->muss nicht sein, da dis vor der eigentlich wichtigen insertVx prozedur sowieso nochmal gecheckt wird

		//1) Errechnen, wo (in procent) sich der neue Vtx befindet zwischen validStart und valid End
		MPoint		newVtxPos;
		newVtxPos = (newVtx <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx) : creator->getVtxPoint(newVtx);

		MPoint		startPoint = creator->getVtxPoint(validStart);
		MVector		direction = creator->getVtxPoint(realValidEnd) - startPoint;
		

		double r = (newVtxPos - startPoint).length() / direction.length();	//hier mueuessen keine weiteren Checks vorgenommen werden, da dieser punkt garantiert auf der edge liegt
		
		
		
		//UVDaten holen (werden immer fueuer alle UVSets zurueueckgegeben, inkusive UVPositionen
		MIntArray startUVs[2];
		MFloatArray startUVPos[2];
		
		
		startUVs[0] = orsl->faceData[0]->getEdgeVtxUVs(validStart);
		creator->getUVPositions(startUVs[0], startUVPos[0]);
		
		
		MIntArray endUVs[2];
		MFloatArray endUVPos[2];
		
		endUVs[0] = orsl->faceData[0]->getEdgeVtxUVs(realValidEnd);
		creator->getUVPositions(endUVs[0], endUVPos[0]);
		
		
		if(orsl->faceData[1] != 0)
		{//die UVDaten fueuers 2. face holen
			startUVs[1] = orsl->faceData[1]->getEdgeVtxUVs(validStart);
			creator->getUVPositions(startUVs[1], startUVPos[1]);
			
			endUVs[1] = orsl->faceData[1]->getEdgeVtxUVs(realValidEnd);
			creator->getUVPositions(endUVs[1], endUVPos[1]);
		}
		else
		{//arrays mueuessen immer gefueuellt sein (wenn auch mit - 1), also die jetzt nachholen
			
			startUVs[1] = endUVs[1] = MIntArray(creator->numUVSets, -1);
		}
		
		
		
		
		MIntArray edgeUVs[2];	//dieses Array hueuelt alle UVs des edgeVtx (falls er existierte) - ansonsten wird es nicht benueuetigt, sondern nur das newUVIds Array
		MFloatArray edgeUVPos[2];	//dieses Array hueuelt die Positionen der edgeUVs, fueuer den Fall, dass sie schon vorhanden sind
		
		if(edgeVtxExisted)
		{
			edgeUVs[0] = orsl->faceData[0]->getEdgeVtxUVs(newVtx);

			creator->getUVPositions(edgeUVs[0], edgeUVPos[0]);
			
			if(orsl->faceData[1] != 0)
			{
				edgeUVs[1] = orsl->faceData[1]->getEdgeVtxUVs(newVtx);

				creator->getUVPositions(edgeUVs[1], edgeUVPos[1]);
			}
		}
		else if(!notExists)
		{//also mueuessen die origUVs nen slide erhalten. Dies bedeutet, dass die edgeUVs beschrieben werden mueuessen mit den IDs der OrigUVs
			edgeUVs[0] = startUVs[0];

			if(orsl->faceData[1] != 0)
				edgeUVs[1] = startUVs[1];

		}
		
		
		
		
		
		UINT l = orsl->faceIDs.length();
		MFloatArray newUVPos[2];	//hueuelt die positionen der neu zu erstellenden UVSets pro face - es wird allerdings fueuer beide faces zwischengespeichert
									//damit in besonderen Fueuellen weniger gerechnet weren muss
		UINT z, x;					//fueuer iteration

			//2)jetzt herausfinden, ob die beiden angrenzenden Faces UVs die UVs fueuer start und endVtx teilen
		for(i = 0; i < l; i++)
		{
			for(UINT n = 0; n < creator->numUVSets; n++)
			{
				
				
				//wenn keins der Faces UVs hat, dann kann man diese Iteration sein lassen
				if( startUVs[i][n] == -1 )	//dies bedeutet implizit, das auch die endUVs nicht gesetzt sind, da ein Face entweder UVs hat oder nicht
				{
					
					//auch das newUVPos Array muss angepasst werden, damit die elemente immer synchron zu numUVSets sind
					newUVPos[i].append(0.0);
					newUVPos[i].append(0.0);
					
					continue;
				}
				
				
				//speichert die erstellten positionen, Kodiert: N = l; l*2+0 = UPos fueuer UV des l'ten Faces
				MFloatArray resultPos(2);	
				z = 0;
				
				
				
				//jetzt die UVPositionen fueuer dieses UVset fueuer das entsprechende Face erstellen	
				
				
				resultPos[z++] = ( startUVPos[i][n*2] + r*(endUVPos[i][n*2] - startUVPos[i][n*2]) );	// == UPosition
				resultPos[z++] = ( startUVPos[i][n*2+1] + r*(endUVPos[i][n*2+1] - startUVPos[i][n*2+1]) );	// == VPosition
				
				
				
				
				//jetzt die Resultate richtig zuweisen: entweder mitteln, oder neuen UV erstellen
				
				//wenn der edgeVtx bereits exisitiert hat, muss auch die UV bereits existiert haben
				//ich berechne das einfach fueuer jedes Face entsprechend, ungeachtet der Tatsache, dass sie eventuell bereits UVs teilen - ist einfacher und spart reichlich "ifs"
				
				
				if(edgeVtxExisted)
				{//UVPositionen mitteln mit den vohandenen
					newUVPos[i].append( (edgeUVPos[i][n*2] + resultPos[0]) / 2 );	//UPos
					newUVPos[i].append( (edgeUVPos[i][n*2+1] + resultPos[1]) / 2 ); //VPos
					
				}
				else
				{//neue UVPositionen in newUVArray ueuebertragen
					newUVPos[i].append(resultPos[0]);
					newUVPos[i].append(resultPos[1]);
				}
				
					
				

				
				
			}//for(UINT n = 0; n < creator->numUVSets; n++)
			
			
	
			
		}//for(UINT i = 0; i < l; i++)
		
		
		
		//Jetzt mueuessen die neuen UVPositionen nur noch verarbeitet werden: entweder werden daraus neue UVs erstellt, oder es werden die Positionen der existierenden UVs verueuendert
		//dies hier geschieht pro face
		
		
		if(edgeVtxExisted | !notExists)	//hier her, wenn die ID breits existierte, um damit korrekt umgehen zu kueuennen
		{//also den Creator anweisen, die edgeUVPos mit den neuen Positionen auszustatten
			//dies ist NICHT redundant, da die neue EdgeVtx position nicht zwangsweise am selben Platz liegen muss!
			

			//UVSlide wurde in diesem Fall auch schon hinzugefueuegt, allerdings mit den alten Werten - jetzt muss man die newUVPos noch aktualisieren
			//oder man packt den Slide einfach nochmal raf, was bedeutet, dass der Punkt mehrfaach transformiert wird, aber immerhin mit den richtigen Werten
			//so wirds beim Slide auch gemacht momentan
			creator->addUVSlide(newUVPos[0],startUVPos[0], edgeUVs[0]);
			
			if(orsl->faceData[1] != 0)
				creator->addUVSlide(newUVPos[1],startUVPos[1], edgeUVs[1]);

			
			//Man muss die Positionen setzen, damit addVtxChange auf aktuelle UV positionen fueuer die edgeVTx zugreifen kann
			//MERKE: Diese Operation darf nur erfolgen, wenn der newVtx wirklich neu ist
			if(notExists)
			{
				creator->setUVPositions(edgeUVs[0], newUVPos[0]);
				
				//dieser Check muss sein, weil im Falle vom Chamfer nicht zwangsweise ein 2. face vorhanden ist
				if(orsl->faceData[1] != 0)
					creator->setUVPositions(edgeUVs[1], newUVPos[1]);
			}
			else
			//SpezialFall: Wenn man sich im chamfermode befindet kommt man nur in diesen Zweig, wenn ein vohandener Vtx veschoben wird und nun als newVtx fungiert
			//in diesem Fall mueuessen die UVs des origVtx als UVChange eingetragen werden
			{
				l = startUVs[selSharedFaceLocID].length();
				MIntArray UVIDs(l*2);
				
				for(i = 0; i < l; i++)
				{
					UVIDs[i*2] = startUVs[selSharedFaceLocID][i];
					UVIDs[i*2+1] = startUVs[selSharedFaceLocID][i];
				}
				
				creator->helper.addIntArrayToLHS(orsl->faceData[selSharedFaceLocID]->UVChange, UVIDs);

			}
			
		}
		else
		{
			//zuert aber herausfinden, wo (in welchen UVSets) die Faces die UVs Teilen.-> geht nur, wenn orsl 2 faces hat

			MIntArray shareUVs(creator->numUVSets, -1);	//1, wenn das UVSet zwei verbundene Faces hat, -1 andernfalls - hueuelt spueueter auch die neuen geshareten UVs
			

			if( (l == 2)  )	
			{
				bool found = false;	//flag, um overhead im creator zu vermeiden	
				for(x = 0; x < creator->numUVSets; x++)
				{
					if(		(orsl->faceData[0]->UVRelOffsets[x] & orsl->faceData[1]->UVRelOffsets[x] )
						&&
							(startUVs[0][x] == startUVs[1][x])
						&&
							(endUVs[0][x] == endUVs[1][x])
															)
					{//Sie teilen die UVs, also Arrays setzen

						shareUVs[x] = 1;
						
						//startUVs nun auf -1 setzen, damit dort spueueter nicht nochmal UVs erzeugt werden, wenn der Pass kommt, welcher nur die unShared UVs erzeugt
						startUVs[0][x] = startUVs[1][x] = -1;

						found = true;
					}

				}

				//jetzt noch die UVs erstellen
				if(found)
				{
					MIntArray tmpArry;

					
					tmpArry= creator->createUVs(shareUVs, newUVPos[0]);
					
					shareUVs = tmpArry;
				}
				
			}
			
			//nun die restlichen UVs erzeugen
			for(i = 0; i < l; i++)
			{
				//nun soll der creator erstmal neue UVs erzeugen (ungeteilt) und diese dann zurueueckgeben
				
				MIntArray newUVs = creator->createUVs(startUVs[i], newUVPos[i]);

				//in dieses Array aus ungeteilten UVs jetzt die geteilten UVs eintragen
				for(x = 0; x < creator->numUVSets; x++)
				{
					if(shareUVs[x] > -1)
						newUVs[x] = shareUVs[x];		//->newUVs war an dieser Stelle sowieso -1

				}

				
				
				//jetzt auch gleich slide hinzufueuegen

				

				//	creator->addUVSlide(endUVPos[i],startUVPos[i], newUVs);
				creator->addUVSlide(newUVPos[i],startUVPos[i], newUVs);



				if(orslSelected)
				{
					//diese neuen UVPositionen mueuessen jetzt noch in faceData gepackt werden
					orsl->faceData[i]->assignUV(newVtx, newUVs);
				}
				else
				{//wenn es sich bei dem gegenwueuertigen Face um das mit der selEdge geteilte handelt, dann mueuessen die UVs als UVChange eingetragen werden , ansonsten
					//als edgeVtxUVChange
					
					if(i == selSharedFaceLocID)
					{//es ist das geteilte Face
						//die UVs werden samt originalID ins UVChangeArray eingetragen: OrigUVID gefolgt von der neuen
						
						//ertsmal die Position von validStart im verticesArray finden, um die Lokale ID zur referenzierung der UVs zu erhalten
						int locIDnewUV = -1;
						UINT vl = orsl->faceData[i]->faceVertices.length();
						for(x = 0; x < vl; x++)
						{
							if(orsl->faceData[i]->faceVertices[x] == validStart)
							{
								locIDnewUV = x;
								break;
							}
						}
						
						INVIS(if(locIDnewUV == -1) cout<<"WARNUNG IN  INSERTEDGEVTX: locID war nicht definiert!!"<<endl;);
						
						
						z = 0;
						MIntArray newVtxUVs(creator->numUVSets*2, -1);			//hueuelt die UVs fueuer den newVtx: Xn*2 = alte UV (-1 falls nicht existent), Xn*2+1 = neue UV
						for(x = 0; x < creator->numUVSets; x++)
						{
							if(newUVs[x] == -1)
							{//fueuer dieses UVSet hat der newVtx keine UV, also 2 mal -1 einfueuegen bzw r um 2 erueuehen
								z += 2;
								continue;
							}
							
							//ansonsten die entsprechnende origUV aufsuchen und hinzufueuegen
							newVtxUVs[z++] = orsl->faceData[i]->UVIndices[orsl->faceData[i]->UVAbsOffsets[x] + locIDnewUV];
							newVtxUVs[z++] = newUVs[x];
							
						}
						
						//jetzt einfach die UVs an UVChange anhueuengen
						creator->helper.addIntArrayToLHS(orsl->faceData[i]->UVChange, newVtxUVs);
						
					}
					else
					{//es ist das Face, welches aussschlieueuelich der unSelEdge gehueuert
						orsl->faceData[i]->assignUV(newVtx, newUVs);
					}
					
					
				}
			}
	
		}
		

	}



}



