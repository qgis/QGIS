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

/*   SqlGeometry/SqlGeography serialization format

Simple Point (SerializationProps & IsSinglePoint)
  [SRID][0x01][SerializationProps][Point][z][m]

Simple Line Segment (SerializationProps & IsSingleLineSegment)
  [SRID][0x01][SerializationProps][Point1][Point2][z1][z2][m1][m2]

Complex Geometries
  [SRID][VersionAttribute][SerializationProps][NumPoints][Point1]..[PointN][z1]..[zN][m1]..[mN]
  [NumFigures][Figure]..[Figure][NumShapes][Shape]..[Shape]

Complex Geometries (FigureAttribute == Curve)
  [SRID][VersionAttribute][SerializationProps][NumPoints][Point1]..[PointN][z1]..[zN][m1]..[mN]
  [NumFigures][Figure]..[Figure][NumShapes][Shape]..[Shape][NumSegments][SegmentType]..[SegmentType]

VersionAttribute (1 byte)
  0x01 = Katmai (MSSQL2008+)
  0x02 = Denali (MSSQL2012+)

SRID
  Spatial Reference Id (4 bytes)

SerializationProps (bitmask) 1 byte
  0x01 = HasZValues
  0x02 = HasMValues
  0x04 = IsValid
  0x08 = IsSinglePoint
  0x10 = IsSingleLineSegment
  0x20 = IsLargerThanAHemisphere

Point (2-4)x8 bytes, size depends on SerializationProps & HasZValues & HasMValues
  [x][y]                  - SqlGeometry
  [latitude][longitude]   - SqlGeography

Figure
  [FigureAttribute][PointOffset]

FigureAttribute - Katmai (1 byte)
  0x00 = Interior Ring
  0x01 = Stroke
  0x02 = Exterior Ring

FigureAttribute - Denali (1 byte)
  0x00 = None
  0x01 = Line
  0x02 = Arc
  0x03 = Curve

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
  -- Denali
  0x08 = CircularString
  0x09 = CompoundCurve
  0x0A = CurvePolygon
  0x0B = FullGlobe

SegmentType (1 byte)
  0x00 = Line
  0x01 = Arc
  0x02 = FirstLine
  0x03 = FirstArc

*/

/************************************************************************/
/*                         Geometry parser macros                       */
/************************************************************************/

#define VA_KATMAI 0x01
#define VA_DENALI 0x02

#define SP_NONE 0
#define SP_HASZVALUES 1
#define SP_HASMVALUES 2
#define SP_ISVALID 4
#define SP_ISSINGLEPOINT 8
#define SP_ISSINGLELINESEGMENT 0x10
#define SP_ISLARGERTHANAHEMISPHERE 0x20

#define ST_UNKNOWN 0
#define ST_POINT 1
#define ST_LINESTRING 2
#define ST_POLYGON 3
#define ST_MULTIPOINT 4
#define ST_MULTILINESTRING 5
#define ST_MULTIPOLYGON 6
#define ST_GEOMETRYCOLLECTION 7
#define ST_CIRCULARSTRING 8
#define ST_COMPOUNDCURVE 9
#define ST_CURVEPOLYGON 10
#define ST_FULLGLOBE 11

#define FA_INTERIORRING 0x00
#define FA_STROKE 0x01
#define FA_EXTERIORRING 0x02

#define FA_NONE 0x00
#define FA_LINE 0x01
#define FA_ARC 0x02
#define FA_CURVE 0x03

#define SMT_LINE 0
#define SMT_ARC 1
#define SMT_FIRSTLINE 2
#define SMT_FIRSTARC 3

#define ReadInt32(nPos) (*((unsigned int*)(pszData + (nPos))))

#define ReadByte(nPos) (pszData[nPos])

#define ReadDouble(nPos) (*((double*)(pszData + (nPos))))

#define ParentOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 ))
#define FigureOffset(iShape) (ReadInt32(nShapePos + (iShape) * 9 + 4))
#define ShapeType(iShape) (ReadByte(nShapePos + (iShape) * 9 + 8))
#define SegmentType(iSegment) (ReadByte(nSegmentPos + (iSegment)))

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

