// The code is heavily based on Christopher Michaelis'  DXF to Shapefile Converter
// (http://www.wanderingidea.com/content/view/12/25/), released under GPL License
//
// This code is based on two other products:
// DXFLIB (http://www.ribbonsoft.com/dxflib.html)
//    This is a library for reading DXF files, also GPL.
// SHAPELIB (http://shapelib.maptools.org/)
//    Used for the Shapefile functionality.
//    It is Copyright (c) 1999, Frank Warmerdam, released under the following "MIT Style" license:
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
//IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <cmath>
#include <string>

#include "builder.h"

#include "qgslogger.h"

Builder::Builder( std::string theFname,
                  int theShapefileType,
                  double *theGrpXVals, double *theGrpYVals,
                  std::string *theGrpNames,
                  int theInsertCount,
                  bool theConvertText )
    : fname( theFname )
    , shapefileType( theShapefileType )
    , grpXVals( theGrpXVals )
    , grpYVals( theGrpYVals )
    , grpNames( theGrpNames )
    , insertCount( theInsertCount )
    , convertText( theConvertText )
    , fetchedprims( 0 )
    , fetchedtexts( 0 )
    , ignoringBlock( false )
    , current_polyline_pointcount( 0 )
    , currentBlockX( 0.0 )
    , currentBlockY( 0.0 )
{
}

Builder::~Builder()
{
  polyVertex.clear();
  shpObjects.clear();
  textObjects.clear();
}

int Builder::textObjectsSize()
{
  return textObjects.size();
}

std::string Builder::outputShp()
{
  return outputshp;
}

std::string Builder::outputTShp()
{
  return outputtshp;
}

void Builder::addBlock( const DL_BlockData& data )
{
  QgsDebugMsg( "start block." );

  if ( data.name.compare( "ADCADD_ZZ" ) == 0 )
  {
    QgsDebugMsg( QString( "Ignoring block %1" ).arg( data.name.c_str() ) );
    ignoringBlock = true;
  }
  else
  {
    for ( int i = 0; i < insertCount; i++ )
    {
      if ( grpNames[i] == data.name )
      {
        currentBlockX = grpXVals[i];
        currentBlockY = grpYVals[i];
        QgsDebugMsg( QString( "Found coord for block: (%1,%2)" ).arg( grpXVals[i] ).arg( grpYVals[i] ) );
      }
    }
  }
}

void Builder::endBlock()
{
  FinalizeAnyPolyline();

  currentBlockX = 0.0;
  currentBlockY = 0.0;
  ignoringBlock = false;

  QgsDebugMsg( "end block." );
}

void Builder::addLayer( const DL_LayerData& data )
{
  Q_UNUSED( data );
  QgsDebugMsg( QString( "Layer: %1" ).arg( data.name.c_str() ) );
}

void Builder::addPoint( const DL_PointData& data )
{
  if ( shapefileType != SHPT_POINT )
  {
    QgsDebugMsg( "ignoring point" );
    return;
  }

  QgsDebugMsg( QString( "point (%1,%2,%3)" ).arg( data.x ).arg( data.y ).arg( data.z ) );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping point in block." );
    return;
  }


  double x = data.x + currentBlockX;
  double y = data.y + currentBlockY;
  double z = data.z;

  SHPObject *psObject;
  psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, 1, &x, &y, &z, NULL );

  shpObjects.push_back( psObject );

  fetchedprims++;
}

void Builder::addLine( const DL_LineData& data )
{
  //data.x1, data.y1, data.z1
  //data.x2, data.y2, data.z2

  if ( shapefileType != SHPT_ARC )
  {
    QgsDebugMsg( "ignoring line" );
    return;
  }

  QgsDebugMsg( QString( "line %1,%2,%3 %4,%5,%6" )
               .arg( data.x1 ).arg( data.y1 ).arg( data.z1 )
               .arg( data.x2 ).arg( data.y2 ).arg( data.z2 ) );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping line in block." );
    return;
  }


  double xv[2], yv[2], zv[2];
  xv[0] = data.x1 + currentBlockX;
  yv[0] = data.y1 + currentBlockY;
  zv[0] = data.z1;

  xv[1] = data.x2 + currentBlockX;
  yv[1] = data.y2 + currentBlockY;
  zv[1] = data.z2;

  SHPObject *psObject;

  psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, 2, xv, yv, zv, NULL );

  shpObjects.push_back( psObject );

  fetchedprims++;
}


