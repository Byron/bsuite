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



// STE MACROS, die fuer die Einrichtung der Parameter zustndig sind
// Diese auf jeden Fall aktuell halten, wenn irgendwan neue Parameter hinzukommen sollen
#	define STE_DEFINE_STATIC_PARAMETERS(prefix)		\
													\
		static	MObject	prefix##distanceObj;		\
		static	MObject	prefix##vfObj;				\
		static	MObject	prefix##ftObj;				\
		static	MObject prefix##inMatrixObj;		\
		static	MObject prefix##vtxOrigPosObj;		\
		static	MObject prefix##vtxSetObj;			\
		static	MObject	prefix##customSl;			\
		static	MObject	prefix##maxWeightsObj;		\
		static	MObject	prefix##smoothenObj;		\
		static	MObject	prefix##edgeDistanceObj;	\
		static	MObject	prefix##useMatrixRotationObj;\
		static	MObject	prefix##pushObj;			\
		static	MObject	prefix##pushRelativeObj;	\
		static	MObject	prefix##meshMatrixObj;		\
		static	MObject prefix##vtxColorObj;		\
		static	MObject prefix##vtxColorObj2;		\
		static	MObject prefix##pointSizeObj;		


#	define STE_DECLARE_STATIC_PARAMETERS(classname,prefix)	\
															\
		MObject	classname::prefix##distanceObj;			\
		MObject	classname::prefix##vfObj;				\
		MObject	classname::prefix##ftObj;				\
		MObject classname::prefix##inMatrixObj;			\
		MObject classname::prefix##vtxOrigPosObj;		\
		MObject classname::prefix##vtxSetObj;			\
		MObject	classname::prefix##customSl;			\
		MObject	classname::prefix##maxWeightsObj;		\
		MObject	classname::prefix##smoothenObj;			\
		MObject	classname::prefix##edgeDistanceObj;		\
		MObject	classname::prefix##useMatrixRotationObj;\
		MObject	classname::prefix##pushObj;				\
		MObject	classname::prefix##pushRelativeObj;		\
		MObject	classname::prefix##meshMatrixObj;		\
		MObject classname::prefix##vtxColorObj;			\
		MObject classname::prefix##vtxColorObj2;		\
		MObject classname::prefix##pointSizeObj;		



