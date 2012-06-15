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

#ifndef BPT_BA_INCLUDE
#define BPT_BA_INCLUDE



#include <maya/MIntArray.h>
#include <malloc.h>


#include <MACROS.h>


namespace BPT
{

/** Implementation einer BitArray Klasse zum effizienten Handlen von grossen bool datenmengen

  @remarks
	Diese Klasse wurde speziell fuer die Zusammenarbeit mit MIntArrays entwickelt und ist optimiert fuer speed
	
*/

class BPT_BA
{

//--------------------------
//Konstruktion/Destuktion
public:
//-----------------------

	BPT_BA();												//!< setzt alle elemente 0
	
	BPT_BA(ULONG initialSize, bool initialValue = false);	//!< initialisiert BA mit gegebener groesse, und setzt alle bits auf initialValue
	
	/** Konstruktor
	@param
		alwaysStartAtZero baut das BA mit lowBound = 0 auf
	@param 
		initialValue false bedeutet, dass der BA defalt wert false ist, ansonsten ist es true (und IntArrayIndices sind false)
	@param
		maxLength wenn man dem highBound vorgibt, muss man nicht danach suchen, was sdie leistung steigert
	*/
	BPT_BA(		const MIntArray& initialArray,
				bool alwaysStartAtZero = false,				
				bool initialValue = false,					
				ULONG maxLength = 0);						
															
															

	BPT_BA(const BPT_BA& rhs);								//!< KopierKonstruktor - erstellt Tiefe Kopie der übergebenen Referenz auf BA

	~BPT_BA(void);




public:
//--------------------------
// ZUGRIFFSMETHODEN 
//--------------------------

	ULONG			getHighBound()	const {return highBound;};
	ULONG			getLowBound()	const {return lowBound;};
	ULONG			getOffset()		const {return offset; };
	const ULONG*	getPointer()	const {return array;};
	ULONG			getNumChunks()	const {return numChunks; };

	ULONG			getTrueCount()	const {return numTrue;};
	ULONG			getFalseCount()	const {return (highBound - lowBound) - numTrue; };

	ULONG			getFirstBitIndex() const;	//!< diese Methode vorsichtig einsetzen, da sie einen Runup benoetigt, um das erste bit zu finden
	ULONG			getLastBitIndex() const;	//!< diese Methode vorsichtig einsetzen, da sie einen Runup benoetigt, um das letzte bit zu finden


	bool			expand		(ULONG  expandBy, 
								 bool valueOfNewBits = false);		//!< array um x Bits expandieren, bool wird den neuen Bits zugewiesen
	
	bool			contract	(ULONG  contractBy);				//!< array um x bits verkleinern
	
	bool			setLength	(ULONG  newLength, 
								 bool valueOfNewBits = false);		//!< arraygroesse auf x Bits festsetzen, bool wird neuen bits zugewiesen, wenn vorhanden


	//SICHERE METHODEN:	Machen Rangecheck und BitCheck
	void			getIntArray		(MIntArray& inResult) const;	//!< gibt intArray zurück mit indices, die im BA true waren

	bool			findMatching	(MIntArray& operand, 
									 MIntArray& emptyResult	);		//!< findet UEbereinstimmungen zu dem übergebenen IntArray (SICHER dank vollem rangeCheck)
	
	bool			remove( MIntArray& operand );					//!< entfernt das übergebene Array vom Bitarray 
																	//!< (setzt die gefundenen Bits False, wenn moeglich) - SICHER

	bool			add (  MIntArray& operand );					//!< fügt das übergebene intarray zum BitArray hinzu, aber erweitert es nicht
																	//!< (setzt die gefundenen Bits True, wenn moeglich) - SICHER

	bool			add (	MIntArray& operand,
							ULONG operandMax = 0,
							ULONG operandMinSize = 0     );			//!< -->wenn man diesen Wert setzt, muss er den index nicht selbst suchen
																	//!< fügt das übergebene intarray zum BitArray hinzu und erweitert es, wenn noetig
	
	bool			isFlagSet (long index) const;					//!< BitFlag abrufen als Bool ->SICHER


	//UNSICHERE METHODEN:	ALLE OHNE RANGECHECK, sind folglich aber schneller
	void			setBits (const MIntArray& indices, bool value);	//!< setzt die Bits entsprechend der enthaltenen indices auf den übergebenen Wert
																	//!< "remove" verwenden oder "add" für VOLLE SICHERHEIT (mit rangeCheck)

	void			setBitTrue		(ULONG index);					//!< UNSICHER - kein RangeCheck, aber bitCheck
	void			setBitFalse		(ULONG index);					//!< UNSICHER - kein RangeCheck, aber bitCheck

	void			invertBits (MIntArray& indices);				//!< true wird false, false wird true, dort wo indices[x] hinzeigt.
																	//!< DIESE METHODE MACHT KEINEN RANGECHECK

