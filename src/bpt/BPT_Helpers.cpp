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

#include "BPT_Helpers.h"
#include "timer.h"



BPT_Helpers::BPT_Helpers(void)
{
}

BPT_Helpers::~BPT_Helpers(void)
{
}


//---------------------------------------------------------
void		BPT_Helpers::sortArray(MIntArray& array) const
//---------------------------------------------------------
{
	int tmp;
	int l = array.length();
	

	//einmal von vorn durchlaufen
	for(int i = 0; i < l; i++ )
	{
		for(int a = i+1;a<l;a++)
		{
			if(array[a] < array[i])
			{
				tmp = array[i];
				array[i] = array[a];
				array[a] = tmp;
				//break;
			}
		}
	}

}

/// Erzeugt einen spezifischen Key aus dem UEbergebenen string
//
//  ----------------------------------------------------
ulong	BPT_Helpers::genKey(const MString& string)
//  ----------------------------------------------------
{
	const char* chars = string.asChar();
	uint length =		string.length();

	ulong key = 0;
	ulong sum = 0;

	// Jetzt einen key errechnen wie folgt:
	// Den ascii code eines jeden chars multiplizieren mit dem index seiner position im string
	// zuguterletzt noch die summer der asciies selbst hinzuaddieren

	for(uint i = 0; i < length; i++)
	{
		key += chars[i] * i;
		sum += chars[i];
	}

	key += sum;

	return key;
};


//------------------------------------------------------------------------------
bool		BPT_Helpers::isOnArray(int value,const MIntArray& array) const
//------------------------------------------------------------------------------
{
	int l = array.length();

	for(int i = 0; i < l; i++)
		if(array[i] == value)
			return true;
	return false;
}


//------------------------------------------------------------------------------
bool		BPT_Helpers::getLocIndex(int value,const MIntArray& array, UINT& localIndex) const
//------------------------------------------------------------------------------
{//wird benoetigt für normals (meshCreator)
	 int l = array.length();
	

	for( int i = 0; i < l; i++)
		if(array[i] == value)
		{
			localIndex = i;
			return true;
		}

	return false;
}


//------------------------------------------------------------------------------
bool		BPT_Helpers::getLocIndex(int value,const MIntArray& array, UINT& localIndex, int lowBound, int highBound) const
//------------------------------------------------------------------------------
{//wird benoetigt für normals (meshCreator)


	for(int i = lowBound; i < highBound; i++)
		if(array[i] == value)
		{
			localIndex = i;
			return true;
		}

	return false;
}


//------------------------------------------------------------------------------
bool		BPT_Helpers::getLocIndexMod2(int value,const MIntArray& array, UINT& localIndex) const
//------------------------------------------------------------------------------
{//wird benoetigt für normals (meshCreator)
	//achtet ausserdem darauf, dass das Ergebnis durch 2 teilbar ist
	 int l = array.length();

	for( int i = 0; i < l; i += 2)
		if(array[i] == value )
		{
			localIndex = i;
			return true;
		}

	return false;
}

//------------------------------------------------------------------------------
bool		BPT_Helpers::getLocIndexMod2(int value,const MIntArray& array, UINT& localIndex, int lowBound, int highBound) const
//------------------------------------------------------------------------------
{//wird benoetigt für normals (meshCreator)
	//achtet ausserdem darauf, dass das Ergebnis durch 2 teilbar ist


	for(int i = lowBound; i < highBound; i += 2)
		if(array[i] == value )
		{
			localIndex = i;
			return true;
		}

	return false;
}



//---------------------------------------------------------------------------------------
bool		BPT_Helpers::isOnArray(int value,const MIntArray& array, int lBound, int hBound) const
//---------------------------------------------------------------------------------------
{//extra für meshCreator
	for(int i = lBound; i < hBound; i++)
		if(array[i] == value)
			return true;
	return false;

}

