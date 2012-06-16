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

#include "softTransformationToolMoveM.h"
// --------------------------------------



//******************************************************************************************************************
//***************************
//MOVE MANIPULATOR METHODEN
//***************************
//******************************************************************************************************************


MTypeId moveManip::id( 0x00108BC1 );

//-----------------------------------------------
void*	moveManip::creator()
//-----------------------------------------------
{
	return new moveManip;
}


//----------------------------------
MStatus moveManip::initialize()
//----------------------------------
{ 
    MStatus stat;
    stat = MPxManipContainer::initialize();
    return stat;
}

//----------------------------------
MStatus moveManip::createChildren()
//----------------------------------
{

    MString pointManipName("pointManip");
    MString pointName("freePoint");
    fFreePointManip = addFreePointTriadManip(pointManipName,
											 pointName);


	fDiscManip = addDiscManip("discManip",
							 "angle");

	fDiscManip2 = addDiscManip("discManip2",
								"angle");

	fStateManip = addStateManip("stateManipu","stateManip");

	fToggleManip = addToggleManip("toggleManipu","toggler");
	
	finishAddingManips();	//Merke: FinishAddingManips funzt hier viiiiel besser -> keine Abstürze mehr
    return MS::kSuccess;
}

//-------------------------------------------------
MDagPath	moveManip::findMeshPath(const MObject& node) const
//-------------------------------------------------
{
	MFnDependencyNode depNodeFn(node);

	MPlugArray	plugArray;
	
	depNodeFn.findPlug("outMesh").connectedTo(plugArray,false, true);

	MFnDagNode dagNodeFn(plugArray[0].node());

	MDagPath path;

	dagNodeFn.getPath(path);

	return path;
}

//------------------------------------------------------------
void	moveManip::calculateScaleFactor()
//------------------------------------------------------------
{
	/*
	MDagPath meshPath = findMeshPath(node);
	
	MFnDagNode	dagFn(meshPath.transform());
	MBoundingBox	box = dagFn.boundingBox();
	
	
	bbScaleFactor = box.max().y - box.min().y;
	*/
	

	MVector translation = transFn.translation(MSpace::kWorld);

	MPoint screenCenter;
	MVector centerVec;
	
	short pw = view.portWidth(),ph = view.portHeight();

	
	view.viewToWorld(short(pw / 2), short(ph / 2),screenCenter, centerVec);
	
	bbScaleFactor = (MPoint(translation) - screenCenter).length() * (1.0/(float(pw) / float(ph)));


}

//------------------------------------------------------------
MStatus moveManip::connectToDependNode(const MObject &node)
//------------------------------------------------------------
{
    MStatus stat;

    // Connect the plugs
    //    
	//für späteres onScreenGl
	view = M3dView::active3dView();

	

    MFnDependencyNode nodeFn(node,&stat);
    if(stat == MS::kFailure)stat.perror("Fehler bei depNode: ");


    MPlug tPlug = nodeFn.findPlug("endPoint", &stat);
	if(stat == MS::kFailure)stat.perror("Fehler bei tPlug: ");


    MFnFreePointTriadManip freePointManipFn(fFreePointManip);

	//transFn initialisieren
	transFn.setObject(fFreePointManip);
	
	stat = freePointManipFn.connectToPointPlug(tPlug);
	if(stat == MS::kFailure)stat.perror("Fehler bei connectToPlug: ");

	freePointIndex = freePointManipFn.pointIndex(); 	
/*	
	translatePlugIndex = addManipToPlugConversionCallback(tPlug, 
										 (manipToPlugConversionCallback) 
										 &moveManip::moveOthersCB);
/*
	addPlugToManipConversionCallback(freePointManipFn.pointIndex(),
										 (plugToManipConversionCallback) 
										 &moveManip::moveOthersCBReverse);
*/
	MPlug togglePlug = nodeFn.findPlug("fv");
	
	MFnToggleManip toggleFn(fToggleManip);
	toggleFn.connectToTogglePlug(togglePlug);

	toggleFn.setLength(0.0);
	addPlugToManipConversionCallback(toggleFn.startPointIndex(),
										 (plugToManipConversionCallback) 
										 &moveManip::setPointCB2);

	INVIS(cout<<freePointIndex<<" = freePointIndex "<<endl;)


	MPlug	sPlug = nodeFn.findPlug("scale");
	MFnDiscManip	discFn(fDiscManip);

	discFn.connectToAnglePlug(sPlug);

	addPlugToManipConversionCallback(discFn.centerIndex(),
										 (plugToManipConversionCallback) 
										 &moveManip::setDiscPointCB);
	/*
	MFnTransform transFn(fDiscManip);
	double tmp[] = {1.5,1.5,1.5};
	transFn.setScale(tmp);
	*/


	MPlug	anPlugged = nodeFn.findPlug("alongNormal");
	discFn.setObject(fDiscManip2);
	
	discFn.connectToAnglePlug(anPlugged);
	
	addPlugToManipConversionCallback(discFn.centerIndex(),
										 (plugToManipConversionCallback) 
										 &moveManip::setDiscPointCB2);



	MPlug mPlug = nodeFn.findPlug("falloffType");
	MFnStateManip	stateFn(fStateManip);

	
	addPlugToManipConversionCallback(stateFn.positionIndex(),
										 (plugToManipConversionCallback) 
										 &moveManip::setPointCB);

	stateFn.connectToStatePlug(mPlug);
	stateFn.setMaxStates(4);	//erstmal 1, solange es nur linear gibt


 /*   stat = finishAddingManips();

	if(stat = MS::kFailure)
	{
		stat.perror("Finish Manipulators: ");
		return stat;
	}*/

    MPxManipContainer::connectToDependNode(node);        
	if(stat = MS::kFailure)
	{
		stat.perror("connect To Node: ");
		return stat;
	}


    return stat;
}



