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


#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H


//INCLUDES
#include <maya/MString.h>
#include <maya/MGlobal.h>

#include <MACROS.h>

namespace BPT
{

/** Diese Klasse erlaubt die Verwendung der MainMayaProgressBar auf bequeme art und weise

*/
class progressBar
{

public:

	/** Konstruktor, der gleich einen Prozess startet
	*/
	// --------------------------------------------------
	progressBar( const MString& text, int maxCount);	
	// --------------------------------------------------

	/** Konstruktor, der gleich einen Prozess startet, allerdings mit StartCount
	*/
	// --------------------------------------------------
	progressBar( const MString& text, int startCount, int maxCount);	
	// --------------------------------------------------

	/** Konstruktor, der keinen Prozess startet
	*/
	// --------------------------------------------------
	progressBar( );	
	// --------------------------------------------------

	/** Bei der Destruktion werden alle laufenden Progressbars beendet
	*/
	// --------------------------------------------------
	~progressBar();
	// --------------------------------------------------

//-----------------------
//öffentliche Methoden
public:
//-----------------------

	/** Setzt den Progress auf den angegebenen Wert
	*/
	// --------------------------
	void set( int progress );
	// --------------------------

	/** Vergrößert den Progress um 1
	*/
	// ----------------
	void increment( );
	// ----------------

	/** Verkleinert den Progress um 1
	*/
	// ----------------
	void decrement( );
	// ----------------

	/** Startet die Progressbar neu
	*/
	// ----------------------------------------------------
	void begin( const MString& text, int startCount, int maxCount );
	// ----------------------------------------------------


	/** Beendet die Progressanzeige
	*/
	// ----------------
	void done( );
	// ----------------


//-----------------------
//private Variablen
private:
//-----------------------

	MString myText;
	int		startCount;
	int		maxCount;
	int		currentProgress;


};


}




#endif