void QgsMssqlGeometryParser::DumpMemoryToLog( const char *pszMsg, unsigned char *pszInput, int nLen )
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
  Q_UNUSED( pszMsg )
  Q_UNUSED( pszInput )
  Q_UNUSED( nLen )
#endif
}

/************************************************************************/
/*                         readPoint()                                  */
/************************************************************************/

QgsPoint QgsMssqlGeometryParser::readCoordinates( int iPoint )
{
  if ( IsGeography )
  {
    if ( ( chProps & SP_HASZVALUES ) && ( chProps & SP_HASMVALUES ) )
      return QgsPoint( QgsWkbTypes::PointZM, ReadY( iPoint ), ReadX( iPoint ), ReadZ( iPoint ), ReadM( iPoint ) );
    else if ( chProps & SP_HASZVALUES )
      return QgsPoint( QgsWkbTypes::PointZ, ReadY( iPoint ), ReadX( iPoint ), ReadZ( iPoint ) );
    else if ( chProps & SP_HASMVALUES )
      return QgsPoint( QgsWkbTypes::PointM, ReadY( iPoint ), ReadX( iPoint ), 0.0, ReadZ( iPoint ) );
    else
      return QgsPoint( QgsWkbTypes::Point, ReadY( iPoint ), ReadX( iPoint ) );
  }
  else
  {
    if ( ( chProps & SP_HASZVALUES ) && ( chProps & SP_HASMVALUES ) )
      return QgsPoint( QgsWkbTypes::PointZM, ReadX( iPoint ), ReadY( iPoint ), ReadZ( iPoint ), ReadM( iPoint ) );
    else if ( chProps & SP_HASZVALUES )
      return QgsPoint( QgsWkbTypes::PointZ, ReadX( iPoint ), ReadY( iPoint ), ReadZ( iPoint ) );
    else if ( chProps & SP_HASMVALUES )
      return QgsPoint( QgsWkbTypes::PointM, ReadX( iPoint ), ReadY( iPoint ), 0.0, ReadZ( iPoint ) );
    else
      return QgsPoint( QgsWkbTypes::Point, ReadX( iPoint ), ReadY( iPoint ) );
  }
}

/************************************************************************/
/*                         readPointSequence()                          */
/************************************************************************/

const QgsPointSequence QgsMssqlGeometryParser::readPointSequence( int iPoint, int iNextPoint )
{
  if ( iPoint >= iNextPoint )
    return QgsPointSequence();

  QgsPointSequence pts;

  int i = 0;
  while ( iPoint < iNextPoint )
  {
    pts << readCoordinates( iPoint );
    ++iPoint;
    ++i;
  }

  return pts;
}

/************************************************************************/
/*                         readPoint()                                  */
/************************************************************************/

std::unique_ptr< QgsPoint > QgsMssqlGeometryParser::readPoint( int iFigure )
{
  if ( iFigure < nNumFigures )
  {
    int iPoint = PointOffset( iFigure );
    if ( iPoint < nNumPoints )
    {
      return qgis::make_unique< QgsPoint >( readCoordinates( iPoint ) );
    }
  }
  return nullptr;
}

/************************************************************************/
/*                         readMultiPoint()                             */
/************************************************************************/

std::unique_ptr< QgsMultiPoint > QgsMssqlGeometryParser::readMultiPoint( int iShape )
{
  std::unique_ptr< QgsMultiPoint > poMultiPoint = qgis::make_unique< QgsMultiPoint >();
  for ( int i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POINT )
        poMultiPoint->addGeometry( readPoint( FigureOffset( i ) ).release() );
    }
  }

  return poMultiPoint;
}

/************************************************************************/
/*                         readLineString()                             */
/************************************************************************/

std::unique_ptr< QgsLineString > QgsMssqlGeometryParser::readLineString( int iPoint, int iNextPoint )
{
  return qgis::make_unique< QgsLineString >( readPointSequence( iPoint, iNextPoint ) );
}

std::unique_ptr< QgsLineString > QgsMssqlGeometryParser::readLineString( int iFigure )
{
  return readLineString( PointOffset( iFigure ), NextPointOffset( iFigure ) );
}

