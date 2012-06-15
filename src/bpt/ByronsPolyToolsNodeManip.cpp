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

// ByronsPolyToolsNodeManip.cpp: implementation of the ByronsPolyToolsNodeManip class.
//
//////////////////////////////////////////////////////////////////////

#include "ByronsPolyToolsNodeManip.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------
ByronsPolyToolsNodeManip::ByronsPolyToolsNodeManip()
//------------------------------------------------------------
{

}

//------------------------------------------------------------
ByronsPolyToolsNodeManip::~ByronsPolyToolsNodeManip()
//------------------------------------------------------------
{

}


MTypeId     ByronsPolyToolsNodeManip::id( 0x34568 );


//------------------------------------------------------------
void* ByronsPolyToolsNodeManip::creator()
//------------------------------------------------------------
{return new ByronsPolyToolsNodeManip;}



//------------------------------------------------------------
MStatus ByronsPolyToolsNodeManip::initialize()
//------------------------------------------------------------
{ 

    MStatus stat;
    stat = MPxManipContainer::initialize();
	
	return stat;
}


//------------------------------------------------------------
MStatus ByronsPolyToolsNodeManip::createChildren()
//------------------------------------------------------------
{
	MStatus	stat = MS::kSuccess;
	//directionManip


	fDirectionManip = addDirectionManip("fDirectionManip","direction");

	MFnDirectionManip dManipFn(fDirectionManip);
	dManipFn.setNormalizeDirection(false);

	

	fSideToggle = addToggleManip("sideToggleManip","side");

	MVector nullVector(0.0,0.0,0.0);
	
	MFnToggleManip	toggleFn(fSideToggle);
		toggleFn.setDirection( nullVector );
		toggleFn.setLength(0.0);

	
	fAbsToggle = addToggleManip("absToggleManip","absolute");
	MFnToggleManip	toggleFn2(fAbsToggle);
		toggleFn2.setDirection( nullVector );
		toggleFn2.setLength(0.0);


	fSlideManip = addDistanceManip("dinstanceManip","slide");
		MFnDistanceManip distFn(fSlideManip);
		distFn.setDirection( nullVector );
		

	return	stat;
}

