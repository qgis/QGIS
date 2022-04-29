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

#define ReadInt32(nPos) (*((unsigned int*)(mData + (nPos))))

#define ReadByte(nPos) (mData[nPos])

#define ReadDouble(nPos) (*((double*)(mData + (nPos))))

#define ParentOffset(iShape) (ReadInt32(mShapePos + (iShape) * 9 ))
#define FigureOffset(iShape) (ReadInt32(mShapePos + (iShape) * 9 + 4))
#define ShapeType(iShape) (ReadByte(mShapePos + (iShape) * 9 + 8))
#define SegmentType(iSegment) (ReadByte(mSegmentPos + (iSegment)))

#define NextFigureOffset(iShape) (iShape + 1 < mNumShapes? FigureOffset((iShape) +1) : mNumFigures)

#define FigureAttribute(iFigure) (ReadByte(mFigurePos + (iFigure) * 5))
#define PointOffset(iFigure) (ReadInt32(mFigurePos + (iFigure) * 5 + 1))
#define NextPointOffset(iFigure) (iFigure + 1 < mNumFigures? PointOffset((iFigure) +1) : mNumPoints)

#define ReadX(iPoint) (ReadDouble(mPointPos + 16 * (iPoint)))
#define ReadY(iPoint) (ReadDouble(mPointPos + 16 * (iPoint) + 8))
#define ReadZ(iPoint) (ReadDouble(mPointPos + 16 * mNumPoints + 8 * (iPoint)))
#define ReadM(iPoint) (ReadDouble(mPointPos + 24 * mNumPoints + 8 * (iPoint)))

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

QgsPoint QgsMssqlGeometryParser::readCoordinates( int iPoint ) const
{
  if ( mIsGeography )
  {
    if ( ( mProps & SP_HASZVALUES ) && ( mProps & SP_HASMVALUES ) )
      return QgsPoint( QgsWkbTypes::PointZM, ReadY( iPoint ), ReadX( iPoint ), ReadZ( iPoint ), ReadM( iPoint ) );
    else if ( mProps & SP_HASZVALUES )
      return QgsPoint( QgsWkbTypes::PointZ, ReadY( iPoint ), ReadX( iPoint ), ReadZ( iPoint ) );
    else if ( mProps & SP_HASMVALUES )
      return QgsPoint( QgsWkbTypes::PointM, ReadY( iPoint ), ReadX( iPoint ), 0.0, ReadZ( iPoint ) );
    else
      return QgsPoint( QgsWkbTypes::Point, ReadY( iPoint ), ReadX( iPoint ) );
  }
  else
  {
    if ( ( mProps & SP_HASZVALUES ) && ( mProps & SP_HASMVALUES ) )
      return QgsPoint( QgsWkbTypes::PointZM, ReadX( iPoint ), ReadY( iPoint ), ReadZ( iPoint ), ReadM( iPoint ) );
    else if ( mProps & SP_HASZVALUES )
      return QgsPoint( QgsWkbTypes::PointZ, ReadX( iPoint ), ReadY( iPoint ), ReadZ( iPoint ) );
    else if ( mProps & SP_HASMVALUES )
      return QgsPoint( QgsWkbTypes::PointM, ReadX( iPoint ), ReadY( iPoint ), 0.0, ReadZ( iPoint ) );
    else
      return QgsPoint( QgsWkbTypes::Point, ReadX( iPoint ), ReadY( iPoint ) );
  }
}

void QgsMssqlGeometryParser::readCoordinates( int iPoint, int iNextPoint, double *x, double *y, double *z, double *m ) const
{
  int i = 0;
  if ( mIsGeography )
  {
    if ( ( mProps & SP_HASZVALUES ) && ( mProps & SP_HASMVALUES ) )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadY( iPoint );
        y[i] = ReadX( iPoint );
        z[i] = ReadZ( iPoint );
        m[i] = ReadM( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else if ( mProps & SP_HASZVALUES )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadY( iPoint );
        y[i] = ReadX( iPoint );
        z[i] = ReadZ( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else if ( mProps & SP_HASMVALUES )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadY( iPoint );
        y[i] = ReadX( iPoint );
        m[i] = ReadZ( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadY( iPoint );
        y[i] = ReadX( iPoint );
        ++iPoint;
        ++i;
      }
    }
  }
  else
  {
    if ( ( mProps & SP_HASZVALUES ) && ( mProps & SP_HASMVALUES ) )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadX( iPoint );
        y[i] = ReadY( iPoint );
        z[i] = ReadZ( iPoint );
        m[i] = ReadM( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else if ( mProps & SP_HASZVALUES )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadX( iPoint );
        y[i] = ReadY( iPoint );
        z[i] = ReadZ( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else if ( mProps & SP_HASMVALUES )
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadX( iPoint );
        y[i] = ReadY( iPoint );
        m[i] = ReadZ( iPoint );
        ++iPoint;
        ++i;
      }
    }
    else
    {
      while ( iPoint < iNextPoint )
      {
        x[i] = ReadX( iPoint );
        y[i] = ReadY( iPoint );
        ++iPoint;
        ++i;
      }
    }
  }
}

const QgsPointSequence QgsMssqlGeometryParser::readPointSequence( int iPoint, int iNextPoint ) const
{
  if ( iPoint >= iNextPoint )
    return QgsPointSequence();

  QgsPointSequence pts;

  while ( iPoint < iNextPoint )
  {
    pts << readCoordinates( iPoint );
    ++iPoint;
  }

  return pts;
}

std::unique_ptr< QgsPoint > QgsMssqlGeometryParser::readPoint( int iFigure )
{
  if ( iFigure < mNumFigures )
  {
    const int iPoint = PointOffset( iFigure );
    if ( iPoint < mNumPoints )
    {
      return std::make_unique< QgsPoint >( readCoordinates( iPoint ) );
    }
  }
  return nullptr;
}

std::unique_ptr< QgsMultiPoint > QgsMssqlGeometryParser::readMultiPoint( int iShape )
{
  std::unique_ptr< QgsMultiPoint > poMultiPoint = std::make_unique< QgsMultiPoint >();
  poMultiPoint->reserve( mNumShapes );
  for ( int i = iShape + 1; i < mNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POINT )
        poMultiPoint->addGeometry( readPoint( FigureOffset( i ) ).release() );
    }
  }

  return poMultiPoint;
}

