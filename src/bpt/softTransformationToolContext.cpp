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

// softTransformationTool.cpp: implementation of the softTransformationTool class.
//
//////////////////////////////////////////////////////////////////////

#include "softTransformationToolContext.h"
#include "progressBar.h"




//******************************************************************************************************************
//*********************************
//MPX_SELECTION_CONTEXT METHODEN
//*********************************
//******************************************************************************************************************

//------------------------------------------------------------
softTransformationToolCtx::softTransformationToolCtx():	dgModFn(0),
														cmd(0),
														softConnector(0),
														animCreator(0),
														copyDagMod(0),
														hasManip(false),
														useLastDistance(false),
														fType(0),
														lastDistance(0.0),
														bbScaleFactor(0.0),
					/*zur sicherheit*/					fVis(true),
														nirFlag(false),
														emergency(false),
														vmCreator(0),
														vmConnector(0),
														fastTrgNodeCreator(0)
//------------------------------------------------------------
{
	lastSelection.clear();
	setTitleString("SoftTransformationTool");
}

//---------------------------
void	update(void* data);
MCallbackId	id1;
//---------------------------

//---------------------------
void	nodeAddedCB(MObject& node, void* data);
MCallbackId	id2;
//---------------------------

//---------------------------
void	nodeRemovedCB(MObject& node, void* data);
MCallbackId	id3;
//---------------------------

//---------------------------------------------
void	nodeRemovedCB(MObject& node, void* data)
//---------------------------------------------
{
	//ptr umwandeln
	softTransformationToolCtx * ctxPtr = (softTransformationToolCtx*) data;

	if(ctxPtr->lastMesh.node() == node)
	{
		MPRINT("WARNUNG - MESH geloescht")
		ctxPtr->emergency = true;
		MGlobal::executeCommand("setToolTo selectSuperContext", false, false);
	}



	INVIS(MFnDependencyNode depNodeFn(node);)
	MPRINT("------------------")
	MPRINT("Node has been REMOVED")
	INVIS(cout<<depNodeFn.name().asChar()<<endl;)
	INVIS(depNodeFn.setObject(ctxPtr->lastMesh.node());)
	INVIS(cout<<"lastMeshNode = "<<depNodeFn.name().asChar()<<endl;)
	MPRINT("------------------")
	
}

//---------------------------------------------
void	nodeAddedCB(MObject& node, void* data)
//---------------------------------------------
{
	MPRINT("------------------")
	MPRINT("Node has been added")
	INVIS(cout<<node.apiTypeStr()<<endl;)
	MPRINT("------------------")
	


	MFnDependencyNode depNodeFn(node);
	INVIS(cout<<depNodeFn.name().asChar()<<endl;)

	if( !(depNodeFn.name() == "softTransformationNode1") && !(depNodeFn.name() == "fastTrgNode1") && depNodeFn.findPlug("vtxWeights").isNull()
		&& !(node.apiType() == MFn::kAnimCurveUnitlessToUnitless) && 
		!(node.apiType() == MFn::kPluginManipContainer) && !(node.apiType() == MFn::kFreePointTriadManip) &&
		!(node.apiType() == MFn::kDiscManip) && !(node.apiType() == MFn::kStateManip) &&
		!(node.apiType() == MFn::kToggleManip) && !(node.apiType() == MFn::kMesh) && !(node.apiType() == MFn::kTransform) )
	{
		MGlobal::executeCommand("setToolTo selectSuperContext", false, false);
		
	}
	


	
}

//--------------------------------------------------------------------------
bool	softTransformationToolCtx::selectionsDiffer(MSelectionList& inList)
//--------------------------------------------------------------------------
{	

	//wenn list keine Comps hat, dann gar nicht erst weiter machen
	MDagPath path; MObject comps;
	inList.getDagPath(0,path, comps);


	//man darf die meshes während des tools nicht wechseln
	if( !(lastMesh.apiType() == MFn::kInvalid) )
	{
		if( !(lastMesh == path) )
			return false;
	}


	if(comps.apiType() == MFn::kInvalid)
	{
		lastMesh = MDagPath();
		MPRINT("Comps waren INVALID")
		return false;
	}

	//nur meshes werden bearbeitet
	if(path.apiType() != MFn::kMesh)
		return false;

	
#ifdef DEMO

	MFnMesh	meshFn(path);
	if(meshFn.numPolygons() > 500)
	{
		MGlobal::displayError("This DEMO will only work on meshes with a maximum number of 500 polygons");
		return false;
	}

#endif

	//wenn path kein mesh, dann auch raus
	INVIS(cout<<"Obj Type: "<<path.node().apiTypeStr()<<endl;)

	//list ist mindestens 1 wenna hier herkommt
	if(inList.length() != lastSelection.length() || lastSelection.length() == 0)
		return true;
	
	//also ist es doch noetig, die Componenten zu vergleichen:

	//jeweils nur die erste auswahl testen
	MDagPath listPath, lastPath;
	MObject	listComps,lastComps;

	inList.getDagPath(0,listPath,listComps);
	lastSelection.getDagPath(0,lastPath,lastComps);

	if( !(listPath == lastPath) )
		return true;

	//jetzt über die componentenCounts gehen
	MFnSingleIndexedComponent	listCompFn(listComps);
	MFnSingleIndexedComponent	lastCompFn(lastComps);

	INVIS(cout<<listCompFn.elementCount()<<" = elementCount"<<endl;)
	if(listCompFn.elementCount() != lastCompFn.elementCount())
		return true;

	//jetzt match machen
	if(listCompFn.isEqual(lastComps))
		return false;
	else
		return true;

	//forbidden PAth
	return false;

}

