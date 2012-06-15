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

// linkedNode.h: interface for the linkedNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINKEDNODE_H__A730C01B_EA7C_4602_BDEB_706BC5D18211__INCLUDED_)
#define AFX_LINKEDNODE_H__A730C01B_EA7C_4602_BDEB_706BC5D18211__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



template <class T>
class linkedNode 
{
public:

	linkedNode():object(0),next(0){};
	linkedNode(T* inObject):object(inObject),next(0){}; //mit dem unteren destruktor wird ein objekt übrigbleiben, mindestens
	~linkedNode(){ /*if(object != 0)delete object;*/ if(next != 0)delete next; };
	//~linkedNode(){cout<<"RUFE LINKED NODE DESTRUKTOR"<<endl;};
	

	void	getIndex( int count, int index,linkedNode<T>*& node);

	

	T*&				getValue(){return object;};
	T*				getValue() const {return object;};

	linkedNode<T>* next;

	

private:
	T* object;

	

};

template <class T>
void	linkedNode<T>::getIndex( int count, int index,linkedNode<T>*& node)
{	

	if(count == index)
		node = this;
	else
		next->getIndex(++count,index,node);

}

#endif // !defined(AFX_LINKEDNODE_H__A730C01B_EA7C_4602_BDEB_706BC5D18211__INCLUDED_)
