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

// Copyright (C) 1997-2003 Alias|Wavefront, a division of Silicon Graphics Limited.
// 
// The information in this file is provided for the exclusive use of the
// licensees of Alias|Wavefront.  Such users have the right to use, modify,
// and incorporate this code into other products for purposes authorized
// by the Alias|Wavefront license agreement, without fee.
// 
// ALIAS|WAVEFRONT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
// EVENT SHALL ALIAS|WAVEFRONT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
// CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

// $RCSfile: cvColorNode.cpp $     $Revision$

#include "visualizeMeshNode.h"

#include "mayabaselib/ogl_headers.h"

MTypeId visualizeMeshNode::id( 0x00108BC2 );
MObject visualizeMeshNode::inputMesh;
MObject visualizeMeshNode::vtxLocations;
MObject visualizeMeshNode::pointSize;
MObject visualizeMeshNode::drawingEnabled;
MObject	visualizeMeshNode::vtxColorObj;
MObject	visualizeMeshNode::vtxColorObj2;
MObject	visualizeMeshNode::vtxWeights;
//MObject	visualizeMeshNode::matrixAttr;
//MObject	visualizeMeshNode::showOriginalObj;

// PolyOffsetParams
#ifdef DEBUG
	MObject			visualizeMeshNode::pOffset1Obj;
	MObject			visualizeMeshNode::pOffset2Obj;
#endif


visualizeMeshNode::visualizeMeshNode()
	: list(450000)
	, listNeedsUpdate(true)
	, wasInCompute(false)
	{}
visualizeMeshNode::~visualizeMeshNode() {}

