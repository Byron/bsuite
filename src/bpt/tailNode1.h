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

// tailNode1.h: interface for the tailNode class.
	//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TAILNODE1_H__AF2DA1AC_0F1A_4980_9241_CFEA2DEBCAF4__INCLUDED_)
#define AFX_TAILNODE1_H__AF2DA1AC_0F1A_4980_9241_CFEA2DEBCAF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "baseNode.h"

template <class T>
class tailNode : public baseNode<T>
{
public:
	tailNode<T>(){};
	~tailNode<T>(){};

	
	virtual void	getIndex(unsigned int count,unsigned int index,baseNode<T>*& node){};
	
	virtual T*&				getValue(){return new T;};
	virtual T*				getValue() const {return new T;};

	baseNode<T>* last;

};

#endif // !defined(AFX_TAILNODE1_H__AF2DA1AC_0F1A_4980_9241_CFEA2DEBCAF4__INCLUDED_)
