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

#include "BPT_BA.h"



INVIS(ULONG		BPT_BA::counter = 0;);


//KONSTANTEN DEFINITIONEN
const int 	BPT_BA::nBitsInUlong		= sizeof( ULONG ) * 8;
const ULONG BPT_BA::null		= (ULONG)0;
const ULONG BPT_BA::one			= (ULONG)1;
const ULONG BPT_BA::fulllong	= ~(ULONG)0;



//--------------------------------------------------------------------
BPT_BA::BPT_BA(): array(NULL),numChunks(null),highBound(null),lowBound(null),offset(null),numTrue(null)
//--------------------------------------------------------------------
{
	INVIS(number = counter++;)
	INVIS(cout<<"ERZEUGE BA NUMMER: "<<number<<endl;)
}



//--------------------------------------------------------------------
BPT_BA::BPT_BA( ULONG initialSize, bool initialValue ):array(NULL), lowBound(null), offset(null),highBound(initialSize)
//--------------------------------------------------------------------
{

	ULONG rest = highBound % nBitsInUlong;

	ULONG allocSize = (highBound - rest) / nBitsInUlong;

	numTrue = initialSize * (ULONG)initialValue;

	allocSize += (rest > null) ? one : null;
	numChunks = allocSize;

	INVIS(cout<<"Größe der SpeicherAllocation = "<<allocSize<<" == "<<allocSize * nBitsInUlong<<" Bit"<<endl;);

	array = (ULONG*) malloc(allocSize*sizeof(ULONG));


	//jetzt durchs LongArray parsen und werte setzen
	ULONG value = (initialValue)? fulllong : null ;

	for(ULONG i = null; i < allocSize; i++)
	{
		 array[i] = value;
	}

	INVIS(number = counter++;)
	INVIS(cout<<"ERZEUGE BA NUMMER: "<<number<<endl;)
}


//--------------------------------------------------------------------
void		BPT_BA::setAllFalse()
//--------------------------------------------------------------------
{
	//durch Array loopen und alles 0 setzen
	
	for(UINT i = 0; i < numChunks; i++)
		array[i] = null;

}


//--------------------------------------------------------------------
BPT_BA::BPT_BA(const MIntArray& initialArray, bool beginIsIndex0, bool initialValue, ULONG maxLength):array(0)
//--------------------------------------------------------------------
{

	ULONG l = initialArray.length();
	ULONG i;

	if(l == null)
	{//sicherheitsCheck)
		array = 0;
		numChunks = highBound = offset = lowBound = null ;
		return;
	}

	

	if(maxLength == null)
	{
		//größten index herausfinden
		long max = 0;
		
		for(i = null; i < l; i++)
		{
			if(initialArray[i] > max)
				max = initialArray[i];
		}
		
		maxLength = max;
	}

	//smallest index herausfinden
	offset = lowBound = null;
	if(!beginIsIndex0)
	{//verhindert, dass kleine Arrays mit großen indizes auch große BitArrays zu folge haben

		long min = 999999999;
	
		for(i = null; i < l; i++)
		{
			if(initialArray[i] < min)
				min = initialArray[i];
		}

		if(min > -1)
		{
			offset = (min / nBitsInUlong) * nBitsInUlong;		//verzicht auf Modulus OP
			lowBound = min;
		}
		else
			offset = null;

	}// END if start index is NOT 0


	//jetzt neues BitArray erzeugen mit der sich ergebenen size. (+1 zu maxLength, weil es 0 Based sein muss wie offset, damit gültige size herauskommt)
//	size = (maxLength + 1) - offset;
	
	highBound = maxLength + one;	//entweder man speichert den Reellen wert, oder man müsste sich die Size jedesmal errechnen, was unsinn ist
								//eventuel fügt man ne: RealSize variabl ein

	INVIS(cout<<highBound<<" == GRÖßE DES NEUEN BITARRAYS"<<endl;);
	

	ULONG rest = highBound % nBitsInUlong;

	ULONG allocSize = (highBound - offset - rest ) / nBitsInUlong;
	

	allocSize += (rest > null) ? one : null;
	numChunks = allocSize;


	INVIS(cout<<"Größe der SpeicherAllocation = "<<allocSize<<" == "<<allocSize * nBitsInUlong<<" Bit"<<endl;);



	array = (ULONG*) malloc(allocSize * sizeof(ULONG));



	//jetzt durchs longArray parsen und werte setzen
	ULONG value;
	if(initialValue)
	{
		value = fulllong;
		
		//true ist invertiert
	//	numTrue = (highBound - lowBound) - l;
	}
	else
	{
		value = null;
	}


	for( i = null; i < allocSize; i++)
	{
		array[i] = value;
	}

	//und jetzt noch die Flags umkehren an der stelle, wo indices aus MIntArray vorhanden sind
	long index;	//damit der index negativ sein kann
	ULONG aIndex;
	ULONG wert;

	ULONG shift;
	ULONG operand;

	if( ! initialValue )
	{
		numTrue = 0;

		for(i = null; i < l; i++)
		{

			index =  initialArray[i];
			index -= offset;

			if(index < 0)	//dieser Check muss sein, damit ich auch Flags mit -1 vernünftig verarbeiten kann
				continue;

			aIndex = (ULONG) (index / nBitsInUlong);
			shift = index - aIndex*nBitsInUlong;
			wert = one << shift;
			operand = array[aIndex]&wert^wert ;
			

			//unbedingt vorher checken, ob bit schon gesetzt, damit mehrfach vorhandene nicht das gesamt bitArray versauen
			//array[aIndex] += ( (array[aIndex] & wert) == null ) 
			//					? wert
			//					: null;

			array[aIndex] += operand;
			numTrue += operand >> shift;

		}
	}
	else
	{
		numTrue = highBound - lowBound ;

		for(i = null; i < l; i++)
		{

			index =  initialArray[i];
			index -= offset;

			
			if(index < 0)	//dieser Check muss sein, damit ich auch Flags mit -1 vernünftig verarbeiten kann
				continue;

			aIndex = (ULONG) (index / nBitsInUlong);
			shift = index - aIndex*nBitsInUlong;
			wert = one << shift;
			operand = array[aIndex]^wert&wert;


		//	array[aIndex] -= ( (array[aIndex] & wert) == null ) 
		//					? null
		//					: wert;

			array[aIndex] -= operand;
			numTrue -= operand >> shift;
		}

	}



	//FERTIG
	INVIS(number = counter++;)
	INVIS(cout<<"ERZEUGE BA NUMMER: "<<number<<endl;)
}