//-----------------------------------------------------------------------
MStatus visualizeMeshNode::compute( const MPlug& plug, MDataBlock& data )
//-----------------------------------------------------------------------
{ 
	MStatus stat;

	// cout << "cvColor::compute\n";

	if ( plug == vtxLocations ) {


		MObject	weightObj = data.inputValue(vtxWeights).data();
		
		MFnDoubleArrayData weightData(weightObj);
		
		vtxWeightArray = weightData.array();
		
		//die displayList muss jetzt auf jeden Fall neu erstellt werden
		listNeedsUpdate = wasInCompute = true;

		stat = data.setClean ( plug );
		if (!stat) {
			stat.perror("visualizeMeshNoder::compute setClean");
			return stat;
		}
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

//-----------------------------------------------------------------------
void visualizeMeshNode::draw( M3dView & view, const MDagPath & path, 
							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus status )
//-----------------------------------------------------------------------
{ 
	// cout << "cvColor::draw\n";

	MStatus		stat;
	MObject		thisNode = thisMObject();

	MPlug		plug(thisNode, drawingEnabled) ;
	
	//soll ueberhaupt etwas gezeichnet werden?
	bool doDraw;
	plug.getValue(doDraw);


	if (!doDraw)
		return;

	//Okay, also die restlichen Daten extrahieren	

	//Lokale Variablen
	float	fPointSize;


	//PointSize holen
	
	plug.setAttribute(pointSize);	
	plug.getValue(fPointSize);
	

	bool	allowBeautyMode = true;

	// Jetzt das Plug auslesen um herauszufinden, ob man schn zeichnen darf oder nicht
	// TODO

	
/*
	// Test
	HDC dc;
	dc = view.deviceContext();
	
	PIXELFORMATDESCRIPTOR pfd;
	int  iPixelFormat;
	
	// get the current pixel format index 
	iPixelFormat = GetPixelFormat(dc); 
	
	// obtain a detailed description of that pixel format 
	int returnVal = DescribePixelFormat(dc, iPixelFormat, 
        sizeof(PIXELFORMATDESCRIPTOR), &pfd); 

	//cout<<pfd.nSize<<endl;
	//cout<<sizeof(PIXELFORMATDESCRIPTOR)<<endl;
	
	cout<<pfd.iPixelType<<" = ReturnVal"<<endl;
	cout<<returnVal<<" = ReturnVal"<<endl;
	cout<<iPixelFormat<<" = PixelFormat"<<endl;
*/



	//Farbe holen	-> Diese Schreibweise hat den (hier unbedeutenden) Vorteil, dass Objekte automatisch zerstrt werden - sie existieren nur innerhalb der Klammern
	{
		Float3 tmpColor;
		

		plug.setAttribute(vtxColorObj);
		MObject colorObj;
		plug.getValue(colorObj);
		
		
		MFnNumericData	numDataFn(colorObj);
		numDataFn.getData(tmpColor.x, tmpColor.y, tmpColor.z);

		//wenn sich die Farben verndert haben, dann die liste updaten - und alles neuzeichnen
		if(tmpColor != vtxColor)
		{
			vtxColor = tmpColor;

			listNeedsUpdate = true;
		}

		plug.setAttribute(vtxColorObj2);
		plug.getValue(colorObj);
		numDataFn.setObject(colorObj);

		numDataFn.getData(tmpColor.x, tmpColor.y, tmpColor.z);

		if(tmpColor != vtxColor2)
		{
			vtxColor2 = tmpColor;
			
			listNeedsUpdate = true;
		}

	}


	//jetzt noch die Node ermuntern, die comuteMethode aufzurufen
	plug.setAttribute(vtxLocations);
	bool dummy;
	plug.getValue(dummy);


	//vtxWeights muessen gesetzt sein
	if(vtxWeightArray.length() == 0)
		return;



	//MeshData holen
	plug.setAttribute(inputMesh);
	
	MObject meshData;
	plug.getValue(meshData);


	visualizeMeshNode::meshStatus mStat = getMeshStatus();

	// Wenn sich der Anezigstatus des Meshes ndert, dann muss die DisplayList neu erstellt werden
	// Nur um den PolygonOffset zu aktualisieren
	if(mStat != lastStat)
	{
		lastStat = mStat;
	}

	//if( mStat == kNone)	// Nichts zeichnen, wenn nix selected
	//	return;

	MHardwareRenderer* renderer = MHardwareRenderer::theRenderer();
	if (!renderer) {
		return;
	}
	
	MGLFunctionTable* glf = renderer->glFunctionTable();
	if (!glf) {
		return;
	}
	


	view.beginGL(); 
//	view.beginOverlayDrawing(); 
	
	
	//Im PointMode werden PointsGezeigt, und wenn das Objekt selected ist (dann funzt shaded nicht mehr)
	if( style == M3dView::kPoints || (mStat == kSelected ) || wasInCompute || !allowBeautyMode)
	{

		//VertiIter initialisieren
		MItMeshVertex vertIter(meshData);
		
		drawPoints(vertIter, fPointSize, glf);
		
		// Das updaten der DisplayLists ist zu langsam, weshalb automatisch vtxMode genommen wird
		wasInCompute = false;

	}
	else
	{//in diesem Modus werden Faces gezeichnet, mit entsprechenden alphawerten als zeichen ihrer Farbe
		
		MItMeshPolygon polyIter(meshData);
		MItMeshVertex	vertIter(meshData);
		


		//Die displayList pruefen
		if( (listNeedsUpdate & !wasInCompute) || lastDMode != style )
		{//neue Liste erzeugen - wird eigentlich nur einmal pro drawAktion gemacht
			
			lastDMode = style;

			if( list != 450000 ) 
				//alte liste lschen
				glf->glDeleteLists(list, 1);
			
			list = glf->glGenLists(1);

			if(glf->glIsList(list))
			{
				listNeedsUpdate = wasInCompute = false;


				//#######################
				//NEUE LISTE AUFZEICHNEN
				//#######################
				glf->glNewList(list, GL_COMPILE_AND_EXECUTE);

		
				//drawShaded(polyIter, vertIter, style, mStat);
				drawShadedTriangles(polyIter, vertIter, style, mStat, glf);


				//#######################
				//ENDE LISTE AUFZEICHNEN
				//#######################

				glf->glEndList();

			}
			else
			{
				//fehler, also alles ohne displayList zeichnen
				//drawShaded(polyIter, vertIter, style, mStat);
				drawShadedTriangles(polyIter, vertIter, style, mStat, glf);
			}

		}
		else 
		{
			glf->glCallList(list);
			wasInCompute = false;
		}

	}



	view.endGL();
//	view.endOverlayDrawing();
}


//-----------------------------------------------------------------------------------
void	visualizeMeshNode::drawPoints( MItMeshVertex& vertIter, float fPointSize, MGLFunctionTable* glf)
//-----------------------------------------------------------------------------------
{

// Push the color settings
			// 
			glf->glPushAttrib( GL_CURRENT_BIT | GL_POINT_BIT );
			
			glf->glEnable ( GL_POINT_SMOOTH );  // Draw square "points"
			glf->glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glf->glEnable(GL_BLEND);
			glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


			/*
			GLint	renderMode;
			glGetIntegerv( GL_RENDER_MODE, &renderMode );
			
			// cout<<renderMode<<endl ;
			
			//	cout<<view.selectMode()<<endl;
			
			
			
			if(renderMode == GL_SELECT)
				cout<<"Bin in SelectMode"<<endl;
			else if(renderMode == GL_RENDER)
				cout<<"StandardRenderMode"<<endl;
			*/

			Float3 tmpColor;
			double dTmp;		// hlt das weight, cache

			//weights verwenden um Farbe zu skalieren
			uint numCVs = vertIter.count();
			for (uint i = 0; i < numCVs; i++, vertIter.next()) 
			{

				dTmp = vtxWeightArray[i];

				glf->glPointSize( static_cast<MGLfloat>((fPointSize * dTmp) + 3.0));

				glf->glBegin( GL_POINTS );	

			//	glPointSize( fPointSize );
				MPoint		point( vertIter.position() );
				
				
			
				
	

				tmpColor = getCalColor(vtxColor, vtxColor2, dTmp );
				glf->glColor4f(tmpColor.x,tmpColor.y,tmpColor.z, static_cast<MGLfloat>(dTmp));
				glf->glVertex3f( (float)point.x, (float)point.y, (float)point.z);

				glf->glEnd();
			} 
			
			
			
			glf->glPopAttrib();


}


//-------------------------------------------------------------------------------------------------------------------------------------------
void	visualizeMeshNode::drawShadedTriangles(MItMeshPolygon& polyIter, MItMeshVertex& vertIter, M3dView::DisplayStyle style, meshStatus meshStat, MGLFunctionTable* glf)
//-------------------------------------------------------------------------------------------------------------------------------------------
{

				//alles zeichnen
				glf->glPushAttrib(GL_ALL_ATTRIB_BITS);
				
				
				glf->glEnable(GL_POLYGON_OFFSET_FILL);
				
				
				glf->glEnable(GL_BLEND);
				glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//glDepthMask(GL_FALSE);
				
				glf->glPolygonMode(GL_BACK, GL_FILL);
				glf->glShadeModel(GL_SMOOTH);
				
				
				
				//Dass muss aussserhalb der displayList bleiben, weil dieser Wert nicht precompiliert werden darf
				float param1 = 0.45f, param2 = 0.55f;

				// im DebugMode werden die Params anhand der NodeParameter gesetzt

				if(style == M3dView::kWireFrame)
				{

						param1 = 0.45f; param2 = 0.55f;

				}
				else
				{
					switch(meshStat)
					{
					case kNone:
						{
							param1 = -0.5f; param2 = -0.6f;

							break;
						}
					case kHilited:
						{
							param1 = 0.45f; param2 = 0.55f;
							
							break;
						}
					default: break;
					}
				}


				#ifdef DEBUG
				MPlug tmpPlug(thisMObject(), pOffset1Obj);
				tmpPlug.getValue(param1);
				
				tmpPlug.setAttribute(pOffset2Obj);
				tmpPlug.getValue(param2);
				#endif

				glf->glPolygonOffset( param1, param2 );
				
				//jedes Poly zeichnen
				
				uint numPolys = polyIter.count();
				uint i, x , l;
				MPoint	point;
				Float3 tmpCol;
				MPointArray triPoints;
				MIntArray	triVtx;
				//glColor4f(0.0f, 0.0f, 1.0f, 0.2);
				
				for(i = 0; i < numPolys; i++, polyIter.next())
				{
					

					polyIter.getTriangles(triPoints, triVtx);
					
					
					l = triVtx.length();
					
					
					glf->glBegin(GL_TRIANGLES);
					
					for(x = 0; x < l ; x+=3)
					{
						tmpCol = getCalColor(vtxColor, vtxColor2 ,  vtxWeightArray[ triVtx[x] ]);
						glf->glColor4f(tmpCol.x, tmpCol.y,tmpCol.z, static_cast<MGLfloat>(vtxWeightArray[ triVtx[x] ]));
						glf->glVertex3d(triPoints[x].x, triPoints[x].y, triPoints[x].z);
						
			
						tmpCol = getCalColor(vtxColor, vtxColor2 ,  vtxWeightArray[ triVtx[x+1] ]);
						glf->glColor4f(tmpCol.x, tmpCol.y,tmpCol.z, static_cast<MGLfloat>(vtxWeightArray[ triVtx[x+1] ]));
						glf->glVertex3d(triPoints[x+1].x, triPoints[x+1].y, triPoints[x+1].z);

						tmpCol = getCalColor(vtxColor, vtxColor2 ,  vtxWeightArray[ triVtx[x+2] ]);
						glf->glColor4f(tmpCol.x, tmpCol.y,tmpCol.z, static_cast<MGLfloat>(vtxWeightArray[ triVtx[x+2] ]));
						glf->glVertex3d(triPoints[x+2].x, triPoints[x+2].y, triPoints[x+2].z);

					}
					
					glf->glEnd();
					
				}
				
				
			//	glDisable(GL_POLYGON_OFFSET_FILL);

				glf->glPopAttrib();


}

//-----------------------------------------------------------------------------------
visualizeMeshNode::meshStatus	visualizeMeshNode::getMeshStatus()
//-----------------------------------------------------------------------------------
{


		MDagPath visNodePath, transform;
		
		MDagPath::getAPathTo( thisMObject(), visNodePath);

		transform = visNodePath;
		transform.pop();	//transform

		
		MSelectionList activeList;
		//momentan checken wir nur, ob das Mesh selected ist oder nicht
		
		MGlobal::getActiveSelectionList(activeList);


		if( ( activeList.hasItem(transform.node()) || activeList.hasItem( transform ) ) )
			return kSelected;
		


		MGlobal::getHiliteList(activeList);

		if( ( activeList.hasItem(transform.node()) || activeList.hasItem( transform ) ) )
			return kHilited;



 		return kNone;

	//	Nur zum testen
	//	return kSelected;

}


//-----------------------------------------------------------------------------------
Float3 visualizeMeshNode::getCalColor(const Float3& color1, const Float3& color2, double weight)
//-----------------------------------------------------------------------------------
{

	return color1 * (float)weight + color2 * (float)(1.0 -  weight) ;

}


//-----------------------------------------------------------------------
bool visualizeMeshNode::isBounded() const
//-----------------------------------------------------------------------
{ 
	return false;
}


//-----------------------------------------------------------------------
MBoundingBox	visualizeMeshNode::boundingBox() const
//-----------------------------------------------------------------------
{


	return MBoundingBox(MPoint(-1.0,-1.0,-1.0), MPoint(1.0,1.0,1.0));


}


//-----------------------------------------------------------------------
void* visualizeMeshNode::creator()
//-----------------------------------------------------------------------
{
	return new visualizeMeshNode();
}

//-----------------------------------------------------------------------
MStatus visualizeMeshNode::initialize()
//-----------------------------------------------------------------------
{ 
	MStatus				stat;
	MFnNumericAttribute	numericAttr;
	MFnTypedAttribute	typedAttr;



//!!!####################################################
	//COLOR
//!!!####################################################

	vtxColorObj = numericAttr.createColor("aColor","col");
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setDefault(1.0,0.0,0.1);


//!!!####################################################
	//COLOR2
//!!!####################################################

	vtxColorObj2 = numericAttr.createColor("aColor2","col2");
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	numericAttr.setDefault(1.0,0.785,0.0);


//!!!####################################################
	//DRAWING ENABLED
//!!!####################################################

	drawingEnabled = numericAttr.create( "drawingEnabled", "en",
									MFnNumericData::kBoolean, 1, &stat );
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	if (!stat) {
		stat.perror("create drawingEnabled attribute");
		return stat;
	}


	
	/* // Dieses Feature ist eigentlich ziemlich unntig
//!!!####################################################
	//SHOW ORIGINAL MESH
//!!!####################################################

	showOriginalObj = numericAttr.create( "showOriginal", "so",
									MFnNumericData::kBoolean, 1, &stat );
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	if (!stat) {
		stat.perror("create showOriginal attribute");
		return stat;
	}
	*/

//!!!####################################################
	//POINTSIZE
//!!!####################################################

	pointSize = numericAttr.create( "pointSize", "ps",
									MFnNumericData::kFloat, 10.0, &stat );
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	if (!stat) {
		stat.perror("create pointSize attribute");
		return stat;
	}
	

//!!!####################################################
	//INPUT MESH
//!!!####################################################

	inputMesh = typedAttr.create( "inputMesh", "is",
							 MFnMeshData::kMesh, &stat);
	if (!stat) {
		stat.perror("create inputMesh attribute");
		return stat;
	}


//!!!####################################################
	//VTXLOCATIONS (DUMMY OUT)
//!!!####################################################
	
	vtxLocations = numericAttr.create( "vtxLocations", "cv",
							 MFnNumericData::kBoolean,1, &stat);
	if (!stat) {
		stat.perror("create vtxLocations attribute");
		return stat;
	}



//!!!####################################################
	//VTXWEIGHTS
//!!!####################################################

	vtxWeights = typedAttr.create( "vtxWeights", "vw",
							 MFnData::kDoubleArray, &stat);
	if (!stat) {
		stat.perror("create vtxLocations attribute");
		return stat;
	}

	MFnDoubleArrayData	defaultWeights;
	typedAttr.setDefault(defaultWeights.create());
	
/*
//!!!####################################################
	//IN MATRIX (TRANSFORMATIONMATRIX FUER DAS VTSSET)
//!!!####################################################
	//matrix

	matrixAttr = typedAttr.create( "inMatrix","imat", MFnData::kMatrix);

	MFnMatrixData	matrixData;
	typedAttr.setDefault(matrixData.create());
*/

#ifdef DEBUG
//!!!####################################################
	//DEBUG PARAMETER: FUER POLYOFFSET
//!!!####################################################
	//matrix

	pOffset1Obj =	numericAttr.create( "offsetValue1", "ov1",
										MFnNumericData::kFloat, 0.45, &stat );
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	

	pOffset2Obj =	numericAttr.create( "offsetValue2", "ov2",
										MFnNumericData::kFloat, 0.55, &stat );
	numericAttr.setKeyable(true);
	numericAttr.setStorable(true);
	

#endif




	//Attribute hinzufuegen
	//------------------------

	stat = addAttribute (drawingEnabled);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}

	stat = addAttribute (pointSize);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}

	stat = addAttribute (inputMesh);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}

	stat = addAttribute (vtxLocations);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}

	stat = addAttribute (vtxColorObj);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}

	stat = addAttribute (vtxColorObj2);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}
	

	stat = addAttribute (vtxWeights);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}
/*
	stat = addAttribute (matrixAttr);
		if (!stat) { stat.perror("addAttribute"); 
		return stat;}
*/

#ifdef DEBUG
		
		stat = addAttribute( pOffset1Obj );
		if (!stat) { stat.perror("addAttribute"); 
			return stat;}

		stat = addAttribute( pOffset2Obj );
		if (!stat) { stat.perror("addAttribute"); 
			return stat;}

#endif





	//Affections setzen
	//------------------------

	stat = attributeAffects(vtxWeights, vtxLocations );
		if (!stat) { stat.perror("attributeAffects"); 
		return stat;}


	stat = attributeAffects(inputMesh, vtxLocations );
		if (!stat) { stat.perror("attributeAffects"); 
		return stat;}
	

	// Die Parameter muessen die computeMethod aufrufen
#ifdef DEBUG
	
		stat = attributeAffects(pOffset1Obj, vtxLocations );
		if (!stat) { stat.perror("attributeAffects"); 
		return stat;}

		stat = attributeAffects(pOffset2Obj, vtxLocations );
		if (!stat) { stat.perror("attributeAffects"); 
		return stat;}

#endif

	return MS::kSuccess;
}

