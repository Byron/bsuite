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
// File: BPT_InsertVtx.cpp
//
// Dependency Graph Node: BPT_InsertVtx
//
// Authors: Sebastian Thiel
//

#include "BPT_insertVtxNode.h"

#include <cassert>

//------------------------
//Statische Variablen
//------------------------




//	auflisten der statischen Datenelemente
MObject BPT_InsertVtx::IVcount;

MObject BPT_InsertVtx::IVslide;
MObject BPT_InsertVtx::IVnormal;


MObject BPT_InsertVtx::IVselEdgeIDs;
MObject BPT_InsertVtx::IVselVertIDs;

MObject BPT_InsertVtx::IVwhichSide;

MObject	BPT_InsertVtx::IVslideRelative;
MObject	BPT_InsertVtx::IVnormalRelative;

MObject BPT_InsertVtx::IVoptions;
MObject BPT_InsertVtx::IVspin;

MObject BPT_InsertVtx::IVoutMesh;
MObject BPT_InsertVtx::IVinMesh;
MObject	BPT_InsertVtx::IVSlideLimited;

STE_DECLARE_STATIC_PARAMETERS(BPT_InsertVtx,IV)

// Unique Node TypeId
//
MTypeId     BPT_InsertVtx::IVid( 0x00108BC5 );


//Constructor/Destructor
//------------------------------------------------------------------
BPT_InsertVtx::BPT_InsertVtx()
: scriptJobInitated(false)
, meshDirty(true)
, lastCount(-1)
, lastSpin(-1)
//------------------------------------------------------------------
{
}


//------------------------------------------------------------------
BPT_InsertVtx::~BPT_InsertVtx()
//------------------------------------------------------------------
{};


//----------------------------------------------------------------------------
MStatus		BPT_InsertVtx::initialize()
//----------------------------------------------------------------------------
{
	
	
	MFnEnumAttribute	FnEnumAttr;
	MFnTypedAttribute	FnTypedAttr;
	MFnNumericAttribute	FnFloatAttr;
	MStatus				status;

	

	IVinMesh = FnTypedAttr.create("inMesh","in",MFnData::kMesh);
	FnTypedAttr.setStorable(true);
	//FnTypedAttr.setCached(false);
	FnTypedAttr.setInternal(true);
	

	IVoutMesh = FnTypedAttr.create("outMesh","out",MFnData::kMesh);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setWritable(false);



	IVcount = FnFloatAttr.create("count","c",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setInternal(true);
	FnFloatAttr.setKeyable(true);
	FnFloatAttr.setMin(1);
	FnFloatAttr.setSoftMax(15);
	
	

	IVselEdgeIDs = FnTypedAttr.create("edgeIDs","eID",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);


	
	IVselVertIDs = FnTypedAttr.create("vertIDs","vID",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);


	
	IVoptions = FnTypedAttr.create("options","op",MFnData::kIntArray);
	FnTypedAttr.setStorable(true);
	FnTypedAttr.setHidden(true);
	FnTypedAttr.setConnectable(false);


	IVslide = FnFloatAttr.create("slide","sl",MFnNumericData::kDouble);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setInternal(true);
//	FnFloatAttr.setMin(0.0);
//	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);

	IVnormal = FnFloatAttr.create("alongNormal","an",MFnNumericData::kDouble);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setDefault(0.0);
	FnFloatAttr.setKeyable(true);

	IVslideRelative = FnFloatAttr.create("slideRelative","sr",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);
	

	IVnormalRelative = FnFloatAttr.create("normalRelative","nr",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);


	IVwhichSide = FnFloatAttr.create("side","si",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(true);

	IVSlideLimited = FnFloatAttr.create("slideLimited","sll",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setMax(1.0);
	FnFloatAttr.setKeyable(false);


	IVspin = FnFloatAttr.create("spin","sp",MFnNumericData::kLong);
	FnFloatAttr.setStorable(true);
	FnFloatAttr.setMin(0.0);
	FnFloatAttr.setKeyable(true);
	FnFloatAttr.setInternal(true);



	// Add attributes

	status = addAttribute(IVslideRelative);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVnormalRelative);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVwhichSide);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVnormal);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVslide);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVSlideLimited);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVoptions);
	MCheckStatus(status, "AddAttrIVNode");
		
	status = addAttribute(IVspin);
	MCheckStatus(status, "AddAttrIVNode");
	
	status = addAttribute(IVcount);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVselEdgeIDs);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVselVertIDs);
	MCheckStatus(status, "AddAttrIVNode");



	status = addAttribute(IVoutMesh);
	MCheckStatus(status, "AddAttrIVNode");

	status = addAttribute(IVinMesh);
	MCheckStatus(status, "AddAttrIVNode");





	// Add attribute affects

	status = attributeAffects( IVspin, IVoutMesh );
	MCheckStatus(status, "AddAttrAffectsIVNode");

	status = attributeAffects( IVcount, IVoutMesh );
	MCheckStatus(status, "AddAttrAffectsIVNode");

	status = attributeAffects( IVinMesh, IVoutMesh );
	MCheckStatus(status, "AddAttrAffectsIVNode");

	status = attributeAffects( IVslide, IVoutMesh);
	MCheckStatus(status, "AddAttrAffectsIVNode");
		
	status = attributeAffects( IVslideRelative, IVoutMesh);
	MCheckStatus(status, "AddAttrAffectsIVNode");

	status = attributeAffects( IVnormalRelative, IVoutMesh);
	MCheckStatus(status, "AddAttrAffectsIVNode");

	status = attributeAffects( IVwhichSide, IVoutMesh);
	MCheckStatus(status, "AddAttrAffectsIVNode");
		
	status = attributeAffects( IVnormal, IVoutMesh);
	MCheckStatus(status, "AddAttrAffectsIVNode");



	// Zuletzt die SoftTransformationAttribute hinzufuegen
	// Per Macro - dirty, aber funktioniert - wie machen die ALIAS Typen das ??? Die leiten auch stuendig von einer BaseNode ab, und da gehen dann keine Attribute flueten
	// Oder werden unbrauchbar so wie bei mir, so dass im Endeffekt suemtliche Attribute ein eindeutiges statisches Attribut haben muessen

	STE_ADD_ATTRIBUTES(IV)


		return status;
}