//-------------------------------------------------------------------------
MStatus	ByronsPolyToolsNodeManip::connectToDependNode(const	MObject& node)
//-------------------------------------------------------------------------
{
	MStatus	stat;	//ist default success
	


		
	fNode = node;

	MFnDependencyNode	FnDagNode(node);
		

	//ich hole den View nur einmal: eventuell muss er bei jedem zugriff neu geholt werden ...
	myView = M3dView::active3dView();
	myView.getCamera(camPath);

	//erstmal die Toggles mit der Node verbinden
	//SIDE TOGGLE
	

	MFnToggleManip fnToggleManip(fSideToggle,&stat);
	

	MPlug	sidePlug = FnDagNode.findPlug("side",&stat);
	MPlug slidePlug = FnDagNode.findPlug("slide");
	MPlug normalPlug = FnDagNode.findPlug("alongNormal");
	
	if(stat != MS::kFailure)
	{
		//wenn plugToManip aktiv, dann ist toggle blockiert
	//	unsigned sideIndex = fnToggleManip.toggleIndex();
	//	cout<<"SideIndex = "<<sideIndex<<endl; 
	//	addPlugToManipConversionCallback(sideIndex,(plugToManipConversionCallback) 
//										 &ByronsPolyToolsNodeManip::toggleSideCallback);

		addManipToPlugConversionCallback(sidePlug,
										(manipToPlugConversionCallback) 
										 &ByronsPolyToolsNodeManip::toggleSideCallbackToPlug);

		unsigned startIndex = fnToggleManip.startPointIndex();
		//cout<<"StartIndex = "<<startIndex<<endl; 
		addPlugToManipConversionCallback(startIndex, 
										(plugToManipConversionCallback) 
										 &ByronsPolyToolsNodeManip::toggleStartCallback);

	}
	else
		cout<<"FAILURE: konnte plug nicht finden"<<endl;

	
	//TypeToggle



	//SlideDistance Manip
	MFnDistanceManip	distFn(fSlideManip,&stat);
	
	
	if(stat != MS::kFailure)
	{
		addManipToPlugConversionCallback(slidePlug,
									(manipToPlugConversionCallback) 
									&ByronsPolyToolsNodeManip::slideOnlyCallbackToPlug);


		
		unsigned startIndex = distFn.startPointIndex();
		addPlugToManipConversionCallback(startIndex, 
										(plugToManipConversionCallback) 
										 &ByronsPolyToolsNodeManip::startPointCallback);
	}

	

	//ABS TOGGLE
	MFnToggleManip fnToggleManip2(fAbsToggle);
	MPlug	absPlug = FnDagNode.findPlug("slideRelative",&stat);

	if(stat != MS::kFailure)
	{
	//	unsigned absIndex = fnToggleManip2.toggleIndex();
	//	addPlugToManipConversionCallback(absIndex,(plugToManipConversionCallback) 
	//									 &ByronsPolyToolsNodeManip::toggleAbsCallback);


		addManipToPlugConversionCallback(absPlug,(manipToPlugConversionCallback) 
										 &ByronsPolyToolsNodeManip::toggleAbsCallbackToPlug);

		int startIndex = fnToggleManip2.startPointIndex();
		addPlugToManipConversionCallback(startIndex, (plugToManipConversionCallback) 
										 &ByronsPolyToolsNodeManip::toggle2StartCallback);

	}
	
	//DIRECTION MANIPULATOR
	
	MFnDirectionManip fnDirection(fDirectionManip);
	
	//Callbacks hinzufügen:
	unsigned	startIndex = fnDirection.startPointIndex();
	addPlugToManipConversionCallback(startIndex,(plugToManipConversionCallback) 
								&ByronsPolyToolsNodeManip::startPointCallback);
	
	//cout<<"StartIndex2 = "<<startIndex<<endl; 
	unsigned directionIndex = fnDirection.directionIndex();

//	addPlugToManipConversionCallback(directionIndex,(plugToManipConversionCallback) 
//									&ByronsPolyToolsNodeManip::directionCallback);
	

	
	
	addManipToPlugConversionCallback(slidePlug,(manipToPlugConversionCallback) 
									&ByronsPolyToolsNodeManip::directionCallbackToPlug);

	
	
	
	//jetzt den PointTriad unter die Cam Parenten (ohne Kompensation)
	//MDagPath	transform = camPath;
//	transform.pop();
	
	//fnDirection.setTranslation(getCenterScreenPoint(),MSpace::kWorld);

	//fnDirection.translateBy(MVector(0.0,0.0,-10.5),MSpace::kTransform);

	//MFnDagNode	dagFn(transform);
	//dagFn.addChild(fnDirection.object());
	//dagFn.removeChild(fnDirection.object());

//	cout<<"!!!!!!!!!!!!!!!!!!!!3!!!!!!!!!!!!!!!!!!!"<<endl;

	//finishing
	stat = finishAddingManips();
	if(stat == MS::kFailure)
		cout<<"FINISHING FEHLGESCHLAGEN"<<endl;

//	stat = MPxManipContainer::connectToDependNode(node);
	if(stat == MS::kFailure)
		cout<<"Connect to dependNode FEHLGESCHLAGEN"<<endl;
	


	return stat;
}






//-------------------------------------------------------------------------------------------
//									CALLBACKS UND HELFER
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------
MManipData	ByronsPolyToolsNodeManip::slideOnlyCallbackToPlug(unsigned index)
//-------------------------------------------------------------------------
{
	MFnDistanceManip	distFn(fSlideManip);
	int distanceIndex = distFn.distanceIndex();

	double distanceVal;
	getConverterManipValue(distanceIndex,distanceVal);


	return MManipData(distanceVal);

}



//----------------------------------------------------
MVector ByronsPolyToolsNodeManip::getCamTranslation() const
//----------------------------------------------------
{
	//müsste so eigentlich funzen, wenn nicht, dann halt so wie im Beispiel
	MDagPath	path = camPath;

	path.pop();

	MFnTransform	fnTrans(path);
	return	fnTrans.translation(MSpace::kWorld);
}


