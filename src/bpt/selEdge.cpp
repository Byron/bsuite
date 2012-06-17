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

// selEdge.cpp: implementation of the selEdge class.
//
//////////////////////////////////////////////////////////////////////

#include <selEdge.h>



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-------------------------------------
selEdge::selEdge(int inID):edge(inID)
//-------------------------------------
{
	//newVtx initialisieren
	int i;
	for(i = 0; i < 4; i++)
	{
			newVtx[i] = -1;
			
			uID[i] = -1;
			 
	}

	for(i = 0; i < 2; i++)
	{
		smallFaceClean[i] = false;
		edgeVtx[i] = -1;
	}


}

//-------------------------------------------------------------------------------------------------------------------
selEdge::selEdge(MIntArray inFaceIDs,int inID):edge(inFaceIDs,inID)
//-------------------------------------------------------------------------------------------------------------------
{
	//newVtx initialisieren
	int i;
	for(i = 0; i < 4; i++)
	{
			newVtx[i] = -1;
			
			uID[i] = -1;
	}

	for(i = 0; i < 2; i++)
	{
		smallFaceClean[i] = false;
		edgeVtx[i] = -1;
	}


}



//------------------------
selEdge::~selEdge()
//------------------------
{
}



//---------------------------------------------------------------------
void	selEdge::setSingleGerade( MDoubleArray inGerade)
//---------------------------------------------------------------------
{
	for(int i = 0; i < 6; i++)
		gerade[i] = inGerade[i];
}

//---------------------------------------------------------------------
void	selEdge::setSinglePlane(int richtungsIndex, MDoubleArray inPlane)
//---------------------------------------------------------------------
{
	for(int i = 0; i < 6; i++)
		plane[richtungsIndex][i] = inPlane[i];
}


//muessen checken, ob ihre nachbarn on border sind: in diesem fall kuennten sie nuemlich auch unselected sein, 
//was besonderes vorgehen erfordert
//-------------------------------------------------------
edge*	selEdge::findNextSelEdge(edge* lastEdge)
//-------------------------------------------------------
{
//selEdges geben einfach ihren zeiger zurueck, da genau dieser das gewuenschte resultat ist

	return this;
}
//-------------------------------------------------------------------------------------------------------------------
MDoubleArray selEdge::getGerade(bool displace, const edge* requester, bool useUId, int uniqueID)
//-------------------------------------------------------------------------------------------------------------------
{
	//erstmal die Seite und richtung von requester im nachbarArray finden
	MDoubleArray	result(6);
	
	//(normalen sind normalisiert)
	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)	//hier muss nicht auf borderEdges geachtet werden
		{
			if(nachbarn[i][a] == requester)
			{
				if(useUId)
				{
					//a anpassen
					if(uniqueID != uID[i*2+a])
						a = 1-a;
					INVIS(cout<<"Habe richtung angepasst! Sie ist jetzt "<<a<<" mit face "<<faceIDs[a]<<endl; )
				}

				//wingedVtxCheck muss nicht vorgenommen werden, da ja lle plane/geradenSlots gefuellt sind
				MVector normal;
				normal.x = plane[a][3];normal.y = plane[a][4];normal.z = plane[a][5];
				
				
				//ortsvektor der gerade entlang der multiplizierten normale verschieben
				if(displace)
				{
					result[0] = gerade[0] + maxLength*normal.x;
					result[1] = gerade[1] + maxLength*normal.y;
					result[2] = gerade[2] + maxLength*normal.z;
				}
				else
				{
					result[0] = gerade[0];
					result[1] = gerade[1];
					result[2] = gerade[2];
				}
				

				//richtungsvektor hinzufuegen
				result[3] = gerade[3];result[4] = gerade[4];result[5] = gerade[5];
				
				

				return result;

			}
		}
	}
	
	//hier sollte er nicht hinkommen
	return result;
}


#include <maya/MFnTransform.h>
#include <maya/MFnDagNode.h>


//-----------------------------------------------------------------------------------------------------------------------------------------------
MPoint		selEdge::intersect(bool isEdgeDisplaced,bool displacePlane,edge* nachbar, bool isNurEinNachbar,bool chamfer)
//-----------------------------------------------------------------------------------------------------------------------------------------------
{

	
	MDoubleArray	ng; int tmp;
	if(isEdgeDisplaced)
	{
		if(displacePlane)
		{
			tmp = newVtx[side*2+dir];
			
			//wenn punkt noch nicht vorhanden, dann einfach
			if(tmp == -1)
			{
				ng = (isNurEinNachbar)?nachbar->getGerade(isEdgeDisplaced,this,true,uID[side*2+dir]):nachbar->getGerade(isEdgeDisplaced,this); //ng = nachbarsGerade
				
			}
			else
				return MPoint(tmp,-1.123456789,0.0);
		}
		else
		{
			//ansonsten im eigenen edgeVtx array schauen 
			//if(edgeVtx[side] != vtxIDs[0] && edgeVtx[side] != vtxIDs[1] && edgeVtx[side] == -1)
			if(edgeVtx[side] == -1)
				ng = nachbar->getGerade(isEdgeDisplaced,this); 
			else
				return MPoint(edgeVtx[side],-1.123456789,0.0);
		}
	}
	else
	{	//wenn edge nicht verschoben ist, muss plane folglich verschoben sein, da andernfalls der gemeinsame punkt
		//der kanten gefunden werden wuerde (->unfug)

		tmp = nachbar->getNewGeradenVtx(this);
		
		if(tmp != -1)
			return MPoint(tmp,-1.123456789,0.0);
		else
			ng = nachbar->getGerade(isEdgeDisplaced,this); 
	}

	//wenna bis 
	MVector normal;
	MVector ort;
	
	normal.x = plane[dir][3];normal.y = plane[dir][4]; normal.z = plane[dir][5];
	ort.x = plane[dir][0]; ort.y = plane[dir][1]; ort.z = plane[dir][2];


	if(displacePlane)
	{
		//normal.x *= maxLength;normal.y *= maxLength; normal.z *= maxLength;
		ort = ort + normal*maxLength;	
	}


	
		//jetzt das "d" der koordinatenform erreichen
	double d =  -ort * normal ;

	d += normal.x * ng[0] + normal.y * ng[1] + normal.z * ng[2]; 

	double r = normal.x * ng[3] + normal.y * ng[4] + normal.z * ng[5];
	
	d = -d/r;

//	cout<<d<<" / "<<r<<endl;


	return MPoint(ng[0] + d * ng[3], ng[1] + d * ng[4], ng[2] + d * ng[5]);

	


}

//------------------------------------------------------------------------------------------------------------------------------
int		selEdge::intersect(bool isEdgeDisplaced,bool displacePlane,edge* nachbar, bool isNurEinNachbar)
//------------------------------------------------------------------------------------------------------------------------------
{
	//diese Proc geht davon aus, dass der edgeNachbar selected ist. Dies muss vor aufruf sichergestellt werden
	//ansatz: nachbar gibt entlang der planeNormale * edge::maxLength verschobene gerade zurueck, die dann mit der ebenfalls
	//verschobenen plane dieser selEdge intersected wird
	//der neue Vertex wird automatisch erstellt falls nuetig und die neue ID wird zurueckgegeben
	//plane muss erst noch verschoben werden und in Koordinatenform umgeformt werden wenn displacePlane = true


	//MERKE: NewVtx werden bei Intersections nie gemittelt! Nur b

	MDoubleArray	ng; int tmp;
	if(isEdgeDisplaced)
	{
		if(displacePlane)
		{
			tmp = newVtx[side*2+dir];
			
			//wenn punkt noch nicht vorhanden, dann einfach
			if(tmp == -1)
			{
				ng = (isNurEinNachbar)?nachbar->getGerade(isEdgeDisplaced,this,true,uID[side*2+dir]):nachbar->getGerade(isEdgeDisplaced,this); //ng = nachbarsGerade
				
			}
			else //derartige Vtx werden niemals gemittelt, da eh das selbe ergebnis rauskueme (im IdealFalle ;) )
				return tmp;
		}
		else
		{
			//ansonsten im eigenen edgeVtx array schauen 
			
			tmp = edgeVtx[side];	

			//auf jeden Fall nen intersection Vtx erstellen fuer spuetere Mittelung
			
			ng = nachbar->getGerade(isEdgeDisplaced,this); 
			
		}
	}
	else
	{	//wenn edge nicht verschoben ist, muss plane folglich verschoben sein, da andernfalls der gemeinsame punkt
		//der kanten gefunden werden wuerde (->unfug)

		tmp = nachbar->getNewGeradenVtx(this);
		
		
		ng = nachbar->getGerade(isEdgeDisplaced,this); 
	}


	MVector normal;
	MVector ort;

	normal.x = plane[dir][3];normal.y = plane[dir][4]; normal.z = plane[dir][5];
	ort.x = plane[dir][0]; ort.y = plane[dir][1]; ort.z = plane[dir][2];

	


	if(displacePlane)
	{
	//	normal.x *= maxLength;normal.y *= maxLength; normal.z *= maxLength;

		ort = ort + normal*maxLength;	
	}



	
	//jetzt das "d" der koordinatenform erreichen
	double d =  -ort * normal ;

	d += normal.x * ng[0] + normal.y * ng[1] + normal.z * ng[2]; 

	double r = normal.x * ng[3] + normal.y * ng[4] + normal.z * ng[5];
	
	d = -d/r;


	
//	if( ((tmp == -1) & !isNurEinNachbar) | ( ( (newVtx[side*2+dir] == -1 ) & isNurEinNachbar ) || !(tmp = newVtx[side*2+dir]) ) )
	if( tmp == -1 )
	{//es muss ein neuer punkt erstellt werden

		return creator->newVtx(MPoint(ng[0] + d * ng[3], ng[1] + d * ng[4], ng[2] + d * ng[5]));
	}
	else
	{//der neue Punkt muss mit dem vorhandenen gemittelt werden
		//MPoint result(ng[0] + d * ng[3], ng[1] + d * ng[4], ng[2] + d * ng[5]);
		
		//damit Linux funzt
		
		MPoint tmpPos = creator->getVtxPoint(tmp);
		MPoint tmpPos2(ng[0] + d * ng[3], ng[1] + d * ng[4], ng[2] + d * ng[5]);
		tmpPos = (tmpPos + tmpPos2) / 2;
		creator->setVtxPoint(tmp, tmpPos);
	
		
		//jetzt noch den vorhandenen, jedoch genittelten punkt zurueckgeben
		return tmp;
	}



}

//--------------------------------------------------
void selEdge::setNewVerts(const edge* requester, MIntArray& inNewVerts, bool useUID, int uniqueID)
//--------------------------------------------------
{
	//setNewVerts muss von solidChamfer aus aufgerufen werden
	//erstmal richtige indizes finden
	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(nachbarn[i][a] == requester)
			{
				if(useUID)
				{
					if(uniqueID != uID[i*2+a])
						a = 1 - a; 
					
				}
				
				if(inNewVerts[0] != -1)
					newVtx[i*2+a] = inNewVerts[0];
				
				
				if(inNewVerts[1] != -1)
					edgeVtx[i] = inNewVerts[1];
				
				return;
			}
		}
	}
	
}

//------------------------------------------------------------------------------------------------
void		selEdge::getValidRichtung(const edge* requester, int& seite, int& richtung, int uniqueID)
//------------------------------------------------------------------------------------------------
{
	//setNewVerts muss von solidChamfer aus aufgerufen werden
	//erstmal richtige indizes finden
	for(seite = 0; seite < 2; seite++)
	{
		for(richtung = 0; richtung < 2; richtung++)
		{
			if(nachbarn[seite][richtung] == requester)
			{
				if(uniqueID > -1)
				{
					if(uniqueID != uID[seite*2+richtung])
						richtung = 1 - richtung; 
					
				}
				
				return;
			}
		}
	}	
}


//--------------------------------------------------
int		selEdge::getNewGeradenVtx(const edge* requester)
//--------------------------------------------------
{
		//erstmal richtige indizes finden
	for(int i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(nachbarn[i][a] == requester)
			{
				return edgeVtx[i];
			}
		}
	}
	//forbiddenPath
	cout<<"BIN IN FORBIDDEN PATH VON GET_NEW_GERADEN_VTX"<<endl;
	return (requester->getVtx())[0];
}


//--------------------------------------------------
void	selEdge::setUIDs(MIntArray& IDs)
//--------------------------------------------------
{
	//IDs muss immer luenge von 4 haben!
	for(int i = 0; i < 4; i++)
	{
		if(IDs[i] != -1)	//dienst als flag, um auch einzelne UIDS (in besonderen fuellen) mit dieser methode setzen uezu kuennen ohne vorhandene zu ueberschreiben
		{
			uID[i] = IDs[i];
		}
	}
}

//--------------------------------------------------------------
int		selEdge::getValidRichtung(int uniqueID)
//--------------------------------------------------------------
{
	for(int i = 0; i < 4; i++)
		if(uID[i] == uniqueID)
			return i%2;

	return -1;
}