//--------------------------------------------------------------------------
void softTransformationToolCtx::toolOnSetup(MEvent &)
//--------------------------------------------------------------------------
{
    setHelpString("Select mesh components and use the manipulator to move them. CTRL + LMB changes distance, CTRL + LMB + RMB changes the mask.");
	
	emergency = false;

	cmd = 0;

	// Jetzt die eigenen Globals davon informieren, dass wir im Tool sind
 //	BGlobals::setContextMode(true); // Maya 6.5 quick fix


	//dafür sorgen, dass lastSelection nicht null ist
//	MGlobal::getActiveSelectionList(lastSelection);
	
	//channelbox muss ausgeschaltet werden, damit der user nicht auf die idee kommen kann, 
	//die softTransformationNode anzuklicken, was einen absturz verursacht wegen mayainternen probs
	MGlobal::executeCommand(MString("channelBox -e -m false $gChannelBoxName"),false,false);
/*
	MSelectionList empty;
	MSelectionList hilite;
	MGlobal::getHiliteList(hilite);


	if(hilite.length() != 0)
		MGlobal::setActiveSelectionList(empty);
*/


	// FastTrgNode erzeugen, wenn sie nicht existiert
	//
	MSelectionList empty;

	empty.add("fastTrgNode1");

	if( empty.length() == 0 )
	{
		if( fastTrgNodeCreator == 0 )
		{
			fastTrgNodeCreator = new MDGModifier();
		}

		// Muss also fastTrgNode erzeugen
		//
		MObject	fTrgNode = fastTrgNodeCreator->createNode(0x00108BC4);
		
		// Jetzt den Namen ändern
		//
		fastTrgNodeCreator->renameNode( fTrgNode, "fastTrgNode1" );

		fastTrgNodeCreator->doIt();
	}


	update(this);	//update checkt auf gültige selections und connected und schreibt daten in softNode

	MStatus status;
	
	id1 = MModelMessage::addCallback(MModelMessage::kActiveListModified,
									 update, 
									 this, &status);

	id2 = MDGMessage::addNodeAddedCallback(nodeAddedCB);
	
	id3 = MDGMessage::addNodeRemovedCallback(nodeRemovedCB,kDefaultNodeType,this, &status);
	
	
}

//------------------------------------------------------------
void softTransformationToolCtx::toolOffCleanup()
//------------------------------------------------------------
{
    MStatus status;
	MModelMessage::removeCallback(id1);
	
	MMessage::removeCallback(id2);

	MMessage::removeCallback(id3);

	MPRINT("bin in toolOffCleanup")
	//softTransformNode loeschen




	// Jetzt die eigenen Globals davon informieren, dass wir das Tool verlassen
	//BGlobals::setContextMode(false);// Maya 6.5 quick fix





	MGlobal::executeCommand(MString("channelBox -e -m true $gChannelBoxName"),false,false);

	if(!useLastDistance)
		lastDistance = 0.0;

	if(useLastDistance && !(lastMesh.apiType() == MFn::kInvalid) && !emergency)
	{
		MFnDependencyNode depFn(findSoftNode());
		depFn.findPlug("dis").getValue(lastDistance);
		depFn.findPlug("fv").getValue(fVis);
	}

	deleteManipulators();


	//softConnector wird von diesen Proceduren zurückgesetzt
	if(hasHistory)
	{
		if(cmd != 0 && !emergency)
			writeBackTweaks();
	}
	else
		if(cmd != 0 && !emergency)
			generateUndoInfo();


	//dupMesh darf erst nach connectSoftNode undo geloescht werden
	//maya stürzt ab, wenn diese node geloescht wird. Dies betrifft wahrscheinlich die in PolyModifier 
	//betreffende limitation  wegen duplicate und fehlendem builtInUndo
/*	if(dupMeshDagPath.apiType() != MFn::kInvalid)
	{
		MDGModifier dgMod;
		dgMod.deleteNode(dupMeshDagPath.node());
		dgMod.doIt();
	}
*/
	//visualizeMeshNode zurücksetzen
	careAboutVNode(true);
	
	
	if(copyDagMod != 0)
	{
		copyDagMod->undoIt();
		delete copyDagMod;
		copyDagMod = 0;
	}

	//damit die änderungen auch nach entfernter node und history wirksam werden muss tweak erzeugt werden
	if(animCreator != 0 && !emergency)
	{
		animCreator->undoIt();
		delete animCreator;
		animCreator = 0;
	}
	else
	{
		delete animCreator;
		animCreator = 0;
	}
	
	
	if(dgModFn != 0 && !emergency)
	{
		dgModFn->undoIt();
		delete dgModFn;
		dgModFn = 0;
	}
	else
	{
		delete dgModFn;
		dgModFn = 0;
	}


	
	lastSelection.clear();

	//resetten der UserTrg
	if(!(lastMesh.apiType() == MFn::kInvalid) && !emergency)
	{
		MFnDependencyNode depNodeFn(lastMesh.node());
		depNodeFn.findPlug("userTrg").setValue("");
	}
	lastMesh = MDagPath();
	
	
	//jetzt die originalVtxColors zurückschreiben
/*
	if(!cmdFinalized)
	{
		if(cmd != 0)
		{
			cmd->finalize();
			cmdFinalized = true;
		}
	}
*/

	// Jetzt die FastTrgNode wieder loeschen
	//
	if( fastTrgNodeCreator )
	{
		fastTrgNodeCreator->undoIt();
		delete fastTrgNodeCreator;
		fastTrgNodeCreator = 0;
	}

	hasManip = false;

	MPRINT("ToolOffCleanup abgeschlossen");

	cmd = 0;

	MPxContext::toolOffCleanup();
}

