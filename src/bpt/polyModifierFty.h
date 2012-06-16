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

#ifndef _polyModifierFty
#define _polyModifierFty
//
// Copyright (C) 2002 Alias | Wavefront 
// 
// File: polyModifierFty.h
//
// polyModifier Factory: polyModifierFty
//
// Author: Lonnie Li
//
// Overview:
//
//		The polyModifierFty class is the main workhorse of the polyModifierCmd operation.
//		It is here that the actual operation is implemented. The idea of the factory is
//		to create a single implementation of the modifier that can be reused in more than
//		one place.
//
//		As such, the details of the factory are quite simple. Each factory contains a doIt()
//		method which should be overridden. This is the method which will be called by the
//		node and the command when a modifier is requested.
//
// How to use:
//
//		1) Create a factory derived from polyModifierFty
//		2) Add any input methods and members to the factory
//		3) Override the polyModifierFty::doIt() method
//
//			(a) Retrieve the inputs from the class
//			(b) Process the inputs
//			(c) Perform the modifier
//
//

// General Includes
//
#include <maya/MStatus.h>

class polyModifierFty
{
public:
						polyModifierFty();
	virtual				~polyModifierFty();

	// Pure virtual doIt()
	//
	virtual MStatus		doIt() = 0;
};

#endif