//------------------------------------------------------------
MManipData	moveManip::setPointCB(unsigned index)
//------------------------------------------------------------
{
	return getCachedTranslation(0.05 * bbScaleFactor,0.175 * bbScaleFactor,0.0);
}


//------------------------------------------------------------
MManipData	moveManip::setPointCB2(unsigned index)
//------------------------------------------------------------
{
	calculateScaleFactor();
	return getCachedTranslation(-0.05 * bbScaleFactor,0.175 * bbScaleFactor,0.0);
}


//------------------------------------------------------------
MManipData	moveManip::setDiscPointCB(unsigned index)
//------------------------------------------------------------
{
	getPlugTranslation();
	return getCachedTranslation(0.0,-0.125 * bbScaleFactor,0.0);
}

//------------------------------------------------------------
MManipData	moveManip::setDiscPointCB2(unsigned index)
//------------------------------------------------------------
{
	return getCachedTranslation(0.0,-0.065 * bbScaleFactor,0.0);
}

//------------------------------------------------------------//------------------------------------------------------------
MManipData	moveManip::getCachedTranslation(double xOffset,double yOffset,double zOffset)
//------------------------------------------------------------//------------------------------------------------------------
{
	MManipData	mData;

	MFnNumericData	numFn;
	MObject numData = numFn.create(MFnNumericData::k3Double);
	


	MDagPath cameraPath;
	view.getCamera(cameraPath);
				
	MFnCamera camFn(cameraPath);
				


	if(camFn.isOrtho())
	{

        MVector upDir = camFn.upDirection(MSpace::kWorld);
        MVector rightDir = camFn.rightDirection(MSpace::kWorld);
	
		
		MPoint point; MVector vec; short px, py;
		view.worldToView(MPoint(translation.x,translation.y,translation.z),px, py);
		
		int v_y = view.portHeight(), v_x = view.portWidth();
		
		


		if (upDir.isEquivalent(MVector::zNegAxis,0.01)) 
		{
			view.viewToWorld(px + (xOffset / bbScaleFactor) * v_x,py + (yOffset / bbScaleFactor),point,vec);
		} 
		else if (rightDir.isEquivalent(MVector::xAxis,0.01)) 
		{
			view.viewToWorld(px + (xOffset / bbScaleFactor) * v_x,py + (yOffset / bbScaleFactor) * v_y,point,vec);
		} 
		else  
		{
			view.viewToWorld(px,py + (yOffset /  bbScaleFactor) * v_y,point,vec);
		}
        
		numFn.setData(point.x,point.y,point.z);
		
	}
	else
	{
		numFn.setData(translation.x + xOffset,translation.y + yOffset,translation.z + zOffset);
	}

	mData = numData;

	return mData;
}

//------------------------------------------------------------
MManipData	moveManip::getPlugTranslation()
//------------------------------------------------------------
{
	MManipData	mData;
	MVector	vec;
	
	
	getConverterManipValue(freePointIndex,vec);

	MFnNumericData	numFn;
	MObject numData = numFn.create(MFnNumericData::k3Double);
	

	numFn.setData(vec.x,vec.y,vec.z);

	mData = numData;

	translation = vec;

	return mData;
}
//------------------------------------------------------------
void moveManip::draw(M3dView & view, 
					 const MDagPath & path, 
					 M3dView::DisplayStyle style,
					 M3dView::DisplayStatus status)
//------------------------------------------------------------
{ 
    MPxManipContainer::draw(view, path, style, status);
	
	//hier eventuell bestimmte parameter onscreen ausgeben
/*	view.beginGL(); 
	
	MPoint textPos;
	getConverterManipValue(freePointIndex,textPos);
    
    textPos.y -= 1;
    
    MString distanceText("Along Normal");
    view.drawText(distanceText, textPos, M3dView::kCenter);

	
    
    textPos.y += 5;
    
    distanceText = "Scale";
    view.drawText(distanceText, textPos, M3dView::kCenter);

    view.endGL(); */
}
