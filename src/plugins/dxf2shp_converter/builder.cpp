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

#include "builder.h"

#include <cmath>
#include <QVector>

#include "qgslogger.h"

Builder::Builder( QString theFname,
                  int theShapefileType,
                  bool theConvertText,
                  bool theConvertInserts )
    : fname( theFname )
    , shapefileType( theShapefileType )
    , convertText( theConvertText )
    , convertInserts( theConvertInserts )
    , ignoringBlock( false )
    , current_polyline_willclose( false )
    , store_next_vertex_for_polyline_close( false )
    , current_polyline_pointcount( 0 )
    , closePolyX( 0.0 )
    , closePolyY( 0.0 )
    , closePolyZ( 0.0 )
{
}

Builder::~Builder()
{
}

void Builder::addBlock( const DL_BlockData& data )
{
  Q_UNUSED( data );
  ignoringBlock = true;
}

void Builder::endBlock()
{
  ignoringBlock = false;
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

  double x = data.x, y = data.y, z = data.z;
  shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, 1, &x, &y, &z, NULL );
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


  double xv[2] = { data.x1, data.x2 };
  double yv[2] = { data.y1, data.y2 };
  double zv[2] = { data.z1, data.z2 };

  shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, 2, xv, yv, zv, NULL );
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

    int dim = polyVertex.size();
    QVector<double> xv( dim );
    QVector<double> yv( dim );
    QVector<double> zv( dim );

    for ( int i = 0; i < dim; i++ )
    {
      xv[i] = polyVertex[i].x;
      yv[i] = polyVertex[i].y;
      zv[i] = polyVertex[i].z;
    }

    shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, dim, xv.data(), yv.data(), zv.data(), NULL );

    polyVertex.clear();

    QgsDebugMsg( QString( "polyline prepared: %1" ).arg( shpObjects.size() - 1 ) );
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

  polyVertex << DL_VertexData( data.x, data.y, data.z );

  current_polyline_pointcount++;

  if ( store_next_vertex_for_polyline_close )
  {
    store_next_vertex_for_polyline_close = false;
    closePolyX = data.x;
    closePolyY = data.y;
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

  int i = 0;
  long shpIndex = 0;

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

    myPoint.x = data.radius * cos( radianMeasure ) + data.cx;
    myPoint.y = data.radius * sin( radianMeasure ) + data.cy;
    myPoint.z = data.cz;

    arcPoints.push_back( myPoint );

    if ( i == toAngle )
      break;
  }

  // Finalize

  int dim = arcPoints.size();
  QVector<double> xv( dim );
  QVector<double> yv( dim );
  QVector<double> zv( dim );

  for ( int i = 0; i < dim; i++ )
  {
    xv[i] = arcPoints[i].x;
    yv[i] = arcPoints[i].y;
    zv[i] = arcPoints[i].z;

  }

  shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, dim, xv.data(), yv.data(), zv.data(), NULL );
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
  long shpIndex = 0;
  for ( double i = 0.0; i <= 2*M_PI; i += M_PI / 180.0, shpIndex++ )
  {
    myPoint.x = data.radius * cos( i ) + data.cx;
    myPoint.y = data.radius * sin( i ) + data.cy;
    myPoint.z = data.cz;

    circlePoints.push_back( myPoint );
  }

  int dim = circlePoints.size();
  QVector<double> xv( dim );
  QVector<double> yv( dim );
  QVector<double> zv( dim );

  for ( int i = 0; i < dim; i++ )
  {
    xv[i] = circlePoints[i].x;
    yv[i] = circlePoints[i].y;
    zv[i] = circlePoints[i].z;
  }

  shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, dim, xv.data(), yv.data(), zv.data(), NULL );

  circlePoints.clear();
}

void Builder::addInsert( const DL_InsertData& data )
{
  if ( !convertInserts )
    return;

  insertObjects << DL_InsertData(
    data.name,
    data.ipx, data.ipy, data.ipz,
    data.sx, data.sy, data.sz,
    data.angle,
    data.cols, data.rows,
    data.colSp, data.rowSp
  );
}

void Builder::addText( const DL_TextData &data )
{
  if ( !convertText )
    return;

  textObjects << DL_TextData(
    data.ipx, data.ipy, data.ipz,
    data.apx, data.apy, data.apz,
    data.height, data.xScaleFactor, data.textGenerationFlags,
    data.hJustification, data.vJustification,
    data.text, data.style, data.angle
  );

  QgsDebugMsg( QString( "text: %1" ).arg( data.text.c_str() ) );
}