//--------------------------------------------------------------------
BPT_BA::BPT_BA(const BPT_BA& rhs)
//--------------------------------------------------------------------
{

	INVIS(cout<<"BA KOPIERKONSTRUKTOR , KOPIERE NUMMER"<<rhs.number<<endl;);

	

	//Tiefe Kopie erzeugen - also neuen speicher holen entsprechend der Size, und dann alle werte übertragen
	highBound = rhs.getHighBound();
	offset = rhs.getOffset();
	lowBound = rhs.getLowBound();
	numTrue = rhs.getTrueCount();

	ULONG allocSize = rhs.getNumChunks();
	numChunks = allocSize;
	

	//speicher
	if(numChunks != null)
	{
		array = (ULONG*) malloc(allocSize * sizeof(ULONG));	//x longs mit je 4 byte

		//werte kopieren
		const ULONG* rhsArray = rhs.getPointer();

		for(ULONG i = null; i < allocSize; i++)
			array[i] = rhsArray[i];
	}
	else
		array = 0;

	INVIS(number = counter++;)
	INVIS(cout<<"ERZEUGE BA NUMMER: "<<number<<endl;)
}

//--------------------------------------------------------------------
BPT_BA&			BPT_BA::operator = (const BPT_BA& rhs)
//--------------------------------------------------------------------
{
	INVIS(cout<<"GEBE NEW BA& ZURÜCK, rhs hat nummer: "<<rhs.number<<endl;);
	INVIS(cout<<"Bin in Nummer: "<<number<<endl;)
	
	highBound = rhs.getHighBound();
	offset = rhs.getOffset();
	lowBound = rhs.getLowBound();
	numTrue = rhs.getTrueCount();

	ULONG allocSize = rhs.getNumChunks();
	numChunks = allocSize;


	//speicher
	array = (ULONG*) malloc(allocSize * sizeof(ULONG));	//x longs mit je 4 byte

	//werte kopieren
	const ULONG* rhsArray = rhs.getPointer();

	for(ULONG i = null; i < allocSize; i++)
		array[i] = rhsArray[i];


	return *this;
	
}

//--------------------------------------------------------------------
BPT_BA::~BPT_BA(void)
//--------------------------------------------------------------------
{
	if(array != null)
		free((void*)array);

	INVIS(cout<<"ZERSTÖRE INSTANZ NUMMER: "<<number<<endl;);
	INVIS(counter--;);
}

//--------------------------------------------------------------------
bool	BPT_BA::operator [] (long index) const
//--------------------------------------------------------------------
{	
		//UNSICHERE METHODE - IS FLAG SET BENUTZEN FÜR KOMPLETTEN RANGECHECK
		// (wenn man also nicht weis, ob index im Array ist)
		//allerdings gehts kaum schneller ...
	
		
		index -= offset;


		//wert zurückgeben
		ULONG wert = one << index % nBitsInUlong;
		
		return ( ( (array[ (ULONG) (index / nBitsInUlong)] ) & wert ) );
		

}


//--------------------------------------------------------------------
void		BPT_BA::getIntArray(MIntArray& inResult) const
//--------------------------------------------------------------------
{
	inResult.clear();	//um sicherzustellen, dass array wirklich leer ist und der aufrufenden Procedur die Arbeit zu ersparen

	ULONG origIncrement = inResult.sizeIncrement();
	inResult.setSizeIncrement((ULONG)highBound / sizeof(ULONG));	//ist am effizientesten, wenn 1/4 der vorhanden indices im BitArray auch wirklich besetzt sind

	//gibt IntArray mit indizes zurück, die true sind.



	ULONG wert ;
	ULONG l;

	ULONG i = lowBound;	

	for(ULONG x = null; x < numChunks; x++)
	{
		if( (array[x] & fulllong) == null)
		{
			//bis zum nächsten Chunk springen, allerdings abhängig von x, falls lowBound != 0
			//i += nBitsInUlong;
			i = ULONG(i / nBitsInUlong) * nBitsInUlong + nBitsInUlong;
			continue;
		}
		else
		{
			for(l = ((x + one) * nBitsInUlong + offset); i < l ; i++)	//auf den entsprechenden Highbound wird hier nicht gecheckt, da ich davon ausgehe, dass alle anderen
			{									//prozeduren die bounds berücksichtigen
				
				wert = one << i % nBitsInUlong;

				if( array[x] & wert ) 
					inResult.append(i + offset);
			}

		}
	}

	inResult.setSizeIncrement(origIncrement);
}