//------------------------------------------------------------
bool	softTransformationToolCtx::noSGAssigned()
//------------------------------------------------------------
{
	//geht davon aus, das LastMesh gültig ist (dies muss also vorher sichergestellt werden)
	MFnMesh fnMesh(lastMesh);

	MObjectArray shaders;
	MIntArray PPAs;	//per polygon assignments


	fnMesh.getConnectedShaders(0,shaders,PPAs);

	if(shaders.length() == 0)
		return true;
	else
		return false;

	//forbidden
	return false;
}


//--------------------------------------------------------------
void	softTransformationToolCtx::careAboutVNode(bool deleteIt)
//--------------------------------------------------------------
{
	
	//jetzt checken, ob visualizationMesh erzeugt werden muss
	//Muss un immer ezeugt werden - VtxColors sind outOfDate :) - OpenGlRules
	if( ( !deleteIt && !(lastMesh.apiType() == MFn::kInvalid) ) )
	{
		if(vmCreator == 0)
		{
			vmCreator = new MDagModifier();
			MObject locatorShape = vmCreator->createNode("visualizeMeshNode", lastMesh.transform() );
			
			vmCreator->doIt();

			vmConnector = new MDGModifier();
			//jetzt outMesh min inMesh connecten und outWeights mit in weights
			MFnDagNode dagNodeFn(locatorShape);

			locatorPath = MDagPath();

			dagNodeFn.getPath(locatorPath);
			INVIS(cout<<locatorPath.fullPathName().asChar()<<endl;)


			INVIS(cout<<dagNodeFn.object().apiTypeStr()<<" = APITYPE"<<endl;)

			MPlug locInMesh = dagNodeFn.findPlug("is");
			MPlug locInWeights = dagNodeFn.findPlug("vw");
			MPlug locFVis = dagNodeFn.findPlug("en");

			


			MFnDependencyNode depNodeFn(lastMesh.node());
			MPlug softOutMesh = depNodeFn.findPlug("outMesh");

			depNodeFn.setObject(findSoftNode());
			INVIS(cout<<softOutMesh.name().asChar()<<" = outMeshAttr"<<endl;)

			MPlug softOutWeights = depNodeFn.findPlug("ow");
			MPlug softFVis = depNodeFn.findPlug("fv");
			

			//plugs verbinden
			vmConnector->connect(softOutMesh,locInMesh);
			vmConnector->connect(softOutWeights, locInWeights);
			vmConnector->connect(softFVis, locFVis);


			vmConnector->doIt();
			
			

		}
	}
	else
	{//locator wieder loeschen wenn noetig
		if(vmCreator != 0)
		{
			vmConnector->undoIt();
			delete vmConnector;
			vmConnector = 0;

			vmCreator->undoIt();
			delete vmCreator;
			vmCreator = 0;
		}
	}


}

//------------------------------------------------------------
MStatus softTransformationToolCtx::doPress(MEvent &event)
//------------------------------------------------------------
{
	MStatus stat;
//	if(!event.isModifierControl())	//so ist es moeglich, noch mit singleClick abzuwählen
		stat = MPxSelectionContext::doPress(event);
   
	
	MPRINT("Bin in DoPress")

    // If we are not in selecting mode (i.e. an object has been selected)
    // then set up for the translation.
    
    event.getPosition(startPos_x, startPos_y);
    view = M3dView::active3dView();
	
	MGlobal::getActiveSelectionList(lastSelection);
	

	if (!isSelecting() || event.isModifierControl()) 
	{
		if( !(lastMesh.apiType() == MFn::kInvalid) )
		{
			calculateScaleFactor();
			//im grunde muss der toolCmdNur finalize ausführen
			MObject	softNode = findSoftNode();
			MFnDependencyNode	depNodeFn(softNode);

			depNodeFn.findPlug("distance").getValue(initialDist);
			depNodeFn.findPlug("maskSize").getValue(initialMask);

			// Ausserdem noch die worldspace ausmasse des screens finden
			// TODO - noch richtig machen
			//
			MPoint pWorld1, pWorld2;
			MVector vec;
			view.viewToWorld(0, 0, pWorld1, vec );
			view.viewToWorld( view.portWidth(), view.portHeight() , pWorld2, vec );
			maxScreenDistance = pWorld1.distanceTo(pWorld2);
		}
        

    }

	
	careAboutVNode();

	view.beginOverlayDrawing();
    return stat;
}

