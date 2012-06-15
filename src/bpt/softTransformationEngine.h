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

// softTransformationEngine.h: interface for the softTransformationEngine class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SOFTTRANSFORMATIONENGINE_H
#define SOFTTRANSFORMATIONENGINE_H


//#########################
// Maya Includes
//#########################

// ARRAYS
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MObjectArray.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MPlugArray.h>


// ATTRIBUTES
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>


// DATA FUNCTIONSETS
#include <maya/MFnPointArrayData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnNumericData.h>

// OTHER FUNCTIONSETS
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimUtil.h>


// ITERATOREN/MESHFNs
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>


// DIVERSES
#include <maya/MString.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MDagModifier.h>
#include <maya/MObject.h>
#include <maya/MEventMessage.h>
#include <maya/MPxNode.h>
#include <maya/MSelectionList.h>
#include <maya/MColor.h>



// EIGENE
#include <MACROS.h>
#include <STEMacros.h>
#include <BPT_Helpers.h>
#include <polyModifierNode.h>
#include <math.h>
#include <componentConverter.h>
#include <GLDrawNode.h>


// STL

#include <map>
#include <vector>



using std::map;
using std::vector;


// Typedefs
typedef	std::pair<double, int> wp;		//!< == WeightPair - double als Key, int als dazugeh�rige localID f�rs PositionArray des vtxSets


namespace BPT
{

	/** Die softTransformationEngine implementiert eine generische Klasse, welche die weiche Transformation von Vertizen erm�glicht

	@remarks
		Die STE kann leicht an beliebige nodes geh�ftet werden, so dass diese softTransformationF�higkeiten 
		erh�lt.

	@par
		Sie unterst�tzt 2 Modi: 
	*		-Transformation anhand von parentVtx 
	*		-Transformation anhand einer Matrix

	@par
		Integriert wird sie mithilfe von Macros (leider n�tig aufgrund von Limitationen), welche in Macros.h definiert sind.
	  
	*/
	
	class softTransformationEngine : public GLDrawNode
	{
		

	// -------------------------------------------------------
	// ENUMS
	private:
	// -------------------------------------------------------
		/** Enum, die den Status des eigenen Meshes angibt
				
			Wird zum zeichnen verwendet.
		*/
		enum	meshStatus{	kNone, 
							kSelected, 
							kHilited } lastStat;



		//---------------------------
		//	KONSTRUKTION/DESTRUKTION
	public:
		//---------------------------
		softTransformationEngine();
		virtual ~softTransformationEngine();
		

		//---------------------------
		//	NODE ATTRIBUTE
	public:
		//---------------------------
	
		// Werden von der abgeleiteten Node definiert (eindeutig)

		//---------------------------
		//	�FFENTLICHE METHODEN
	public:
		//---------------------------

		MStatus	extractNonstaticData(	const MPlug& plug, 
										MDataBlock& data);					//!< liest die Atttribute der SoftTransformationEngine, 
																			//!< so dass alle methoden darauf zugreifen k�nnen - nur die, die sich st�ndig ver�ndern k�nnen
																			
		MStatus	extractStaticData(const MPlug& plug, MDataBlock& data);		//!< liest die VtxOrigPos, so vorhanden, und das VtxSet

		void	doTweak(MDataBlock& data);									//!< f�hrt die eigentliche Transformation durch und setzt outWeights

		void	setVtxSet(MDataBlock& data);								//!< erstellt aus den Daten eine ComponentList 

		void	gatherAttributeObjects(const MObject& thisNode);			//!< Holt die AttributObjekte mithilfe einer MFnDepNode

		void	setVtxSet(const MIntArray& mewIDs, MDataBlock& data);		//!< wie oben, nur mit intArray als input

		void	saveMeshPathes();											//!< holt einen pfad zur MeshShape, und einen zur Transform


		//---------------------------
		//	ZUGRIFFS METHODEN
	public:
		//---------------------------
		void	getMeshPath(MDagPath& inPath){ if(! meshPath.isValid() ) saveMeshPathes(); inPath = meshPath; };




		//---------------------------
		//	DRAW METHODEN
	public:
		//---------------------------
		/// zeichnet die weights
		//
		void	draw(M3dView& view);


		
	
		// **********************
		// DRAW HELFER METHODEN
		private:
		// **********************
		
		meshStatus	getMeshStatus();				//!< Gibt den gegenw�rtigen status des meshes zur�ck

