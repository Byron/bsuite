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

// softTransformationNode.cpp: implementation of the softTransformationNode class.
//
//////////////////////////////////////////////////////////////////////

#include "softTransformationNode.h"



//------------------------
//DEBUGMacros
//------------------------



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------------------------------
softTransformationNode::softTransformationNode()
	: maxScale(1.0)
	, oldMaskSize(0)
	, oldFType(-1)
	, oldVis(false)
	, oldFDistance(-1.0)
	, vtxColorsStored(false)
	, onStartup(true)
//--------------------------------------------------------------------------------------------------------------------------
{

}

//--------------------------------------------------------------------------------------------------------------------------
softTransformationNode::~softTransformationNode()
//--------------------------------------------------------------------------------------------------------------------------
{

}

//Variablen Initialisierungen

MObject	softTransformationNode::outPnts;		//plugArrayAttribut
MObject	softTransformationNode::inMesh;
MObject	softTransformationNode::outMesh;
MObject	softTransformationNode::inVertices;

MObject	softTransformationNode::startPoint;
MObject	softTransformationNode::endPoint;


MObject	softTransformationNode::alongNormalObj;
MObject softTransformationNode::scaleObj;


MObject	softTransformationNode::finalizeObj;
MObject	softTransformationNode::normalRelativeOBJ;


MObject	softTransformationNode::radius;
MObject	softTransformationNode::maskSizeOBJ;

MObject	softTransformationNode::fTypeOBJ;
MObject	softTransformationNode::inverseMatrix;

MObject	softTransformationNode::customFalloff;
MObject	softTransformationNode::falloffVis;

MObject	softTransformationNode::outWeights;

MTypeId		softTransformationNode::id(0x00108BC3);


//--------------------------------------------------------------------------------------------------------------------------
void*		softTransformationNode::creator()
//--------------------------------------------------------------------------------------------------------------------------
{
	return new softTransformationNode;
}