//------------------------------------------------------------
MStatus softTransformationToolCtx::doDrag(MEvent & event)
//------------------------------------------------------------
{
    MStatus stat;

	if(!event.isModifierControl())
		stat = MPxSelectionContext::doDrag(event);

    // If we are not in selecting mode (i.e. an object has been selected)
    // then do the translation.
    //
    if (!isSelecting() || event.isModifierControl()) {
        event.getPosition(endPos_x, endPos_y);
        
		if( !(lastMesh.apiType() == MFn::kInvalid) )
		{
			MObject	softNode = findSoftNode();
			MFnDependencyNode	depNodeFn(softNode);
			
			//hier noch alles relativ zur bounding box machen
			//am besten ganz auf MMB verzichten, da der den Manip beeinflusst
			if(event.mouseButton() == MEvent::kLeftMouse)
			{
				//hier eventuell noch weitere ModifierStates verarbeiten
				
				if(event.isModifierMiddleMouseButton())
				{
					int value;
					
					if(isOrthoCam)
					{
						int v_w = view.portWidth();
						value = initialMask + (double((endPos_x - startPos_x)) / double(v_w) ) * 5;
					}
					else
					{
						value = initialMask + ((endPos_x - startPos_x) /*+ sqrt(pow(endPos_y - startPos_y,2))*/ * bbScaleFactor * 3);
					}

					depNodeFn.findPlug("maskSize").setValue( value );
					
					MString str = "Mask Size: ";
					if(value < 0)
						str += 0;
					else
						str += value;
					
					setHelpString(str);
					
				}
				else
				{
					double value;
					
					if(isOrthoCam)
					{
						int v_w = view.portWidth();
						value = initialDist + (double((endPos_x - startPos_x)) / double(v_w) );
					}
					else
					{
						//value = initialDist + ((endPos_x - startPos_x) /*+ sqrt(pow(endPos_y - startPos_y,2))*/ * bbScaleFactor / 4);
						value = initialDist + ( (float)(endPos_x - startPos_x) / (float)view.portWidth() ) * maxScreenDistance * 20;
					}

					depNodeFn.findPlug("distance").setValue( value );
					
					MString str = "Distance: ";
					if(value < 0.0)
						str += 0.0;
					else
						str += value;

					setHelpString(str);
					
				}
				
			}
			
		}
		INVIS(cout<<"ScreenPos: "<<endPos_x<<" - "<<endPos_y<<endl;)

      
		  view.refresh(false, false);
    }

//	view.clearOverlayPlane();

	// Es hat also funktioniert, nun mal was zeichnen
//	GLUquadricObj *qobj = gluNewQuadric();
//	gluQuadricDrawStyle(qobj, GLU_FILL);
//	gluCylinder( qobj, endPos_x / 20, endPos_y / 20, 20, 8, 1 );

	//SwapBuffers( view.deviceContext() );



    return stat;
}

//------------------------------------------------------------
MStatus softTransformationToolCtx::doRelease(MEvent &event)
//------------------------------------------------------------
{
	MStatus stat;
	if(!event.isModifierControl())
		 stat = MPxSelectionContext::doRelease(event);

    if (!isSelecting() || event.isModifierControl()) 
	{
        event.getPosition(endPos_x, endPos_y);


		
		//das loeschen des commands übernimmt die undoQueue
    }
/*
	if(cmd != 0 && !cmdFinalized)
	{
		stat = cmd->finalize();
		cmdFinalized = true;
	}
*/

	setHelpString(MString("Select mesh components and use the manipulator to move them. Hit [+]/[-] to change the distance or [Ctrl] + [+]/[-] to change MaskSize."));

	view.endOverlayDrawing();


	view.refresh(true);
    return stat;
}


//------------------------------------------------------------
MStatus softTransformationToolCtx::doEnterRegion(MEvent &event)
//------------------------------------------------------------
{
    return setHelpString(MString("Select mesh components and use the manipulator to move them. Hit [+]/[-] to change the distance or [Ctrl] + [+]/[-] to change MaskSize."));
}

//----------------------------------------------------------
void	softTransformationToolCtx::calculateScaleFactor()
//----------------------------------------------------------
{
/*
	MFnDagNode	dagFn(lastMesh.transform());

	//für den fall das der hier noch nicht existiert da doPress nicht gerufen wurde
	view = M3dView::active3dView();

	MBoundingBox	box = dagFn.boundingBox();

	
	bbScaleFactor = (box.max() - box.min()).length() / (view.portWidth() + view.portHeight()) / 4;

	//dafür sorgen, dass hochskalierte objekte ordentlich erscheinen

	//kann passieren bei planes
	if(bbScaleFactor < 0.00001)
		bbScaleFactor = 1;	

	*/

	MPoint screenCenter;
	MVector centerVec;

	view = M3dView::active3dView();
	
	short pw = view.portWidth(),ph = view.portHeight();
	

	view.viewToWorld(short(pw / 2), short(ph / 2),screenCenter, centerVec);
	
	bbScaleFactor = (manipPos - screenCenter).length() * (1.0/(float(pw) / float(ph))) / 1000;


	//checken, cam orthogonal, und wenn ja dann flag setzen um distance berechnungen anzupassen
	MDagPath cameraPath;
	view.getCamera(cameraPath);

	MFnCamera camFn(cameraPath);

	isOrthoCam = camFn.isOrtho();
		
}

//----------------------------------------------------------
MObject	softTransformationToolCtx::findSoftNode()
//-------------------------------------------------
{
	//diese Procedur koennte versagen, wenn mesh keine history hat!
	MFnDependencyNode	depNodeFn(lastMesh.node());

	
	MPlugArray	connections;
	depNodeFn.findPlug("inMesh").connectedTo(connections,true,false);
	
	
	return connections[0].node();
	
}