std::unique_ptr< QgsLineString > QgsMssqlGeometryParser::readLineString( int iPoint, int iNextPoint )
{
  QVector< double > xOut( iNextPoint - iPoint );
  QVector< double > yOut( iNextPoint - iPoint );
  QVector< double > zOut;
  if ( mProps & SP_HASZVALUES )
    zOut.resize( iNextPoint - iPoint );
  QVector< double > mOut;
  if ( mProps & SP_HASMVALUES )
    mOut.resize( iNextPoint - iPoint );
  double *x = xOut.data();
  double *y = yOut.data();
  double *z = zOut.data();
  double *m = mOut.data();

  readCoordinates( iPoint, iNextPoint, x, y, z, m );

  return std::make_unique< QgsLineString >( xOut, yOut, zOut, mOut );
}

std::unique_ptr< QgsLineString > QgsMssqlGeometryParser::readLineString( int iFigure )
{
  return readLineString( PointOffset( iFigure ), NextPointOffset( iFigure ) );
}

std::unique_ptr< QgsCircularString > QgsMssqlGeometryParser::readCircularString( int iPoint, int iNextPoint )
{
  std::unique_ptr< QgsCircularString > poCircularString = std::make_unique< QgsCircularString >();
  poCircularString->setPoints( readPointSequence( iPoint, iNextPoint ) );
  return poCircularString;
}

std::unique_ptr< QgsCircularString > QgsMssqlGeometryParser::readCircularString( int iFigure )
{
  return readCircularString( PointOffset( iFigure ), NextPointOffset( iFigure ) );
}

std::unique_ptr< QgsMultiLineString > QgsMssqlGeometryParser::readMultiLineString( int iShape )
{
  std::unique_ptr< QgsMultiLineString > poMultiLineString = std::make_unique< QgsMultiLineString >();
  poMultiLineString->reserve( mNumShapes );
  for ( int i = iShape + 1; i < mNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_LINESTRING )
        poMultiLineString->addGeometry( readLineString( FigureOffset( i ) ).release() );
    }
  }

  return poMultiLineString;
}