//--------------------------------------------------------------------
bool		BPT_BA::isFlagSet ( long index) const
//--------------------------------------------------------------------
{
		
		//index MUSS singned sein, damit index -= offset auch negativ werden kann und die ifKlausel anschlagen kann
		
		//diese Funktion ist etwa s wie operator[], nur dass sie noch nen RangeCheck macht
		if(index > highBound || index < lowBound)		//rangeCheking eventuell rausnehmen, wenn es unnötig ist
			return false;


		index -= offset;

		//hier muss auf jeden Fall überprft werden, ob index kleiner 0
		//was durchaus vorkommen kann, wenn offset gesetzt ist


		//wert zurückgeben	--> der Wert einhaltet ne long mit nur einem gesetzten Bit

		ULONG wert = one << index % nBitsInUlong;
		
		//return ( ((array[ (ULONG)(index / nBitsInUlong )]) & wert) == wert)
		//		? true
		//		: false;
		
		return (bool)(array[ (ULONG)(index / nBitsInUlong ) ] & wert);
}

//--------------------------------------------------------------------
void		BPT_BA::setBitTrue (ULONG index)
//--------------------------------------------------------------------
{
	//da man aufgrund der speicherart (long) kein Bool mit dem []Operator zurückgeben kann, 
	//muss man das individuelle setzen von Bits mit dieser Procedur machen.



	//erstmal wir üblich den chunk und den bitrest heruasfinden für zuriff

	index -= offset;

	//CHECK(index,size);

	ULONG chunk = index / nBitsInUlong;
	ULONG shift = index - chunk*nBitsInUlong;
	//ULONG wert = one << index % nBitsInUlong;
	ULONG wert = one << shift;

	ULONG operand = ( array[chunk] & wert ^ wert);


	//kein check mehr, um geschwindigkeit zu gewinnen
	//wenn man diese Procedur aufruft, muss man sicher sein, dass der Flag auch wirklich false ist


	//dochCheck
//	( (array[(ULONG)(index / nBitsInUlong)] & wert) == null ) ? array[(ULONG)(index / nBitsInUlong)] += wert, ++numTrue: null;
	
	array[chunk] += operand;
	numTrue += operand >> shift;

}



//--------------------------------------------------------------------
void	BPT_BA::setBitFalse (ULONG index)
//--------------------------------------------------------------------
{
	
	//erstmal wir üblich den chunk und den bitrest heruasfinden für zuriff

	index -= offset;

	//CHECK(index,size);

	ULONG chunk = index / nBitsInUlong;
	ULONG shift = index - chunk*nBitsInUlong;
	//ULONG wert = one << index % nBitsInUlong;
	ULONG wert = one << shift;

	ULONG operand = ( array[chunk] & wert );

	//kein check mehr, um geschwindigkeit zu gewinnen
	//wenn man diese Procedur aufruft, muss man sicher sein, dass der Flag auch wirklich false ist
	
	//doch Check
	array[chunk] -= operand;
	numTrue -= operand >> shift;

	//( (array[(ULONG)(index / nBitsInUlong)] & wert) == wert ) ? array[(ULONG)(index / nBitsInUlong)] -= wert, --numTrue: null;

}

//--------------------------------------------------------------------
void	BPT_BA::setBits (const MIntArray& indices, bool value)
//--------------------------------------------------------------------
{
	ULONG l = indices.length();
	ULONG index;	//im inices Array könnten auch negative Werte sein, allerdings wird diese Klasse eh nur 
							//mit positiven KomponentenArrays verwendet
	ULONG wert;
	ULONG aIndex;

	ULONG shift;
	ULONG operand;


	if(value)
	{
		for(ULONG i = null; i < l; i++)
		{
			index = indices[i];


		//	if(index > highBound || index < lowBound)		//rangeCheking eventuell rausnehmen, wenn es unnötig ist
		//		continue;


			

			index -= offset;

			aIndex = (ULONG)(index / nBitsInUlong);
			shift = index - aIndex*nBitsInUlong;
			//CHECK(index,size);

			//wert = one << index % nBitsInUlong;
			wert = one << shift;
			operand = array[aIndex]&wert^wert;

			//kein check mehr, um geschwindigkeit zu gewinnen
			//wenn man diese Procedur aufruft, muss man sicher sein, dass der Flag auch wirklich false is
			//doppelte einträge im indices array würden dann nen ganzen chunk versauen, also dann lieber remove/add nehmen

			

		//	( (array[aIndex] & wert) == null ) 
			//		? array[aIndex] += wert, ++numTrue
			//		: null;

			array[aIndex] += operand;
			numTrue += operand >> shift;
		}
	}
	else
	{
		for(ULONG i = null; i < l; i++)
		{
			index = indices[i];


		//	if(index > highBound || index < lowBound)		//rangeCheking eventuell rausnehmen, wenn es unnötig ist
		//		continue;

			index -= offset;

			aIndex = (ULONG)(index / nBitsInUlong);
			shift = index - aIndex*nBitsInUlong;
			//CHECK(index,size);

			//wert = one << index % nBitsInUlong;
			wert = one << shift;
			operand = array[aIndex]&wert;
			
			//( (array[aIndex] & wert) == wert ) 
				//	? array[aIndex] -= wert, --numTrue
				//	: null;

			array[aIndex] -= operand;
			numTrue -= operand >> shift;
		}
	}


}

//--------------------------------------------------------------------
void	BPT_BA::invertBits (MIntArray& indices)
//--------------------------------------------------------------------
{
	ULONG index;			//im inices Array könnten auch negative Werte sein, allerdings wird diese Klasse eh nur 
							//mit positiven KomponentenArrays verwendet

	ULONG l = indices.length();
	ULONG wert;
	ULONG aIndex;
	ULONG operand;
	ULONG shift;

	for(ULONG i = null; i < l; i++)
	{

		index = indices[i];

		index -= offset;

		//WARNUNG: UNGETESTET!!!	
		//sollte aber klappen *g*
		
		aIndex = (ULONG)(index / nBitsInUlong);
		shift = index - aIndex*nBitsInUlong;
		wert = one << shift;
		operand = (array[aIndex] & wert) >> shift;	
		
		array[aIndex] += (1 - operand) * wert - operand*wert;
		numTrue += (1 - operand) - operand;


/*
		if( (array[aIndex] & wert) )
		{
			array[aIndex] -= wert;
			--numTrue;
		}
		else
		{
			array[aIndex] += wert;
			++numTrue;
		}
*/
	}

}

