/***************************************************************************
      qgsmssqlgeometryparser.cpp  -  SqlGeometry parser for mssql server
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlgeometryparser.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsapplication.h"

/*   SqlGeometry serialization format

Simple Point (SerializationProps & IsSinglePoint)
  [SRID][0x01][SerializationProps][Point][z][m]

Simple Line Segment (SerializationProps & IsSingleLineSegment)
  [SRID][0x01][SerializationProps][Point1][Point2][z1][z2][m1][m2]

Complex Geometries
  [SRID][0x01][SerializationProps][NumPoints][Point1]..[PointN][z1]..[zN][m1]..[mN]
  [NumFigures][Figure]..[Figure][NumShapes][Shape]..[Shape]

SRID
  Spatial Reference Id (4 bytes)

SerializationProps (bitmask) 1 byte
  0x01 = HasZValues
  0x02 = HasMValues
  0x04 = IsValid
  0x08 = IsSinglePoint
  0x10 = IsSingleLineSegment
  0x20 = IsWholeGlobe

Point (2-4)x8 bytes, size depends on SerializationProps & HasZValues & HasMValues
  [x][y]                  - SqlGeometry
  [latitude][longitude]   - SqlGeography

Figure
  [FigureAttribute][PointOffset]

FigureAttribute (1 byte)
  0x00 = Interior Ring
  0x01 = Stroke
  0x02 = Exterior Ring

Shape
  [ParentFigureOffset][FigureOffset][ShapeType]

ShapeType (1 byte)
  0x00 = Unknown
  0x01 = Point
  0x02 = LineString
  0x03 = Polygon
  0x04 = MultiPoint
  0x05 = MultiLineString
  0x06 = MultiPolygon
  0x07 = GeometryCollection

*/

/************************************************************************/
/*                         Geometry parser macros                       */
/************************************************************************/

#define SP_NONE 0
#define SP_HASZVALUES 1
#define SP_HASMVALUES 2
#define SP_ISVALID 4
#define SP_ISSINGLEPOINT 8
#define SP_ISSINGLELINESEGMENT 0x10
#define SP_ISWHOLEGLOBE 0x20

#define ST_UNKNOWN 0
#define ST_POINT 1
#define ST_LINESTRING 2
#define ST_POLYGON 3
#define ST_MULTIPOINT 4
#define ST_MULTILINESTRING 5
#define ST_MULTIPOLYGON 6
#define ST_GEOMETRYCOLLECTION 7

#define ReadInt32(nPos) (*((unsigned int*)(pszData + (nPos))))

#define ReadByte(nPos) (pszData[nPos])

#define ReadDouble(nPos) (*((double*)(pszData + (nPos))))

#define ParentOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 ))
#define FigureOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 + 4))
#define ShapeType(iShape) (ReadByte(nShapePos + (iShape) * 9 + 8))

#define NextFigureOffset(iShape) (iShape + 1 < nNumShapes? FigureOffset((iShape) +1) : nNumFigures)

#define FigureAttribute(iFigure) (ReadByte(nFigurePos + (iFigure) * 5))
#define PointOffset(iFigure) (ReadInt32(nFigurePos + (iFigure) * 5 + 1))
#define NextPointOffset(iFigure) (iFigure + 1 < nNumFigures? PointOffset((iFigure) +1) : nNumPoints)

#define ReadX(iPoint) (ReadDouble(nPointPos + 16 * (iPoint)))
#define ReadY(iPoint) (ReadDouble(nPointPos + 16 * (iPoint) + 8))
#define ReadZ(iPoint) (ReadDouble(nPointPos + 16 * nNumPoints + 8 * (iPoint)))
#define ReadM(iPoint) (ReadDouble(nPointPos + 24 * nNumPoints + 8 * (iPoint)))

/************************************************************************/
/*                   QgsMssqlGeometryParser()                           */
/************************************************************************/

QgsMssqlGeometryParser::QgsMssqlGeometryParser()
{
}