void Builder::addPolyline( const DL_PolylineData& data )
{
  if ( shapefileType != SHPT_ARC && shapefileType != SHPT_POLYGON )
  {
    QgsDebugMsg( "ignoring polyline" );
    return;
  }

  QgsDebugMsg( "reading polyline - expecting vertices" );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping polyline in block" );
    return;
  }

  // Add previously created polyline if not finalized yet
  if ( current_polyline_pointcount > 0 )
  {
    if ( current_polyline_willclose )
    {

      DL_VertexData myVertex;

      myVertex.x = closePolyX;
      myVertex.y = closePolyY;
      myVertex.z = closePolyZ;

      polyVertex.push_back( myVertex );

    }

    SHPObject *psShape;
    int dim = polyVertex.size();
    double *xv = new double[dim];
    double *yv = new double[dim];
    double *zv = new double[dim];

    for ( int i = 0; i < dim; i++ )
    {
      xv[i] = polyVertex[i].x;
      yv[i] = polyVertex[i].y;
      zv[i] = polyVertex[i].z;
    }

    psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL );

    delete [] xv;
    delete [] yv;
    delete [] zv;

    shpObjects.push_back( psShape );


    fetchedprims++;

    polyVertex.clear();

    QgsDebugMsg( QString( "polyline prepared: %1" ).arg( fetchedprims - 1 ) );
    current_polyline_pointcount = 0;
  }

  // Now that the currently-adding polyline (if any) is
  // finalized, parse out the flag options

  if ( data.flags == 1 || data.flags == 32 )
  {
    current_polyline_willclose = true;
    store_next_vertex_for_polyline_close = true;
  }
  else
  {
    current_polyline_willclose = false;
    store_next_vertex_for_polyline_close = false;
  }

  current_polyline_pointcount = 0;

}


void Builder::addVertex( const DL_VertexData& data )
{
  if ( shapefileType != SHPT_ARC && shapefileType != SHPT_POLYGON )
  {
    QgsDebugMsg( "ignoring vertex" );
    return;
  }

  QgsDebugMsg( QString( "vertex (%1,%2,%3)" ).arg( data.x ).arg( data.y ).arg( data.z ) );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping vertex in block" );
    return;
  }

  DL_VertexData myVertex;
  myVertex.x = data.x + currentBlockX;
  myVertex.y = data.y + currentBlockY;
  myVertex.z = data.z;

  polyVertex.push_back( myVertex );

  current_polyline_pointcount++;

  if ( store_next_vertex_for_polyline_close )
  {
    store_next_vertex_for_polyline_close = false;
    closePolyX = data.x + currentBlockX;
    closePolyY = data.y + currentBlockY;
    closePolyZ = data.z;
  }
}


void Builder::endSequence()
{
  QgsDebugMsg( "endSequence" );
}

void Builder::addArc( const DL_ArcData& data )
{
  if ( shapefileType != SHPT_ARC )
  {
    QgsDebugMsg( "ignoring arc" );
    return;
  }

  int fromAngle = ( int ) data.angle1 + 1;
  int toAngle = ( int ) data.angle2 + 1;

  QgsDebugMsg( QString( "arc (%1,%2,%3 r=%4 a1=%5 a2=%6)" )
               .arg( data.cx ).arg( data.cy ).arg( data.cz )
               .arg( data.radius )
               .arg( data.angle1 ).arg( data.angle2 ) );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping arc in block" );
    return;
  }

  register int i = 0;
  register long shpIndex = 0;

  // Approximate the arc

  double radianMeasure;

  std::vector <DL_PointData> arcPoints;
  DL_PointData myPoint;

  for ( i = fromAngle; ; i++, shpIndex++ )
  {
    if ( i > 360 )
      i = 0;

    if ( shpIndex > 1000 )
      break;

    radianMeasure = i * M_PI / 180.0;

    myPoint.x = data.radius * cos( radianMeasure ) + data.cx + currentBlockX;
    myPoint.y = data.radius * sin( radianMeasure ) + data.cy + currentBlockY;
    myPoint.z = data.cz;

    arcPoints.push_back( myPoint );

    if ( i == toAngle )
      break;
  }

  // Finalize

  SHPObject *psShape;
  int dim = arcPoints.size();
  double *xv = new double[dim];
  double *yv = new double[dim];
  double *zv = new double[dim];

  for ( int i = 0; i < dim; i++ )
  {
    xv[i] = arcPoints[i].x;
    yv[i] = arcPoints[i].y;
    zv[i] = arcPoints[i].z;

  }

  psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL );

  delete [] xv;
  delete [] yv;
  delete [] zv;

  shpObjects.push_back( psShape );

  fetchedprims++;

  arcPoints.clear();

}