//--------------------------------------------------------------------
void BPT_BA::useThisData(	ULONG* allocatedMemoryPtr, 
							ULONG inHighBound,
							ULONG inLowBound,
							ULONG inOffset,
							ULONG inNumChunks)
//--------------------------------------------------------------------
{
	if( array != 0 )
		free( array );
	
	array = allocatedMemoryPtr;
	highBound = inHighBound;
	lowBound = inLowBound;
	offset = inOffset;
	numChunks = inNumChunks;

	//size wird hier nicht ermittelt, da der rechenaufwand hier ungerechtfertigt ist - 
	//wenn diese prozedur aufgerufen wird, wurde ein BA mit anerem BA verrechnet, wo eigentlich keine Size benötigt wird

	INVIS(cout<<"Übertrage Daten auf Nummer: "<<number<<endl;);

}

//----------------------------------------------------------------------------------------------------
bool	BPT_BA::expand	(ULONG  expandBy, bool valueOfNewBits )
//----------------------------------------------------------------------------------------------------
{
	if(expandBy == null)
		return false;

	ULONG newSize = highBound + expandBy;

	//wenn alte größe für die expansion ausreicht, dann ...
	if( newSize < numChunks * nBitsInUlong )
	{//... einfach highBound umsetzen und fertig
	
		if(valueOfNewBits)
		{//neue bits sollen true gesetzt werden ...
			
			ULONG wert;
			ULONG aIndex;
			ULONG operand;
			ULONG shift;

			for(ULONG i = highBound; i < newSize; i++)
			{
				aIndex = (ULONG)(i / nBitsInUlong);
				shift = i - aIndex * nBitsInUlong;
				wert = one << shift;
				operand = array[aIndex] & wert ^ wert;
				

				array[aIndex] += operand;
				numTrue += operand >> shift;
				//eigentlich könnte man hier auch gleich +wert machen, aber ich checke das sicherheitshalber nochmal ...
				
				//( (array[aIndex] & wert) == null ) ? array[aIndex] += wert, ++numTrue : null;
			}

		}
		
		highBound = newSize;


		return true;
	}

	//... ansonsten neuen Speicher allokalisieren
	ULONG allocSize = (ULONG)(newSize) / nBitsInUlong ;
	allocSize		+= (newSize % nBitsInUlong) ? one : null;

	ULONG* newArray = (ULONG*)malloc( allocSize * sizeof(ULONG));

	//neuen Speicher initialisieren mit valueOfNewBits
	ULONG value = valueOfNewBits ? fulllong : null;
	numChunks += (numChunks == null) ? one : null;

	ULONG i;
	for(i = numChunks - 1; i < allocSize; i++)
		newArray[i] = value;


	//alten Speicher in neuen Speicher kopieren
	
	for(i = null; i < numChunks; i++)
		newArray[i] = array[i];


	//wenn das alte Array nicht bis zum anschlag gefüllt war, dann sind jetzt wieder einige Bytes überschrieben worden mit den werten aus
	//dem OriginalArray - dies ändern
	ULONG wert;
	ULONG operand;
	ULONG shift;
	
	ULONG aIndex = (ULONG)(highBound / nBitsInUlong);
	ULONG chunkBound = (aIndex + one) * nBitsInUlong;
	
	if(value)
	{//Bits true setzen (sicher)
		// numTrue anpassen (nur für zusätzliche Chunks, bei denen dann auf jeden Fall alles true ist)
		numTrue += (allocSize - numChunks) * nBitsInUlong;


		for(i = highBound; i < chunkBound; i++)
		{
			shift = i - aIndex * nBitsInUlong;
			wert = one << shift;
			operand = newArray[aIndex] & wert ^ wert;

			newArray[aIndex] += operand;
			numTrue += operand >> shift;
/*
			( (newArray[aIndex] & wert) == null ) 
				? newArray[aIndex] += wert, ++numTrue		//und die einzeln gesetzten bits auch mitzählen (sie sind in der chunkweisen numTrue rechnung natürlich nicht enthalten
				: null;
*/

		}
	}
	else
	{//Bits false setzen (sicher)
	 //das ist hier ja eigentlich nicht nötig, wenn wirklich immer alles stimmt mit den "überzähligen" bits
		for(i = highBound; i < chunkBound; i++)
		{
			shift = i - aIndex * nBitsInUlong;
			wert = one << shift;
			operand = newArray[aIndex] & wert;
			
			newArray[aIndex] -= operand;
			
			/*
			newArray[aIndex] -= ( (newArray[aIndex] & wert) == wert ) 
				? wert 
				: null;
			*/

		}
	}


	//Okay, jetzt noch highBound ändern, alten speicher freigeben, ptr umsetzen und fertig
	highBound = newSize;

	if(array != null)
		free((void*)array);

	array = newArray;

	return true;
}