//--------------------------------------------------------------
bool	selEdge::isParallel(double threshold, edge* nachbar,double& value, int richtung, int uniqueID)
//--------------------------------------------------------------
{
	MVectorArray*	nNormals = nachbar->getNormals();
	MVector normal(0.0,0.0,0.0);

//	cout<<"nNormals Length = "<<nNormals->length()<<endl;
//	cout<<"normals.length = "<<normals.length()<<endl;

	int nRichtung = 0;
	if(richtung != -1 && nachbar->isSelected() )
	{	
		//checken, ob faces in zwei richtungen parallel sind oder nur in einer
		MIntArray nFaces = nachbar->getFaces();
		nRichtung = nachbar->getValidRichtung(uniqueID);
		//entsprechende richtung fuers nFace holen
		//solche edges werden immer biDirektional bearbeitet
	}

	  //alle normalen der umgebung mitteln

	if(richtung == -1  || !(nachbar->isSelected()))
	{
		UINT i;	
		for(i = 0; i < nNormals->length();i++)
		{
			normal += (*nNormals)[i];
		}
		
		for(i = 0; i < normals.length();i++)
		{
			normal += normals[i];
		}

	}
	else
	{
		normal += normals[richtung];
		normal += (*nNormals)[nRichtung];
	}

	MIntArray allVtx = nachbar->getVtx(); //bei borderEdges mit wingedVtx fuehrt dieses Verhalten dazu, das die vektoren falsch eingestellt werden
											//man sollte hier mit wingedVtx arbeiten um gueltige Vektoren zu erhalten.
	
	//uebereinstimmenden alle Vtx in reihenfolge eintragen : left - center - right
	int i;
	for(i = 0; i < 2; i++)
	{
		for(int a = 0; a < 2; a++)
		{
			if(allVtx[i] == vtxIDs[a])
			{
				if(i == 1)
					allVtx.append(vtxIDs[1-a]);		
				else
					allVtx.insert(vtxIDs[1-a],0); //dann hat dieses array unter Umstuenden einen Vtx zu viel drauf, was aber 
												//nix macht, da nur die ersten 3 in der richtigen reihenfolge wichtig sind
				
				break;
			}
		}
	}

	//printArray(allVtx," = allVtx");

	//beliebigen punkt der edge als startPubkt nehmen
	MVector ort = creator->getVtxPoint(vtxIDs[0]);
	
	//alle positionen der vertizen holen
	MPointArray	vtxPoints;
	for(i = 0; i < 3; i++)
		vtxPoints.append(creator->getVtxPoint(allVtx[i]));


	//setUp der intersectPlane
	double r, tmp;
	double d = normal * -ort ;

	r = normal * normal;
	
	for(i = 0; i < 3; i++)
	{
		tmp = d;
		//intersection starten mit geraden die entlang normale verschoben sind
		
		tmp += normal * vtxPoints[i]; 
		

		tmp = -tmp/r;

		//neuen Vtx gleich hier eintragen und den alten 
		vtxPoints[i] = vtxPoints[i] + tmp*normal; //hier eventuell pro komponente multiplizieren
	}

	MVector dir1 = vtxPoints[1] - vtxPoints[0];
	MVector dir2 = vtxPoints[2] - vtxPoints[1];
	
	/*//debug
	MFnTransform tr;MFnDagNode dagFn;

	for(i = 0; i < 3; i++)
	{
		tr.setObject(dagFn.create("locator", "verschoben"));
		tr.setTranslation(vtxPoints[i],MSpace::kTransform);
	}
	*/
	//output
	INVIS(cout<<"WERT VON IS_PARALLEL IST: "<<fabs(1.0 - fabs((dir1*dir2)/(dir1.length() * dir2.length())))<<endl;)
	value = fabs( 1.0 - fabs((dir1*dir2)/(dir1.length() * dir2.length())) );
	//if( fabs( 1.0 - fabs((dir1*dir2)/(dir1.length() * dir2.length())) ) < threshold) 
	if( value < threshold) 
		return true;
	else
		return false;

	
	
}





//-------------------------------------------------------------------
MPoint		selEdge::getDisplacePoint(int startID, edge* otherEdge)
//-------------------------------------------------------------------
{
	//Erstellt punkt, der um maxLength entlang der definierten edge vom startpunkt aus verschoben ist
	MIntArray nVtx = otherEdge->getVtx();

	MPoint A = creator->getVtxPoint(startID);
	
	MPoint B = creator->getVtxPoint(nVtx[0] == startID ? nVtx[1] : nVtx[0] );

	MVector c = B - A;
	c.normalize();

	return A + c * maxLength;

}


//--------------------------------------------------------------
MIntArray	selEdge::checkNewFaceIntegrity(MIntArray& newFaces)
//--------------------------------------------------------------
{
	//Da es immernoch zu fehlern bei den NewID kommen kann, sollte diese Procedur zu debugzwecken darueber 
	//aufklueren und den fehler beheben
	int lastID = -1;
	int currentID;
	for(unsigned int i = 0; i < newFaces.length();i++)
	{
		currentID = newFaces[i];
		if(currentID == -1 || currentID == lastID)
		{
			INVIS(MGlobal::displayError("ERROR: New Face was defect, corrected this error!"););
			newFaces.remove(i--);
		}

		lastID = currentID;
	}
		return newFaces;
}


//--------------------------------------------------------------
double			selEdge::get3dArcRatio(edge* nachbar, int seite)
//--------------------------------------------------------------
{
	MIntArray nVtx = nachbar->getVtx();

	MPoint thisCorner = creator->getVtxPoint(vtxIDs[seite]);

	MVector dir1 = thisCorner - creator->getVtxPoint(vtxIDs[1 - seite]);
	dir1.normalize();

	MVector dir2 = thisCorner - creator->getVtxPoint( (nVtx[0] == vtxIDs[seite]) ? nVtx[1] : nVtx[0] );
	dir2.normalize();

	return fabs( dir1*dir2 );
}





