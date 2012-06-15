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
// File: ByronsPolyToolsNode.cpp
//
// Dependency Graph Node: ByronsPolyToolsNode
//
// Authors: Sebastian Thiel
//

#include "ByronsPolyToolsNode.h"


//------------------------
//statische ELEMENTE
//------------------------


//	auflisten der statischen Datenelemente
MObject ByronsPolyToolsNode::mode;

MObject ByronsPolyToolsNode::slide;
MObject ByronsPolyToolsNode::normal;

MObject ByronsPolyToolsNode::selVertIDs;
MObject ByronsPolyToolsNode::selEdgeIDs;
MObject ByronsPolyToolsNode::selFaceIDs;

MObject ByronsPolyToolsNode::whichSide;

MObject	ByronsPolyToolsNode::slideRelative;
MObject	ByronsPolyToolsNode::normalRelative;

MObject ByronsPolyToolsNode::options;

MObject	ByronsPolyToolsNode::BPTinMesh;	
MObject	ByronsPolyToolsNode::BPToutMesh;

MObject	ByronsPolyToolsNode::slideLimitedObj;


STE_DECLARE_STATIC_PARAMETERS(ByronsPolyToolsNode,BPT)


// Unique Node TypeId
//
// --------------------------------------------------
MTypeId     ByronsPolyToolsNode::id( 0x00108BC0 );
// --------------------------------------------------


//Constructor/Destructor
//----------------------------------------------------------------------------------------------------------
ByronsPolyToolsNode::ByronsPolyToolsNode():	meshDirty(true),
											oldShowMesh(false), 
											undoItCalled(false),
											isAutoSelect(false),
											check(false),
											isSCMode(false),
											dgMod(0)
//----------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------
ByronsPolyToolsNode::~ByronsPolyToolsNode()
//----------------------------------------------------------------------------------------------------------
{
}


//----------------------------------------------------------------------------------------------------------
MStatus	ByronsPolyToolsNode::connectionBroken( const MPlug& plug, const MPlug& otherPlug, bool asSrc )
//----------------------------------------------------------------------------------------------------------
{
	//PROBLEM: UNDO/REDO erzeugt exceptions wenn das hier an ist

	/*
	MPRINT("-------------------------------")
	MPRINT("Connection Wird gelueuescht")
	INVIS(cout<<"In Plug: "<<plug.name()<<endl;)
	INVIS(cout<<"otherPlug: "<<otherPlug.name()<<endl;)
	INVIS(cout<<"IsSource: "<<asSrc<<endl;)

	if(plug == outMesh && asSrc)
	{
		if(!otherOutMeshPlug.isNull())
		{//savedOtherPlug mit diesem OtherPlug verbinden
			if(dgMod == 0)
			{
				dgMod = new MDGModifier();
				dgMod->connect(otherOutMeshPlug,otherPlug);
				dgMod->doIt();
				return MPxNode::connectionBroken(plug,otherPlug,asSrc);
			}
			else
			{
				dgMod->doIt();
				return MPxNode::connectionBroken(plug,otherPlug,asSrc);
			}
		}
		
		return MPxNode::connectionBroken(plug,otherPlug,asSrc);
	}
	else if(plug == inMesh && !otherPlug.isNull())
	{//other out attribut speichern	
		otherOutMeshPlug = otherPlug;
		return MPxNode::connectionBroken(plug,otherPlug,asSrc);
	}
*/

	// Mal die softTransformationEngine prozedur aufrufen - sie kuemmert sich um die visNode, welche ansonsten verhindern wuerde, dass die history wirklich gekillt wird
//	return	softTransformationEngine::connectionBroken(plug, otherPlug, asSrc);

//	return MPxNode::connectionBroken(plug,otherPlug,asSrc);
	return MS::kUnknownParameter;
//	return MS::kFailure;
}


//-----------------------------------------------------------------------------------------------------
MStatus	ByronsPolyToolsNode::connectionMade( const MPlug& plug, const MPlug& otherPlug, bool asSrc )
//-----------------------------------------------------------------------------------------------------
{
	/*
	MPRINT("-------------------------------")
	MPRINT("Connection Wird erstellt")
	INVIS(cout<<"In Plug: "<<plug.name()<<endl;)
	INVIS(cout<<"otherPlug: "<<otherPlug.name()<<endl;)
	INVIS(cout<<"IsSource: "<<asSrc<<endl;)

	if(dgMod != 0)
	{

		dgMod->undoIt();
		delete dgMod;
		dgMod = 0;

	}
*/
	//return MPxNode::connectionMade(plug,otherPlug, asSrc);
	return MS::kUnknownParameter;
}