/************************************************************************/
/*                         readCircularString()                         */
/************************************************************************/

std::unique_ptr< QgsCircularString > QgsMssqlGeometryParser::readCircularString( int iPoint, int iNextPoint )
{
  std::unique_ptr< QgsCircularString > poCircularString = qgis::make_unique< QgsCircularString >();
  poCircularString->setPoints( readPointSequence( iPoint, iNextPoint ) );
  return poCircularString;
}

std::unique_ptr< QgsCircularString > QgsMssqlGeometryParser::readCircularString( int iFigure )
{
  return readCircularString( PointOffset( iFigure ), NextPointOffset( iFigure ) );
}

/************************************************************************/
/*                         readMultiLineString()                        */
/************************************************************************/

std::unique_ptr< QgsMultiLineString > QgsMssqlGeometryParser::readMultiLineString( int iShape )
{
  std::unique_ptr< QgsMultiLineString > poMultiLineString = qgis::make_unique< QgsMultiLineString >();
  for ( int i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_LINESTRING )
        poMultiLineString->addGeometry( readLineString( FigureOffset( i ) ).release() );
    }
  }

  return poMultiLineString;
}

/************************************************************************/
/*                         readPolygon()                                */
/************************************************************************/

std::unique_ptr< QgsPolygon > QgsMssqlGeometryParser::readPolygon( int iShape )
{
  int iFigure;
  int iRingCount = 0;
  int iNextFigure = NextFigureOffset( iShape );

  std::unique_ptr< QgsPolygon > poPoly = qgis::make_unique< QgsPolygon >();
  for ( iFigure = FigureOffset( iShape ); iFigure < iNextFigure; iFigure++ )
  {
    if ( iRingCount == 0 )
      poPoly->setExteriorRing( readLineString( iFigure ).release() );
    else
      poPoly->addInteriorRing( readLineString( iFigure ).release() );

    ++iRingCount;
  }
  return poPoly;
}

/************************************************************************/
/*                         readMultiPolygon()                           */
/************************************************************************/

std::unique_ptr< QgsMultiPolygon > QgsMssqlGeometryParser::readMultiPolygon( int iShape )
{
  std::unique_ptr< QgsMultiPolygon > poMultiPolygon = qgis::make_unique< QgsMultiPolygon >();
  for ( int i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POLYGON )
        poMultiPolygon->addGeometry( readPolygon( i ).release() );
    }
  }

  return poMultiPolygon;
}

/************************************************************************/
/*                         readCompoundCurve()                          */
/************************************************************************/

