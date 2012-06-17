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
// File: ByronsPolyToolsInit.cpp
//
// Author: Sebastian Thiel
//

#define BPT_VERSION "1.0 RV"

#include "ByronsPolyToolsCmd.h"
#include "ByronsPolyToolsNode.h"
#include "BPT_insertVtxNode.h"
#include "softTransformationNode.h"
#include "visualizeMeshNode.h"

#include "softTransformationToolContextCmd.h"
#include "softTransformationToolCmd.h"

#include "fastTrgNode.h"
#include "BPT_BA.h"


#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnIntArrayData.h>



#include <maya/MFnPlugin.h>

#include "mayabaselib/base.h"


//-----------------------------------------------------------------
ByronsPolyTools::ByronsPolyTools():	chamferFlagSet(false), 
									solidChamferFlagSet(false)
//-----------------------------------------------------------------
{}


//-----------------------------------------------------------------
ByronsPolyTools::~ByronsPolyTools()
//-----------------------------------------------------------------
{}


//-----------------------------------------------------------------
void*		ByronsPolyTools::creator()
//-----------------------------------------------------------------
{
	return new ByronsPolyTools();
}


//-----------------------------------------------------------------
bool		ByronsPolyTools::isUndoable() const
//-----------------------------------------------------------------
{
	return	true;
}

//-----------------------------------------------------------------
void ByronsPolyTools::getEdgeComponents(MSelectionList &sList)
//-----------------------------------------------------------------
{
	MObject edgeComps;
	MItSelectionList	selIter(sList, MFn::kMeshEdgeComponent);
	MDagPath path;


	UINT r = 0;
	for(;!selIter.isDone(); selIter.next())
	{
		selIter.getDagPath(path, edgeComps);
		
		MItMeshEdge			edgeIter(path, edgeComps);
		
		edgeIDs.setLength(edgeIDs.length() + edgeIter.count());
		
		
		for(;!edgeIter.isDone(); edgeIter.next())
			edgeIDs[r++] = edgeIter.index();
	}
	
}

//-----------------------------------------------------------------
void ByronsPolyTools::convertToEdges(MSelectionList& sList,bool doNotConvertVertices)
//-----------------------------------------------------------------
{
	//Faces werden in contained edges umgewandelt. Darurch wird bewirkt, dass dass der Algo in noSLide modus wechselt, da die Richtungs-
	//bestimmung verhindert wird
	edgeIDs.setSizeIncrement(100);


	MObject polyComps;
	MItSelectionList	selIter(sList, MFn::kMeshPolygonComponent);
	MDagPath path;

	for(; !selIter.isDone(); selIter.next())
	{
		selIter.getDagPath(path, polyComps);
		
		
		MItMeshPolygon polyIter(path,polyComps);
		for(;!polyIter.isDone(); polyIter.next())
		{
			MIntArray faceEdges;
			polyIter.getEdges(faceEdges);
			
			unsigned int i = edgeIDs.length(),			r = 0;
			unsigned int l = i + faceEdges.length();
			
			edgeIDs.setLength(l);
			
			for(;i < l; i++)
				edgeIDs[i] = faceEdges[r++];
		}
	}

	//standardmässig die gweählten komponenten mit einbeziehen
	////wenn -peb aktiv ist, sollen die Vertizen behalten werden wie sie sind, da sie in connect mit einbezogen werden
	/*
	if(! doNotConvertVertices )
	{
		MObject vertComps;
		selIter.setFilter(MFn::kMeshVertComponent);
		selIter.getDagPath(path,vertComps);
		//jetzt die contained edges der vertices holen
		MItMeshVertex vertIter(path,vertComps);
		
		if(vertIter.count())
		{
			ULONG count = vertIter.count();
			MIntArray conEdges;
			
			UINT indexValue;
			
			MFnMesh meshFn(meshDagPath);
			BPT_BA LUT(meshFn.numEdges());
			
			for(; !vertIter.isDone(); vertIter.next())
			{
				vertIter.getConnectedEdges(conEdges);
				
				for(UINT i = 0; i < conEdges.length(); i++)
				{
					indexValue = conEdges[i];
					if( LUT[indexValue] )
						edgeIDs.append(indexValue);
					else
						LUT.setBitTrue(indexValue);
				}
				
			}
			
		}
	}
	else
	{//sogar selbst die vertIds holen
*/
		getVertComponents(sList);
		
//	}

	//edgeIDs müssen nicht gepruned werden, da sie später sowieso in ein BA eingetragen werden, was eh einem Prune entspricht

}