//----------------------------------------------------------------------------------------------------
bool	BPT_BA::contract (ULONG  contractBy)
//----------------------------------------------------------------------------------------------------
{
	if(contractBy == null)
		return false;

	long newSize = highBound - contractBy;
	if(newSize < null)
	{
		return setLength(null);
	}

	//hier lohnt sich die arbeit mit operand usw. nicht unbedingt.
	ULONG wert;
	//wenn alte größe für die kontraktion ausreicht, dann ...
	if( newSize > (numChunks - 1) * nBitsInUlong )
	{//... einfach highBound umsetzen und fertig

		//aber vorher noch die numTrue aktualisieren
		for(ULONG i = newSize; i < highBound; i++)
		{
			wert = one << i % nBitsInUlong;
			
			//wenn hier true werte waren, dann numTrue verringern 
			numTrue -= ( (array[(ULONG)(i / nBitsInUlong)] & wert) ) ? one : null;
		}

		highBound = newSize; 
		return true;
	}

	//... ansonsten neuen Speicher allokalisieren
	ULONG allocSize = (ULONG)(newSize) / nBitsInUlong ;

	allocSize		+= (newSize % nBitsInUlong) ? one : null;

	ULONG* newArray = (ULONG*)malloc( allocSize * sizeof(ULONG));

	//alten Speicher übertragen
	ULONG i;
	for(i = null; i < allocSize; i++)
		newArray[i] = array[i];
	
	//noch die numTrue aktualisieren
	for(i = newSize; i < highBound; i++)
	{
		wert = one << i % nBitsInUlong;

		//wenn hier true werte waren, dann numTrue verringern 
		numTrue -= ( (array[(ULONG)(i / nBitsInUlong)] & wert) ) ? one : null;
	}

	//Okay, jetzt noch highBound ändern, alten speicher freigeben, ptr umsetzen und fertig
	highBound = newSize;

	//hier kein check, da bereits früher überprüft wird, ob es möglich ist
	free((void*)array);

	array = newArray;

	return true;

}
//----------------------------
ULONG	BPT_BA::syncNumTrue()
//----------------------------
{
	// TODO: Highbound is not taken into consideration - fix this !	
	//reset numTrue
	numTrue = null;

	//durch alle bits parsen und zählen
	ULONG i = lowBound;
	ULONG l;
	ULONG wert;
	for(ULONG x = null; x < numChunks; x++)
	{
		if( (array[x] & fulllong) == null)
		{
			//bis zum nächsten Chunk springen, allerdings abhängig von x, falls lowBound != 0
			i = ULONG(i / nBitsInUlong) * nBitsInUlong + nBitsInUlong;
			continue;
		}
		else
		{
			for(l = ((x + one) * nBitsInUlong + offset); i < l ; i++)	//auf den entsprechenden Highbound wird hier nicht gecheckt, da ich davon ausgehe, dass alle anderen
			{									//prozeduren die bounds berücksichtigen
				
				wert = one << i % nBitsInUlong;

				if( array[x] & wert) 
					++numTrue;
			}

		}
	}

	return numTrue;

}

//----------------------------------------------------------------------------------------------------
ULONG	BPT_BA::getFirstBitIndex() const
//----------------------------------------------------------------------------------------------------
{
	//erstmal sichergehen, dass er nicht unnützerweise sucht
	if(!numTrue)
		return null;

	 //eventuell könnte man sich den check sparen, wenn der check in der aufrufenden prozedur gemacht wirdb aber naja ;)

	//jetzt runup machen und das erste gefundene bit samt index zurückgeben
	ULONG i = lowBound;
	ULONG l;
	ULONG wert;
	for(ULONG x = null; x < numChunks; x++)
	{
		if( (array[x] & fulllong) == null)
		{
			//bis zum nächsten Chunk springen, allerdings abhängig von x, falls lowBound != 0
			//i += nBitsInUlong;
			//i = (x + one) * nBitsInUlong;
			i = ULONG(i / nBitsInUlong) * nBitsInUlong + nBitsInUlong;
			continue;
		}
		else
		{
			for(l = ((x + one) * nBitsInUlong + offset); i < l ; i++)	//auf den entsprechenden Highbound wird hier nicht gecheckt, da ich davon ausgehe, dass alle anderen
			{									//prozeduren die bounds berücksichtigen
				
				wert = one << i % nBitsInUlong;

				if( array[x] & wert) 
					return i * ( i < highBound);
			}

		}
	}

	//forbidden
	MPRINT("GET FIRST BIT INDEX: WAR IM FORBIDDEN PATH");
	return null;
}

//----------------------------------------------------------------------------------------------------
ULONG	BPT_BA::getLastBitIndex() const
//----------------------------------------------------------------------------------------------------
{
	//erstmal sichergehen, dass er nicht unnützerweise sucht
	if(!numTrue)
		return null;

	 //eventuell könnte man sich den check sparen, wenn der check in der aufrufenden prozedur gemacht wirdb aber naja ;)

	//jetzt runup machen und das erste gefundene bit samt index zurückgeben
	ULONG i = highBound;
	ULONG l;
	ULONG wert;
	for(ULONG x = numChunks; x > null; x--)
	{
		if( (array[x] & fulllong) == null)
		{
			//bis zum nächsten Chunk springen, allerdings abhängig von x, falls lowBound != 0
			//i += nBitsInUlong;
			i = ULONG(i / nBitsInUlong) * nBitsInUlong + nBitsInUlong;

			continue;
		}
		else
		{
			for(l = ((x - one) * nBitsInUlong + offset); i > l ; i--)	
			{									
				
				wert = one << i % nBitsInUlong;

				if( array[x] & wert ) 
					return i * ( i >= lowBound);	//damit die Boungrenze beachtet wird (kann man eventuell rausnehmen, 
																	//da alle anderen proceduren diese grenzen *eingentlich* beachten)
			}

		}
	}

	//forbidden
	MPRINT("GET FIRST BIT INDEX: WAR IM FORBIDDEN PATH");
	return null;
}

