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

// vertexFty.cpp: implementation of the vertexFty class.
//
//////////////////////////////////////////////////////////////////////

#include "vertexFty.h"


#include <math.h>



//statische Datenmitglider

	const UINT vertexFty::null =	(UINT)0; 
	const UCHAR vertexFty::is   =	(UCHAR)1;
	const UCHAR vertexFty::iv	=	(UCHAR)2;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------
vertexFty::vertexFty(MIntArray* inSelVtxIDs,
					 meshCreator* inCreator,
					 int inSpin)
									:				selVtxIDs(inSelVtxIDs),
													ftyCreator(inCreator),
													spin(inSpin)
//-------------------------------------------------------------------------------------------
{
	
}

//-------------------------------------------------------------------------------------------
vertexFty::~vertexFty()
//-------------------------------------------------------------------------------------------
{

}


//-------------------------------------------------------------------------------------------
void	vertexFty::doIt()
//-------------------------------------------------------------------------------------------
{
	meshCreator& creator = *ftyCreator;
	MIntArray& vtxList = *selVtxIDs;
	BPT_Helpers helper;

	//MERKE:Diese prozedur arbeitet nur mit dem MeshCreator, benutzt also kein MFnMesh etc.

	//zuersteinmal ein Ba aufbauen mit den gewählten vertizen
	BPT_BA selCheckList(vtxList,true,false,creator.getLastVtxID());	//ArbeitsArray für die folgende Prozedur
//	BPT_BA selList = selCheckList;									//wird als lookup verwendet, der nicht verändert wird

	//Die einfachste Loesung: Ba aufbauen mit allen Faces, die schon bearbeitet wurden - bedeutet aber auch, dass für jeden SelVtx die 
	//verbundenen Faces geholt werden müssen - momentan ist die Sache ziemlich billig (CPU ZEIT)
	//man müsste zuerst die verbundenen Faces zu allen Vertizen holen, und dese Dann abarbeiten

	MIntArray allConnectedFaces;
	UINT r = null;
	

	UINT l,i,x;				//für iterationen
//	UINT cv;				// == currentVtx

	MIntArray	faceVtx;	//vertizen des jeweiligen faces
	MIntArray	connectedFaces;	//hält die mit dem Vtx verbundenen Faces
	MIntArray	match;			//Array fr übereinstimmende Vtx
	//jetzt durch die SelVtx parsen und eine Facezurodnung herstellen
	l = vtxList.length();

	//alle verbundenen Faces holen
	for(i = null; i < l; i++)
	{

		creator.getConnectedFaces(vtxList[i],connectedFaces);
		
		allConnectedFaces.setLength(allConnectedFaces.length() + connectedFaces.length());


		for(x = null; x < connectedFaces.length(); x++)
		{
			allConnectedFaces[r++] = connectedFaces[x];
		}

	}

	helper.memoryPrune(allConnectedFaces);

	l = allConnectedFaces.length();
	for(i = null; i < l; i++)
	{

		creator.getFaceVtxIDs(allConnectedFaces[i],faceVtx);

		selCheckList.findMatching(faceVtx, match);

		if(match.length() > 1)
			splitFace(faceVtx,match,allConnectedFaces[i]);


	}
	
	/*
	for(i = 0; i < l; i++)
	{
		cv = vtxList[i];
		//wenn der gegenwärtige Vtx noch nicht bearbeitet wurde
		if( ! selCheckList[cv])	//hier lieber operator [] nehmen, da isFlagSet noch nen unnoetigen rangecheck macht
		{
			continue;
		}

		//ansonsten weitermachen und:
		//die verbundenen Faces holen
		creator.getConnectedFaces(cv,connectedFaces);

		//jetzt in jedem verbundenen Face checken, ob von den entsprechenden faceVertices
		//gewählte vertices vorhanden sind
		//zuvor aber den gegenwärtigen cv entfernen
		selCheckList.setBitFalse(cv);

		for(x = 0; x < connectedFaces.length(); x++)
		{
			creator.getFaceVtxIDs(connectedFaces[x],faceVtx);

			//wenn mehr als nur der cv auf dem face gewählt sind, dann connectProzedur starten
			selList.findMatching(faceVtx, match);

			//if(match.length() > 1)
			if(match.length() > 1)	//nur testweise
			{//es handelt sich um ein gültiges face, da mindestens 2 vertizen im Face gewählt waren ->splitten
				INVIS(helper.printArray(faceVtx," = zu bearbeitendes Face"););

				splitFace(faceVtx, match, connectedFaces[x]);
				
				//zuguterletzt noch die matches aus selCheckList entfernen, damit diese nicht nochmal bearbeitet werden
		
				selCheckList.remove(match);	
			}
			
		}

	}
*/
}