		bool		nodeStatusAllowsDrawing();		//!< Analysiert den status der BPTNode und entscheidet, ob gezeichnet werden darf

		void		drawShadedTriangles(		MItMeshPolygon& polyIter, 
												MItMeshVertex& vertIter, 
												M3dView::DisplayStyle style, 
												meshStatus meshStat);

		void		drawPoints( MItMeshVertex& vertIter, 
								float fPointSize);


		MColor		getCalColor(const MColor& color1, 
								const MColor& color2, 
								double weight);



		//---------------------------
		//	PRIVATE METHODEN ( TWEAK )
	private:
		//---------------------------

		bool	rebuildTweakArrays();					//!< Baut s�mtliche Arrays neu auf, die f�r die gewichtete Transformation ben�tigt werden
		
		void	recalculateTweakScaleFactors( );		//!< Resampled die linearen Weights

		void	clearAnimCurve();

		void	initAnimFn();							//!< richtet das animFn functionSet ein

		void	createWeightsBasedOnDistance();			//!< Erzeugt die Weights unter Verwendung der Distance

		void	createWeightsBasedOnEdgeDistance();		//!< Weights auf Basis der edgeLength;

		void	generateNormalVectors();				//!< holt die normalVektoren der TweakVtx

		void	generateEdgeWeights(const MIntArray& vtxSet, 
									uint ringNumber, 
									MItMeshVertex& inVertIter,
									int minID = -1);	//!< berechnet und setzt die weights f�r einen growRing

		// EdgeLength
		void	copyArrayPartial(	MIntArray& copyTo, 
									const MIntArray& copyFrom, 
									uint inclMinID, 
									uint exclMaxID);

		void	resampleWeights();						//!< passt die bereits vorhandenen Weights an die neue edgeLength an	


		// TWEAK VTX PUSH FUNKTIONEN
		void	doNoPush(MVector& direction, uint index){};			//!< Macht nix

		void	doSizeRelativePush(MVector& direction, uint index ){ direction += td.normals[index] * nd.push * wa[td.vtx[index]] ; };

		void	doStandardPush(MVector& direction, uint index ){ direction += td.normals[index].normal() * nd.push * wa[td.vtx[index]] * maxScale ; };


		// PARENT VTX PUSH FUNKTIONEN
		void	pDoSizeRelativePush(MVector& direction, uint index ) {direction += td.pNormals[index] * nd.push ;};

		void	pDoStandardPush(MVector& direction, uint index ) {direction += td.pNormals[index].normal() * nd.push * maxScale;};


		//---------------------------
		//	�FFENTLICHE VARIABLEN
	public:	// - werden von idleEvent aus ge�ndert
		//---------------------------

		bool	mayCreateIdleEvent;

		//---------------------------
		//	Gesch�tzte VARIABLEN
	protected:
		//---------------------------

		MDagPath		meshPath;
		MDagPath		meshTransform;

		//---------------------------
		//	PRIVATE VARIABLEN
	private:
		//---------------------------


		
		//* H�lt die Daten, welche die Vertizen betreffen
		struct	vertexData
		{
			MIntArray	vtxSet;		//!< Vertizen, die gew�hlt sind - von denen der Falloff ausgehen soll

			MIntArray	potVtx;		//!< Potentielle Vertex - die Vtx, die durch die SoftSelection transformiert werden d�rfen
			
			MPointArray	vtxOrigPos;	//!< originalPositionen der Vertizen im VtxSet - wird ben�tigt, um direction zu errechnen, PosCache
			
		
		}	vd;
		



		//* H�lt die TweakDaten
		struct	tweakData
		{
			MIntArray vtx;					//!< TweakVtx - Alle VertexIDs, die durch die softSelection transformiert werden sollen

			vector<MIntArray> pVtxPosID;	//!< Index in das vtxOrigPos Array, um die originalPosition zu erhalten
		
			vector<MDoubleArray> weights;	//!< Array mit den Weights, per Vtx, die die St�rke der Transformation bestimmen - sie sind bereits resampled

			vector<MDoubleArray> bWeights;	//!< BasicWeigths - lineare Wichtungen anhand der distanz zum ParentVtx

			MVectorArray normals;			//!< Enh�lt die normalWectoren, und deren l�nge kodiert die Gr��e des Faces - so wird speicher gespart zu moderaten CPU kosten

