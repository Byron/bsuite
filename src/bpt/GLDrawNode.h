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

// GLDrawNode.h: interface for the header class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GLDRAWNODE_H
#define GLDRAWNODE_H



// -----------------
//	Maya Includes
// -----------------

#include <maya/M3dView.h>

// -----------------
//	Eigene Includes
// -----------------

// -----------------
//	Diverse Includes
// -----------------
#include <GLDrawNodeBase.h>



// Diese Node enthlt alles was man braucht, um mit openGl zu zeichnen
//
#include <GL/glu.h>


namespace BPT
{


/* *	Hierbei handelt es sich um eine abstrakte Klasse, welche sich bei Byronimo ( dem Verteiler fuer drawEvents )
		registriert.

		@remarks
			So wird es einer normalen dependNode ermglicht, in den viewPort zu zeichnen.
			All dies geschieht intern, so dass keine Fehlertrchtigen connections gemacht werden muessen
*/

//#################################################
//-------------------------------------------------
//	KLASSENBESCHREIBUNG
	class GLDrawNode	: public GLDrawNodeBase
//-------------------------------------------------
//#################################################
{

//------------------------------
// Konstruktion/Destruktion
public:
//------------------------------
	
	/** Registriert sich bei den BGlobals mit seinem Pointer, so dass von dort aus dann drawCommands ueber
		den Pointer an die abgeleitete Klasse gesendet werden knnen.
	*/
	GLDrawNode(){
					// BGlobals::drawNodeQueue.insert(this);
				};


	/** Deregistriert sich bei den BGlobals
	*/
	~GLDrawNode(){
					// BGlobals::drawNodeQueue.erase(this);
				};
	

//------------------------------
// Methoden
public:
//------------------------------

	/* * Zeichnet die abgeleitete Klasse

	*/
	virtual void	draw( M3dView& view) = 0;



};

}

#endif 
