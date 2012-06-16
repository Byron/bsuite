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

#include "ByronsPolyToolsCmd.h"
#include "ByronsPolyToolsNode.h"
#include "BPT_insertVtxNode.h"


// Function Sets
//
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>

// Iterators
//
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>





//TEST
//Einfach mal die WndPoc, die fuers messageHandling verantwortlich ist, mit einer eigenen ersetzen
//dies ermglicht das abfangen smtlicher Commandos
/*
LRESULT CALLBACK myWinProc(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LONG    lRet = 0; 
	//cout<<"Bin in WinProc"<<endl;
	HWND mayaWin = M3dView::applicationShell();
	
	lRet = CallWindowProc(	(WNDPROC)mayaWndProc, 
						hWnd, uMsg,wParam, lParam);

    //lRet = DefWindowProc (hWnd, uMsg, wParam, lParam); 

    return lRet;										// Return by default
}


HWND mayaWin = M3dView::applicationShell();
	
const long mayaWndProc = SetWindowLong(mayaWin, GWL_WNDPROC, (long)&myWinProc);
*/


//-----------------------------------------------------------------
MStatus		ByronsPolyTools::doIt(const MArgList& args)
//-----------------------------------------------------------------
{

	GLuint textureID;
	glGenTextures( 1, &textureID);

	// Ist die version abgelaufen ?
	//
#ifdef EXPIRES

	if( ! checkExpires() )
	{
		MGlobal::displayError( "This AlphaVersion is expired. Scenes will still load properly. " );
		
		return MS::kSuccess;
	}

#endif

	MSelectionList sList;


	MStatus	status;
	MArgDatabase	argData(syntax(),args);


	
	if(argData.isFlagSet("-help"))
	{
		MGlobal::executeCommand("showBPTHelpWindow",false,false);														
		return MS::kSuccess;
	}

	

//SELECTION UEBERPRUEFEN

//	Selection holen
//
	argData.getObjects(sList);
	
//	OrigList fuer UndoZwecke speichern	
	origList = sList;






//	Check, ob ueberhaupt Objekt gewaehlt

   if ( (sList.length() == 0) )

   {

      displayError("ONE mesh or its transform node must be selected.");

      return MStatus::kFailure;

   }
   


	//MDagPath			meshDagPath;
	
	MItSelectionList sIter(sList);//,MFn::kMesh);
	bool meshOK = false,hasComps = false;

	INVIS(cout<<"SELECTIONLIST LAENGE: "<<sList.length()<<endl);

	for(;!sIter.isDone();sIter.next())
	{
		sIter.getDagPath(meshDagPath, components);
		//sList.getDagPath(0, meshDagPath, components);
	
	





		
		//if(!meshDagPath.hasFn(MFn::kPluginDependNode))
		if(meshDagPath.apiType() == (MFn::kMesh))
		{
			
			if(!meshDagPath.hasFn(MFn::kMesh) )
				meshOK = false;
			else
				meshOK = true;
			
			
			
			
			
			if( components.apiType() == MFn::kInvalid )
				hasComps = false;
			else
			{
				hasComps = true;
				break;
			}
			
		}


	}


	if(!meshOK)
	{
		displayError("Invalid type!  Only a mesh or its transform with selected components can be specified!");
		return MStatus::kFailure;
	}



#ifdef	DEMO
	
	MFnMesh	meshFn(meshDagPath);
	if(meshFn.numPolygons() > 500)
	{
		MGlobal::displayError("This DEMO will only work on meshes with a maximum number of 500 polygons");
		return MS::kFailure;
	}

#endif



	if(!hasComps)
	{
		displayError("Please select some components to operate on");
		return MStatus::kFailure;
	}



//	Flags und Argumente Holen
//----------------------------------

	if(argData.isFlagSet("-iv"))
		operationMode = 0;
	else
		operationMode = 1;

	
	//zuerst mal edgeComoponents holen - wenn welche vorhanden und -peb flag gesetzt, dann wird auf jeden Fall der case 0 mode verwendet
	getEdgeComponents(sList);
	
	if( argData.isFlagSet("-peb") && edgeIDs.length() != 0 )
		operationMode = 0;


//------------------------------------
//	OPTIONS
//------------------------------------
	

	switch(operationMode)
	{
	case 0://insert Vtx mode
		{
			if(argData.isFlagSet("-ic"))
				argData.getFlagArgument("-ic",0,initialCount);
			else
				initialCount = 1;
		
			goto default_values;
			
		}

	case 1:	//PolyToolsMode
		{

			
			//------------------------------------
			//	FLAGS
			//------------------------------------








			smartSplitFlagSet = argData.isFlagSet("-smartSplit");
			edgeLoopFlagSet = argData.isFlagSet("-edgeLoop");
			edgeRingFlagSet = argData.isFlagSet("-edgeRing");
			boundaryFlagSet = argData.isFlagSet("-boundary");

			chamferFlagSet = argData.isFlagSet("-chamfer");
			solidChamferFlagSet = argData.isFlagSet("-solidChamfer");

			smcFlagSet = argData.isFlagSet("-smartMoveComponent");

			growFlagSet = argData.isFlagSet("-gro");
			schrinkFlagSet = argData.isFlagSet("-shr");



			//	abbruch, wenn flags nicht eindeutig und actionMode nicht gesetzt werden kann
			status = setActionMode();

			if(status == MS::kFailure)
				return status;


			//-------------------ENDE-----------------------------


			goto default_values;

			
		}
	default:
		{
default_values:


			

			avt = (argData.isFlagSet("-avf"))?true:false;
			avm = (argData.isFlagSet("-avm"))?true:false;


			options.setLength(10);

			options[0] = argData.isFlagSet("-connectEnds");

			options[1] = argData.isFlagSet("-triangulateEnds");

			if(argData.isFlagSet("-maxEdgeCount"))
				argData.getFlagArgument("-maxEdgeCount",0,options[2]);
			else
				options[2] = 16666666;

			options[3] = 0;	//side ist standardmaessig 0:


			if(argData.isFlagSet("-snl") )
				options[4] = 1;
			else
				options[4] = 0;


			options[5] = (argData.isFlagSet("-ast"))?1:0;

			options[6] = 0;
			if(argData.isFlagSet("-se"))
				options[6] = 1;

			if( CMDactionMode == 5 && argData.isFlagSet("-") )
				options[6] += 3;

			if(argData.isFlagSet("-sf"))
				options[6] = 2;

			if(argData.isFlagSet("-sv"))
				options[6] = 5;


			//IV flag - wenn -peb aktiv ist, wird er auf jeden Fall gesetzt
			if( argData.isFlagSet("-peb") )
				options[9] = 1;
			else
				options[9] = argData.isFlagSet("-civ")?true:false;

			//----------------------------------------OPTIONS ENDE
	
			slideIsRelative = argData.isFlagSet("-sir");

			//slideIsRelative ist an, wenn Chamfer aktiv ist
			if(chamferFlagSet+solidChamferFlagSet > 0)
				slideIsRelative = 1;

			if(CMDactionMode == 6)
				slideIsRelative = true;

			normalIsRelative = argData.isFlagSet("-nir");

			//slide soll automatisch auf  gesetzt werden, wenn -peb aktiv (dies entspricht einem slidewert von 1 
			if( argData.isFlagSet("-peb") && edgeIDs.length() != 0)
				directSlide = 1;	
			else 
			{
				if(argData.isFlagSet("-slide"))
					
					argData.getFlagArgument("-slide",0,directSlide);
				else
					directSlide = 0.5;	//defaultValue
			}

		}


	}

	//undoIndo sammeln

	MSelectionMask tmpMask(MGlobal::componentSelectionMask());
	undoMask = tmpMask;



	/*
		CallWindowProc(	(WNDPROC)(GetWindowLong(mayaWin, GWL_WNDPROC)), 
						mayaWin, WM_CLOSE, NULL, NULL);
	

	
	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//MessageBox(mayaWin, "Help", "Whatever", MB_OK);
	HWND myWin = CreateWindowEx( 0,"MyFirstWin","Lala" , dwStyle, 0, 0,
								300, 400, 
								mayaWin, NULL, NULL, NULL);

	ShowWindow(myWin, SW_SHOWNORMAL);
	SetFocus(myWin);	
	UpdateWindow(myWin); 
	*/

	//checken ob makeHistory aus ist. Wenn ja, dann wieder an machen, weil maya sonst crashed
	int result;
	MGlobal::executeCommand( "constructionHistory -q -tgl", result );
	
	if(!result)
		MGlobal::executeCommand( "constructionHistory -tgl 1",false,false);




	meshDagPath.extendToShape();
//	Mesh von polyModifierCommand setzen
	setMeshNode(meshDagPath);
	


	switch(operationMode)
	{
	case 0:
		{
			//getEdgeComponents(sList);
			convertToEdges(sList, argData.isFlagSet("-peb") );
				
			// setCreateAnimCurves(false);//damit nicht versucht wird, auf dieser Node animCurves fuer tweak zu erstellen
			// Jetzt sind tweaks vonnten

			setModifierNodeType( BPT_InsertVtx::IVid );
			break;
		}
	case 1:
		{
			//	Komponenten herausfiltern und IntIDArrays schreiben

			//getEdgeComponents(sList);
			getVertComponents(sList);
			getPolyComponents(sList);

			setModifierNodeType( ByronsPolyToolsNode::id );
			break;
		}
	}

//	Aktion ausfuehren

	if(CMDactionMode == 0 || CMDactionMode == 4 || CMDactionMode == 5 || CMDactionMode == 6 || operationMode == 0)
	{

		MGlobal::executeCommand("setToolTo selectSuperContext",false,false);

		status = doModifyPoly();
		
		MFnDependencyNode depNodeFn(bptNode);
		setResult(depNodeFn.name());

		if( !(status == MS::kSuccess) )
		{
			MGlobal::displayError( "An error occoured." );
			MGlobal::displayError( status.errorString() );
			
		}
	}
	else
	{
//		Wenn mesh nicht veraendert wird, wird einfach so die Factory gerufen
		directModifier(meshDagPath.node());
		//setResult( "BPTOperation succeeded" );
	}


	//Command wird nur einmal verwendet, also muessen die Datenarrays auch m#keinen Speicher mehr verschwenden
	//.clear() wuerde sie nicht physisch loeschen
	polyIDs.setLength(0);
	edgeIDs.setLength(0);
	vertIDs.setLength(0);

//	return status;	
	return MS::kSuccess;	
}


