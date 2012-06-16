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

#include <timer.h>


//-----------------------------
timer::timer(const string& message)			
//-----------------------------
{
	messageStr = message;
	
	#ifdef WIN32

	/*
	cout<<endl<<endl;
	cout<<"----------------------------"<<endl;
	cout<<"... Beginne Zeitmessung ... "<<endl;
	cout<<message.asChar()<<endl;
	cout<<"----------------------------"<<endl<<endl;
	*/

	//Frequenz holen, um in sekunden umzurechnen
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	//cout<<"STARTZEIT: "<< ( (double)start.QuadPart / (double)freq.QuadPart) <<endl;
	#else
	gettimeofday(&startTime, NULL);
	#endif 
}

//-----------------------------
timer::~timer(void)
//-----------------------------
{
	#ifdef WIN32
	
	_LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	double elapsedTime = ((double)end.QuadPart - (double)start.QuadPart) / (double)freq.QuadPart;
	
	#else

	struct timeval now;
    gettimeofday(&now, NULL);
	unsigned long elapsedTime =
		(now.tv_sec - startTime.tv_sec) * 1000 + ( now.tv_usec - startTime.tv_usec ) / 1000;
	#endif
}