//--------------------------------------------------------------
bool	softTransformationToolCtx::changeDistance(double value)
//--------------------------------------------------------------
{
	if(!(lastMesh.apiType() == MFn::kInvalid))
	{

		MFnDependencyNode depNodeFn(findSoftNode());
		
		MPlug distPlug = depNodeFn.findPlug("dis");
		
		if(bbScaleFactor == 0.0)
			calculateScaleFactor();

		double currentValue;
		distPlug.getValue(currentValue);
		initialDist = currentValue;


		double finalValue;
		
		if(isOrthoCam)
		{
			
			int v_w = view.portWidth();
			finalValue = currentValue + (double(value) / double(v_w) ) * bbScaleFactor * 10;
		}
		else
		{
			finalValue = currentValue + value * bbScaleFactor;
		}
		distPlug.setValue(finalValue);

		MString str = "Distance: ";
		
		if(finalValue < 0.0)
			str += 0.0;
		else
			str += finalValue;
		
		setHelpString(str);
	}
	else
		return false;

	return true;
}

//--------------------------------------------------------------
bool	softTransformationToolCtx::changeMask(int value)
//--------------------------------------------------------------
{
	if(!(lastMesh.apiType() == MFn::kInvalid))
	{
		MFnDependencyNode depNodeFn(findSoftNode());
		
		MPlug distPlug = depNodeFn.findPlug("ms");
		
		int currentValue;
		distPlug.getValue(currentValue);
		
		initialMask = currentValue;
		

		int finalValue = currentValue + value;
		

		distPlug.setValue(finalValue);

		MString str = "Mask Size: ";
		if(finalValue < 0)
			str += 0;
		else
			str += finalValue;

		setHelpString(str);
	}
	else
		return false;

	return true;
}



//--------------------------------------------------------------------------------------
void	softTransformationToolCtx::connectSoftNode(MObject& softNode,MObject& meshNode)
//---------------------------------------------------------------------------------------
{
/*1.wenn meshNode history hat, dann die softNode in History einfügen

	history->outMesh ----> inMesh<- softNode ->outMesh ->inMesh<-MeshNode
	tweaks sind bei dieser Variante egal

2.	wenn mesh keine history hat und keine tweaks:
	outMesh MeshNode cachen und in inMesh der SoftNode schreiben, und wenn fertig gecachdes Mesh wieder in 
		inMesh MeshNode schreiben

3.	wenn keine history und Tweaks:
	outMesh cachen, tweaks entfernen, outmesh wieder in inMesh softNode schreiben, und wenn fertig, gecachdes Mesh wieder in inMesh
	Meshnode schreiben

	MERKE: Am ende einer jede Node operation werden die OutPnts der SoftNode in die Tweaks der meshnode geschrieben
	
*/

	MFnDependencyNode	depNodeFn(meshNode);
	

	MPlug	inMeshPlug = depNodeFn.findPlug("inMesh");

	if(inMeshPlug.isConnected())
	{//hat History
		softConnector = new MDGModifier();
		
		MPRINT("Mesh hat history")
		MPlugArray connections;

		inMeshPlug.connectedTo(connections,true, false);

		//disconnection
		softConnector->disconnect(connections[0],inMeshPlug);
		
		depNodeFn.setObject(softNode);
		//connection zur softNode
		softConnector->connect(connections[0],depNodeFn.findPlug("inMesh"));

		//soft mit meshNode verbinden
		softConnector->connect(depNodeFn.findPlug("outMesh"), inMeshPlug);

		softConnector->doIt();
		
		hasHistory = true;
	}
	else
	{//hat keine History
		
		MPlug dupOutMesh;

		if(dupMeshDagPath.apiType() == MFn::kInvalid)
		{//dupMeshShape noch nicht vorhanden, also erstellen
			
			//meshShape erzeugen, outMesh connecten zu inMesh copy, dgEval forcieren, connection aufheben
			//und dann weiter wie zuvor
			copyDagMod = new MDagModifier();
			MObject dupMeshNode = copyDagMod->createNode("mesh");
			copyDagMod->doIt();
			
			MFnDagNode dagNodeFn(dupMeshNode);
			dagNodeFn.setObject(dagNodeFn.child(0));
			dagNodeFn.setIntermediateObject(true);

			//meshNode outMesh in inMesh copy schreiben
			MDagPath::getAPathTo( dupMeshNode, dupMeshDagPath );
			MFnDependencyNode dupDepFn(dupMeshDagPath.child(0));
			
			MDGModifier tempMod;
			tempMod.connect(depNodeFn.findPlug("outMesh"),dupDepFn.findPlug("inMesh"));
			tempMod.doIt();

			//DGeval forcieren
			MString cmd = "dgeval -src ";
			cmd += dupDepFn.name();
			cmd += ".outMesh";
			MGlobal::executeCommand(cmd,false,false);
			
			tempMod.undoIt();
				

			dupOutMesh = dupDepFn.findPlug("outMesh");


			MPlug tweaks = depNodeFn.findPlug("pnts");
			//if(tweaks.numElements() > 0 && tweaksDontStay)
			//cout<<"DEPFn Name: "<<depNodeFn.name()<<endl;
			if(tweaks.numElements() > 0 )
			{//tweaks sind vorhanden
				//tweaks loeschen
				//cout<<"CONNECTSOFTNODES: Will Tweaks zurücksetzen"<<endl;
				
				MFnNumericData numDataFn;
				MObject data = numDataFn.create(MFnNumericData::k3Float);
				
				int l = tweaks.numElements();
				for(int i = 0; i < l; i++)
				{
					tweaks.elementByPhysicalIndex(i).setValue(data);
				}
				
			}

			
		}
		else
		{//dupMeshShape existiert bereits, also outMesh in inMesh der DupNode speisen
			MFnDependencyNode dupDepFn(dupMeshDagPath.child(0));
			
			dupOutMesh = dupDepFn.findPlug("outMesh");
		}
		
		


		
		softConnector = new MDGModifier();

		MFnDependencyNode	softFn(softNode);

		softConnector->connect(dupOutMesh,softFn.findPlug("inMesh"));

		softConnector->connect(softFn.findPlug("outMesh"), inMeshPlug);

		softConnector->doIt();

		//Check ob tweaks vorhanden oder nicht

		hasHistory = false;

	}
	


}