//----------------------------------------------------------------------------------------------------
bool	BPT_BA::setLength(ULONG  newLength, bool valueOfNewBits )
//----------------------------------------------------------------------------------------------------
{
	//wenn es eine radikale Säuberung ist, dann alles 0 setzen und speicher freimachen
	if(newLength == null)
	{
		if(highBound == null)
			return false;

		if(array != null)	//sicherheitshalber
			free((void*) array);
		
		array = 0;
		
		highBound = lowBound = offset = numTrue = numChunks = null;
		
		return true;
	}

	//bestimmen, ob es sich um kontraktion oder expansion handelt, und dann die entsprechende methode aufrufen
	if(newLength < highBound)
		return contract(highBound - newLength);
	else
		return expand(newLength - highBound,valueOfNewBits);

}

//----------------------------------------------------------------------------------------------------
bool	BPT_BA::add (  MIntArray& operand, ULONG max, ULONG minSize )
//----------------------------------------------------------------------------------------------------
{
	//mit diese Methode kann man ein BA mit nem beliebig großen (was indices angeht) intArray zusammenführen, inklusive dynamischer 
	//expansion

	//->numTrue ändert sich bei der ganzen Sache nicht

	//wenn max 0 ist, dann maxIndex des operanden herausfinden
	if(max == null)
	{

		ULONG i,l = operand.length();

		for(i = null;i < l; i++)
		{	
			if(operand[i] > max)
				max = operand[i];
		}
	}

	if(minSize == null)
	{
		long locMinSize = 999999999;
		ULONG i,l = operand.length();

		for(i = null;i < l; i++)
		{	
			if(operand[i] < locMinSize)
				locMinSize = operand[i];
		}

		//umgang mit negativen indices (sicherheitscheck)
		if(locMinSize < 0)
			minSize = null;
		else
			minSize = locMinSize;
	}



	//wenn die zu mergende Spanne zu groß, dann array vergrößern
	if( minSize < lowBound || max > highBound )
	{//array expansion: Da expand und contract nur die HighBound ändern können, muss hier ne eigene prozedur her
		
		//für das nach dem highbound noch genügend bits zur Verfügung stehen 

		if( minSize > lowBound && max < numChunks * nBitsInUlong + offset)
		{//einfach highBound anpassen und mit Add weitermachen
			highBound = max;
			goto outOfIf;
		}

		//allocSize ist die maximale spanne
		highBound = highBound > max ? highBound : max;
		lowBound = (minSize < lowBound) ? minSize : lowBound;

		ULONG allocSize = (ULONG)( highBound - lowBound) / nBitsInUlong;
		
		allocSize += (( highBound - lowBound) % nBitsInUlong) ? one : null;

		ULONG* newArray = (ULONG*)malloc(allocSize * sizeof(ULONG));

		//alle Chunks im neuen Array 0 setzen
		ULONG i;
		for(i = null; i < allocSize; i++)
			newArray[i] = null;


		//jetzt die Daten übertragen
		ULONG	myOffset = (offset  - lowBound) / nBitsInUlong;

		//ULONG	myOffset = (minSize < lowBound) ? (ULONG)(lowBound - minSize) / nBitsInUlong : null ;
		
		for(i = null; i < numChunks; i++)
			newArray[i + myOffset] = array[i];


		//okay, jetzt noch ptr umsetzen, Bounds ändern und dann weiter
		if(array != null)
			free((void*)array);

		
		
		offset = (ULONG)(lowBound / nBitsInUlong) * nBitsInUlong;

		numChunks = allocSize;

		array = newArray;
		
		
	}

outOfIf:

	//wenn er hier ist, hat das array auf jeden Fall die passende größe, so dass die operation ausgeführt werden kann
	return performLogicalOperation(operand,2,operand);

}



//----------------------------------------------------------------------------------------------------
bool	BPT_BA::findMatching(	MIntArray& operand, MIntArray& emptyResult	) 
//----------------------------------------------------------------------------------------------------
{
	emptyResult.clear();	//sicherheitshalber das Array leer machen
	return	performLogicalOperation(operand,null,emptyResult);

}

//----------------------------------------------------------------------------------------------------
bool	BPT_BA::remove( MIntArray& operand 	)
//----------------------------------------------------------------------------------------------------
{
	MIntArray dummy;
	return	performLogicalOperation(operand,1,dummy);

}

//----------------------------------------------------------------------------------------------------
bool		BPT_BA::add ( MIntArray& operand )
//----------------------------------------------------------------------------------------------------
{
	MIntArray dummy;
	return	performLogicalOperation(operand,2,dummy);

}


//----------------------------------------------------------------------------------------------------
bool		BPT_BA::performLogicalOperation(const MIntArray& rhs, long operation,MIntArray& inResult)
//----------------------------------------------------------------------------------------------------
{



	//es wird ein IntArray zurückgegeben, dessen Indices im BA gefunden wurden - es wird als der match zurückgegeben

	ULONG l = rhs.length();

	ULONG origIncrement = inResult.sizeIncrement();
	inResult.setSizeIncrement(l / 2);		//die SpeicherEffizienz/geschwindigkeitsRelation ist am besten, 
											//wenn die Hälfte der indices im rhs auch wirklich gefunden werden

	
	ULONG wert;
	ULONG aIndex;
	ULONG rhsIndex;
	ULONG operand;
	ULONG shift;

	switch(operation)
	{
	case 0:	//& Operation
		{
			for(ULONG i = null; i < l; i++)
			{
				if(isFlagSet(rhs[i]))
					inResult.append(rhs[i]);
			}
			break;
		}
	case 1:	//- Operation -> muss den BA modifizieren, hat aber keinen Rückgabewert (erstmal)
		{

			for(ULONG i = null; i < l; i++)
			{
				rhsIndex = rhs[i];

				if(rhsIndex < lowBound || rhsIndex > highBound)
					continue;

				rhsIndex -= offset;
				
				shift = rhsIndex % nBitsInUlong;
				wert = one << shift;

				aIndex = (ULONG)(rhsIndex / nBitsInUlong);

				operand = array[aIndex] & wert;


				array[aIndex] += operand;	
				numTrue -= operand >> shift;

			}
			break;
		}

	case 2:	//+ Operation -> muss den BA modifizieren, hat aber keinen Rückgabewert (erstmal)
		{

			for(ULONG i = null; i < l; i++)
			{
				rhsIndex = rhs[i];

				if(rhsIndex < lowBound || rhsIndex > highBound)
					continue;
				
				rhsIndex -= offset;
				
				shift = rhsIndex % nBitsInUlong;
				wert = one << shift;

				aIndex = (ULONG)(rhsIndex / nBitsInUlong);

				operand = (array[aIndex] & wert) ^ wert;


				array[aIndex] += operand;	
				numTrue += operand >> shift;

			}
			break;
		}
	}


	inResult.setSizeIncrement(origIncrement);
	
	return true;
}