//--------------------------------------------------------------------------
void	selEdge::engageBevel(int connectEnds, int triangulateEnds, bool isSC)
//-------------------------------------------------------------------------
{

	
	MPRINT("--------------------------------------------------------")
	INVIS(cout<<"ENGAGE EDGE (BEVEL): <<<<  "<<id<<"  >>>>"<<endl;)
	MPRINT("--------------------------------------------------------")
	MPRINT("--------------------------------------------------------")
	MPRINT("--------------------------------------------------------")



	//zuerst pruefen, ob bereits faceData vorhanden ist, oder noch erstellt werden muss

	
	for(dir = 0; dir < faceIDs.length(); dir++)
	{
		//ist faceData in dieser Richtung vorhanden?
		if( faceData[dir] == 0)
		{//nein, also neues Face erstellen
			//MERKE: Es kann dazu kommen, dass eine FaceEinheit mehrmals fuer ein und dieselbe FaceID erstellt wird. Dies macht allerdings nichts!
			//InitFaceData sorgt auch dafuer, dass die Angrenzenden SelEdges nen Ptr zur faceData erhalten
			initFaceData();
			
		}
	}

	//jetzt die Seiten pruefen: Wenn auf einer Seite in jede Richtung 2 unselectedEdges sind, dann muss geprueft werden, ob sich diese eine FaceData einheit teilen muessen
	//(damit die connectEnds methode noch richtig funktioniert)
	if(faceIDs.length() > 1)
	{//nachbarn sind jetzt auf jeden Fall definiert
		for(side = 0; side < 2; side++)
		{
			if(  !nachbarn[side][0]->isSelected() && !nachbarn[side][1]->isSelected() ) 
			{
				MIntArray nFaces = nachbarn[side][0]->getFaces();
				MIntArray match = creator->helper.matchArray(nFaces, nachbarn[side][1]->getFaces());
				
				if(match.length() == 1)
				{//sie teilen sich ein face, also eine anweisen, die faceData zu initialisieren, und dann das gemeinsame faceobject der anderen edge zuweisen
					UINT i;
					for(i = 0; i < 2; i++)	//man kann hier definitiv davon ausgehen, dass die nselEdge 2 faces hat
					{
						if(nFaces[i] == match[0])
						{//jetzt fuer dieses Seite eine faceData einheit erstellen - diese wurde garantiert noch nicht erzeugt
							nachbarn[side][0]->initFaceData(i);
							break;
						}
					}

					//jetzt noch dem anderen nachbarn dieses Face zuweisen
					edgeFaceData* nFP[2];
					nachbarn[side][0]->getFaceData(nFP[0], nFP[1]);

					nachbarn[side][1]->setThisFaceData(nFP[i]->id,nFP[i]); 
					
				}
			}
		}
	}

	

	clean = true;
	
	double value = 0.0;

	//jetzt pro seite pro richtung alles abarbeiten, unterscheiden zwischen wingend und nonwingend Edge
	bool nurEinNachbar[2] = {false,false}; //beeinflusst das spuetere verhalten bei erstellung der faces
	bool onBorder[4] = {false,false,false,false}; //fuer spuetere faces wichtig
	bool emergencyStop[2] = {false,false};	//pro Richtung
	//pro seite

	C_IChangedUV[0] = false;	//default ist false - diese Edge hat die UV von origVtx nicht verschoben
	C_IChangedUV[1] = false;
//	int refIndices[2] = {-1,-1}; //-1 ist flag
	int refFaces[2] = {-1,-1}; //-1 ist flag;
	int refDirs[2] = {0,0};

	MPoint tmpPos;	//als tempvariable - ntig, damit Linux richtig funzt - MERKE: Referenzen drfen in Linux KEINE temporre Variable sein!
	MVector tmpVec;


	bool	newVtxExists = false;		//flag: Wenn NewVtxX bereits durch andere Edge gesetzt wurde, ist er true
	bool	edgeVtxExists = false;		//flag: Wenn edgeVtxX bereits durch andere Edge gesetzt wurde, ist er true

	int origConnectEnds = connectEnds;	//wenn connectEnds deaktiviert wird (weil die refFaces uebereinstimmen, geschieht dies pro seite. Also muss pro seite der oriinalzustand wiederhergestellt werden
	


	bool hitsBorder;			//der uerst wichtig ist fr ChamferMode und edges, die direkt oder indirekt mit borderEdges verbunden sind


	for(side = 0; side < 2; side++)
	{
		connectEnds = origConnectEnds;
		
		refDirs[0] = refDirs[1] = -1;

		refFaces[0]  = refFaces[1] = -1;

		emergencyStop[0] = emergencyStop[1] = false;
		
		//im nurEinNachbar fall ist Chamfer wie SolidChamfer, im Grunde ist fr CHAMFER aber egal - es funzt im nurEin Nachbar modus genauso wie ohne den Modus
		nurEinNachbar[side] =		(faceIDs.length() == 1 && nachbarn[side][0]->isSelected())
							||	nachbarn[side][0] == nachbarn[side][1];
							
		hitsBorder = (nachbarn[side][0]->isSelected() && nachbarn[side][0]->borderThere(id, this))
					||	(nachbarn[side][1] != 0 && nachbarn[side][1]->isSelected() && nachbarn[side][1]->borderThere(id, this));


		if(	nurEinNachbar[side] | hitsBorder)//wenn eine Edge in einer der beiden richtungen on border, dann das nur ein nachbar schema verwenden
			//Extrem aufwendig hier, muss aber sein!
		{

			//check, ob die vektoren parallel sind

			int t[2];
			
			for(dir = 0; dir < faceIDs.length();dir++)
			{

				newVtxExists = (newVtx[side*2+dir] == -1) ? false : true;	//ist notwendig, damit die nSelEdges effizienter arbeiten
				

				if(nachbarn[side][dir]->isClean())
				{
					if(!isSC & (faceIDs.length() == 2) & hitsBorder )
					{//beim Chamfer muss man jetzt davon ausgehen, dass ein nachbar on border liegt, der vorher schon evaluiert hat. Also muss man den edgeVtx setzen
						edgeVtx[side] = vtxIDs[side];
					}

					goto finishIt;
				}

				

				if(nSelNachbarn[side][dir] != 0 && onlyOneNSelNachbar[side*2+dir] && nachbarn[side][dir]->isSelected() )	//es kommt in seltenen Situationen vor, dass der nachbar nicht gewhlt ist - dann muss ein extrapfad gegangen werden
				{
				//	double arcValue =  get3dArcRatio(nSelNachbarn[side][dir], side);

					//hier immer intersecten, wenn mueglich
				//	if( fabs(arcValue - (1 - value)) < 0.75)
				//	{
						MPRINT("BEARBEITE PARALLELE EDGES , ES GIBT NUR INEN NACHBARN, VERWENDE NSELEDGE")
							
							
						MPRINT("NACHBAR WAR CLEAN")
							
						if(isSC)
							newVtx[side*2+dir] = intersect(false,true,nSelNachbarn[side][dir],true);
						else
						{
							tmpPos = intersect(false,true,nSelNachbarn[side][dir],true, true);
							insertNewVtx( tmpPos ,hitsBorder);
						}	


						
						goto finishIt;
				//	}
				

				}
				
				if(isParallel(0.06,nachbarn[side][dir],value,dir,uID[side*2+dir]) )
				{


					MPRINT("ES GIBT NUR EINEN NACHBARN, PARALLEL")
						
					if(!newVtxExists)
					{//vtx existiert noch nicht, also neuen erstellen

						if(isSC)
						{		
							tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength);
							newVtx[side*2+dir] = creator->addWingedDisplacedVtx(vtxIDs[side],tmpVec);				
						}
						else
						{
								tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength);
								tmpPos = creator->getWingedDisplacedPoint(vtxIDs[side],tmpVec);
								insertNewVtx(tmpPos, hitsBorder);
						}

					}

					
				}
				else
				{
					MPRINT("ES GIBT NUR EINEN NACHBARN")
					//in diesem fall muss die gefundene Edge

					if(isSC)
						newVtx[side*2+dir] = intersect(true,true,nachbarn[side][dir],true); 
					else
					{
						tmpPos = intersect(true,true,nachbarn[side][dir],true, true);
						insertNewVtx(tmpPos,hitsBorder);
					}



					

				}


finishIt:



				//jetzt die nachbarfaces anpassen, welche ueber diese selEdge nicht erreichbar sind
				
				if(newVtxExists)	//kommt er hier ueberhaupt je rein??
				{//vtx hat bereits existiert, also muss die nSelEdge geluescht werden, falls vorhanden
					//wenn man dies nicht machen wuerde, wuerde das entsprechende Face doppelt veruendert werden, was nun wirklich nicht sein muss .)
					/*
					//auch wenn der Vtx schon existierte, muss er noch mal hinzugefuegt werden, wenn dieses Face nicht mit nachbarn geteilt wird
					MIntArray nFaces = nachbarn[side][dir]->getFaces();
					bool dsNotShareFace = ((creator->helper.matchArray(faceIDs, nFaces).length() == 1) ? false : true);
					
					
					
					if(dsNotShareFace)
					{//Sie teilen kein Face, also Vtx hinzufuegen
					//	addVtxChange(newVtx[side*2+dir], nachbarn[side][dir]);	->das muss nicht gemacht werden, weil dies definitiv schon gemacht wurde von der edge, die den Vtx erzeugt hat
						
						if(nSelNachbarn[side][dir] != 0)
						{
							MIntArray nVtx = nSelNachbarn[side][dir]->getVtx();
							//den Vtx auch nochmal als edgeVtx eintragen  damit der NormalLookup ordentlich funzt
							faceData[dir]->insertEdgeVtx(vtxIDs[side], (vtxIDs[side] == nVtx[0]) ? nVtx[1] : nVtx[0],newVtx[side*2+dir]); 
						} 
					}
					


					nSelNachbarn[side][dir] = 0;
					*/
					
				}
				else
				{//einfach die changeFace methode der nSelEdge aufrufen bzw. alles erstmal auf Array zwischenspeichern
					
					//jetzt den neuen Vtx eintragen in faceData
					//wenn der nachbar dieser Seite NICHT selected sein sollte (was durchaus in seltenen Borderedges Situationen vorkommen kann)
					//if( nachbarn[side][dir]->isSelected() )
						addVtxChange(newVtx[side*2+dir], nachbarn[side][dir]);
					//else
					//	insertNewEdgeVtx(newVtx[side*2+dir], false, nachbarn[side][dir]);

					
					
					if(nSelNachbarn[side][dir] != 0)
					{	
						
						
						
						MIntArray nVtx = nSelNachbarn[side][dir]->getVtx();
						
						//den Vtx auch nochmal als edgeVtx eintragen  damit der NormalLookup ordentlich funzt - UVs sind hier egal, da es sich ja um einen newVtx handelt
						//aber nur, wenn der newVtx wirklich neu ist (was im Fall  von Chamfer WICHTIG ist
						if(newVtx[side*2+dir] > creator->getInitialVtxCount())
							faceData[dir]->insertEdgeVtx(vtxIDs[side], (vtxIDs[side] == nVtx[0]) ? nVtx[1] : nVtx[0],newVtx[side*2+dir]); 
						
										


						nSelNachbarn[side][dir] = 0;	//man sollte nicht mehr auf den Ptr zugreifen
					}
					
					t[0] = newVtx[side*2+dir];		t[1] = -1;
			
					MIntArray tmpArray(t,2);
					nachbarn[side][dir]->setNewVerts(this, tmpArray,true,uID[side*2+dir] );

					
					
				}
				





			}//for(unsigned int dir = 0; dir < faceIDs.length();dir++) ENDE

			
			if(isSC)//jetzt einfach die edgeVtx auf Vtx setzen, man muss auf nichts achten, da hier eh nur ein nachbar ist
				edgeVtx[side] = vtxIDs[side];
		}
		else
		{
			
			for(dir = 0; dir < faceIDs.length(); dir++)
			{


				newVtxExists = (newVtx[side*2+dir] == -1) ? false : true;

						//hier wird quasi gecheckt, ob eine andere edge (oder diese edge selbst) bereits nen edgeVtx erzeugt hat
				edgeVtxExists = (edgeVtx[side] == -1) ? false : true;


				//hier noch checken, ob die nachbarEdge parallel zu dieser hier ist
				//!!Dieser check sollte besser in "intersect untergebracht werden"
				//sel oder nSel?
				edge* nEdge = nachbarn[side][dir]; //nEdge == nachbarEdge
				isParallel(0.015,nEdge,value);
				//ich habe das so gemacht, damit ich notfalls separate kontrolle ueber den Threshold von nurEinNachbarEdges und
				// allen anderen habe ... dies eventuell noch uendern
				int ngVtx = -1;	//==nachbargeradenVtxID (der Vtx, der auf der edge des nacbarn liegt
				
				
				if(nEdge->isSelected())
				{
					
					if( value < 0.015 )
					{
						
						MPRINT("BEARBEITE PARALLELE SELECTEDEDGE")
							
				
							
							if(!newVtxExists)
							{
								if(nSelNachbarn[side][dir] != 0 && onlyOneNSelNachbar[side*2+dir])
								{//nach mueglichkeit mit nem nSelNachbarn intersecten
									double arcValue =  get3dArcRatio(nSelNachbarn[side][dir], side);
									
									if( fabs(arcValue - (1 - value)) < 0.75)
									{
											if(isSC)
												newVtx[side*2+dir] = intersect(false,true,nSelNachbarn[side][dir]);
											else
											{
												tmpPos = intersect(false,true,nSelNachbarn[side][dir], false, true);
												insertNewVtx(tmpPos,hitsBorder);
											}

												

										
									}
									else
									{
										if(isSC)
										{		
											tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength);
											newVtx[side*2+dir] = creator->addWingedDisplacedVtx(vtxIDs[side],tmpVec);	
										}
										else
										{

											tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength);
											tmpPos = edge::creator->getWingedDisplacedPoint(vtxIDs[side],tmpVec);
											insertNewVtx(tmpPos,hitsBorder);
										}
									}
									
									
								}
								else
								{
									if(isSC)
									{	
										tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength);
										newVtx[side*2+dir] = creator->addWingedDisplacedVtx(vtxIDs[side],tmpVec);	
									}
									else
									{
										tmpVec = MVector(plane[dir][3]*maxLength,plane[dir][4]*maxLength,plane[dir][5]*maxLength); 
										tmpPos = creator->getWingedDisplacedPoint(vtxIDs[side],tmpVec);
										insertNewVtx(tmpPos,hitsBorder);
									}
								}

								addVtxChange(newVtx[side*2+dir], nachbarn[side][dir]);
								
							}
							

							//edge und otherVtx werden nur von SC berechnet
							if(isSC)
							{//OTHEREDGE VTX
								MPoint pos = getDisplacePoint(vtxIDs[side], nEdge);
								int tmp = nEdge->getNewGeradenVtx(this);
								
								if( tmp == -1)
								{
									//also einen displacePoint auf geraden erstellen
									
									ngVtx = creator->newVtx(pos);
									
									insertNewEdgeVtx(ngVtx,  false, nEdge);
								}
								else
								{//neuen Punt erstellen und mitteln
									
									ngVtx = tmp;	//muss hier eigentlich nicht sein, aber ich mache das mal der ordnung halber ;)
									
									tmpPos = (creator->getVtxPoint(ngVtx) + pos) / 2;
									creator->setVtxPoint(ngVtx, tmpPos);

												
									insertNewEdgeVtx(ngVtx, true, nEdge);
								}
								
								
								//EDGEVTX
								tmp = edgeVtx[side];
								pos = getDisplacePoint(vtxIDs[side], this);
								
								if(edgeVtx[side] == -1)
								{
									edgeVtx[side] = creator->newVtx(getDisplacePoint(vtxIDs[side], this));
									insertNewEdgeVtx(edgeVtx[side], false, this);
								}
								else
								{
									tmpPos = (creator->getVtxPoint(tmp) + pos) / 2;
									creator->setVtxPoint(tmp, tmpPos);

									
									insertNewEdgeVtx(edgeVtx[side], true, this);
								}
								
							}
					}
					else
					{
						
						
						INVIS(cout<<"BEARBEITE NORMALE SELECTED EDGE, RICHTUNG "<<faceIDs[dir]<<", SEITE "<<vtxIDs[side]<<endl;)
							

							
							//vertizen erstellen - aber nur wenn er noch nicht vorhanden ist - es muss in diesem Fall keine Mittelung von 2 samples erfolgen
							if(!newVtxExists)
							{
								if(nSelNachbarn[side][dir] != 0 && onlyOneNSelNachbar[side*2+dir] )
								{
									double arcValue =  get3dArcRatio(nSelNachbarn[side][dir], side);
									
									if( fabs(arcValue - (1 - value)) < 0.75)
									{	
										if(isSC)
											newVtx[side*2+dir] = intersect(false,true,nSelNachbarn[side][dir]);
										else
										{
											tmpPos = intersect(false,true,nSelNachbarn[side][dir], false, true);
											insertNewVtx(tmpPos,hitsBorder);
										}

											
									
									}
									else
									{
										if(isSC)
											newVtx[side*2+dir] = intersect(true,true,nEdge);
										else
										{
											tmpPos = intersect(true,true,nEdge, false, true);
											insertNewVtx(tmpPos,hitsBorder);
										}

											
									}
									
									
								}
								else
								{
									if(isSC)
										newVtx[side*2+dir] = intersect(true,true,nEdge);
									else
									{
										tmpPos = intersect(true,true,nEdge, false, true);
										insertNewVtx(tmpPos,hitsBorder);
									}
									
										
								}

								//Vtx hinzufuegen
								addVtxChange(newVtx[side*2+dir], nachbarn[side][dir]);

						
							}
						


							//EDGEVTX SIND NUR FueR SC
							if(isSC)
							{
								//wenn nachbar unter einem gewissen treshold werden die neuen Vtx auf der edge durch displacement erzeugt anstatt durch intersection
								//, dies hat einen uehnlichen effekt wie die Rekursion, ist aber verstuendlicher, leichter und weniger fehleranfuellig
								if(value < 0.5)
									//if(true)
								{//punkte durch displacement
									
									//OTHEREDGE VTX
									int tmp =  nEdge->getNewGeradenVtx(this);
									MPoint pos;

									pos = getDisplacePoint(vtxIDs[side], nEdge);
									
									if(tmp == -1)
									{
										ngVtx = creator->newVtx(pos);
										
										insertNewEdgeVtx(ngVtx, false, nEdge);
									}
									else
									{//ansonsten punkte mitteln
										ngVtx = tmp;

										tmpPos = (creator->getVtxPoint(ngVtx) + pos) / 2;
										creator->setVtxPoint(ngVtx, tmpPos);

										
										
										insertNewEdgeVtx(ngVtx, true, nEdge);
									}
									
									
									
									//EDGEVTX
									tmp = edgeVtx[side];
									//dasselbe nochmal fuer den edgeVtx
									pos = getDisplacePoint(vtxIDs[side], this);
									
									
									if(tmp == -1)
									{
										edgeVtx[side] = creator->newVtx(pos);
										
										insertNewEdgeVtx( edgeVtx[side], false, this );
									}
									else
									{
										tmpPos = (creator->getVtxPoint(tmp) + pos) / 2;
										creator->setVtxPoint(tmp, tmpPos);

															
										insertNewEdgeVtx( edgeVtx[side], true, this);
									}
									
									
								}
								else
								{//punkte durch intersection
									bool tmpVar = (nEdge->getNewGeradenVtx(this) == -1) ? false : true;	//wird fuer insert edgeVtx verwendet
									
									ngVtx = intersect(false,true,nachbarn[side][dir]);
									
									insertNewEdgeVtx( ngVtx, tmpVar, nEdge);
									
									
									tmpVar = (edgeVtx[side] == -1) ? false : true;
									
									edgeVtx[side] = intersect(true, false,nEdge);
									
									insertNewEdgeVtx( edgeVtx[side], tmpVar, this);
								}

							}

						
						}//NORMALE SELEDGE ENDE



						
						//neue Vtx nur setzen, wenn nachbar noch nicht dran war, wobei es auch egal wuere, wenn er schon dran war -> es bringt huechstens etwas performance
						if(  !(nachbarn[side][dir]->isClean())  )
						{
							int t[] = {newVtx[side*2+dir],ngVtx};

							MIntArray tmpArray(t,2);
							nachbarn[side][dir]->setNewVerts(this, tmpArray );

							
						}
					
					
						

					}//SELEDGES ENDE
					else
					{
						//UNSEL EDGES
						MIntArray faces = nEdge->getFaces();



						
						if(value < 0.015)
						{
							//alles andere kann nicht bearbeitet werden
							cout<<"Warning, angle between edge "<<id<<" and edge "<<nachbarn[side][dir]->getID()<<" is too low, triangle created instead."<<endl;
							MGlobal::displayError("Warning, angle between edges is too low: Triangle has been created instead.");
							//Vtx manipulieren, um ein dreieck zu erzeugen
							newVtx[side*2+dir] = edgeVtx[side] = vtxIDs[side];
							
							onBorder[side*2+dir] = true;
							emergencyStop[dir] = true;
						}
						else
						{
							//MERKE:Bei unselEdges werden die newVtxIDs wie endEdges betrachtet und entsprechend in die faceData eingefuegt
							//es muss eine angepasste prozedur her, die mit nSelEdges umgehen kann

							
							MPRINT("BEARBEITE NORMALE UNSELECTED EDGE ON BORDER")
							
							//in diesem Fall ist es egal, wieviele Faces diese edge hat - darum wird sich spueter gekuemmert, alls methoden die hier
							//gerufen werden kuennen damit umgehen

							onBorder[side*2+dir] = true;	//damit kein versuch gestartet wird, smallFaces zu erstellen
							
							if(isSC)
								newVtx[side*2+dir] = intersect(false,true,nEdge);
							else
							{
								tmpPos = intersect(false,true,nEdge, false, true);
								insertNewVtx(tmpPos,hitsBorder);
							}

								
							
							
							
							insertNewEdgeVtx(newVtx[side*2+dir],false, nEdge);
							
							


						}

						//nun die abschlussarbeiten durchfuehren - allerdings nur, wenn unSeledge NICHT auf border liegt
						
						//if(faceIDs.length() > 1)
						if(faces.length() > 1)
						{
							//jetzt passendes Face finden und endFaces erstellen
							int validFace = -1;
							for(unsigned int u = 0; u < faces.length(); u++)
							{
								for(unsigned int v = 0; v < faceIDs.length();v++)
								{
									if(faces[u] == faceIDs[v])
									{
										validFace = faces[1-u];
									//	break;
									}
								}
							}
							
							
							nSelEdge* realNSelEdge = static_cast<nSelEdge*>(nEdge);
							
							
							
							INVIS(cout<<"ValidFace = "<<validFace<<endl;);

							refDirs[dir] = realNSelEdge->getRefDirection(vtxIDs[side],  validFace);
							refFaces[dir] = validFace;

						}

					} //UNSEL EDGES ENDE
					
					
				
					
			}//FOR(dir) Loop Ende in condition
			



			int myCe = connectEnds;	//damit ich den Wert von CE uendern kann, ohne dass connectEnds fuer die gesamte edge ausgeschaltet wird
			//nur diese Variable wird verwendet
			if(refFaces[0] != -1 && (refFaces[0] == refFaces[1]) )
			{//die RefFaces sind gleich, was bedeutet, dass man nichts machen kann
				myCe = 0;
			}
			
			//hier endFaces bearbeiten wenn triangulate off
			

			if( !(emergencyStop[0] | emergencyStop[1]) )
			{
				if(isSC &&!triangulateEnds && myCe && refFaces[0] != -1 && refFaces[1] != -1 )
				{//jetzt alles Vorbeiten - MERKE:er landet hier nur, wenn auch wirklich 2 Faces vorhanden sind - ansonsten wuerde er hier nicht hinkommen
					//zuerst die 2 Ptr zu den passenden Faces holen , welche von nSelEdges verwaltet werden
					edgeFaceData* twoFacePtrs[2];
					edgeFaceData* tmpData[2];

					nachbarn[side][0]->getFaceData(tmpData[0],tmpData[1]);
					
					twoFacePtrs[0] = ( tmpData[0] == faceData[0]) ? tmpData[1] : tmpData[0];

					nachbarn[side][1]->getFaceData(tmpData[0], tmpData[1]);
					twoFacePtrs[1] = ( tmpData[0] == faceData[1]) ? tmpData[1] : tmpData[0];
					
					int t[2] = {newVtx[side*2],newVtx[side*2+1]};



					int ce[2] = {-2,-2};

					edge::endFacePtrs.push_back(new endFaceData(vtxIDs[side],t, twoFacePtrs, ce, refDirs, id));

				}
				else
				{//ES GIBT HIER KEINE BESONDERHEITEN FueR CHAMFER; UM DIE MAN SICH SPueTER NICHT kueMMERN WueRDE
					//fuer chamfer kann es vorkommen, das newID == vtxID ist - derartige Fuelle werden ausgeschlossen spueter von insertVtx und von changeEndFace

					//also x* die StandardMethode aufrufen;
					int newIDs[2] = {-1,-1};


					int ce[2] = {-1,-1}; //-> == connectEnds
					edgeFaceData* twoFacePtrs[2] = {0,0};
					

					for(unsigned int g =  0; g < faceIDs.length();g++)
					{
						if(emergencyStop[g] || refFaces[g] == -1)
						{						
							continue;
						}
							//als FaceData muss das gegnuebrliegende Face der unSelEdge gegeben werden - diese hat auf jeden Fall 2 faces
							edgeFaceData* nFaceData[2];
							nachbarn[side][g]->getFaceData(nFaceData[0],nFaceData[1]);


						//	creator->changeEndFace(vtxIDs[side],newVtx[side*2+g],refIndices[g],refFaces[g],bool(connectEnds),faceIDs[g], ( nFaceData[0] == faceData[g]) ? nFaceData[1] : nFaceData[0]);	//RefFaces braucht man hier eigentlich nicht, die ID der faceData == refFAce ist

							newIDs[g] = newVtx[side*2+g];

	
							ce[g] = myCe;

							twoFacePtrs[g] = ( nFaceData[0] == faceData[g]) ? nFaceData[1] : nFaceData[0];
					}

					//jetzt das neue endFaceObjekt erzeugen
					edge::endFacePtrs.push_back(new endFaceData(vtxIDs[side], newIDs, twoFacePtrs, ce, refDirs, id));
				}
			}	
		}
		
	}//FOR(side) Loop ende

	//beim erstellen der faces achten auf:  onBorder und "nurEinNachbar"
	//alles pro Seite, pro Face

	
	//directions holen