//----------------------------------------------------
MPoint	ByronsPolyToolsNodeManip::getLeftScreenPoint()
//----------------------------------------------------
{
	MPoint	point; short x,y; MVector vec;
	MFnDirectionManip	dfn(fDirectionManip);
	getConverterManipValue(dfn.endPointIndex(),point);

	myView.worldToView(point,x,y);
	myView.viewToWorld(x-40,y+40,point,vec);

	 

	
	
	return point + 2 * vec;
}


//----------------------------------------------------
MPoint	ByronsPolyToolsNodeManip::getRightScreenPoint()
//-----------------------------------------------------
{


	//diese Funktion sollte sich später noch daten der Node holen. so dass die togglePositions so
	//gewählt werden, dass sie niemals mit dem DirectionManip interferieren



	MPoint	point; short x,y; MVector vec;
	MFnDirectionManip	dfn(fDirectionManip);
	getConverterManipValue(dfn.endPointIndex(),point);

	myView.worldToView(point,x,y);
	myView.viewToWorld(x+40,y+40,point,vec);

	 

	
	
	return point + 2 * vec;
}


//----------------------------------------------------
MPoint	ByronsPolyToolsNodeManip::getCenterScreenPoint()
//-----------------------------------------------------
{
	int breite = myView.portWidth();
	int hoch = myView.portHeight();

	//diese Funktion sollte sich später noch daten der Node holen. so dass die togglePositions so
	//gewählt werden, dass sie niemals mit dem DirectionManip interferieren

	breite = (breite - (breite % 2))/2;
	hoch = (hoch - (hoch % 2))  / 2;

	MPoint	p;
	MVector	vec;

	myView.viewToWorld(breite,hoch,p,vec);
	
	return p+2*vec;
}


//--------------------------------------------------------------------------------
MManipData ByronsPolyToolsNodeManip::toggleSideCallbackToPlug(unsigned index)
//--------------------------------------------------------------------------------
{

	MFnToggleManip	fnToggle(fSideToggle);

	return MManipData((int)fnToggle.toggle());

}


//--------------------------------------------------------------------------------
MManipData ByronsPolyToolsNodeManip::toggleAbsCallbackToPlug(unsigned index)
//--------------------------------------------------------------------------------
{

	MFnToggleManip	fnToggle(fAbsToggle);

	return MManipData((int)fnToggle.toggle());

}


//--------------------------------------------------------------------------------
MManipData ByronsPolyToolsNodeManip::toggleSideCallback(unsigned index) const
//--------------------------------------------------------------------------------
{
	MFnDependencyNode	dNode(fNode);
	MPlug sidePlug = dNode.findPlug("side");

	int i;
	sidePlug.getValue(i);
	return	MManipData((bool)i);
}

//-------------------------------------------------------------------------
MManipData ByronsPolyToolsNodeManip::toggleStartCallback(unsigned index)
//-------------------------------------------------------------------------
{
	MFnNumericData	numData;
	MObject	dataObj = numData.create(MFnNumericData::k3Double);
	MVector vec = getRightScreenPoint();

	//cout<<"Bin in Toggle Start"<<" STARTPUNKT IST " <<vec.x<<" "<<vec.y<<" "<<vec.z<<endl;


	numData.setData(vec.x,vec.y,vec.z);
	//numData.setData(0.0,0.0,0.0);

	return MManipData(dataObj);
}


//-------------------------------------------------------------------------
MManipData ByronsPolyToolsNodeManip::toggle2StartCallback(unsigned index)
//-------------------------------------------------------------------------
{
	MFnNumericData	numData;
	MObject	dataObj = numData.create(MFnNumericData::k3Double);
	MVector vec = getLeftScreenPoint();

	//cout<<"Bin in Toggle Start"<<" STARTPUNKT IST " <<vec.x<<" "<<vec.y<<" "<<vec.z<<endl;

	numData.setData(vec.x,vec.y,vec.z);
	//numData.setData(0.0,0.0,0.0);

	return MManipData(dataObj);
}