//-----------------------------------------------------------------
void		BPT_Helpers::addIntArrayToLHS(MIntArray& LHS, const MIntArray& RHS) const
//-----------------------------------------------------------------
{
	/*
	int l = RHS.length();
	
	LHS.setLength(LHS.length() + l);

	for(int i = 0; i < l; i++)
		LHS.append(RHS[i]);
	*/

	//EFFIZIENZSTEIGERUNG: Speicher wird nur einmal allokalisiert
	int l = RHS.length();
	int i = LHS.length();
	unsigned int r = 0;

	LHS.setLength(i + l);
	l = l + i;

	for(; i < l; i++)
		LHS[i] = RHS[r++];

}





//-----------------------------------------------------------------
void		BPT_Helpers::arrayRemove(MIntArray& lhs,MIntArray& rhs, bool affectLhsOnly)
//-----------------------------------------------------------------
{

	//diese Methode geht davon aus, dass die Arrays keine doppelten
	//Einträge haben. Wenn dies zu erwarten ist, also vorher prunen.
	
	
	int leftL = lhs.length();
	int rightL = rhs.length();

	if(leftL + rightL < 200 && !affectLhsOnly)

	{//wenn maximal 10000 iterationen vonnoeten sind

		int i = 0;
		int x = 0;

		int ROffset,LOffset;
		ROffset = 0;
		LOffset = 0;

		for(;i < leftL - LOffset; i++)
		{
			x = 0;
			for(;x < rightL - ROffset; x++)
			{
				if(lhs[i] == rhs[x])
				{
					rhs.remove(x--);
					lhs.remove(i--);
					ROffset++;
					LOffset++;
					break;
				}
			}
		}

	}
	else
	{//ansonsten MemoryMatch verwenden

		BPT_BA removeFromBA(lhs,true);
		BPT_BA removalsBA(rhs,true);
		

		lhs.clear();
		(removeFromBA - removalsBA).getIntArray(lhs);

	}

}



//-----------------------------------------------------------------
void		BPT_Helpers::memoryArrayRemove(MIntArray& removeFrom,const MIntArray& removals)
//-----------------------------------------------------------------
{
	

	BPT_BA removeFromBA(removeFrom,true);
	BPT_BA removalsBA(removals,true);

	removeFrom.clear();
	(removeFromBA - removalsBA).getIntArray(removeFrom);
	
}

//OBSOLETE
//-----------------------------------------------------------------
MIntArray		 BPT_Helpers::pruneIntArray(MIntArray& orig,MIntArray& pruned)
//-----------------------------------------------------------------
{
	MIntArray matches;
	bool isMatch = 0;
	bool isInMatches = 0;

	int lOrig = orig.length();
	
	for(int i = 0; i < lOrig; i++)
	{
		isMatch = 0;
		isInMatches = 0;
		
		int lPruned = pruned.length();

		for(int a = 0; a < lPruned; a++)
		{
			if(orig[i] == pruned[a])
			{
				isMatch = 1;
				int lMatches = matches.length();
				
				for(int b = 0; b < lMatches; b++)
				{
					if(orig[i] == matches[b])
					{
						isInMatches = 1;
						break;
					}
				}
				
				if(!isInMatches)
					matches.append(orig[1]);

				break;
			}
		}
		
		if(!isMatch)
			pruned.append(orig[i]);
	}



	return matches;
}


//-----------------------------------------------------------------
void			BPT_Helpers::pruneIntArray(MIntArray& orig)
//-----------------------------------------------------------------
{
	
	int l = orig.length();
	int lOffset = 0;
	int x;

	for(int i = 0; i < l-lOffset; i++)
	{
		x = i + 1;
		for(; x < l-lOffset;x++)
		{
			if(orig[i] == orig[x])
			{
				orig.remove(x--);	
				lOffset++;
			}
		}
	}
}



//-----------------------------------------------------------------
void		BPT_Helpers::memoryPrune(MIntArray& orig)
//-----------------------------------------------------------------
{
	BPT_BA mem(orig,true);
	orig.clear();
	mem.getIntArray(orig);
}