//	MIntArray directions(2); directions[0] = 1;directions[1] = 0; //nurn test, eventuall funzt es nicht immer!
	//getDirections(nachbarn[0][0],nachbarn[0][1],directions);

	
	MPRINT("--------------------------------------------------------")
	INVIS(cout<<"ERSTELLE FACES EDGE<<<<  "<<id<<"  >>>>"<<endl;)
	MPRINT("--------------------------------------------------------")


	int count = faceIDs.length();	//soll verhindern, das selEdges on Border zicken machen
	MIntArray newFaceIDs(4);
	
	

	MIntArray checkedIDs;	//tempVariable, weil unter Linux keine temporren variablen (z.B Rckgabewerte einer Funktion) untersttzt werden, wenn diese als referenz bergeben werden sollen

	
	if(isSC)
	{//SC FaceMode
		for(side = 0; side < 2; side++)
		{
			
			
			//mittleres face erstellen
			if(count-- != 0)
			{
		
				
				//im Fall, dass an dieser Seite eine unSelEdge war, muss statt edgeVtx die VtxID verwendet werden
				if(edgeVtx[0] == -1)
					newFaceIDs[0] = vtxIDs[0];
				else
					newFaceIDs[0] = edgeVtx[0];	
				
				if(edgeVtx[1] == -1)
					newFaceIDs[1] = vtxIDs[1];
				else
					newFaceIDs[1] = edgeVtx[1];	
				
				
				newFaceIDs[2] = newVtx[2+side];	
				newFaceIDs[3] = newVtx[side];
				
				
				
				if(isFlipped[side])	//wenn das face geflipped ist, dann muss das array umgekehrt werden
				{
					//	cout<<"Array inverted"<<endl;
					creator->helper.invertArray(newFaceIDs);
				}
				
				INVIS(printArray(newFaceIDs," = newFaceIDs mittelFace");)
					INVIS(printArray(faceData[side]->normalIndices," = NORMALINDICES ORIGFACE");)
					

					checkedIDs = checkNewFaceIntegrity(newFaceIDs);
					creator->createPoly(checkedIDs, faceData[side]);

					
				
				
			}
			
			

			//normalSlide/slide fuer die edgeVtx,vtx
			if(edgeVtx[side] != vtxIDs[side] && edgeVtx[side] != -1)
			{
				creator->addNormalSlide(vtxIDs[side]);
				creator->addSlideAndNormal(vtxIDs[side],edgeVtx[side],faceIDs);
			}
			else
			{
				creator->addNormalSlide(vtxIDs[side]);
			}
			
			for(dir = 0; dir < faceIDs.length(); dir++)
			{
				
				//kleine Faces erstellen, wenn mehrere nachbarn oder keine onBorder
				if(onBorder[side*2+dir] + nurEinNachbar[side] == 0)
				{
					if( !(nachbarn[side][dir]->isClean()) )
					{
						creator->addSlide(vtxIDs[side],newVtx[side*2+dir]);
						
						INVIS(cout<<"SmallFace fuer seite "<<vtxIDs[side]<<" und richtung "<<faceIDs[dir]<<endl;)
							//kleine Faces erstellen
						newFaceIDs[0] = edgeVtx[side];
						newFaceIDs[1] = vtxIDs[side];
						newFaceIDs[2] = nachbarn[side][dir]->getNewGeradenVtx(this);
						newFaceIDs[3] = newVtx[side*2+dir];
						
						
						
						
						//man muss bedenken, dass dieses Face nicht erstellt werden kann, wenn die UVs der Faces nicht geteilt sind  
						//Man muss also triangulieren, wenn die UV des newVtx in mindestens einem UVSet nicht mit der UV aus einem anderen UVSet uebereinstimmt
						if(! creator->helper.isOnArray(faceIDs[dir], nachbarn[side][dir]->getFaces()) )
						{//dieses Face wird nicht mit dem anderen Face geteilt - jetzt genauer pruefen, ob die UVs der newVtx irgendwo ungleich sind
							int seite, richtung;
							nachbarn[side][dir]->getValidRichtung(this, seite, richtung, uID[side*2+dir]);
							
							//jetzt das passende nachbarFace holen
							edgeFaceData* nFaces[2];
							nachbarn[side][dir]->getFaceData(nFaces[0], nFaces[1]);
							
							
							//jetzt die UVs holen und vergleichen ->Beide Faces muessen in jedem Set den edgeVtx haben, damit das klargeht
							MIntArray newUVs[2] = {	faceData[dir]->getEdgeVtxUVs(newFaceIDs[2]), 
								nFaces[richtung]->getEdgeVtxUVs(newFaceIDs[2])	};	//pro face
							
							for(UINT x = 0; x < creator->numUVSets; x++)
							{
								if(newUVs[0][x] != newUVs[1][x])
								{//okay, also muss trianguliert werden -> 2 neue faces erstellen
									MIntArray triangle[2];
									triangle[0].setLength(3);
									triangle[1].setLength(3);
									
									triangle[0][0] = newFaceIDs[1];
									triangle[0][1] = newFaceIDs[2];
									triangle[0][2] = newFaceIDs[3];
									// == nachbarFace
									
									
									triangle[1][0] = newFaceIDs[3];
									triangle[1][1] = newFaceIDs[0];
									triangle[1][2] = newFaceIDs[1];
									
									
									if(side)	//die kleinen Faces sind auch noich abhuengig von der Seite - diese ifKonstruktion bezieht sie mit ein
									{
										if(isFlipped[dir])
										{
											creator->helper.invertArray(triangle[0]);
											creator->helper.invertArray(triangle[1]);
										}
									}
									else
									{
										if(!isFlipped[dir])
										{
											creator->helper.invertArray(triangle[0]);
											creator->helper.invertArray(triangle[1]);
										}
									}
									
									INVIS(printArray(newFaceIDs," = smallNewFaceIDs"););

									checkedIDs = checkNewFaceIntegrity(triangle[1]);				
									creator->createPoly(checkedIDs, faceData[dir]);
										
									checkedIDs = checkNewFaceIntegrity(triangle[0]);
									creator->createPoly(checkedIDs, nFaces[richtung]);


									
									goto endIt;
								}
							}
							
						}
						
						
						if(side)	//die kleinen Faces sind auch noich abhuengig von der Seite - diese ifKonstruktion bezieht sie mit ein
						{
							if(isFlipped[dir])
							{
								MPRINT("habe array umgekehrt zum schluss")
									creator->helper.invertArray(newFaceIDs);
							}
						}
						else
						{
							if(!isFlipped[dir])
							{
								MPRINT("habe array umgekehrt zum schluss")
									creator->helper.invertArray(newFaceIDs);
							}
						}
						
						INVIS(printArray(newFaceIDs," = smallNewFaceIDs"););
						

						checkedIDs = checkNewFaceIntegrity(newFaceIDs);
						creator->createPoly(checkedIDs, faceData[dir] );

						
endIt:;
						
					//	newFaceIDs.clear();
					}//if( !(nachbarn[side][dir]->isClean()) ) ENDE
					
				}//if(onBorder[side*2+dir] + nurEinNachbar[side] == 0) ENDE
				else if( !(nachbarn[side][dir]->isClean()) )
				{
					//wenn nachbar aber unselected, dann auf jeden fall hinzufuegen
				
					creator->addSlide(vtxIDs[side],newVtx[side*2+dir]);
				}
			
				//jeder neue Vtx bekommt slide, alle innenVtx bekommen normal
			
			
			
			//	newFaceIDs.clear();
			}
		
		
		}
	}
	else
	{//CHAMFER FaceMode
		
		//zuerst mal dafuer sorgen, das alle newVtx korrekt sind:
		//bei borderEdges kann es dazu kommen, dass mindestens einer der newVtx nicht gesetzt ist. Diesen Fall hier behandeln
		if(faceIDs.length() == 1)
		{//hiervor wird side/dir immer entsprechend gesetzt, damit die FixNewVtxUVProzedur af den Vtx der anderen Seite Zugreifen kann
			side = 0;	//side muss gesetzt werden , weil der nachbar in CFix... () gecheckt wird
			newVtx[0] = (newVtx[0] == -1) ? C_fixNewVtxUV(vtxIDs[0],newVtx[1], faceData[0]) : newVtx[0];	
			newVtx[1] = (newVtx[1] == -1) ? C_fixNewVtxUV(vtxIDs[0],newVtx[0], faceData[0]) : newVtx[1];
			
			side = 1;
			newVtx[2] = (newVtx[2] == -1) ? C_fixNewVtxUV(vtxIDs[1],newVtx[3], faceData[0]) : newVtx[2];
			newVtx[3] = (newVtx[3] == -1) ? C_fixNewVtxUV(vtxIDs[1],newVtx[2], faceData[0]) : newVtx[3];
		}


		//nun die aktuellen positionen holen und speichern
		MPoint newPos[4];

		//man kuennte die sache ein wenig beschleunigen, wenn man newPos als elementVariable anlegt und beim setNewVtx(...) dafuer sorgt, 
		//dass auch i positionen aktualisiert werden
		newPos[0] = (newVtx[0] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx[0]) : creator->getVtxPoint(newVtx[0]);
		newPos[1] = (newVtx[1] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx[1]) : creator->getVtxPoint(newVtx[1]);
		newPos[2] = (newVtx[2] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx[2]) : creator->getVtxPoint(newVtx[2]);
		newPos[3] = (newVtx[3] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtx[3]) : creator->getVtxPoint(newVtx[3]);


		//jetzt alle newVtx auf ihre korrekten Positionen setzen: NewVtx, die eigentlich schon vorhanden sind, muessen erst noch an ihre neue Position gesetzt werden
		//dies ist bei wirklich neu erstellten newVtx nicht nuetig, allerdings unterscheide ich hier nicht, da dies sicherlich mehr kosten wuerde als
		//einfach alle newVtx zu setzen
		

		//jetzt muss zuersteinmal ueberprueft werden, ob die UVs an den vtxIDs der edge fuer beide Faces uebereinstimmen, wenn es denn 2 gibt
		if(faceIDs.length() == 2)
		{
			bool areConnected = true;
			UINT i;
			//nur wenn eines der beiden Faces ueberhaupt UVs hat, anfangen
			MIntArray vtxUVs[2][2];
			if( faceData[0]->hasUVs() || faceData[1]->hasUVs() )
			{

				areConnected =		!(	doUVsDiffer(vtxIDs[0], faceData[0], faceData[1], &vtxUVs[0][0], &vtxUVs[0][1])
								|	/*das andere CheckUVs muss auf jeden Fll ausgefhrt werden */
										doUVsDiffer(vtxIDs[1], faceData[0], faceData[1], &vtxUVs[1][0], &vtxUVs[1][1]) );
			}					




			if(areConnected)
			{//sie sind verbunden, also kann man problemlos ein ganzes Face erstellen
				goto CStandardFace;
			}
			else
			{//die UVs sind getrennt. Das bedeutet, dass man neue edgeVtx erstellen muss, welche mittig zwischen den newVtx einer Seite liegen.
				//dann wird je richtung ein einzelnes Face erstellt. Die Spueter folgende RecreateSmallFaces methode kann damit umgehen

				

				//NEUEN VTX ERZEUGEN
				
				
				//MIntArray	origUVs;	//IDs der originalUVs von vtxIDs ->die OrigUVs sind noch immer in vtxUVs[2][2] gepeichert - wiederverwertung
				MIntArray newUVs[2];	//huelt die neu erzeugten UVs fuer den edgeVtx (pro Face)- es sind hier die originalUVs von der entsprechenden vtxID
				MFloatArray newUVPos;	//Positionen er IDs

				MPoint start,end, edgePoint;
				MVector newVec, edgeVec;
				MFloatArray UVPos[2];

				for(i = 0; i < 2; i++)
				{
					if(edgeVtx[i] > -1)
					{//der edgeVtx wurde bereits erzeugt, als er beantragt wurde von einer anderen Edge -> fuer diese Seite abbrechen
						continue;
					}//ansonsten 
					
					//checken, ob der Nachbar vielleicht edgeVtx fuer einen hat, aber nur wenn er schon evaluiert hat (da er andernfalls sowieso diesen nachbarn hier fragen wird)
					if( nachbarn[i][0]->isClean() & nurEinNachbar[i] )
					{
						edgeVtx[i] = nachbarn[i][0]->C_checkForEdgeVtx(this, uID[i*2], true, MPoint(), MPoint()/*points sind egal*/);
					}
					
					//wenn der edgeVtx immernoch nagativ ist, dann hat der nachbar nix gehabt. Also muss man doch selber ran
					if(edgeVtx[i] == -1)
					{

						
						edgeVtx[i] = creator->newVtx( newPos[i*2] + 0.5*(newPos[i*2+1] - newPos[i*2]));
	

						//jetzt muss auch noch der Slide hinzugefuegt werden - quasi nicht zur Seite sondern nach oben (richtung vtxIDs) und nach unten
						//aber nur, wenn der vtx wirklich gerade ers erzeugt wurde
						edge::creator->addSlideAndNormal(vtxIDs[i], edgeVtx[i], faceIDs);	
					}
									
					
					

					//jetzt muessen die neuen edgeVtx noch passend in die Faces eingefuegt werden - einfach an die Position der VtxID[side] packen
					//in beide Faces - es spielt hier keine Rolle, ob das nun korrekt ist, da daraus ja direkt kein Face erzeugt wird

					//man muesste eigentich doch darauf achten, in welcher richtung man dne Vtx einfuegt, damit die normalen ordentlich aktualisiert werden kuennen
					faceData[0]->insertEdgeVtx(vtxIDs[i], edgeVtx[i], (isFlipped[0]) ? ( (i == 0) ? false : true) : ((i == 0) ? true : false) ) ;
					faceData[1]->insertEdgeVtx(vtxIDs[i], edgeVtx[i], (isFlipped[1]) ? ( (i == 0) ? false : true) : ((i == 0) ? true : false));
					

					start = creator->getVtxPoint(vtxIDs[i]), end = creator->getVtxPoint(vtxIDs[1-i]);
					edgePoint = creator->getVtxPoint(edgeVtx[i]);
					MVector newVec = edgePoint - start;
					MVector edgeVec = end - start;
					
					double r = ( cos(edgeVec.angle(newVec)) * newVec.length() ) / edgeVec.length();


					//neue UVs erzeugen 
					bool UVsDiffer = creator->helper.UVSetDiffers(vtxUVs[i][0], vtxUVs[i][1]);

					for(UINT a = 0; a < 2; a++)
					{
						//Diese muessen fuer den Fall dass:
						//	a)Nur Ein nachbar existiert und beide edges kein Face teilen einfach nur am ort der vtxIDs erstellt werden
						// b) ansonsten muessen neue UVs mit slide gemacht werden, mit neuen UVPositionen die zwischen denen 

						//immer neue UV erzeugen. Man muss dann nur testen, ob beide faces die neu ertellten UVs teilen kuennen	
						
//!!!!!						//pro UVSet vorgehen! Das heisst, letztenlich muesste man tatsuechlich ein sharedArray erstellen - Das muss mich aber erst kuemmern, 
							//wenn ich auch wirklich mehrere UVSets schreiben kann ins mesh

							//wenn es sich um einen neuen newVtx handelt und die UVs sich unterscheiden, dann kann man getrost die UVs des origVtx dem edgVtx zuordnen
					//		if(newVtx[i*2+a] > creator->getInitialVtxCount() && UVsDiffer)
					//		{
					//			newUVs[a] = vtxUVs[i][a];
					//		}
							/*else */if(a == 1 && !UVsDiffer & C_IChangedUV[i])
							{
								newUVs[a] = newUVs[1-a];	//in diesem Fall die zuletzt zugewiesenen UVs nehmen
							}	
							else
							{
								//erstmal standrardVariante
								/*
								creator->getUVPositions(vtxUVs[i][a], newUVPos);
								newUVs[a] = edge::creator->createUVs(vtxUVs[i][a], newUVPos);
								*/
								//die Luesung ist, dass man den nuechstgelegenen Punkt des edgeVtx auf der gegenwuertigen edge finden muss.
								//Die Luenge dieses Vektors steht in dem Verhueltnis r zur gesamtluenge der edge.
								//dieses Verhueltnis muss man dann auf die UVs uebertragen und erhuelt so den UVPunkt,
								//Nachteil: Diser XYZ closest point wird immer nur auf der edge liegen, obwohl er manchmal auch auueerhalb liegen muesste.
								

								//jetzt die UVsPositionen holen von den bereits vorhandenen Vertizen
								
								creator->getUVPositions(vtxUVs[i][a], UVPos[i]);
								creator->getUVPositions(vtxUVs[1-i][a], UVPos[1-i]);

								//jetzt die neuen Positionen errechnen anhand von r
								newUVPos.setLength(2*creator->numUVSets);
								for(UINT n = 0; n < creator->numUVSets; n++)
								{
									newUVPos[n*2] = static_cast<float>(UVPos[i][n*2] + r * (UVPos[1-i][n*2] - UVPos[i][n*2]));
									newUVPos[n*2+1] = static_cast<float>(UVPos[i][n*2+1] + r * (UVPos[1-i][n*2+1] - UVPos[i][n*2+1]));
								}

								//UVs erzeugen
								newUVs[a] = edge::creator->createUVs(vtxUVs[i][a], newUVPos);
								
								//slide
								creator->addUVSlide(newUVPos, UVPos[i], newUVs[a]);
								
							
							}
						
						//newUVs setzen: 
						faceData[a]->assignUV(edgeVtx[i], newUVs[a]);


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						//auueerdem muss noch darauf geachtet werden, dass eventuelle unsel Edges ja jetzt mit dem edgeVtx verbinden muessen.
						//Dies ist allerdings momentan kaum machbar, da keine Ptr zu den entsprechenden edges vorhanden sind - es kuennte allerhuechstens von den endStuecken 
						//gemacht werden 
						//->Hmm, hat sich das nach den jngsten nderungen erledigt? Mal sehen ...
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

					}
					
				}
				
				//man darf hier natuerlich nicht davon ausgehen, dass fuer diese Edge auch tatsuechlich Endstuecken existieren
				if(edge::endFacePtrs.size() > 0)
				{			
					std::list<endFaceData*>::iterator endIter; 
					edgeFaceData* eTmp[2];
					MIntArray origUVs;
					bool changed = false;

					//man muss erst warten, bis die beiden edgeVtx geholt sind 
					
					for(i = 0; i < 2; i++)
					{
						//man muss bedenken, dass
						//die endFaces mit falschen newVtx informationen gefuettert wurden, welches jetzt nicht mehr zutreffend sind
						//deshalb muss die betroffene endFaceData gefunden und aktualisiert werden
						
						//suchen nach den VtxIDs -> das Wissen, dass die betreffenden Faces zuletzt hinzugefuegt wurden hilft, das ganz fix zu machen
						//mit der Suche beginnen muss man also von hinten
						

						//hat diese Seite ueberhaupt ne borderEdge?
						if( nachbarn[i][0]->isSelected() || nachbarn[i][1]->isSelected())	//es gibt hier auf jeden Fall 2 faces, so dass man nicht noch auf 0Ptr checkeb muss
							//diese Seite hat mindestens eine Selected edge, also kann es keine endseite mehr sein und man muss nicht erst suchen in endFaceData
							continue;
							
						changed = false;
						endIter = edge::endFacePtrs.end();
						//jetzt von hinten angefangen die Liste durchsuchen nach der passenden FaceData
						for(UINT x = static_cast<unsigned>(edge::endFacePtrs.size()); x > 0; x--)
						{

							endIter--;
								
							
							//jetzt noch den edgeVtx fachgerecht in die FaceData einfuegen - faceDir im endFaceobject als info nehmen
							eTmp[0] = (*endIter)->faceData[0];
							eTmp[1] = (*endIter)->faceData[1];


							//wenn das branding nicht mit der eigenen ID uebereinstimmt, ist es nicht das eigene Face
							//auueerdem darf das eigene Face nicht mehrmals veruendert werden, also nur unbehandelte faces bearbeiten
							if((*endIter)->branding != id || (*endIter)->origVtx != vtxIDs[i])
								continue;
							
							//wenn das endFace bei beiden Faces gleich ist, dann muss lediglich der edgeVtx ins neue Face eingetragen werden und zwar nur einmal
							UINT max = 2;
							//bool onlyOneFace = false;
							if( eTmp[1] != 0 && (eTmp[1]->id == eTmp[0]->id) )
							{
								//a = ((*endIter)->newVtx[0] > creator->getInitialVtxCount()) ? 1 : 0 ;	//die Richtung whlen, in der der alte Vtx ist
								//max = a+1;	//damit die Schleife auch wirklich nur einmal durchlaufen wird
								max = 1;
							}
							
							//	ansonsten muss man checken, ob die beiden Faces zusammenhngen. Tun sie es nicht bedeutet das, dass auch die endfaces
							//	aufgesplittet werden. In diesem Fall liegen also weitere Faces zwischen den endFaces, die auch auf den edgeVtx angepasst weden mssen
							
							//	Am einfachsten wre es, wenn man einfach spter jedes Vorkommen des origVtx mit dem edgeVtx vertauschen wrde und umgekehrt
							//	->Liste erstellen, die mitgefhrt wird
							
							for(UINT a = 0; a < max; a++)
							{
								
								if(eTmp[a] == 0)	//das Face muss nicht zangsweise besetzt sein (z.B bei borderEdges) 
									continue;
								/*else
								{//also ist das Face korrekt - demnach mssen jetzt ertmal die faceVtx geholt werden
									creator->getFaceVtxIDs(eTmp[a]->id, faceVtxIDs[a]);
								}*/
								
								changed = true;	//damit man aus dem endFaceListLooo austeigen kann - das Face wurde ja bereits gefunden und geuendert

								(*endIter)->origVtx = edgeVtx[i];	//der Wert wird jetzt zwar doppelt veruendert - oben darf das noch nicht gemacht werden
								//-> es darf hier nur was veruendert werden, wenn es sich garantiert um das richtige Face handelt
								
								//die UVS zuerst bearbeiten (damit man die UVs der vtxIDs noch findet)
								
								if(max == 1)
								{//in diesem Fall muss die neue UV auch einen Slide erhalten. Dazu die UVs der neun Vtx holen der Faces und deren Positionen mitteln.
									//daraus entsteht dann ein neuer Slide
									if(eTmp[0]->hasUVs())
									{//das ganze natrlich nur, wenn auch wirklich UVs vorhanden
										
										MIntArray newUVsTmp[2] =	{eTmp[0]->getEdgeVtxUVs((*endIter)->newVtx[0]), 
																	 eTmp[0]->getEdgeVtxUVs((*endIter)->newVtx[1]) };
										
										//man muss hier mal wieder darauf achten, dass alte Vtx noch auf ihrer oriPos sind und das die neue Pos nur i SlideArray zu finden 
										MFloatArray newUVPos[2];

										if((*endIter)->newVtx[0] > creator->getInitialVtxCount() )
											creator->getUVPositions(newUVsTmp[0],newUVPos[0]);
										else
											edge::creator->getUVSlideStartPoint(newUVsTmp[0], newUVPos[0]);

										if((*endIter)->newVtx[1] > creator->getInitialVtxCount() )
											creator->getUVPositions(newUVsTmp[1],newUVPos[1]);
										else
											edge::creator->getUVSlideStartPoint(newUVsTmp[1], newUVPos[1]);

										//jetzt die Positionen mitteln
										MFloatArray edgeUVPos(creator->numUVSets * 2);

										for(UINT x = 0; x < creator->numUVSets; x++)
										{
											edgeUVPos[x] = (newUVPos[0][x] + newUVPos[1][x]) / 2;
											edgeUVPos[x+1] = (newUVPos[0][x+1] + newUVPos[1][x+1]) / 2;
										}

										//neue UVs erzeugen 
										newUVs[a] = creator->createUVs(newUVsTmp[0], edgeUVPos);
										
										
										//slide erzeugen (origUVs mssen extra geholt werden, da dieses Face nicht mit den faces der edge verbunden sein  muss UVmssig)
										MIntArray origUVs = eTmp[0]->getEdgeVtxUVs(vtxIDs[i]);
										MFloatArray origUVPos;
										creator->getUVPositions(origUVs, origUVPos);

										creator->addUVSlide(edgeUVPos, origUVPos, newUVs[a]);	//wobei a in diesem Fall eh nur 0 sein kann
									}


								}
								else
								{
									//OrigUVs holen und deren positionen verwenden, um ne UVs zu erzeugen
									//hier muesste man eigenlich auch noch ueberpruefen, ob die UVs dieses Faces irgendwo mit denen des faces dieser Edge uebereinstimmen
									//so dass nur fuer die ungeteilten UVs neue UVs erstellt werden
									origUVs = eTmp[a]->getEdgeVtxUVs(vtxIDs[i]);
									
									creator->getUVPositions(origUVs, newUVPos);
									
									newUVs[a] = creator->createUVs(origUVs, newUVPos);
								}
								
								
								
								UINT locID;
								
								if(creator->helper.getLocIndex(vtxIDs[i], eTmp[a]->faceVertices, locID))
									eTmp[a]->faceVertices[locID] = edgeVtx[i];
								INVIS(else	cout<<"FEHLER CHAMFER CREATE FACE: Konnt vtxID[i] nicht finden im faceVertices Array"<<endl<<endl;)
									
									
									//zuweisen der UVS
									eTmp[a]->assignUV(edgeVtx[i], newUVs[a]);
								
								if( (*endIter)->newVtx[a] > creator->getInitialVtxCount() )
								{
									//der Witz ist, dass das ganze nur funktionieren kann, wenn man das OriginalFace entsprechend veruendert
									edge::creator->changeSideFace(vtxIDs[i], edgeVtx[i], eTmp[a]->id, newUVs[a]);
								}
								else
								{								
									
									//der neue Vtx ist bereits vorhanden - die insertVtx methode des creators wird dann keinen neuen Vtx erstellen
									//newVTx also nicht veruendern
									//allerdings muss der edgeVtx korrekt ins originalFace eingefuegt werden mithilfe der insertVtx methode
									edge::creator->insertVtx(eTmp[a], vtxIDs[i], edgeVtx[i], -(*endIter)->directions[a]);
									
								}
								

								
							}//for(UINT a = 0; a < 2; a++) ENDE		
							
							/*
							//jetzt schauen, ob die faces zusammenhngen oder nicht
							if(	faceVtxIDs[0].length() && faceVtxIDs[1].length() )
							{
								if( creator->helper.matchArray(faceVtxIDs[0], faceVtxIDs[1]).length() == 1)
								{
									//sie hngen nur an einem Vtx zusammen - demnach mssen die Vtx ausgetauscht werden
									//origVtx (vtxID) gegen den edgeVtx

									
									//Leider ist es ein wenig schwierig an die bentigten Faces zu kommen - man msste das MayaMesh verwenden um schnell an
									//die bentigten Infos zu gelangen
								}
							}
							*/



							if(changed)
								break;
							
						}//for(UINT x = edge::endFacePtrs.size(); x > 0; x--) ENDE
						
					}//for(i = 0; i < 2; i++) ENDE

				}//if(edge::endFacePtrs.size() > 0) ENDE

				//das mit den veruenderten endFaces geht jetzt automatisch seinen Gang - jetzt kann man einfach die Faces erstellen - 2 an der Zahl
				newFaceIDs.setLength(4);

				for(i = 0; i < 2; i++)
				{
					newFaceIDs[2] = edgeVtx[0];
					newFaceIDs[3] = edgeVtx[1];

					newFaceIDs[0] = newVtx[2+i];
					newFaceIDs[1] = newVtx[i];


					
					if(isFlipped[i])
						creator->helper.invertArray(newFaceIDs);

					INVIS(creator->helper.printArray(newFaceIDs, " = newFaceIDs gesplittetes MITTELFACE (Chamfer)");)
					

					checkedIDs = checkNewFaceIntegrity(newFaceIDs);
					creator->createPoly(checkedIDs, faceData[i]);

				
				}
				
				//FERTIG
			}

		}
		else
		{

CStandardFace:
			//jetzt noch ein TmpFace erstellen, welches alle UVs enthuelt und alle normalen
			//MERKE: Diese Prozedur muss checken, ob noch edgeVtx mit einzubinden sind, welche von einem nachbarn erzeugt und gesetzt wurden!
		

			//diese prozedur muss auch mit boundaryFaces zurechtkommen
			if(faceIDs.length() == 1)
			{
				faceData[1] = faceData[0];	//weil alles weitere davon ausgeht, dass es 2 faces gibt 
			}

			//jetzt einfach das neue Face erzeugen wie gehabt		
			newFaceIDs[0] = newVtx[0];
			newFaceIDs[1] = newVtx[2];
			newFaceIDs[2] = newVtx[3];	
			newFaceIDs[3] = newVtx[1];
			
			
			edgeFaceData tmpFace(0);	//id ist egal
			tmpFace.faceVertices = newFaceIDs;

			tmpFace.UVIndices.setLength(creator->numUVSets * 4);
			

			tmpFace.UVAbsOffsets = (ULONG*)malloc(creator->numUVSets*4);
			tmpFace.UVRelOffsets = (bool*)malloc(creator->numUVSets);
				

			MIntArray tmpUVs;
			//jetzt das offsets Array aufbauen anhand der Infos ueber einen Vtx
			//UVs vom ersten Vtx holen (MERKE: Auch wenn es sich um einen verschobenen Vtx handelt, wird dieser aufs VtxChange gepackt und kann somit auch gefunden werden)
			tmpUVs = faceData[0]->getNewVtxUVs(newFaceIDs[0]);

			UINT count = 0;
			UINT i;
			for(i = 0; i < creator->numUVSets; i++)
			{
				tmpFace.UVRelOffsets[i] = (tmpUVs[i] > -1) ? true : false;
				
				tmpFace.UVAbsOffsets[i] = count;

				count += int(tmpFace.UVRelOffsets[i]) * 4;
			}

			//jetzt muss jetzt erstmal ueberprueft werden, ob ein nachbar(welcher noch nicht evaluiert hat, eventuell edgeVtx erzeugen sollte, die hier
			//noch zur Verwendung kommen sollen
			
			for(i = (2-faceIDs.length())*2; i < 2; i++)	//i = (2-faceIDs.length())*2 ->nur wenn 2 faces vorhanden (edge ist NICHT auf border), berhaupt auf edgeVtx checken ->borderedges habn nie edgeVtx
			{
				UINT myDir = 0;	//bestimmt, von welchem Face die UVs geholt werden. Default ist 0
				if( (edgeVtx[i] == -1) & nurEinNachbar[i] /*bedeutet auch, dass die edge auf der Seite gewuehlt ist ->NICHT ZWANGSWEISE*/ )	//wenn edgeVtx noch nicht gesetzt und es nur einen nachbarn gibt auf dieser Seite 
				{
					if(nachbarn[i][0] != 0 && nachbarn[i][0]->isSelected())
						edgeVtx[i] = nachbarn[i][0]->C_checkForEdgeVtx(this, uID[i*2], false, newPos[i*2],newPos[i*2+1]);
					else if(nachbarn[i][1] != 0)
						//der andere muss gewuehlt sein
						edgeVtx[i] = nachbarn[i][1]->C_checkForEdgeVtx(this, uID[i*2], false, newPos[i*2],newPos[i*2+1]);
					else break;	//wenn alle nacharn ungueltig waren (was ja eigentlich nicht passieren sollte, abbrechen

					
					//wenn der edgeVtx gesetzt wurde muss man die entsprechenden UVs noch setzen in tmpFace
					if(edgeVtx[i] > -1)
					{
UVDoDiffer:
						//eigenes Array verwenden, da TmpUVs so bleiben muss
						MIntArray origUVs;
						origUVs = faceData[myDir]->getEdgeVtxUVs(vtxIDs[i]);

						MFloatArray origUVPos;	//huelt die Positionen der origUVs
						edge::creator->getUVPositions(origUVs, origUVPos);

						//jetzt neue UVs erzeugen fuer origUVs
						MIntArray newUVs = edge::creator->createUVs(origUVs, origUVPos);

						//vor dem assignment muss natuerlich der edgeVtx natuerlich auch noch eingefuegt werden!!!!
						//tmpFace.insertEdgeVtx(vtxIDs[i], edgeVtx[i], (isFlipped[0]) ? ( (i == 0) ? false : true) : ((i == 0) ? true : false) );
						//einfach am anfang einfgen - pos ist egal
						tmpFace.insertEdgeVtx(tmpFace.faceVertices[0], edgeVtx[i], true);
						//nun noch die UVs zuweisen - edgeVtx ist bereits in faceVertices des faceObjektes eingetragen worden
						tmpFace.assignUV(edgeVtx[i], newUVs);
					}
				}
				else if(edgeVtx[i] > -1)
				{//in seltenen Borderedge konstellationen kommt es vor, dass ein edgeVtx in insertNewVtx definiert wird. Dieser muss dann jetzt noch entsprechende UVs
					//erhalten, damit das face ordentlich ertsellt werden kann 
					//quasi so wie oben, nur ohne neue UVs, wenn sich die Faces die UVs am origUV teilen
						if(doUVsDiffer(vtxIDs[i], faceData[0], faceData[1]))
						{//die UVs unterscheiden sich, also muss man die Seite herausfinden, auf der der gewhlte nachbar ist und myDir damit setzen
							//an dieser stelle sollte es nur einen selNachbarn  geben von zweien
							myDir = (nachbarn[i][0]->isSelected()) ? 0 : 1;
							
							goto UVDoDiffer;
						}
						else
						{	
							MIntArray origUVs;
							origUVs = faceData[0]->getEdgeVtxUVs(vtxIDs[i]);
							
							
							//einfach am anfang einfgen - pos ist egal
							tmpFace.insertEdgeVtx(tmpFace.faceVertices[0], edgeVtx[i], true);
							//nun noch die UVs zuweisen - edgeVtx ist bereits in faceVertices des faceObjektes eingetragen worden
							tmpFace.assignUV(edgeVtx[i], origUVs);		
						}
				}
			}

			//wenn edgeVtx gueltig ist, dann diesen in newFaceIDs einfuegen. Er wurde bereits in tmpFace eingefuegt samt UVs
			
			
			//jetzt diese ersten UVs zuweisen
			tmpFace.assignUV(newFaceIDs[0], tmpUVs);

			//und nun dasselbe mit den anderen UVs
			tmpUVs = faceData[0]->getNewVtxUVs(newFaceIDs[1]);
			tmpFace.assignUV(newFaceIDs[1], tmpUVs);
			
			tmpUVs = faceData[1]->getNewVtxUVs(newFaceIDs[2]);
			tmpFace.assignUV(newFaceIDs[2], tmpUVs);

			tmpUVs = faceData[1]->getNewVtxUVs(newFaceIDs[3]);
			tmpFace.assignUV(newFaceIDs[3], tmpUVs);

			//UVs fertig
			//jetzt die newUVIDs nochmal ganz neu aufbauen, um keine Probleme mit den edgeVtx zu bekommen, welche ja erst jetzt bekannt sind (wenn ueberhaupt)
			newFaceIDs.clear();
			
			if(edgeVtx[0] > -1)
				newFaceIDs.append(edgeVtx[0]);

			newFaceIDs.append(newVtx[0]);
			newFaceIDs.append(newVtx[2]);

			if(edgeVtx[1] > -1)
				newFaceIDs.append(edgeVtx[1]);

			newFaceIDs.append(newVtx[3]);
			newFaceIDs.append(newVtx[1]);


			//jetzt normalen bearbeiten -> es reicht, wenn man die normalen der VtxIDs (welche ja die originale fuer die newVtx sind), in faceData sucht und
			//ggf hinzufuegt
			//count als locID nehmen
			for(i = 0; i < 2; i++)
			{//damit auf jeden Fall entsprechende NormalInformaiton gefunden wird, so vorhanden
				if(creator->helper.getLocIndex(vtxIDs[i], faceData[0]->normalIndices, count))
				{
					//hier mssen natrlich die IDs der newVtx hin! Da eh nur nach mod2 eintrgen geschaut wird spter, trage ich jeweils 2 mal den entsprechenden newVtx ein
					tmpFace.normalIndices.append(newVtx[i*2]);
					tmpFace.normalIndices.append(newVtx[i*2]);

					if(edgeVtx[i]>0)
					{
						tmpFace.normalIndices.append(edgeVtx[i]);
						tmpFace.normalIndices.append(edgeVtx[i]);
					}

					tmpFace.normalIndices.append(newVtx[i*2+1]);
					tmpFace.normalIndices.append(newVtx[i*2+1]);
				}
			}
			
			
			if(!isFlipped[0])
			{
				//	cout<<"Array inverted"<<endl;
				creator->helper.invertArray(newFaceIDs);
			}
			
			INVIS(printArray(newFaceIDs," = newFaceIDs mittelFace CHAMFER");)
				
			//JETZT ENDLICH - NACH ALL DER VORBEREITUNG - DAS FACE ERSTELLEN


			checkedIDs = checkNewFaceIntegrity(newFaceIDs);
			creator->createPoly(checkedIDs,&tmpFace);
			
			


			//jetzt den Fall behandeln, dass edge on border liegt und der eventuell neu erstellte Vtx jetzt an die angrenzende BorderEdge weitergegeben werden muss
			//wenn die noch nicht evaluiert hat, oder dass man das von ihr erstellte Face ndert, so dass es den echten newVtx erhlt
			//wenn nur ein nachbar da ist, dann geht das automatisch klar :))
			

		}


	}

	MPRINT("ENDEENDEENDEENDEENDEENDEENDEENDEENDEENDEENDEENDEENDEENDE")
	MPRINT("Bin fertig")
	MPRINT("--------------------------------------------------------")
	MPRINT("--------------------------------------------------------")
	MPRINT("--------------------------------------------------------")


}




