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
// Copyright (C) 2002 Alias | Wavefront 
// 
// File: polyModifierCmd.cpp
//
// MEL Command: polyModifierCmd
//
// Author: Lonnie Li
//

//////////////
/* Includes */
//////////////

#include "polyModifierCmd.h"
#include "progressBar.h"

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MFloatVector.h>
#include <maya/MObjectArray.h>
#include <maya/MPlugArray.h>


// Function Sets
//
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNumericData.h>


////////////
/* Macros */
////////////

// MCheckStatus (Debugging tool)
//
#ifdef DEBUG
#	define MCheckStatus(status,message)			\
		if( MS::kSuccess != status ) {			\
			MString error("Status failed: ");	\
			error += message;					\
			MGlobal::displayError(error);		\
			return status;						\
		}
#else
#	define MCheckStatus(status,message)
#endif

// MAssert (Debugging tool)
//
#ifdef _DEBUG
#	define MAssert(state,message)					\
		if( !state ) {								\
			MString error("Assertion failed: ");	\
			error += message;						\
			MGlobal::displayError(error);			\
			return;									\
		}
#else
#	define MAssert(state,message)
#endif

// MStatusAssert (Debugging tool)
//
#ifdef _DEBUG
#	define MStatusAssert(state,message)				\
		if( !state ) {								\
			MString error("Assertion failed: ");	\
			error += message;						\
			MGlobal::displayError(error);			\
			return MS::kFailure;					\
		}
#else
#	define MStatusAssert(state,message)
#endif


////////////////////////////////////
/* polyModifierCmd Implementation */
////////////////////////////////////

polyModifierCmd::polyModifierCmd()
{
	fDagPathInitialized = false;
	fModifierNodeTypeInitialized = false;
	fModifierNodeNameInitialized = false;

	createAnimCurves = true;
}

polyModifierCmd::~polyModifierCmd()
{}

///////////////////////
// Protected Methods //
///////////////////////

