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

// ByronsPolyToolsNodeManip.h: interface for the ByronsPolyToolsNodeManip class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(BYRONSPOLYTOOLSNODEMANIP)
#define BYRONSPOLYTOOLSNODEMANIP

#include <maya/MManipData.h>
#include <maya/MPxManipContainer.h>
#include <maya/MFnDirectionManip.h>
#include <maya/MFnDistanceManip.h>
#include <maya/MFnToggleManip.h>


#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/M3dView.h>
#include <maya/MFnNumericData.h>

#include <maya/MDagPath.h>
#include <maya/MVector.h>
#include <maya/MPoint.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include <iostream>

using std::cout;
using std::endl;


class ByronsPolyToolsNodeManip  : public	MPxManipContainer
{
public:
	ByronsPolyToolsNodeManip();
	virtual ~ByronsPolyToolsNodeManip();


    static void * creator();
    static MStatus initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject & node);

    virtual void draw(M3dView & view, 
					  const MDagPath & path, 
					  M3dView::DisplayStyle style,
					  M3dView::DisplayStatus status);
	

	MManipData toggleSideCallbackToPlug(unsigned index) ;	//die *ToManip Methoden dienen der Umwandlung der Werte von Node zu Manip
	MManipData toggleAbsCallbackToPlug(unsigned index) ;	

	MManipData toggleSideCallback(unsigned index) const;	//wandelt int von side in Bool um
	//MManipData toggleAbsCallback(unsigned index) const;		//wandelt int von Absolute in Bool um
	
	MManipData toggleStartCallback(unsigned index) ;
	MManipData toggle2StartCallback(unsigned index) ;	//toggleStartCB's holen die Bildschirmpositionen und setzen sie als startPunkte

	MManipData directionCallback(unsigned index) ;
	MManipData directionCallbackToPlug(unsigned index) ;
	MManipData startPointCallback(unsigned index);
	//ManipData toggleDirectionCallback(unsigned index) const;	//soll normalSlidewert holen: wenn negativ, flippen  Toggle
	//Ich kann auch gleich direction+length() auf 0 setzen und die Position (startPoint) dann vom Screen holen, ev. auch ohne Flip	
	MManipData	slideOnlyCallbackToPlug(unsigned index);	//direktverbindung zum Slide
	


	MVector getCamTranslation() const;
	MPoint	getLeftScreenPoint();		//holt punkt von linker screenSeite;
	MPoint	getRightScreenPoint();	//holt Punkt von rechter ScreenSeite;
	MPoint	getCenterScreenPoint();


    MDagPath	fDirectionManip;
	MDagPath	fSideToggle;
	MDagPath	fAbsToggle;		//toggleManip fuer abs/relative Slide
	MDagPath	fSlideManip;


	MObject		fNode;
	MDagPath	camPath;		//cam vom View
	M3dView		myView;			//view selbst


public:
    static MTypeId id;
};

#endif // !defined(AFX_BYRONSPOLYTOOLSNODEMANIP_H__E74233D4_F095_4B12_B04F_5477520631E4__INCLUDED_)