//--------------------------------------------------------------
void	selEdge::insertNewVtx(MPoint& inPoint, bool specialMode)
//--------------------------------------------------------------
{
	if(faceIDs.length() == 1)
	{
		
	

		if(inPoint.y == -1.123456789)
		{//wahrscheinlich muss dieser check hier nicht sein
			newVtx[side*2+dir] = int(inPoint.x);
			//die statische variable fuer die position muss ebenfalls aktualisiert werden

			INVIS(cout<<"Vtx "<<inPoint.x<<" war bereits vorhanden. Ihm wurde newVtx["<<side*2+dir<<"] zugewiesen"<<endl;)
		}
		else
		{
			

				newVtx[side*2+dir] = creator->newVtx(inPoint);
				

				creator->addSlideAndNormal(vtxIDs[side],newVtx[side*2+dir],inPoint);
		}

	//	if(!specialMode)
		//	newVtx[side*2+(1-dir)] = C_fixNewVtxUV(vtxIDs[side],newVtx[side*2+dir],  faceData[0]);	//damit newVtx immer gefuellt sind, dir ist hier eh immer , aber ich
																								//lasse das einfach mal ausgeschrieben, der Klarheit wegen
		//ist hier nicht ntig - wird spter dann gemacht
	
	}
	else
	{
		
		if(inPoint.y == -1.123456789)
		{
			INVIS(cout<<"Vtx "<<inPoint.x<<" war bereits vorhanden. Ihm wurde newVtx["<<side*2+dir<<"] zugewiesen"<<endl;)
			newVtx[side*2+dir] = int(inPoint.x);

			//die statische variable fuer ie position muss ebenfalls aktualisiert werden

			
		}
		else
		{
		
		
			
			if( origVtxDoneBA.isFlagSet(vtxIDs[side]) & !specialMode ) //nur wenn der Vtx noch nicht auf der Liste ist, wird seine neue Position ins positionsArray hinzugefuegt
			{
				
				newVtx[side*2+dir] = vtxIDs[side];
				
		

				INVIS(cout<<"HABE Vtx "<<vtxIDs[side]<<" newVtx zugewiesen"<<endl;)
					
				origVtxDoneBA.setBitFalse(vtxIDs[side]);				
				
			
			}
			else
			{
				
				newVtx[side*2+dir] = creator->newVtx(inPoint);

				//wenn special, dann auf jeden Fall noch den edgeVtx versehen mit der VtxID, damit das Face dann stimmt
				if(specialMode)
				{
					edgeVtx[side] = vtxIDs[side];
				}

			}

			//um slide kuemmern

				creator->addSlideAndNormal(vtxIDs[side],newVtx[side*2+dir],inPoint,true);


		}
		
		
	}
}