//----------------------------------------------------------------------------------------------------
bool		BPT_BA::performLogicalOperation(const BPT_BA& rhs, long operation, BPT_BA& inResult, bool keepLHSSize) 
//----------------------------------------------------------------------------------------------------
{
	//arrays müssen nicht gleichGroß sein - er nimmt das Gemeinsame Vielfache
	
	//größten Offset finden und kleinste Size
	ULONG rhsOffset		= rhs.getOffset();
	
	ULONG rhsHighBound	= rhs.getHighBound();
	
	ULONG myHighBound	= (highBound < rhsHighBound) ? highBound : rhsHighBound ;
	
	ULONG myOffset		= (offset > rhsOffset) ? offset : rhsOffset;

	//lowBound wird der von dem Array genommen, der auch schon den größten Offset hatte (damit beide zusammenpassen)
	ULONG myLowBound	= (lowBound > rhs.getLowBound()) ? lowBound : rhs.getLowBound();
	


	//fehlerprüfung
	if( (rhsOffset > highBound) || rhsHighBound < offset )
	{
		MPRINT("ARRAYS  HABEN KEIN GEMEINSAMES VIELFACHES!!");
		return false;
	}



	//Array größe errechen
	ULONG rest = (myHighBound - myOffset) % nBitsInUlong;

	ULONG allocSize = (myHighBound - myOffset - rest) / nBitsInUlong;


	allocSize += (rest > null) ? one : null;



	//chunks sind immer aligned, da offset immer vielfachhes von nBitsInUlong ist (kein SloMoMode mehr
	


	//chunks sind passend angeordnet, so dass die Chunks selbst verglichen werden können
	//hier muss bedacht werden, dass der beginn des & vergleichs eventuell mitten in einem Chunk liegt, 
	//so dass der errechnete Speicherbedarf für den ResultPtr eventuell nicht reicht.
	//deshalb auf nummer sicher gehen und einen Chunk mehr allokalisieren, wenn nötg

	//unnötig - glaube ich ...
//	if(	rest + (myOffset % nBitsInUlong) > nBitsInUlong)
//		allocSize++;



	//Array Ptr holen für schnellen speicherzugriff
	const ULONG* rhsArray = rhs.getPointer();

	//ArrayOffset für Arrays errechnen (indem man myOffset einfach als Index behandelt und (fast) die selbe formel nimmt wie im []operator
	long rhsAOffset = ( (long)myOffset - (long)rhsOffset ) / nBitsInUlong;
	long thisAOffset = ( (long)myOffset - (long)offset ) / nBitsInUlong;




	ULONG OPminOffset = null;
	ULONG* result = NULL;

	if(keepLHSSize) //nur für Operator minus (erstmnal), sollte also auch nur vpn OP -  gesetzt werden
	{//dieser flag bewirkt, dass das resultierende Array genau so groß ist wie das LHS array (dies ist wichtig bei der minus operation)

		MPRINT(" VERWENDE NUMCHUNKS ALS ARAYGRÖ?E VON RESULT");
		result = 	(ULONG*)malloc(numChunks * sizeof(ULONG));

		//jetzt alle Eintrage aus diesem Array un resultArray kopieren
		for(unsigned int i = null; i < numChunks; i++)
			result[i] = array[i];

		//GEFAHR!
		//OPminOffset anpassen (wenn die Arrays bei 0 anfangen ist das egal, aber bei allen anderen nicht mehr
		OPminOffset = thisAOffset;

		//ALLOCSIZE ist NIE größer als numChunks, da ja immer das KGV genommen wird

	}
	else
	{
		//alles klar,also erstmal ergenisArray ertstellen mit passender Größe
		//IM Standardfall kann einfach die AllocSize verwendet werden
		result = (ULONG*)malloc(allocSize * sizeof(ULONG));
	}

	
	//jetzt erstmal generell alle Chunks bearbeiten

	switch(operation)
	{
	case 0: //& OPERATION
		{
			for(ULONG x = null; x < allocSize; x++)
			{
				result[x] = array[x + thisAOffset] & rhsArray[x + rhsAOffset];
			}
			break;
		}


	case 1: //| OPERATION
		{
			for(ULONG x = null; x < allocSize; x++)
			{
				result[x] = array[x + thisAOffset] | rhsArray[x + rhsAOffset];
			}
			break;
		}


	case 2: //^ OPERATION
		{
			for(ULONG x = null; x < allocSize; x++)
			{
				result[x] = array[x + thisAOffset] ^ rhsArray[x + rhsAOffset];
			}
			break;
		}
	case 3: //- OPERATION (minus)
		{
			ULONG tmp;
			for(ULONG x = null; x < allocSize; x++)
			{
				tmp = array[x + thisAOffset] & rhsArray[x + rhsAOffset];	//da er die variable sonst wrapped, erstmal die bits finden, die übereinstimmen, und dann abziehen		
				result[x + OPminOffset] = array[x + thisAOffset] - tmp;
			}

			break;
		}
	}



	//und BA zurückgeben
	if(keepLHSSize)
	{// eventuell müssten hier auch einige überschüssige Bits null gesetzt werden (was ja eigentlich unnötig ist dank Hi und lowBound ...)
		inResult.useThisData(result, highBound, lowBound ,offset ,numChunks );
		return true;
	}
	
	
	//--> DIES IST JA EIGENTLICH UNNÖTIG ... (LO UND HIGHBOUNDS VERHINDERN EINEN ZUGRIFF AUF UNGESETZTE SPEICHERBEREICHE)

	//ansonten noch die Bits, welche eventuell berechnet würden, aber über den boundarys liegen, 0 setzen
	//LOW BOUND
	ULONG l = myLowBound % nBitsInUlong;
	ULONG wert;
	ULONG i;
	for(i = null; i < l ; i++)
	{
		wert = one << i;
		result[null] -= ( (result[null] & wert) == wert) ? wert : null;
	}

	//HIGH BOUND
	l = allocSize * nBitsInUlong;
	thisAOffset = allocSize - 1;	//ich verwende thisAOffset einfach nochmal, weil die variable jetzt eh nicht mehr benutzt wird
	for(i = myHighBound ; i < l; i++)
	{
		wert = one << i;
		result[thisAOffset] -= ( (result[thisAOffset] & wert) == wert) ? wert : null;
	}
	
	inResult.useThisData(result, myHighBound, myLowBound ,myOffset ,allocSize );

	return true;





}