void Builder::addCircle( const DL_CircleData& data )
{
  if ( shapefileType != SHPT_ARC && shapefileType != SHPT_POLYGON )
  {
    QgsDebugMsg( "ignoring circle" );
    return;
  }

  QgsDebugMsg( QString( "circle (%1,%2,%3 r=%4)" ).arg( data.cx ).arg( data.cy ).arg( data.cz ).arg( data.radius ) );

  if ( ignoringBlock )
  {
    QgsDebugMsg( "skipping circle in block" );
    return;
  }


  std::vector <DL_PointData> circlePoints;
  DL_PointData myPoint;

  // Approximate the circle with 360 line segments connecting points along that circle
  register long shpIndex = 0;
  for ( double i = 0.0; i <= 2*M_PI; i += M_PI / 180.0, shpIndex++ )
  {
    myPoint.x = data.radius * cos( i ) + data.cx + currentBlockX;
    myPoint.y = data.radius * sin( i ) + data.cy + currentBlockY;
    myPoint.z = data.cz;

    circlePoints.push_back( myPoint );
  }

  SHPObject *psShape;
  int dim = circlePoints.size();
  double *xv = new double[dim];
  double *yv = new double[dim];
  double *zv = new double[dim];

  for ( int i = 0; i < dim; i++ )
  {
    xv[i] = circlePoints[i].x;
    yv[i] = circlePoints[i].y;
    zv[i] = circlePoints[i].z;
  }

  psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL );

  delete [] xv;
  delete [] yv;
  delete [] zv;

  shpObjects.push_back( psShape );

  fetchedprims++;

  circlePoints.clear();
}

void Builder::addText( const DL_TextData &data )
{
  if ( convertText )
  {
    DL_TextData myText(
      data.ipx + currentBlockX, data.ipy + currentBlockY, data.ipz,
      data.apx, data.apy, data.apz,
      data.height, data.xScaleFactor, data.textGenerationFlags,
      data.hJustification, data.vJustification,
      data.text, data.style, data.angle
    );

    textObjects.push_back( myText );

    QgsDebugMsg( QString( "text: %1" ).arg( data.text.c_str() ) );
    fetchedtexts++;
  }
}

void Builder::FinalizeAnyPolyline()
{
  // Save the last polyline / polygon if one exists.
  if ( current_polyline_pointcount > 0 )
  {
    if ( current_polyline_willclose )
    {
      DL_VertexData myVertex;
      myVertex.x = closePolyX;
      myVertex.y = closePolyY;
      myVertex.z = closePolyZ;

      polyVertex.push_back( myVertex );
    }

    SHPObject *psObject;
    int dim = polyVertex.size();
    double *xv = new double[dim];
    double *yv = new double[dim];
    double *zv = new double[dim];

    for ( int i = 0; i < dim; i++ )
    {
      xv[i] = polyVertex[i].x;
      yv[i] = polyVertex[i].y;
      zv[i] = polyVertex[i].z;
    }

    psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL );

    delete [] xv;
    delete [] yv;
    delete [] zv;

    shpObjects.push_back( psObject );

    polyVertex.clear();

    fetchedprims++;

    QgsDebugMsg( QString( "Finalized adding of polyline shape %1" ).arg( fetchedprims - 1 ) );
    current_polyline_pointcount = 0;
  }
}