//--------------------------------------------------------------------------------------------------------------------------
MStatus		softTransformationNode::initialize()
//--------------------------------------------------------------------------------------------------------------------------
{
	MFnNumericAttribute	numFn;
	MFnEnumAttribute	enumFn;
	MFnTypedAttribute	typedFn;
	MFnUnitAttribute	unitFn;


	//########################################
	//OUT VECTORS
	//########################################
	
	outPnts = typedFn.create("outVectors","ov",MFnData::kVectorArray);
	typedFn.setStorable(false);
	typedFn.setWritable(false);
	

	//########################################
	//OUTWEIGHT
	//########################################

	outWeights = typedFn.create("outWeights","ow",MFnData::kDoubleArray);
	typedFn.setStorable(false);
	typedFn.setWritable(false);
	
	
	//########################################
	//MATRIX
	//########################################
	inverseMatrix = typedFn.create("inverseMatrix","im",MFnData::kMatrix );


	inMesh = typedFn.create("inMesh","in",MFnData::kMesh);
	typedFn.setStorable(true);

	//########################################
	//OUT MESH
	//########################################

	outMesh = typedFn.create("outMesh","out",MFnData::kMesh);
	typedFn.setStorable(true);
	typedFn.setWritable(false);


	//########################################
	//INVERTICES
	//########################################

	inVertices = typedFn.create("inVertices","inv",MFnData::kIntArray);
	typedFn.setWritable(true);
	typedFn.setStorable(true);


	//########################################
	//START POINT
	//########################################
	startPoint = numFn.create("startPoint","spt",MFnNumericData::k3Double);
	numFn.setStorable(true);

	//########################################
	//FALLOFF VIS
	//########################################
	falloffVis = numFn.create("falloffVisualization","fv",MFnNumericData::kBoolean);
	numFn.setStorable(true);

	//########################################
	//NORMAL RELATIVE
	//########################################

	normalRelativeOBJ = numFn.create("normalRelative","nr",MFnNumericData::kBoolean);
	numFn.setStorable(true);


	//########################################
	//FINALIZE
	//#######################################
	finalizeObj = numFn.create("finalize","flz",MFnNumericData::kBoolean);


	//########################################
	//ENDPOINT
	//########################################
	endPoint = numFn.create("endPoint","ept",MFnNumericData::k3Double);
	numFn.setStorable(true);

	//########################################
	//ALONG NORMAL
	//########################################
	alongNormalObj = unitFn.create("alongNormal","an",MFnUnitAttribute::kAngle);
	unitFn.setKeyable(true);
	unitFn.setStorable(true);


	//########################################
	//SCALE 
	//########################################
	scaleObj = unitFn.create("scale","s",MFnUnitAttribute::kAngle);
	unitFn.setKeyable(true);
	unitFn.setStorable(true);

	//########################################
	//FALLOFF TYPE
	//########################################
	fTypeOBJ = numFn.create("falloffType","ft",MFnNumericData::kLong);
	numFn.setStorable(true);
	numFn.setKeyable(true);


	//########################################
	//RADIUS
	//########################################
	radius = numFn.create("distance","dis",MFnNumericData::kDouble);
	numFn.setMin(0.0);
	numFn.setSoftMax(30.0);
	numFn.setInternal(true);
	numFn.setStorable(true);
	numFn.setKeyable(true);


	//########################################
	//MASK SIZE
	//########################################
	maskSizeOBJ = numFn.create("maskSize","ms",MFnNumericData::kLong);
	numFn.setMin(0);
	numFn.setSoftMax(10);
	numFn.setInternal(true);
	numFn.setStorable(true);
	numFn.setKeyable(true);


	//########################################
	//customFalloff
	//########################################
	customFalloff = numFn.create("customFalloff","cf",MFnNumericData::kDouble);
	numFn.setStorable(true);
	numFn.setKeyable(true);
	numFn.setHidden(true);


	//------------------------------------------
	//ADD ALL ATTRIBUTES
	//------------------------------------------

	addAttribute(inverseMatrix);
	addAttribute(outPnts);
	addAttribute(inMesh);
	addAttribute(outMesh);
	addAttribute(inVertices);
	addAttribute(finalizeObj);
	
	addAttribute(startPoint);
	addAttribute(endPoint);

	addAttribute(falloffVis);
	addAttribute(fTypeOBJ);
	addAttribute(maskSizeOBJ);
	addAttribute(radius);
	addAttribute(scaleObj);
	addAttribute(alongNormalObj);
	addAttribute(normalRelativeOBJ);
	
	addAttribute(customFalloff);
	addAttribute(outWeights);
	

	//------------------------------------------
	//ATTRIBUTE AFFECTS
	//------------------------------------------

	attributeAffects(finalizeObj,outPnts);
	attributeAffects(normalRelativeOBJ,outMesh);

	attributeAffects(falloffVis,outMesh);

	attributeAffects(fTypeOBJ,outMesh);
	attributeAffects(inMesh,outMesh);
	attributeAffects(endPoint,outMesh);
	
	attributeAffects(alongNormalObj,outMesh);
	attributeAffects(scaleObj,outMesh);

	attributeAffects(radius,outMesh);
	attributeAffects(maskSizeOBJ,outMesh);

	attributeAffects(normalRelativeOBJ,outWeights);

	attributeAffects(falloffVis,outWeights);

	attributeAffects(fTypeOBJ,outWeights);
	attributeAffects(inMesh,outWeights);
	attributeAffects(endPoint,outWeights);
	
	attributeAffects(alongNormalObj,outWeights);
	attributeAffects(scaleObj,outWeights);

	attributeAffects(radius,outWeights);
	attributeAffects(maskSizeOBJ,outWeights);


	//die Attribute : startPoint und inVertices beeinflussen den output nicht, weil sie eigentlich nur einmalig gesetzt werden
	//bei mode muss ich sehen, ob man das wueuerend der operation ueuendern kueuennen soll
	return MS::kSuccess;

}