//-----------------------------------------------------------------
void		BPT_Helpers::memoryPrune(MIntArray& orig, MIntArray& doppelte)
//-----------------------------------------------------------------
{
	int l = orig.length();
	int	biggest = 0;
	int smallest = 166666666;

//	wenns zu speicherintesiv wird, einfach noch den kleinsten Wert ermitteln
//	und die ArrayGroesse auf [biggest - smallest] stellen.
//	beim zuweisen der Werte = orig[i] - smallest
//	beim wiedereintragen on orig.append(x + smallest)

//	groesstes und kleinstes element holen
	int i;
	for(i = 0; i < l; i++)
	{
		if(orig[i] > biggest)
			biggest = orig[i];
		
		if(orig[i] < smallest)
			smallest = orig[i];
	}


/*
	i = 0;
//	kleinstes Element holen
	for(; i < l; i++)
		if(orig[i] < smallest)
			smallest = orig[i];
*/

//	MemArray eventuell zu normalem Feld machen

	i = 0;
	MIntArray* memArray = new MIntArray( ( (biggest + 1) - smallest +1) ,-1);

	
//	indices eintragen
	for(;i < l; i++ )
	{
		int myTmp = (*memArray)[orig[i] - smallest];

		if(myTmp == -1)
			(*memArray)[orig[i] - smallest] = orig[i];
		else
			doppelte.append(myTmp);
	}

	i = 0;
	l = memArray->length();

	////cout<<l <<" == Länge memArray"<<endl;
	orig.clear();
//	nullen rausschmeissen
	for(;i < l; i++)
		if( (*memArray)[i] != -1)
			orig.append( (*memArray)[i]);
		
	delete memArray;
}


//-----------------------------------------------------------------
void		BPT_Helpers::invertArray(MIntArray& array)
//-----------------------------------------------------------------
{
	int l2 = ( array.length() - (array.length() % 2) ) / 2;
	int lminus = array.length() - 1;
	int tmp;

	for(int i = 0; i < l2; i++)
	{
		tmp = array[i];
		array[i] = array[lminus - i];
		array[lminus - i] = tmp;
	}

}

//-----------------------------------------------------------------
void		BPT_Helpers::invertArray(MIntArray& array, int lBound, int rBound)
//-----------------------------------------------------------------
{
	//dasselbe wie oben, nur mit variablen zur arbeit au groesseren arrays - bzw. der Umkehr von array teilen
	int l2 = ((rBound - lBound) / 2 ) + lBound + ((rBound - lBound) % 2);
	int tmp;
	UINT r = 0;

	for(int i = lBound; i < l2; i++)
	{
		tmp = array[i];
		array[i] = array[rBound - r];
		array[rBound - r] = tmp;
		++r;
	}

}


//---------------------------------------------------------------------
void		BPT_Helpers::memoryInvertSelection(MIntArray& lhs, int maxElement, MIntArray& inverted)
//---------------------------------------------------------------------
{
	MIntArray	*memArray = new MIntArray(maxElement, -1);
	
	//memArray aufmachen und mit Daten füllen
	int tmp;
	int l = lhs.length(), i;
	for(i = 0; i < l;i++)
	{
		tmp = lhs[i];
		(*memArray)[tmp] = tmp;
	}


//	MemArray durchgehen und jedes -1 Element als Selection nehmen
	l = memArray->length();
	i = 0;
	for(;i < l; i++)
	{
		tmp = (*memArray)[i];
		if(tmp == -1)
			inverted.append(i);
	}

}