//-----------------------------------------------------------------------------------
inline void vertexFty::splitFace( MIntArray& faceVtx, MIntArray& matchVtx, int faceID)
//-----------------------------------------------------------------------------------
{//matchVtx ist immer gepruned, wenn er hier ankommt
	//faceID wird nur zum erstellen der neuen Faces benoetigt

	//jetzt erstmal lookup arrays aufbauen, mit denen gearbeitet wird
	UINT i;
	UINT l = faceVtx.length();
	
	MPointArray points;

	ftyCreator->getVtxPositions(faceVtx,points);
	refValue = ftyCreator->getFaceSize(points);
	


	UCHAR* f = (UCHAR*)malloc(l);	//'f'lagLUT - kann mit bitOperationen getestet werden
	

	//jetzt die LUT Arrays beschreiben
	for(i = null; i < l; i++)
	{
		//flags setzen - isValid ist true, alles andere ist eingangs false
		f[i] = iv;

	}

	
	//selectedVtxArray schreiben - lokale indices finden
	for(i = null; i < matchVtx.length(); i++)
	{
		for(UINT x = null; x < l; x++)
		{
			if(matchVtx[i] == faceVtx[x])
			{
				f[x] += is;	//an dieser Stelle ist also ein Vtx gewählt
				break;
			}

		}
	}



	MIntArray	nf(l);							//speicher für neue Faces ('N'ew'F'ace) (temp) - gross genug, um das ganze Face nochmal zu halten (um reallocs zu vermeiden)
												//newFaces sind so konfiguriert, dass man mit createPoly und changePoly problemlos neue faces erstellen kann
	
	int			x,z;							//verwendet für iterationen,  dis  = distance ->wie weit entfernt von gegenwärtiger position soll gesucht werden?
	UINT		tsb, teb;						//tmpStartBound, tmpEndBound: Dient der festsetzung von arraybereichen, die da neue Face definieren
												//erst wenn das neue Face verifiziert wurde dürfen auch die memoryArrays entsprechend angepasst werden

	UINT tsb2, teb2;							//die erweiterte start/EndBoundary für die Kommenden Faces - sie werden definiert durch bis zu 4 corners
												//und beliebig vielen vertizen

	bool success = false;						//flag, ob die whileSchleife mindestens 1 face erstellt hat
	bool go = true;								//Für EndlosSchleife
					

	UCHAR		flagValue;						//tmpWert für Flags, damit man nicht immer auf array zugreifen muss

	list<MIntArray> newFaces;					//verkettete Liste von IntArrays, die alle neuen Faces aus "nf" speichert, um dann zum Schluss der prozedur alle Polys
												//zu erstellen (wobei der 1 Eintrag dafür sorgt, dass das originalface entsprechend geändert wird

	bool changeFace = false;					//flag, damit das Face nicht verändert wird, wenn kein gültiges Face gefunden wurde

	bool (vertexFty::*condition)(UCHAR) ;			//ptr zu bestimmtem Conditiontest
	condition = &vertexFty::isSelected;				//defaultCondition ist isSelected;
	
	
	
	//----------------------------------------------
	//jetzt die splits ermitteln
	//----------------------------------------------
	
	//es soll versucht werden, unabhängig vom startPunkt konsistente Ergebnisse zu erzielen.
	//dies bedeutet, dass man im forraus nicht genau weiss, wann die Sache beendet sein wird, eventuell
	//muss das Face mehrmals durchgearbeitet werden. Wenn "svtd" auf null ist, wurde jeder SelVtx bearbeitet

	//erstmal runup machen zum spinWert
	i = null;
	UINT l3 = null;	//couter für schleifendurchlauf
	for(UINT u = null; u < spin; u++)
	{
		for(i = (i+1) % l; i < l; i = (i+1)%l)
		{
			if(f[i] & is)
			{
				break;
			}
		}
	}

	for(UINT o = null; o < 2; o++)
	{//äussere Schleife
		l3 = null;
		//for(i = 0; i < l; i++)
		for(; l3++ != l; i = (i+1)%l)	//damit spin mit einbezogen wird
		{//jeden Vtx im Array bearbeiten, der selected ist

			if( f[i]&is^is || f[i]&iv^iv)	//ohne klammerung, da operatorvorrang eindeutig
				continue;	//wenn er nich gewählt ist oder nicht gültig, dann mit nächstem Vtx fortfahren

			//jetzt checken ob in ArrayRichtung mindestens 1 ungewählter Vtx vorhanden ist
			//und natürlich ob die richtung noch nicht geblockt ist
	

			for(x = 1; x > -2; x-=2)	//x fungiert als multiplikator
			{

				flagValue = f[(i+x+l)%l];


				if( !( (this->*condition)( flagValue ) ) && flagValue&iv )
				{//richtung ist gültig und der nächste Vertex ist  nicht Selected (bzw er ist selected, aber die bedingung wurde geändert
					//jetzt solange suchen, bis ein selektierter Vtx gefunden wurde, und dann weitersehen


					tsb = i;	//momentaner lowBound, muss eventuell später umgesetzt werden
					


					for(z = (i+x+x+l)%l; ; z = (z+x+l)%l)
					{// jetzt den nächstbesten selVtx finden


						//hier findet er auf jeden Fall nen passenden Vtx, weshalb man keinen LoopSchutz einbauen muss

						if(f[z] & is && f[z] & iv)
						{//schleife beenden und endBound setzen
							
							teb = z;
							break;
						}
						

					}

					//Nach moeglichkeit sollten Quads oder nGons erstellt werden, weshalb für den Fall, dass es ein dreieck ist, noch in die andere Richtung
					//geschaut wird
					//->>nein, der User entscheidet durch seine Auswahl, was er haben will - mit spin kann er das dann noch verändern

					//jetzt das Aray fürs neue Face aufbauen
					//wenn die richtung umgekehrt, dann müssen auch die Bounds umgekehrt werden, damit er nicht das falsche Face erstell

					if( (x == 1) ? (! createFace(tsb,teb,f,faceVtx,nf) ) : (! createFace(teb,tsb,f,faceVtx,nf) ) )
					{//face ist ungültig, also mit nächster Richtung weitermachen
						continue;
					}

				
					changeFace = true;
					
					//ansonsten face zur liste hinzufügen
					newFaces.push_back(nf);

					

					
					tsb2 = tsb; teb2 = teb;		//-->jetzt dienen tsb2&teb2 aber als backup für original tsb/teb, falls die while schleife nix findet
					success = false;			//flag, ob die whileSchleife mindestens 1 fce erstellt hat
					go = true;					//EndlosSchleife

					//jetzt noch noch versuchen, weitere Faces anhand dieser methode zu erstellen
					while(go)
					{
						UINT t = tsb = teb;
						

						flagValue = f[(t+x+l)%l];

						//hier wird die Condition nicht benutzt
						if( flagValue & is || flagValue & iv^iv )
						//wenn vtx selected oder nicht gültig ist, abbruch
							break;

						//ansonsten die selben schritte ausführen wie zuvor schon
						for(z = (t+x+x+l)%l; ; z = (z+x+l)%l)
						{// jetzt den nächstbesten selVtx finden
							
							//in dem moment, wo der vtx ungültig ist, 
							//muss die Suche eh gestoppt werden
							if(f[z] & iv ^ iv)
								goto machWeiter;
							
							if(f[z] & is )
							{//schleife beenden und endBound setzen
								
								teb = z;
								break;
							}
							
						}

						//jetzt das Face erstellen
						if( (x == 1) ? (! createFace(tsb,teb,f,faceVtx,nf) ) : (! createFace(teb,tsb,f,faceVtx,nf) ) )
						{//face ist ungültig, also enfach mit nächster Iteration fortfahren
							continue;
						}
						else
						{//face ist gültig, es also auf liste packen
							changeFace = true;
							success = true;
							newFaces.push_back(nf);
						}

					}
					
					//wenn Sache nicht erfolgreich war, dann originalWerte zurückholen und weitermachen
					if(!success)
					{
						tsb = tsb2;
						teb = teb2;
					}
					
					//jetzt die erweiterte Suche starten, um den Rest der Faces zu erstellen 
					UINT bounds[4];



					while(go)
					{

						//jetzt von tsb ausgehen und nächstbesten selVtx finden (--> op beachten, +l zur sicherheit, da op auch negativ sein kann)
						for(z = (tsb+l-x)%l; ; z = (z+l-x)%l)
						{
							if( f[z] & iv ^ iv )
								//wenn ungültiger Vtx gefunden wurde, muss abgebrochen werden
								goto machWeiter;

							if( f[z] & is )
							{
								tsb2 = z;
								break;
							}

						}

						//und auch von teb aus in richtung der operation teb2 finden
						for(z = (teb+l+x)%l; ; z = (z+l+x)%l)
						{
							
							if( f[z] & iv ^ iv )
								//wenn ungültiger Vtx gefunden wurde, muss abgebrochen werden
								goto machWeiter;

							if( f[z] & is )
							{
								teb2 = z;
								break;
							}

						}
	

						//letztendlich Face erstellen, wenn alles korrekt
						if(x == -1)
						{//wenn op negativ, dann koennen bounds bleiben wie gehabt
							bounds[null] = tsb;			bounds[1] = tsb2;
							bounds[2] = teb;			bounds[3] = teb2;

						//	op = 1;	//richtung ist ab jetzt nicht mehr vertauscht
						}
						else
						{//wenn op positiv, müssen bounds umgekehrt werden
							bounds[2] = tsb;			bounds[3] = tsb2;
							bounds[null] = teb;			bounds[1] = teb2;
						}


						if( tsb == teb2 || teb == tsb2 || !createExtraFace( &bounds[null],f,faceVtx,nf)  )
						{// Okay, hier kann man nicht mehr weitermachen
							//also einfach die Bounds umsetzen und so mit nächstem face weitermachen

							
							tsb = tsb2;
							teb = teb2;


							continue;
						}

						//ansonsten ist alles in ordnung, und man kann die Marker umsetzen und das Face erstellen
						newFaces.push_back(nf);
						changeFace = true;

						tsb = tsb2;
						teb = teb2;

					}
					
					//da er einen gültigen Einstiegspunkt gefunden hat, muss er auf jeden Fall aussteigen aus allen Schleifen
					goto machWeiter;
					

				}


			}//for(x = 1; x > -2; x-=2)

			
		}//for(i = 0; i < l; i++


		//wenn er hier hinkommt, dann hat die obige Schleife keinen einstiegspunkt finden koennen
		condition = &vertexFty::gimmeFalse;
		//da die condition jetzt auf ginmmeFalse umgesetzt wurde, wird beim nächsten durchlauf auf jeden Fall ein passender Vtx gefunden
		


	}//for(UINT o = 0; o < 2; o++)
	

machWeiter:;
	//----------------------------------------------------------------------------------------
	//zuguterletzt die faces erstellen mit dem creator - er kümmert sich um UVs und Normalen
	//----------------------------------------------------------------------------------------

	if(!changeFace)
	{//wenn face nicht verändert wurde, dann speicher freigeben und raus hier
		free((void*)f);
		
		return;
	}

	//ansonsten zuerst noch die verbliebenen ValidVertex zu Face hinzufügen
	nf.clear();


	for(i = null; i < l; i++)
	{
		if(f[i] & iv)
			nf.append(faceVtx[i]);

	}

	if(nf.length() > 2)	//wenn nur 2 vertizen in diesem Face gewählt sind, bleiben auch nur 2 vertizen übrig - daraus sollte man kein Face macen :)
		newFaces.push_back(nf);
	
	//das erste Face sorgt für die Änderung des OriginalFaces
	MIntArray	 DummyArray;												//wird der changeUVIDs methode als leeres Array übergeben, weil es verlangt wird



	std::list<MIntArray>::iterator iter = newFaces.begin();


	
	
	ftyCreator->changePolyVtxIDs(faceID, *iter);


	ftyCreator->changeUVIds(faceID,DummyArray,*iter,faceVtx);


	ftyCreator->changeNormals(faceID,*iter);


	//aufs nächste Face schalten
	++iter;

	l = newFaces.size();
	for(i = 1; i < l; i++)
	{//jetzt die anderen faces erstellen

	 INVIS(helper.printArray(*iter," = FaceToCreate");)
	 
		ftyCreator->createPoly(*iter);


			ftyCreator->changeUVIds(faceID,DummyArray,*iter, faceVtx,true);


			ftyCreator->changeNormals(faceID,*iter,true);

		iter++;
	}


	//----------------------------------
	//Speicher freigeben
	//----------------------------------

	free((void*)f);


}