//-----------------------------------------------------------------
MStatus		ByronsPolyTools::redoIt()
//-----------------------------------------------------------------


{
	MStatus status;

	// Process the polyModifierCmd
	//
	if(CMDactionMode == 0 || CMDactionMode == 4 || CMDactionMode == 5 || CMDactionMode == 6 || operationMode == 0)
	{
		MPRINT("Mache BPT redo")

		status = redoModifyPoly();
		
//		MSelectionList	nullList;
//		MGlobal::setActiveSelectionList(nullList);

		if( status == MS::kSuccess )
		{
			setResult( "Redone BPT" );
		}
		else
		{
			displayError( "Redo failed. Your undo stack has been flushed." );
		}
	}
	else
	{
//		Wenn mesh nicht veraendert wird, wird einfach so die Factory gerufen
		fBPTfty.redoFty();
		setResult( "BPTOperation succeeded" );
	}

	return status;
	
}




//-----------------------------------------------------------------
MStatus		ByronsPolyTools::undoIt()
//-----------------------------------------------------------------


{
	MStatus status;


	if(CMDactionMode == 0 || CMDactionMode == 4 || CMDactionMode == 5 || CMDactionMode == 6 || operationMode == 0)
	{
	
		MPRINT("Mache BPT rueckgaengig")
		status = undoModifyPoly();


	//	MGlobal::setActiveSelectionList(origList);
	//	MGlobal:: setComponentSelectionMask(undoMask);
		if( status == MS::kSuccess )
		{
			setResult( "Undone BPT" );
		}
		else
		{
			displayError( "Undo failed!" );
		}
	}

	//in jedem Fall die Original selection und mask wiederherstellen
	MGlobal::setActiveSelectionList(origList);
	MGlobal:: setComponentSelectionMask(undoMask);

	return status;
}