//--------------------------------------------------------------------------------
MManipData	ByronsPolyToolsNodeManip::startPointCallback(unsigned index)
//--------------------------------------------------------------------------------
{
	MFnNumericData	numData;
	MObject	dataObj = numData.create(MFnNumericData::k3Double);

	MVector t = getCenterScreenPoint();

	numData.setData(t.x,t.y,t.z);
	return	MManipData(dataObj);

}

//---------------------------------------------------------------------
MManipData	ByronsPolyToolsNodeManip::directionCallback(unsigned index)
//---------------------------------------------------------------------
{
	//cout<<"BIN IN DIRECTION CALLBACK"<<endl;
	//slide und normal werte holen
	MFnDirectionManip	dManip(fDirectionManip);
	MFnDependencyNode	dagFn(fNode);
	
	int breite = myView.portWidth();
	int hoch = myView.portHeight();

	MPlug	slidePlug = dagFn.findPlug("slide");
	MPlug	normal = dagFn.findPlug("alongNormal");
	double	slide,normalVal;

	slidePlug.getValue(slide);
	normal.getValue(normalVal);


	//hier müssen die Werte eigenticht nur zurückgerechnet werden.
	int newX = slide * breite;
	int newY = 100 * normalVal + hoch/2;

	
	MPoint	startPoint;
	getConverterManipValue(dManip.startPointIndex(),startPoint);
	//MVector	vec;
	//getConverterManipValue(index,vec);

	MPoint newPos;MVector newVec;
	myView.viewToWorld(newX,newY,newPos,newVec);

	MFnNumericData	numData;
	MObject	data = numData.create(MFnNumericData::k3Double);

	newVec = newPos - startPoint;

	numData.setData(newVec.x,newVec.y,newVec.z);
	

	return	MManipData(data);


}


//---------------------------------------------------------------------
MManipData	ByronsPolyToolsNodeManip::directionCallbackToPlug(unsigned index)
//---------------------------------------------------------------------
{
	//cout<<"BIN IN DIRECTIONCALLBACK TO PLUG"<<endl;
	//slide und normal werte holen
	MFnDirectionManip	dManip(fDirectionManip);

	MVector vec;
	//getConverterManipValue(dManip.directionIndex(),vec);
	getConverterManipValue(dManip.endPointIndex(),vec);

	short x,y,breite,hoch;
	myView.worldToView(vec,x,y);
	breite = myView.portWidth();
	hoch = myView.portHeight();

	double newSlide = ((float)x / (float)breite);
	double newNormal = ((float)y - (float)hoch/2 ) / 100;

	MFnDependencyNode	dagFn(fNode);

	////MPlug slidePlug = dagFn.findPlug("slide");
	MPlug normal = dagFn.findPlug("alongNormal");
	

	//slidePlug.setValue(vec.x);
	//normal.setValue(vec.y);
	normal.setValue(newNormal);

	/*
	//vec.x umrechnen
	double	s = vec.x;
	const double	divider = 2;

	s += divider/2;
	s /= divider;

	if(s > 1.0)
		s = 1.0;
	else if(s < 0)
		s = 0;
	 
	*/
	//ist eigentlich unnötig, vielleicht kann man auch leeres MManipDataObj  übergeben
	
	//return	MManipData(s);
	return	MManipData(newSlide);

}


//--------------------------------------------------------------------------------
void ByronsPolyToolsNodeManip::draw(M3dView & view, 
								 const MDagPath &path, 
								 M3dView::DisplayStyle style,
								 M3dView::DisplayStatus status)
//--------------------------------------------------------------------------------
{ 

    MPxManipContainer::draw(view, path, style, status);


	 view.beginGL(); 
	//view.beginSelect();

    char str[100];
    MString distanceText;
   

    sprintf(str, "    Side");
	distanceText = str;

	view.drawText(distanceText, getRightScreenPoint(), M3dView::kLeft);
	

	sprintf(str, "sideRelative    ");
	distanceText = str;

	view.drawText(distanceText, getLeftScreenPoint(), M3dView::kRight);
	


//	view.endSelect();
	view.endGL();
}