//------------------------------------------------------------------------------------
bool  softTransformationNode::setInternalValue ( const MPlug& plug, const MDataHandle& dataHandle)
//------------------------------------------------------------------------------------
{
	double value;
	
	if(plug == radius)
	{
		value = dataHandle.asDouble();
		
		
		if(value < 0.0)
		{
			const_cast<MDataHandle&>(dataHandle).set(0.0);
		}
	}
	else
	{
		value = dataHandle.asInt();
		
		
		if(value < 0)
		{
			const_cast<MDataHandle&>(dataHandle).set(int(0));
		}
	}

	return MPxNode::setInternalValue (plug, dataHandle);

}


//--------------------------------------------------------------------------------------------------------------------------
MStatus	softTransformationNode::compute(const MPlug& plug, MDataBlock& data)
//--------------------------------------------------------------------------------------------------------------------------
{
	MStatus status;

	if(onStartup)
	{
		MPRINT("Bin in StartUp")
		
		MFnIntArrayData	arrayData(data.inputValue(inVertices).data());
		//inVtx auf keinen Fall editieren, da das von arrayData gegebene OBJ editierbar ist
		inVtx = arrayData.array();
		
		//gleich auf outueueMesh arbeiten
		data.outputValue(outMesh).set(data.inputValue(inMesh).asMesh());

		//weil die inVtx nicht perfekt sein mueuessen, wenn faces oder edges gewueuehlt waren
		if(inVtx.length() != 0)
			helper.memoryPrune(inVtx);
	

		startPos = data.inputValue(startPoint).asVector();

		INVIS(cout<<"inVtx lueuenge = "<<inVtx.length()<<endl;)


		//jetzt das AnimFn installieren
		MObjectArray objects;
						
		if(MAnimUtil::findAnimation(MPlug(thisMObject(),customFalloff),objects))
		{
			animFn.setObject(objects[0]);
		}



		onStartup = false;
	}

	if(plug == outMesh)
	{
		MDataHandle outputHandle = data.outputValue(outMesh);
		
		//jetzt die Daten setzen ->hier eventuelll die CopyMethode von dataHandle nehmen
	//	outputHandle.set(data.inputValue(inMesh).asMesh());
		//jetzt direkt auf dem output arbeiten
		fMesh = outputHandle.asMesh(); //inMesh muss auch jedes mal wieder geholt werden, da man ja auf inMesh arbeitet
		
		
		vis = data.inputValue(falloffVis).asBool();
		fDistance = data.inputValue(radius).asDouble();

		
		if(fDistance == 0.0)
			fDistance = 0.0001;	//damit eingangsAuswahl immer selectiert ist

		maskSize = data.inputValue(maskSizeOBJ).asInt();
		
		fType = data.inputValue(fTypeOBJ).asShort();


		MFnMatrixData	matData(data.inputValue(inverseMatrix).data());
		MMatrix	matrix = matData.matrix();

		//nur zeitweilig
		alongNormal = data.inputValue(alongNormalObj).asDouble();
		scale		= data.inputValue(scaleObj).asDouble() / 10;

		isNormalRelative = data.inputValue(normalRelativeOBJ).asBool();
		
		vector = (MPoint(data.inputValue(endPoint).asVector()) - startPos ) * matrix;
		origin = startPos;
		origin = origin * matrix;



		//jetzt die eigentliche Arbeit machen
		//----------------
		doTweak();
		//----------------



		outputHandle.setClean();
	}
	else if( plug == outPnts )
	{
		MPRINT("Will outPnts evaluieren")
		//MFnMesh	fnMesh(data.inputValue(inMesh).asMesh());
		MFnMesh	fnMesh(data.outputValue(outMesh).asMesh());

		MDataHandle	outVectors = data.outputValue(outPnts);

		/*
		if(colorStorage.length() == 0)
		{//-> es ist nix passiert, also gleich aussteigen
			data.outputValue(outMesh).setClean();
			outVectors.setClean();
		
			return MS::kSuccess;
		}
		*/

		int numVtx = fnMesh.numVertices();

		MVectorArray	vectors(numVtx);

		int l = tweakVtx.length();
		
		if(isNormalRelative)
		{
			for(int i = 0; i < l; i++)
			{
				vectors[tweakVtx[i]] = (vector + tweakNormals[i] * maxScale * alongNormal + scaleVectors[i] * scale) * recalTweakVtxScale[i];		
			}
		}
		else
		{
			for(int i = 0; i < l; i++)
			{
				vectors[tweakVtx[i]] = (vector + tweakNormals[i] * tweakNormalScale[i] * alongNormal + scaleVectors[i] * scale) * recalTweakVtxScale[i];				
			}
		}

		MFnVectorArrayData	vectorFn;
		
		
		outVectors.set(vectorFn.create(vectors));


		/*
		//jetzt noch die VtxColors zurueuecksetzen
		fnMesh.setObject(data.outputValue(outMesh).asMesh());
		

		
	
		MIntArray tmpArray;
		
		for(int i = 0; i < numVtx; i++)
			tmpArray.append(i);
		
		
		status = fnMesh.setVertexColors(colorStorage,tmpArray);
		//status.perror("");
		*/

		MPRINT("HABE FARBEN ZURueueCKGESETZT")

		data.outputValue(outMesh).setClean();
		outVectors.setClean();
	}
	else if(plug == outWeights)
	{
		MDataHandle outHandle = data.outputValue(outWeights);
		MFnDoubleArrayData	doubleData;

		//fueuer den Fall dass noch keine werte erzeugt wurden, dieses nachholen
		//wahrscheinlich nicht nueuetig, wenn outmesh immer zuerst connected wird

		//jetzt noch komplettes array erstellen
		MFnMesh	meshFn(data.outputValue(outMesh).asMesh());
		MDoubleArray	result(meshFn.numVertices(),0.0);

		unsigned int numTweaks = tweakVtx.length();


		for(unsigned int i = 0; i < numTweaks; i++)
		{
			result[tweakVtx[i]] = recalTweakVtxScale[i];
		}


		outHandle.set(doubleData.create(result));
		outHandle.setClean();

	}
	else
	{
		return MS::kUnknownParameter;
	}


	return status;

}