void QgsMssqlGeometryParser::DumpMemoryToLog( const char* pszMsg, unsigned char* pszInput, int nLen )
{
#if 0
  char buf[55];
  int len = 0;
  QFile file( "qgsmssql.log" );
  file.open( QIODevice::Append );
  file.write( pszMsg, strlen( pszMsg ) );
  file.write( "\n" );
  sprintf( buf + len, "%05d ", 0 );
  len += 6;
  for ( int i = 0; i < nLen; i++ )
  {
    sprintf( buf + len, "%02x ", pszInput[i] );
    len += 3;
    if ( len == 54 )
    {
      file.write( buf, len );
      len = 0;
      file.write( "\n" );
      sprintf( buf + len, "%05d ", i + 1 );
      len = 6;
    }
  }
  file.write( "\n" );
  file.close();
#else
  Q_UNUSED( pszMsg );
  Q_UNUSED( pszInput );
  Q_UNUSED( nLen );
#endif
}

/************************************************************************/
/*                         CopyBytes()                                  */
/************************************************************************/

void QgsMssqlGeometryParser::CopyBytes( void* src, int len )
{
  if ( nWkbLen + len > nWkbMaxLen )
  {
    QgsDebugMsg( "CopyBytes wkb buffer realloc" );
    unsigned char* pszWkbTmp = new unsigned char[nWkbLen + len + 100];
    memcpy( pszWkbTmp, pszWkb, nWkbLen );
    delete pszWkb;
    pszWkb = pszWkbTmp;
    nWkbMaxLen = nWkbLen + len + 100;
  }
  memcpy( pszWkb + nWkbLen, src, len );
  nWkbLen += len;
}

/************************************************************************/
/*                         CopyCoordinates()                            */
/************************************************************************/

void QgsMssqlGeometryParser::CopyCoordinates( int iPoint )
{
  if ( IsGeography )
  {
    CopyBytes( pszData + nPointPos + 16 * iPoint + 8, 8 ); // longitude
    CopyBytes( pszData + nPointPos + 16 * iPoint, 8 ); // latitude
  }
  else
    // copy geometry coords
    CopyBytes( pszData + nPointPos + 16 * iPoint, 16 );

  if ( chProps & SP_HASZVALUES )
    CopyBytes( pszData + nPointPos + 16 * nNumPoints + 8 * iPoint, 8 ); // copy z value
}

/************************************************************************/
/*                         CopyPoint()                                  */
/************************************************************************/

void QgsMssqlGeometryParser::CopyPoint( int iPoint )
{
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBPoint25D;
  else
    wkbType = QGis::WKBPoint;
  CopyBytes( &wkbType, 4 );
  // copy coordinates
  CopyCoordinates( iPoint );
}

/************************************************************************/
/*                         ReadPoint()                                  */
/************************************************************************/

void QgsMssqlGeometryParser::ReadPoint( int iShape )
{
  int iFigure = FigureOffset( iShape );
  if ( iFigure < nNumFigures )
  {
    int iPoint = PointOffset( iFigure );
    if ( iPoint < nNumPoints )
    {
      CopyPoint( iPoint );
    }
  }
}

/************************************************************************/
/*                         ReadMultiPoint()                             */
/************************************************************************/

void QgsMssqlGeometryParser::ReadMultiPoint( int iShape )
{
  int i, iCount;
  iCount = nNumShapes - iShape - 1;
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBMultiPoint25D;
  else
    wkbType = QGis::WKBMultiPoint;
  CopyBytes( &wkbType, 4 );
  // copy point count
  CopyBytes( &iCount, 4 );
  // copy points
  for ( i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POINT )
        ReadPoint( i );
    }
  }
}

/************************************************************************/
/*                         ReadLineString()                             */
/************************************************************************/

void QgsMssqlGeometryParser::ReadLineString( int iShape )
{
  int iFigure, iPoint, iNextPoint, i, iCount;
  iFigure = FigureOffset( iShape );

  iPoint = PointOffset( iFigure );
  iNextPoint = NextPointOffset( iFigure );
  iCount = iNextPoint - iPoint;
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBLineString25D;
  else
    wkbType = QGis::WKBLineString;
  CopyBytes( &wkbType, 4 );
  // copy length
  CopyBytes( &iCount, 4 );
  // copy coordinates
  i = 0;
  while ( iPoint < iNextPoint )
  {
    CopyCoordinates( iPoint );
    ++iPoint;
    ++i;
  }
}