void Builder::print_shpObjects()
{
  int dim = shpObjects.size();
  int dimTexts = textObjects.size();

  QgsDebugMsg( QString( "Number of primitives: %1" ).arg( dim ) );
  QgsDebugMsg( QString( "Number of text fields: %1" ).arg( dimTexts ) );

  SHPHandle hSHP;

  if ( fname.substr( fname.length() - 4 ).compare( ".shp" ) == 0 )
  {
    outputdbf = fname;
    outputdbf = outputdbf.replace(( outputdbf.length() - 3 ), outputdbf.length(), "dbf" );
    outputshp = fname;
    outputshp = outputshp.replace(( outputshp.length() - 3 ), outputshp.length(), "shp" );
    outputtdbf = fname;
    outputtdbf = outputtdbf.replace(( outputtdbf.length() - 4 ), outputtdbf.length(), "_texts.dbf" );
    outputtshp = fname;
    outputtshp = outputtshp.replace(( outputtshp.length() - 4 ), outputtshp.length(), "_texts.shp" );
  }
  else
  {
    outputdbf = outputtdbf = fname + ".dbf";
    outputshp = outputtshp = fname + ".shp";
  }

  DBFHandle dbffile = DBFCreate( outputdbf.c_str() );
  DBFAddField( dbffile, "myid", FTInteger, 10, 0 );

  hSHP = SHPCreate( outputshp.c_str(), shapefileType );

  QgsDebugMsg( "Writing to main shp file..." );

  for ( int i = 0; i < dim; i++ )
  {
    SHPWriteObject( hSHP, -1, shpObjects[i] );
    SHPDestroyObject( shpObjects[i] );
    DBFWriteIntegerAttribute( dbffile, i, 0, i );
  }

  SHPClose( hSHP );
  DBFClose( dbffile );

  QgsDebugMsg( "Done!" );

  if ( convertText && dimTexts > 0 )
  {
    SHPHandle thSHP;

    DBFHandle Tdbffile = DBFCreate( outputtdbf.c_str() );
    thSHP = SHPCreate( outputtshp.c_str(), SHPT_POINT );

    DBFAddField( Tdbffile, "tipx", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tipy", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tipz", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapx", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapy", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapz", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "height", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "scale", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "flags", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "hjust", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "vjust", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "text", FTString, 50, 0 );
    DBFAddField( Tdbffile, "style", FTString, 50, 0 );
    DBFAddField( Tdbffile, "angle", FTDouble, 20, 10 );

    QgsDebugMsg( "Writing Texts' shp File..." );

    for ( int i = 0; i < dimTexts; i++ )
    {
      SHPObject *psObject;
      double x = textObjects[i].ipx;
      double y = textObjects[i].ipy;
      double z = textObjects[i].ipz;
      psObject = SHPCreateObject( SHPT_POINT, i, 0, NULL, NULL, 1, &x, &y, &z, NULL );

      SHPWriteObject( thSHP, -1, psObject );

      DBFWriteDoubleAttribute( Tdbffile, i, 0, textObjects[i].ipx );
      DBFWriteDoubleAttribute( Tdbffile, i, 1, textObjects[i].ipy );
      DBFWriteDoubleAttribute( Tdbffile, i, 2, textObjects[i].ipz );

      DBFWriteDoubleAttribute( Tdbffile, i, 3, textObjects[i].apx );
      DBFWriteDoubleAttribute( Tdbffile, i, 4, textObjects[i].apy );
      DBFWriteDoubleAttribute( Tdbffile, i, 5, textObjects[i].apz );

      DBFWriteDoubleAttribute( Tdbffile, i, 6, textObjects[i].height );
      DBFWriteDoubleAttribute( Tdbffile, i, 7, textObjects[i].xScaleFactor );
      DBFWriteIntegerAttribute( Tdbffile, i, 8, textObjects[i].textGenerationFlags );

      DBFWriteIntegerAttribute( Tdbffile, i, 9, textObjects[i].hJustification );
      DBFWriteIntegerAttribute( Tdbffile, i, 10, textObjects[i].vJustification );

      DBFWriteStringAttribute( Tdbffile, i, 11, textObjects[i].text.c_str() );
      DBFWriteStringAttribute( Tdbffile, i, 12, textObjects[i].style.c_str() );

      DBFWriteDoubleAttribute( Tdbffile, i, 13, textObjects[i].angle );

      SHPDestroyObject( psObject );
    }
    SHPClose( thSHP );
    DBFClose( Tdbffile );

    QgsDebugMsg( "Done!" );
  }
}