//--------------------------------------------------------------------------------
bool	softTransformationNode::doTweak()
//--------------------------------------------------------------------------------
{
		
	MItMeshVertex	vertIter(fMesh);
	MFnMesh	fnMesh(fMesh);
	
	computeNumVtx = fnMesh.numVertices();

	
	
	MItMeshPolygon polyIter(fMesh);

	int l,tmp;




	//vtxColors Speichern zwecks spueueterer wiederherstellung




	if(oldFDistance != fDistance || maskSize != oldMaskSize)
	{//Die TweakArrays mueuessen neu aufgebaut werden

		rebuildTweakArrays(polyIter,vertIter );
		
		//schlussendlich die TweaksSCales neuberechnen
		recalculateTweakScaleFactors(fnMesh,true);

		

		oldFDistance = fDistance;
	}





	if(oldVis == false && vis == true)
	{
		recalculateTweakScaleFactors(fnMesh,true);
		oldVis = vis;
		MGlobal::executeCommand("polyOptions -cs 1;",false,false);
	}
	else
	{
		//fueuer den Fall, dass sich falloffTypes geueuendert haben, neuberechnen
		recalculateTweakScaleFactors(fnMesh);

	}
	




	//jetzt die eigentliche verschiebeArbeit machen (eventuell gleich ganzes pointArray extrahieren und dann darauf arbeiten

	l = tweakVtx.length();
	
	if(isNormalRelative)
	{
		for(int i = 0; i < l; i++)
		{
			
			
			vertIter.setIndex(tweakVtx[i],tmp);
			
			
			vertIter.setPosition(tweakOrigPos[i] + (vector + tweakNormals[i] * maxScale * alongNormal + scaleVectors[i] * scale) * recalTweakVtxScale[i]);
			
		}
		
	}
	else
	{
		for(int i = 0; i < l; i++)
		{
			
			
			vertIter.setIndex(tweakVtx[i],tmp);
			
			
			vertIter.setPosition(tweakOrigPos[i] + (vector + tweakNormals[i] * tweakNormalScale[i] * alongNormal + scaleVectors[i] * scale) * recalTweakVtxScale[i]);
			
		}
	}



	/*
	if(oldVis != vis)
	{
		if(oldVis == true)
		{//vis war vorher an: jetzt mueuessen VtxColors wiederhergestellt werden
			MPRINT("HABE FARBEN ZURueueCKGESETZT")

				resetToOrigColor(fnMesh);

		}

		oldVis = vis;
	}
	*/

	fnMesh.updateSurface();
	
	return true;
	
}