//-----------------------------------------------------------------
void ByronsPolyTools::getVertComponents(MSelectionList &sList)
//-----------------------------------------------------------------
{
	MObject vertComps;
	MItSelectionList	selIter(sList, MFn::kMeshVertComponent);
	MDagPath path;

	UINT r = 0;
	for(; !selIter.isDone(); selIter.next())
	{
		selIter.getDagPath(path, vertComps);
		
		MItMeshVertex		vertIter(path, vertComps);
		
		vertIDs.setLength(vertIDs.length() + vertIter.count());
		
		
		
		for(;!vertIter.isDone();vertIter.next())
			vertIDs[r++] = vertIter.index();
		
	}
}


//-----------------------------------------------------------------
void ByronsPolyTools::getPolyComponents(MSelectionList &sList)
//-----------------------------------------------------------------
{
	MObject polyComps;
	MItSelectionList	selIter(sList, MFn::kMeshPolygonComponent);
	MDagPath path;

	selIter.getDagPath(path, polyComps);

	MItMeshPolygon		polyIter(path, polyComps);

	polyIDs.setLength(polyIter.count());

	UINT r = 0;

	for(;!polyIter.isDone();polyIter.next())
		polyIDs[r++] = polyIter.index();


}

//-----------------------------------------------------------------
void	ByronsPolyTools::getModifierUINodeName(MString& newName)
//-----------------------------------------------------------------
{
	if(operationMode == 0)
	{//es ist IVNode

		//wenn civ flag gesetzt, dann isses ne BPT connect
		if(options[9])
			newName = "BPT_ConnectEdges";
		else //es ist also ein insert Vtx
			newName = "BPT_InsertVtx";

	//	if(options
			


	}
	else if(operationMode == 1)
	{//es ist BPTNode
		
		//wenn vertizen gewählt im smartSplitMode sind, dann isses eine ConnectVtxNode
		if(vertIDs.length() != 0 && CMDactionMode == 0)
			newName = "BPT_ConnectVtx";

		else if(CMDactionMode == 0)
			newName = "BPT_SmartSplit";
		//wenn es SolidChamfer ist
		else if(CMDactionMode == 5 )
			newName = "BPT_SolidChamfer";
		//chamfer
		else if(CMDactionMode == 4)
			newName = "BPT_Chamfer";
		//SMC
		else if(CMDactionMode == 6)
			newName = "BPT_SMC";

	}
	//else if( und wer weiss was noch alles kommt ;)

}

//-----------------------------------------------------------------
MStatus	ByronsPolyTools::setActionMode()
//-----------------------------------------------------------------
{

	if (smartSplitFlagSet + edgeLoopFlagSet + edgeRingFlagSet + boundaryFlagSet + chamferFlagSet + solidChamferFlagSet + smcFlagSet + growFlagSet + schrinkFlagSet > 1)
	{
		displayError(" Set only ONE Flag please.");
		return MS::kFailure;
	}

//	-1 == errorCode
//
	CMDactionMode = -1;

	if(smartSplitFlagSet)
		CMDactionMode = 0;

	else if (edgeLoopFlagSet)
		CMDactionMode = 1;

	else if (edgeRingFlagSet)
		CMDactionMode = 2;

	else if (boundaryFlagSet)
		CMDactionMode = 3;
	
	else if (chamferFlagSet)
		CMDactionMode = 4;
	
	else if (solidChamferFlagSet)
		CMDactionMode = 5;
	
	else if (smcFlagSet)
	{
		slideIsRelative = true;
		CMDactionMode = 6;
	}
	else if (growFlagSet)
		CMDactionMode = 7;
	
	else if (schrinkFlagSet)
		CMDactionMode = 8;
//	wenn kein Flag, dann failure
//
	if(CMDactionMode == -1)
	{
		displayError( "Please select ONE Flag (smartSplit OR edgeLoop OR edgeRing OR boundary OR chamfer OR solidChamfer OR smartMoveComponent OR growSelection OR shrinkSelection)" );
		return MS::kFailure;
	}
	else
		return MS::kSuccess;
}