void Builder::FinalizeAnyPolyline()
{
  // Save the last polyline / polygon if one exists.
  if ( current_polyline_pointcount > 0 )
  {
    if ( current_polyline_willclose )
    {
      polyVertex << DL_VertexData( closePolyX, closePolyY, closePolyZ );
    }

    int dim = polyVertex.size();
    QVector<double> xv( dim );
    QVector<double> yv( dim );
    QVector<double> zv( dim );

    for ( int i = 0; i < dim; i++ )
    {
      xv[i] = polyVertex[i].x;
      yv[i] = polyVertex[i].y;
      zv[i] = polyVertex[i].z;
    }

    shpObjects << SHPCreateObject( shapefileType, shpObjects.size(), 0, NULL, NULL, dim, xv.data(), yv.data(), zv.data(), NULL );
    polyVertex.clear();

    QgsDebugMsg( QString( "Finalized adding of polyline shape %1" ).arg( shpObjects.size() - 1 ) );
    current_polyline_pointcount = 0;
  }
}

void Builder::print_shpObjects()
{
  QgsDebugMsg( QString( "Number of primitives: %1" ).arg( shpObjects.size() ) );
  QgsDebugMsg( QString( "Number of text fields: %1" ).arg( textObjects.size() ) );
  QgsDebugMsg( QString( "Number of inserts fields: %1" ).arg( insertObjects.size() ) );

  SHPHandle hSHP;

  if ( fname.endsWith( ".shp", Qt::CaseInsensitive ) )
  {
    QString fn( fname.mid( fname.length() - 4 ) );

    outputdbf = fn + ".dbf";
    outputshp = fn + ".shp";
    outputtdbf = fn + "_texts.dbf";
    outputtshp = fn + "_texts.shp";
    outputidbf = fn + "_inserts.dbf";
    outputishp = fn + "_inserts.shp";
  }
  else
  {
    outputdbf = outputtdbf = outputidbf = fname + ".dbf";
    outputshp = outputtshp = outputishp = fname + ".shp";
  }

  DBFHandle dbffile = DBFCreate( outputdbf.toUtf8() );
  DBFAddField( dbffile, "myid", FTInteger, 10, 0 );

  hSHP = SHPCreate( outputshp.toUtf8(), shapefileType );

  QgsDebugMsg( "Writing to main shp file..." );

  for ( int i = 0; i < shpObjects.size(); i++ )
  {
    SHPWriteObject( hSHP, -1, shpObjects[i] );
    SHPDestroyObject( shpObjects[i] );
    DBFWriteIntegerAttribute( dbffile, i, 0, i );
  }

  SHPClose( hSHP );
  DBFClose( dbffile );

  QgsDebugMsg( "Done!" );

  if ( textObjects.size() > 0 )
  {
    SHPHandle thSHP;

    DBFHandle Tdbffile = DBFCreate( outputtdbf.toUtf8() );
    thSHP = SHPCreate( outputtshp.toUtf8(), SHPT_POINT );

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

    for ( int i = 0; i < textObjects.size(); i++ )
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

  if ( insertObjects.size() > 0 )
  {
    SHPHandle ihSHP;

    DBFHandle Idbffile = DBFCreate( outputidbf.toUtf8() );
    ihSHP = SHPCreate( outputishp.toUtf8(), SHPT_POINT );

    DBFAddField( Idbffile, "name", FTString, 200, 0 );
    DBFAddField( Idbffile, "ipx", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "ipy", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "ipz", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "sx", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "sy", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "sz", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "angle", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "cols", FTInteger, 20, 0 );
    DBFAddField( Idbffile, "rows", FTInteger, 20, 0 );
    DBFAddField( Idbffile, "colsp", FTDouble, 20, 10 );
    DBFAddField( Idbffile, "rowsp", FTDouble, 20, 10 );

    QgsDebugMsg( "Writing Insert' shp File..." );

    for ( int i = 0; i < insertObjects.size(); i++ )
    {
      SHPObject *psObject;
      double &x = insertObjects[i].ipx;
      double &y = insertObjects[i].ipy;
      double &z = insertObjects[i].ipz;
      psObject = SHPCreateObject( SHPT_POINT, i, 0, NULL, NULL, 1, &x, &y, &z, NULL );

      SHPWriteObject( ihSHP, -1, psObject );

      int c = 0;
      DBFWriteStringAttribute( Idbffile, i, c++, insertObjects[i].name.c_str() );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].ipx );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].ipy );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].ipz );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].sx );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].sy );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].sz );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].angle );
      DBFWriteIntegerAttribute( Idbffile, i, c++, insertObjects[i].cols );
      DBFWriteIntegerAttribute( Idbffile, i, c++, insertObjects[i].rows );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].colSp );
      DBFWriteDoubleAttribute( Idbffile, i, c++, insertObjects[i].rowSp );

      SHPDestroyObject( psObject );
    }
    SHPClose( ihSHP );
    DBFClose( Idbffile );

    QgsDebugMsg( "Done!" );
  }
}