/************************************************************************/
/*                         ReadMultiLineString()                        */
/************************************************************************/

void QgsMssqlGeometryParser::ReadMultiLineString( int iShape )
{
  int i, iCount;
  iCount = nNumShapes - iShape - 1;
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBMultiLineString25D;
  else
    wkbType = QGis::WKBMultiLineString;
  CopyBytes( &wkbType, 4 );
  // copy length
  CopyBytes( &iCount, 4 );
  // copy linestrings
  for ( i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_LINESTRING )
        ReadLineString( i );
    }
  }
}

/************************************************************************/
/*                         ReadPolygon()                                */
/************************************************************************/

void QgsMssqlGeometryParser::ReadPolygon( int iShape )
{
  int iFigure, iPoint, iNextPoint, iCount, i;
  int iNextFigure = NextFigureOffset( iShape );
  iCount = iNextFigure - FigureOffset( iShape );
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBPolygon25D;
  else
    wkbType = QGis::WKBPolygon;
  CopyBytes( &wkbType, 4 );
  // copy ring count
  CopyBytes( &iCount, 4 );
  // copy rings
  for ( iFigure = FigureOffset( iShape ); iFigure < iNextFigure; iFigure++ )
  {
    iPoint = PointOffset( iFigure );
    iNextPoint = NextPointOffset( iFigure );
    iCount = iNextPoint - iPoint;
    if ( iCount <= 0 )
      continue;
    // copy point count
    CopyBytes( &iCount, 4 );
    // copy coordinates
    i = 0;
    while ( iPoint < iNextPoint )
    {
      CopyCoordinates( iPoint );
      ++iPoint;
      ++i;
    }
  }
}

/************************************************************************/
/*                         ReadMultiPolygon()                           */
/************************************************************************/

void QgsMssqlGeometryParser::ReadMultiPolygon( int iShape )
{
  int i;
  int iCount = nNumShapes - iShape - 1;;
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType;
  if ( chProps & SP_HASZVALUES )
    wkbType = QGis::WKBMultiPolygon25D;
  else
    wkbType = QGis::WKBMultiPolygon;
  CopyBytes( &wkbType, 4 );
  // copy poly count
  CopyBytes( &iCount, 4 );
  // copy polygons
  for ( i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POLYGON )
        ReadPolygon( i );
    }
  }
}

/************************************************************************/
/*                         ReadGeometryCollection()                     */
/************************************************************************/

void QgsMssqlGeometryParser::ReadGeometryCollection( int iShape )
{
  int i;
  int iCount = nNumShapes - iShape - 1;;
  if ( iCount <= 0 )
    return;
  // copy byte order
  CopyBytes( &chByteOrder, 1 );
  // copy type
  int wkbType = QGis::WKBUnknown;;
  CopyBytes( &wkbType, 4 );
  // copy geom count
  CopyBytes( &iCount, 4 );
  for ( i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      switch ( ShapeType( i ) )
      {
        case ST_POINT:
          ReadPoint( i );
          break;
        case ST_LINESTRING:
          ReadLineString( i );
          break;
        case ST_POLYGON:
          ReadPolygon( i );
          break;
        case ST_MULTIPOINT:
          ReadMultiPoint( i );
          break;
        case ST_MULTILINESTRING:
          ReadMultiLineString( i );
          break;
        case ST_MULTIPOLYGON:
          ReadMultiPolygon( i );
          break;
        case ST_GEOMETRYCOLLECTION:
          ReadGeometryCollection( i );
          break;
      }
    }
  }
}

/************************************************************************/
/*                         ParseSqlGeometry()                           */
/************************************************************************/

