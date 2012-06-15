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

#ifndef BPT_HELPERS_INCLUDE
#define BPT_HELPERS_INCLUDE


#include <maya/MIntArray.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>
#include "BPT_BA.h"


//EINFACHE HELPER KLASSE ALS KAPSEL FÜR OFT VERWENDETE FUNKTIONEN

namespace BPT
{

/** Einfache KapselKlasse, welche hilfsmethoden beinhaltet und von vielen BPT Klassen verwendet wird

*/
class BPT_Helpers
{
public:
	BPT_Helpers(void);
	~BPT_Helpers(void);


public:	
	//METHODEN


void		sortArray(MIntArray& array) const;

bool		isOnArray(int valueToLookFor,const MIntArray& searchThisArray) const;

bool		getLocIndex(int value,const MIntArray& array, UINT& localIndex) const;
bool		getLocIndex(int value,const MIntArray& array, UINT& localIndex, int lowBound, int highBound) const;

bool		getLocIndexMod2(int value,const MIntArray& array, UINT& localIndex) const;
bool		getLocIndexMod2(int value,const MIntArray& array, UINT& localIndex, int lowBound, int highbBound) const;

bool		isOnArray(int value,const MIntArray& array, int lBound, int hBound) const;

void		addIntArrayToLHS(MIntArray& LHS,const MIntArray& RHS) const;

bool		UVSetDiffers(const MIntArray& UVs1, const MIntArray& UVs2) const;	//vergleicht die enthaltetenen UVs paarweise, und gibt true zurück, wenn sie sich unterscheiden

void		arrayRemove(MIntArray& lhs,MIntArray& rhs, bool affectLhsOnly = false);


ulong		genKey(const MString& string);


//	gibt IntArray mit IDs der doppelten Indices, OBSOLETE
MIntArray	pruneIntArray(MIntArray& orig, MIntArray& pruned);
//	Überladener PrundAlgorhytmus, der direkt auf array arbeitet

void		pruneIntArray(MIntArray& orig);

void		invertArray(MIntArray& array);

void		invertArray(MIntArray& array, int lBound, int rBound);


//---------------------
//SPEICHERLASTIGES
//---------------------

MIntArray	matchArray(	MIntArray lhs, 
						MIntArray rhs) const;

MIntArray	memoryMatch(MIntArray lhs, 
						MIntArray rhs);

void		memoryPrune(MIntArray& orig);

void		memoryPrune(MIntArray& orig, 
						MIntArray& doppelte);

void		memoryInvertSelection(MIntArray& lhs, 
								  int maxElement, 
								  MIntArray& inverted);

void		memoryArrayRemove(MIntArray& removeFrom,
							  const MIntArray& removals);

//DEBUG PROCS
//#ifdef DEBUG

	void		printArray(const MIntArray& array, MString message);

//#endif



///////////////////////
private: //VARIABLEN
	//////////////////


};


}

#endif