//---------------------------------------------------------
void		softTransformationNode::clearAnimCurve()
//---------------------------------------------------------
{
	unsigned int count = animFn.numKeys();

	for(unsigned int i = 0; i < count; i++)
	{
		animFn.remove(0);
	}
}

//-----------------------------------------------------------------------------------------
void	softTransformationNode::recalculateTweakScaleFactors(MFnMesh& fnMesh,bool force)
//-----------------------------------------------------------------------------------------
{
	int l = tweakVtxScale.length();


	if(oldFType != fType || force)
	{
		INVIS(cout<<"Berechne Falloff neu, fType ist "<<fType<<endl;)
		//Scale neu ausrechnen
		switch(fType)
		{
		case 0:
			{//linear
				recalTweakVtxScale = tweakVtxScale;
				break;
			}
			case 1:
			{
				//smooth
			
				double value;
				clearAnimCurve();

				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);

				animFn.setAngle(0,MAngle(0.0),false);
				animFn.setAngle(1,MAngle(0.0),true);


				recalTweakVtxScale.clear();
				for(int i = 0; i < l; i++)
				{
					animFn.evaluate(tweakVtxScale[i],value);
					recalTweakVtxScale.append(value);
				}

				break;
			}
		case 2:
			{
				//Spike
			
				double value;
				clearAnimCurve();

				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);

				animFn.setAngle(0,MAngle(0.0),false);
				animFn.setAngle(1,MAngle(0.093349),true);


				recalTweakVtxScale.clear();
				for(int i = 0; i < l; i++)
				{
					animFn.evaluate(tweakVtxScale[i],value);
					recalTweakVtxScale.append(value);
				}

				break;
			}
		case 3:
			{
				//Dome
			
				double value;
				clearAnimCurve();

				animFn.addKey(0.0,0.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);
				animFn.addKey(1.0,1.0,MFnAnimCurve::kTangentSmooth,MFnAnimCurve::kTangentSmooth,NULL);

				animFn.setAngle(0,MAngle(0.093349),false);
				animFn.setAngle(1,MAngle(0.0),true);


				recalTweakVtxScale.clear();
				for(int i = 0; i < l; i++)
				{
					animFn.evaluate(tweakVtxScale[i],value);
					recalTweakVtxScale.append(value);
				}

				break;
			}

			

		}

		oldFType = fType;
	}

}