	void			setAllFalse();									//!< setzt alle Bits false; Quasi eine art Reset

//--------------------------
//FUNKTIONEN
//--------------------------

private : void	useThisData(	ULONG* allocatedMemoryPtr, 
						ULONG inHighBound,
						ULONG inLowBound,
						ULONG inOffset,
						ULONG inNumChunks  );				//!< kopiert die übergebenen Daten in die eigene Instanz (effizienzsteigerung)

public: 
/** findet die Zahl an trueBits heraus. 
 Diese Methode sollte nach einer BAmitBA Aktion
 gerufen werden, da diese die numTrue verlieren und sich (um zeit zu sparen) nicht
 automatisch synchronisieren ->GIBT NUMTRUE ZURUECK 
*/
ULONG	syncNumTrue();										

//------------------------------------
//EFFIZIENTE LOGISCHE OPERATIONEN
//------------------------------------

//MERKE!!!: DIESE FUNKTIONEN BEACHTEN DAS SIZE ATTRIBUT NICHT! DIES IST BEI DER AUSCHLIE?LICHEN VERWENDUNG VON BA mit BA IRRELEVANT (zur Zeit)
//result hat also keinen gültigen size wert, was wirklich massig rechenzeit spart
//will mans doch, dann syncNumTrue aufrufen

bool	_and_ (const BPT_BA& rhs, BPT_BA& result);				//!< AND OP, nur das er effizienter ist, da kein BA aus der procedur herauskopiert werden muss
bool	inclusive_or (const BPT_BA& rhs, BPT_BA& result);		//!< | OP
bool	exclusive_or (const BPT_BA& rhs, BPT_BA& result);		//!< ^ OP
bool	minus(const BPT_BA& rhs, BPT_BA& result);				//!< - OP


//hier eventuell noch funktionen hinzufügen, die direkt auf dem parentArray arbeiten
//wenn ich's brauche


//--------------------------
//OPERATOREN
//--------------------------
	
/*! 
BitFlag abrufen als bool.	
Daten koennen nicht direkt gesetzt werden, da sie eigentlich keine Bools sind 
*/
bool				operator [] (long index) const;	

//MERKE!!!: DIESE FUNKTIONEN BEACHTEN DAS SIZE ATTRIBUT NICHT! DIES IST BEI DER AUSCHLIE?LICHEN VERWENDUNG VON BA mit BA IRRELEVANT (zur Zeit)
//result hat also keinen gültigen size wert, was wirklich massig rechenzeit spart

//BA Operatern ->arbeiten auf vollen BitArrays und geben BA zurück
BPT_BA&				operator = (const BPT_BA& rhs);

BPT_BA				operator & (const BPT_BA& rhs );	//!< bitweiser AND operator

BPT_BA				operator | (const BPT_BA& rhs );	//!< bitweiser inclusiv OR operator	(Diese OPs kann man auch mit IntArrays verwenden dank impliziter Umwandlung)

BPT_BA				operator ^ (const BPT_BA& rhs );	//!< bitweiser exclusiv OR operator

BPT_BA				operator - (const BPT_BA& rhs );	//!< MINUS operator	//zieht bits des rhs von lhs ab


//IntArray Operatoren
MIntArray			operator & (const MIntArray& rhs );	//!< bitweiser AND operator für IntArrays

//Anwendung: BA + IntArray, BA - IntArray (SIZE WIRD BEACHTET)
bool				operator + (const MIntArray& rhs ); //!< fügt die indices aus rhs in lhs ein, und gib einfach bool zurück
bool				operator - (const MIntArray& rhs ); //!< entfernt die indices aus rhs aus lhs, und gib einfach bool zurück

//--------------------------
//DEBUG
//--------------------------

#ifdef DEBUG
void			print();

static			ULONG		counter;

unsigned		long		number;
#endif

//--------------------------
//PRIVATE FUNKTIONEN
private:
//--------------------------

bool				performLogicalOperation(const BPT_BA& rhs, 
											long operation,
											BPT_BA& inResult, 
											bool keepLhsSize = false);	//!< arbeitet auf BAs

bool				performLogicalOperation(const MIntArray& rhs,
											long operation,
											MIntArray& inResult);		//!< arbeitet auf IntArrays

//--------------------------
//PRIVATE DATENELEMENTE
private:
//--------------------------

ULONG		lowBound;	//!< da unter Umständen auch mehr Bits bearbeitet werden, als "offiziell" im Array gespeichert werden, muss im nachhinein noch dafür gesorgt
						//!< werden, dass diese Bits wieder 0 gesetzt werden - dies geschieht mit lowBound, um zu wissen, wo die untere Grenze ist

ULONG		highBound;	//!< groesse des Arrays in bit, die für Werte vorgesehen sind (array ist ja im grunde immer groesser als size und ist vielfaches von 32)
						//!< Highbound war früher Size, ist mit der neuen Benennung allerdings konsistenter

ULONG		offset;		//!< offset ist die Zahl in Bit, die das Array nach links verschoben ist -> immer vielFaces von 32!; 
ULONG		numChunks;	//!< Zahl der Longs im Array (da sizeOf ptr nur die Groesse des Ptrs gibt -> vielleicht sollte man ihn mal dereferenzieren?)

ULONG		numTrue;	//!< zahl der true gesetzten Flags -> wird immer entsprechend aktualisiert (bei verwendung mit IntArrays)
						//!< auf diese Weise kann man BA wie normalen Array benutzen

//Speicher Poiner
//------------------

ULONG*		 array;		//!< zeigt auf den Begin des zugrundeliegenden LongArrays - es wird allerdings nur mit unsigned longs gefüllt, damit kein sign Bit vorhanden ist;


//Konstanten (damit diese nich immer als literale konstanten erzeugt werden, um dann wieder verworfen zu werden
//------------------
static const int   nBitsInUlong;	//!< number of bits per chunk
static const ULONG null;
static const ULONG one;
static const ULONG fulllong;

};

}

#endif