//-----------------------------------------------------------------
MSyntax		ByronsPolyTools::newSyntax()
//-----------------------------------------------------------------
{
	MSyntax	syntax;

//	Flags hinzufügen

	//-----------------------------------
	//ACTION MODE FLAGS
	//-----------------------------------

	syntax.addFlag("-sp","-smartSplit",MSyntax::kNoArg);
	syntax.addFlag("-el","-edgeLoop",MSyntax::kNoArg);
	syntax.addFlag("-er","-edgeRing",MSyntax::kNoArg);
	syntax.addFlag("-bnd","-boundary",MSyntax::kNoArg);
	syntax.addFlag("-gro","-growSelection",MSyntax::kNoArg);
	syntax.addFlag("-shr","-shrinkSelection",MSyntax::kNoArg);
	syntax.addFlag("-smc","-smartMoveComponent",MSyntax::kNoArg);

	//-----------------------------------
	//Bevel + OPTIONS
	//-----------------------------------
	syntax.addFlag("-c","-chamfer",MSyntax::kNoArg);
	syntax.addFlag("-sc","-solidChamfer",MSyntax::kNoArg);
	syntax.addFlag("-ce","-connectEnds",MSyntax::kNoArg);
	syntax.addFlag("-te","-triangulateEnds",MSyntax::kNoArg);

	//-----------------------------------
	//INITIAL PARAMETERS
	//-----------------------------------
	syntax.addFlag("-sl","-slide", MSyntax::kDouble);
	syntax.addFlag("-snl","-slideNotLimited", MSyntax::kNoArg);
	syntax.addFlag("-sir","-slideIsRelative",MSyntax::kNoArg);
	syntax.addFlag("-nir","-normalIsRelative",MSyntax::kNoArg);
	syntax.addFlag("-ast","-autoSwitchComponentType", MSyntax::kNoArg);

	syntax.addFlag("-mec","-maxEdgeCount",MSyntax::kLong);
	syntax.addFlag("-h","-help",MSyntax::kNoArg);

	syntax.addFlag("-avf","-alwaysVisualizeFalloff",MSyntax::kNoArg);
	syntax.addFlag("-avm","-alwaysVisualizeOriginalMesh",MSyntax::kNoArg);
	

	syntax.addFlag("-se","-selectEdges",MSyntax::kNoArg);
	syntax.addFlag("-sf","-selectFaces",MSyntax::kNoArg);
	syntax.addFlag("-oe","-outerEdges",MSyntax::kNoArg);
	syntax.addFlag("-sv","-selectVertices",MSyntax::kNoArg);
	

	//-----------------------------------
	//InsertVtx Stuff
	//-----------------------------------
	syntax.addFlag("-iv","-insertVertices",MSyntax::kNoArg);
	syntax.addFlag("-civ","-connectInsertedVertices",MSyntax::kNoArg);

	syntax.addFlag("-ic","-initialCount",MSyntax::kLong);
	syntax.addFlag("-peb","-preferEdgeBased",MSyntax::kNoArg);


// DEFINE BEHAVIOUR OF COMMAND ARGUMENT THAT SPECIFIES THE MESH NODE:

   syntax.useSelectionAsDefault(true);

   syntax.setObjectType(MSyntax::kSelectionList, 0, 300);



// MAKE COMMAND NON-QUERYABLE AND NON-EDITABLE:

   syntax.enableQuery(false);

   syntax.enableEdit(false);

   return syntax;

}