//-------------------------------------------
void	selEdge::createSmallChamferFaces()
//-------------------------------------------
{
	//holt rekursiv die newVtx der anliegenden Edges, bis man wieder bei der originalEdge (->requesterID) angekommen ist
	

	MIntArray newVtxIDs;	
	
	//nachbarn[i][0] ist immer besetzt, auch bei borderEdges
	INVIS(cout<<"Bin in Create small Faces fuer ID :: <<< "<<id<<" >>>"<<endl);
	MPoint tmpPos;	//fr Linux Kompatibilitt
	
	clean = false;


	int centerVtx;
	
	//begin je Seite
	for(int i = 0; i < 2; i++)
	{
		//wenn nur ein nachbar, dann existieren keine smallFaces auf dieser Seite, oder on Boder
		if( smallFaceClean[i] || origVtxDoneBA[vtxIDs[i]] || nachbarn[i][0] == nachbarn[i][1] || (nachbarn[i][0]->getFaces()).length() == 1 )
			continue;
		
		origVtxDoneBA.setBitTrue(vtxIDs[i]);
		

		smallFaceClean[i] = true;


		UINT a = 0;		//marker fr die richtung - spter wird nur noch a oder 1-a verwendet
		centerVtx = -1;
		newVtxIDs.clear();

//PREPROCESSING _______________________________


		newVtxIDs.append(newVtx[i*2+0]);

		

		//zuerst einen CenterVtx holen, wenn mglich
		
		//er ist also an border gestoen - jetzt noch in die andere richtung gehen, wenn mgich

		if((static_cast<selEdge*>(nachbarn[i][0]))->C_queryCenterVtx(id, this, newVtxIDs, centerVtx) && nachbarn[i][1] != 0)
		{	//in der anderen richtung suchen
			(static_cast<selEdge*>(nachbarn[i][1]))->C_queryCenterVtx(id, this, newVtxIDs, centerVtx);
		}
		

		//wenn der centerVtx definiert wurde, seine eigentliche position ermitteln und sie setzen
		if(centerVtx > -1)
		{
			MPoint newCPos;
			UINT l = newVtxIDs.length();
			for(UINT x = 0; x < l; x++)
			{
				newCPos += (newVtxIDs[x] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtxIDs[x]) : creator->getVtxPoint(newVtxIDs[x]);
			}
			
			//erzeugen des Slides - ohne slide ist allerdings alles einfacher
			newCPos = newCPos / l;
			creator->addSlideAndNormal(vtxIDs[i], centerVtx, newCPos, true);

			
		}


		

//PREPROCESSING DONE______________________________


		for( a = 0; a < faceIDs.length(); a++ )
		{
			
			if(nachbarn[i][a] == 0 || !nachbarn[i][a]->isSelected()  )
				continue;

			//CREATE SMALL FACES______________________________
			
			//tmpFace erzeugen
			
			edgeFaceData* tmpFace = new edgeFaceData(0);
			
			int thisVtx;
			if(edgeVtx[i] > -1)
			{//edgeVtx existiert, also ihn zuerst hinzufuegen
				thisVtx = edgeVtx[i];
				C_initFaceData(tmpFace, thisVtx, faceData[a], false);
			}
			else
			{
				//thisVtx = newVtx[i*2+a];
				thisVtx = newVtx[i*2+1-a];
				C_initFaceData(tmpFace, thisVtx, faceData[1-a], true);
			}
			
			
			
			
			
			//jetzt noch, wenn ntig, den newVtx hinzufuegen
			
			//der nachbar ist auf jeden Fall gueltig  wenn er hier hin kommt.
			//!!! Man muss bedenkten, dass auch bestimmte Konstellationen mglich sind, bei denen auf einer Seite eine nSelEdge ist, auf der anderen
			//ein selEdge - Man sollte dafuer sorgen, dass nie versucht wird, eine selEdge aus einem nSelPtr zu machen ->done
			
			//if(edgeVtx[i] > -1)
				C_dataAddVtx(tmpFace, faceData[a], newVtx[i*2+a], true);

			

			int result = (static_cast<selEdge*>(nachbarn[i][a]))->C_addVtx(id, this, tmpFace, centerVtx, edgeVtx[i] > -1);

			//jetzt anhand des vtxCounts des faces und des return values auswerten

			switch(result)
			{
			
			case 1:	//Die rekursion ist an einer borderedge gescheitert, also muss das face umgekehrt werden, und in die andere Richtung gesucht werden, wenn kein edgeVtx involviert ist
				{
					if(edgeVtx[i] == -1)
					{//das face umkehren und in die andere richtung suchen
						tmpFace->invert();
						
						//wenn der andere Nachbar korrekt ist, andere richtung achecken
						if(nachbarn[i][1-a] != 0 && !nachbarn[i][1-a]->isClean() & nachbarn[i][1-a]->isSelected())
							(static_cast<selEdge*>(nachbarn[i][1-a]))->C_addVtx(id, this, tmpFace, centerVtx, true);

						//face wird dann weiter unten erzeugt

					}
					//else einfach so ein neues Face erzeugen

					break;
				}
			/*case 2:
				{
					
										
					break;
				}*/

			}

			//wenn mueglich noch den centerVtx hinzufuegen
			if(centerVtx > -1)
			{//MERKE : AUF DIESE WEISE WERDEN DIE CENTERUVS EVENTEULL DOPPELT ERSTELLT - DIES BEI GELEGENHEIT AUSSCHLIEEN

				tmpFace->insertEdgeVtx(tmpFace->faceVertices[tmpFace->faceVertices.length() -1], centerVtx, true);
				
				MFloatArray UVPos;
				MIntArray vtxUVs = faceData[a]->getEdgeVtxUVs(vtxIDs[i]);
				edge::creator->getUVPositions(vtxUVs, UVPos);
				
				//merke - die UVs muessen auch gemittelt werden - bzw. man uss sie der prozedur mitgeben
				MIntArray tmpArray = edge::creator->createUVs(vtxUVs, UVPos);
				tmpFace->assignUV(centerVtx, tmpArray);

				

				//UVSlide ... ,muesst es auch noch geben
				//creator->addUVSlide(
			}

			//face erzeugen
			//if((edgeVtx[i] == -1) ? !isFlipped[a] : isFlipped[a])	//NOT is flipped, da er ja in die andere (1-a) richtung sucht, was umgelehrtes Verhalten zufolge hat
			if( (edgeVtx[i] == -1) ? (i == 1) ? !isFlipped[a] : isFlipped[a] : (i == 0) ? isFlipped[a] : !isFlipped[a] )
			//if( (i == 1) ? isFlipped[1-i] : isFlipped[i])
				tmpFace->invert();
			
			creator->createPoly(tmpFace);

			
			delete tmpFace;


			//wenn es keinen edgeVtx gab, dann auf keinen Fall noch in der anderen Seite suchen und einfach abbrechen
			if(edgeVtx[i] == -1)
				return;
				
		}
		
		

		
	}

	
	


}


