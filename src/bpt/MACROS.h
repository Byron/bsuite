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


//DIESE DATEI ENTHAELLT AUSSCHLIESSLICH MAKROS UND WIRD VON JEDER ANDEREN HEADER DATEI EINGEBUNDEN

#ifndef MACRO_FILE
#define MACRO_FILE


// MATHE
//
#define	PI		3.14159265358979323846	/* pi */
#define	PI2		1.57079632679489661923	/* pi/2 */

// IOStream includes - ist absolut ntig
//
#ifndef REQUIRE_IOSTREAM
	#include <iostream>
	using std::cout;
	using std::endl;
#endif

// PRAGMA DISABLES
// ----------------

#ifdef WIN32
#pragma warning( disable : 4010 )	// Die nervigen singleLine CommentWarnings ausschalten
#pragma warning( disable : 4786 )	// Die nervigen TemplateStringTruncation Warnings ausschalten
#pragma warning (disable : 4251 )	// Template BaseClass without dll interface - AUS
#pragma warning (disable : 4275 )	//"Diese variable braucht ein dll interface - nak nak " - AUS
#endif



// ------------------------
// Typedefs
// ------------------------
typedef unsigned short ushort;

// ------------------------
// Namespaces
// ------------------------
namespace BPT{}

using namespace BPT;



#ifdef TIMER 
#include "timer.h"
#	define SPEED(message)	timer myTimer(message)
#	define NEWSPEED(message) timer* myTimer = new timer(message)
#	define DELETESPEED delete myTimer;
#else
#	define SPEED(message)
#	define NEWSPEED(message)
#	define DELETESPEED
#endif



#ifdef DEBUG
#	define MPRINT(message) cout<<message<<endl; 
#	define INVIS(code) code
#	define MCheckStatus(status,message)			\
		if( MS::kSuccess != status ) {			\
			MString error("Status failed: ");	\
			error += message;					\
			MGlobal::displayError(error);		\
			return status;						\
		}
#else
#	define MPRINT(message)
#	define INVIS(code)
#	define MCheckStatus(status,message)	
#endif



//*****************************
// GENERELLE DEFINES/ TYPEDEF
//*****************************

typedef	unsigned int uint;


//************************
//LINUX MODIFIKATIONEN
//************************

#ifndef WIN32
//das hier ist eigentlich ABSOLUTER UNFUG - wieso immer ULONG schreiben und es ersetzen lassen, wenn man auch gleich ulong verwenden kann..
//naja, so hat man immerhin maximale Kontrolle ueber das, was ULONG wirklich ist
	typedef unsigned long ULONG;
	typedef ULONG ulong;
	typedef unsigned int UINT;
	typedef unsigned char UCHAR;

#endif


//*****************************
// EXPIRES DEKLARATIONEN
//*****************************
#ifdef EXPIRES

// Gibt die Zeit zurueck, die die version noch gueltig ist
//
uint	checkExpires();
void	createExpireFile();
bool	createExpireFileToo();
uint	checkExpireOptionVars();
#endif



#endif // FILE