//-----------------------------------------------------------------
MStatus ByronsPolyTools::initModifierNode(MObject modifierNode)
//-----------------------------------------------------------------
{
	MStatus		status;
	MFnDependencyNode	FnDepNode( modifierNode );
	MObject objTmp;


	switch(operationMode)
	{

	case 1:
		{
			//	Nun sämtliche Werte der durch PolyModifyCmd ertellten Node setzen
			//	inMesh/outMesh connections werden bereits vom Cmd gesetzt 


			MObject	attrTmp;


			attrTmp = FnDepNode.attribute("mode", &status);
			MCheckStatus(status,"Konnte Plug (Mode) nicht finden!!!");

			MPlug	PlugMode(modifierNode,attrTmp);
			PlugMode.setValue(CMDactionMode);

			//	schneller über .findPlug attribute setzen



			//	selections in Node schreiben

			//-------------------------------------------------------------

			//	MPlug	plugIDs = FnDepNode.findPlug("vertIDs");

			attrTmp = FnDepNode.attribute("vertIDs", &status);
			MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");
			MPlug	plugIDs(modifierNode,attrTmp);

			if(plugIDs.isNull())
				cout<<"Plug VertID war ungueltig"<<endl;

			MFnIntArrayData		intDataArray;
			
			
			objTmp = intDataArray.create(vertIDs);
			plugIDs.setValue(objTmp);
	
			//-------------------------------------------------------------
			
			plugIDs = FnDepNode.findPlug("edgeIDs",&status);
			MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");

			if(plugIDs.isNull())
				cout<<"Plug VertID war ungültig"<<endl;
	
	
			objTmp = intDataArray.create(edgeIDs);
			plugIDs.setValue(objTmp);

			//-------------------------------------------------------------	

			plugIDs = FnDepNode.findPlug("faceIDs",&status);
			MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");

			if(plugIDs.isNull())
				cout<<"Plug VertID war ungültig"<<endl;
			
			
			
			objTmp = intDataArray.create(polyIDs);
			plugIDs.setValue(objTmp);

			//-------------------------------------------------------------
			//	Optionen in Node schreiebn

			plugIDs = FnDepNode.findPlug("options",&status);
			MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");
			
			
			objTmp = intDataArray.create(options);
			plugIDs.setValue(objTmp);


			//-------------------------------------------------------------
			//	Slide in Node schreiben, jetzt da options gesetzt sind

			attrTmp = FnDepNode.attribute("slide", &status);
			MPlug	PlugSlide(modifierNode,attrTmp);



			PlugSlide.setValue(directSlide);



			PlugSlide = FnDepNode.findPlug("slideRelative");
			PlugSlide.setValue(slideIsRelative);


			MPlug nrPlug = FnDepNode.findPlug("normalRelative");
			nrPlug.setValue(normalIsRelative);

			//-------------------------------------------------------------


			break;
		}

		case 0:
			{

				//------------------------------
				//EDGE IDS
				//------------------------------

				MPlug plugIDs;
				MFnIntArrayData intDataArray;

				plugIDs = FnDepNode.findPlug("edgeIDs",&status);
				MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");


				objTmp = intDataArray.create(edgeIDs);
				plugIDs.setValue(objTmp);

				//------------------------------
				//Vert IDS
				//------------------------------
				
				plugIDs = FnDepNode.findPlug("vertIDs",&status);
				
				
				objTmp = intDataArray.create(vertIDs);
				plugIDs.setValue(objTmp);
								
				//------------------------------
				//options
				//------------------------------


				plugIDs = FnDepNode.findPlug("options",&status);
				MCheckStatus(status,"Konnte Plug (Slide) nicht finden!!!");
				
				objTmp = intDataArray.create(options);
				plugIDs.setValue(objTmp);

				//------------------------------
				//directSlide
				//------------------------------

				MObject attrTmp = FnDepNode.attribute("slide", &status);
				MPlug	PlugSlide(modifierNode,attrTmp);



				PlugSlide.setValue(directSlide);

				//------------------------------
				//slideRelative
				//------------------------------

				PlugSlide = FnDepNode.findPlug("slideRelative");
				PlugSlide.setValue(slideIsRelative);


				//------------------------------
				//normalRelative
				//------------------------------


				MPlug nrPlug = FnDepNode.findPlug("normalRelative");
				nrPlug.setValue(normalIsRelative);


				nrPlug = FnDepNode.findPlug("count");
				nrPlug.setValue((int)initialCount);

			}

	}


	// Jede Node enthlt das slideLimted attribut, welches jetzt direkt in die  Node gesetzt wird
	//
	FnDepNode.findPlug("sll").setValue( !options[4] );
	

	// Alle nodes die erzeugt werden haben eine STE - also deren Parameter setzen
	MPlug rPlug;
				
				
	rPlug = FnDepNode.findPlug("showWeights");
	rPlug.setValue(avt);
				
				
	int result;	//hält die optionVar
	MPlug slideFPlug = FnDepNode.findPlug("falloffType");
				
	//wert holen
	MGlobal::executeCommand("optionVar -q BPT_slfi", result, false, false);
			
	slideFPlug.setValue(result);
				

	bptNode = modifierNode;
	return status;
}


//-----------------------------------------------------------------
MStatus ByronsPolyTools::directModifier(MObject mesh)
//-----------------------------------------------------------------
{
	MStatus		status;
//	die Factory initialisieren
//
	fBPTfty.setMesh(mesh);

	fBPTfty.setActionMode(CMDactionMode);


	fBPTfty.setVertIDs(vertIDs);
	fBPTfty.setEdgeIDs(edgeIDs);
	fBPTfty.setPolyIDs(polyIDs);

	fBPTfty.setOptions(options);



	fBPTfty.setMeshPath(meshDagPath);

	status = fBPTfty.doIt();

	
	return status;
}


//-----------------------------------------------------------------
EXPORT MStatus initializePlugin( MObject obj )
//-----------------------------------------------------------------