//-----------------------------------------------------------------------------------------
bool	softTransformationToolCtx::getVtxSelection(MIntArray& elements, MPoint& point,MDagPath& selObj, MObject& comps)
//-----------------------------------------------------------------------------------------
{

	if(comps.apiType() == MFn::kMeshVertComponent)
	{
		MPRINT("hole mittelpunkt (VERTEX)")
		
		MFnSingleIndexedComponent	compFn(comps);
		
		compFn.getElements(elements);
		
		INVIS(cout<<"Zahl elemente: "<<elements.length()<<endl;)
		
		
		MItMeshVertex	vertIter(selObj,comps);
		
		for(; !(vertIter.isDone()); vertIter.next())
			point += vertIter.position(MSpace::kWorld);
		
		point = point / elements.length();

		return false;
	}
	else if(comps.apiType() == MFn::kMeshEdgeComponent)
	{
		MPRINT("EdgeAuswahl")
		
		MItMeshEdge	edgeIter(selObj,comps);

		
		for(;!(edgeIter.isDone());edgeIter.next())
		{
			point += edgeIter.center(MSpace::kWorld);
			
			elements.append(edgeIter.index(0));
			elements.append(edgeIter.index(1));
		}

		point = point / edgeIter.count();

		return false;
		
	}
	else if(comps.apiType() == MFn::kMeshPolygonComponent)
	{
		MItMeshPolygon	polyIter(selObj,comps);

		MPRINT("PolyAuswahl")
		
		
		MIntArray tmpArray;
		for(;!(polyIter.isDone());polyIter.next())
		{
			point += polyIter.center(MSpace::kWorld);

			polyIter.getVertices(tmpArray);

			for(unsigned int i = 0; i < tmpArray.length(); i++)
				elements.append(tmpArray[i]);
		}

		point = point / polyIter.count();

		return false;
	}
	
	

	return true;


}

//------------------------------------------------------
bool	softTransformationToolCtx::writeBackTweaks()
//------------------------------------------------------
{
//hier müssen jetzt noch die Tweaks ins mesh geschrieben werden
	if(lastMesh.apiType() == MFn::kInvalid)
		return false;
	
	
	MObject softNode = findSoftNode();
	
	if(softNode.apiType() != MFn::kPluginDependNode)
		return false;

	
	MPRINT("Schreibe Tweaks in Mesh")


	MFnDependencyNode depNodeFn(softNode);
	
	
	MPlug vPlug = depNodeFn.findPlug("outVectors");
	MObject arrayData;
	vPlug.getValue(arrayData);

	//jetzt noch schnell die aktualisierten VtxColors durchspülen
	double scale;
	depNodeFn.findPlug("scale").getValue(scale);
	depNodeFn.findPlug("scale").setValue(scale + 0.000000001);
	MFnDependencyNode surfaceDepFn(lastMesh.node());
	MObject outM;
	surfaceDepFn.findPlug("outMesh").getValue(outM);


	MFnVectorArrayData	vectorFn(arrayData);
	MVectorArray vectors = vectorFn.array();
	
	INVIS(cout<<"VectorsLänge = "<<vectors.length()<<endl;)
	//jetzt sind tweaks in der softNode vorhanden und koennen ins mesh geschrieben werden
	depNodeFn.setObject(lastMesh.node());
	MPlug	tweaks = depNodeFn.findPlug("pnts");
	
	
	MFnNumericData	numData;
	MPlug pnt;

	uint l = vectors.length();
	
	
	//progressBar
	progressBar	progress("Writing Tweak Data", l);




	uint count = 0;
	for(uint i = 0; i < l; i++)
	{
		if(vectors[i].length() != 0.0)
		{						
			MVector vec = vectors[i] ;
			
			pnt = tweaks.elementByLogicalIndex(i);
			MObject origData;
			pnt.getValue(origData);
			
			float one,two,three;
			numData.setObject(origData);
			numData.getData(one,two, three);
			
			MObject data = numData.create(MFnNumericData::k3Float);
			numData.setData(float(vec.x) + one,float(vec.y) + two,float(vec.z) + three);
			
			pnt.setValue(data);

					
			cmd->undoTweaks.append(vec);
			cmd->indexArray.append(i);
			
			
			if(count++%50 == 0)
				progress.set(count);
		}
	}
	
	
	
	cmd->meshPath = lastMesh;

	
	if(!cmdFinalized) //dieser check ist eigentlich nicht noetig, aber sicher ist sicher!
	{
		cmd->finalize();
		cmdFinalized = true;
	}
	
		
	//wenn er hier ist, muss schon ein durchgang stattgefunden haben und softConnector ist definiert
	//aber nur lschen, wenn history vorhanden(-> er ist nicht 0)
	if(softConnector != 0)
	{
		softConnector->undoIt();
		delete softConnector;
		softConnector = 0;
	}
	
	MPRINT("WriteBackTweaks beendet")
	
	return true;

}