//------------------------------------------------------------------------------------------------------------------------------
void		selEdge::C_dataAddVtx(edgeFaceData* fData,const edgeFaceData* nRefFace, int thisVtx, bool isNew)
//------------------------------------------------------------------------------------------------------------------------------
{
	//MERKE: DIESE METHODE DARF NUR VERWENDET WERDEN, WENN BEREITS MINDESTENS EIN VTX VORHANDEN IST
	//MERKE2: CenterVtx knenn mit dieser Methode nicht hinzugefuegt werden - dies muss extra gemacht werden
	//isNew flag wird bentigt, damit die normalen gefunden werden knnen
	
	
	UINT lid;
	UINT l = fData->faceVertices.length(); //l and index fuer di UVs
	//vtx draufpacken - faceDate methoden knnen hier nicht verwendet werden, da das Face nich voll ist
	
	fData->faceVertices.append(thisVtx);

	//jetzt die UVs anpassen
	//-> der letzte UV muss angehuengt werden - die anderen knnen eingefuegt werden
	//NEUE UVS SOFORT EINFUEGEN
	MIntArray newUVs;

		//jetzt noch die UVs zuweisen 
	if(isNew)
		newUVs = nRefFace->getNewVtxUVs(thisVtx) ;
	else
		newUVs = nRefFace->getEdgeVtxUVs(thisVtx) ;



	//Die letzte UV kann man einfach so anhaengen, allerdings nur wenn ein UVSet vorhanden ist - die UVIDs sind sparse, und sind nur vorhanen, 
	//wenn auch das entsprechende UVSet auch existiert
	if(fData->UVRelOffsets[creator->numUVSets - 1])
		fData->UVIndices.append(newUVs[creator->numUVSets - 1]);


	//und nun die restlichen UVs einfgen. Dies muss so gemacht werden, weil man .insert nur innerhalb des Arrays nehmen kann, ansonsten muss append verwendet werden
	UINT indexChange = 0;	//als offsetMarker
	for(UINT i = 0; i < creator->numUVSets - 1U;i++)
	{
		if(fData->UVRelOffsets[i])
		{
			fData->UVIndices.insert(newUVs[i], (fData->UVAbsOffsets[i] + indexChange++ + l + 1));
		}
	}
	

	
	
	//und zuletzt nach normalen schauen
	
	if(isNew)
	{
		if(creator->helper.getLocIndex(thisVtx, nRefFace->vtxChange, lid))
		{//der dazugehuerige vtx kann jetzt im normalindices array gesucht werden
			if(creator->helper.isOnArray(nRefFace->vtxChange[lid-1], nRefFace->normalIndices))
			{
				fData->normalIndices.append(thisVtx);
				fData->normalIndices.append(thisVtx);
			}
		}
	}
	else

	{
		if(creator->helper.isOnArray(thisVtx, nRefFace->normalIndices))
		{
			//vtxIds hinzufuegen - einfach 2 mal den selben Vtx, um die edge garantiert hart zu machen
			fData->normalIndices.append(thisVtx);
			fData->normalIndices.append(thisVtx);
		}
	}
	

}

//--------------------------------------------------------------------------------------------------------------
void		selEdge::C_initFaceData(edgeFaceData* thisFace, int thisVtx, edgeFaceData* refFace, bool isNew)
//--------------------------------------------------------------------------------------------------------------
{

	//im Untrschid zu der prozedur hierueber kommt diese methode damit klar, dass das face vllig leer ist
	
	//edgeVtx hinzu
	thisFace->faceVertices.append(thisVtx);
	
	
	//UVs ____________________________________________________
	
	//UVs hinzu bzw. erstmal aufbauen
	if(creator->numUVSets)
	{
		thisFace->UVRelOffsets = (bool*)malloc(creator->numUVSets);
		
		thisFace->UVAbsOffsets = (unsigned long*)malloc(creator->numUVSets*4);
	}
	
	
	
	//jetzt die UVIDs setzen fuer thisVtx
	if(isNew)
		thisFace->UVIndices = refFace->getNewVtxUVs(thisVtx);	//thisVtx ist in diesem Fall der newVtx
	else
		thisFace->UVIndices = refFace->getEdgeVtxUVs(thisVtx);	//thisVtx ist in diesem Fall der edgeVtx
	
	
	
	//das indices array ist jetzt erstma initialisiert - anhand dessen jetzt die OffsetsArrays aufbauen
	for(UINT i = 0; i < creator->numUVSets; i++)
	{
		thisFace->UVRelOffsets[i] = (thisFace->UVIndices[i] == -1) ? false : true;
		
		thisFace->UVAbsOffsets[i] = i;	//das geht, da momentan eh nur ein vtx vorhanden ist
	}
	
	

	if(isNew)
	{
		UINT lid;
		if(edge::creator->helper.getLocIndex(thisVtx,refFace->vtxChange, lid))
		{//der dazugehuerige vtx kann jetzt im normalindices array gesucht werden
			if(creator->helper.isOnArray(refFace->vtxChange[lid-1], refFace->normalIndices))
			{
				thisFace->normalIndices.append(thisVtx);
				thisFace->normalIndices.append(thisVtx);
			}
		}
	}
	else

	{
		if(creator->helper.isOnArray(thisVtx, refFace->normalIndices))
		{
			//vtxIds hinzufuegen - einfach 2 mal den selben Vtx, um die edge garantiert hart zu machen
			thisFace->normalIndices.append(thisVtx);
			thisFace->normalIndices.append(thisVtx);
		}
	}
	

}


