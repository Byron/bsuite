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


#include <progressBar.h>


//CONSTRUCTION/DESTRUCTION

//----------------------------------------------------------------
progressBar::progressBar( )
: 
currentProgress(0)
//----------------------------------------------------------------
{}

//----------------------------------------------------------------
progressBar::progressBar(const MString& text, const int inMaxCount)
: 
myText(text), 
startCount(0),
maxCount(inMaxCount),
currentProgress(0)
//----------------------------------------------------------------
{
	begin( text, startCount, inMaxCount );
}


//----------------------------------------------------------------
progressBar::progressBar(const MString& text, int inStartCount, int inMaxCount)
: 
myText(text), 
startCount(inStartCount),
maxCount(inMaxCount),
currentProgress(0)
//----------------------------------------------------------------
{
	begin( text, startCount, inMaxCount );
}


//----------------------------------------------------------------
progressBar::~progressBar()
//----------------------------------------------------------------
{

	done();

}


//----------------------------------------------------------------
void progressBar::set( int progress )
//----------------------------------------------------------------
{
	
	currentProgress = progress;

	MString outStr(" global string 	$gMainProgressBar; 	progressBar -edit -pr "); 
	outStr += currentProgress;
	outStr += " -status \"";
	outStr += myText;
	outStr += " ( ";
	outStr += progress;
	outStr += " / ";
	outStr += maxCount;
	outStr += " ) \" $gMainProgressBar;";


	MGlobal::executeCommand(outStr, false, false); 

}



// ------------------------
void 
progressBar::increment( )
// ------------------------
{
	set( ++currentProgress );
}

// ----------------
void 
progressBar::decrement( )
// ----------------
{
	set( --currentProgress );
}


// ----------------------------------------------------
void 
progressBar::begin( const MString& text,int inStartCount, int inMaxCount )
// ----------------------------------------------------
{
	myText = text;
	maxCount = inMaxCount;
	startCount = inStartCount;
	currentProgress = inStartCount;
	
	MString outStr(" global string 	$gMainProgressBar; 	progressBar -edit -beginProgress -isInterruptable false	-status " + MString("\"") + myText + MString("\"") + " -maxValue " + maxCount + " -minValue " + startCount +  " $gMainProgressBar; ");
	MGlobal::executeCommand(outStr, false, false);
}


// ----------------
void 
progressBar::done( )
// ----------------
{
	MGlobal::executeCommand( (" global string 	$gMainProgressBar; 	progressBar -edit -ep $gMainProgressBar; "), false, false );
}