//------------------------------------------------------
bool	softTransformationToolCtx::generateUndoInfo()
//------------------------------------------------------
{
	if(lastMesh.apiType() == MFn::kInvalid)
	return false;
	
	
	MObject softNode = findSoftNode();
	
	if(softNode.apiType() != MFn::kPluginDependNode)
		return false;

	
	MPRINT("Generiere Undo Info")



	MFnDependencyNode depNodeFn(softNode);
	
	
	MPlug vPlug = depNodeFn.findPlug("outVectors");
	MObject arrayData;
	vPlug.getValue(arrayData);

	
	//jetzt die geupdateten Corlors durchspülen(noetig bei meshes ohne history
	double scale;
	depNodeFn.findPlug("scale").getValue(scale);
	depNodeFn.findPlug("scale").setValue(scale + 0.000000001);
	MFnDependencyNode surfaceDepFn(lastMesh.node());
	MObject outM;
	surfaceDepFn.findPlug("outMesh").getValue(outM);
	


	MFnVectorArrayData	vectorFn(arrayData);
	MVectorArray vectors = vectorFn.array();
	
	INVIS(cout<<"VectorsLänge = "<<vectors.length()<<endl;)
	//jetzt sind tweaks in der softNode vorhanden und koennen ins mesh geschrieben werden

	
	
	MFnNumericData	numData;
	
	int l = vectors.length();
	
	

	for(int i = 0; i < l; i++)
	{
		if(vectors[i].length() != 0.0)
		{							
			cmd->undoTweaks.append(vectors[i]);
			cmd->indexArray.append(i);	
		}
	}
	
	MPRINT("FERTIG")
		
		
		cmd->meshPath = lastMesh;

	if(!cmdFinalized) //dieser check ist eigentlich nicht noetig, aber sicher ist sicher!
	{
		cmd->finalize();
		cmdFinalized = true;
	}
	
	//wenn er hier ist, muss schon ein durchgang stattgefunden haben und softConnector ist definiert
	//aber nur lschen, wenn history vorhanden(-> er ist nicht 0)
	if(softConnector != 0)
	{ 
		softConnector->undoIt();
		delete softConnector;
		softConnector = 0;
	}
	
				
	return true;

}