//--------------------------------------------------------------------------------------------------------------
UINT		selEdge::C_addVtx(int requesterID,const selEdge* requester, edgeFaceData* fData, int& centerVtx, bool needsEdgeVtx)
//--------------------------------------------------------------------------------------------------------------
{
	//true wird zurueckgegeben, wenn eine Borderedge getroffen wurde - in diesem Fall muss dann noch in die andere richtung gesucht werden
	//vom requester, nachdem das face umgekehrt wurde

	//abbruch, wenn diese edge schon dran war
	//NEIN - Mit clean drfen diese Sekundaeredges nicht arbeiten - allerdings mssen sie sich clean setzen., damit dasselbe face nicht zweimal evaluiert wird
//	if(!clean)
	//	return 0;

	int s, r; 

	getValidRichtung(requester, s, r );

	if(smallFaceClean[s])
		return 0;
	
	smallFaceClean[s] = true;


	if(id == requesterID)
	{//von diesem Face ging alles aus, also allerhuechstens noch einen edgeVtx draufpacken, wenn mueglich
		//der newVtx dieser Seite ist bereits drauf - aber nur, wenn der requester NICHT die requesteredge ist

		//diese Stelle kann acuh zu problemen fuehren, weil fuer den Fall, das nur der reqzester einen edgevtx hat
		//dieser dann doppelt hinzugefuegt wuerde -> dies ist durch den erweiterten check ausgeschlossen

/*
		if(edgeVtx[s] > -1 && requester != this)
		{
			smallFaceClean[s] = true;	//dies bewirkt, dass der requester NICHT mehr versucht, dieses Face zu erstellen, wenn er sich der anderen richtung zuwendet

			C_dataAddVtx(fData, faceData[r], edgeVtx[s], false);
			
			//hier wird 2 zurueckgegeben, damit der requester weiue, dass er nicht noch  in die andere richtung suchen muss
			return 2;
		}
*/

		return 0;
	}



	
	//Okay, jetzt erstmal loslegen

	//wenn es eine border edge ist, gleich abbrechen
	if(faceIDs.length() == 1)
		return 1;

	
	bool UVsDiffer;

	//Wenn ein edgeVtx bentigt wird, aber wir keinen haben, dann blockieren wir und machen unser eigenes Face
	if(needsEdgeVtx & (edgeVtx[s] == -1 ))
	{
		//wir haben keinen edgeVtx, braeuchten aber einen - also einfach ein eigenes Face erstellen und das alte unverndert zurueckgeben
		
		edgeFaceData* tmpFace = new edgeFaceData(0);
		
		
		C_initFaceData(tmpFace, newVtx[s*2+r], faceData[r],true);

		C_dataAddVtx(tmpFace, faceData[1-r], newVtx[s*2+1-r], true);
		
		if(nachbarn[s][1-r]->isSelected())
			(static_cast<selEdge*>(nachbarn[s][1-r]))->C_addVtx(requesterID, this, tmpFace, centerVtx, needsEdgeVtx);

		//und jetzt noch der CenterVtx - sonst ist das Face ungltig. Es gibt hier auf jeden Fall einen, 
		//weil der needEdgeVtx Flag nur aktiviert wird, wenn auch centerVtx vorhanden ist 
		tmpFace->insertEdgeVtx(tmpFace->faceVertices[tmpFace->faceVertices.length() -1], centerVtx, true);
		
		MFloatArray UVPos;
		MIntArray vtxUVs = faceData[1-r]->getEdgeVtxUVs(vtxIDs[s]);
		edge::creator->getUVPositions(vtxUVs, UVPos);


		MIntArray tmpArray = edge::creator->createUVs(vtxUVs, UVPos);
		tmpFace->assignUV(centerVtx, tmpArray);

		if( (s == 1) ? isFlipped[r] : isFlipped[1-r])
			tmpFace->invert();


		creator->createPoly(tmpFace);

		delete tmpFace;

		return 2;
	}
	

	//wenn hier ein edgeVtx vorhanden ist, dann wird er draufgepackt und die methode wird beendet, nachdem man einen eigenen versuch gestartet hat
	//ein face zu erzeugen

	MIntArray tmpArray;	//wegen Linux

	//man muss hier auch checken, ob die UVs am gemeinsamen newVtx zusammenhngen - er ist der letzte, welcher zum inFace hinzugefgt wurde
	MIntArray lastVtxUVs = fData->getEdgeVtxUVs(fData->faceVertices[fData->faceVertices.length() -1]);
//	INVIS(creator->helper.printArray(lastVtxUVs, " = LASTVTX UVS!!!!!!!!!!!!!!!"));
//	INVIS(creator->helper.printArray(faceData[r]->getNewVtxUVs(newVtx[s*2+r]), " = NEWVTX OTHERFACE UVS!!!!!!!!!!!!!!!!!"));
	UVsDiffer = edge::creator->helper.UVSetDiffers(lastVtxUVs, faceData[r]->getNewVtxUVs(newVtx[s*2+r]));



	//jetzt berprfen, ob bereits ein centerVtx vorhanden ist - die eigentliche prozedur erstellt nur einen, wenn ein edgeVtx vorhanden ist
	if(UVsDiffer & (centerVtx == -1))
	{//es muss auf jeden Fall ein neuer CenterVtx erzeugt werden
		MIntArray newVtxIDs;

		//zuerst einen CenterVtx holen, wenn mglich
		
	
		//er ist also an border gestoueen - jetzt noch in die andere richtung gehen, wenn muegich
		if((static_cast<selEdge*>(nachbarn[s][0]))->C_queryCenterVtx(id, this, newVtxIDs, centerVtx) && nachbarn[s][1] != 0 )
		{	//in der anderen richtung suchen
			(static_cast<selEdge*>(nachbarn[s][1]))->C_queryCenterVtx(id, this, newVtxIDs, centerVtx);
		}
		
		//ein CenterVtx wurde jetzt definitiv nicht definiert - dies muss nun von dieser Edge gemacht werden
		
		
		MPoint newCPos;
		for(UINT x = 0; x < newVtxIDs.length(); x++)
		{
			newCPos += (newVtxIDs[x] <= creator->getInitialVtxCount()) ? creator->getSlideStartPoint(newVtxIDs[x]) : creator->getVtxPoint(newVtxIDs[x]);
		}
		
		//setzen
		centerVtx = creator->newVtx(newCPos);	
		
	}


	if(edgeVtx[s] > -1 || UVsDiffer)
	{
		
		//edgeVtx zu InfaceData hinzufuegen, wenn die UVs zusammenhuengen
		
		int thisVtx;
createOtherFace:


		if(UVsDiffer)
		{//in diesem Fall muss der erste Vtx der neueVtx sein auf der Seite des Requesters - keinen edgeVtx hinzufgen zum Face
			thisVtx = newVtx[s*2+r];
		}
		else
		{//also reicht der edgeVtx - man kann ihn dann auch zur inFaceData hinzufuegen
			thisVtx = edgeVtx[s];
			C_dataAddVtx(fData, faceData[r], edgeVtx[s], false);
		}
		

		//FERTIG MIT DEM HINZUFGEN

		//EIGENES
		//FACE ERSTELLEN ____________________________________________________ ->Dies manuell machen, da die CAddVtx methode mindesten einen Vtx braucht als referenz

		//Okay, jetzt erstmal ein eigenes Face erzeugen, den EdgeVtx und den newVtx hinzufuegen
		edgeFaceData* tmpFace = new edgeFaceData(0);
		


		C_initFaceData(tmpFace, thisVtx, faceData[1-r], UVsDiffer);



		//jetzt den edgeVtx drauftun, wenn nuetig
		if(UVsDiffer)
		{//dies bedeutet, dass zuerst der newVtx draufgetan wurde 
			//wenn sich die UVs unterscheiden, muss erstmal ein dreieck erstellt werden

			//edgeVtx - wenn vorhanden
			if(edgeVtx[s] > -1)
			{
				C_dataAddVtx(tmpFace, faceData[1-r], edgeVtx[s], false);
				
				//centerVtx
				tmpFace->insertEdgeVtx(edgeVtx[s], centerVtx, true);
				
				MFloatArray UVPos;
				MIntArray vtxUVs = faceData[1-r]->getEdgeVtxUVs(vtxIDs[s]);
				edge::creator->getUVPositions(vtxUVs, UVPos);
							

				tmpArray = edge::creator->createUVs(vtxUVs, UVPos);
				tmpFace->assignUV(centerVtx, tmpArray);

				
				
				//face umkehren, wenn ntig
				if(isFlipped[0])
					tmpFace->invert();
				
				//dreieck erstellen, nachde der centerVtx daraufgepackt wurde
				edge::creator->createPoly(tmpFace);
				
				//jetzt einfach den UVsDiffer Flag auschalten und die Sach nochmal ablaufen lassen von marker aus
				UVsDiffer = false;
				
				//lueschung face
				delete tmpFace;
				
				//jetzt wird das face auf der anderen Seite erstellt, wie geplant
				goto createOtherFace;
			}
			//else
			//wenn er nicht vorhanden ist, dann einfach normal weitermachen

		}



		//NEW VTX -------------------------------------
		//jetzt den newVtx drauftun

		C_dataAddVtx(tmpFace, faceData[1-r], newVtx[s*2+1-r], true);

		
		
		//Okay, jetzt dieses Face an den nachbarn schicken auf dass er etwas neues draufpackt
		int returnValue;

		if(nachbarn[s][1-r]->isSelected())
			returnValue = (static_cast<selEdge*>(nachbarn[s][1-r]))->C_addVtx(requesterID, this, tmpFace, centerVtx, edgeVtx[s] > -1);
		else
		{
			delete tmpFace;
			return 1;	//requester soll noch in die andere Richtung schauen
		}

		//wenn das face, welches zurueckkommt, unveruendert viele vertizen hat, dann bestimmt der returnvalue - wie man sich verhalten muss
		
	//	if(returnValue == 2)
	//	{//der nuechste nachbar war der requester - er kuemmert sich um dieses Face, also abbrechen
		//	delete tmpFace;
		//	return 0;
	//	}
		//ansonsten ists egal, und man erstellt dreieck
		

		//CenterVtx
		//gut, jetzt einfach noch den centerVtx drauftun aufs tmpFace - die UVs msen neu erzeugt werden
		tmpFace->insertEdgeVtx(tmpFace->faceVertices[tmpFace->faceVertices.length() -1], centerVtx, true);
		
		MFloatArray UVPos;
		MIntArray vtxUVs = faceData[1-r]->getEdgeVtxUVs(vtxIDs[s]);
		edge::creator->getUVPositions(vtxUVs, UVPos);


		MIntArray tmpArray = edge::creator->createUVs(vtxUVs, UVPos);
		tmpFace->assignUV(centerVtx, tmpArray);

		

		//Normalen sind bereits durch die anderen vertizen im Face definiert

		//FERTIG: Jetzt das neue Face erstellen und die Prozedur beenden
		//if(!isFlipped[0])
		if( (s == 0) ? isFlipped[1-r] : isFlipped[r])
			tmpFace->invert();

		creator->createPoly(tmpFace);

		//zuguterletzt den speicher freigeben
		delete tmpFace;


	//	if(UVsDiffer)
	//		return 1;	//so wird der requester gezwungen, das face umzukehren und in der anderen richtung zu schauen ->unntig
	
	
		return 2;
	
	}
	else
	{//aha, dieses face hat also keinen edgeVtx - einfach den newVtx raufpacken und den nachbarn zwingen, seinen Vtx draufzutun, wenn mueglich
		
		//wenn die nchste edge bereits wieder der requester ist, dann abbrechen - der Vtx, den man jetzt hinzufgen wollte ist bereits drauf
		if(nachbarn[s][1-r]->getID() == requesterID)
			return 0;


		C_dataAddVtx(fData, faceData[1-r], newVtx[s*2+1-r],  true);
		
		//FERTIG
		
		//man muss hier nicht checken auf 0Ptr, da das schon durch den faceIds.length() == 1 check abgedeckt ist
		if(nachbarn[s][1-r]->isSelected())
			return (static_cast<selEdge*>(nachbarn[s][1-r]))->C_addVtx(requesterID, this, fData, centerVtx, needsEdgeVtx);
		else
			return 1;

	}

	//FORBIDDEN PATH
	return 0;

}


//--------------------------------------------------------------------------------------------------------------
bool		selEdge::C_queryCenterVtx(int requesterID,edge* lastEdge, MIntArray& newVtxIDs, int& centerVtxID)
//--------------------------------------------------------------------------------------------------------------
{

	//clean = false; //Ob sich eine Edge aktiviert oder nicht huengt jetzt vom smallFaceClean wert ab

	if(id == requesterID)
	{//wir sind wieder am anfang angekommen - normalerweise kuennte man jetzt die eigentlich position ermitteln, allerdings
		//kuennte es auch sein, dass man hierher nie kommt da der loop durch borders gestoppt wurde. Aus dem grund macht das
		//die aufrufende prozedur

		return false;
	}

	
	int s, r;
	edge* opposite = findOppositeRichtung(lastEdge, s, r);

	

	//newVtx anhuengen
	newVtxIDs.append(newVtx[s*2+r]);

	//wenn diese Edge einen edgeVtx hat und cernterVtxID noch -1 ist, dann einen neuen Vtx erzeugen und in centerVtx schreiben
	//MAN MUSS HIER AUCH NOCH CHECKEN, OB DIE BEIDEN eventell die UVs an vtxID nicht teilen - dann muss auch ein centerVtx erzeugt werden
	//darum kuemmert sich die addVtx prozedur
	if( (edgeVtx[s] > -1) & (centerVtxID == -1) )
	{//position ist egal erstmal
		centerVtxID = edge::creator->newVtx(MPoint());
	}

	if(opposite == 0)
	{

		return true;

	}
	else
	{
		
		return (static_cast<selEdge*>(opposite))->C_queryCenterVtx(requesterID,this,newVtxIDs, centerVtxID);
	}

	//forbiddenPath
	return false;

}