//---------------------------------------------------------------------------------------------------
bool	BPT_InsertVtx::setInternalValue ( const MPlug & plug, const MDataHandle & dataHandle)
//---------------------------------------------------------------------------------------------------
{ 

//	cout<<"War hier, plug war "<<plug.info()<<endl;


	if(plug == IVslide)
	{
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
	else if( plug == IVcount )
	{
		int value = dataHandle.asInt();
		
		if(value != lastCount)
		{//count hat sich nicht verueuendert, also outHandle sueueubern und raus hier
			meshDirty = true;
			lastCount = value;	
		}
		
	}
	else if( plug == IVspin )
	{
		int value = dataHandle.asInt();
/*
		MFnIntArrayData arrayData;
		MPlug optionPlug(thisMObject(), options);	//dieser wert kann gefahrlos geholt werden, da er das outmesh nicht beeinflusst
		
		MObject data;
		optionsPlug.getValue(data);
		arrayData.setObject(data);

		if(!(arrayData.array())[9])
		{//wenn civ nicht gesetzt ist, dann macht dieser Wert keinen sinn



		}
*/
		
		if(value != lastSpin)
		{
			meshDirty = true;
			lastSpin = value;
		}

	}

	
	// Sich jetzt um die STE internals kuemmern
	//
	//

	STE_CHECK_INTERNALS( IV,dataHandle );

	

	if(plug==IVinMesh)
		meshDirty = true; 
	
	return MPxNode::setInternalValue (plug, dataHandle);
}

//---------------------------------------------
void	IV_makeSelection(void* data)
//---------------------------------------------
{

	
	BPT_InsertVtx* nodePtr = (BPT_InsertVtx*) data;
	MEventMessage::removeCallback(nodePtr->eID);

	MDagPath meshPath;
	nodePtr->getMeshPath(meshPath);

	if( !(meshPath.apiType() == MFn::kInvalid) && nodePtr->validIndices.length() != 0)	//zur Sicherheit, sollte aber eigentlich nicht mueueglich sein
	{
		MFnSingleIndexedComponent compFn;
		
		MFn::Type type = MFn::kInvalid;
		if(nodePtr->validIndices[0] == 1)
			type = MFn::kMeshEdgeComponent;
		else if(nodePtr->validIndices[0] == 2)
			type = MFn::kMeshPolygonComponent;
		else if(nodePtr->validIndices[0] == 3)
			type = MFn::kMeshVertComponent;
		
		assert(type != MFn::kInvalid);
		//flag wieder entfernen
		nodePtr->validIndices.remove(0);

		MSelectionList compList, current;
		
		MObject comps = compFn.create(type);
		compFn.addElements(nodePtr->validIndices);
		
		compList.add(meshPath,comps);
		compList.add(nodePtr->thisMObject());
	

		

	//	MGlobal::getActiveSelectionList(current);
		
		MGlobal::setActiveSelectionList(compList, MGlobal::kAddToList);
	//	MGlobal::setActiveSelectionList(current,MGlobal::kAddToList);

	
	}
}

//----------------------------------------------------------------------------
MStatus		BPT_InsertVtx::doCompleteCompute( MDataBlock& data )
//----------------------------------------------------------------------------
{

				SPEED("Berechne EdgeSplit neu: ");

				MStatus status;

				MPRINT("MACHE KOMPLETTE BERECHNUNG")



				MDataHandle inMeshHandle = data.inputValue(IVinMesh);
				MDataHandle outMeshHandle = data.outputValue(IVoutMesh);


					//splitCount setzen
				
				MDataHandle countHandle	= data.inputValue(IVcount);			
				fIVfty.setCount(countHandle.asInt());
			
				

				MDataHandle spinHandle = data.inputValue(IVspin);

				fIVfty.setSpin(spinHandle.asInt());
				

				int initialVtxCount;	//wird spueueter benueuetigt, um das ValidIndicesArray gleich in der rictigen grueueueuee zu erstellen und zu schreiben


				//gleich zu beginn muss der MeshPath initialisiert werden, damit der MeshPath an die fty ueuebergeben werden kann
				// Dies geschieht besser durch die STE - sie ist darauf ausgelegt
				softTransformationEngine::gatherAttributeObjects(thisMObject());
				softTransformationEngine::saveMeshPathes();

				

				fIVfty.setMeshPath(meshPath);
				
				
			
				MDataHandle	rHandle = data.inputValue(IVslideRelative);
				fIVfty.setRelative(rHandle.asInt());
				
				
				MDataHandle nRelativeHandle = data.inputValue(IVnormalRelative);
				fIVfty.setNormalRelative(nRelativeHandle.asInt());
				
				
				//selection setzen
				MFnIntArrayData		intDataArray;	
				
				MDataHandle arrayHandle = data.inputValue(IVselEdgeIDs);
				intDataArray.setObject(arrayHandle.data());
				
				fIVfty.setEdgeIDs( intDataArray.array() );

				arrayHandle = data.inputValue(IVselVertIDs);
				intDataArray.setObject(arrayHandle.data());

				fIVfty.setVertIDs(intDataArray.array());
				

				//				optionen holen
				
				arrayHandle = data.inputValue(IVoptions);
				intDataArray.setObject(arrayHandle.data());
				MIntArray optionsArray(intDataArray.array());
				

				fIVfty.setOptions(optionsArray);
				
				
				
				
				MDataHandle slideHandle = data.inputValue(IVslide);
				fIVfty.setSlide(slideHandle.asDouble());

				
				//whichSide attribute wird nur fueuer SLide selbst verwendet und kann nicht bereits beim command gestetzt werden
				
				


				MObject inMeshRef = inMeshHandle.asMesh();
				fIVfty.setMesh(inMeshRef);


				MFnMesh meshFn(inMeshHandle.asMesh());
				initialVtxCount = meshFn.numVertices();
				

				

				//ACTION
				try
				{
					status = fIVfty.doIt();
				}
				
				catch(...)
				{
					MGlobal::displayError(" An unknown, severe, error occoured.\nIf it happens again in this situation, please write a bug report.\nPlease undo the operation and save your work!");
					return MS::kUnknownParameter;
				}

				
				MObject newOutMesh = fIVfty.getMesh();

				
				outMeshHandle.set(newOutMesh);

				
				// ---------------------
				// SOFT TRANSFORMATION
				// ---------------------
				// VtxSet setzen - hier reicht es, wenn er einfach die neuen Vtx nimmt


				softTransformationEngine::setVtxSet(data);

		

				//------------SELECTION ROUTINE----------------------


			
				//nur wenn sich spin nicht verueuendert hat, darf ne neue selection gemacht werden - dies wird auch von der IV berueuecksichtigt
				//die selection wird nur noch einmal ausgefueuehrt, weshalb scriptJobInitiated nicht mehr gesetzt wird vom scriptjob
				if( optionsArray[6] && !scriptJobInitated && !(meshPath.apiType() == MFn::kInvalid) )
				{
						
						//auf jeden Fall erstmal die neuen Vertizen holen, damit die anderen prozeduren auch darauf arbeiten kueuennen

						//alles neuen Vertces sollen gewueuehlt werden, also einfach alle Indices eintragen vom initialVtxCount
						//bis zum jetzigen VtxCount
						MIntArray validEdges, validFaces;
						componentConverter CC(newOutMesh);

						int i = 0;


						meshFn.setObject(newOutMesh);
						int newCount = meshFn.numVertices();
						
						validIndices.clear();
						validIndices.setLength(newCount - initialVtxCount);
						
						

						for(; initialVtxCount < newCount; initialVtxCount++)
							validIndices[i++] = initialVtxCount;
						
						

					
					if(optionsArray[6] == 1 || optionsArray[6] == 2) //select edges
					{
						
						CC.getContainedEdges(validIndices,validEdges);
					}


					BPT_Helpers helper;


					if(optionsArray[6] == 2) //select Faces
					{
						CC.getConnectedFaces(validEdges,validFaces);

						//jetzt kann gleich alles beendet werden, da hiernach keine componente mehr kommt, in die man faces umwandeln mueuesste
						validIndices.clear();
						validIndices.append(2);

						
						helper.addIntArrayToLHS(validIndices,validFaces);

					}

					if(optionsArray[6] == 1)
					{//edges fertigmachen
						
						validIndices.clear();
						validIndices.append(1);

						helper.addIntArrayToLHS(validIndices,validEdges);

					}
					else if(optionsArray[6] == 5)
						validIndices.insert(3,0);


					//component Mode umschalten bei bedarf
					if(optionsArray[5])
					{
						MSelectionMask::SelectionType type = MSelectionMask::kSelectMeshVerts;
						
						if(optionsArray[6] == 5)
						{
							type = MSelectionMask::kSelectMeshVerts;
						}
						else if(optionsArray[6] == 2)
						{
							type = MSelectionMask::kSelectMeshFaces;
						}
						else if(optionsArray[6] == 1)
						{
							type = MSelectionMask::kSelectMeshEdges;
						}
						
						
						MSelectionMask mask(type);
						
						
						MGlobal:: setComponentSelectionMask(mask);
					}


					eID = MEventMessage::addEventCallback("idle",IV_makeSelection,this);

					scriptJobInitated = true;

				}
				else
				{//ansonsten muss die SelectionList neu aufgebaut werden, allerdings ohne komponenten
					//diese Aktion solte auch nur einmal ausgefueuehrt werden

					//gegenwueuertige selection holen
					MSelectionList currentList;
					MSelectionList newList;
					MGlobal::getActiveSelectionList(currentList);

					//durch die Liste iterieren und Komponenten Filtern
					MItSelectionList selIter(currentList);
					MObject currentObj;
					for( ; !selIter.isDone();selIter.next() )
					{
						
						selIter.getDependNode(currentObj);
						
						newList.add(currentObj);
					}

					MGlobal::setActiveSelectionList(newList, MGlobal::kAddToList);


		


				}


				



				return status;

}



//----------------------------------------------------------------------------
MStatus		BPT_InsertVtx::compute(const MPlug& plug, MDataBlock& data)
//----------------------------------------------------------------------------
{
	
	
//	FactoryWerte setzen
//	(hier ueueberall eventuell noch MCheck nutzen fueuer Debug wenn nueuetig)
	MStatus status;
	
		MDataHandle stateHandle = data.outputValue(state);
		
		if(stateHandle.asShort() == 1)
		{
			MDataHandle inMeshHandle = data.inputValue(IVinMesh);
			MDataHandle outMeshHandle = data.outputValue(IVoutMesh);

//			inMesh direkt an outMesh und MObject mesh an factory geben
			outMeshHandle.set(inMeshHandle.asMesh());
			outMeshHandle.setClean();
		}
		else
		{
			if( (plug == IVoutMesh) )
			{

				if(meshDirty)
				{
					MPRINT("COMPLETE COMPUTE!!!!!!!!!!!!!!!!!!!!!!!!!!!")

					status = doCompleteCompute(data);

					INVIS(cout<<"MeshDirty ist "<<meshDirty<<endl;)
						meshDirty = false;
					INVIS(cout<<"--------------------------------"<<endl;)

						MFnDependencyNode depNodeFn(thisMObject());

					INVIS(cout<<"---------------"<<endl;)
						INVIS(cout<<depNodeFn.name().asChar()<<endl;)
						INVIS(cout<<"---------------"<<endl);

					
					// Jetzt die statischen Daten holen in der STE
					softTransformationEngine::extractStaticData(plug, data);

						
					if(status == MS::kUnknownParameter)
					{
						MDataHandle outMeshHandle = data.outputValue(IVoutMesh);
						outMeshHandle.setClean();
						return status;
					}
						//das problem ist, das mein Script bei bevelOPs nach der Operation noch die SelectionList verueuendert und der Callback so gleich wieder
						//ausgeschaltet wird ... alles scheisse!
				}

				//es wird generell nach der creation erstmal slide ausgefueuehrt, damit auch along normal und andere parameter stimmen nach ladeVorgang


				//jetzt Daten, die slide erzeugen, extrahieren und mit gespeicherten werten vergleichen.
				//wenn abweichung, wurde also nur einer dieser werte verueuendert und slide muss erfolgen


				MDataHandle slideHandle = data.inputValue(IVslide);
				MDataHandle	rHandle = data.inputValue(IVslideRelative);
				MDataHandle nRelativeHandle = data.inputValue(IVnormalRelative);
				MDataHandle sideHandle = data.inputValue(IVwhichSide);
				MDataHandle normalHandle = data.inputValue(IVnormal);




				MDataHandle outMeshHandle = data.outputValue(IVoutMesh);



				fIVfty.setNormal(normalHandle.asDouble());
				fIVfty.setRelative(rHandle.asInt());
				fIVfty.setNormalRelative(nRelativeHandle.asInt());
				fIVfty.setSlide(slideHandle.asDouble());
				fIVfty.setSide(sideHandle.asInt());





				//slide initialisieren
				//merke: Die SlidingEngine verursacht diesen komischen crash fehler!!!
				//testen: funzt es, wenn sie MObject wieder zurueueckgibt und die handle damit setzt?
				
				

				MObject outMeshObj = outMeshHandle.asMesh();
				fIVfty.doSlide(outMeshObj);

				
				
				// Jetzt noch tweaken
				softTransformationEngine::extractNonstaticData(plug, data);


				softTransformationEngine::doTweak(data);


				outMeshHandle.setClean();


				M3dView view = M3dView::active3dView();


				/*
				
				MFnMesh mymeshFn(meshPath);
				MObjectArray sets, comps;
				
				
				status = mymeshFn.getConnectedSetsAndMembers(0, sets, comps,true);
				status.perror("Prob bei Sets");

				status = mymeshFn.createUVSet(MString("myFirstNewUVSet"));
				status.perror(" Prob bei CreateUVSet");

				
				cout<<meshPath.fullPathName()<<endl;
				cout<<sets.length()<<" == Num Sets"<<endl;
				cout<<comps.length()<<" == Num comps"<<endl;

				//funzt nicht wuehrend DGEval
				*/


			}
			else
				return MS::kUnknownParameter;

		}

	
	return status;
}



//----------------------------------------------------------------------------
void*	BPT_InsertVtx::creator()
//----------------------------------------------------------------------------
{
	return new BPT_InsertVtx;
}