{ 
	// init UI
	MGlobal::executeCommand("$BPTedgeCount[0] = -1;if ( !(`menu -exists mainBPTMenu`) ){ string $test; $test = `whatIs \"ByronsPolyToolsMenu\"`; if($test != \"Presumed Mel procedure; no definition seen yet.\")ByronsPolyToolsMenu;}",false,false);

	cout<<endl;
	cout<<"Byron's Poly Tools v" << BPT_VERSION << " initialized"<<endl;




	MStatus   status;


	MFnPlugin plugin( obj, "Sebastian Thiel", BPT_VERSION );

	
	//#####################################
	// WICHTIG
	// Erstmal die globals initialisieren
	//#####################################


	// ***************
	// ** BPTCMD ****
	// ***************
	status = plugin.registerCommand( "ByronsPolyTools", ByronsPolyTools::creator, ByronsPolyTools::newSyntax );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}


	status = plugin.registerNode(	"visualizeMeshNode", 
									visualizeMeshNode::id, 
									&visualizeMeshNode::creator, 
									&visualizeMeshNode::initialize,
									MPxNode::kLocatorNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	// ******************************
	// ** softTransformationNode ****
	// ******************************
	status = plugin.registerNode(	"softTransformationNode", 
									softTransformationNode::id, 
									&softTransformationNode::creator, 
									&softTransformationNode::initialize,
									MPxNode::kDependNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}


	// ****************
	// ** BPTNODE  ****
	// ****************
	status = plugin.registerNode( "ByronsPolyToolsNode",
								  ByronsPolyToolsNode::id,
								  &ByronsPolyToolsNode::creator,
								  &ByronsPolyToolsNode::initialize,
								  MPxNode::kDependNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	
	// *****************
	// ** INSERTVTX ****
	// *****************
	status = plugin.registerNode( "BPT_InsertVtx",
								   BPT_InsertVtx::IVid,
								  &BPT_InsertVtx::creator,
								  &BPT_InsertVtx::initialize,
								   MPxNode::kDependNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	

	// **********
	// ** STT ****
	// **********
   status = plugin.registerContextCommand("softTransformationTool",
											&softTransformationToolCtxCommand::creator,
										   "softTransformationToolCmd", 
										   &softTransformationToolCmd::creator);
    if (!status) {
        status.perror("registerContextCommand");
        return status;
    }

		
	
	// ****************
	// ** STT MANIP ****
	// ****************

	status = plugin.registerNode("moveManip", 
								 moveManip::id, 
								 &moveManip::creator, 
								 &moveManip::initialize,
								 MPxNode::kManipContainer);
    if (!status) {
        status.perror("registerNode");
        return status;
	}
	

	// ********************
	// ** FAST TRG NODE ****
	// ********************

	status = plugin.registerNode(		"fastTrgNode",
                 						fastTrgNode::id,
                 						fastTrgNode::creator,
                 						fastTrgNode::initialize);


	if (!status) {
        status.perror("registerTriangulation");
        return status;
	}
	
	//tools erzeugen, aber nur, wenn die session noch nicht abgelaufen ist
	//
	MGlobal::executeCommand("softTransformationTool softTransformationTool1",false,false);
	

	return MS::kSuccess;
}


//-----------------------------------------------------------------
EXPORT MStatus uninitializePlugin( MObject obj )
//-----------------------------------------------------------------
{
	MStatus   status;
	MFnPlugin plugin( obj );


	//#####################################
	// WICHTIG
	// Erstmal die globals deinitialisieren
	//#####################################


	status = plugin.deregisterCommand( "ByronsPolyTools" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}


	status = plugin.deregisterNode( visualizeMeshNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	
	status = plugin.deregisterNode( ByronsPolyToolsNode::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterNode( BPT_InsertVtx::IVid );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	
	

	status = plugin.deregisterNode(moveManip::id);
    if (!status) {
        status.perror("deregisterNode");
        return status;
    }
	

	MStatus stat = plugin.deregisterNode(fastTrgNode::id);

	status = plugin.deregisterContextCommand("softTransformationTool", "softTransformationToolCmd");
    if (!status) {
        status.perror("deregisterContextCommand");
        return status;
    }
	

	
	/*
	status = plugin.deregisterNode(ByronsPolyToolsNodeManip::id);
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	*/



	//Menue wegmachen
	MGlobal::executeCommand("if ( (`menu -exists mainBPTMenu`) ) deleteUI mainBPTMenu;",false,false);

	return status;
}