//muss von abgeleiteter Klasse Ã¼berschrieben werden
// --------------------------------------------------------------------------------------------
void	polyModifierCmd::getModifierUINodeName(MString& /*newName*/)
// --------------------------------------------------------------------------------------------
{
	return;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::initModifierNode( MObject /* modifierNode */ )
// --------------------------------------------------------------------------------------------
//
//	Description:
//
//		Override this method in a derived class to set input attributes on the
//		modifier node. If not overidden, the modifier node will remain in it's
//		default state upon construction.
//
//		The argument 'MObject modifierNode', is not used by this base class
//		implementation. However, it may be used by derived classes. To avoid
//		compiler warnings of unreferenced parameters, we comment out the parameter
//		name.
//
{
	return MS::kSuccess;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::directModifier( MObject /* mesh */ )
// --------------------------------------------------------------------------------------------
//
//	Description:
//
//		Override this method in a derived class to provide an implementation for
//		directly modifying the mesh (writing on the mesh itself). This method is
//		only called in the case where history does not exist and history is turned
//		off (ie. DG operations are not desirable).
//
//		The argument 'MObject mesh', is not used by this base class implementation.
//		However, it may be used by derived classes. To avoid compiler warnings
//		of unreferenced parameters, we comment out the parameter name.
//
{
	return MS::kSuccess;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::doModifyPoly()
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kFailure;

	if( isCommandDataValid() )
	{
		// Get the state of the polyMesh
		//
		collectNodeState();

		if( !fHasHistory && !fHasRecordHistory )
		{
			MObject meshNode = fDagPath.node();

			// Pre-process the mesh - Cache old mesh (including tweaks, if applicable)
			//
			cacheMeshData();
			cacheMeshTweaks();

			// Call the directModifier
			//
			status = directModifier( meshNode );
		}
		else
		{
			MObject modifierNode;
			

			createModifierNode( modifierNode );
			initModifierNode( modifierNode );
			status = connectNodes( modifierNode );

			MFnDependencyNode depNodeFn(modifierNode);
			
			MString newName;
			getModifierUINodeName(newName);

			depNodeFn.setName(newName);

			//jetzt noch die customAttributes nonKeyable machen
			//kÃ¶nnte zu fehlern fÃ¼hren bei undo und redo ->tuts aber nicht :)
			if(createAnimCurves)
			{//diese Attribute kÃ¶nnen nur gesetzt werden, wenn eine BPTNode erstellt wird
				depNodeFn.findPlug("customSlFalloff").setKeyable(false);
			}
			
			// Selection vorher frei machen, weil unter umständen noch eine parameter gewählt war, welcher
			// dann verhindert, dass die Channelbox den neuen anzeigt
			//
			MGlobal::clearSelectionList();
			
			//jetzt die Node auf die SelectionList setzen/hinzufÃ¼gen
			MGlobal::select(modifierNode);
		}
	}

	

	return status;
}


// --------------------------------------------------------------------------------------------
void		polyModifierCmd::deleteTweaks()
// --------------------------------------------------------------------------------------------
{
		
	
	// Now, set the tweak values on the meshNode(s) to zero (History dependent)
		//
		MStatus stat;
		MFnNumericData numDataFn;
		MObject nullVector;

		// Create a NULL vector (0,0,0) using MFnNumericData to pass into the plug
		//

		MFnDependencyNode depTmpFn(fDagPath.node(),&stat);
		//stat.perror("");

		MPlug meshTweakPlug = depTmpFn.findPlug("pnts");

		numDataFn.create( MFnNumericData::k3Float );
		numDataFn.setData( 0, 0, 0 );
		nullVector = numDataFn.object();
		unsigned numTweaks = meshTweakPlug.numElements();
		MPlug tweak;
		for(int  i = 0; i < numTweaks; i++ )
		{
			// Access using logical indices since they are the only plugs guaranteed
			// to hold tweak data.
			//
			tweak = meshTweakPlug.elementByPhysicalIndex(i);
			tweak.setValue( nullVector );
		}

}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::redoModifyPoly()
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	if( !fHasHistory && !fHasRecordHistory )
	{
		MObject meshNode = fDagPath.node();

		// Call the directModifier - No need to pre-process the mesh data again
		//							 since we already have it.
		//
		status = directModifier( meshNode );
	}
	else
	{
		// Call the redo on the DG and DAG modifiers
		//
		if( !fHasHistory )
		{
			//fDagModifier.doIt();
			createDuplicate.doIt();
			reparentDuplicate.doIt();
			deleteDuplicate.doIt();
			//We can simply redo the operation without having to feed in a meshe into the duplicated shape

		}

		//if(speedupTweakProcessing() || fHasHistory)
		if(fHasHistory)	
			deleteTweaks();

		status = fDGModifier.doIt();


		if(createAnimCurves)
		{
			createSlideAnim.doIt();
		}

		//select the BPTNode again
		MFnDependencyNode depNodeFn(myMeshPath.node());
		MPlugArray connections;
		depNodeFn.findPlug("inMesh").connectedTo(connections,true, false);
		//it defintely has history

		MObject conObj = connections[0].node();
		MGlobal::select(conObj);


		// Da das modifierNodeHandle im letzten Undo gekillt wurde, regenerieren wir diese wieder für die  nächste benutzung
		myModifierNode = conObj;

	}


	return status;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::undoModifyPoly()
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	if( !fHasHistory && !fHasRecordHistory )
	{
		status = undoDirectModifier();
	}
	else
	{

		// Merke: Das MObject zur ModifierNode sollte jetzt zerstört werden, da es ungültig wird, wenn der modifier rückgängig gemacht wird.
		// Ansonsten gibts nen absturz
		myModifierNode = MObject::kNullObj;

		if(createAnimCurves)
		{
			createSlideAnim.undoIt();
		}

		fDGModifier.undoIt();

		// undoCachedMesh must be called before undoTweakProcessing because 
		// undoCachedMesh copies the original mesh *without* tweaks back onto
		// the existing mesh. Any changes done before the copy will be lost.
		//
		if( !fHasHistory )
		{
			status = undoCachedMesh();
			MCheckStatus( status, "undoCachedMesh" );
			//fDagModifier.undoIt();
			deleteDuplicate.undoIt();
			reparentDuplicate.undoIt();
			createDuplicate.undoIt();
		}

		status = undoTweakProcessing();
		MCheckStatus( status, "undoTweakProcessing" );
	}

	return status;
}


/////////////////////
// Private Methods //
/////////////////////

// --------------------------------------------------------------------------------------------
bool polyModifierCmd::isCommandDataValid()
// --------------------------------------------------------------------------------------------
{
	bool valid = true;

	// Check the validity of the DAG path
	//
	if( fDagPathInitialized )
	{
		fDagPath.extendToShape();
		if( !fDagPath.isValid()	|| fDagPath.apiType() != MFn::kMesh )
		{
			valid = false;
		}
	}
	else
	{
		valid = false;
	}

	// Check the validity of the Modifier node type/name
	//
	if( !fModifierNodeTypeInitialized && !fModifierNodeNameInitialized )
	{
		valid = false;
	}

	return valid;
}


// --------------------------------------------------------------------------------------------
void polyModifierCmd::collectNodeState()
// --------------------------------------------------------------------------------------------
{
	MStatus status;

	// Collect node state information on the given polyMeshShape
	//
	// - HasHistory (Construction History exists)
	// - HasTweaks
	// - HasRecordHistory (Construction History is turned on)
	//
	fDagPath.extendToShape();
	MObject meshNodeShape = fDagPath.node();

	MFnDependencyNode depNodeFn;
	depNodeFn.setObject( meshNodeShape );

	MPlug inMeshPlug = depNodeFn.findPlug( "inMesh" );
	fHasHistory = inMeshPlug.isConnected();

	// Tweaks exist only if the multi "pnts" attribute contains plugs
	// which contain non-zero tweak values. Use false, until proven true
	// search algorithm.
	//
	fHasTweaks = false;
	MPlug tweakPlug = depNodeFn.findPlug( "pnts" );
	if( !tweakPlug.isNull() )
	{
		// ASSERT: tweakPlug should be an array plug!
		//
		MAssert( (tweakPlug.isArray()),
				 "tweakPlug.isArray() -- tweakPlug is not an array plug" );

		MPlug tweak;
		MFloatVector tweakData;
		int i;
		int numElements = tweakPlug.numElements();

		for( i = 0; i < numElements; i++ )
		{
			tweak = tweakPlug.elementByPhysicalIndex( i, &status );
			if( status == MS::kSuccess && !tweak.isNull() )
			{
				getFloat3PlugValue( tweak, tweakData );
				if( 0 != tweakData.x ||
					0 != tweakData.y ||
					0 != tweakData.z )
				{
					fHasTweaks = true;
					break;
				}
			}
		}
	}

	int result;
	MGlobal::executeCommand( "constructionHistory -q -tgl", result );
	fHasRecordHistory = (0 != result);
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::createModifierNode( MObject& modifierNode )
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kFailure;

	if( fModifierNodeTypeInitialized || fModifierNodeNameInitialized )
	{
		if( fModifierNodeTypeInitialized )
		{
			modifierNode = fDGModifier.MDGModifier::createNode( fModifierNodeType, &status );
			myModifierNode = modifierNode;
		}
		else if( fModifierNodeNameInitialized )
		{
			modifierNode = fDGModifier.MDGModifier::createNode( fModifierNodeName, &status );
		}

		// Check to make sure that we have a modifier node of the appropriate type.
		// Requires an inMesh and outMesh attribute.
		//
		MFnDependencyNode depNodeFn( modifierNode );
		MObject inMeshAttr;
		MObject outMeshAttr;
		inMeshAttr = depNodeFn.attribute( "inMesh" );
		outMeshAttr = depNodeFn.attribute( "outMesh" );

		if( inMeshAttr.isNull() || outMeshAttr.isNull() )
		{
			displayError( "Invalid Modifier Node: inMesh and outMesh attributes are required." );
			status = MS::kFailure;
		}
	}
	
	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::processMeshNode( modifyPolyData& data )
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	// Declare our function sets. Use MFnDagNode here so
	// we can retrieve the parent transform.
	//
	MFnDagNode dagNodeFn;
	

	// Use the DAG path to retrieve our mesh shape node. 
	//
	data.meshNodeShape = fDagPath.node();
	dagNodeFn.setObject( data.meshNodeShape );

	// ASSERT: meshNodeShape node should have a parent transform!
	//
	MStatusAssert( (0 < dagNodeFn.parentCount()),
				   "0 < dagNodeFn.parentCount() -- meshNodeshape has no parent transform" );
	data.meshNodeTransform = dagNodeFn.parent(0);
	
	data.meshNodeDestPlug = dagNodeFn.findPlug( "inMesh" );
	data.meshNodeDestAttr = data.meshNodeDestPlug.attribute();



	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::processUpstreamNode( modifyPolyData& data )
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	// Declare our function sets - Although dagNodeFn derives from depNodeFn, we need
	//							   both since dagNodeFn can only refer to DAG objects.
	//							   We will use depNodeFn for all times other when dealing
	//							   with the DAG.
	//
	MFnDependencyNode	depNodeFn;
	MFnDagNode			dagNodeFn;

	// Use the selected node's plug connections to find the upstream plug.
	// Since we are looking at the selected node's inMesh attribute, it will
	// always have only one connection coming in if it has history, and none
	// otherwise.
	//
	// If there is no history, copy the selected node and place it ahead of the
	// modifierNode as the new upstream node so that the modifierNode has an
	// input mesh to operate on.
	//

	//save the meshDagPath for later use
	MDagPath::getAPathTo(data.meshNodeShape,myMeshPath);

	MPlugArray tempPlugArray;

 	if( fHasHistory )
	{
		// Since we have history, look for what connections exist on the
		// meshNode "inMesh" plug. "inMesh" plugs should only ever have one
		// connection.
		//
		data.meshNodeDestPlug.connectedTo( tempPlugArray, true, false);
		

		// ASSERT: Only one connection should exist on meshNodeShape.inMesh!
		//
		MStatusAssert( (tempPlugArray.length() == 1),
					   "tempPlugArray.length() == 1 -- 0 or >1 connections on meshNodeShape.inMesh" );
		data.upstreamNodeSrcPlug = tempPlugArray[0];

		
		// Construction history only deals with shapes, so we can grab the
		// upstreamNodeShape off of the source plug.
		//
		// Dieser Bereich muss bleiben, weil diese Attribute noch benötigt werden
		data.upstreamNodeShape = data.upstreamNodeSrcPlug.node();
		depNodeFn.setObject( data.upstreamNodeShape );
		data.upstreamNodeSrcAttr = data.upstreamNodeSrcPlug.attribute();
		

		// Disconnect the upstream node and the selected node, so we can
		// replace them with our own connections below.
		//
		MPlug	nodePlug(data.meshNodeShape,data.meshNodeDestAttr ) ;
		INVIS(cout<<data.upstreamNodeSrcPlug.name().asChar()<<" --|-- "<<nodePlug.name().asChar()<<endl);
		status =	fDGModifier.disconnect( data.upstreamNodeSrcPlug,
											nodePlug );
											
		
		MCheckStatus( status, "Disconnect Upstream mit meshNode" );


	}
	else	// No History (!fHasHistory)
	{
		// Use the DAG node function set to duplicate the shape of the meshNode.
		// The duplicate method will return an MObject handle to the transform
		// of the duplicated shape, so traverse the dag to locate the shape. Store
		// this duplicate shape as our "upstream" node to drive the input for the 
		// modifierNode.
		//
		depNodeFn.setObject( data.meshNodeShape );
		data.upstreamNodeTransform = createDuplicate.createNode("mesh");
		createDuplicate.doIt();

		dagNodeFn.setObject( data.upstreamNodeTransform );
		

		// Ensure that our upstreamNode is pointing to a shape.
		//
		MStatusAssert( (0 < dagNodeFn.childCount()),
					   "0 < dagNodeFn.childCount() -- Duplicate meshNode transform has no shape." );
		data.upstreamNodeShape = dagNodeFn.child(0);

		MPlug outMeshPlug = depNodeFn.findPlug("outMesh");
		depNodeFn.setObject(data.upstreamNodeShape);

		//jetzt inMesh upstreamNodeShape mit outMesh meshShape fÃ¼llen
		MDGModifier tempMod;
		tempMod.connect(outMeshPlug,depNodeFn.findPlug("inMesh"));
		tempMod.doIt();

		//force DGEVAL
		MString cmd = "dgeval -src ";
		cmd += depNodeFn.name();
		cmd += ".outMesh";
		MGlobal::executeCommand(cmd,false,false);

		tempMod.undoIt();

		
		// Re-parent the upstreamNodeShape under our original transform
		//
		reparentDuplicate.reparentNode( data.upstreamNodeShape, data.meshNodeTransform );
		reparentDuplicate.doIt();


		deleteDuplicate.deleteNode( data.upstreamNodeTransform );
		deleteDuplicate.doIt();
		/*
		status = fDagModifier.reparentNode( data.upstreamNodeShape, data.meshNodeTransform );
		MCheckStatus( status, "reparentNode" );

		// Perform the DAG re-parenting
		// 
		// Note: This reparent must be performed before the deleteNode() is called.
		//		 See polyModifierCmd.h (see definition of fDagModifier) for more details.
		//
		status = fDagModifier.doIt();
		MCheckStatus( status, "fDagModifier.doIt()" );
		*/
		// Mark the upstreamNodeShape (the original shape) as an intermediate object
		// (making it invisible to the user)
		//
		dagNodeFn.setObject( data.upstreamNodeShape );
		dagNodeFn.setIntermediateObject( true );

		// Get the upstream node source attribute
		//
		data.upstreamNodeSrcAttr = dagNodeFn.attribute( "outMesh" );
		data.upstreamNodeSrcPlug = MPlug(data.upstreamNodeShape, data.upstreamNodeSrcAttr);

		/*
		// Remove the duplicated transform node (clean up)
		//
		status = fDagModifier.deleteNode( data.upstreamNodeTransform );
		MCheckStatus( status, "deleteNode" );

		// Perform the DAG delete node
		//
		// Note: This deleteNode must be performed after the reparentNode() method is
		//		 completed. See polyModifierCmd.h (see definition of fDagModifier) for
		//		 details.
		//
		status = fDagModifier.doIt();
		MCheckStatus( status, "fDagModifier.doIt()" );
		*/

		// Cache the DAG path to the duplicate shape
		//
		dagNodeFn.getPath( fDuplicateDagPath );

		//finally delete the tweaks to avoid double transformation
		deleteTweaks();
	}

	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::processModifierNode( MObject modifierNode,
											  modifyPolyData& data )
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	MFnDependencyNode depNodeFn ( modifierNode );
	data.modifierNodeSrcAttr = depNodeFn.attribute( "outMesh" , &status);

	MCheckStatus( status, "Finden des outMesh Attributes" );

	data.modifierNodeDestAttr = depNodeFn.attribute( "inMesh" , &status );


	return status;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::processTweaks( modifyPolyData& data )
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	// Clear tweak undo information (to be rebuilt)
	//
	fTweakIndexArray.clear();
	fTweakVectorArray.clear();

	// Extract the tweaks and place them into a polyTweak node. This polyTweak node
	// will be placed ahead of the modifier node to maintain the order of operations.
	// Special care must be taken into recreating the tweaks:
	//
	//		1) Copy tweak info (including connections!)
	//		2) Remove tweak info from both meshNode and a duplicate meshNode (if applicable)
	//		3) Cache tweak info for undo operations
	//
	//if( fHasTweaks && fHasHistory && !speedupTweakProcessing())
	if( fHasTweaks && fHasHistory )
	{
		// Declare our function sets
		//
		MFnDependencyNode depNodeFn;

		// Declare our attributes and plugs
		//
		MPlug	meshTweakPlug;
		MPlug	upstreamTweakPlug;
		MObject tweakNodeTweakAttr;

		// Declare our tweak processing variables
		//
		MPlug				tweak;
		MPlug				tweakChild;
		MObject				tweakData;
		MObjectArray		tweakDataArray;
		MFloatVector		tweakVector;

		MIntArray			tweakSrcConnectionCountArray;
		MPlugArray			tweakSrcConnectionPlugArray;
		MIntArray			tweakDstConnectionCountArray;
		MPlugArray			tweakDstConnectionPlugArray;

		MPlugArray			tempPlugArray;

		unsigned i;
		unsigned j;
		unsigned k;

		// Create the tweak node and get its attributes
		//
		data.tweakNode = fDGModifier.MDGModifier::createNode( "polyTweak" );
		depNodeFn.setObject( data.tweakNode );
		data.tweakNodeSrcAttr = depNodeFn.attribute( "output" );
		data.tweakNodeDestAttr = depNodeFn.attribute( "inputPolymesh" );
		tweakNodeTweakAttr = depNodeFn.attribute( "tweak" );

		depNodeFn.setObject( data.meshNodeShape );
		meshTweakPlug = depNodeFn.findPlug( "pnts" );

		// ASSERT: meshTweakPlug should be an array plug!
		//
		MStatusAssert( (meshTweakPlug.isArray()),
					   "meshTweakPlug.isArray() -- meshTweakPlug is not an array plug" );
		unsigned numElements = meshTweakPlug.numElements();

		// Gather meshTweakPlug data
		//
		for( i = 0; i < numElements; i++ )
		{
			// MPlug::numElements() only returns the number of physical elements
			// in the array plug. Thus we must use elementByPhysical index when using
			// the index i.
			//
			tweak = meshTweakPlug.elementByPhysicalIndex(i);

			// If the method fails, the element is NULL. Only append the index
			// if it is a valid plug.
			//
			if( !tweak.isNull() )
			{
				// Cache the logical index of this element plug
				//
				unsigned logicalIndex = tweak.logicalIndex();

				// Collect tweak data and cache the indices and float vectors
				//
				tweak.getValue( tweakData );
				tweakDataArray.append( tweakData );
				getFloat3PlugValue( tweak, tweakVector );
				fTweakIndexArray.append( logicalIndex );
				fTweakVectorArray.append( tweakVector );

				// Collect tweak connection data
				//
				// Parse down to the deepest level of the plug tree and check
				// for connections - look at the child nodes of the element plugs.
				// If any connections are found, record the connection and disconnect
				// it.
				//

				// ASSERT: The element plug should be compound!
				//
				MStatusAssert( (tweak.isCompound()),
							   "tweak.isCompound() -- Element tweak plug is not compound" );

				unsigned numChildren = tweak.numChildren();
				for( j = 0; j < numChildren; j++ )
				{
					tweakChild = tweak.child(j);
					if( tweakChild.isConnected() )
					{
						// Get all connections with this plug as source, if they exist
						//
						tempPlugArray.clear();
						if( tweakChild.connectedTo( tempPlugArray, false, true ) )
						{
							unsigned numSrcConnections = tempPlugArray.length();
							tweakSrcConnectionCountArray.append( numSrcConnections );

							for( k = 0; k < numSrcConnections; k++ )
							{
								tweakSrcConnectionPlugArray.append( tempPlugArray[k] );
								fDGModifier.disconnect( tweakChild, tempPlugArray[k] );
							}
						}
						else
						{
							tweakSrcConnectionCountArray.append(0);
						}

						// Get the connection with this plug as destination, if it exists
						//
						tempPlugArray.clear();
						if( tweakChild.connectedTo( tempPlugArray, true, false ) )
						{
							// ASSERT: tweakChild should only have one connection as destination!
							//
							MStatusAssert( (tempPlugArray.length() == 1),
										   "tempPlugArray.length() == 1 -- 0 or >1 connections on tweakChild" );

							tweakDstConnectionCountArray.append(1);
							tweakDstConnectionPlugArray.append( tempPlugArray[0] );
							fDGModifier.disconnect( tempPlugArray[0], tweakChild );
						}
						else
						{
							tweakDstConnectionCountArray.append(0);
						}
					}
					else
					{
						tweakSrcConnectionCountArray.append(0);
						tweakDstConnectionCountArray.append(0);
					}
				}
			}
		}

		// Apply meshTweakPlug data to our polyTweak node
		//
		MPlug polyTweakPlug( data.tweakNode, tweakNodeTweakAttr );
		unsigned numTweaks = fTweakIndexArray.length();
		int srcOffset = 0;
		int dstOffset = 0;
		
		
		//Progress initialisieren
		progressBar progress("Processing Tweaks", numTweaks);
		

		for( i = 0; i < numTweaks; i++ )
		{
			// Apply tweak data
			//
			tweak = polyTweakPlug.elementByLogicalIndex( fTweakIndexArray[i] );
			tweak.setValue( tweakDataArray[i] );

			// ASSERT: Element plug should be compound!
			//
			MStatusAssert( (tweak.isCompound()),
						   "tweak.isCompound() -- Element plug, 'tweak', is not compound" );

			unsigned numChildren = tweak.numChildren();
			for( j = 0; j < numChildren; j++ )
			{
				tweakChild = tweak.child(j);

				// Apply tweak source connection data
				//
				if( 0 < tweakSrcConnectionCountArray[i*numChildren + j] )
				{
					for( k = 0;
						 k < (unsigned) tweakSrcConnectionCountArray[i*numChildren + j];
						 k++ )
					{
						fDGModifier.connect( tweakChild,
											 tweakSrcConnectionPlugArray[srcOffset] );
						srcOffset++;
					}
				}
						
				// Apply tweak destination connection data
				//
				if( 0 < tweakDstConnectionCountArray[i*numChildren + j] )
				{
					fDGModifier.connect( tweakDstConnectionPlugArray[dstOffset],
										 tweakChild );
					dstOffset++;
				}
			}

			if(i%50 == 0)
			{	
				progress.set(i);
			}
		}

		


		// Now, set the tweak values on the meshNode(s) to zero (History dependent)
		//
		MFnNumericData numDataFn;
		MObject nullVector;

		// Create a NULL vector (0,0,0) using MFnNumericData to pass into the plug
		//
		numDataFn.create( MFnNumericData::k3Float );
		numDataFn.setData( 0, 0, 0 );
		nullVector = numDataFn.object();

		for( i = 0; i < numTweaks; i++ )
		{
			// Access using logical indices since they are the only plugs guaranteed
			// to hold tweak data.
			//
			tweak = meshTweakPlug.elementByLogicalIndex( fTweakIndexArray[i] );
			tweak.setValue( nullVector );
		}

		// Only have to clear the tweaks off the duplicate mesh if we do not have history
		// and we want history.
		//
		if( !fHasHistory && fHasRecordHistory )
		{
			depNodeFn.setObject( data.upstreamNodeShape );
			upstreamTweakPlug = depNodeFn.findPlug( "pnts" );

			if( !upstreamTweakPlug.isNull() )
			{
				for( i = 0; i < numTweaks; i++ )
				{
					tweak = meshTweakPlug.elementByLogicalIndex( fTweakIndexArray[i] );
					tweak.setValue( nullVector );
				}
			}
		}
	}
	else
		fHasTweaks = false;

	return status;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::connectNodes( MObject modifierNode )
// --------------------------------------------------------------------------------------------
//
//	Description:
//
//		This method connects up the modifier nodes, while accounting for DG factors
//		such as construction history and tweaks. The method has a series of steps which
//		it runs through to process nodes under varying circumstances:
//
//		1) Gather meshNode connection data (ie. attributes and plugs)
//
//		2) Gather upstreamNode data - This is history-dependent. If the node has history,
//									  an actual upstreamNode exists and that is used to
//									  drive the input of our modifierNode.
//
//									  Otherwise, if the node does not have history, the
//									  meshNode is duplicated, set as an intermediate object
//									  and regarded as our new upstreamNode which will drive
//									  the input of our modifierNode. The case with history
//									  already has this duplicate meshNode at the top, driving
//									  all other history nodes and serving as a reference
//									  to the "original state" of the node before any
//									  modifications.
//
//		3) Gather modifierNode connection data
//
//		4) Process tweak data (if it exists) - This is history-dependent. If there is
//											   history, the tweak data is extracted and deleted
//											   from the meshNode and encapsulated inside a
//											   polyTweak node. The polyTweak node is then
//											   inserted ahead of the modifier node.
//
//											   If there is no history, the same is done as
//											   in the history case, except the tweaks are
//											   deleted from the duplicate meshNode in addition
//											   to the actual meshNode.
//
//		5) Connect the nodes
//
//		6) Collapse/Bake nodes into the actual meshNode if the meshNode had no previous
//		   construction history and construction history recording is turned off.
//		   (ie. (!fHasHistory && !fHasRecordHistory) == true )
//
{
	MStatus status;

	// Declare our internal processing data structure (see polyModifierCmd.h for definition)
	//
	modifyPolyData data;	//->jetzt als ElementVariable

	// Get the mesh node, plugs and attributes
	//
	status = processMeshNode( data );
	MCheckStatus( status, "processMeshNode" );

	// Get upstream node, plugs and attributes
	//
	status = processUpstreamNode( data );
	MCheckStatus( status, "processUpstreamNode" );

	// Get the modifierNode attributes
	//
	status = processModifierNode( modifierNode,
						 data );
	MCheckStatus( status, "processModifierNode" );

	// Process tweaks on the meshNode
	//
	status = processTweaks( data );
	MCheckStatus( status, "processTweaks" );



	
	

	if( fHasTweaks )
	{

		status = fDGModifier.connect( data.upstreamNodeShape,
							 data.upstreamNodeSrcAttr,
							 data.tweakNode,
							 data.tweakNodeDestAttr );
		MCheckStatus( status, "upstream-tweak connect failed" );

		status = fDGModifier.connect( data.tweakNode,
							 data.tweakNodeSrcAttr,
							 modifierNode,
							 data.modifierNodeDestAttr );
		MCheckStatus( status, "tweak-modifier connect failed" );
	}
	else
	{
		//hier müssen die Plugs verwendet werden, weil die Attribute eventuell multis sind
		
		MPlug destPlug(modifierNode,data.modifierNodeDestAttr );
		
		INVIS(cout<<data.upstreamNodeSrcPlug.name().asChar()<<" --> "<<destPlug.name().asChar()<<endl);

		status = fDGModifier.connect( data.upstreamNodeSrcPlug,
										destPlug
									);
		MCheckStatus( status, "upstream-modifier connect failed" );
	}



	
	
	// outMesh mit inputMesh verbinden
	//
	fDGModifier.connect(MPlug(modifierNode,data.modifierNodeSrcAttr), data.meshNodeDestPlug);
	
	
	// Außerdem das MatrixAttribut des Meshes mit dem meshMatrix Attr der Node verbinden
	//
	MFnDependencyNode depFn( modifierNode );
	MPlug	meshMatrix( modifierNode, depFn.attribute("meshMatrix") );



	depFn.setObject(data.meshNodeTransform);
	MPlug	shapeMatrix( data.meshNodeTransform, depFn.attribute("worldMatrix") );

	fDGModifier.connect(shapeMatrix.elementByLogicalIndex( 0 ), meshMatrix );


	
	

	// Jetzt die ModifierNode mit der VisNode connecten
/*	
	status = fDGModifier.connect( modifierNode,
						 data.modifierNodeSrcAttr,
						 data.meshNodeShape,
						 data.meshNodeDestAttr );
*/


//	MCheckStatus( status, "modifier-mesh connect failed" );



	status = fDGModifier.doIt();
	MCheckStatus( status, "modifier-mesh DOIT failed" );


	if(createAnimCurves)
	{
		//AnimCurve erstellen(dies ist nur hier mÃ¶glich, da die Node erst nach dem DoIt wirklich existiert (scheinbar)
		MFnAnimCurve	aCurve;

		MFnDependencyNode myDepNodeFn(modifierNode);

		aCurve.create(modifierNode,myDepNodeFn.attribute("csl"),MFnAnimCurve::kAnimCurveUU,&createSlideAnim,&status);
	}

	return status;
}




MStatus polyModifierCmd::cacheMeshData()
{
	MStatus status = MS::kSuccess;

	MFnDependencyNode depNodeFn;
	MFnDagNode dagNodeFn;

	MObject meshNode = fDagPath.node();
	MObject dupMeshNode;
	MPlug dupMeshNodeOutMeshPlug;

	// Duplicate the mesh
	//
	dagNodeFn.setObject( meshNode );
	dupMeshNode = dagNodeFn.duplicate();

	MDagPath dupMeshDagPath;
	MDagPath::getAPathTo( dupMeshNode, dupMeshDagPath );
	dupMeshDagPath.extendToShape();

	depNodeFn.setObject( dupMeshDagPath.node() );
	dupMeshNodeOutMeshPlug = depNodeFn.findPlug( "outMesh", &status );
	MCheckStatus( status, "Could not retrieve outMesh" );

	// Retrieve the meshData
	//
	status = dupMeshNodeOutMeshPlug.getValue( fMeshData );
	MCheckStatus( status, "Could not retrieve meshData" );

	// Delete the duplicated node
	//

	MDGModifier deleter;
	deleter.deleteNode(dupMeshNode);
	deleter.doIt();
	//MGlobal::deleteNode( dupMeshNode );

	return status;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::cacheMeshTweaks()
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	// Clear tweak undo information (to be rebuilt)
	//
	fTweakIndexArray.clear();
	fTweakVectorArray.clear();

	// Extract the tweaks and store them in our local tweak cache members
	//
	if( fHasTweaks )
	{
		// Declare our function sets
		//
		MFnDependencyNode depNodeFn;

		MObject meshNode = fDagPath.node();
		MPlug	meshTweakPlug;

		// Declare our tweak processing variables
		//
		MPlug				tweak;
		MPlug				tweakChild;
		MObject				tweakData;
		MObjectArray		tweakDataArray;
		MFloatVector		tweakVector;

		MPlugArray			tempPlugArray;

		unsigned i;

		depNodeFn.setObject( meshNode );
		meshTweakPlug = depNodeFn.findPlug( "pnts" );

		// ASSERT: meshTweakPlug should be an array plug!
		//
		MStatusAssert( (meshTweakPlug.isArray()),
					   "meshTweakPlug.isArray() -- meshTweakPlug is not an array plug" );
		unsigned numElements = meshTweakPlug.numElements();

		// Gather meshTweakPlug data
		//
		for( i = 0; i < numElements; i++ )
		{
			// MPlug::numElements() only returns the number of physical elements
			// in the array plug. Thus we must use elementByPhysical index when using
			// the index i.
			//
			tweak = meshTweakPlug.elementByPhysicalIndex(i);

			// If the method fails, the element is NULL. Only append the index
			// if it is a valid plug.
			//
			if( !tweak.isNull() )
			{
				// Cache the logical index of this element plug
				//
				unsigned logicalIndex = tweak.logicalIndex();

				// Collect tweak data and cache the indices and float vectors
				//
				getFloat3PlugValue( tweak, tweakVector );
				fTweakIndexArray.append( logicalIndex );
				fTweakVectorArray.append( tweakVector );
			}
		}
	}

	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::undoCachedMesh()
// --------------------------------------------------------------------------------------------
{
	MStatus status;

	// Only need to restore the cached mesh if there was no history. Also
	// check to make sure that we are in the record history state.
	//
	MStatusAssert( (fHasRecordHistory), "fHasRecordHistory == true" );

	if( !fHasHistory )
	{
		MFnDependencyNode depNodeFn;

		MString meshNodeName;
		MObject meshNodeShape;
		MPlug	meshNodeDestPlug;
		MPlug	meshNodeOutMeshPlug;
		MObject dupMeshNodeShape;
		MPlug	dupMeshNodeSrcPlug;

		meshNodeShape = fDagPath.node();
		dupMeshNodeShape = fDuplicateDagPath.node();

		depNodeFn.setObject( meshNodeShape );
		meshNodeName = depNodeFn.name();
		meshNodeDestPlug = depNodeFn.findPlug( "inMesh", &status );
		MCheckStatus( status, "Could not retrieve inMesh" );
		meshNodeOutMeshPlug = depNodeFn.findPlug( "outMesh", &status );
		MCheckStatus( status, "Could not retrieve outMesh" );

		depNodeFn.setObject( dupMeshNodeShape );
		dupMeshNodeSrcPlug = depNodeFn.findPlug( "outMesh", &status );
		MCheckStatus( status, "Could not retrieve outMesh" );

		// For the case with tweaks, we cannot write the mesh directly back onto
		// the cachedInMesh, since the shape can have out of date information from the
		// cachedInMesh, thus we temporarily connect the duplicate mesh shape to the
		// mesh shape and force a DG evaluation.
		//
		// For the case without tweaks, we can simply write onto the outMesh, since
		// the shape relies solely on an outMesh when there is no history nor tweaks.
		//
		if( fHasTweaks )
		{
			MDGModifier dgModifier;
			dgModifier.connect( dupMeshNodeSrcPlug, meshNodeDestPlug );
			status = dgModifier.doIt();
			MCheckStatus( status, "Could not connect dupMeshNode -> meshNode" );

			// Need to force a DG evaluation now that the input has been changed.
			//
			MString cmd( "dgeval -src " );
			cmd += meshNodeName;
			cmd += ".outMesh";	//outMesh statt inMesh, damit undo (ohne history) funzt
			status = MGlobal::executeCommand( cmd, false, false );
			MCheckStatus( status, "Could not force DG eval" );

			// Disconnect the duplicate meshNode now
			//
			dgModifier.undoIt();

			
		}
		else
		{
			MObject meshData;
			status = dupMeshNodeSrcPlug.getValue( meshData );
			MCheckStatus( status, "Could not retrieve meshData" );

			status = meshNodeOutMeshPlug.setValue( meshData );
			MCheckStatus( status, "Could not set outMesh" );

			
		}
	}

	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::undoTweakProcessing()
// --------------------------------------------------------------------------------------------
{
	MStatus status = MS::kSuccess;

	if( fHasTweaks )
	{
		MFnDependencyNode depNodeFn;

		MObject	meshNodeShape;
		MPlug	meshTweakPlug;
		MPlug	tweak;
		MObject	tweakData;

		meshNodeShape = fDagPath.node();
		depNodeFn.setObject( meshNodeShape );
		meshTweakPlug = depNodeFn.findPlug( "pnts" );

		MStatusAssert( (meshTweakPlug.isArray()),
					   "meshTweakPlug.isArray() -- meshTweakPlug is not an array plug" );

		unsigned i;
		unsigned numElements = fTweakIndexArray.length();

		for( i = 0; i < numElements; i++ )
		{
			tweak = meshTweakPlug.elementByLogicalIndex( fTweakIndexArray[i] );
			getFloat3asMObject( fTweakVectorArray[i], tweakData );
			tweak.setValue( tweakData );
		}

		// In the case of no history, the duplicate node shape will be disconnected on undo
		// so, there is no need to undo the tweak processing on it.
		//
	}

	return status;
}



// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::undoDirectModifier()
// --------------------------------------------------------------------------------------------
{
	MStatus status;

	MFnDependencyNode depNodeFn;
	MFnDagNode dagNodeFn;

	MObject meshNode = fDagPath.node();
	depNodeFn.setObject( meshNode );
	
	// For the case with tweaks, we cannot write the mesh directly back onto
	// the cachedInMesh, since the shape can have out of date information from the
	// cachedInMesh. Thus we temporarily create an duplicate mesh, place our
	// old mesh on the outMesh attribute of our duplicate mesh, connect the
	// duplicate mesh shape to the mesh shape, and force a DG evaluation.
	//
	// For the case without tweaks, we can simply write onto the outMesh, since
	// the shape relies solely on an outMesh when there is no history nor tweaks.
	//
	if( fHasTweaks )
	{
		// Retrieve the inMesh and name of our mesh node (for the DG eval)
		//
		depNodeFn.setObject( meshNode );
		MPlug meshNodeInMeshPlug = depNodeFn.findPlug( "inMesh", &status );
		MCheckStatus( status, "Could not retrieve inMesh" );
		MString meshNodeName = depNodeFn.name();

		// Duplicate our current mesh
		//
		dagNodeFn.setObject( meshNode );
		MObject dupMeshNode = dagNodeFn.duplicate();

		// The dagNodeFn::duplicate() returns a transform, but we need a shape
		// so retrieve the DAG path and extend it to the shape.
		//
		MDagPath dupMeshDagPath;
		MDagPath::getAPathTo( dupMeshNode, dupMeshDagPath );
		dupMeshDagPath.extendToShape();

		// Retrieve the outMesh of the duplicate mesh and set our mesh data back
		// on it.
		//
		depNodeFn.setObject( dupMeshDagPath.node() );
		MPlug dupMeshNodeOutMeshPlug = depNodeFn.findPlug( "outMesh", &status );
		MCheckStatus( status, "Could not retrieve outMesh" );
		status = dupMeshNodeOutMeshPlug.setValue( fMeshData );

		// Temporarily connect the duplicate mesh node to our mesh node
		//
		MDGModifier dgModifier;
		dgModifier.connect( dupMeshNodeOutMeshPlug, meshNodeInMeshPlug );
		status = dgModifier.doIt();
		MCheckStatus( status, "Could not connect dupMeshNode -> meshNode" );

		// Need to force a DG evaluation now that the input has been changed.
		//
		MString cmd("dgeval -src ");
		cmd += meshNodeName;
		cmd += ".inMesh";
		status = MGlobal::executeCommand( cmd, false, false );
		MCheckStatus( status, "Could not force DG eval" );

		// Disconnect and delete the duplicate mesh node now
		//
		dgModifier.undoIt();
		MGlobal::deleteNode( dupMeshNode );

		// Restore the tweaks on the mesh
		//
		status = undoTweakProcessing();
	}
	else
	{
		// Restore the original mesh by writing the old mesh data (fMeshData) back
		// onto the outMesh of our meshNode
		//
		depNodeFn.setObject( meshNode );
		MPlug meshNodeOutMeshPlug = depNodeFn.findPlug( "outMesh", &status );
		MCheckStatus( status, "Could not retrieve outMesh" );
		status = meshNodeOutMeshPlug.setValue( fMeshData );
		MCheckStatus( status, "Could not set meshData" );
	}

	return status;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::getFloat3PlugValue( MPlug plug, MFloatVector & value )
// --------------------------------------------------------------------------------------------
{
	// Retrieve the value as an MObject
	//
	MObject object;
	plug.getValue( object );

	// Convert the MObject to a float3
	//
	MFnNumericData numDataFn( object );
	numDataFn.getData( value[0], value[1], value[2] );
	return MS::kSuccess;
}


// --------------------------------------------------------------------------------------------
MStatus polyModifierCmd::getFloat3asMObject( MFloatVector value, MObject& object )
// --------------------------------------------------------------------------------------------
{
	// Convert the float value into an MObject
	//
	MFnNumericData numDataFn;
	numDataFn.create( MFnNumericData::k3Float );
	numDataFn.setData( value[0], value[1], value[2] );
	object = numDataFn.object();
	return MS::kSuccess;
}