//-----------------------------------------------------------------------------------------
void update(void * data)
//------------------------------------------------------------
{

	INVIS(cout<<"-----------------------------"<<endl;)
	INVIS(cout<<"UPDATE"<<endl;)
	INVIS(cout<<"-----------------------------"<<endl;)
	
	//Schulen
#ifdef SCHOOL
		cout<<endl;
		cout<<"---------------------------------------------------------------"<<endl;
		cout<<"This license of BPT is for educational purposes only, reselling"<<endl
			<<"and usage in comercial projects is prohibited!"<<endl;
		
		cout<<"This license is locally restricted to the campus of the this school and may not be used outside this area"<<endl;
		cout<<"If you have been sold this program, please report this to: "<<endl;
		cout<<"Sebastian.Thiel@byronimo.de"<<endl;
#endif





	MStatus stat = MStatus::kSuccess;

	MSelectionList list;
    stat = MGlobal::getActiveSelectionList(list);
    MItSelectionList iter(list, MFn::kInvalid, &stat);

	softTransformationToolCtx * ctxPtr = (softTransformationToolCtx*) data;
	
	INVIS(cout<<list.length()<<" und "<<ctxPtr->lastSelection.length()<<endl;)
	if(list.length() != 0 && ctxPtr->selectionsDiffer(list) )
	{
		
		MPRINT("Bin in Update!!!")
		
		if(ctxPtr->hasManip)
		{
			MPRINT("Loesche Manipulators!!!")
			ctxPtr->deleteManipulators();
			ctxPtr->hasManip = false;
		}

		ctxPtr->lastSelection = list;

		

		//noch lastDistance holen (dies muss vor writeBackTweaks geschehen, da sonst findSoftNode versagt
		if(ctxPtr->useLastDistance && !(ctxPtr->lastMesh.apiType() == MFn::kInvalid))
		{
			MFnDependencyNode depFn(ctxPtr->findSoftNode());
			depFn.findPlug("dis").getValue(ctxPtr->lastDistance);
			depFn.findPlug("fv").getValue(ctxPtr->fVis);
			depFn.findPlug("ft").getValue(ctxPtr->fType);
		}


		//zuerst die VNode loeschen wenn moeglich
		ctxPtr->careAboutVNode(true);

		//diese proc kümmert sich um undo von softConnector
		ctxPtr->writeBackTweaks();
		


		if(ctxPtr->animCreator != 0)
		{
			ctxPtr->animCreator->undoIt();
			delete ctxPtr->animCreator;
			ctxPtr->animCreator = 0;
		}
		
		
		if(ctxPtr->dgModFn != 0)
		{	MPRINT("Will dgModFn rückgängig machen")
			ctxPtr->dgModFn->undoIt();
			delete	ctxPtr->dgModFn;
		//	ctxPtr->dgModFn = 0;
		}
		

				
		MObject		comps;
		MDagPath	selObj;
		
		int count = 0;
		
		MPRINT("Hole node Daten")

		for (; !(iter.isDone()); iter.next()) 
		{
			//nur fürs erste gewählte object manip erstellen
			if(count > 0)
				break;
			
			
			
			//if(!(iter.hasComponents()))	//wird in selectionsDiffer gecheckt
			//	break;

			
			
			//jedesMal nen neuen dagFn erstellen, weil sich die interne undoQueue leider nicht resetten lässt
			
			ctxPtr->dgModFn = new MDGModifier();
			MPRINT("Habe neuen Modifier erstellt")
			//neue softNode erstellen
			//	MObject	softNode2;
			
			
			//cout<<ctxPtr->softNode.apiTypeStr()<<" == softNode"<<endl;
			MObject localSoftNode;
			localSoftNode = ctxPtr->dgModFn->createNode("softTransformationNode");
			MPRINT("Habe neue softNode erstellt")
			
			INVIS(cout<<localSoftNode.apiTypeStr()<<" == softNode"<<endl;)
			ctxPtr->dgModFn->doIt();

		

			// iterate through the selected objects:
			// 
			
			
			
			iter.getDagPath(selObj,comps);

			ctxPtr->cmd = (softTransformationToolCmd *) ctxPtr->myNewToolCmd();
			ctxPtr->cmdFinalized = false;

			//jetzt den gemittelten Punkt der Komponenten herausfinden
			//erstmal nur für vertizen
			MPoint	point(0.0,0.0,0.0);
			MIntArray	elements;
			
			//COMPONENTEN HOLEN
			ctxPtr->getVtxSelection(elements,point,selObj,comps);
			ctxPtr->manipPos = point;
			//für den Fall, dass die DoPressMethode nicht gerufen wird
			ctxPtr->calculateScaleFactor();
			
			ctxPtr->lastMesh = selObj;
			//softNode Punkte setzen: start und end auf selben wert
			MFnNumericData	numDataFn;
			MObject numData = numDataFn.create(MFnNumericData::k3Double);
			numDataFn.setData(point.x,point.y,point.z);
			
			MFnDependencyNode	depNodeFn(localSoftNode);


			//jetzt AnimCurve erstellen
			ctxPtr->animCreator = new MDGModifier();
			ctxPtr->animFn.create(localSoftNode,depNodeFn.attribute("cf"),MFnAnimCurve::kAnimCurveUU,ctxPtr->animCreator);
			

			depNodeFn.findPlug("startPoint").setValue(numData);
			depNodeFn.findPlug("endPoint").setValue(numData);
		//	depNodeFn.findPlug("anEndPoint").setValue(numData);
		//	depNodeFn.findPlug("sEndPoint").setValue(numData);
			
			if(ctxPtr->nirFlag)
				depNodeFn.findPlug("nr").setValue(true);

			//distance und falloffInitialisierungen
			if(ctxPtr->useLastDistance)
			{
				MPRINT("Habe lastDistance gesetzt")
				depNodeFn.findPlug("dis").setValue(ctxPtr->lastDistance);
			}

			INVIS(cout<<"VIS!!!!: "<<ctxPtr->fVis<<endl;)
			if(ctxPtr->fVis)
			{
				depNodeFn.findPlug("fv").setValue(ctxPtr->fVis);
			}
			depNodeFn.findPlug("ft").setValue(ctxPtr->fType);
			

			MObject	meshNode = selObj.node();
			//userTrg umstellen
			depNodeFn.setObject(meshNode);

			MString userTrgStr;
			depNodeFn.findPlug("userTrg").getValue(userTrgStr);
			if(userTrgStr == "")
				depNodeFn.findPlug("userTrg").setValue("fastTrg");

			depNodeFn.setObject(localSoftNode);
			selObj.pop();	//transform finden
			
			//jetzt die inclusiveInverseMatrix in softNode schreiben
			MFnMatrixData	matrixFn;
			//so funzt es auch wenn Mesh teil einer hirarchie

			MObject tmpObj = matrixFn.create(selObj.inclusiveMatrixInverse());
			depNodeFn.findPlug("inverseMatrix").setValue(tmpObj);

			
			
			//jetzt noch die elements in node schreiben
			MFnIntArrayData	arrayData;
			MObject	arrayObj = arrayData.create();
			arrayData.set(elements);
			depNodeFn.findPlug("inVertices").setValue(arrayObj);
			
			//jetzt die SoftNode verbinden
			ctxPtr->connectSoftNode(localSoftNode,meshNode);

			//manip erstellen
			INVIS(cout<<"ComponentType: "<<comps.apiTypeStr()<<endl;)
			
			MObject manipObject;
			ctxPtr->moveM =
				(moveManip *) moveManip::newManipulator("moveManip", 
				manipObject);
			
			ctxPtr->addManipulator(manipObject);
			ctxPtr->hasManip = true;
			
			
			MPRINT("Will mit softNode verbinden")
			ctxPtr->moveM->connectToDependNode(localSoftNode);
			MPRINT("Habs geschafft")
			//und jetzt die componenten extrahieren, und deren gemittelte position holen
			
			//letztendlich checken, ob VNode erstellt werden muss
			ctxPtr->careAboutVNode();
			
			count++;
			
		}
	}
//	else if(list.length() == 0)
//	{
//		ctxPtr->deleteManipulators();
//		ctxPtr->lastSelection.clear();
//	}

	INVIS(cout<<"------------------------------------"<<endl;)
	INVIS(cout<<"END UPDATE END UPDATE ENDUPDATE END"<<endl;)
	INVIS(cout<<"------------------------------------"<<endl;)
	
    
}

