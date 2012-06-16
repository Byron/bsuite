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

#include "softTransformationToolContextCmd.h"




//hier befinden sich die definitionen zu allen befehlen

//*****************************************
//***************************
//softT contextCommand
//***************************
//***************************************

MPxContext * softTransformationToolCtxCommand::makeObj()
{
	context = new softTransformationToolCtx;
	return context;
	//return new softTransformationToolCtx;
}

//-------------------------------------------------------------
void*		softTransformationToolCtxCommand::creator()
//-------------------------------------------------------------
{
	return new softTransformationToolCtxCommand;
}

//-------------------------------------------------------------
MStatus		softTransformationToolCtxCommand::appendSyntax()
//-------------------------------------------------------------
{
	MSyntax mySyntax = syntax();

	mySyntax.addFlag("ft","falloffType", MSyntax::kLong);
	mySyntax.addFlag("uld","useLastDistance", MSyntax::kBoolean);
	mySyntax.addFlag("vf","visualize", MSyntax::kBoolean);
	mySyntax.addFlag("nir","normalIsRelative", MSyntax::kBoolean);
	mySyntax.addFlag("rdc","relativeDistanceChange", MSyntax::kDouble);
	mySyntax.addFlag("rmc","relativeMaskChange", MSyntax::kLong);

	return MS::kSuccess;
}

//-------------------------------------------------------------
MStatus		softTransformationToolCtxCommand::doEditFlags()
//-------------------------------------------------------------
{
	MArgParser argData = parser();
	int fi;

	if(argData.isFlagSet("ft"))
	{
		argData.getFlagArgument("ft",0,fi);
		
		
			context->fType = fi;
	}

	bool arg;
	if(argData.isFlagSet("uld"))
	{
		
		argData.getFlagArgument("uld",0,arg);
		context->useLastDistance = arg;
	}

	if(argData.isFlagSet("vf"))
	{
		
		argData.getFlagArgument("vf",0,arg);
		context->fVis = arg;
	}

	if(argData.isFlagSet("nir"))
	{
		
		argData.getFlagArgument("nir",0,arg);
		context->nirFlag = arg;
	}
	
	if(argData.isFlagSet("rdc"))
	{
		
		argData.getFlagArgument("rdc",0,arg);
		if(!context->changeDistance(arg))
			MGlobal::displayWarning("You cannot change the distance as long the manipulator dsn't show up");
	}

	if(argData.isFlagSet("rmc"))
	{
		int arg;
		argData.getFlagArgument("rmc",0,arg);
		if(!context->changeMask(arg))
			MGlobal::displayWarning("You cannot change the maskSize as long the manipulator dsn't show up");
	}

	return MS::kSuccess;

}

//-------------------------------------------------------------
MStatus		softTransformationToolCtxCommand::doQueryFlags()
//-------------------------------------------------------------
{
	setResult(MString("Not implemented"));
	return MS::kSuccess;
}