//-----------------------------------------------------------------------------------------------------
void	softTransformationNode::rebuildTweakArrays(MItMeshPolygon& polyIter, MItMeshVertex& vertIter)
//-----------------------------------------------------------------------------------------------------
{

	//erstmal checken, ob alles legitim
								INVIS(cout<<endl;)
								INVIS(cout<<"------------------------------------------------"<<endl;)
								INVIS(cout<<"BAUE TWEAKARRAYS AUF"<<endl;)
								INVIS(cout<<"------------------------------------------------"<<endl;)
	int tmp;
	unsigned l;
	unsigned numVerts = vertIter.count();
	//zuallererst die positionen der vertizen wiederherstell
	MPRINT("Habe Tweaks zurueueckgesetzt")

	l = tweakVtx.length();
	for(unsigned x = 0; x < l; x++)
	{
		vertIter.setIndex(tweakVtx[x], tmp);
		vertIter.setPosition(tweakOrigPos[x]);
	}


	
	
	//erstmal alle arrays lueueschen
//	tweakParentOrigPos.clear();		//diese Arrays werden einfach in die TweaksArrays integriert, da nun ja keine slideProzedur
									//mehr vorgeschaltet ist
//	tweakParentVtx.clear();

	
	tweakNormals.clear();
	tweakNormalScale.clear();
	
	scaleVectors.clear();
	tweakVtxScale.clear();			//original TweakScale (linear)
	recalTweakVtxScale.clear();	//durch Funktion verueuenderte Version von tweakScale
	
	
	tweakVtx.clear();			//IDs der Vtx, die durch SoftSelection verschoben werden sollen
	tweakOrigPos.clear();


			
	if(ptVtx.length() == 0)
	{
		if(maskSize != 0)
		{
			//ptVtx mit growSelection fueuellen ueueber extraProc

			generateTweakMask(vertIter,polyIter);

		}
		else
		{
			ptVtx.setLength(numVerts);	//potentielle TweakVtx
			
			//array fueuellen:
			for(unsigned i = 0; i < numVerts; i++)
			{
				ptVtx[i] = i;
			}


		}

		
	}
	else
	{

			if(maskSize == 0)
			{//useMask wurde deaktiviert
				oldMaskSize = 0;
				//also wieder alle PTs nehmen
				ptVtx.setLength(numVerts);	//potentielle TweakVtx
				
				//array fueuellen:
				for(unsigned i = 0; i < numVerts; i++)
				{
					ptVtx[i] = i;
				}
				
				//jetzt die parentVtx abwueuehlen
				//memoryArrayRemove(ptVtx, inVtx);
			}
			else if(maskSize != oldMaskSize)
			{//useMask wurde aktiviert
				//ptVtx neu aufbauen, wenn vorher offensichtlich
				//if(ptVtx.length() + slideIndices.length() >= vertIter.count() && oldMaskSize == 0)
				if(oldMaskSize == 0)
					ptVtx.clear();
				
				generateTweakMask(vertIter,polyIter);
			}
		
	}


	INVIS(cout<<"PTVtx.length in Main = "<<ptVtx.length()<<endl;)

	//jetzt durchs array parsen
	double minDistance, dTmp; int minIndex;
	MPoint startPoint;
	bool slideValid, normalValid;
	
	maxScale = 0.0;

	l = ptVtx.length();		unsigned l2 = inVtx.length();

	
	INVIS(cout<<"REBUILT TWEAK ARRAYS: MUSS "<<l * l2<<" RECHNUNGEN AUSFueueHREN"<<endl;)
	


	for(unsigned i = 0; i < l; i++)
	{
		minIndex = -1;
		minDistance = 16666666666;
		slideValid = normalValid = false;

		vertIter.setIndex(ptVtx[i],tmp);
		startPoint = vertIter.position();

		//distancen ausrechnen: hier eventuell noch ne adaptivitueuet einbauen: z.B nur jeden 2. vtx ind SlideIndices einbeziehen
		for(unsigned x = 0; x < l2; x++)
		{//nun ist die Frage, ob es schneller ist, den vertIter zu strapazieren, oder sich die Position einfach zu berechnen ... 
			vertIter.setIndex(inVtx[x],tmp);
			
			dTmp = (vertIter.position() - startPoint).length();

			if(minDistance > dTmp)
			{
				minDistance = dTmp;
				minIndex = x;
			}
		}


		//jetzt angaben auswerten und tweakVtx hinzufueuegen
		if(minDistance <= fDistance)
		{

			
			tweakVtx.append(ptVtx[i]);
			tweakOrigPos.append(startPoint);
			
			//scaleVektor ausrechnen
			scaleVectors.append(startPoint - origin);

			//scales errechnen
		
			tweakVtxScale.append(1 - (minDistance / fDistance) );
			
			
			
			
			//normale errechnen
			MIntArray faces;
			vertIter.setIndex(ptVtx[i],tmp);
			vertIter.getConnectedFaces(faces);
			MVector	tmpNormal;
			MVector	normal;
			double scale = 0.0;
			
			
			for(unsigned x = 0; x < faces.length(); x++)
			{
				polyIter.setIndex(faces[x],tmp);
				polyIter.getNormal(tmpNormal);
				
				polyIter.getArea(dTmp);
				
				scale += dTmp;
				normal += tmpNormal;
			}
			
			normal.normalize();
			scale /= faces.length();

			//plopping vermeiden
			if(scale > maxScale)
				maxScale = scale;
			
			tweakNormals.append(normal);		//fueuer den Fall speichern, das gepushed wird
			tweakNormalScale.append(scale);
		}
		


	}
	//for(element in ptVtx) ENDE

	INVIS(cout<<"ZAHL DER TWEAKS GESAMT: "<<tweakVtx.length()<<endl;)
	INVIS(cout<<"TweakVtxScale: "<<tweakVtxScale.length()<<endl;)
	

}

