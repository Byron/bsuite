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

#ifndef _polyModifierNode
#define _polyModifierNode
//
// Copyright (C) 2002 Alias | Wavefront
// 
// File: polyModifierNode.h
//
// Dependency Graph Node: polyModifierNode
//
// Author: Lonnie Li
//
// Overview:
//
//		The polyModifierNode class is a intermediate class used by polyModifierCmd to
//		modify a polygonal object in Maya. The purpose of the polyModifierNode is to 
//		generalize a node that can receive an input mesh object, modify the mesh and
//		return the modified mesh.
//
//		polyModifierNode is an abstraction which ds not need to know about the DG
//		and simply needs to know about the process outlined above. polyModifierCmd
//		manages when and how this node will be used.
//
//		Each polyModifierNode is recommended to contain an instance of a polyModifierFty
//		which will do the actual work, leaving only the node setup and attribute
//		associations to the node. The recommended structure of a polyModifierNode is
//		as follows:
//
//              _____________
//             /        ___  \
//           /         /   \   \
//        O |   Node  | Fty |   | O
//        |  \         \___/   /  |
//        |    \_____________/    |
//      inMesh                 outMesh
//
//
//		The purpose of the node is to simply define the attributes (inputs and outputs) of
//		the node and associate which attribute affects each other. This is basic node setup
//		for a DG node. Using the above structure, the node's inherited "compute()" method
//		(from MPxNode) should retrieve the inputs and pass the appropriate data down to the
//		polyModifierFty for processing.
//
//
// How to use: 
//
//		(1) Create a class derived from polyModifierNode
//		(2) Define and associate inMesh and outMesh attributes (inMesh --> affects --> outMesh)
//		(3) Add any additional attributes specific to the derived node and setup associations
//		(4) Define an instance of your specific polyModifierFty to perform the operation on the node
//		(5) Override the MPxNode::compute() method
//		(6) Inside compute():
//
//			(a) Retrieve input attributes
//			(b) Use inputs to setup your factory to operate on the given mesh
//			(c) Call the factory's inherited doIt() method
//
//
// Proxies
//
#include <maya/MPxNode.h>

class polyModifierNode : public MPxNode
{
public:
						polyModifierNode();
	virtual				~polyModifierNode(); 

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static  MObject		inMesh;
	static  MObject		outMesh;
};

#endif