//--------------------------------------------------------------------
MIntArray		BPT_BA::operator & (const MIntArray& rhs )
//--------------------------------------------------------------------
{

	MIntArray result;

	performLogicalOperation(rhs,0,result);

	return result;

}

//--------------------------------------------------------------------
bool		BPT_BA::operator + (const MIntArray& rhs )
//--------------------------------------------------------------------
{

	MIntArray result;

	return performLogicalOperation(rhs,2,result);

}

//--------------------------------------------------------------------
bool		BPT_BA::operator - (const MIntArray& rhs )
//--------------------------------------------------------------------
{

	MIntArray result;

	return performLogicalOperation(rhs,1,result);

}


//--------------------------------------------------------------------
bool	BPT_BA::_and_ (const BPT_BA& rhs, BPT_BA& result)
//--------------------------------------------------------------------
{

	return performLogicalOperation(rhs,0,result);

}

//--------------------------------------------------------------------
bool	BPT_BA::inclusive_or (const BPT_BA& rhs, BPT_BA& result)
//--------------------------------------------------------------------
{

	return performLogicalOperation(rhs,1,result);

}

//--------------------------------------------------------------------
bool	BPT_BA::exclusive_or (const BPT_BA& rhs, BPT_BA& result)
//--------------------------------------------------------------------
{

	return performLogicalOperation(rhs,2,result);

}

//--------------------------------------------------------------------
bool	BPT_BA::minus(const BPT_BA& rhs, BPT_BA& result)
//--------------------------------------------------------------------
{

	return performLogicalOperation(rhs,3,result, true);

}

//--------------------------------------------------------------------
BPT_BA	BPT_BA::operator & (const BPT_BA& rhs )
//--------------------------------------------------------------------
{

	BPT_BA result;
	performLogicalOperation(rhs,0,result);
	return result;

}

//--------------------------------------------------------------------
BPT_BA	BPT_BA::operator | (const BPT_BA& rhs )
//--------------------------------------------------------------------
{

	BPT_BA result;
	performLogicalOperation(rhs,1,result);
	return result;

}



//--------------------------------------------------------------------
BPT_BA	BPT_BA::operator ^ (const BPT_BA& rhs )
//--------------------------------------------------------------------
{

	BPT_BA result;
	performLogicalOperation(rhs,2,result);
	return result;

}

//--------------------------------------------------------------------
BPT_BA		BPT_BA::operator - (const BPT_BA& rhs )
//--------------------------------------------------------------------
{

	BPT_BA result;
	performLogicalOperation(rhs,3,result,true);
	return result;

}


#ifdef DEBUG

//--------------------------------------------------------------------
void	BPT_BA::print()
//--------------------------------------------------------------------
{
	

 	ULONG chunkSize = numChunks;
	ULONG i = null, r = null;
	ULONG l = null;
	
	cout<<endl<<"---------------------------------------------------"<<endl;
	cout<<"Logische Maximalgröße des Arrays ist: "<<highBound<<endl;
	cout<<"WertSpanne ist: "<<lowBound<<" bis "<<highBound<<endl;
	cout<<"Speichergröße in bit ist "<<numChunks * 8<<", (byte)"<<numChunks<<endl;
	cout<<"---------------------------------------------------"<<endl<<endl;

	for(ULONG x = null; x < chunkSize; x++)
	{
		cout<<"ZAHL in CHUNK:" <<array[x]<<endl;
		r = null;
		for(l = (x + one) * nBitsInUlong ; i < l; i++)
		{
			ULONG bitwert = one << r++;
			
			bool value = ( ((array[x]) & bitwert) == bitwert);
			//cout<<"Ergebnis des Logischen Tests: "<<( ((array[x]) & bitwert) == bitwert)<<endl;
			//cout<<"Ergenis der & OP: "<<( (array[x]) & bitwert)<<endl;
			cout<<"Index "<<offset + i<<" == "<<value<<endl;
		}
	}

}
#endif
