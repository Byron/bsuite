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

// simpleLinkedList.h: interface for the simpleLinkedList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLELINKEDLIST_H__A9303B1E_487E_4EDA_920F_2CD39FED5918__INCLUDED_)
#define AFX_SIMPLELINKEDLIST_H__A9303B1E_487E_4EDA_920F_2CD39FED5918__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "linkedNode.h"


template <class T>
class simpleLinkedList  
{
public:
	simpleLinkedList<T>():size(0),currentPosition(0),currentPositionPtr(0),startPosition(0){tail = new linkedNode<T>;};
	simpleLinkedList<T>(const simpleLinkedList<T>& other){ };
	~simpleLinkedList<T>(){/*cout<<"RUFE SIMPLELINKEDLIST DESTRUKTOR"<<endl;*/ tail->next = 0; delete tail; if(size != 0){ delete startPosition;}  };
	//~simpleLinkedList<T>(){cout<<"RUFE SIMPLELINKEDLIST DESTRUKTOR"<<endl;/*delete startPosition;*/};

	int length(){return size;};

	void append(T* object);

	void	clear(){ tail->next = 0;delete tail; if(size != 0){ delete startPosition;} };


	T*& operator[]( int index ); //Dieser operator erlaubt auch zuweisung, der untere nicht
	
	T* operator[]( int index ) const;


private:

	linkedNode<T>* currentPositionPtr;

	linkedNode<T>* startPosition;

	linkedNode<T>* tail;

	
	
	int size ;

	int currentPosition;

};



template <class T>
void simpleLinkedList<T>::append(T* object)
{
	if(size == 0)
	{
		linkedNode<T>* node = new linkedNode<T>(object);
		tail->next = node;	//im nextSlot von tial ist immer die letzte gueltige node gespeichert (next ist sozusagen last)
		startPosition = currentPositionPtr = node;
	}
	else
	{
		linkedNode<T>* node = new linkedNode<T>(object);
		tail->next->next = node;
		tail->next = node;
		//node wird nie mit tail verbunden, damit die destruction noch funzt
	}
	++size;
}


template <class T>
T*& simpleLinkedList<T>::operator[]( int index )
{
	linkedNode<T>* tmpNode = 0;

//	if(index >= size || index < 0)
//	{
//		cout<<"WARNUNG, Index zu gross, habe ihn korrigiert"<<endl;
//		index = size - 1;
//	}
//	cout<<"SL: Index vs Size: "<<index<<" vs "<<size<<endl;
	//kein Check, ob index ueber die vorhandenen slots hinaus geht
	if(index >= currentPosition)
	{
		currentPositionPtr->getIndex(currentPosition,index,tmpNode);
		currentPosition = index; currentPositionPtr = tmpNode; 
		return tmpNode->getValue();
	}
	else
	{//ansonsten wieder von 0 anfangen zu zhlen
		currentPosition = index;
		startPosition->getIndex(0,index,tmpNode);
		currentPositionPtr = tmpNode;
		return tmpNode->getValue();
	}
	
	
}

template <class T>
T* simpleLinkedList<T>::operator[]( int index ) const
{
//	if(index >= size)
//{
	//	cout<<"WARNUNG, Index zu gross, habe ihn korrigiert"<<endl;
	//	index = size - 1;
	//}

//	cout<<"SL: Index vs Size: "<<index<<" vs "<<size<<endl;

	linkedNode<T>* tmpNode = 0;
	//kein Check, ob index ueber die vorhandenen slots hinaus geht
	if(index >= currentPosition)
	{
		currentPositionPtr->getIndex(currentPosition,index,tmpNode);
		currentPosition = index; currentPositionPtr = tmpNode; 
		return tmpNode->getValue();
	}
	else
	{//ansonsten wieder von 0 anfangen zu zhlen
		currentPosition = index;
		startPosition->getIndex(0,index,tmpNode);
		currentPositionPtr = tmpNode;
		return tmpNode->getValue();
	}
	
}

#endif // !defined(AFX_SIMPLELINKEDLIST_H__A9303B1E_487E_4EDA_920F_2CD39FED5918__INCLUDED_)