#	define	STE_ADD_ATTRIBUTES(prefix)														\
																							\
	MFnNumericAttribute	numFn;																\
	MFnEnumAttribute	enumFn;																\
	MFnTypedAttribute	typedFn;															\
																							\
																							\
	prefix##inMatrixObj = typedFn.create("matrix","mat",MFnData::kMatrix );					\
																							\
	prefix##meshMatrixObj = typedFn.create("meshMatrix","mmx",MFnData::kMatrix );			\
																							\
																							\
																							\
																							\
	/*########################################*\											\
	VTXCOLOR 																				\
	\*########################################*/											\
																							\
	prefix##vtxColorObj = numFn.createColor("aColor1","col1");								\
	numFn.setKeyable(false);																\
	numFn.setStorable(true);																\
	numFn.setDefault(1.0,0.0,0.1);															\
	numFn.setInternal( true );																\
																							\
	/*########################################*\											\
	VTX COLOR2																				\
	\*########################################*/											\
																							\
	prefix##vtxColorObj2 = numFn.createColor("aColor2","col2");								\
	numFn.setKeyable(false);																\
	numFn.setStorable(true);																\
	numFn.setDefault(1.0,0.785,0.0);														\
	numFn.setInternal( true );																\
																							\
	/*########################################*\											\
	POINTSIZEOBJ																			\
	\*########################################*/											\
																							\
	prefix##pointSizeObj = numFn.create( "pointSize", "ps",	MFnNumericData::kFloat, 10.0 );	\
	numFn.setKeyable(false);																\
	numFn.setStorable(true);																\
	numFn.setInternal( true );																\
																							\
																							\
	/*########################################*\											\
	DISTANCE																				\
	\*########################################*/											\
																							\
	prefix##distanceObj = numFn.create("distance","dis",MFnNumericData::kDouble);			\
	numFn.setMin(0.0);																		\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
	numFn.setInternal( true );																\
	/*########################################*\											\
	PUSH																					\
	\*########################################*/											\
																							\
	prefix##pushObj = numFn.create("push","psh",MFnNumericData::kDouble);					\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
																							\
																							\
  	/*########################################*\											\
	PUSH RELATIVE																			\
	\*########################################*/											\
																							\
	prefix##pushRelativeObj = numFn.create("pushRelativeToFaceSize","phr",MFnNumericData::kBoolean);	\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
																							\
	/*########################################*\											\
	smoothen																				\
	\*########################################*/											\
																							\
	prefix##smoothenObj = numFn.create("smoothen","smo",MFnNumericData::kFloat, 1.0);		\
	numFn.setMin(0.0);																		\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
	numFn.setInternal( true );																\
																							\
	/*########################################*\											\
	edgeDistance																			\
	\*########################################*/											\
																							\
	prefix##edgeDistanceObj = numFn.create("edgeDistance","edi",MFnNumericData::kLong);		\
	numFn.setMin(0.0);																		\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
	numFn.setInternal( true );																\
																							\
	/*########################################*\											\
		SHOW WEIGHTS																		\
	\*########################################*/											\
																							\
	prefix##vfObj = numFn.create("showWeights","sw",MFnNumericData::kBoolean);				\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
																							\
																							\
	/*########################################*\											\
		USE MATRIX ROTATION																	\
	\*########################################*/											\
																							\
	prefix##useMatrixRotationObj = numFn.create("useMatrixRotation","umr",MFnNumericData::kBoolean);\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
																							\
																							\
	/*########################################*\											\
		FALLOFF TYPE																		\
	\*########################################*/											\
	prefix##ftObj = enumFn.create("falloffType","ft");										\
	enumFn.addField("linear", 0);															\
	enumFn.addField("smooth", 1);															\
	enumFn.addField("spike", 2);															\
	enumFn.addField("dome", 3);																\
	enumFn.setStorable(true);																\
	enumFn.setWritable(true);																\
	enumFn.setKeyable(true);																\
																							\
																							\
	/*########################################*\											\
		CACHED VTX POSITIONS																\
	\*########################################*/											\
																							\
	prefix##vtxOrigPosObj = typedFn.create("cachedVertexPositions","cvp",MFnData::kPointArray);	\
	typedFn.setStorable(true);																	\
	typedFn.setWritable(true);																	\
	typedFn.setHidden(true);																	\
																								\
																								\
	/*########################################*\											\
		VTX SET - COMPONENT LIST															\
	\*########################################*/											\
																								\
	prefix##vtxSetObj = typedFn.create("vertexList","vl",MFnData::kComponentList);				\
	typedFn.setStorable(true);																	\
	typedFn.setWritable(true);																	\
	typedFn.setHidden(false);																	\
																								\
																								\
																								\
	prefix##customSl = numFn.create("customSlFalloff","csl",MFnNumericData::kDouble);			\
	numFn.setMin(0.0);																			\
	numFn.setMax(1.0);																			\
	numFn.setKeyable(true);																		\
																								\
																								\
	/*########################################*\											\
	MAXWEIGHTS																				\
	\*########################################*/											\
																							\
	prefix##maxWeightsObj = numFn.create("maximumWeights","mw",MFnNumericData::kLong, 1.0);	\
	numFn.setMin(1);																		\
	numFn.setSoftMax(8);																	\
	numFn.setStorable(true);																\
	numFn.setWritable(true);																\
	numFn.setKeyable(true);																	\
	numFn.setInternal( true );																\
																							\
																								\
																								\
																								\
	/*jetzt die Atribute hinzufuegen																\
	//-------------------------------*/															\
																								\
	status = MPxNode::addAttribute(prefix##vfObj);												\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##distanceObj);										\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##edgeDistanceObj);									\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
  	status = MPxNode::addAttribute(prefix##pushObj);											\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
    	status = MPxNode::addAttribute(prefix##pushRelativeObj);								\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##ftObj);												\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
																								\
	status = MPxNode::addAttribute(prefix##vtxOrigPosObj);										\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##vtxSetObj);											\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##customSl);											\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##inMatrixObj);										\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##maxWeightsObj);										\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
	status = MPxNode::addAttribute(prefix##smoothenObj);										\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
  	status = MPxNode::addAttribute(prefix##useMatrixRotationObj);								\
	MCheckStatus(status, "AddAttrSoftEngine");													\
																								\
    status = MPxNode::addAttribute(prefix##meshMatrixObj);										\
	MCheckStatus(status, "AddMeshMatrix");													\
																								\
	status = MPxNode::addAttribute(prefix##vtxColorObj);										\
	MCheckStatus(status, "AddVtxColor");													\
																								\
	status = MPxNode::addAttribute(prefix##vtxColorObj2);										\
	MCheckStatus(status, "AddVtxColor2");													\
																								\
	status = MPxNode::addAttribute(prefix##pointSizeObj);										\
	MCheckStatus(status, "AddPointSize");													\
																								\
	/* Attribute Affects setzen																	\
	//-------------------------------*/															\
																								\
	status = MPxNode::attributeAffects( prefix##ftObj, prefix##outMesh );						\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##vfObj, prefix##outMesh );						\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##distanceObj, prefix##outMesh );					\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects(prefix##customSl,prefix##outMesh);						\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
  	status = MPxNode::attributeAffects( prefix##inMatrixObj, prefix##outMesh );					\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##maxWeightsObj, prefix##outMesh );				\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##smoothenObj, prefix##outMesh );					\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##edgeDistanceObj, prefix##outMesh );				\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##useMatrixRotationObj, prefix##outMesh );		\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
	status = MPxNode::attributeAffects( prefix##pushObj, prefix##outMesh );						\
	MCheckStatus(status, "SetAttrAffects");														\
																								\
  	status = MPxNode::attributeAffects( prefix##pushRelativeObj, prefix##outMesh );				\
	MCheckStatus(status, "SetAttrAffects");														
																								


#define		STE_CHECK_INTERNALS( prefix,datahandle )														\
																								\
	if( plug == prefix##vtxColorObj || plug == prefix##vtxColorObj2 || plug == prefix##pointSizeObj )	\
	{																							\
		/* Den screen refreshen, wenn eines der zechenrelevanten Plugs gesetzt wurde,			\
			damit diese sich aktualisieren.														\
		*/																						\
																								\
		M3dView::active3dView().refresh(true, true);											\
																								\
	}																							\
																								\
	/* Wenn es sich um ein plug handelt, welches nocht kleiner 0 sein darf, dann diese			\
		Regelung durchsetzen																	\
	*/																							\
	else if(		plug == prefix##distanceObj		)											\
	{																							\
		double value = datahandle.asDouble();													\
																								\
																								\
		if( value < 0.0 )																		\
			const_cast<MDataHandle&>(datahandle).set( (double)0.0 );							\
																								\
	}																							\
	else if(		plug == prefix##edgeDistanceObj												\
				||																				\
					plug == prefix##smoothenObj													\
				||																				\
					plug == prefix##maxWeightsObj	)											\
	{																							\
		int value = datahandle.asInt();															\
																								\
																								\
		if( value < 0 )																			\
			const_cast<MDataHandle&>(datahandle).set( (int)0 );									\
																								\
	}	


