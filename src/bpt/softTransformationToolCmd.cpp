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

#include "softTransformationToolCmd.h"



//******************************************************************************************************************
//*********************************
//MPX_TOOL_COMMAND METHODEN
//*********************************
//******************************************************************************************************************

//-------------------------------------------------------------
void* softTransformationToolCmd::creator()
//-------------------------------------------------------------
{
	return new softTransformationToolCmd;
}

int softTransformationToolCmd::counter = 0;

//-------------------------------------------------------------
MStatus softTransformationToolCmd::finalize()
//-------------------------------------------------------------
{
	MArgList	command;
	command.addArg(MString("SoftTransformationTool"));

	return	MPxToolCommand::doFinalize(command);
}


//------------------------------------------------------------
MStatus softTransformationToolCmd::doIt(const MArgList &args)
//------------------------------------------------------------
{	
	return MS::kSuccess;
}

//------------------------------------------------------------
MStatus	softTransformationToolCmd::undoIt()
//------------------------------------------------------------
{
	INVIS(cout<<"War in UndoIt: "<<number<<endl;)

	//undo kann immer gleich ausgeführt werden
	if(indexArray.length() == 0)
		return MS::kSuccess;

	MFnDependencyNode depNodeFn(meshPath.node());
	MPlug tweaks = depNodeFn.findPlug("pnts");
	
	int l = indexArray.length();
	MPlug pnt;
	MFnNumericData	numFn;
	for(int i = 0; i < l; i++)
	{
		pnt = tweaks.elementByLogicalIndex(indexArray[i]);
		MObject value;
		pnt.getValue(value);
		
		numFn.setObject(value);

		float one,two,three;
		numFn.getData(one,two, three);

		
		MObject data = numFn.create(MFnNumericData::k3Float);
		
		MVector vec = undoTweaks[i];
		numFn.setData(float(one-vec.x),float(two-vec.y),float(three-vec.z));

		pnt.setValue(data);
	}


	return	MS::kSuccess;
}

//------------------------------------------------------------
MStatus	softTransformationToolCmd::redoIt()
//------------------------------------------------------------
{
	INVIS(cout<<"War in REdoIt: "<<number<<endl;)
	
	//redo nur ausführen, wenn man sich nicht im softTransformationTool befindet
/*	
	MString resultStr;
	MGlobal::executeCommand(MString("currentCtx"),resultStr,false,false);
	
	
	cout<<resultStr<<" = Result"<<endl;

	if(resultStr == "softTransformationTool1")
		return MS::kSuccess;

*/	

//	if(number == counter - 1)	//niemals redo machen, wenn noch gültige softSelNode besteht
//		return MS::kSuccess;	//wobei er zu diesem ZeitPunkt eh noch keine undoInfo hat

	MFnDependencyNode depNodeFn(meshPath.node());
	MPlug tweaks = depNodeFn.findPlug("pnts");
	
	int l = indexArray.length();
	MPlug pnt;
	MFnNumericData	numFn;
	for(int i = 0; i < l; i++)
	{
		pnt = tweaks.elementByLogicalIndex(indexArray[i]);
		MObject value;
		pnt.getValue(value);
		
		numFn.setObject(value);

		float one,two,three;
		numFn.getData(one,two, three);

		
		MObject data = numFn.create(MFnNumericData::k3Float);
		
		MVector vec = undoTweaks[i];
		numFn.setData(float(one+vec.x),float(two+vec.y),float(three+vec.z));

		pnt.setValue(data);
	}


	return	MS::kSuccess;
}


//------------------------------------------------------------
bool	softTransformationToolCmd::isUndoable() const
//------------------------------------------------------------
{
	return true;
}