std::unique_ptr< QgsCompoundCurve > QgsMssqlGeometryParser::readCompoundCurve( int iFigure )
{
  int iPoint, iNextPoint, nPointsPrepared;
  std::unique_ptr< QgsCompoundCurve > poCompoundCurve = qgis::make_unique< QgsCompoundCurve >();
  iPoint = PointOffset( iFigure );
  iNextPoint = NextPointOffset( iFigure ) - 1;

  std::unique_ptr< QgsCurve > poGeom;

  nPointsPrepared = 0;
  bool isCurve = false;
  while ( iPoint < iNextPoint && iSegment < nNumSegments )
  {
    switch ( SegmentType( iSegment ) )
    {
      case SMT_FIRSTLINE:
        if ( isCurve )
          poCompoundCurve->addCurve( readCircularString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
        else
          poCompoundCurve->addCurve( readLineString( iPoint - nPointsPrepared, iPoint + 1 ).release() );

        isCurve = false;
        nPointsPrepared = 1;
        ++iPoint;
        break;
      case SMT_LINE:
        ++nPointsPrepared;
        ++iPoint;
        break;
      case SMT_FIRSTARC:
        if ( isCurve )
          poCompoundCurve->addCurve( readCircularString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
        else
          poCompoundCurve->addCurve( readLineString( iPoint - nPointsPrepared, iPoint + 1 ).release() );

        isCurve = true;
        nPointsPrepared = 2;
        iPoint += 2;
        break;
      case SMT_ARC:
        nPointsPrepared += 2;
        iPoint += 2;
        break;
    }
    ++iSegment;
  }

  // adding the last curve
  if ( iPoint == iNextPoint )
  {
    if ( isCurve )
      poCompoundCurve->addCurve( readCircularString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
    else
      poCompoundCurve->addCurve( readLineString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
  }

  return poCompoundCurve;
}

/************************************************************************/
/*                         readCurvePolygon()                         */
/************************************************************************/

std::unique_ptr< QgsCurvePolygon > QgsMssqlGeometryParser::readCurvePolygon( int iShape )
{
  int iFigure;
  int iRingCount = 0;
  int iNextFigure = NextFigureOffset( iShape );

  std::unique_ptr< QgsCurvePolygon > poPoly = qgis::make_unique< QgsCurvePolygon >();
  for ( iFigure = FigureOffset( iShape ); iFigure < iNextFigure; iFigure++ )
  {
    switch ( FigureAttribute( iFigure ) )
    {
      case FA_LINE:
        if ( iRingCount == 0 )
          poPoly->setExteriorRing( readLineString( iFigure ).release() );
        else
          poPoly->addInteriorRing( readLineString( iFigure ).release() );
        break;
      case FA_ARC:
        if ( iRingCount == 0 )
          poPoly->setExteriorRing( readCircularString( iFigure ).release() );
        else
          poPoly->addInteriorRing( readCircularString( iFigure ).release() );
        break;
      case FA_CURVE:
        if ( iRingCount == 0 )
          poPoly->setExteriorRing( readCompoundCurve( iFigure ).release() );
        else
          poPoly->addInteriorRing( readCompoundCurve( iFigure ).release() );
        break;
    }
    ++iRingCount;
  }
  return poPoly;
}

/************************************************************************/
/*                         readGeometryCollection()                     */
/************************************************************************/

std::unique_ptr< QgsGeometryCollection > QgsMssqlGeometryParser::readGeometryCollection( int iShape )
{
  std::unique_ptr< QgsGeometryCollection> poGeomColl = qgis::make_unique< QgsGeometryCollection >();
  for ( int i = iShape + 1; i < nNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      switch ( ShapeType( i ) )
      {
        case ST_POINT:
          poGeomColl->addGeometry( readPoint( FigureOffset( i ) ).release() );
          break;
        case ST_LINESTRING:
          poGeomColl->addGeometry( readLineString( FigureOffset( i ) ).release() );
          break;
        case ST_POLYGON:
          poGeomColl->addGeometry( readPolygon( i ).release() );
          break;
        case ST_MULTIPOINT:
          poGeomColl->addGeometry( readMultiPoint( i ).release() );
          break;
        case ST_MULTILINESTRING:
          poGeomColl->addGeometry( readMultiLineString( i ).release() );
          break;
        case ST_MULTIPOLYGON:
          poGeomColl->addGeometry( readMultiPolygon( i ).release() );
          break;
        case ST_GEOMETRYCOLLECTION:
          poGeomColl->addGeometry( readGeometryCollection( i ).release() );
          break;
        case ST_CIRCULARSTRING:
          poGeomColl->addGeometry( readCircularString( FigureOffset( i ) ).release() );
          break;
        case ST_COMPOUNDCURVE:
          poGeomColl->addGeometry( readCompoundCurve( FigureOffset( i ) ).release() );
          break;
        case ST_CURVEPOLYGON:
          poGeomColl->addGeometry( readCurvePolygon( i ).release() );
          break;
      }
    }
  }

  return poGeomColl;
}

/************************************************************************/
/*                         parseSqlGeometry()                           */
/************************************************************************/

std::unique_ptr<QgsAbstractGeometry> QgsMssqlGeometryParser::parseSqlGeometry( unsigned char *pszInput, int nLen )
{
  if ( nLen < 10 )
  {
    QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
    DumpMemoryToLog( "Not enough data", pszInput, nLen );
    return nullptr;
  }

  pszData = pszInput;

  /* store the SRS id for further use */
  nSRSId = ReadInt32( 0 );

  chVersion = ReadByte( 4 );

  if ( chVersion == 0 || chVersion > 2 )
  {
    QgsDebugMsg( QStringLiteral( "ParseSqlGeometry corrupt data" ) );
    DumpMemoryToLog( "Corrupt data", pszInput, nLen );
    return nullptr;
  }

  chProps = ReadByte( 5 );

  if ( chProps & SP_HASZVALUES && chProps & SP_HASMVALUES )
    nPointSize = 32;
  else if ( chProps & SP_HASZVALUES || chProps & SP_HASMVALUES )
    nPointSize = 24;
  else
    nPointSize = 16;

  std::unique_ptr< QgsAbstractGeometry> poGeom;

  if ( chProps & SP_ISSINGLEPOINT )
  {
    // single point geometry
    nNumPoints = 1;
    nPointPos = 6;

    if ( nLen < 6 + nPointSize )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    poGeom = qgis::make_unique< QgsPoint >( readCoordinates( 0 ) );
  }
  else if ( chProps & SP_ISSINGLELINESEGMENT )
  {
    // single line segment with 2 points
    nNumPoints = 2;
    nPointPos = 6;

    if ( nLen < 6 + 2 * nPointSize )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    poGeom = qgis::make_unique< QgsLineString >( readCoordinates( 0 ), readCoordinates( 1 ) );
  }
  else
  {
    // complex geometries
    nNumPoints = ReadInt32( 6 );

    if ( nNumPoints <= 0 )
    {
      return nullptr;
    }

    // position of the point array
    nPointPos = 10;

    // position of the figures
    nFigurePos = nPointPos + nPointSize * nNumPoints + 4;

    if ( nLen < nFigurePos )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    nNumFigures = ReadInt32( nFigurePos - 4 );

    if ( nNumFigures <= 0 )
    {
      return nullptr;
    }

    // position of the shapes
    nShapePos = nFigurePos + 5 * nNumFigures + 4;

    if ( nLen < nShapePos )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    nNumShapes = ReadInt32( nShapePos - 4 );

    if ( nLen < nShapePos + 9 * nNumShapes )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    if ( nNumShapes <= 0 )
    {
      return nullptr;
    }

    // position of the segments (for complex curve figures)
    if ( chVersion == 0x02 )
    {
      iSegment = 0;
      nSegmentPos = nShapePos + 9 * nNumShapes + 4;
      if ( nLen > nSegmentPos )
      {
        // segment array is present
        nNumSegments = ReadInt32( nSegmentPos - 4 );
        if ( nLen < nSegmentPos + nNumSegments )
        {
          QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
          DumpMemoryToLog( "Not enough data", pszInput, nLen );
          return nullptr;
        }
      }
    }

    // pick up the root shape
    if ( ParentOffset( 0 ) != 0xFFFFFFFF )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry corrupt data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    // determine the shape type
    switch ( ShapeType( 0 ) )
    {
      case ST_POINT:
        poGeom = readPoint( FigureOffset( 0 ) );
        break;
      case ST_LINESTRING:
        poGeom = readLineString( FigureOffset( 0 ) );
        break;
      case ST_POLYGON:
        poGeom = readPolygon( 0 );
        break;
      case ST_MULTIPOINT:
        poGeom = readMultiPoint( 0 );
        break;
      case ST_MULTILINESTRING:
        poGeom = readMultiLineString( 0 );
        break;
      case ST_MULTIPOLYGON:
        poGeom = readMultiPolygon( 0 );
        break;
      case ST_GEOMETRYCOLLECTION:
        poGeom = readGeometryCollection( 0 );
        break;
      case ST_CIRCULARSTRING:
        poGeom = readCircularString( FigureOffset( 0 ) );
        break;
      case ST_COMPOUNDCURVE:
        poGeom = readCompoundCurve( FigureOffset( 0 ) );
        break;
      case ST_CURVEPOLYGON:
        poGeom = readCurvePolygon( 0 );
        break;
      default:
        QgsDebugMsg( QStringLiteral( "ParseSqlGeometry unsupported geometry type" ) );
        DumpMemoryToLog( "Unsupported geometry type", pszInput, nLen );
        return nullptr;
    }
  }

  return poGeom;
}

