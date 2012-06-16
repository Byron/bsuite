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

#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#include <MACROS.h>
#include <iostream>
#include <string>
#include <time.h>

#ifdef WIN32
#include "windows.h"
#else
#include <sys/time.h>
#endif 



using std::cout;
using std::endl;
using std::string;




/** Diese Klasse misst die Zeit vo Beginn ihrer Erzeugzung bis zur zerstoerung und gibt dise aus
	
	Sie funktioniert nur unter win32, Momentan.

	@note
		Sie wird exportiert und so generell verfügbar gemacht.

	@todo
		Einen PerformanceManager bauen, welcher es ermoeglicht, die Zeít zu messen, die in den einzelnen
		gemonitorten Prozeduren verbraucht wird.
*/

// -----------------------------
class timer
// -----------------------------
{

// /////////////////////////////
//			Konstruktion	 //
public: //***************** //
// //////////////////////////

	timer(const string& message);			 
	~timer(void);


// ////////////////////////
//			Variablen   //
private: //*********** //
// /////////////////////
	std::string messageStr;

#ifdef WIN32
	_LARGE_INTEGER start;
	_LARGE_INTEGER freq;	//frequenz des Counters
#else
	struct timeval startTime;
#endif // WIN32

	
};




#endif /*timer included*/