//-----------------------------------------------------------------------------------------------------------------------------------
void	softTransformationNode::generateTweakMask(MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//-----------------------------------------------------------------------------------------------------------------------------------
{

	if(ptVtx.length() == 0)
	{	

		ptVtx = MIntArray(inVtx);	//wirkliche kopie machen
	}

	MPRINT("GENERIERE TWEAK MASK")

	int tmp = maskSize - oldMaskSize;
	if(tmp > 0)
	{//size hat sich vergrueueueueert
		for(int i = 0; i < tmp; i++)
		{
			growVtxSelection(ptVtx,vertIter,polyIter);
			INVIS(cout<<"PTVtxLength = "<<ptVtx.length()<<endl;)
		}

	}
	else
	{//size hat sich verkleinert
		tmp = abs(tmp);
	//	addIntArrayToLHS(ptVtx,inVtx);	inVtx sind ja jetzt immer in ptVtx
		for(int i= 0; i < tmp; i++)
			schrinkVtxSelection(ptVtx,vertIter,polyIter);

	}


	helper.memoryPrune(ptVtx);
	//zumSchluss slideIndies wieder abziehen
//	MIntArray copy = inVtx; //weil arrayRemove auch rhs beeinflusst
//	arrayRemove(ptVtx,copy);
	
	INVIS(cout<<"PtVtx Lueuenge in GMask: "<<ptVtx.length()<<endl;)

	oldMaskSize = maskSize;

}



//---------------------------------------------------------------------------------------------------------------------------
void		softTransformationNode::schrinkVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter,MItMeshPolygon& polyIter)
//---------------------------------------------------------------------------------------------------------------------------
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

	helper.arrayRemove(vtxArray,removals);
	

	MPRINT("Habe Vtx geschrinkt")

}

//--------------------------------------------------------------------------------------------------------------------------
void		softTransformationNode::growVtxSelection(MIntArray& vtxArray, MItMeshVertex& vertIter, MItMeshPolygon& polyIter)
//-----------------------------------------------------------------------------------------------------------------------
{
	MPRINT("BIN IN GROW")
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


	helper.addIntArrayToLHS(vtxArray,growedSelection);


	MPRINT("Habe Vtx gegrowed")



}