			MVectorArray pNormals;			//!< ParentNormals, aufbau/kodierung wie die normals

			void	clearAll() {	vtx.setLength(0); 
									pVtxPosID.clear(); 
									weights.clear();
									bWeights.clear();
									normals.clear();};	

		}	td;


		//* nodeData - h�lt alle attribute der Node
		struct	nodeData			
		{
			double	distance;		//!< radius des falloffs

			double	push;			//!< push/pullWert - entlang der normale des Faces, wahlweise relativ zur gr��e des Faces oder nicht

			bool	pushSizeRelative;	//!< Relativ zur Gr��e des Faces oder nicht?

			short	fType;			//!< Typ des Falloffs (smooth, linear ... )

			long	maxWeights;		//!< Maximale anzahl der ParentVtx pro tweakVtx, default == 1;

			float	smoothen;		//!< exponent, der alles glatter macht

			long	edgeDistance;	//!< wie oft soll gegrowed werden?

			bool	fVis;			//!< falloff visualisieren ?

			bool	useEdgeDist;	//!< edgeDistance oder radius verwenden ?

			bool	useMatrixRotation;	//!< Wenn an, dann wird die matrix verwendet, um die tweakVtx zu transformieren

			MObject	outMesh;		//!< das gegenw�rtige Mesh - outMesh
			MObject	inMesh;			//!< das gegenw�rtige Mesh - inMesh

			MMatrix matrix;			//!< Matrix, f�r rotation/translations/Skalierung - nur f�r softTransformationNode

		}	nd;


		/** Diese Struct h�lt die Daten, welche in der DrawMethode gerufen werden

		*/
		struct	drawData
		{
			MColor	vtxColor1;		//!< Erste VtxFarbe
			MColor	vtxColor2;		//!< Zweite VtxFarbe

			float	pointSize;		//!< Gr��e der VertexPoints

			meshStatus	mstat;		//!< Status des Meshes

			GLuint	list;			//!< Speicher f�r die display List

		} dd;


		//!< Diese struct h�lt einen cache, umd die edgeDistanceParameterChanges effizienter verarbeiten zu k�nnen
		struct	edgeCache
		{	
			
			// VARIABLEN		
			MIntArray	ringCounts;							//!< H�lt die Zahl an Vertizen pro ring - um einen Zugriff zu erm�glichen


		} ec;


		//Weitere Variablen:
		//
		BPT_Helpers		helper;
		MFnAnimCurve	animFn;
		bool			animFnInitialized;
	

		bool			isMatrixMode;		//!< Ist true, wenn eine TransformationMatrix vorhanden ist, und diese zur transformation verwendet werden soll


		// Jetzt kommen die backup variablen, um zwischen zwei Zyclen vergleichen zu k�nnen
		double	lastDistance;
		short	lastFType;
		long	lastMaxWeights;
		float	lastSmoothen;
		int		lastEdgeDistance;


		double	maxScale;
		

		MDoubleArray wa;		//!< WeightArray f�r die normalisierten weights, welches f�r die visualisierung verwendet wird

		// ---------------------------------------------------------------------------------------------------------------------------------------------------------
		// MObjects, weil pro plugin nur ein statisches MObject mit dem selben namen existieren darf pro node.
		// Da hier mehrere Nodes dieselbe STE Basis haben, sind die statischen MObjects nicht mehr einzigartig und m�ssen demnach von der 
		// abgeleiteten Klasse definiert werden
		// ---------------------------------------------------------------------------------------------------------------------------------------------------------

		MObject	outMesh;		//!< Dies ist ein handle zum outMeshAttr - dies ist leider n�tig, da das outMeshAttr nen einzigartigen Namen haben muss und deshalb
								//!< von der �bergeordneten Node definiert werden muss
		MObject inMesh;

		MObject inMatrix;

		MObject	vtxSetObj;

		MObject	distanceObj;

		MObject	ftObj;

		MObject	vfObj;

		MObject	vtxOrigPosObj;

		MObject maxWeightsObj;

		MObject	smoothenObj;

		MObject	edgeDistanceObj;

		MObject useMatrixRotationObj;

		MObject	pushObj;

		MObject	pushRelativeObj;

		MObject meshMatrixObj;

		MObject	pointSizeObj;

		MObject	vtxColorObj;

		MObject	vtxColorObj2;

		// HANDLE ZU THISMOBJECT
		MObject	thisNode;

	};
	
}


#endif 