//----------------------------------------------------------------------------
MStatus		ByronsPolyToolsNode::initialize()
//----------------------------------------------------------------------------
{
	
	
	MFnEnumAttribute	FnEnumAttr;
	MFnTypedAttribute	FnTypedAttr;
	MFnNumericAttribute	FnFloatAttr;
	MStatus				status;



	
	// InMesh
	BPTinMesh = FnTypedAttr.create("inMesh","in",MFnData::kMesh);
	FnTypedAttr.setStorable(true);
	//FnTypedAttr.setCached(false);
	FnTypedAttr.setInternal(true);
	

	// outMesh
	BPToutMesh = FnTypedAttr.create("outMesh","out",MFnData::kMesh);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setWritable(false);




	mode = FnFloatAttr.create("mode","mde",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setHidden(true);
	FnFloatAttr.setConnectable(false);

	selVertIDs = FnTypedAttr.create("vertIDs","vID",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);




	selEdgeIDs = FnTypedAttr.create("edgeIDs","eID",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);



	selFaceIDs = FnTypedAttr.create("faceIDs","fID",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);
	

	options = FnTypedAttr.create("options","op",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);


	slide = FnFloatAttr.create("slide","sl",MFnNumericData::kDouble);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setInternal(true);
//	FnFloatAttr.setMin(0.0);
//	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);

	normal = FnFloatAttr.create("alongNormal","an",MFnNumericData::kDouble);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setDefault(0.0);
	FnFloatAttr.setKeyable(true);

	slideRelative = FnFloatAttr.create("slideRelative","sr",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);
	

	normalRelative = FnFloatAttr.create("normalRelative","nr",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);


	whichSide = FnFloatAttr.create("side","si",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);


		
	slideLimitedObj = FnFloatAttr.create("slideLimited","sll",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(false);







	status = addAttribute(BPTinMesh);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(slideRelative);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(normalRelative);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(whichSide);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(normal);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(slide);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(slideLimitedObj);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(options);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(mode);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(selVertIDs);
	MCheckStatus(status, "AddAttrBPTNode");

	status = addAttribute(selEdgeIDs);
	MCheckStatus(status, "AddAttrBPTNode");
	
	status = addAttribute(selFaceIDs);
	MCheckStatus(status, "AddAttrBPTNode");




	// ATTRIBUTE AFFECTS
	// --------------------

	status = attributeAffects( BPTinMesh, BPToutMesh );
	MCheckStatus(status, "AddAttrBPTNode");

	status = attributeAffects( slide, BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");
		
	status = attributeAffects( slideRelative, BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");

	status = attributeAffects( normalRelative, BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");

	status = attributeAffects( whichSide, BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");
		
	status = attributeAffects( normal, BPToutMesh);
	MCheckStatus(status, "AddAttrBPTNode");
		

	// Zuletzt die SoftTransformationAttribute hinzufuegen
	// Per Macro - dirty, aber funktioniert - wie machen die ALIAS Typen das ??? Die leiten auch stuendig von einer BaseNode ab, und da gehen dann keine Attribute flueten
	// Oder werden unbrauchbar so wie bei mir, so dass im Endeffekt suemtliche Attribute ein eindeutiges statisches Attribut haben muessen

	STE_ADD_ATTRIBUTES(BPT)

		// zum Schluss den MAnipulator initialisieren
	//	MPxManipContainer::addToManipConnectTable( id );


		return status;
}

//---------------------------------------------------------------------------------------------------
bool	ByronsPolyToolsNode::setInternalValue ( const MPlug & plug, const MDataHandle & dataHandle)
//---------------------------------------------------------------------------------------------------
{ 

//	cout<<"War hier, plug war "<<plug.info()<<endl;

	if(plug == slide)
	{//sichergehen, dass die Options gefunden werden, auch kurz nach erstellung der node
		int isLimited;
		
		MFnDependencyNode depFn(plug.node());
		depFn.findPlug( "sll" ).getValue( isLimited );

		
		if( isLimited )
		{//limits setzen, wenn gewueuenscht
			
			double value = dataHandle.asDouble();
			
	//		cout<<value<<endl;

			if(value > 1.0)
				value = 1.0;

			if(value < 0.0)
				value = 0.0;

			
			//Das muss hier einfach sein, um limits dynamisch zu setzen je nach userwunsch
			 const_cast<MDataHandle&>(dataHandle).set(value);
		}



	}



	if(plug==BPTinMesh)
		meshDirty = true; 


	// Sich jetzt um die STE internals kuemmern
	//
	//

	STE_CHECK_INTERNALS( BPT,dataHandle );


	return MPxNode::setInternalValue (plug, dataHandle);
}


//---------------------------------------------
void	makeSelection(void* data)
//---------------------------------------------
{

	
	ByronsPolyToolsNode* nodePtr = (ByronsPolyToolsNode*) data;
	MEventMessage::removeCallback(nodePtr->eID);

	MDagPath meshPath;
	nodePtr->getMeshPath(meshPath);

	if(meshPath.apiType() != MFn::kInvalid && nodePtr->validIndices.length() != 0)	//zur Sicherheit, sollte aber eigentlich nicht mueueglich sein
	{
		MFnSingleIndexedComponent compFn;
		
		MFn::Type type;
		if(nodePtr->validIndices[0] == 1)
			type = MFn::kMeshEdgeComponent;
		else if(nodePtr->validIndices[0] == 2)
			type = MFn::kMeshPolygonComponent;
		else if(nodePtr->validIndices[0] == 3)
			type = MFn::kMeshVertComponent;

		//flag wieder entfernen
		nodePtr->validIndices.remove(0);

		MSelectionList compList, current;
		
		MObject comps = compFn.create(type);
		compFn.addElements(nodePtr->validIndices);
		
		compList.add(meshPath,comps);
		compList.add(nodePtr->thisMObject());
	

	

	//	MGlobal::getActiveSelectionList(current);
		
		MGlobal::setActiveSelectionList(compList, MGlobal::kAddToList);

		
		// Speicher freigeben
		nodePtr->validIndices.setLength(0);
		
		//	MGlobal::setActiveSelectionList(current,MGlobal::kAddToList);

	}
}

//----------------------------------------------------------------------------
MStatus ByronsPolyToolsNode::doCompleteCompute(MDataBlock& data)
//----------------------------------------------------------------------------
{
				MStatus status;

				MPRINT("MACHE KOMPLETTE BERECHNUNG")

				//gleich zu beginn muss der MeshPath initialisiert werden, damit der MeshPath an die fty ueuebergeben werden kann
				//dieser Block wird von mehreren nachfolgenden Prozeduren vrwendet und muss erhalten bleiben

				// ****************
				// MESHPATH HOLEN
				// ****************

				// Dies wird von der softTransformationEngine gemacht
				
				// Diese Methode ist essentiell, da sie zugriff auf alle Attribute ermueglicht
				softTransformationEngine::gatherAttributeObjects(thisMObject());
				softTransformationEngine::saveMeshPathes();

				fBPTfty.setMeshPath(meshPath);


				// ***************************
				// IN/OUTMESH HANDLES HOLEN
				// ***************************

				MDataHandle inMeshHandle = data.inputValue(BPTinMesh, &status );
				MDataHandle outMeshHandle = data.outputValue(BPToutMesh);
				
				
				// INITIALVTXCOUNT SETZEN
				inMeshHandle.data();
				MFnMesh meshFn(inMeshHandle.asMesh());
				initialVtxCount = meshFn.numVertices();

				
			//	MPxNode::setExistWithoutOutConnections(true);
			//	MPxNode::setExistWithoutInConnections(true);
				
				//				inMesh direkt an outMesh und MObject mesh geben
				//	outMeshHandle.set(inMeshHandle.asMesh());
				//	fBPTfty.setMesh(outMeshHandle.asMesh());

				
				
				
				// SLIDE RELATIVE PARAMETER SETZEN
				
				MDataHandle	rHandle = data.inputValue(slideRelative);
				fBPTfty.setRelative(rHandle.asInt());
				
				
				// NORMAL RELATIVE PARAM SETZEN

				MDataHandle nRelativeHandle = data.inputValue(normalRelative);
				fBPTfty.setNormalRelative(nRelativeHandle.asInt());
				

				// SEL VERT IDS SETZEN

				MDataHandle		arrayHandle = data.inputValue(selVertIDs);
				MFnIntArrayData		intDataArray(arrayHandle.data());
				
				fBPTfty.setVertIDs( intDataArray.array() );
				
				

				// SEL FACE IDS SETZEN
				arrayHandle = data.inputValue(selFaceIDs,&status);
				intDataArray.setObject(arrayHandle.data());
				fBPTfty.setPolyIDs( intDataArray.array() );
				
				
				
				// SEL EDGE IDS SETZEN
				arrayHandle = data.inputValue(selEdgeIDs);
				intDataArray.setObject(arrayHandle.data());
				fBPTfty.setEdgeIDs( intDataArray.array() );
				


				// OPTIONEN HOLEN
				
				arrayHandle = data.inputValue(options);
				intDataArray.setObject(arrayHandle.data());
				MIntArray optionsArray(intDataArray.array());
				
				fBPTfty.setOptions(optionsArray);
				
				
				
				// SLIDE PARAM SETZEN
				MDataHandle slideHandle = data.inputValue(slide);
				fBPTfty.setSlide(slideHandle.asDouble());
				//fBPTfty.setSlide(0.5);

				
				
				
				// ACTIONMODE SETZEN
				MDataHandle modeHandle	= data.inputValue(mode);
				fBPTfty.setActionMode(modeHandle.asInt());
				
				//				damit Undo funzen kann
			
				

				// MESH HANDLE SETZEN IDS SETZEN
				fBPTfty.setMesh(inMeshHandle.asMesh());
				


				// ThisNode setzen
				MObject thisObj = thisMObject();
				fBPTfty.setThisMObject(thisObj);

				
				try
				{
				//ACTION
				status = fBPTfty.doIt();
				}

				catch(...)
				{
					MGlobal::displayError(" An unknown, severe, error occoured.\nIf it happens again in this situation, please write a bug report.\nPlease undo the operation and save your work!");
					return MS::kUnknownParameter;
				}

				
				MObject newOutMesh;
				if(status == MS::kSuccess)
				{
					newOutMesh = fBPTfty.getMesh();
					outMeshHandle.set(newOutMesh);
				}
				else
				{//Fehler ist aufgetreten - Failure
					return MS::kFailure;
				}
				//outMeshHandle.set(fBPTfty.getMesh());
				
				
			

				// **************************************************************************************
				// JETZT DIE INPUT DATEN SETZEN FueR SOFTTRANSFORMATION NODE
				// **************************************************************************************

				int mode = modeHandle.asInt() ;
				switch( mode )
				{
				case 4:	// Chamfer
				case 5: // SC
					{
						// Hier ist es aufwendig: Alle neuen Vtx und jene, die im inMesh die gewuehlten kanten ausgemacht haben muessen gewuehlt werden
						// Man kann einfach die SlideIndices verwenden
						softTransformationEngine::setVtxSet( fBPTfty.getSlideIndices(), data );
						break;
					}
				case 6:
					{	// SMC - Die VtxSelection muss verwendet werden

						MIntArray allVtxToMove;
						
						fBPTfty.convertInputToVtxIndices(allVtxToMove);
						
						softTransformationEngine::setVtxSet(allVtxToMove, data);
						
						break;
					}
				default:
					{
						softTransformationEngine::setVtxSet( data );
					}
				}
				

				//------------SELECTION ROUTINE----------------------


				//jetzt die Components anwueuehlen
				

				isSCMode = (mode == 4 || mode == 5)?true:false;

				//MDagPath vom Mesh holen
				bool selectEdges = true;
				//MFnDependencyNode depNodeFn(thisMObject());
				
				
				//depNodeFn.findPlug(outMesh).connectedTo(connections,false, true);

				if( mode != 6 && optionsArray[6] > 0)
				{
					if( meshPath.isValid() )
					{

						//componentConverter erzeugen
						componentConverter CC(newOutMesh);

						//MDagPath::getAPathTo(connections[0].node(),meshPath);
						
						validIndices.setSizeIncrement(100);

						MIntArray& slideIndices = fBPTfty.getSlideIndices();

						isAutoSelect = true;

						uint l, i;

						
						if(optionsArray[6] != 5)
						{
							if(mode == 0)
							{
								//contained edges holen
								CC.getContainedEdges(slideIndices,validIndices);
								
								//wenn nueuetig edges in Faces umwandeln
								if(optionsArray[6] == 2)
								{
														
									if(mode == 0 || mode == 5)
									{//connectedFaces
										MIntArray validFaces;
										validFaces.setSizeIncrement(100);

										CC.getConnectedFaces(validIndices,validFaces);

										validIndices = validFaces;
										
									}

									validIndices.insert(2,0);
								}
								else
								{
									validIndices.insert(1,0);
								}
								
							}
							else if(mode == 4)
							{
								
								
								MIntArray polygons;
								polygons.setSizeIncrement(100);

								//polySelection erstellen (contained)
								MItMeshPolygon polyIter(newOutMesh);
								int newCount = polyIter.count();
								
								MItMeshPolygon polyIter2(inMeshHandle.data());
								
								int oldCount = polyIter2.count();
								int l = oldCount + (newCount - oldCount);
								for(int i = oldCount; i < l; i++)
									polygons.append(i);
								
								
								
								if(optionsArray[6] != 2 && polygons.length() > 0)
								{
									//fty NH erstellen fueuer external Boundary
									BPTfty_NH fty_NH;
									fty_NH.externalBoundary(validIndices,polygons,newOutMesh);
									validIndices.insert(1,0);
								}
								else
								{
									validIndices = polygons;
									validIndices.insert(2,0);
								}
								
								
							}
							else if(mode == 5)
							{
								
								MIntArray polygons;
								polygons.setSizeIncrement(100);
								
								if(optionsArray[6] > 1 && optionsArray[6] < 5)
								{//polySelection erstellen (contained)
								

									MItMeshPolygon polyIter(newOutMesh);
									int newCount = polyIter.count();
									
									MItMeshPolygon polyIter2(inMeshHandle.data());
									
									int oldCount = polyIter2.count();
									int l = newCount;
									for(int i = oldCount; i < l; i++)
										polygons.append(i);
									

									if(optionsArray[6] != 2 && polygons.length() > 0)
									{
										BPTfty_NH fty_NH;
										fty_NH.externalBoundary(validIndices,polygons,newOutMesh);
									}
								}
								

			
								if( optionsArray[6] == 1 || optionsArray[6] == 4)
								{
									MIntArray& maySlide = fBPTfty.getMaySlide();
									MIntArray mayNotSlideIDs;
									mayNotSlideIDs.setSizeIncrement(ULONG(maySlide.length() / 8));
									
									
									
									//maySlideIndices und nicht MaySlideIndices trennen
									l = maySlide.length();
									for(i = 0; i < l; i++)
									{
										if(!maySlide[i])
											mayNotSlideIDs.append(slideIndices[i]);
									}
									
									helper.memoryPrune(mayNotSlideIDs);



									MItMeshVertex vertIter(newOutMesh);
									

									
									if(optionsArray[6] == 1 || optionsArray[6] == 4)
									{
										
										CC.getContainedEdges(mayNotSlideIDs, validIndices);
									}
							
								}

								if(optionsArray[6] != 2 )
								{
									validIndices.insert(1,0);
								}
								else
								{
									validIndices = polygons;
									validIndices.insert(2,0);
								}
										
							}
						
							
						}
						else
						{//verts wueuehlen
							validIndices = slideIndices;
							validIndices.insert(3,0);
						}
						
						eID = MEventMessage::addEventCallback("idle",makeSelection,this);

					}
				}
			


				return status;

}





//----------------------------------------------------------------------------
MStatus		ByronsPolyToolsNode::compute(const MPlug& plug, MDataBlock& data)
//----------------------------------------------------------------------------
{
	
	
//	FactoryWerte setzen
//	(hier ueueberall eventuell noch MCheck nutzen fueuer Debug wenn nueuetig)
	MStatus status;
	
		MDataHandle stateHandle = data.outputValue(state);
		
	//	INVIS(cout<<"Plug in BPTCompute ist: "<<plug.name()<<endl);		

		if(stateHandle.asShort() != 0)
		{
			MDataHandle inMeshHandle = data.inputValue(BPTinMesh);
			MDataHandle outMeshHandle = data.outputValue(BPToutMesh);

			outMeshHandle.set(inMeshHandle.asMesh());
			outMeshHandle.setClean();

		}
		else
		{
			if( (plug == BPToutMesh) )
			{
					
				// -----------------------------------------------------------------
				// Erstmal suemtliche Parameter holen, die den output beeinflussen
				// -----------------------------------------------------------------

					cp.mode = data.inputValue(mode).asLong();
					
					cp.slide = data.inputValue(slide).asDouble();
					cp.alongNormal= data.inputValue(normal).asDouble();


					cp.slideRelative = data.inputValue(slideRelative).asLong();
					cp.normalRelative = data.inputValue(normalRelative).asLong();
				
					cp.whichSide = data.inputValue(whichSide).asLong();


					// Dazu gehuert auch die matrix
					MFnMatrixData	matFn(data.inputValue(BPTinMatrixObj).data());
					cp.matrix = matFn.matrix();

					
					
				if(meshDirty)
				{
					MPRINT("COMPLETE COMPUTE!!!!!!!!!!!!!!!!!!!!!!!!!!!")
					
					status = doCompleteCompute(data);
					
					INVIS(cout<<"MeshDirty ist "<<meshDirty<<endl;)
					INVIS(cout<<"--------------------------------"<<endl;)



					
					INVIS(MFnDependencyNode depNodeFn(thisMObject());)
					
					INVIS(cout<<"---------------"<<endl;)
					INVIS(cout<<depNodeFn.name().asChar()<<endl;)
					INVIS(cout<<"---------------"<<endl;)


					//jetzt noch die SoftTransformationEngine mit daten bestuecken
					//------------------------------------------------------------
					softTransformationEngine::extractStaticData(plug, data);


					// DEAKTIVIEREN DER VISUALISIERUNG,  WENN DIE NODE ABGEWueHLT WIRD - 
					// DIES KANN DANN GETROST ENTFERNT WERDEN
					//cID = MModelMessage::addCallback(MModelMessage::kActiveListModified,activeListChangedCBOnce,this);


					if(status == MS::kUnknownParameter)
					{
						MDataHandle outMeshHandle = data.outputValue(BPToutMesh);
						outMeshHandle.setClean();
						return status;
					}

				}
				else //if
				{
					// An dieser stelle ist es gut mueglich, dass die history geuendert wurde
					// Wenn alle parameter der Node gleich geblieben sind, dann ist dies der Fall
					/* // Macht exceptions ... 
					if(cp == op)
					{
						
						MDataHandle inMeshHandle = data.inputValue(BPTinMesh);
						
						MFnMesh inMeshFn(inMeshHandle.asMesh());
						
						// jetzt ueberpruefen, ob sich die Topologie im Vergleich zum ersten inMesh geuendert hat 
						// - wenn nicht, dann die vtxPositionen des inMeshes aufs outMesh uebertragen und dann normal mit Slide weitermachen
					
						//Absturz nach ein paar durchlueufen aus mir eunbekanntem Grund
						if(initialVtxCount == inMeshFn.numVertices())
						{
							// Topologie mit groueer wahrschenlichkeit unveruendert, also die VtxPositionen uebernehmen, die sich jetzt wahrschenlich geuendert haben
							MPointArray points;
							inMeshFn.getPoints(points);

							MFnMesh meshFn(data.outputValue(BPToutMesh).asMesh() );

							meshFn.setPoints(points);
						}
						
						// jetzte die aktualisierten Daten fuer softTransformationEngine holen
						softTransformationEngine::extractStaticData(plug, data);

					}
					*/

				}

				//es wird generell nach der creation erstmal slide ausgefueuehrt, damit auch along normal und andere parameter stimmen nach ladeVorgang


				//jetzt Daten, die slide erzeugen, extrahieren und mit gespeicherten werten vergleichen.
				//wenn abweichung, wurde also nur einer dieser werte verueuendert und slide muss erfolgen

					
					
					
					
					MDataHandle outMeshHandle = data.outputValue(BPToutMesh);
					
					
					
					fBPTfty.setNormal(cp.alongNormal);
					fBPTfty.setRelative(cp.slideRelative);
					fBPTfty.setNormalRelative(cp.normalRelative);


					//damit reload ordentlich klappt
					//if(meshDirty)
					//	fBPTfty.setSlide(0.5);
					//else
						fBPTfty.setSlide(cp.slide);
					

					meshDirty = false;


					//fBPTfty.setMesh(outMeshHandle.asMesh());
					fBPTfty.setSide(cp.whichSide);


					//test
					fBPTfty.setMesh(outMeshHandle.asMesh());


					if(cp.mode == 6)
					{
						
						fBPTfty.doSlideSMC( false );
						
					}
					else
					{
												
						fBPTfty.doSlide( false );
						
					}


					// nun die Tweaks hinzufuegen
					
					// erstmal Daten extrahieren
					softTransformationEngine::extractNonstaticData(plug, data);
					

					//Jetzt die softSelection anwenden
					softTransformationEngine::doTweak(data);
					
					
					
					outMeshHandle.setClean();


					// zuguterletzt noch die ops aktualisieren
					op = cp;
			//	}

				
			}
			else
				return MS::kUnknownParameter;
		
		}

	
	//return status;
	return MS::kSuccess;
}



//----------------------------------------------------------------------------
void*	ByronsPolyToolsNode::creator()
//----------------------------------------------------------------------------
{
	return new ByronsPolyToolsNode;
}


