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

// baseNode.h: interface for the baseNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASENODE_H__32B186FA_5310_4A64_B613_9BE3ECB0FC2A__INCLUDED_)
#define AFX_BASENODE_H__32B186FA_5310_4A64_B613_9BE3ECB0FC2A__INCLUDED_


template <class T>
class baseNode  
{
public:
	baseNode(){};
	virtual ~baseNode(){};

	
	virtual void	getIndex(unsigned int count,unsigned int index,baseNode<T>*& node) = 0;

	virtual T*&				getValue() = 0;
	virtual T*				getValue() const = 0;

	

};

#endif // !defined(AFX_BASENODE_H__32B186FA_5310_4A64_B613_9BE3ECB0FC2A__INCLUDED_)