unsigned char* QgsMssqlGeometryParser::ParseSqlGeometry( unsigned char* pszInput, int nLen )
{
  if ( nLen < 10 )
  {
    QgsDebugMsg( "ParseSqlGeometry not enough data" );
    DumpMemoryToLog( "Not enough data", pszInput, nLen );
    return NULL;
  }

  pszData = pszInput;
  nWkbMaxLen = nLen;

  /* store the SRS id for further use */
  nSRSId = ReadInt32( 0 );

  if ( ReadByte( 4 ) != 1 )
  {
    QgsDebugMsg( "ParseSqlGeometry corrupt data" );
    DumpMemoryToLog( "Corrupt data", pszInput, nLen );
    return NULL;
  }

  chProps = ReadByte( 5 );

  if ( chProps & SP_HASMVALUES )
    nPointSize = 32;
  else if ( chProps & SP_HASZVALUES )
    nPointSize = 24;
  else
    nPointSize = 16;

  /* store byte order */
  chByteOrder = QgsApplication::endian();

  pszWkb = new unsigned char[nLen]; // wkb should be less or equal in size
  nWkbLen = 0;

  if ( chProps & SP_ISSINGLEPOINT )
  {
    // single point geometry
    nNumPoints = 1;
    nPointPos = 6;

    if ( nLen < 6 + nPointSize )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry not enough data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    CopyPoint( 0 );
  }
  else if ( chProps & SP_ISSINGLELINESEGMENT )
  {
    // single line segment with 2 points
    nNumPoints = 2;
    nPointPos = 6;

    if ( nLen < 6 + 2 * nPointSize )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry not enough data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    // copy byte order
    CopyBytes( &chByteOrder, 1 );
    // copy type
    int wkbType;
    if ( chProps & SP_HASZVALUES )
      wkbType = QGis::WKBLineString25D;
    else
      wkbType = QGis::WKBLineString;
    CopyBytes( &wkbType, 4 );
    // copy point count
    int iCount = 2;
    CopyBytes( &iCount, 4 );
    // copy points
    CopyCoordinates( 0 );
    CopyCoordinates( 1 );
  }
  else
  {
    // complex geometries
    nNumPoints = ReadInt32( 6 );

    if ( nNumPoints <= 0 )
    {
      free( pszWkb );
      return NULL;
    }

    // position of the point array
    nPointPos = 10;

    // position of the figures
    nFigurePos = nPointPos + nPointSize * nNumPoints + 4;

    if ( nLen < nFigurePos )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry not enough data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    nNumFigures = ReadInt32( nFigurePos - 4 );

    if ( nNumFigures <= 0 )
    {
      free( pszWkb );
      return NULL;
    }

    // position of the shapes
    nShapePos = nFigurePos + 5 * nNumFigures + 4;

    if ( nLen < nShapePos )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry not enough data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    nNumShapes = ReadInt32( nShapePos - 4 );

    if ( nLen < nShapePos + 9 * nNumShapes )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry not enough data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    if ( nNumShapes <= 0 )
    {
      free( pszWkb );
      return NULL;
    }

    // pick up the root shape
    if ( ParentOffset( 0 ) != 0xFFFFFFFF )
    {
      free( pszWkb );
      QgsDebugMsg( "ParseSqlGeometry corrupt data" );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return NULL;
    }

    // determine the shape type
    switch ( ShapeType( 0 ) )
    {
      case ST_POINT:
        ReadPoint( 0 );
        break;
      case ST_LINESTRING:
        ReadLineString( 0 );
        break;
      case ST_POLYGON:
        ReadPolygon( 0 );
        break;
      case ST_MULTIPOINT:
        ReadMultiPoint( 0 );
        break;
      case ST_MULTILINESTRING:
        ReadMultiLineString( 0 );
        break;
      case ST_MULTIPOLYGON:
        ReadMultiPolygon( 0 );
        break;
        //case ST_GEOMETRYCOLLECTION:
        //ReadGeometryCollection(0);
        //break;
      default:
        free( pszWkb );
        QgsDebugMsg( "ParseSqlGeometry unsupported geometry type" );
        DumpMemoryToLog( "Unsupported geometry type", pszInput, nLen );
        return NULL;
    }
  }

  return pszWkb;
}