//-------------------------------------------------------------------------------
inline bool	vertexFty::createFace(UINT tsb, UINT teb, UCHAR* flags, MIntArray& faceVtx, MIntArray& result)
//-------------------------------------------------------------------------------
{
	result.clear();

	UINT l = faceVtx.length();


		//vorCheck: Wenn das neue Face das gesamte alte face umspannt, dann ist es ungültig und methode wird beendet
	if( (  ( (int)teb - (int)tsb < 0 ) ? (l - ((int)tsb - 1)) + teb : teb - tsb  )  == l )
		return false;
	
	
	//jetzt das neue Face anhand der bounds zusammenstellen und die Flags entsprechend ändern, allerdings nur für alle vertizen, die innerhalb der Bounds sind
	
	
	
	//zuerst Vtx1 anhämgen 
	result.append(faceVtx[tsb]);
	tsb = (tsb + 1)	% l;	//tsb ++ ist zu unsicher, weil es so über die bounds hinaus kommen kann
	

	//jetzt für den Rest der Faces vertizen anhängen und sie invalid machen - nur gültige vertizen werden angehängt
	//richtung beachten
	UINT i;
	for(i = tsb  ; i != teb; i = (i+1)%l)
	{
		
		result.append(faceVtx[i]);
		
		//da flags eigentlich nur einmal invalid gemacht werden, muss man hier keinen Sicherheitscheck machen
		flags[i] -= iv;
	}
	
	
	
	//nun noch den letzten Vtx hinzufügen
	result.append(faceVtx[teb]);
	
	

	//vorher noch face verifizieren -  am besten anhand seiner Fläche
	//diese Methode checkt auch (rechnerisch), ob das übrigbleibende Face korrekt wäre, weshalb hier nur eine abfrage gemacht wird
	if(!checkFaceIntegrity(result))
	{
		//jetzt alle UVFlags, die false gesetzt wurdem, wieder korrigieren und true setzen, damit andere prozeduren noch ein face erstellen koennen
		
		for( i = tsb ; i != teb; i = (i+1)%l)
		{
			
			result.append(faceVtx[i]);
			
			//da flags eigentlich nur einmal invalid gemacht werden, muss man hier keinen Sicherheitscheck machen
			//wenn man sie wieder valid macht
			flags[i] += iv;
		}
		
		return false;
	}


	return true;

}

