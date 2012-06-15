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

#ifndef GLDRAWNODEBASE_H
#define GLDRAWNODEBASE_H



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




// Diese Node enthält alles was man braucht, um mit openGl zu zeichnen
//
#include <GL/glu.h>


namespace BPT
{


/* *	Ne völlige Unsinnsklasse: Diese Base wird von den Globals included, während sich die GLDRAWNODE die Globals holt
*/

//#################################################
//-------------------------------------------------
//	KLASSENBESCHREIBUNG
	class GLDrawNodeBase
//-------------------------------------------------
//#################################################
{

//------------------------------
// Konstruktion/Destruktion - nur standard
public:
//------------------------------
	

	

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