//-----------------------------------------------------------------
MIntArray		BPT_Helpers::matchArray(MIntArray lhs,MIntArray rhs) const
//-----------------------------------------------------------------
{
	MIntArray match;

	int leftL = lhs.length();
	int rightL = rhs.length();
	
	//EFFIZIENZMODUS -> eventuell ist es für algos nicht okay, das automatisch gepruned und die reihenfolge verändert wird
	if(leftL + rightL < 200)
	{

		int i = 0;
		int x = 0;

		int ROffset;
		ROffset = 0;

		for(;i < leftL; i++)
		{
			x = 0;
			for(;x < rightL - ROffset; x++)
			{
				if(lhs[i] == rhs[x])
				{
					match.append(lhs[i]);
					rhs.remove(x--);
					ROffset++;
					break;
				}
			}
		}

		return match;
	}
	else
	{

		BPT_BA memArray(lhs,true);	//auf diese Weise muss nur noch die obere Grenze herausgefunden werden
		
		
		MIntArray match;
		
		if(rhs.length() < 500)
		{//in diesem Fall einfach bitweise vergleichen
			memArray.findMatching(rhs,match);
		}
		else
		{//ansonsten ist es effizienter, ein eigenes Array für rhs aufbauen
			BPT_BA rhsBA(rhs,true);
			(memArray & rhsBA).getIntArray(match);
		}

		return match;

	}
	
}



//-----------------------------------------------------------------
MIntArray		BPT_Helpers::memoryMatch(MIntArray lhs, MIntArray rhs)
//-----------------------------------------------------------------
{
	/*	
	MIntArray	match;

	int l = lhs.length();
	int	biggest = 0;
	int smallest = 166666666;


	for(int i = 0; i < l; i++)
	{
		int tmp = lhs[i];
		if(tmp > biggest)
			biggest = tmp;

		if(tmp < smallest)
			smallest = tmp;
	}

	if(smallest < 0)
		smallest = 0;

	MIntArray* memArray = new MIntArray( (biggest + 1) - smallest, -1);

	//	MEMORYCHECK

	//	if(memArray.length() < (biggest + 1) - smallest + 10)
	//		MGlobal::displayError("OutOfMemory");



	//	alle aus lhs in MemArray eintragen	
	i = 0;
	for(;i < l; i++ )
		(*memArray)[lhs[i] - smallest] = lhs[i];

	////cout <<memArray.length()<<" = Länge von MemArrayEdges"<<endl;

	i = 0;
	l = rhs.length();

	//	eventuell versuchr rhs wegen hoeherer indices als die von
	//	lhs, auf nicht vorhandene memArrayElemente zuzugreifen.
	//	MIntArray dürfte dies abfangen

	int ml = 0;
	int elm = 0;
	//	PERFORMANCE: für match eventuell nen element counter 
	//	mitführen, damit nicht so oft length() abgefragt wird
	for(;i < l; i++)
	{
		elm = rhs[i] - smallest;
		if( elm >= 0 && elm <= biggest - smallest  )
		{
			if( (rhs[i] == (*memArray)[ elm ]) )
				match.append(rhs[i]);
		}
	}
	////cout<< match.length() <<" = Länge von MatchArray"<<endl;
	delete memArray;

	*/

	 //noch testen
	BPT_BA memArray(lhs,true);	//auf diese Weise muss nur noch die obere Grenze herausgefunden werden


	MIntArray match;

	if(rhs.length() < 500)
	{//in diesem Fall einfach bitweise vergleichen
		memArray.findMatching(rhs,match);
	}
	else
	{//ansonsten ist es effizienter, ein eigenes Array für rhs aufbauen
		BPT_BA rhsBA(rhs,true);
		(memArray & rhsBA).getIntArray(match);
	}

	return match;
}



//----------------------------------------------------------------------------------------------
bool		BPT_Helpers::UVSetDiffers(const MIntArray& UVs1, const MIntArray& UVs2) const
//----------------------------------------------------------------------------------------------
{
	UINT l = UVs1.length();
	for(UINT i = 0; i < l; i++)
	{
		if(UVs1[i] != UVs2[i])
			return true;
	}
		
	return false;

}


//-------------------------
//DEBUG METHODEN
//-------------------------
//#ifdef DEBUG

//-----------------------------------------------------------------
void		BPT_Helpers::printArray(const MIntArray& array, MString message)
//-----------------------------------------------------------------
{
	for(unsigned i = 0; i < array.length(); i++)
		cout <<array[i]<<message.asChar()<<endl;

	cout <<"//printArray Ende"<<endl;
	cout <<endl;
	cout <<endl;
	cout <<endl;
	
}

//#endif