std::unique_ptr< QgsPolygon > QgsMssqlGeometryParser::readPolygon( int iShape )
{
  int iFigure;
  int iRingCount = 0;
  const int iNextFigure = NextFigureOffset( iShape );

  std::unique_ptr< QgsPolygon > poPoly = std::make_unique< QgsPolygon >();
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

std::unique_ptr< QgsMultiPolygon > QgsMssqlGeometryParser::readMultiPolygon( int iShape )
{
  std::unique_ptr< QgsMultiPolygon > poMultiPolygon = std::make_unique< QgsMultiPolygon >();
  poMultiPolygon->reserve( mNumShapes );
  for ( int i = iShape + 1; i < mNumShapes; i++ )
  {
    if ( ParentOffset( i ) == ( unsigned int )iShape )
    {
      if ( ShapeType( i ) == ST_POLYGON )
        poMultiPolygon->addGeometry( readPolygon( i ).release() );
    }
  }

  return poMultiPolygon;
}

std::unique_ptr< QgsCompoundCurve > QgsMssqlGeometryParser::readCompoundCurve( int iFigure )
{
  int iPoint, iNextPoint, nPointsPrepared;
  std::unique_ptr< QgsCompoundCurve > poCompoundCurve = std::make_unique< QgsCompoundCurve >();
  iPoint = PointOffset( iFigure );
  iNextPoint = NextPointOffset( iFigure ) - 1;

  const std::unique_ptr< QgsCurve > poGeom;

  nPointsPrepared = 0;
  bool isCurve = false;
  while ( iPoint < iNextPoint && mSegment < mNumSegments )
  {
    switch ( SegmentType( mSegment ) )
    {
      case SMT_FIRSTLINE:
        if ( nPointsPrepared > 0 )
        {
          if ( isCurve )
            poCompoundCurve->addCurve( readCircularString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
          else
            poCompoundCurve->addCurve( readLineString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
        }
        isCurve = false;
        nPointsPrepared = 1;
        ++iPoint;
        break;
      case SMT_LINE:
        ++nPointsPrepared;
        ++iPoint;
        break;
      case SMT_FIRSTARC:
        if ( nPointsPrepared > 0 )
        {
          if ( isCurve )
            poCompoundCurve->addCurve( readCircularString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
          else
            poCompoundCurve->addCurve( readLineString( iPoint - nPointsPrepared, iPoint + 1 ).release() );
        }
        isCurve = true;
        nPointsPrepared = 2;
        iPoint += 2;
        break;
      case SMT_ARC:
        nPointsPrepared += 2;
        iPoint += 2;
        break;
    }
    ++mSegment;
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

std::unique_ptr< QgsCurvePolygon > QgsMssqlGeometryParser::readCurvePolygon( int iShape )
{
  int iFigure;
  int iRingCount = 0;
  const int iNextFigure = NextFigureOffset( iShape );

  std::unique_ptr< QgsCurvePolygon > poPoly = std::make_unique< QgsCurvePolygon >();
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

std::unique_ptr< QgsGeometryCollection > QgsMssqlGeometryParser::readGeometryCollection( int iShape )
{
  std::unique_ptr< QgsGeometryCollection> poGeomColl = std::make_unique< QgsGeometryCollection >();
  poGeomColl->reserve( mNumShapes );
  for ( int i = iShape + 1; i < mNumShapes; i++ )
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

std::unique_ptr<QgsAbstractGeometry> QgsMssqlGeometryParser::parseSqlGeometry( unsigned char *pszInput, int nLen )
{
  if ( nLen < 10 )
  {
    QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
    DumpMemoryToLog( "Not enough data", pszInput, nLen );
    return nullptr;
  }

  mData = pszInput;

  /* store the SRS id for further use */
  mSRSId = ReadInt32( 0 );

  mVersion = ReadByte( 4 );

  if ( mVersion == 0 || mVersion > 2 )
  {
    QgsDebugMsg( QStringLiteral( "ParseSqlGeometry corrupt data" ) );
    DumpMemoryToLog( "Corrupt data", pszInput, nLen );
    return nullptr;
  }

  mProps = ReadByte( 5 );

  if ( mProps & SP_HASZVALUES && mProps & SP_HASMVALUES )
    mPointSize = 32;
  else if ( mProps & SP_HASZVALUES || mProps & SP_HASMVALUES )
    mPointSize = 24;
  else
    mPointSize = 16;

  std::unique_ptr< QgsAbstractGeometry> poGeom;

  if ( mProps & SP_ISSINGLEPOINT )
  {
    // single point geometry
    mNumPoints = 1;
    mPointPos = 6;

    if ( nLen < 6 + mPointSize )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    poGeom = std::make_unique< QgsPoint >( readCoordinates( 0 ) );
  }
  else if ( mProps & SP_ISSINGLELINESEGMENT )
  {
    // single line segment with 2 points
    mNumPoints = 2;
    mPointPos = 6;

    if ( nLen < 6 + 2 * mPointSize )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    poGeom = std::make_unique< QgsLineString >( readCoordinates( 0 ), readCoordinates( 1 ) );
  }
  else
  {
    // complex geometries
    mNumPoints = ReadInt32( 6 );

    if ( mNumPoints <= 0 )
    {
      return nullptr;
    }

    // position of the point array
    mPointPos = 10;

    // position of the figures
    mFigurePos = mPointPos + mPointSize * mNumPoints + 4;

    if ( nLen < mFigurePos )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    mNumFigures = ReadInt32( mFigurePos - 4 );

    if ( mNumFigures <= 0 )
    {
      return nullptr;
    }

    // position of the shapes
    mShapePos = mFigurePos + 5 * mNumFigures + 4;

    if ( nLen < mShapePos )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    mNumShapes = ReadInt32( mShapePos - 4 );

    if ( nLen < mShapePos + 9 * mNumShapes )
    {
      QgsDebugMsg( QStringLiteral( "ParseSqlGeometry not enough data" ) );
      DumpMemoryToLog( "Not enough data", pszInput, nLen );
      return nullptr;
    }

    if ( mNumShapes <= 0 )
    {
      return nullptr;
    }

    // position of the segments (for complex curve figures)
    if ( mVersion == 0x02 )
    {
      mSegment = 0;
      mSegmentPos = mShapePos + 9 * mNumShapes + 4;
      if ( nLen > mSegmentPos )
      {
        // segment array is present
        mNumSegments = ReadInt32( mSegmentPos - 4 );
        if ( nLen < mSegmentPos + mNumSegments )
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
