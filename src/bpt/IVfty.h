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


#ifndef IVFTY_INCLUDE
#define IVFTY_INCLUDE

#include "polyModifierFty.h"


#include "IVMeshCreator.h"
#include "meshCreator.h"
#include "BPT_Helpers.h"
#include "vertexFty.h"


// General Includes
//
#include <maya/MObject.h>
#include <maya/MFloatArray.h>
#include <maya/MString.h>

#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>

#include <maya/MItSelectionList.h>


#include <list>

using std::list;

namespace BPT
{

/** Diese Klasse kuemmert sich um das Einfuegen von vertizen ins mesh, lsst diese arbeit aber nur ueber einen VtxWorker machen
	
*/

class IVfty :	public polyModifierFty
{

	//FUNKTIONEN
public:
	
	IVfty(void);
	virtual ~IVfty(void);

//-------------------------------------
//HAUPT METHODEN
//-------------------------------------
//erledigen allgemeine Aufgaben

	MStatus		doIt();

	void		doSlide(MObject& slideMesh);


//-------------------------------------
//ZUGRIFFS METHODEN
//-------------------------------------
//erledigen allgemeine Aufgaben

//------
//SET
//------
	void		setMesh( MObject& mesh ){fMesh = mesh;};
	
	void		setCount(int inCount){count = inCount;};
	void		setSpin(int inSpin){spin = inSpin;};

	void		setSlide(double inSlide){slide = inSlide;};
	void		setNormal(double inNormal){normal = inNormal;};
	void		setSide(int inSide){side = (bool)inSide;};

	void		setEdgeIDs(const MIntArray& array){MIntArray tmp = array; edgeIDs = tmp;};	//!< dies musste irgendwie so umständlich gemacht werden, da man ansonsten drekt auf den NodeDaten gearbeitet hätte
	void		setVertIDs(const MIntArray& array){vertIDs = array;};

	void		setOptions(const MIntArray& o){options = o;};
	void		setRelative(int value){isSlideRelative = (bool)value;};
	void		setNormalRelative(int value){isNormalRelative = (bool)value;};

	void		setMeshPath(MDagPath& inMeshPath){meshPath = inMeshPath;};


//------
//GET
//------
	MObject					getMesh(){	MObject returnValue = newMeshData; 
										newMeshData = MObject::kNullObj; 
										return returnValue; };

	MIntArray&				getSlideIndices(){return *slideIndices;};
	int						getSlideLimited() const {return options[4];};

	MStringArray			getUVSetNames(){return UVSetNames;};

/*
	void					getUVPositions(	list<MFloatArray>*& inU,
											list<MFloatArray>*& inV);

	void					getUVIntArrays(	list<MIntArray>*& inUVCounts,
											list<MIntArray>*& inUVperVtx );
*/

//--------------------
//PRIVATE FUNKTIONEN
private:
//--------------------

	void	doUVSlide(MObject& mesh);	//wird von doSLide aufgerufen

	/** Analysiert die EdgeAuswahl und unterteilt sie in gültige slidePathes (dann inklusive start/endID)
		und ungültige Pathes (ohne slideID)
	*/

	void	trenneEdgePathes(	list<MIntArray>& validPathes, 
								list<MIntArray>& startEndIDs, 
								MIntArray&		 invalidPathes	);					

	//* im Grunde sorgt diese Prozedur nur für gezielte aufrufe an den creator, welcher die ganze Arbeit macht
	void	insertVertices(		IVMeshCreator&	 creator,
								list<MIntArray>& validPathes,
								list<MIntArray>& startEndIDs,
								MIntArray&		 invalidPathes	);					




//---------------
//HELFER
private:
//---------------

//	für trenneEdgePathes

	//* bestimmt, ob die edgeVerts Vertizen conform mit der Arrayrichtung der faceVerts sind
	bool	isDirConform(	const int index0, 
							const int index1, 
							const MIntArray& faceVerts	);		
	

	void	createAbsoluteVectors(	MVectorArray& slideDirections,
									MDoubleArray& slideScale			);		//!< für vtxSlideArrays

	void	createAbsoluteVectors(	MFloatVectorArray& slideDirections,
									MFloatArray& slideScale				);		//!< für UVArrays


	void	modifySelectionList();


	void	cleanUp();			//!< loescht ptr und setzt variablen zurück


//---------------
//VARIABLEN
private:
//---------------

	MIntArray	edgeIDs;		//!< hält die gewählten edgeIDs, mit denen gearbeitet wird
	MIntArray	vertIDs;		//!< gewählte Vertizen, werden dann von connect mit verarbeitet, zusammen mit den neu hinzugefügten vertizen

	MIntArray	options;		//!< hält optionen für aktion

	double		slide;
	double		normal;
	

	int			count;			//!< wie oft soll die Unterteilung geschehen ? 
	int			spin;			//!< wie sollen die connecteten Kanten orientiert sein ?
	bool		side;			//!< an welcher Seite soll sich der Slide orienteren ?
	
	bool		isSlideRelative;	//!< slide per edgeLength oder nicht?
	bool		isNormalRelative;	//!< normal relativ zu faceSize oder nicht?


	MObject		fMesh;			//!< hält ptr zu mesh, auf dem man arbeitet
	MObject		newMeshData;	//!< hält die Daten des neu erzeugten meshes
	

	
//--------------------------
private: //UVSLIDE ARRAYS
//--------------------------

	list<MFloatVectorArray>*		UVSlideStart;
	list<MFloatVectorArray>*		UVSlideEnd;
	list<MIntArray>*				UVSlideIndices;
	list<MFloatVectorArray>*		UVSlideDirections;
	list<MFloatArray>*				UVSlideScale;


//--------------------------
private:	//SLIDE ARRAYS
//--------------------------

	MDoubleArray*				normalScale;		//!< doubles, die die Normalen anhand der zugrundeliegenden faceGroesse skalieren
	MDoubleArray*				slideScale;			//!< skaliert die richtungsvektoren auf die kleinste gemeinsame länge herunter
	MVectorArray*				slideDirections;	//!< richtungsvector von start zu end
	MVectorArray*				slideNormals;		//!< normalen vektoren
	MPointArray*				slideStartPoints;	//!< startPoints
	MPointArray*				slideEndPoints;		//!< endpoints
	MIntArray*					slideIndices;		//!< indices, die geslided werden koennen 
	MIntArray*					maySlide;			//!< nur true oder false
	MDagPath					meshPath;

	double						maxStandardScale;	//!< skaliert normalScale auf die länge der längsten normale


//----------------------------
private:  //Arbeitsvariablen
//----------------------------

	//UVs
	MStringArray UVSetNames;

	//Helfer
	BPT_Helpers	helper;



};

}

#endif