//-------------------------------------------------------------------------------
inline bool	vertexFty::createExtraFace(UINT* bounds, UCHAR* flags, MIntArray& faceVtx, MIntArray& result)
//-------------------------------------------------------------------------------
{
	result.clear();

	UINT l = faceVtx.length();
	//bounds haben die Form: {tsb, tsb2, teb, teb2}




	UINT end = (bounds[1]+1) % l;	//damit bounds [1] auch noch auf result array kommt
	//erstmal alle Vtx von tsb - tsb2 hinzufügen
	UINT i;
	for(i = bounds[null]; i != end; i = (i+1)%l)
	{
		result.append(faceVtx[i]);
		flags[i] -= iv;
	}




	//tsb2 muss aber noch gültig bleiben, weshalb es jetzt wieder valid gesetzt wird
	flags[ bounds[1] ] += iv;





	//jetzt die restlichen (end)vtx hinzufügen
	//face hat nur 3 corners, also sollte die 3 corner nur einmal hinzugefügt werden
	if(bounds[1] != bounds[3])
	{//gut, man kann also teb2 hinzufügen, da teb2 != tsb2 - face hat also 4 corners
		result.append( faceVtx[ bounds[3] ] );
		//flags[ bounds[3]++] -= iv;	//es bleibt valid
	}
	


	bounds[3] = ( bounds[3] + 1 ) % l;



	//jetzt die restlichen Vtx aufs array tun
	end = ( bounds[2] + 1 ) % l;
	for(i = bounds[3]; i != end; i = (i+1)%l)
	{
		result.append(faceVtx[i]);
		flags[i] -= iv;
	}

	//teb2 muss aber noch gültig bleiben, weshalb es jetzt wieder valid gesetzt wird
	//flags[ bounds[3] ] += iv;
	

		//vorher noch face verifizieren -  am besten anhand seiner Fläche
	//diese Methode checkt auch (rechnerisch), ob das übrigbleibende Face korrekt wäre, weshalb hier nur eine abfrage gemacht wird
	if(!checkFaceIntegrity(result))
	{
		//jetzt alle UVFlags, die false gesetzt wurdem, wieder korrigieren und true setzen, damit andere prozeduren noch ein face erstellen koennen
		
		for( i = bounds[null]; i != bounds[1]; i = (i+1)%l)	//auf diese Weise wird bounds[1] nicht mit auf de validListe gesetzt, da es ja schon valid ist
		{
			flags[i] += iv;
		}


		for(i = bounds[3]; i != end; i = (i+1)%l)
		{
			flags[i] += iv;
		}
	
		return false;
	}




	return true;

}


//--------------------------------------------------------------------------
bool	vertexFty::checkFaceIntegrity		(	MIntArray& vtxList	)
//--------------------------------------------------------------------------
{
	//dieser algo wird probleme haben bei concaven Faces 
	double faceSize;
	double min = .001;		
	MPointArray points;


	ftyCreator->getVtxPositions(vtxList, points);
	
	faceSize = ftyCreator->getFaceSize(points);
	

//	INVIS(cout<<fabs(refValue - faceSize) / refValue<<" UND "<<faceSize / refValue<<endl;);


	if( fabs(refValue - faceSize) / refValue < min || faceSize / refValue < min )
		return false;
	else
		return true;


}



//----------------------------------------------------
bool	vertexFty::isSelected			(UCHAR value)
//----------------------------------------------------
{ 
	return (bool)(value&is);
}

//----------------------------------------------------
bool	vertexFty::gimmeFalse			(UCHAR value)
//----------------------------------------------------
{ //damit die aufrufende function nach negation true erhält
	return false;
}
