/***************************************************************************
  qgsgeometry.cpp - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <cstdarg>
#include <cstdio>
#include <cmath>

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgsgeometryvalidator.h"

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#define DEFAULT_QUADRANT_SEGMENTS 8

#define CATCH_GEOS(r) \
  catch (GEOSException &e) \
  { \
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr("GEOS") ); \
    return r; \
  }

class GEOSException
{
  public:
    GEOSException( QString theMsg )
    {
      if ( theMsg == "Unknown exception thrown"  && lastMsg.isNull() )
      {
        msg = theMsg;
      }
      else
      {
        msg = theMsg;
        lastMsg = msg;
      }
    }

    // copy constructor
    GEOSException( const GEOSException &rhs )
    {
      *this = rhs;
    }

    ~GEOSException()
    {
      if ( lastMsg == msg )
        lastMsg = QString::null;
    }

    QString what()
    {
      return msg;
    }

  private:
    QString msg;
    static QString lastMsg;
};

QString GEOSException::lastMsg;

static void throwGEOSException( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  QgsDebugMsg( QString( "GEOS exception: %1" ).arg( buffer ) );

  throw GEOSException( QString::fromUtf8( buffer ) );
}

static void printGEOSNotice( const char *fmt, ... )
{
#if defined(QGISDEBUG)
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  QgsDebugMsg( QString( "GEOS notice: %1" ).arg( QString::fromUtf8( buffer ) ) );
#else
  Q_UNUSED( fmt );
#endif
}

class GEOSInit
{
  public:
    GEOSInit()
    {
      initGEOS( printGEOSNotice, throwGEOSException );
    }

    ~GEOSInit()
    {
      finishGEOS();
    }
};

static GEOSInit geosinit;


#if defined(GEOS_VERSION_MAJOR) && (GEOS_VERSION_MAJOR<3)
#define GEOSGeom_getCoordSeq(g) GEOSGeom_getCoordSeq( (GEOSGeometry *) g )
#define GEOSGetExteriorRing(g) GEOSGetExteriorRing( (GEOSGeometry *)g )
#define GEOSGetNumInteriorRings(g) GEOSGetNumInteriorRings( (GEOSGeometry *)g )
#define GEOSGetInteriorRingN(g,i) GEOSGetInteriorRingN( (GEOSGeometry *)g, i )
#define GEOSDisjoint(g0,g1) GEOSDisjoint( (GEOSGeometry *)g0, (GEOSGeometry*)g1 )
#define GEOSIntersection(g0,g1) GEOSIntersection( (GEOSGeometry*) g0, (GEOSGeometry*)g1 )
#define GEOSBuffer(g, d, s) GEOSBuffer( (GEOSGeometry*) g, d, s )
#define GEOSArea(g, a) GEOSArea( (GEOSGeometry*) g, a )
#define GEOSTopologyPreserveSimplify(g, t) GEOSTopologyPreserveSimplify( (GEOSGeometry*) g, t )
#define GEOSGetCentroid(g) GEOSGetCentroid( (GEOSGeometry*) g )
#define GEOSPointOnSurface(g) GEOSPointOnSurface( (GEOSGeometry*) g )

#define GEOSCoordSeq_getSize(cs,n) GEOSCoordSeq_getSize( (GEOSCoordSequence *) cs, n )
#define GEOSCoordSeq_getX(cs,i,x) GEOSCoordSeq_getX( (GEOSCoordSequence *)cs, i, x )
#define GEOSCoordSeq_getY(cs,i,y) GEOSCoordSeq_getY( (GEOSCoordSequence *)cs, i, y )

static GEOSGeometry *createGeosCollection( int typeId, QVector<GEOSGeometry*> geoms );

static GEOSGeometry *cloneGeosGeom( const GEOSGeometry *geom )
{
  // for GEOS < 3.0 we have own cloning function
  // because when cloning multipart geometries they're copied into more general geometry collection instance
  int type = GEOSGeomTypeId(( GEOSGeometry * ) geom );

  if ( type == GEOS_MULTIPOINT || type == GEOS_MULTILINESTRING || type == GEOS_MULTIPOLYGON )
  {
    QVector<GEOSGeometry *> geoms;

    try
    {
      for ( int i = 0; i < GEOSGetNumGeometries(( GEOSGeometry * )geom ); ++i )
        geoms << GEOSGeom_clone(( GEOSGeometry * ) GEOSGetGeometryN(( GEOSGeometry * ) geom, i ) );

      return createGeosCollection( type, geoms );
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      for ( int i = 0; i < geoms.count(); i++ )
        GEOSGeom_destroy( geoms[i] );

      return 0;
    }
  }
  else
  {
    return GEOSGeom_clone(( GEOSGeometry * ) geom );
  }
}

#define GEOSGeom_clone(g) cloneGeosGeom(g)
#endif

QgsGeometry::QgsGeometry()
    : mGeometry( 0 )
    , mGeometrySize( 0 )
    , mGeos( 0 )
    , mDirtyWkb( false )
    , mDirtyGeos( false )
{
}

QgsGeometry::QgsGeometry( QgsGeometry const & rhs )
    : mGeometry( 0 )
    , mGeometrySize( rhs.mGeometrySize )
    , mDirtyWkb( rhs.mDirtyWkb )
    , mDirtyGeos( rhs.mDirtyGeos )
{
  if ( mGeometrySize && rhs.mGeometry )
  {
    mGeometry = new unsigned char[mGeometrySize];
    memcpy( mGeometry, rhs.mGeometry, mGeometrySize );
  }

  // deep-copy the GEOS Geometry if appropriate
  if ( rhs.mGeos )
    mGeos = GEOSGeom_clone( rhs.mGeos );
  else
    mGeos = 0;

}

//! Destructor
QgsGeometry::~QgsGeometry()
{
  if ( mGeometry )
    delete [] mGeometry;

  if ( mGeos )
    GEOSGeom_destroy( mGeos );

}

static unsigned int getNumGeosPoints( const GEOSGeometry *geom )
{
  unsigned int n;
  const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( geom );
  GEOSCoordSeq_getSize( cs, &n );
  return n;
}

static GEOSGeometry *createGeosPoint( const QgsPoint &point )
{
  GEOSCoordSequence *coord = GEOSCoordSeq_create( 1, 2 );
  GEOSCoordSeq_setX( coord, 0, point.x() );
  GEOSCoordSeq_setY( coord, 0, point.y() );
  return GEOSGeom_createPoint( coord );
}

static GEOSCoordSequence *createGeosCoordSequence( const QgsPolyline& points )
{
  GEOSCoordSequence *coord = 0;

  try
  {
    coord = GEOSCoordSeq_create( points.count(), 2 );
    int i;
    for ( i = 0; i < points.count(); i++ )
    {
      GEOSCoordSeq_setX( coord, i, points[i].x() );
      GEOSCoordSeq_setY( coord, i, points[i].y() );
    }
    return coord;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    /*if ( coord )
      GEOSCoordSeq_destroy( coord );*/
    throw;
  }
}

static GEOSGeometry *createGeosCollection( int typeId, QVector<GEOSGeometry*> geoms )
{
  GEOSGeometry **geomarr = new GEOSGeometry*[ geoms.size()];
  if ( !geomarr )
    return 0;

  for ( int i = 0; i < geoms.size(); i++ )
    geomarr[i] = geoms[i];

  GEOSGeometry *geom = 0;

  try
  {
    geom = GEOSGeom_createCollection( typeId, geomarr, geoms.size() );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
  }

  delete [] geomarr;

  return geom;
}

static GEOSGeometry *createGeosLineString( const QgsPolyline& polyline )
{
  GEOSCoordSequence *coord = 0;

  try
  {
    coord = createGeosCoordSequence( polyline );
    return GEOSGeom_createLineString( coord );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    //MH: for strange reasons, geos3 crashes when removing the coordinate sequence
    //if ( coord )
    //GEOSCoordSeq_destroy( coord );
    return 0;
  }
}

static GEOSGeometry *createGeosLinearRing( const QgsPolyline& polyline )
{
  GEOSCoordSequence *coord = 0;

  if ( polyline.count() == 0 )
    return 0;

  try
  {
    if ( polyline[0] != polyline[polyline.size()-1] )
    {
      // Ring not closed
      QgsPolyline closed( polyline );
      closed << closed[0];
      coord = createGeosCoordSequence( closed );
    }
    else
    {
      coord = createGeosCoordSequence( polyline );
    }

    return GEOSGeom_createLinearRing( coord );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    /* as MH has noticed ^, this crashes geos
    if ( coord )
      GEOSCoordSeq_destroy( coord );*/
    return 0;
  }
}

static GEOSGeometry *createGeosPolygon( const QVector<GEOSGeometry*> &rings )
{
  GEOSGeometry *shell;

  if ( rings.size() == 0 )
  {
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
    return GEOSGeom_createEmptyPolygon();
#else
    shell = GEOSGeom_createLinearRing( GEOSCoordSeq_create( 0, 2 ) );
#endif
  }
  else
  {
    shell = rings[0];
  }

  GEOSGeometry **holes = NULL;
  int nHoles = 0;

  if ( rings.size() > 1 )
  {
    nHoles = rings.size() - 1;
    holes = new GEOSGeometry*[ nHoles ];
    if ( !holes )
      return 0;

    for ( int i = 0; i < nHoles; i++ )
      holes[i] = rings[i+1];
  }

  GEOSGeometry *geom = GEOSGeom_createPolygon( shell, holes, nHoles );

  if ( holes )
    delete [] holes;

  return geom;
}

static GEOSGeometry *createGeosPolygon( GEOSGeometry *shell )
{
  return createGeosPolygon( QVector<GEOSGeometry*>() << shell );
}

static GEOSGeometry *createGeosPolygon( const QgsPolygon& polygon )
{
  if ( polygon.count() == 0 )
    return 0;

  QVector<GEOSGeometry *> geoms;

  try
  {
    for ( int i = 0; i < polygon.count(); i++ )
      geoms << createGeosLinearRing( polygon[i] );

    return createGeosPolygon( geoms );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    for ( int i = 0; i < geoms.count(); i++ )
      GEOSGeom_destroy( geoms[i] );
    return 0;
  }
}

static QgsGeometry *fromGeosGeom( GEOSGeometry *geom )
{
  if ( !geom )
    return 0;

  QgsGeometry *g = new QgsGeometry;
  g->fromGeos( geom );
  return g;
}

QgsGeometry* QgsGeometry::fromWkt( QString wkt )
{
  try
  {
#if defined(GEOS_VERSION_MAJOR) && (GEOS_VERSION_MAJOR>=3)
    GEOSWKTReader *reader = GEOSWKTReader_create();
    QgsGeometry *g = fromGeosGeom( GEOSWKTReader_read( reader, wkt.toLocal8Bit().data() ) );
    GEOSWKTReader_destroy( reader );
    return g;
#else
    return fromGeosGeom( GEOSGeomFromWKT( wkt.toLocal8Bit().data() ) );
#endif
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 0;
  }
}

QgsGeometry* QgsGeometry::fromPoint( const QgsPoint& point )
{
  return fromGeosGeom( createGeosPoint( point ) );
}

QgsGeometry* QgsGeometry::fromPolyline( const QgsPolyline& polyline )
{
  return fromGeosGeom( createGeosLineString( polyline ) );
}

QgsGeometry* QgsGeometry::fromPolygon( const QgsPolygon& polygon )
{
  return fromGeosGeom( createGeosPolygon( polygon ) );
}

QgsGeometry* QgsGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  QVector<GEOSGeometry *> geoms;

  try
  {
    for ( int i = 0; i < multipoint.size(); ++i )
      geoms << createGeosPoint( multipoint[i] );

    return fromGeosGeom( createGeosCollection( GEOS_MULTIPOINT, geoms ) );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

    for ( int i = 0; i < geoms.size(); ++i )
      GEOSGeom_destroy( geoms[i] );

    return 0;
  }
}

QgsGeometry* QgsGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QVector<GEOSGeometry *> geoms;

  try
  {
    for ( int i = 0; i < multiline.count(); i++ )
      geoms << createGeosLineString( multiline[i] );

    return fromGeosGeom( createGeosCollection( GEOS_MULTILINESTRING, geoms ) );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

    for ( int i = 0; i < geoms.count(); i++ )
      GEOSGeom_destroy( geoms[i] );

    return 0;
  }
}

QgsGeometry* QgsGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  if ( multipoly.count() == 0 )
    return 0;

  QVector<GEOSGeometry *> geoms;

  try
  {
    for ( int i = 0; i < multipoly.count(); i++ )
      geoms << createGeosPolygon( multipoly[i] );

    return fromGeosGeom( createGeosCollection( GEOS_MULTIPOLYGON, geoms ) );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

    for ( int i = 0; i < geoms.count(); i++ )
      GEOSGeom_destroy( geoms[i] );

    return 0;
  }
}

QgsGeometry* QgsGeometry::fromRect( const QgsRectangle& rect )
{
  QgsPolyline ring;
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );

  QgsPolygon polygon;
  polygon.append( ring );

  return fromPolygon( polygon );
}


QgsGeometry & QgsGeometry::operator=( QgsGeometry const & rhs )
{
  if ( &rhs == this )
    return *this;

  // remove old geometry if it exists
  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  mGeometrySize    = rhs.mGeometrySize;

  // deep-copy the GEOS Geometry if appropriate
  GEOSGeom_destroy( mGeos );
  mGeos = rhs.mGeos ? GEOSGeom_clone( rhs.mGeos ) : 0;

  mDirtyGeos = rhs.mDirtyGeos;
  mDirtyWkb  = rhs.mDirtyWkb;

  if ( mGeometrySize && rhs.mGeometry )
  {
    mGeometry = new unsigned char[mGeometrySize];
    memcpy( mGeometry, rhs.mGeometry, mGeometrySize );
  }

  return *this;
} // QgsGeometry::operator=( QgsGeometry const & rhs )


void QgsGeometry::fromWkb( unsigned char *wkb, size_t length )
{
  // delete any existing WKB geometry before assigning new one
  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  if ( mGeos )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = 0;
  }

  mGeometry = wkb;
  mGeometrySize = length;

  mDirtyWkb   = false;
  mDirtyGeos  = true;
}

const unsigned char *QgsGeometry::asWkb() const
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  return mGeometry;
}

size_t QgsGeometry::wkbSize() const
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  return mGeometrySize;
}

const GEOSGeometry* QgsGeometry::asGeos() const
{
  if ( mDirtyGeos )
  {
    if ( !exportWkbToGeos() )
    {
      return 0;
    }
  }

  return mGeos;
}


QGis::WkbType QgsGeometry::wkbType() const
{
  QgsConstWkbPtr wkbPtr( asWkb() + 1 ); // ensure that wkb representation exists

  if ( mGeometry && wkbSize() >= 5 )
  {
    QGis::WkbType wkbType;
    wkbPtr >> wkbType;
    return wkbType;
  }
  else
  {
    return QGis::WKBUnknown;
  }
}


QGis::GeometryType QgsGeometry::type()
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  switch ( wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      return QGis::Point;

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return QGis::Line;

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return QGis::Polygon;

    default:
      return QGis::UnknownGeometry;
  }
}

bool QgsGeometry::isMultipart()
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  return QGis::isMultiType( wkbType() );
}

void QgsGeometry::fromGeos( GEOSGeometry *geos )
{
  // TODO - make this more heap-friendly

  if ( mGeos )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = 0;
  }

  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  mGeos = geos;

  mDirtyWkb   = true;
  mDirtyGeos  = false;
}

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist )
{
  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return QgsPoint( 0, 0 );
  }

  double actdist = std::numeric_limits<double>::max();

  beforeVertex = -1;
  afterVertex = -1;

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  QgsPoint p;
  bool hasZValue = false;
  int vertexnr = -1;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
    case QGis::WKBPoint:
    {
      double x, y;
      wkbPtr >> x >> y;
      p.set( x, y );
      actdist = point.sqrDist( x, y );
      vertexnr = 0;
      break;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        double dist = point.sqrDist( x, y );
        if ( dist < actdist )
        {
          p.set( x, y );
          actdist = dist;
          vertexnr = index;

          beforeVertex = index - 1;
          afterVertex = index == nPoints - 1 ? -1 : index + 1;
        }
      }
      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;
      for ( int index = 0, pointIndex = 0; index < nRings; ++index )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          double dist = point.sqrDist( x, y );
          if ( dist < actdist )
          {
            p.set( x, y );
            actdist = dist;
            vertexnr = pointIndex;

            // assign the rubberband indices
            if ( index2 == 0 )
            {
              beforeVertex = pointIndex + ( nPoints - 2 );
              afterVertex = pointIndex + 1;
            }
            else if ( index2 == nPoints - 1 )
            {
              beforeVertex = pointIndex - 1;
              afterVertex = pointIndex - ( nPoints - 2 );
            }
            else
            {
              beforeVertex = pointIndex - 1;
              afterVertex = pointIndex + 1;
            }
          }
          ++pointIndex;
        }
      }
      break;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        wkbPtr += 1 + sizeof( int ); // skip endian and point type

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        double dist = point.sqrDist( x, y );
        if ( dist < actdist )
        {
          p.set( x, y );
          actdist = dist;
          vertexnr = index;
        }
      }
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int index = 0, pointIndex = 0; index < nLines; ++index )
      {
        wkbPtr += 1 + sizeof( int );

        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          double dist = point.sqrDist( x, y );
          if ( dist < actdist )
          {
            p.set( x, y );
            actdist = dist;
            vertexnr = pointIndex;

            if ( index2 == 0 )//assign the rubber band indices
              beforeVertex = -1;
            else
              beforeVertex = vertexnr - 1;

            if ( index2 == nPoints - 1 )
              afterVertex = -1;
            else
              afterVertex = vertexnr + 1;

          }
          ++pointIndex;
        }
      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      wkbPtr >> nPolys;
      for ( int index = 0, pointIndex = 0; index < nPolys; ++index )
      {
        wkbPtr += 1 + sizeof( int ); //skip endian and polygon type
        int nRings;
        wkbPtr >> nRings;
        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          int nPoints;
          wkbPtr >> nPoints;
          for ( int index3 = 0; index3 < nPoints; ++index3 )
          {
            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            double dist = point.sqrDist( x, y );
            if ( dist < actdist )
            {
              p.set( x, y );
              actdist = dist;
              vertexnr = pointIndex;

              //assign the rubber band indices
              if ( index3 == 0 )
              {
                beforeVertex = pointIndex + ( nPoints - 2 );
                afterVertex = pointIndex + 1;
              }
              else if ( index3 == nPoints - 1 )
              {
                beforeVertex = pointIndex - 1;
                afterVertex = pointIndex - ( nPoints - 2 );
              }
              else
              {
                beforeVertex = pointIndex - 1;
                afterVertex = pointIndex + 1;
              }
            }
            ++pointIndex;
          }
        }
      }
      break;
    }

    default:
      break;
  }

  sqrDist = actdist;
  atVertex = vertexnr;
  return p;
}

void QgsGeometry::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex )
{
  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  beforeVertex = -1;
  afterVertex = -1;

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return;
  }

  if ( atVertex < 0 )
    return;

  QGis::WkbType wkbType;
  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  wkbPtr >> wkbType;

  switch ( wkbType )
  {
    case QGis::WKBPoint:
    {
      // NOOP - Points do not have adjacent verticies
      break;
    }

    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex >= nPoints )
        return;

      const int index = atVertex;

      // assign the rubber band indices

      beforeVertex = index - 1;

      if ( index == nPoints - 1 )
        afterVertex = -1;
      else
        afterVertex = index + 1;

      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int index0 = 0, pointIndex = 0; index0 < nRings; ++index0 )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index1 = 0; index1 < nPoints; ++index1 )
        {
          wkbPtr += ( hasZValue ? 3 : 2 ) * sizeof( double );

          if ( pointIndex == atVertex )
          {
            if ( index1 == 0 )
            {
              beforeVertex = pointIndex + ( nPoints - 2 );
              afterVertex = pointIndex + 1;
            }
            else if ( index1 == nPoints - 1 )
            {
              beforeVertex = pointIndex - 1;
              afterVertex = pointIndex - ( nPoints - 2 );
            }
            else
            {
              beforeVertex = pointIndex - 1;
              afterVertex = pointIndex + 1;
            }
          }

          ++pointIndex;
        }
      }
      break;
    }

    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      // NOOP - Points do not have adjacent verticies
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int index0 = 0, pointIndex = 0; index0 < nLines; ++index0 )
      {
        wkbPtr += 1 + sizeof( int );

        int nPoints;
        wkbPtr >> nPoints;

        for ( int index1 = 0; index1 < nPoints; ++index1 )
        {
          wkbPtr += ( hasZValue ? 3 : 2 ) * sizeof( double );

          if ( pointIndex == atVertex )
          {
            // Found the vertex of the linestring we were looking for.
            if ( index1 == 0 )
              beforeVertex = -1;
            else
              beforeVertex = pointIndex - 1;

            if ( index1 == nPoints - 1 )
              afterVertex = -1;
            else
              afterVertex = pointIndex + 1;
          }

          ++pointIndex;
        }
      }

      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      wkbPtr >> nPolys;
      for ( int index0 = 0, pointIndex = 0; index0 < nPolys; ++index0 )
      {
        wkbPtr += 1 + sizeof( int ); //skip endian and polygon type
        int nRings;
        wkbPtr >> nRings;

        for ( int index1 = 0; index1 < nRings; ++index1 )
        {
          int nPoints;
          wkbPtr >> nPoints;
          for ( int index2 = 0; index2 < nPoints; ++index2 )
          {
            wkbPtr += ( hasZValue ? 3 : 2 ) * sizeof( double );

            if ( pointIndex == atVertex )
            {
              // Found the vertex of the linear-ring of the polygon we were looking for.
              // assign the rubber band indices

              if ( index2 == 0 )
              {
                beforeVertex = pointIndex + ( nPoints - 2 );
                afterVertex = pointIndex + 1;
              }
              else if ( index2 == nPoints - 1 )
              {
                beforeVertex = pointIndex - 1;
                afterVertex = pointIndex - ( nPoints - 2 );
              }
              else
              {
                beforeVertex = pointIndex - 1;
                afterVertex = pointIndex + 1;
              }
            }
            ++pointIndex;
          }
        }
      }

      break;
    }

    default:
      break;
  } // switch (wkbType)
}

bool QgsGeometry::insertVertex( double x, double y,
                                int beforeVertex,
                                const GEOSCoordSequence *old_sequence,
                                GEOSCoordSequence **new_sequence )
{
  // Bounds checking
  if ( beforeVertex < 0 )
  {
    *new_sequence = 0;
    return false;
  }

  unsigned int numPoints;
  GEOSCoordSeq_getSize( old_sequence, &numPoints );

  *new_sequence = GEOSCoordSeq_create( numPoints + 1, 2 );
  if ( !*new_sequence )
    return false;

  bool inserted = false;
  for ( unsigned int i = 0, j = 0; i < numPoints; i++, j++ )
  {
    // Do we insert the new vertex here?
    if ( beforeVertex == static_cast<int>( i ) )
    {
      GEOSCoordSeq_setX( *new_sequence, j, x );
      GEOSCoordSeq_setY( *new_sequence, j, y );
      j++;
      inserted = true;
    }

    double aX, aY;
    GEOSCoordSeq_getX( old_sequence, i, &aX );
    GEOSCoordSeq_getY( old_sequence, i, &aY );

    GEOSCoordSeq_setX( *new_sequence, j, aX );
    GEOSCoordSeq_setY( *new_sequence, j, aY );
  }

  if ( !inserted )
  {
    // The beforeVertex is greater than the actual number of vertices
    // in the geometry - append it.
    GEOSCoordSeq_setX( *new_sequence, numPoints, x );
    GEOSCoordSeq_setY( *new_sequence, numPoints, y );
  }

  // TODO: Check that the sequence is still simple, e.g. with GEOS_GEOM::Geometry->isSimple()

  return inserted;
}

bool QgsGeometry::moveVertex( QgsWkbPtr &wkbPtr, const double &x, const double &y, int atVertex, bool hasZValue, int &pointIndex, bool isRing )
{
  int nPoints;
  wkbPtr >> nPoints;

  const int ps = ( hasZValue ? 3 : 2 ) * sizeof( double );

  // Not this linestring/ring?
  if ( atVertex >= pointIndex + nPoints )
  {
    wkbPtr += ps * nPoints;
    pointIndex += nPoints;
    return false;
  }

  if ( isRing && atVertex == pointIndex + nPoints - 1 )
    atVertex = pointIndex;

  // Goto point in this linestring/ring
  wkbPtr += ps * ( atVertex - pointIndex );
  wkbPtr << x << y;
  if ( hasZValue )
    wkbPtr << 0.0;

  if ( isRing && atVertex == pointIndex )
  {
    wkbPtr += ps * ( nPoints - 2 );
    wkbPtr << x << y;
    if ( hasZValue )
      wkbPtr << 0.0;
  }

  return true;
}

bool QgsGeometry::moveVertex( double x, double y, int atVertex )
{
  if ( atVertex < 0 )
    return false;

  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return false;
  }

  QGis::WkbType wkbType;
  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  wkbPtr >> wkbType;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
    case QGis::WKBPoint:
    {
      if ( atVertex != 0 )
        return false;

      wkbPtr << x << y;
      mDirtyGeos = true;
      return true;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int pointIndex = 0;
      if ( moveVertex( wkbPtr, x, y, atVertex, hasZValue, pointIndex, false ) )
      {
        mDirtyGeos = true;
        return true;
      }

      return false;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex < nPoints )
      {
        wkbPtr += atVertex * ( 1 + sizeof( int ) + ( hasZValue ? 3 : 2 ) * sizeof( double ) ) + 1 + sizeof( int );
        wkbPtr << x << y;
        if ( hasZValue )
          wkbPtr << 0.0;
        mDirtyGeos = true;
        return true;
      }
      else
      {
        return false;
      }
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        if ( moveVertex( wkbPtr, x, y, atVertex, hasZValue, pointIndex, false ) )
        {
          mDirtyGeos = true;
          return true;
        }
      }

      return false;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nLines;
      wkbPtr >> nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        if ( moveVertex( wkbPtr, x, y, atVertex, hasZValue, pointIndex, true ) )
        {
          mDirtyGeos = true;
          return true;
        }
      }
      return false;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int ); // skip endian and polygon type

        int nRings;
        wkbPtr >> nRings;

        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          if ( moveVertex( wkbPtr, x, y, atVertex, hasZValue, pointIndex, true ) )
          {
            mDirtyGeos = true;
            return true;
          }
        }
      }
      return false;
    }

    default:
      return false;
  }
}

// copy vertices from srcPtr to dstPtr and skip/delete one vertex
// @param srcPtr ring/part starting with number of points (adjusted in each call)
// @param dstPtr ring/part to copy to (adjusted in each call)
// @param atVertex index of vertex to skip
// @param hasZValue points have 3 elements
// @param pointIndex reference to index of first ring/part vertex in overall object (adjusted in each call)
// @param isRing srcPtr points to a ring
// @param lastItem last ring/part, atVertex after this one must be wrong
// @return
//   0 no delete was done
//   1 "normal" delete was done
//   2 last element of the ring/part was deleted
int QgsGeometry::deleteVertex( QgsConstWkbPtr &srcPtr, QgsWkbPtr &dstPtr, int atVertex, bool hasZValue, int &pointIndex, bool isRing, bool lastItem )
{
  QgsDebugMsg( QString( "atVertex:%1 hasZValue:%2 pointIndex:%3 isRing:%4" ).arg( atVertex ).arg( hasZValue ).arg( pointIndex ).arg( isRing ) );
  const int ps = ( hasZValue ? 3 : 2 ) * sizeof( double );
  int nPoints;
  srcPtr >> nPoints;

  // copy complete ring/part if vertex is in a following one
  if ( atVertex < pointIndex || atVertex >= pointIndex + nPoints )
  {
    // atVertex does not exist
    if ( lastItem && atVertex >= pointIndex + nPoints )
      return 0;

    dstPtr << nPoints;

    int len = nPoints * ps;
    memcpy( dstPtr, srcPtr, len );
    dstPtr += len;
    srcPtr += len;
    pointIndex += nPoints;
    return 0;
  }

  // delete the first vertex of a ring instead of the last
  if ( isRing && atVertex == pointIndex + nPoints - 1 )
    atVertex = pointIndex;

  if ( nPoints == ( isRing ? 2 : 1 ) )
  {
    // last point of the part/ring is deleted
    // skip the whole part/ring
    srcPtr += nPoints * ps;
    pointIndex += nPoints;
    return 2;
  }

  dstPtr << nPoints - 1;

  // copy ring before vertex
  int len = ( atVertex - pointIndex ) * ps;
  if ( len > 0 )
  {
    memcpy( dstPtr, srcPtr, len );
    dstPtr += len;
    srcPtr += len;
  }

  // skip deleted vertex
  srcPtr += ps;

  // copy reset of ring
  len = ( pointIndex + nPoints - atVertex - 1 ) * ps;

  // save position of vertex, if we delete the first vertex of a ring
  const unsigned char *first = 0;
  if ( isRing && atVertex == pointIndex )
  {
    len -= ps;
    first = srcPtr;
  }

  if ( len > 0 )
  {
    memcpy( dstPtr, srcPtr, len );
    dstPtr += len;
    srcPtr += len;
  }

  // copy new first vertex instead of the old last, if we deleted the original first vertex
  if ( first )
  {
    memcpy( dstPtr, first, ps );
    dstPtr += ps;
    srcPtr += ps;
  }

  pointIndex += nPoints;

  return 1;
}

bool QgsGeometry::deleteVertex( int atVertex )
{
  QgsDebugMsg( QString( "atVertex:%1" ).arg( atVertex ) );
  if ( atVertex < 0 )
    return false;

  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return false;
  }

  QgsConstWkbPtr srcPtr( mGeometry );
  char endianness;
  QGis::WkbType wkbType;
  srcPtr >> endianness >> wkbType;

  bool hasZValue = QGis::wkbDimensions( wkbType ) == 3;

  int ps = ( hasZValue ? 3 : 2 ) * sizeof( double );
  if ( QGis::flatType( wkbType ) == QGis::WKBMultiPoint )
    ps += 1 + sizeof( int );

  unsigned char *dstBuffer = new unsigned char[mGeometrySize - ps];
  QgsWkbPtr dstPtr( dstBuffer );
  dstPtr << endianness << wkbType;

  bool deleted = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      break; //cannot remove the only point vertex

    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    {
      int pointIndex = 0;
      int res = deleteVertex( srcPtr, dstPtr, atVertex, hasZValue, pointIndex, false, true );
      if ( res == 2 )
      {
        // Linestring with 0 points
        dstPtr << 0;
      }

      deleted = res != 0;
      break;
    }

    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      int nRings;
      srcPtr >> nRings;
      QgsWkbPtr ptrN( dstPtr );
      dstPtr << nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )
      {
        int res = deleteVertex( srcPtr, dstPtr, atVertex, hasZValue, pointIndex, true, ringnr == nRings - 1 );
        if ( res == 2 )
          ptrN << nRings - 1;

        deleted |= res != 0;
      }

      break;
    }

    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      srcPtr >> nPoints;

      if ( atVertex < nPoints )
      {
        dstPtr << nPoints - 1;

        int len = ps * atVertex;
        if ( len > 0 )
        {
          memcpy( dstPtr, srcPtr, len );
          srcPtr += len;
          dstPtr += len;
        }

        srcPtr += ps;

        len = ps * ( nPoints - atVertex - 1 );
        if ( len > 0 )
        {
          memcpy( dstPtr, srcPtr, len );
          srcPtr += len;
          dstPtr += len;
        }

        deleted = true;
      }

      break;
    }

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      int nLines;
      srcPtr >> nLines;
      QgsWkbPtr ptrN( dstPtr );
      dstPtr << nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        QgsWkbPtr saveDstPtr( dstPtr );
        srcPtr >> endianness >> wkbType;
        dstPtr << endianness << wkbType;

        int res = deleteVertex( srcPtr, dstPtr, atVertex, hasZValue, pointIndex, false, linenr == nLines - 1 );
        if ( res == 2 )
        {
          // line string was completely removed
          ptrN << nLines - 1;
          dstPtr = saveDstPtr;
        }

        deleted |= res != 0;
      }

      break;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      srcPtr >> nPolys;
      QgsWkbPtr ptrNPolys( dstPtr );
      dstPtr << nPolys;

      for ( int polynr = 0, pointIndex = 0; polynr < nPolys; ++polynr )
      {
        int nRings;
        srcPtr >> endianness >> wkbType >> nRings;
        QgsWkbPtr saveDstPolyPtr( dstPtr );
        dstPtr << endianness << wkbType;
        QgsWkbPtr ptrNRings( dstPtr );
        dstPtr << nRings;

        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int res = deleteVertex( srcPtr, dstPtr, atVertex, hasZValue, pointIndex, true, polynr == nPolys - 1 && ringnr == nRings - 1 );
          if ( res == 2 )
          {
            // ring was completely removed
            if ( nRings == 1 )
            {
              // last ring => remove polygon
              ptrNPolys << nPolys - 1;
              dstPtr = saveDstPolyPtr;
            }
            else
            {
              ptrNRings << nRings - 1;
            }
          }

          deleted |= res != 0;
        }
      }
      break;
    }

    case QGis::WKBNoGeometry:
    case QGis::WKBUnknown:
      break;
  }

  if ( deleted )
  {
    delete [] mGeometry;
    mGeometry = dstBuffer;
    mGeometrySize -= ps;
    mDirtyGeos = true;
    return true;
  }
  else
  {
    delete [] dstBuffer;
    return false;
  }
}

bool QgsGeometry::insertVertex( QgsConstWkbPtr &srcPtr, QgsWkbPtr &dstPtr, int beforeVertex, const double &x, const double &y, bool hasZValue, int &pointIndex, bool isRing )
{
  int nPoints;
  srcPtr >> nPoints;

  bool insertHere = beforeVertex >= pointIndex && beforeVertex < pointIndex + nPoints;

  int len;
  if ( insertHere )
  {
    dstPtr << nPoints + 1;
    len = ( hasZValue ? 3 : 2 ) * ( beforeVertex - pointIndex ) * sizeof( double );
    if ( len > 0 )
    {
      memcpy( dstPtr, srcPtr, len );
      srcPtr += len;
      dstPtr += len;
    }

    dstPtr << x << y;
    if ( hasZValue )
      dstPtr << 0.0;

    len = ( hasZValue ? 3 : 2 ) * ( pointIndex + nPoints - beforeVertex ) * sizeof( double );
    if ( isRing && beforeVertex == pointIndex )
      len -= ( hasZValue ? 3 : 2 ) * sizeof( double );
  }
  else
  {
    dstPtr << nPoints;
    len = ( hasZValue ? 3 : 2 ) * nPoints * sizeof( double );
  }

  memcpy( dstPtr, srcPtr, len );
  srcPtr += len;
  dstPtr += len;

  if ( isRing && beforeVertex == pointIndex )
  {
    dstPtr << x << y;
    if ( hasZValue )
      dstPtr << 0.0;
  }

  pointIndex += nPoints;
  return insertHere;
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return false;
  }

  if ( beforeVertex < 0 )
    return false;

  QgsConstWkbPtr srcPtr( mGeometry );
  char endianness;
  QGis::WkbType wkbType;
  srcPtr >> endianness >> wkbType;

  bool hasZValue = QGis::wkbDimensions( wkbType ) == 3;

  int ps = ( hasZValue ? 3 : 2 ) * sizeof( double );
  if ( QGis::flatType( wkbType ) == QGis::WKBMultiPoint )
    ps += 1 + sizeof( int );

  unsigned char *dstBuffer = new unsigned char[mGeometrySize + ps];
  QgsWkbPtr dstPtr( dstBuffer );
  dstPtr << endianness << wkbType;

  bool inserted = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint: //cannot insert a vertex before another one on point types
      break;

    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    {
      int pointIndex = 0;
      inserted = insertVertex( srcPtr, dstPtr, beforeVertex, x, y, hasZValue, pointIndex, false );
      break;
    }

    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      int nRings;
      srcPtr >> nRings;
      dstPtr << nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )
        inserted |= insertVertex( srcPtr, dstPtr, beforeVertex, x, y, hasZValue, pointIndex, true );

      break;
    }

    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      srcPtr >> nPoints;

      if ( beforeVertex <= nPoints )
      {
        dstPtr << nPoints + 1;

        int len = ps * beforeVertex;
        if ( len > 0 )
        {
          memcpy( dstPtr, srcPtr, len );
          srcPtr += len;
          dstPtr += len;
        }

        dstPtr << endianness << ( hasZValue ? QGis::WKBPoint25D : QGis::WKBPoint ) << x << y;
        if ( hasZValue )
          dstPtr << 0.0;

        len = ps * ( nPoints - beforeVertex );
        if ( len > 0 )
          memcpy( dstPtr, srcPtr, len );

        inserted = true;
      }

      break;
    }

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      int nLines;
      srcPtr >> nLines;
      dstPtr << nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        srcPtr >> endianness >> wkbType;
        dstPtr << endianness << wkbType;
        inserted |= insertVertex( srcPtr, dstPtr, beforeVertex, x, y, hasZValue, pointIndex, false );
      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      srcPtr >> nPolys;
      dstPtr << nPolys;

      for ( int polynr = 0, pointIndex = 0; polynr < nPolys; ++polynr )
      {
        int nRings;
        srcPtr >> endianness >> wkbType >> nRings;
        dstPtr << endianness << wkbType << nRings;

        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
          inserted |= insertVertex( srcPtr, dstPtr, beforeVertex, x, y, hasZValue, pointIndex, true );
      }
      break;
    }

    case QGis::WKBNoGeometry:
    case QGis::WKBUnknown:
      break;
  }

  if ( inserted )
  {
    delete [] mGeometry;
    mGeometry = dstBuffer;
    mGeometrySize += ps;
    mDirtyGeos = true;
    return true;
  }
  else
  {
    delete [] dstBuffer;
    return false;
  }
}

QgsPoint QgsGeometry::vertexAt( int atVertex )
{
  if ( atVertex < 0 )
    return QgsPoint( 0, 0 );

  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return QgsPoint( 0, 0 );
  }

  QgsConstWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      if ( atVertex != 0 )
        return QgsPoint( 0, 0 );

      double x, y;
      wkbPtr >> x >> y;

      return QgsPoint( x, y );
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex >= nPoints )
        return QgsPoint( 0, 0 );

      // copy the vertex coordinates
      wkbPtr += atVertex * ( hasZValue ? 3 : 2 ) * sizeof( double );

      double x, y;
      wkbPtr >> x >> y;

      return QgsPoint( x, y );
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )
      {
        int nPoints;
        wkbPtr >> nPoints;

        if ( atVertex >= pointIndex + nPoints )
        {
          wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
          pointIndex += nPoints;
          continue;
        }

        wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

        double x, y;
        wkbPtr >> x >> y;
        return QgsPoint( x, y );
      }

      return QgsPoint( 0, 0 );
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex >= nPoints )
        return QgsPoint( 0, 0 );

      wkbPtr += atVertex * ( 1 + sizeof( int ) + ( hasZValue ? 3 : 2 ) * sizeof( double ) ) + 1 + sizeof( int );

      double x, y;
      wkbPtr >> x >> y;
      return QgsPoint( x, y );
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;

        if ( atVertex >= pointIndex + nPoints )
        {
          wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
          pointIndex += nPoints;
          continue;
        }

        wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

        double x, y;
        wkbPtr >> x >> y;

        return QgsPoint( x, y );
      }

      return QgsPoint( 0, 0 );
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;

      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int );

        int nRings;
        wkbPtr >> nRings;
        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int nPoints;
          wkbPtr >> nPoints;

          if ( atVertex >= pointIndex + nPoints )
          {
            wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
            pointIndex += nPoints;
            continue;
          }

          wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

          double x, y;
          wkbPtr >> x >> y;

          return QgsPoint( x, y );
        }
      }
      return QgsPoint( 0, 0 );
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QgsPoint( 0, 0 );
  }
}

double QgsGeometry::sqrDistToVertexAt( QgsPoint& point, int atVertex )
{
  QgsPoint pnt = vertexAt( atVertex );
  if ( pnt != QgsPoint( 0, 0 ) )
  {
    QgsDebugMsg( "Exiting with distance to " + pnt.toString() );
    return point.sqrDist( pnt );
  }
  else
  {
    QgsDebugMsg( "Exiting with std::numeric_limits<double>::max()." );
    // probably safest to bail out with a very large number
    return std::numeric_limits<double>::max();
  }
}

double QgsGeometry::closestVertexWithContext( const QgsPoint& point, int& atVertex )
{
  double sqrDist = std::numeric_limits<double>::max();

  try
  {
    // Initialise some stuff
    int closestVertexIndex = 0;

    // set up the GEOS geometry
    if ( mDirtyGeos )
      exportWkbToGeos();

    if ( !mGeos )
      return -1;

    const GEOSGeometry *g = GEOSGetExteriorRing( mGeos );
    if ( !g )
      return -1;

    const GEOSCoordSequence *sequence = GEOSGeom_getCoordSeq( g );

    unsigned int n;
    GEOSCoordSeq_getSize( sequence, &n );

    for ( unsigned int i = 0; i < n; i++ )
    {
      double x, y;
      GEOSCoordSeq_getX( sequence, i, &x );
      GEOSCoordSeq_getY( sequence, i, &y );

      double testDist = point.sqrDist( x, y );
      if ( testDist < sqrDist )
      {
        closestVertexIndex = i;
        sqrDist = testDist;
      }
    }

    atVertex = closestVertexIndex;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return -1;
  }

  return sqrDist;
}

double QgsGeometry::closestSegmentWithContext(
  const QgsPoint& point,
  QgsPoint& minDistPoint,
  int& afterVertex,
  double *leftOf,
  double epsilon )
{
  QgsDebugMsg( "Entering." );

  // TODO: implement with GEOS
  if ( mDirtyWkb ) //convert latest geos to mGeometry
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return -1;
  }

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  // Initialise some stuff
  double sqrDist = std::numeric_limits<double>::max();

  QgsPoint distPoint;
  int closestSegmentIndex = 0;
  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      // Points have no lines
      return -1;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      double prevx = 0.0, prevy = 0.0;
      for ( int index = 0; index < nPoints; ++index )
      {
        double thisx, thisy;
        wkbPtr >> thisx >> thisy;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( index > 0 )
        {
          double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
          if ( testdist < sqrDist )
          {
            closestSegmentIndex = index;
            sqrDist = testdist;
            minDistPoint = distPoint;
            if ( leftOf )
            {
              *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
            }
          }
        }

        prevx = thisx;
        prevy = thisy;
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
            if ( testdist < sqrDist )
            {
              closestSegmentIndex = pointIndex;
              sqrDist = testdist;
              minDistPoint = distPoint;
              if ( leftOf )
              {
                *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
              }
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )//loop over rings
      {
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )//loop over points in a ring
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
            if ( testdist < sqrDist )
            {
              closestSegmentIndex = pointIndex;
              sqrDist = testdist;
              minDistPoint = distPoint;
              if ( leftOf )
              {
                *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
              }
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int nPoints;
          wkbPtr >> nPoints;

          double prevx = 0.0, prevy = 0.0;
          for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
          {
            double thisx, thisy;
            wkbPtr >> thisx >> thisy;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            if ( pointnr > 0 )
            {
              double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
              if ( testdist < sqrDist )
              {
                closestSegmentIndex = pointIndex;
                sqrDist = testdist;
                minDistPoint = distPoint;
                if ( leftOf )
                {
                  *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
                }
              }
            }

            prevx = thisx;
            prevy = thisy;
            ++pointIndex;
          }
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBUnknown:
    default:
      return -1;
      break;
  } // switch (wkbType)

  QgsDebugMsg( QString( "Exiting with nearest point %1, dist %2." )
               .arg( point.toString() ).arg( sqrDist ) );

  return sqrDist;
}

int QgsGeometry::addRing( const QList<QgsPoint>& ring )
{
  //bail out if this geometry is not polygon/multipolygon
  if ( type() != QGis::Polygon )
    return 1;

  //test for invalid geometries
  if ( ring.size() < 4 )
    return 3;

  //ring must be closed
  if ( ring.first() != ring.last() )
    return 2;

  //create geos geometry from wkb if not already there
  if ( mDirtyGeos )
  {
    exportWkbToGeos();
  }

  if ( !mGeos )
  {
    return 6;
  }

  int type = GEOSGeomTypeId( mGeos );

  //Fill GEOS Polygons of the feature into list
  QVector<const GEOSGeometry*> polygonList;

  if ( wkbType() == QGis::WKBPolygon )
  {
    if ( type != GEOS_POLYGON )
      return 1;

    polygonList << mGeos;
  }
  else if ( wkbType() == QGis::WKBMultiPolygon )
  {
    if ( type != GEOS_MULTIPOLYGON )
      return 1;

    for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); ++i )
      polygonList << GEOSGetGeometryN( mGeos, i );
  }

  //create new ring
  GEOSGeometry *newRing = 0;
  GEOSGeometry *newRingPolygon = 0;

  try
  {
    newRing = createGeosLinearRing( ring.toVector() );
    if ( !GEOSisValid( newRing ) )
    {
      throwGEOSException( "ring is invalid" );
    }

    newRingPolygon = createGeosPolygon( newRing );
    if ( !GEOSisValid( newRingPolygon ) )
    {
      throwGEOSException( "ring is invalid" );
    }
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

    if ( newRingPolygon )
      GEOSGeom_destroy( newRingPolygon );
    else if ( newRing )
      GEOSGeom_destroy( newRing );

    return 3;
  }

  QVector<GEOSGeometry*> rings;

  int i;
  for ( i = 0; i < polygonList.size(); i++ )
  {
    for ( int j = 0; j < rings.size(); j++ )
      GEOSGeom_destroy( rings[j] );
    rings.clear();

    GEOSGeometry *shellRing = 0;
    GEOSGeometry *shell = 0;
    try
    {
      shellRing = GEOSGeom_clone( GEOSGetExteriorRing( polygonList[i] ) );
      shell = createGeosPolygon( shellRing );

      if ( !GEOSWithin( newRingPolygon, shell ) )
      {
        GEOSGeom_destroy( shell );
        continue;
      }
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

      if ( shell )
        GEOSGeom_destroy( shell );
      else if ( shellRing )
        GEOSGeom_destroy( shellRing );

      GEOSGeom_destroy( newRingPolygon );

      return 4;
    }

    // add outer ring
    rings << GEOSGeom_clone( shellRing );

    GEOSGeom_destroy( shell );

    // check inner rings
    int n = GEOSGetNumInteriorRings( polygonList[i] );

    int j;
    for ( j = 0; j < n; j++ )
    {
      GEOSGeometry *holeRing = 0;
      GEOSGeometry *hole = 0;
      try
      {
        holeRing = GEOSGeom_clone( GEOSGetInteriorRingN( polygonList[i], j ) );
        hole = createGeosPolygon( holeRing );

        if ( !GEOSDisjoint( hole, newRingPolygon ) )
        {
          GEOSGeom_destroy( hole );
          break;
        }
      }
      catch ( GEOSException &e )
      {
        QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

        if ( hole )
          GEOSGeom_destroy( hole );
        else if ( holeRing )
          GEOSGeom_destroy( holeRing );

        break;
      }

      rings << GEOSGeom_clone( holeRing );
      GEOSGeom_destroy( hole );
    }

    if ( j == n )
      // this is it...
      break;
  }

  if ( i == polygonList.size() )
  {
    // clear rings
    for ( int j = 0; j < rings.size(); j++ )
      GEOSGeom_destroy( rings[j] );
    rings.clear();

    GEOSGeom_destroy( newRingPolygon );

    // no containing polygon found
    return 5;
  }

  rings << GEOSGeom_clone( newRing );
  GEOSGeom_destroy( newRingPolygon );

  GEOSGeometry *newPolygon = createGeosPolygon( rings );

  if ( wkbType() == QGis::WKBPolygon )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = newPolygon;
  }
  else if ( wkbType() == QGis::WKBMultiPolygon )
  {
    QVector<GEOSGeometry*> newPolygons;

    for ( int j = 0; j < polygonList.size(); j++ )
    {
      newPolygons << ( i == j ? newPolygon : GEOSGeom_clone( polygonList[j] ) );
    }

    GEOSGeom_destroy( mGeos );
    mGeos = createGeosCollection( GEOS_MULTIPOLYGON, newPolygons );
  }

  mDirtyWkb = true;
  mDirtyGeos = false;
  return 0;
}

int QgsGeometry::addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType )
{
  if ( geomType == QGis::UnknownGeometry )
  {
    geomType = type();
  }

  switch ( geomType )
  {
    case QGis::Point:
      // only one part at a time
      if ( points.size() != 1 )
      {
        QgsDebugMsg( "expected 1 point: " + QString::number( points.size() ) );
        return 2;
      }
      break;

    case QGis::Line:
      // line needs to have at least two points
      if ( points.size() < 2 )
      {
        QgsDebugMsg( "line must at least have two points: " + QString::number( points.size() ) );
        return 2;
      }
      break;

    case QGis::Polygon:
      // polygon needs to have at least three distinct points and must be closed
      if ( points.size() < 4 )
      {
        QgsDebugMsg( "polygon must at least have three distinct points and must be closed: " + QString::number( points.size() ) );
        return 2;
      }

      // Polygon must be closed
      if ( points.first() != points.last() )
      {
        QgsDebugMsg( "polygon not closed" );
        return 2;
      }
      break;

    default:
      QgsDebugMsg( "unsupported geometry type: " + QString::number( geomType ) );
      return 2;
  }

  GEOSGeometry *newPart = 0;

  switch ( geomType )
  {
    case QGis::Point:
      newPart = createGeosPoint( points[0] );
      break;

    case QGis::Line:
      newPart = createGeosLineString( points.toVector() );
      break;

    case QGis::Polygon:
    {
      //create new polygon from ring
      GEOSGeometry *newRing = 0;

      try
      {
        newRing = createGeosLinearRing( points.toVector() );
        if ( !GEOSisValid( newRing ) )
          throw GEOSException( "ring invalid" );

        newPart = createGeosPolygon( newRing );
      }
      catch ( GEOSException &e )
      {
        QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

        if ( newRing )
          GEOSGeom_destroy( newRing );

        return 2;
      }
    }
    break;

    default:
      QgsDebugMsg( "unsupported type: " + QString::number( type() ) );
      return 2;
  }

  if ( type() == QGis::UnknownGeometry )
  {
    fromGeos( newPart );
    return 0;
  }
  return addPart( newPart );
}

int QgsGeometry::addPart( QgsGeometry *newPart )
{
  if ( !newPart )
    return 4;

  const GEOSGeometry * geosPart = newPart->asGeos();
  return addPart( GEOSGeom_clone( geosPart ) );
}

int QgsGeometry::addPart( GEOSGeometry *newPart )
{
  QGis::GeometryType geomType = type();

  if ( !isMultipart() && !convertToMultiType() )
  {
    QgsDebugMsg( "could not convert to multipart" );
    return 1;
  }

  //create geos geometry from wkb if not already there
  if ( mDirtyGeos )
  {
    exportWkbToGeos();
  }

  if ( !mGeos )
  {
    QgsDebugMsg( "GEOS geometry not available!" );
    return 4;
  }

  int geosType = GEOSGeomTypeId( mGeos );

  Q_ASSERT( newPart );

  try
  {
    if ( !GEOSisValid( newPart ) )
      throw GEOSException( "new part geometry invalid" );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );

    if ( newPart )
      GEOSGeom_destroy( newPart );

    QgsDebugMsg( "part invalid: " + e.what() );
    return 2;
  }

  QVector<GEOSGeometry*> parts;

  //create new multipolygon
  int n = GEOSGetNumGeometries( mGeos );
  int i;
  for ( i = 0; i < n; ++i )
  {
    const GEOSGeometry *partN = GEOSGetGeometryN( mGeos, i );

    if ( geomType == QGis::Polygon && GEOSOverlaps( partN, newPart ) )
      //bail out if new polygon overlaps with existing ones
      break;

    parts << GEOSGeom_clone( partN );
  }

  if ( i < n )
  {
    // bailed out
    for ( int i = 0; i < parts.size(); i++ )
      GEOSGeom_destroy( parts[i] );

    QgsDebugMsg( "new polygon part overlaps" );
    return 3;
  }

  int nPartGeoms = GEOSGetNumGeometries( newPart );
  for ( int i = 0; i < nPartGeoms; ++i )
  {
    parts << GEOSGeom_clone( GEOSGetGeometryN( newPart, i ) );
  }
  GEOSGeom_destroy( newPart );

  GEOSGeom_destroy( mGeos );

  mGeos = createGeosCollection( geosType, parts );

  mDirtyWkb = true;
  mDirtyGeos = false;

  return 0;
}

int QgsGeometry::translate( double dx, double dy )
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return 1;
  }

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      translateVertex( wkbPtr, dx, dy, hasZValue );
    }
    break;

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
        translateVertex( wkbPtr, dx, dy, hasZValue );

      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;
      for ( int index = 0; index < nRings; ++index )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
          translateVertex( wkbPtr, dx, dy, hasZValue );

      }
      break;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;

      for ( int index = 0; index < nPoints; ++index )
      {
        wkbPtr += 1 + sizeof( int );
        translateVertex( wkbPtr, dx, dy, hasZValue );
      }
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int index = 0; index < nLines; ++index )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
          translateVertex( wkbPtr, dx, dy, hasZValue );
      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      wkbPtr >> nPolys;
      for ( int index = 0; index < nPolys; ++index )
      {
        wkbPtr += 1 + sizeof( int ); //skip endian and polygon type

        int nRings;
        wkbPtr >> nRings;

        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          int nPoints;
          wkbPtr >> nPoints;
          for ( int index3 = 0; index3 < nPoints; ++index3 )
            translateVertex( wkbPtr, dx, dy, hasZValue );
        }
      }
      break;
    }

    default:
      break;
  }
  mDirtyGeos = true;
  return 0;
}

int QgsGeometry::transform( const QgsCoordinateTransform& ct )
{
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return 1;
  }

  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      transformVertex( wkbPtr, ct, hasZValue );
    }
    break;

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
        transformVertex( wkbPtr, ct, hasZValue );

      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;
      for ( int index = 0; index < nRings; ++index )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
          transformVertex( wkbPtr, ct, hasZValue );

      }
      break;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        wkbPtr += 1 + sizeof( int );
        transformVertex( wkbPtr, ct, hasZValue );
      }
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int index = 0; index < nLines; ++index )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
          transformVertex( wkbPtr, ct, hasZValue );

      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      wkbPtr >> nPolys;
      for ( int index = 0; index < nPolys; ++index )
      {
        wkbPtr += 1 + sizeof( int ); //skip endian and polygon type
        int nRings;
        wkbPtr >> nRings;
        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          int nPoints;
          wkbPtr >> nPoints;
          for ( int index3 = 0; index3 < nPoints; ++index3 )
            transformVertex( wkbPtr, ct, hasZValue );

        }
      }
    }

    default:
      break;
  }
  mDirtyGeos = true;
  return 0;
}

int QgsGeometry::splitGeometry( const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries, bool topological, QList<QgsPoint> &topologyTestPoints )
{
  int returnCode = 0;

  //return if this type is point/multipoint
  if ( type() == QGis::Point )
  {
    return 1; //cannot split points
  }

  //make sure, mGeos and mWkb are there and up-to-date
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 1;

  if ( !GEOSisValid( mGeos ) )
    return 7;

  //make sure splitLine is valid
  if (( type() == QGis::Line    && splitLine.size() < 1 ) ||
      ( type() == QGis::Polygon && splitLine.size() < 2 ) )
    return 1;

  newGeometries.clear();

  try
  {
    GEOSGeometry* splitLineGeos;
    if ( splitLine.size() > 1 )
    {
      splitLineGeos = createGeosLineString( splitLine.toVector() );
    }
    else if ( splitLine.size() == 1 )
    {
      splitLineGeos = createGeosPoint( splitLine.at( 0 ) );
    }
    else
    {
      return 1;
    }
    if ( !GEOSisValid( splitLineGeos ) || !GEOSisSimple( splitLineGeos ) )
    {
      GEOSGeom_destroy( splitLineGeos );
      return 1;
    }

    if ( topological )
    {
      //find out candidate points for topological corrections
      if ( topologicalTestPointsSplit( splitLineGeos, topologyTestPoints ) != 0 )
        return 1;
    }

    //call split function depending on geometry type
    if ( type() == QGis::Line )
    {
      returnCode = splitLinearGeometry( splitLineGeos, newGeometries );
      GEOSGeom_destroy( splitLineGeos );
    }
    else if ( type() == QGis::Polygon )
    {
      returnCode = splitPolygonGeometry( splitLineGeos, newGeometries );
      GEOSGeom_destroy( splitLineGeos );
    }
    else
    {
      return 1;
    }
  }
  CATCH_GEOS( 2 )

  return returnCode;
}

/**Replaces a part of this geometry with another line*/
int QgsGeometry::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  if ( reshapeWithLine.size() < 2 )
    return 1;

  if ( type() == QGis::Point )
    return 1; //cannot reshape points

  GEOSGeometry* reshapeLineGeos = createGeosLineString( reshapeWithLine.toVector() );

  //make sure this geos geometry is up-to-date
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 1;

  //single or multi?
  int numGeoms = GEOSGetNumGeometries( mGeos );
  if ( numGeoms == -1 )
    return 1;

  bool isMultiGeom = false;
  int geosTypeId = GEOSGeomTypeId( mGeos );
  if ( geosTypeId == GEOS_MULTILINESTRING || geosTypeId == GEOS_MULTIPOLYGON )
    isMultiGeom = true;

  bool isLine = ( type() == QGis::Line );

  //polygon or multipolygon?
  if ( !isMultiGeom )
  {
    GEOSGeometry* reshapedGeometry;
    if ( isLine )
      reshapedGeometry = reshapeLine( mGeos, reshapeLineGeos );
    else
      reshapedGeometry = reshapePolygon( mGeos, reshapeLineGeos );

    GEOSGeom_destroy( reshapeLineGeos );
    if ( reshapedGeometry )
    {
      GEOSGeom_destroy( mGeos );
      mGeos = reshapedGeometry;
      mDirtyWkb = true;
      return 0;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    //call reshape for each geometry part and replace mGeos with new geometry if reshape took place
    bool reshapeTookPlace = false;

    GEOSGeometry* currentReshapeGeometry = 0;
    GEOSGeometry** newGeoms = new GEOSGeometry*[numGeoms];

    for ( int i = 0; i < numGeoms; ++i )
    {
      if ( isLine )
        currentReshapeGeometry = reshapeLine( GEOSGetGeometryN( mGeos, i ), reshapeLineGeos );
      else
        currentReshapeGeometry = reshapePolygon( GEOSGetGeometryN( mGeos, i ), reshapeLineGeos );

      if ( currentReshapeGeometry )
      {
        newGeoms[i] = currentReshapeGeometry;
        reshapeTookPlace = true;
      }
      else
      {
        newGeoms[i] = GEOSGeom_clone( GEOSGetGeometryN( mGeos, i ) );
      }
    }
    GEOSGeom_destroy( reshapeLineGeos );

    GEOSGeometry* newMultiGeom = 0;
    if ( isLine )
    {
      newMultiGeom = GEOSGeom_createCollection( GEOS_MULTILINESTRING, newGeoms, numGeoms );
    }
    else //multipolygon
    {
      newMultiGeom = GEOSGeom_createCollection( GEOS_MULTIPOLYGON, newGeoms, numGeoms );
    }

    delete[] newGeoms;
    if ( !newMultiGeom )
      return 3;

    if ( reshapeTookPlace )
    {
      GEOSGeom_destroy( mGeos );
      mGeos = newMultiGeom;
      mDirtyWkb = true;
      return 0;
    }
    else
    {
      GEOSGeom_destroy( newMultiGeom );
      return 1;
    }
  }
}

int QgsGeometry::makeDifference( QgsGeometry* other )
{
  //make sure geos geometry is up to date
  if ( !other )
    return 1;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 1;

  if ( !GEOSisValid( mGeos ) )
    return 2;

  if ( !GEOSisSimple( mGeos ) )
    return 3;

  //convert other geometry to geos
  if ( other->mDirtyGeos )
    other->exportWkbToGeos();

  if ( !other->mGeos )
    return 4;

  //make geometry::difference
  try
  {
    if ( GEOSIntersects( mGeos, other->mGeos ) )
    {
      //check if multitype before and after
      bool multiType = isMultipart();

      mGeos = GEOSDifference( mGeos, other->mGeos );
      mDirtyWkb = true;

      if ( multiType && !isMultipart() )
      {
        convertToMultiType();
        exportWkbToGeos();
      }
    }
    else
    {
      return 0; //nothing to do
    }
  }
  CATCH_GEOS( 5 )

  if ( !mGeos )
  {
    mDirtyGeos = true;
    return 6;
  }

  return 0;
}

QgsRectangle QgsGeometry::boundingBox()
{
  double xmin =  std::numeric_limits<double>::max();
  double ymin =  std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    // Return minimal QgsRectangle
    QgsRectangle invalidRect;
    invalidRect.setMinimal();
    return invalidRect;
  }

  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  // consider endian when fetching feature type
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      double x, y;
      wkbPtr >> x >> y;

      if ( x < xmin )
        xmin = x;

      if ( x > xmax )
        xmax = x;

      if ( y < ymin )
        ymin = y;

      if ( y > ymax )
        ymax = y;

    }
    break;

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; idx++ )
      {
        wkbPtr += 1 + sizeof( int );

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( x < xmin )
          xmin = x;

        if ( x > xmax )
          xmax = x;

        if ( y < ymin )
          ymin = y;

        if ( y > ymax )
          ymax = y;

      }
      break;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; idx++ )
      {
        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( x < xmin )
          xmin = x;

        if ( x > xmax )
          xmax = x;

        if ( y < ymin )
          ymin = y;

        if ( y > ymax )
          ymax = y;

      }
      break;
    }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        // each of these is a wbklinestring so must handle as such
        wkbPtr += 1 + sizeof( int );   // skip type since we know its 2
        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( x < xmin )
            xmin = x;

          if ( x > xmax )
            xmax = x;

          if ( y < ymin )
            ymin = y;

          if ( y > ymax )
            ymax = y;

        }
      }
      break;
    }
    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      // get number of rings in the polygon
      int nRings;
      wkbPtr >> nRings;
      for ( int idx = 0; idx < nRings; idx++ )
      {
        // get number of points in the ring
        int nPoints;
        wkbPtr >> nPoints;
        for ( int jdx = 0; jdx < nPoints; jdx++ )
        {
          // add points to a point array for drawing the polygon
          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( x < xmin )
            xmin = x;

          if ( x > xmax )
            xmax = x;

          if ( y < ymin )
            ymin = y;

          if ( y > ymax )
            ymax = y;

        }
      }
      break;
    }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      // get the number of polygons
      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int kdx = 0; kdx < nPolygons; kdx++ )
      {
        //skip the endian and mGeometry type info and
        // get number of rings in the polygon
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int idx = 0; idx < nRings; idx++ )
        {
          // get number of points in the ring
          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            // add points to a point array for drawing the polygon
            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            if ( x < xmin )
              xmin = x;

            if ( x > xmax )
              xmax = x;

            if ( y < ymin )
              ymin = y;

            if ( y > ymax )
              ymax = y;

          }
        }
      }
      break;
    }

    default:
      QgsDebugMsg( QString( "Unknown WkbType %1 ENCOUNTERED" ).arg( wkbType ) );
      return QgsRectangle( 0, 0, 0, 0 );
      break;

  }
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

bool QgsGeometry::intersects( const QgsRectangle& r ) const
{
  QgsGeometry* g = fromRect( r );
  bool res = intersects( g );
  delete g;
  return res;
}

bool QgsGeometry::intersects( const QgsGeometry* geometry ) const
{
  if ( !geometry )
    return false;

  try // geos might throw exception on error
  {
    // ensure that both geometries have geos geometry
    exportWkbToGeos();
    geometry->exportWkbToGeos();

    if ( !mGeos || !geometry->mGeos )
    {
      QgsDebugMsg( "GEOS geometry not available!" );
      return false;
    }

    return GEOSIntersects( mGeos, geometry->mGeos );
  }
  CATCH_GEOS( false )
}

bool QgsGeometry::contains( const QgsPoint* p ) const
{
  exportWkbToGeos();

  if ( !p )
  {
    QgsDebugMsg( "pointer p is 0" );
    return false;
  }

  if ( !mGeos )
  {
    QgsDebugMsg( "GEOS geometry not available!" );
    return false;
  }

  GEOSGeometry *geosPoint = 0;

  bool returnval = false;

  try
  {
    geosPoint = createGeosPoint( *p );
    returnval = GEOSContains( mGeos, geosPoint );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    returnval = false;
  }

  if ( geosPoint )
    GEOSGeom_destroy( geosPoint );

  return returnval;
}

bool QgsGeometry::geosRelOp(
  char( *op )( const GEOSGeometry*, const GEOSGeometry * ),
  const QgsGeometry *a,
  const QgsGeometry *b )
{
  if ( !a || !b )
    return false;

  try // geos might throw exception on error
  {
    // ensure that both geometries have geos geometry
    a->exportWkbToGeos();
    b->exportWkbToGeos();

    if ( !a->mGeos || !b->mGeos )
    {
      QgsDebugMsg( "GEOS geometry not available!" );
      return false;
    }
    return op( a->mGeos, b->mGeos );
  }
  CATCH_GEOS( false )
}

bool QgsGeometry::contains( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSContains, this, geometry );
}

bool QgsGeometry::disjoint( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSDisjoint, this, geometry );
}

bool QgsGeometry::equals( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSEquals, this, geometry );
}

bool QgsGeometry::touches( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSTouches, this, geometry );
}

bool QgsGeometry::overlaps( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSOverlaps, this, geometry );
}

bool QgsGeometry::within( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSWithin, this, geometry );
}

bool QgsGeometry::crosses( const QgsGeometry* geometry ) const
{
  return geosRelOp( GEOSCrosses, this, geometry );
}

QString QgsGeometry::exportToWkt( const int &precision ) const
{
  QgsDebugMsg( "entered." );

  // TODO: implement with GEOS
  if ( mDirtyWkb )
  {
    exportGeosToWkb();
  }

  if ( !mGeometry || wkbSize() < 5 )
  {
    QgsDebugMsg( "WKB geometry not available or too short!" );
    return QString::null;
  }

  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  QString wkt;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      double x, y;
      wkbPtr >> x >> y;
      wkt += "POINT(" + qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision ) + ")";
      return wkt;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      wkt += "LINESTRING(";
      // get number of points in the line
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( idx != 0 )
          wkt += ", ";

        wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      wkt += "POLYGON(";
      // get number of rings in the polygon
      int nRings;
      wkbPtr >> nRings;
      if ( nRings == 0 )  // sanity check for zero rings in polygon
        return QString();

      for ( int idx = 0; idx < nRings; idx++ )
      {
        if ( idx != 0 )
          wkt += ",";

        wkt += "(";
        // get number of points in the ring
        int nPoints;
        wkbPtr >> nPoints;

        for ( int jdx = 0; jdx < nPoints; jdx++ )
        {
          if ( jdx != 0 )
            wkt += ",";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;

      wkt += "MULTIPOINT(";
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        wkbPtr += 1 + sizeof( int );
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;

      wkt += "MULTILINESTRING(";
      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        if ( jdx != 0 )
          wkt += ", ";

        wkt += "(";
        wkbPtr += 1 + sizeof( int ); // skip type since we know its 2
        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;

      wkt += "MULTIPOLYGON(";
      for ( int kdx = 0; kdx < nPolygons; kdx++ )
      {
        if ( kdx != 0 )
          wkt += ",";

        wkt += "(";
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int idx = 0; idx < nRings; idx++ )
        {
          if ( idx != 0 )
            wkt += ",";

          wkt += "(";
          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
              wkt += ",";

            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
          }
          wkt += ")";
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QString::null;
  }
}

QString QgsGeometry::exportToGeoJSON( const int &precision ) const
{
  QgsDebugMsg( "entered." );

  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return QString::null;
  }

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;
  bool hasZValue = false;

  QString wkt;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {

      double x, y;
      wkbPtr >> x >> y;

      wkt += "{ \"type\": \"Point\", \"coordinates\": ["
             + qgsDoubleToString( x, precision ) + ", "
             + qgsDoubleToString( y, precision )
             + "] }";
      return wkt;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {

      wkt += "{ \"type\": \"LineString\", \"coordinates\": [ ";
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {

      wkt += "{ \"type\": \"Polygon\", \"coordinates\": [ ";
      // get number of rings in the polygon
      int nRings;
      wkbPtr >> nRings;
      if ( nRings == 0 )  // sanity check for zero rings in polygon
        return QString();

      for ( int idx = 0; idx < nRings; idx++ )
      {
        if ( idx != 0 )
          wkt += ", ";

        wkt += "[ ";
        // get number of points in the ring
        int nPoints;
        wkbPtr >> nPoints;

        for ( int jdx = 0; jdx < nPoints; jdx++ )
        {
          if ( jdx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      wkt += "{ \"type\": \"MultiPoint\", \"coordinates\": [ ";
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        wkbPtr += 1 + sizeof( int );
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      wkt += "{ \"type\": \"MultiLineString\", \"coordinates\": [ ";

      int nLines;
      wkbPtr >> nLines;
      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        if ( jdx != 0 )
          wkt += ", ";

        wkt += "[ ";
        wkbPtr += 1 + sizeof( int ); // skip type since we know its 2

        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {

      wkt += "{ \"type\": \"MultiPolygon\", \"coordinates\": [ ";

      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int kdx = 0; kdx < nPolygons; kdx++ )
      {
        if ( kdx != 0 )
          wkt += ", ";

        wkt += "[ ";

        wkbPtr += 1 + sizeof( int );

        int nRings;
        wkbPtr >> nRings;
        for ( int idx = 0; idx < nRings; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          wkt += "[ ";

          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
              wkt += ", ";

            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
          }
          wkt += " ]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QString::null;
  }
}

bool QgsGeometry::exportWkbToGeos() const
{
  QgsDebugMsgLevel( "entered.", 3 );

  if ( !mDirtyGeos )
  {
    // No need to convert again
    return true;
  }

  if ( mGeos )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = 0;
  }

  // this probably shouldn't return true
  if ( !mGeometry )
  {
    // no WKB => no GEOS
    mDirtyGeos = false;
    return true;
  }

  bool hasZValue = false;

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  try
  {
    switch ( wkbType )
    {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
      {
        double x, y;
        wkbPtr >> x >> y;
        mGeos = createGeosPoint( QgsPoint( x, y ) );
        mDirtyGeos = false;
        break;
      }

      case QGis::WKBMultiPoint25D:
        hasZValue = true;
      case QGis::WKBMultiPoint:
      {
        QVector<GEOSGeometry *> points;

        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          double x, y;
          wkbPtr += 1 + sizeof( int );
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          points << createGeosPoint( QgsPoint( x, y ) );
        }
        mGeos = createGeosCollection( GEOS_MULTIPOINT, points );
        mDirtyGeos = false;
        break;
      }

      case QGis::WKBLineString25D:
        hasZValue = true;
      case QGis::WKBLineString:
      {
        QgsPolyline sequence;

        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          sequence << QgsPoint( x, y );
        }
        mDirtyGeos = false;
        mGeos = createGeosLineString( sequence );
        break;
      }

      case QGis::WKBMultiLineString25D:
        hasZValue = true;
      case QGis::WKBMultiLineString:
      {
        QVector<GEOSGeometry*> lines;
        int nLines;
        wkbPtr >> nLines;
        for ( int jdx = 0; jdx < nLines; jdx++ )
        {
          QgsPolyline sequence;

          // each of these is a wbklinestring so must handle as such
          wkbPtr += 1 + sizeof( int );   // skip type since we know its 2
          int nPoints;
          wkbPtr >> nPoints;
          for ( int idx = 0; idx < nPoints; idx++ )
          {
            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            sequence << QgsPoint( x, y );
          }

          // ignore invalid parts, it can come from ST_Simplify operations
          if ( sequence.count() > 1 )
            lines << createGeosLineString( sequence );
        }
        mGeos = createGeosCollection( GEOS_MULTILINESTRING, lines );
        mDirtyGeos = false;
        break;
      }

      case QGis::WKBPolygon25D:
        hasZValue = true;
      case QGis::WKBPolygon:
      {
        // get number of rings in the polygon
        int nRings;
        wkbPtr >> nRings;

        QVector<GEOSGeometry*> rings;

        for ( int idx = 0; idx < nRings; idx++ )
        {
          //QgsDebugMsg("Ring nr: "+QString::number(idx));

          QgsPolyline sequence;

          // get number of points in the ring
          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            // add points to a point array for drawing the polygon
            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            sequence << QgsPoint( x, y );
          }

          rings << createGeosLinearRing( sequence );
        }
        mGeos = createGeosPolygon( rings );
        mDirtyGeos = false;
        break;
      }

      case QGis::WKBMultiPolygon25D:
        hasZValue = true;
      case QGis::WKBMultiPolygon:
      {
        QVector<GEOSGeometry*> polygons;

        // get the number of polygons
        int nPolygons;
        wkbPtr >> nPolygons;

        for ( int kdx = 0; kdx < nPolygons; kdx++ )
        {
          //QgsDebugMsg("Polygon nr: "+QString::number(kdx));
          QVector<GEOSGeometry*> rings;

          //skip the endian and mGeometry type info and
          // get number of rings in the polygon
          wkbPtr += 1 + sizeof( int );
          int numRings;
          wkbPtr >> numRings;
          for ( int idx = 0; idx < numRings; idx++ )
          {
            //QgsDebugMsg("Ring nr: "+QString::number(idx));

            QgsPolyline sequence;

            // get number of points in the ring
            int nPoints;
            wkbPtr >> nPoints;
            for ( int jdx = 0; jdx < nPoints; jdx++ )
            {
              // add points to a point array for drawing the polygon
              double x, y;
              wkbPtr >> x >> y;
              if ( hasZValue )
                wkbPtr += sizeof( double );

              sequence << QgsPoint( x, y );
            }

            rings << createGeosLinearRing( sequence );
          }

          polygons << createGeosPolygon( rings );
        }
        mGeos = createGeosCollection( GEOS_MULTIPOLYGON, polygons );
        mDirtyGeos = false;
        break;
      }

      default:
        return false;
    }
  }
  CATCH_GEOS( false )

  return true;
}

bool QgsGeometry::exportGeosToWkb() const
{
  //QgsDebugMsg("entered.");
  if ( !mDirtyWkb )
  {
    // No need to convert again
    return true;
  }

  // clear the WKB, ready to replace with the new one
  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  if ( !mGeos )
  {
    // GEOS is null, therefore WKB is null.
    mDirtyWkb = false;
    return true;
  }

  // set up byteOrder
  char byteOrder = QgsApplication::endian();

  switch ( GEOSGeomTypeId( mGeos ) )
  {
    case GEOS_POINT:                 // a point
    {
      mGeometrySize = 1 +
                      sizeof( int ) +
                      2 * sizeof( double );
      mGeometry = new unsigned char[mGeometrySize];


      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( mGeos );

      double x, y;
      GEOSCoordSeq_getX( cs, 0, &x );
      GEOSCoordSeq_getY( cs, 0, &y );

      QgsWkbPtr wkbPtr( mGeometry );
      wkbPtr << byteOrder << QGis::WKBPoint << x << y;

      mDirtyWkb = false;
      return true;
    } // case GEOS_GEOM::GEOS_POINT

    case GEOS_LINESTRING:            // a linestring
    {
      //QgsDebugMsg("Got a geos::GEOS_LINESTRING.");

      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( mGeos );
      unsigned int nPoints;
      GEOSCoordSeq_getSize( cs, &nPoints );

      // allocate some space for the WKB
      mGeometrySize = 1 +   // sizeof(byte)
                      sizeof( int ) +
                      sizeof( int ) +
                      (( sizeof( double ) +
                         sizeof( double ) ) * nPoints );

      mGeometry = new unsigned char[mGeometrySize];
      QgsWkbPtr wkbPtr( mGeometry );

      wkbPtr << byteOrder << QGis::WKBLineString << nPoints;

      const GEOSCoordSequence *sequence = GEOSGeom_getCoordSeq( mGeos );

      // assign points
      for ( unsigned int n = 0; n < nPoints; n++ )
      {
        double x, y;
        GEOSCoordSeq_getX( sequence, n, &x );
        GEOSCoordSeq_getY( sequence, n, &y );
        wkbPtr << x << y;
      }

      mDirtyWkb = false;
      return true;

      // TODO: Deal with endian-ness
    } // case GEOS_GEOM::GEOS_LINESTRING

    case GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_LINEARRING

    case GEOS_POLYGON:               // a polygon
    {
      int nPointsInRing = 0;

      //first calculate the geometry size
      int geometrySize = 1 + 2 * sizeof( int ); //endian, type, number of rings
      const GEOSGeometry *theRing = GEOSGetExteriorRing( mGeos );
      if ( theRing )
      {
        geometrySize += sizeof( int );
        geometrySize += getNumGeosPoints( theRing ) * 2 * sizeof( double );
      }
      for ( int i = 0; i < GEOSGetNumInteriorRings( mGeos ); ++i )
      {
        geometrySize += sizeof( int ); //number of points in ring
        theRing = GEOSGetInteriorRingN( mGeos, i );
        if ( theRing )
        {
          geometrySize += getNumGeosPoints( theRing ) * 2 * sizeof( double );
        }
      }

      mGeometry = new unsigned char[geometrySize];
      mGeometrySize = geometrySize;

      //then fill the geometry itself into the wkb
      QgsWkbPtr wkbPtr( mGeometry );

      int nRings = GEOSGetNumInteriorRings( mGeos ) + 1;
      wkbPtr << byteOrder << QGis::WKBPolygon << nRings;

      //exterior ring first
      theRing = GEOSGetExteriorRing( mGeos );
      if ( theRing )
      {
        nPointsInRing = getNumGeosPoints( theRing );

        wkbPtr << nPointsInRing;

        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( theRing );
        unsigned int n;
        GEOSCoordSeq_getSize( cs, &n );

        for ( unsigned int j = 0; j < n; ++j )
        {
          double x, y;
          GEOSCoordSeq_getX( cs, j, &x );
          GEOSCoordSeq_getY( cs, j, &y );
          wkbPtr << x << y;
        }
      }

      //interior rings after
      for ( int i = 0; i < GEOSGetNumInteriorRings( mGeos ); i++ )
      {
        theRing = GEOSGetInteriorRingN( mGeos, i );

        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( theRing );

        unsigned int nPointsInRing;
        GEOSCoordSeq_getSize( cs, &nPointsInRing );
        wkbPtr << nPointsInRing;

        for ( unsigned int j = 0; j < nPointsInRing; j++ )
        {
          double x, y;
          GEOSCoordSeq_getX( cs, j, &x );
          GEOSCoordSeq_getY( cs, j, &y );
          wkbPtr << x << y;
        }
      }
      mDirtyWkb = false;
      return true;
    } // case GEOS_GEOM::GEOS_POLYGON
    break;

    case GEOS_MULTIPOINT:            // a collection of points
    {
      // determine size of geometry
      int geometrySize = 1 + 2 * sizeof( int );
      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        geometrySize += 1 + sizeof( int ) + 2 * sizeof( double );
      }

      mGeometry = new unsigned char[geometrySize];
      mGeometrySize = geometrySize;

      QgsWkbPtr wkbPtr( mGeometry );
      int numPoints = GEOSGetNumGeometries( mGeos );

      wkbPtr << byteOrder << QGis::WKBMultiPoint << numPoints;

      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        //copy endian and point type
        wkbPtr << byteOrder << QGis::WKBPoint;

        const GEOSGeometry *currentPoint = GEOSGetGeometryN( mGeos, i );

        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( currentPoint );

        double x, y;
        GEOSCoordSeq_getX( cs, 0, &x );
        GEOSCoordSeq_getY( cs, 0, &y );
        wkbPtr << x << y;
      }
      mDirtyWkb = false;
      return true;
    } // case GEOS_GEOM::GEOS_MULTIPOINT

    case GEOS_MULTILINESTRING:       // a collection of linestrings
    {
      // determine size of geometry
      int geometrySize = 1 + 2 * sizeof( int );
      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        geometrySize += 1 + 2 * sizeof( int );
        geometrySize += getNumGeosPoints( GEOSGetGeometryN( mGeos, i ) ) * 2 * sizeof( double );
      }

      mGeometry = new unsigned char[geometrySize];
      mGeometrySize = geometrySize;

      QgsWkbPtr wkbPtr( mGeometry );

      int numLines = GEOSGetNumGeometries( mGeos );
      wkbPtr << byteOrder << QGis::WKBMultiLineString << numLines;

      //loop over lines
      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        //endian and type WKBLineString
        wkbPtr << byteOrder << QGis::WKBLineString;

        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( GEOSGetGeometryN( mGeos, i ) );

        //line size
        unsigned int lineSize;
        GEOSCoordSeq_getSize( cs, &lineSize );
        wkbPtr << lineSize;

        //vertex coordinates
        for ( unsigned int j = 0; j < lineSize; ++j )
        {
          double x, y;
          GEOSCoordSeq_getX( cs, j, &x );
          GEOSCoordSeq_getY( cs, j, &y );
          wkbPtr << x << y;
        }
      }
      mDirtyWkb = false;
      return true;
    } // case GEOS_GEOM::GEOS_MULTILINESTRING

    case GEOS_MULTIPOLYGON:          // a collection of polygons
    {
      //first determine size of geometry
      int geometrySize = 1 + 2 * sizeof( int ); //endian, type, number of polygons
      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        const GEOSGeometry *thePoly = GEOSGetGeometryN( mGeos, i );
        geometrySize += 1 + 2 * sizeof( int ); //endian, type, number of rings
        //exterior ring
        geometrySize += sizeof( int ); //number of points in exterior ring
        const GEOSGeometry *exRing = GEOSGetExteriorRing( thePoly );
        geometrySize += 2 * sizeof( double ) * getNumGeosPoints( exRing );

        const GEOSGeometry *intRing = 0;
        for ( int j = 0; j < GEOSGetNumInteriorRings( thePoly ); j++ )
        {
          geometrySize += sizeof( int ); //number of points in ring
          intRing = GEOSGetInteriorRingN( thePoly, j );
          geometrySize += 2 * sizeof( double ) * getNumGeosPoints( intRing );
        }
      }

      mGeometry = new unsigned char[geometrySize];
      mGeometrySize = geometrySize;

      QgsWkbPtr wkbPtr( mGeometry );
      int numPolygons = GEOSGetNumGeometries( mGeos );
      wkbPtr << byteOrder << QGis::WKBMultiPolygon << numPolygons;

      //loop over polygons
      for ( int i = 0; i < GEOSGetNumGeometries( mGeos ); i++ )
      {
        const GEOSGeometry *thePoly = GEOSGetGeometryN( mGeos, i );
        int numRings = GEOSGetNumInteriorRings( thePoly ) + 1;

        //exterior ring
        const GEOSGeometry *theRing = GEOSGetExteriorRing( thePoly );
        int nPointsInRing = getNumGeosPoints( theRing );

        wkbPtr << byteOrder << QGis::WKBPolygon << numRings << nPointsInRing;

        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( theRing );

        for ( int k = 0; k < nPointsInRing; ++k )
        {
          double x, y;
          GEOSCoordSeq_getX( cs, k, &x );
          GEOSCoordSeq_getY( cs, k, &y );
          wkbPtr << x << y;
        }

        //interior rings
        for ( int j = 0; j < GEOSGetNumInteriorRings( thePoly ); j++ )
        {
          theRing = GEOSGetInteriorRingN( thePoly, j );

          int nPointsInRing = getNumGeosPoints( theRing );
          wkbPtr << nPointsInRing;

          const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq( theRing );

          for ( int k = 0; k < nPointsInRing; ++k )
          {
            double x, y;
            GEOSCoordSeq_getX( cs, k, &x );
            GEOSCoordSeq_getY( cs, k, &y );
            wkbPtr << x << y;
          }
        }
      }
      mDirtyWkb = false;
      return true;
    } // case GEOS_GEOM::GEOS_MULTIPOLYGON

    case GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
    {
      // TODO
      QgsDebugMsg( "geometry collection - not supported" );
      break;
    } // case GEOS_GEOM::GEOS_GEOMETRYCOLLECTION

  } // switch (mGeos->getGeometryTypeId())

  return false;
}

QgsGeometry* QgsGeometry::convertToType( QGis::GeometryType destType, bool destMultipart )
{
  switch ( destType )
  {
    case QGis::Point:
      return convertToPoint( destMultipart );

    case QGis::Line:
      return convertToLine( destMultipart );

    case QGis::Polygon:
      return convertToPolygon( destMultipart );

    default:
      return 0;
  }
}

bool QgsGeometry::convertToMultiType()
{
  // TODO: implement with GEOS
  if ( mDirtyWkb )
  {
    exportGeosToWkb();
  }

  if ( !mGeometry )
  {
    return false;
  }

  QGis::WkbType geomType = wkbType();

  if ( geomType == QGis::WKBMultiPoint || geomType == QGis::WKBMultiPoint25D ||
       geomType == QGis::WKBMultiLineString || geomType == QGis::WKBMultiLineString25D ||
       geomType == QGis::WKBMultiPolygon || geomType == QGis::WKBMultiPolygon25D || geomType == QGis::WKBUnknown )
  {
    return false; //no need to convert
  }

  size_t newGeomSize = mGeometrySize + 1 + 2 * sizeof( int ); //endian: 1, multitype: sizeof(int), number of geometries: sizeof(int)
  unsigned char* newGeometry = new unsigned char[newGeomSize];

  //copy endian
  char byteOrder = QgsApplication::endian();

  QgsWkbPtr wkbPtr( newGeometry );
  wkbPtr << byteOrder;

  //copy wkbtype
  //todo
  QGis::WkbType newMultiType;
  switch ( geomType )
  {
    case QGis::WKBPoint:
      newMultiType = QGis::WKBMultiPoint;
      break;
    case QGis::WKBPoint25D:
      newMultiType = QGis::WKBMultiPoint25D;
      break;
    case QGis::WKBLineString:
      newMultiType = QGis::WKBMultiLineString;
      break;
    case QGis::WKBLineString25D:
      newMultiType = QGis::WKBMultiLineString25D;
      break;
    case QGis::WKBPolygon:
      newMultiType = QGis::WKBMultiPolygon;
      break;
    case QGis::WKBPolygon25D:
      newMultiType = QGis::WKBMultiPolygon25D;
      break;
    default:
      delete [] newGeometry;
      return false;
  }

  wkbPtr << newMultiType << 1;

  //copy the existing single geometry
  memcpy( wkbPtr, mGeometry, mGeometrySize );

  delete [] mGeometry;
  mGeometry = newGeometry;
  mGeometrySize = newGeomSize;
  mDirtyGeos = true;
  return true;
}

void QgsGeometry::translateVertex( QgsWkbPtr &wkbPtr, double dx, double dy, bool hasZValue )
{
  double x, y, translated_x, translated_y;

  //x-coordinate
  memcpy( &x, wkbPtr, sizeof( double ) );
  translated_x = x + dx;
  memcpy( wkbPtr, &translated_x, sizeof( double ) );
  wkbPtr += sizeof( double );

  //y-coordinate
  memcpy( &y, wkbPtr, sizeof( double ) );
  translated_y = y + dy;
  memcpy( wkbPtr, &translated_y, sizeof( double ) );
  wkbPtr += sizeof( double );

  if ( hasZValue )
    wkbPtr += sizeof( double );
}

void QgsGeometry::transformVertex( QgsWkbPtr &wkbPtr, const QgsCoordinateTransform& ct, bool hasZValue )
{
  double x, y, z;

  memcpy( &x, wkbPtr, sizeof( double ) );
  memcpy( &y, wkbPtr + sizeof( double ), sizeof( double ) );
  z = 0.0; // Ignore Z for now.

  ct.transformInPlace( x, y, z );

  // new coordinate
  wkbPtr << x << y;
  if ( hasZValue )
    wkbPtr += sizeof( double );

}

GEOSGeometry* QgsGeometry::linePointDifference( GEOSGeometry* GEOSsplitPoint )
{
  int type = GEOSGeomTypeId( mGeos );
  QgsMultiPolyline multiLine;

  if ( type == GEOS_MULTILINESTRING )
    multiLine = asMultiPolyline();
  else if ( type == GEOS_LINESTRING )
    multiLine = QgsMultiPolyline() << asPolyline();
  else
    return 0;

  QgsPoint splitPoint = fromGeosGeom( GEOSsplitPoint )->asPoint();

  QgsMultiPolyline lines;
  QgsPolyline line;
  QgsPolyline newline;

  //For each part
  for ( int i = 0; i < multiLine.size() ; ++i )
  {
    line = multiLine[i];
    newline = QgsPolyline();
    newline.append( line[0] );
    //For each segment
    for ( int j = 1; j < line.size() - 1 ; ++j )
    {
      newline.append( line[j] );
      if ( line[j] == splitPoint )
      {
        lines.append( newline );
        newline = QgsPolyline();
        newline.append( line[j] );
      }
    }
    newline.append( line.last() );
    lines.append( newline );
  }
  QgsGeometry* splitLines = fromMultiPolyline( lines );
  GEOSGeometry* splitGeom = GEOSGeom_clone( splitLines->asGeos() );

  return splitGeom;

}

int QgsGeometry::splitLinearGeometry( GEOSGeometry *splitLine, QList<QgsGeometry*>& newGeometries )
{
  if ( !splitLine )
    return 2;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 5;

  //first test if linestring intersects geometry. If not, return straight away
  if ( !GEOSIntersects( splitLine, mGeos ) )
    return 1;

  //check that split line has no linear intersection
  int linearIntersect = GEOSRelatePattern( mGeos, splitLine, "1********" );
  if ( linearIntersect > 0 )
    return 3;

  int splitGeomType = GEOSGeomTypeId( splitLine );

  GEOSGeometry* splitGeom;
  if ( splitGeomType == GEOS_POINT )
  {
    splitGeom = linePointDifference( splitLine );
  }
  else
  {
    splitGeom = GEOSDifference( mGeos, splitLine );
  }
  QVector<GEOSGeometry*> lineGeoms;

  int splitType = GEOSGeomTypeId( splitGeom );
  if ( splitType == GEOS_MULTILINESTRING )
  {
    int nGeoms = GEOSGetNumGeometries( splitGeom );
    for ( int i = 0; i < nGeoms; ++i )
      lineGeoms << GEOSGeom_clone( GEOSGetGeometryN( splitGeom, i ) );

  }
  else
  {
    lineGeoms << GEOSGeom_clone( splitGeom );
  }

  mergeGeometriesMultiTypeSplit( lineGeoms );

  if ( lineGeoms.size() > 0 )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = lineGeoms[0];
    mDirtyWkb = true;
  }

  for ( int i = 1; i < lineGeoms.size(); ++i )
  {
    newGeometries << fromGeosGeom( lineGeoms[i] );
  }

  GEOSGeom_destroy( splitGeom );
  return 0;
}

int QgsGeometry::splitPolygonGeometry( GEOSGeometry* splitLine, QList<QgsGeometry*>& newGeometries )
{
  if ( !splitLine )
    return 2;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 5;

  //first test if linestring intersects geometry. If not, return straight away
  if ( !GEOSIntersects( splitLine, mGeos ) )
    return 1;

  //first union all the polygon rings together (to get them noded, see JTS developer guide)
  GEOSGeometry *nodedGeometry = nodeGeometries( splitLine, mGeos );
  if ( !nodedGeometry )
    return 2; //an error occured during noding

  GEOSGeometry *polygons = GEOSPolygonize( &nodedGeometry, 1 );
  if ( !polygons || numberOfGeometries( polygons ) == 0 )
  {
    if ( polygons )
      GEOSGeom_destroy( polygons );

    GEOSGeom_destroy( nodedGeometry );

    return 4;
  }

  GEOSGeom_destroy( nodedGeometry );

  //test every polygon if contained in original geometry
  //include in result if yes
  QVector<GEOSGeometry*> testedGeometries;
  GEOSGeometry *intersectGeometry = 0;

  //ratio intersect geometry / geometry. This should be close to 1
  //if the polygon belongs to the input geometry

  for ( int i = 0; i < numberOfGeometries( polygons ); i++ )
  {
    const GEOSGeometry *polygon = GEOSGetGeometryN( polygons, i );
    intersectGeometry = GEOSIntersection( mGeos, polygon );
    if ( !intersectGeometry )
    {
      QgsDebugMsg( "intersectGeometry is NULL" );
      continue;
    }

    double intersectionArea;
    GEOSArea( intersectGeometry, &intersectionArea );

    double polygonArea;
    GEOSArea( polygon, &polygonArea );

    const double areaRatio = intersectionArea / polygonArea;
    if ( areaRatio > 0.99 && areaRatio < 1.01 )
      testedGeometries << GEOSGeom_clone( polygon );

    GEOSGeom_destroy( intersectGeometry );
  }

  bool splitDone = true;
  int nGeometriesThis = numberOfGeometries( mGeos ); //original number of geometries
  if ( testedGeometries.size() == nGeometriesThis )
  {
    splitDone = false;
  }

  mergeGeometriesMultiTypeSplit( testedGeometries );

  //no split done, preserve original geometry
  if ( !splitDone )
  {
    for ( int i = 0; i < testedGeometries.size(); ++i )
    {
      GEOSGeom_destroy( testedGeometries[i] );
    }
    return 1;
  }
  else if ( testedGeometries.size() > 0 ) //split successfull
  {
    GEOSGeom_destroy( mGeos );
    mGeos = testedGeometries[0];
    mDirtyWkb = true;
  }

  int i;
  for ( i = 1; i < testedGeometries.size() && GEOSisValid( testedGeometries[i] ); ++i )
    ;

  if ( i < testedGeometries.size() )
  {
    for ( i = 0; i < testedGeometries.size(); ++i )
      GEOSGeom_destroy( testedGeometries[i] );

    return 3;
  }

  for ( i = 1; i < testedGeometries.size(); ++i )
    newGeometries << fromGeosGeom( testedGeometries[i] );

  GEOSGeom_destroy( polygons );
  return 0;
}

GEOSGeometry* QgsGeometry::reshapePolygon( const GEOSGeometry* polygon, const GEOSGeometry* reshapeLineGeos )
{
  //go through outer shell and all inner rings and check if there is exactly one intersection of a ring and the reshape line
  int nIntersections = 0;
  int lastIntersectingRing = -2;
  const GEOSGeometry* lastIntersectingGeom = 0;

  int nRings = GEOSGetNumInteriorRings( polygon );
  if ( nRings < 0 )
    return 0;

  //does outer ring intersect?
  const GEOSGeometry* outerRing = GEOSGetExteriorRing( polygon );
  if ( GEOSIntersects( outerRing, reshapeLineGeos ) == 1 )
  {
    ++nIntersections;
    lastIntersectingRing = -1;
    lastIntersectingGeom = outerRing;
  }

  //do inner rings intersect?
  const GEOSGeometry **innerRings = new const GEOSGeometry*[nRings];

  try
  {
    for ( int i = 0; i < nRings; ++i )
    {
      innerRings[i] = GEOSGetInteriorRingN( polygon, i );
      if ( GEOSIntersects( innerRings[i], reshapeLineGeos ) == 1 )
      {
        ++nIntersections;
        lastIntersectingRing = i;
        lastIntersectingGeom = innerRings[i];
      }
    }
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    nIntersections = 0;
  }

  if ( nIntersections != 1 ) //reshape line is only allowed to intersect one ring
  {
    delete [] innerRings;
    return 0;
  }

  //we have one intersecting ring, let's try to reshape it
  GEOSGeometry* reshapeResult = reshapeLine( lastIntersectingGeom, reshapeLineGeos );
  if ( !reshapeResult )
  {
    delete [] innerRings;
    return 0;
  }

  //if reshaping took place, we need to reassemble the polygon and its rings
  GEOSGeometry* newRing = 0;
  const GEOSCoordSequence* reshapeSequence = GEOSGeom_getCoordSeq( reshapeResult );
  GEOSCoordSequence* newCoordSequence = GEOSCoordSeq_clone( reshapeSequence );

  GEOSGeom_destroy( reshapeResult );

  newRing = GEOSGeom_createLinearRing( newCoordSequence );
  if ( !newRing )
  {
    delete [] innerRings;
    return 0;
  }

  GEOSGeometry* newOuterRing = 0;
  if ( lastIntersectingRing == -1 )
    newOuterRing = newRing;
  else
    newOuterRing = GEOSGeom_clone( outerRing );

  //check if all the rings are still inside the outer boundary
  QList<GEOSGeometry*> ringList;
  if ( nRings > 0 )
  {
    GEOSGeometry* outerRingPoly = GEOSGeom_createPolygon( GEOSGeom_clone( newOuterRing ), 0, 0 );
    if ( outerRingPoly )
    {
      GEOSGeometry* currentRing = 0;
      for ( int i = 0; i < nRings; ++i )
      {
        if ( lastIntersectingRing == i )
          currentRing = newRing;
        else
          currentRing = GEOSGeom_clone( innerRings[i] );

        //possibly a ring is no longer contained in the result polygon after reshape
        if ( GEOSContains( outerRingPoly, currentRing ) == 1 )
          ringList.push_back( currentRing );
        else
          GEOSGeom_destroy( currentRing );
      }
    }
    GEOSGeom_destroy( outerRingPoly );
  }

  GEOSGeometry** newInnerRings = new GEOSGeometry*[ringList.size()];
  for ( int i = 0; i < ringList.size(); ++i )
    newInnerRings[i] = ringList.at( i );

  delete [] innerRings;

  GEOSGeometry* reshapedPolygon = GEOSGeom_createPolygon( newOuterRing, newInnerRings, ringList.size() );
  delete[] newInnerRings;

  return reshapedPolygon;
}

GEOSGeometry* QgsGeometry::reshapeLine( const GEOSGeometry* line, const GEOSGeometry* reshapeLineGeos )
{
  if ( !line || !reshapeLineGeos )
    return 0;

  bool atLeastTwoIntersections = false;

  try
  {
    //make sure there are at least two intersection between line and reshape geometry
    GEOSGeometry* intersectGeom = GEOSIntersection( line, reshapeLineGeos );
    if ( intersectGeom )
    {
      atLeastTwoIntersections = ( GEOSGeomTypeId( intersectGeom ) == GEOS_MULTIPOINT && GEOSGetNumGeometries( intersectGeom ) > 1 );
      GEOSGeom_destroy( intersectGeom );
    }
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    atLeastTwoIntersections = false;
  }

  if ( !atLeastTwoIntersections )
    return 0;

  //begin and end point of original line
  const GEOSCoordSequence* lineCoordSeq = GEOSGeom_getCoordSeq( line );
  if ( !lineCoordSeq )
    return 0;

  unsigned int lineCoordSeqSize;
  if ( GEOSCoordSeq_getSize( lineCoordSeq, &lineCoordSeqSize ) == 0 )
    return 0;

  if ( lineCoordSeqSize < 2 )
    return 0;

  //first and last vertex of line
  double x1, y1, x2, y2;
  GEOSCoordSeq_getX( lineCoordSeq, 0, &x1 );
  GEOSCoordSeq_getY( lineCoordSeq, 0, &y1 );
  GEOSCoordSeq_getX( lineCoordSeq, lineCoordSeqSize - 1, &x2 );
  GEOSCoordSeq_getY( lineCoordSeq, lineCoordSeqSize - 1, &y2 );
  GEOSGeometry* beginLineVertex = createGeosPoint( QgsPoint( x1, y1 ) );
  GEOSGeometry* endLineVertex = createGeosPoint( QgsPoint( x2, y2 ) );

  bool isRing = false;
  if ( GEOSGeomTypeId( line ) == GEOS_LINEARRING || GEOSEquals( beginLineVertex, endLineVertex ) == 1 )
    isRing = true;

//node line and reshape line
  GEOSGeometry* nodedGeometry = nodeGeometries( reshapeLineGeos, line );
  if ( !nodedGeometry )
  {
    GEOSGeom_destroy( beginLineVertex );
    GEOSGeom_destroy( endLineVertex );
    return 0;
  }

  //and merge them together
  GEOSGeometry *mergedLines = GEOSLineMerge( nodedGeometry );
  GEOSGeom_destroy( nodedGeometry );
  if ( !mergedLines )
  {
    GEOSGeom_destroy( beginLineVertex );
    GEOSGeom_destroy( endLineVertex );
    return 0;
  }

  int numMergedLines = GEOSGetNumGeometries( mergedLines );
  if ( numMergedLines < 2 ) //some special cases. Normally it is >2
  {
    GEOSGeom_destroy( beginLineVertex );
    GEOSGeom_destroy( endLineVertex );
    if ( numMergedLines == 1 ) //reshape line is from begin to endpoint. So we keep the reshapeline
      return GEOSGeom_clone( reshapeLineGeos );
    else
      return 0;
  }

  QList<GEOSGeometry*> resultLineParts; //collection with the line segments that will be contained in result
  QList<GEOSGeometry*> probableParts; //parts where we can decide on inclusion only after going through all the candidates

  for ( int i = 0; i < numMergedLines; ++i )
  {
    const GEOSGeometry* currentGeom;

    currentGeom = GEOSGetGeometryN( mergedLines, i );
    const GEOSCoordSequence* currentCoordSeq = GEOSGeom_getCoordSeq( currentGeom );
    unsigned int currentCoordSeqSize;
    GEOSCoordSeq_getSize( currentCoordSeq, &currentCoordSeqSize );
    if ( currentCoordSeqSize < 2 )
      continue;

    //get the two endpoints of the current line merge result
    double xBegin, xEnd, yBegin, yEnd;
    GEOSCoordSeq_getX( currentCoordSeq, 0, &xBegin );
    GEOSCoordSeq_getY( currentCoordSeq, 0, &yBegin );
    GEOSCoordSeq_getX( currentCoordSeq, currentCoordSeqSize - 1, &xEnd );
    GEOSCoordSeq_getY( currentCoordSeq, currentCoordSeqSize - 1, &yEnd );
    GEOSGeometry* beginCurrentGeomVertex = createGeosPoint( QgsPoint( xBegin, yBegin ) );
    GEOSGeometry* endCurrentGeomVertex = createGeosPoint( QgsPoint( xEnd, yEnd ) );

    //check how many endpoints of the line merge result are on the (original) line
    int nEndpointsOnOriginalLine = 0;
    if ( pointContainedInLine( beginCurrentGeomVertex, line ) == 1 )
      nEndpointsOnOriginalLine += 1;

    if ( pointContainedInLine( endCurrentGeomVertex, line ) == 1 )
      nEndpointsOnOriginalLine += 1;

    //check how many endpoints equal the endpoints of the original line
    int nEndpointsSameAsOriginalLine = 0;
    if ( GEOSEquals( beginCurrentGeomVertex, beginLineVertex ) == 1 || GEOSEquals( beginCurrentGeomVertex, endLineVertex ) == 1 )
      nEndpointsSameAsOriginalLine += 1;

    if ( GEOSEquals( endCurrentGeomVertex, beginLineVertex ) == 1 || GEOSEquals( endCurrentGeomVertex, endLineVertex ) == 1 )
      nEndpointsSameAsOriginalLine += 1;

    //check if the current geometry overlaps the original geometry (GEOSOverlap does not seem to work with linestrings)
    bool currentGeomOverlapsOriginalGeom = false;
    bool currentGeomOverlapsReshapeLine = false;
    if ( QgsGeometry::lineContainedInLine( currentGeom, line ) == 1 )
      currentGeomOverlapsOriginalGeom = true;

    if ( QgsGeometry::lineContainedInLine( currentGeom, reshapeLineGeos ) == 1 )
      currentGeomOverlapsReshapeLine = true;

    //logic to decide if this part belongs to the result
    if ( nEndpointsSameAsOriginalLine == 1 && nEndpointsOnOriginalLine == 2 && currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone( currentGeom ) );
    }
    //for closed rings, we take one segment from the candidate list
    else if ( isRing && nEndpointsOnOriginalLine == 2 && currentGeomOverlapsOriginalGeom )
    {
      probableParts.push_back( GEOSGeom_clone( currentGeom ) );
    }
    else if ( nEndpointsOnOriginalLine == 2 && !currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone( currentGeom ) );
    }
    else if ( nEndpointsSameAsOriginalLine == 2 && !currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone( currentGeom ) );
    }
    else if ( currentGeomOverlapsOriginalGeom && currentGeomOverlapsReshapeLine )
    {
      resultLineParts.push_back( GEOSGeom_clone( currentGeom ) );
    }

    GEOSGeom_destroy( beginCurrentGeomVertex );
    GEOSGeom_destroy( endCurrentGeomVertex );
  }

  //add the longest segment from the probable list for rings (only used for polygon rings)
  if ( isRing && probableParts.size() > 0 )
  {
    GEOSGeometry* maxGeom = 0; //the longest geometry in the probabla list
    GEOSGeometry* currentGeom = 0;
    double maxLength = -DBL_MAX;
    double currentLength = 0;
    for ( int i = 0; i < probableParts.size(); ++i )
    {
      currentGeom = probableParts.at( i );
      GEOSLength( currentGeom, &currentLength );
      if ( currentLength > maxLength )
      {
        maxLength = currentLength;
        GEOSGeom_destroy( maxGeom );
        maxGeom = currentGeom;
      }
      else
      {
        GEOSGeom_destroy( currentGeom );
      }
    }
    resultLineParts.push_back( maxGeom );
  }

  GEOSGeom_destroy( beginLineVertex );
  GEOSGeom_destroy( endLineVertex );
  GEOSGeom_destroy( mergedLines );

  GEOSGeometry* result = 0;
  if ( resultLineParts.size() < 1 )
    return 0;

  if ( resultLineParts.size() == 1 ) //the whole result was reshaped
  {
    result = resultLineParts[0];
  }
  else //>1
  {
    GEOSGeometry **lineArray = new GEOSGeometry*[resultLineParts.size()];
    for ( int i = 0; i < resultLineParts.size(); ++i )
    {
      lineArray[i] = resultLineParts[i];
    }

    //create multiline from resultLineParts
    GEOSGeometry* multiLineGeom = GEOSGeom_createCollection( GEOS_MULTILINESTRING, lineArray, resultLineParts.size() );
    delete [] lineArray;

    //then do a linemerge with the newly combined partstrings
    result = GEOSLineMerge( multiLineGeom );
    GEOSGeom_destroy( multiLineGeom );
  }

  //now test if the result is a linestring. Otherwise something went wrong
  if ( GEOSGeomTypeId( result ) != GEOS_LINESTRING )
  {
    GEOSGeom_destroy( result );
    return 0;
  }

  return result;
}

int QgsGeometry::topologicalTestPointsSplit( const GEOSGeometry* splitLine, QList<QgsPoint>& testPoints ) const
{
  //Find out the intersection points between splitLineGeos and this geometry.
  //These points need to be tested for topological correctness by the calling function
  //if topological editing is enabled

  testPoints.clear();
  GEOSGeometry* intersectionGeom = GEOSIntersection( mGeos, splitLine );
  if ( !intersectionGeom )
    return 1;

  bool simple = false;
  int nIntersectGeoms = 1;
  if ( GEOSGeomTypeId( intersectionGeom ) == GEOS_LINESTRING || GEOSGeomTypeId( intersectionGeom ) == GEOS_POINT )
    simple = true;

  if ( !simple )
    nIntersectGeoms = GEOSGetNumGeometries( intersectionGeom );

  for ( int i = 0; i < nIntersectGeoms; ++i )
  {
    const GEOSGeometry* currentIntersectGeom;
    if ( simple )
      currentIntersectGeom = intersectionGeom;
    else
      currentIntersectGeom = GEOSGetGeometryN( intersectionGeom, i );

    const GEOSCoordSequence* lineSequence = GEOSGeom_getCoordSeq( currentIntersectGeom );
    unsigned int sequenceSize = 0;
    double x, y;
    if ( GEOSCoordSeq_getSize( lineSequence, &sequenceSize ) != 0 )
    {
      for ( unsigned int i = 0; i < sequenceSize; ++i )
      {
        if ( GEOSCoordSeq_getX( lineSequence, i, &x ) != 0 )
        {
          if ( GEOSCoordSeq_getY( lineSequence, i, &y ) != 0 )
          {
            testPoints.push_back( QgsPoint( x, y ) );
          }
        }
      }
    }
  }
  GEOSGeom_destroy( intersectionGeom );
  return 0;
}

GEOSGeometry *QgsGeometry::nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom )
{
  if ( !splitLine || !geom )
    return 0;

  GEOSGeometry *geometryBoundary = 0;
  if ( GEOSGeomTypeId( geom ) == GEOS_POLYGON || GEOSGeomTypeId( geom ) == GEOS_MULTIPOLYGON )
    geometryBoundary = GEOSBoundary( geom );
  else
    geometryBoundary = GEOSGeom_clone( geom );

  GEOSGeometry *splitLineClone = GEOSGeom_clone( splitLine );
  GEOSGeometry *unionGeometry = GEOSUnion( splitLineClone, geometryBoundary );
  GEOSGeom_destroy( splitLineClone );

  GEOSGeom_destroy( geometryBoundary );
  return unionGeometry;
}

int QgsGeometry::lineContainedInLine( const GEOSGeometry* line1, const GEOSGeometry* line2 )
{
  if ( !line1 || !line2 )
  {
    return -1;
  }

  double bufferDistance = pow( 1.0L, geomDigits( line2 ) - 11 );

  GEOSGeometry* bufferGeom = GEOSBuffer( line2, bufferDistance, DEFAULT_QUADRANT_SEGMENTS );
  if ( !bufferGeom )
    return -2;

  GEOSGeometry* intersectionGeom = GEOSIntersection( bufferGeom, line1 );

  //compare ratio between line1Length and intersectGeomLength (usually close to 1 if line1 is contained in line2)
  double intersectGeomLength;
  double line1Length;

  GEOSLength( intersectionGeom, &intersectGeomLength );
  GEOSLength( line1, &line1Length );

  GEOSGeom_destroy( bufferGeom );
  GEOSGeom_destroy( intersectionGeom );

  double intersectRatio = line1Length / intersectGeomLength;
  if ( intersectRatio > 0.9 && intersectRatio < 1.1 )
    return 1;

  return 0;
}

int QgsGeometry::pointContainedInLine( const GEOSGeometry* point, const GEOSGeometry* line )
{
  if ( !point || !line )
    return -1;

  double bufferDistance = pow( 1.0L, geomDigits( line ) - 11 );

  GEOSGeometry* lineBuffer = GEOSBuffer( line, bufferDistance, 8 );
  if ( !lineBuffer )
    return -2;

  bool contained = false;
  if ( GEOSContains( lineBuffer, point ) == 1 )
    contained = true;

  GEOSGeom_destroy( lineBuffer );
  return contained;
}

int QgsGeometry::geomDigits( const GEOSGeometry* geom )
{
  GEOSGeometry* bbox = GEOSEnvelope( geom );
  if ( !bbox )
    return -1;

  const GEOSGeometry* bBoxRing = GEOSGetExteriorRing( bbox );
  if ( !bBoxRing )
    return -1;

  const GEOSCoordSequence* bBoxCoordSeq = GEOSGeom_getCoordSeq( bBoxRing );

  if ( !bBoxCoordSeq )
    return -1;

  unsigned int nCoords = 0;
  if ( !GEOSCoordSeq_getSize( bBoxCoordSeq, &nCoords ) )
    return -1;

  int maxDigits = -1;
  for ( unsigned int i = 0; i < nCoords - 1; ++i )
  {
    double t;
    GEOSCoordSeq_getX( bBoxCoordSeq, i, &t );

    int digits;
    digits = ceil( log10( fabs( t ) ) );
    if ( digits > maxDigits )
      maxDigits = digits;

    GEOSCoordSeq_getY( bBoxCoordSeq, i, &t );
    digits = ceil( log10( fabs( t ) ) );
    if ( digits > maxDigits )
      maxDigits = digits;
  }

  return maxDigits;
}

int QgsGeometry::numberOfGeometries( GEOSGeometry* g ) const
{
  if ( !g )
    return 0;

  int geometryType = GEOSGeomTypeId( g );
  if ( geometryType == GEOS_POINT || geometryType == GEOS_LINESTRING || geometryType == GEOS_LINEARRING
       || geometryType == GEOS_POLYGON )
    return 1;

  //calling GEOSGetNumGeometries is save for multi types and collections also in geos2
  return GEOSGetNumGeometries( g );
}

int QgsGeometry::mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult )
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 1;

  //convert mGeos to geometry collection
  int type = GEOSGeomTypeId( mGeos );
  if ( type != GEOS_GEOMETRYCOLLECTION &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_MULTIPOINT )
    return 0;

  QVector<GEOSGeometry*> copyList = splitResult;
  splitResult.clear();

  //collect all the geometries that belong to the initial multifeature
  QVector<GEOSGeometry*> unionGeom;

  for ( int i = 0; i < copyList.size(); ++i )
  {
    //is this geometry a part of the original multitype?
    bool isPart = false;
    for ( int j = 0; j < GEOSGetNumGeometries( mGeos ); j++ )
    {
      if ( GEOSEquals( copyList[i], GEOSGetGeometryN( mGeos, j ) ) )
      {
        isPart = true;
        break;
      }
    }

    if ( isPart )
    {
      unionGeom << copyList[i];
    }
    else
    {
      QVector<GEOSGeometry*> geomVector;
      geomVector << copyList[i];

      if ( type == GEOS_MULTILINESTRING )
        splitResult << createGeosCollection( GEOS_MULTILINESTRING, geomVector );
      else if ( type == GEOS_MULTIPOLYGON )
        splitResult << createGeosCollection( GEOS_MULTIPOLYGON, geomVector );
      else
        GEOSGeom_destroy( copyList[i] );
    }
  }

  //make multifeature out of unionGeom
  if ( unionGeom.size() > 0 )
  {
    if ( type == GEOS_MULTILINESTRING )
      splitResult << createGeosCollection( GEOS_MULTILINESTRING, unionGeom );
    else if ( type == GEOS_MULTIPOLYGON )
      splitResult << createGeosCollection( GEOS_MULTIPOLYGON, unionGeom );
  }
  else
  {
    unionGeom.clear();
  }

  return 0;
}

QgsPoint QgsGeometry::asPoint( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const
{
  wkbPtr += 1 + sizeof( int );

  double x, y;
  wkbPtr >> x >> y;
  if ( hasZValue )
    wkbPtr += sizeof( double );

  return QgsPoint( x, y );
}

QgsPolyline QgsGeometry::asPolyline( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const
{
  wkbPtr += 1 + sizeof( int );

  unsigned int nPoints;
  wkbPtr >> nPoints;

  QgsPolyline line( nPoints );

  // Extract the points from the WKB format into the x and y vectors.
  for ( uint i = 0; i < nPoints; ++i )
  {
    double x, y;
    wkbPtr >> x >> y;
    if ( hasZValue )
      wkbPtr += sizeof( double );

    line[i] = QgsPoint( x, y );
  }

  return line;
}

QgsPolygon QgsGeometry::asPolygon( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const
{
  wkbPtr += 1 + sizeof( int );

  // get number of rings in the polygon
  unsigned int numRings;
  wkbPtr >> numRings;

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return QgsPolygon();

  QgsPolygon rings( numRings );

  for ( uint idx = 0; idx < numRings; idx++ )
  {
    int nPoints;
    wkbPtr >> nPoints;

    QgsPolyline ring( nPoints );

    for ( int jdx = 0; jdx < nPoints; jdx++ )
    {
      double x, y;
      wkbPtr >> x >> y;
      if ( hasZValue )
        wkbPtr += sizeof( double );

      ring[jdx] = QgsPoint( x, y );
    }

    rings[idx] = ring;
  }

  return rings;
}

QgsPoint QgsGeometry::asPoint() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBPoint && type != QGis::WKBPoint25D )
    return QgsPoint( 0, 0 );

  QgsConstWkbPtr wkbPtr( mGeometry );
  return asPoint( wkbPtr, type == QGis::WKBPoint25D );
}

QgsPolyline QgsGeometry::asPolyline() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBLineString && type != QGis::WKBLineString25D )
    return QgsPolyline();

  QgsConstWkbPtr wkbPtr( mGeometry );
  return asPolyline( wkbPtr, type == QGis::WKBLineString25D );
}

QgsPolygon QgsGeometry::asPolygon() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBPolygon && type != QGis::WKBPolygon25D )
    return QgsPolygon();

  QgsConstWkbPtr wkbPtr( mGeometry );
  return asPolygon( wkbPtr, type == QGis::WKBPolygon25D );
}

QgsMultiPoint QgsGeometry::asMultiPoint() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBMultiPoint && type != QGis::WKBMultiPoint25D )
    return QgsMultiPoint();

  bool hasZValue = ( type == QGis::WKBMultiPoint25D );

  QgsConstWkbPtr wkbPtr( mGeometry + 1 + sizeof( int ) );

  int nPoints;
  wkbPtr >> nPoints;

  QgsMultiPoint points( nPoints );
  for ( int i = 0; i < nPoints; i++ )
  {
    points[i] = asPoint( wkbPtr, hasZValue );
  }

  return points;
}

QgsMultiPolyline QgsGeometry::asMultiPolyline() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBMultiLineString && type != QGis::WKBMultiLineString25D )
    return QgsMultiPolyline();

  bool hasZValue = ( type == QGis::WKBMultiLineString25D );

  QgsConstWkbPtr wkbPtr( mGeometry + 1 + sizeof( int ) );

  int numLineStrings;
  wkbPtr >> numLineStrings;

  QgsMultiPolyline lines( numLineStrings );

  for ( int i = 0; i < numLineStrings; i++ )
    lines[i] = asPolyline( wkbPtr, hasZValue );

  return lines;
}

QgsMultiPolygon QgsGeometry::asMultiPolygon() const
{
  QGis::WkbType type = wkbType();
  if ( type != QGis::WKBMultiPolygon && type != QGis::WKBMultiPolygon25D )
    return QgsMultiPolygon();

  bool hasZValue = ( type == QGis::WKBMultiPolygon25D );

  QgsConstWkbPtr wkbPtr( mGeometry + 1 + sizeof( int ) );

  int numPolygons;
  wkbPtr >> numPolygons;

  QgsMultiPolygon polygons( numPolygons );

  for ( int i = 0; i < numPolygons; i++ )
    polygons[i] = asPolygon( wkbPtr, hasZValue );

  return polygons;
}

double QgsGeometry::area()
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return -1.0;

  double area;

  try
  {
    if ( GEOSArea( mGeos, &area ) == 0 )
      return -1.0;
  }
  CATCH_GEOS( -1.0 )

  return area;
}

double QgsGeometry::length()
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return -1.0;

  double length;

  try
  {
    if ( GEOSLength( mGeos, &length ) == 0 )
      return -1.0;
  }
  CATCH_GEOS( -1.0 )

  return length;
}
double QgsGeometry::distance( QgsGeometry& geom )
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geom.mDirtyGeos )
    geom.exportWkbToGeos();

  if ( !mGeos || !geom.mGeos )
    return -1.0;

  double dist = -1.0;

  try
  {
    GEOSDistance( mGeos, geom.mGeos, &dist );
  }
  CATCH_GEOS( -1.0 )

  return dist;
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments )
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSBuffer( mGeos, distance, segments ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry*QgsGeometry::buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit )
{
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
 ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSBufferWithStyle( mGeos, distance, segments, endCapStyle, joinStyle, mitreLimit ) );
  }
  CATCH_GEOS( 0 )
#else
  return 0;
#endif
}

QgsGeometry* QgsGeometry::offsetCurve( double distance, int segments, int joinStyle, double mitreLimit )
{
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
 ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos || this->type() != QGis::Line )
    return 0;

  try
  {
    return fromGeosGeom( GEOSOffsetCurve( mGeos, distance, segments, joinStyle, mitreLimit ) );
  }
  CATCH_GEOS( 0 )
#else
  return 0;
#endif
}

QgsGeometry* QgsGeometry::simplify( double tolerance )
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSTopologyPreserveSimplify( mGeos, tolerance ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::centroid()
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSGetCentroid( mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::pointOnSurface()
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSPointOnSurface( mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::convexHull()
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSConvexHull( mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::interpolate( double distance )
{
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=2)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSInterpolate( mGeos, distance ) );
  }
  CATCH_GEOS( 0 )
#else
  QgsMessageLog::logMessage( QObject::tr( "GEOS prior to 3.2 doesn't support GEOSInterpolate" ), QObject::tr( "GEOS" ) );
#endif
}

QgsGeometry* QgsGeometry::intersection( QgsGeometry* geometry )
{
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSIntersection( mGeos, geometry->mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::combine( QgsGeometry* geometry )
{
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    GEOSGeometry* unionGeom = GEOSUnion( mGeos, geometry->mGeos );
    if ( !unionGeom )
      return 0;

    if ( type() == QGis::Line )
    {
      GEOSGeometry* mergedGeom = GEOSLineMerge( unionGeom );
      if ( mergedGeom )
      {
        GEOSGeom_destroy( unionGeom );
        unionGeom = mergedGeom;
      }
    }
    return fromGeosGeom( unionGeom );
  }
  CATCH_GEOS( new QgsGeometry( *this ) ) //return this geometry if union not possible
}

QgsGeometry* QgsGeometry::difference( QgsGeometry* geometry )
{
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSDifference( mGeos, geometry->mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QgsGeometry* QgsGeometry::symDifference( QgsGeometry* geometry )
{
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSSymDifference( mGeos, geometry->mGeos ) );
  }
  CATCH_GEOS( 0 )
}

QList<QgsGeometry*> QgsGeometry::asGeometryCollection() const
{
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return QList<QgsGeometry*>();

  int type = GEOSGeomTypeId( mGeos );
  QgsDebugMsg( "geom type: " + QString::number( type ) );

  QList<QgsGeometry*> geomCollection;

  if ( type != GEOS_MULTIPOINT &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_GEOMETRYCOLLECTION )
  {
    // we have a single-part geometry - put there a copy of this one
    geomCollection.append( new QgsGeometry( *this ) );
    return geomCollection;
  }

  int count = GEOSGetNumGeometries( mGeos );
  QgsDebugMsg( "geom count: " + QString::number( count ) );

  for ( int i = 0; i < count; ++i )
  {
    const GEOSGeometry * geometry = GEOSGetGeometryN( mGeos, i );
    geomCollection.append( fromGeosGeom( GEOSGeom_clone( geometry ) ) );
  }

  return geomCollection;
}

bool QgsGeometry::deleteRing( int ringNum, int partNum )
{
  if ( ringNum <= 0 || partNum < 0 )
    return false;

  switch ( wkbType() )
  {
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      if ( partNum != 0 )
        return false;

      QgsPolygon polygon = asPolygon();
      if ( ringNum >= polygon.count() )
        return false;

      polygon.remove( ringNum );

      QgsGeometry* g2 = QgsGeometry::fromPolygon( polygon );
      *this = *g2;
      delete g2;
      return true;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = asMultiPolygon();

      if ( partNum >= mpolygon.count() )
        return false;

      if ( ringNum >= mpolygon[partNum].count() )
        return false;

      mpolygon[partNum].remove( ringNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolygon( mpolygon );
      *this = *g2;
      delete g2;
      return true;
    }

    default:
      return false; // only makes sense with polygons and multipolygons
  }
}

bool QgsGeometry::deletePart( int partNum )
{
  if ( partNum < 0 )
    return false;

  switch ( wkbType() )
  {
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      QgsMultiPoint mpoint = asMultiPoint();

      if ( partNum >= mpoint.size() || mpoint.size() == 1 )
        return false;

      mpoint.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPoint( mpoint );
      *this = *g2;
      delete g2;
      break;
    }

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      QgsMultiPolyline mline = asMultiPolyline();

      if ( partNum >= mline.size() || mline.size() == 1 )
        return false;

      mline.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolyline( mline );
      *this = *g2;
      delete g2;
      break;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = asMultiPolygon();

      if ( partNum >= mpolygon.size() || mpolygon.size() == 1 )
        return false;

      mpolygon.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolygon( mpolygon );
      *this = *g2;
      delete g2;
      break;
    }

    default:
      // single part geometries are ignored
      return false;
  }

  return true;
}

/** Return union of several geometries - try to use unary union if available (GEOS >= 3.3) otherwise use a cascade of unions.
 *  Takes ownership of passed geometries, returns a new instance */
static GEOSGeometry* _makeUnion( QList<GEOSGeometry*> geoms )
{
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && (((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)) || (GEOS_VERSION_MAJOR>3))
  GEOSGeometry* geomCollection = 0;
  geomCollection = createGeosCollection( GEOS_GEOMETRYCOLLECTION, geoms.toVector() );
  GEOSGeometry* geomUnion = GEOSUnaryUnion( geomCollection );
  GEOSGeom_destroy( geomCollection );
  return geomUnion;
#else
  GEOSGeometry* geomCollection = geoms.takeFirst();

  while ( !geoms.isEmpty() )
  {
    GEOSGeometry* g = geoms.takeFirst();
    GEOSGeometry* geomCollectionNew = GEOSUnion( geomCollection, g );
    GEOSGeom_destroy( geomCollection );
    GEOSGeom_destroy( g );
    geomCollection = geomCollectionNew;
  }

  return geomCollection;
#endif
}

int QgsGeometry::avoidIntersections( QMap<QgsVectorLayer*, QSet< QgsFeatureId > > ignoreFeatures )
{
  int returnValue = 0;

  //check if g has polygon type
  if ( type() != QGis::Polygon )
    return 1;

  QGis::WkbType geomTypeBeforeModification = wkbType();

  //read avoid intersections list from project properties
  bool listReadOk;
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", QStringList(), &listReadOk );
  if ( !listReadOk )
    return true; //no intersections stored in project does not mean error

  QList<GEOSGeometry*> nearGeometries;

  //go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
  QgsVectorLayer* currentLayer = 0;
  QStringList::const_iterator aIt = avoidIntersectionsList.constBegin();
  for ( ; aIt != avoidIntersectionsList.constEnd(); ++aIt )
  {
    currentLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( *aIt ) );
    if ( currentLayer )
    {
      QgsFeatureIds ignoreIds;
      QMap<QgsVectorLayer*, QSet<qint64> >::const_iterator ignoreIt = ignoreFeatures.find( currentLayer );
      if ( ignoreIt != ignoreFeatures.constEnd() )
        ignoreIds = ignoreIt.value();

      QgsFeatureIterator fi = currentLayer->getFeatures( QgsFeatureRequest( boundingBox() )
                              .setFlags( QgsFeatureRequest::ExactIntersect )
                              .setSubsetOfAttributes( QgsAttributeList() ) );
      QgsFeature f;
      while ( fi.nextFeature( f ) )
      {
        if ( ignoreIds.contains( f.id() ) )
          continue;

        if ( !f.geometry() )
          continue;

        nearGeometries << GEOSGeom_clone( f.geometry()->asGeos() );
      }
    }
  }

  if ( nearGeometries.isEmpty() )
    return 0;

  GEOSGeometry* nearGeometriesUnion = 0;
  GEOSGeometry* geomWithoutIntersections = 0;
  try
  {
    nearGeometriesUnion = _makeUnion( nearGeometries );
    geomWithoutIntersections = GEOSDifference( asGeos(), nearGeometriesUnion );

    fromGeos( geomWithoutIntersections );

    GEOSGeom_destroy( nearGeometriesUnion );
  }
  catch ( GEOSException &e )
  {
    if ( nearGeometriesUnion )
      GEOSGeom_destroy( nearGeometriesUnion );
    if ( geomWithoutIntersections )
      GEOSGeom_destroy( geomWithoutIntersections );

    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 3;
  }

  //make sure the geometry still has the same type (e.g. no change from polygon to multipolygon)
  if ( wkbType() != geomTypeBeforeModification )
    return 2;

  return returnValue;
}

void QgsGeometry::validateGeometry( QList<Error> &errors )
{
  QgsGeometryValidator::validateGeometry( this, errors );
}

bool QgsGeometry::isGeosValid()
{
  try
  {
    const GEOSGeometry *g = asGeos();

    if ( !g )
      return false;

    return GEOSisValid( g );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
}

bool QgsGeometry::isGeosEqual( QgsGeometry &g )
{
  return geosRelOp( GEOSEquals, this, &g );
}

bool QgsGeometry::isGeosEmpty()
{
  try
  {
    const GEOSGeometry *g = asGeos();

    if ( !g )
      return false;

    return GEOSisEmpty( g );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
}

double QgsGeometry::leftOf( double x, double y, double& x1, double& y1, double& x2, double& y2 )
{
  double f1 = x - x1;
  double f2 = y2 - y1;
  double f3 = y - y1;
  double f4 = x2 - x1;
  return f1*f2 - f3*f4;
}

QgsGeometry* QgsGeometry::convertToPoint( bool destMultipart )
{
  switch ( type() )
  {
    case QGis::Point:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && !srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // layer is multipart => make a multipoint with a single point
        return fromMultiPoint( QgsMultiPoint() << asPoint() );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPoint multiPoint = asMultiPoint();
        if ( multiPoint.count() == 1 )
        {
          return fromPoint( multiPoint[0] );
        }
      }
      return 0;
    }

    case QGis::Line:
    {
      // only possible if destination is multipart
      if ( !destMultipart )
        return 0;

      // input geometry is multipart
      if ( isMultipart() )
      {
        QgsMultiPolyline multiLine = asMultiPolyline();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolyline::const_iterator multiLineIt = multiLine.constBegin(); multiLineIt != multiLine.constEnd(); ++multiLineIt )
          for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
            multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
      // input geometry is not multipart: copy directly the line into a multipoint
      else
      {
        QgsPolyline line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPoint( line );
      }
      return 0;
    }

    case QGis::Polygon:
    {
      // can only transfrom if destination is multipoint
      if ( !destMultipart )
        return 0;

      // input geometry is multipart: make a multipoint from multipolygon
      if ( isMultipart() )
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolygon::const_iterator polygonIt = multiPolygon.constBegin(); polygonIt != multiPolygon.constEnd(); ++polygonIt )
          for ( QgsMultiPolyline::const_iterator multiLineIt = ( *polygonIt ).constBegin(); multiLineIt != ( *polygonIt ).constEnd(); ++multiLineIt )
            for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
              multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
      // input geometry is not multipart: make a multipoint from polygon
      else
      {
        QgsPolygon polygon = asPolygon();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolyline::const_iterator multiLineIt = polygon.constBegin(); multiLineIt != polygon.constEnd(); ++multiLineIt )
          for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
            multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
    }

    default:
      return 0;
  }
}

QgsGeometry* QgsGeometry::convertToLine( bool destMultipart )
{
  switch ( type() )
  {
    case QGis::Point:
    {
      if ( !isMultipart() )
        return 0;

      QgsMultiPoint multiPoint = asMultiPoint();
      if ( multiPoint.count() < 2 )
        return 0;

      if ( destMultipart )
        return fromMultiPolyline( QgsMultiPolyline() << multiPoint );
      else
        return fromPolyline( multiPoint );
    }

    case QGis::Line:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single line
        QgsPolyline line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPolyline( QgsMultiPolyline() << line );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPolyline multiLine = asMultiPolyline();
        if ( multiLine.count() == 1 )
          return fromPolyline( multiLine[0] );
      }
      return 0;
    }

    case QGis::Polygon:
    {
      // input geometry is multipolygon
      if ( isMultipart() )
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        QgsMultiPolyline multiLine;
        for ( QgsMultiPolygon::const_iterator polygonIt = multiPolygon.constBegin(); polygonIt != multiPolygon.constEnd(); ++polygonIt )
          for ( QgsMultiPolyline::const_iterator multiLineIt = ( *polygonIt ).constBegin(); multiLineIt != ( *polygonIt ).constEnd(); ++multiLineIt )
            multiLine << *multiLineIt;

        if ( destMultipart )
        {
          // destination is multipart
          return fromMultiPolyline( multiLine );
        }
        else if ( multiLine.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolyline( multiLine[0] );
        }
      }
      // input geometry is single polygon
      else
      {
        QgsPolygon polygon = asPolygon();
        // if polygon has rings
        if ( polygon.count() > 1 )
        {
          // cannot fit a polygon with rings in a single line layer
          // TODO: would it be better to remove rings?
          if ( destMultipart )
          {
            QgsPolygon polygon = asPolygon();
            QgsMultiPolyline multiLine;
            for ( QgsMultiPolyline::const_iterator multiLineIt = polygon.constBegin(); multiLineIt != polygon.constEnd(); ++multiLineIt )
              multiLine << *multiLineIt;
            return fromMultiPolyline( multiLine );
          }
        }
        // no rings
        else if ( polygon.count() == 1 )
        {
          if ( destMultipart )
          {
            return fromMultiPolyline( polygon );
          }
          else
          {
            return fromPolyline( polygon[0] );
          }
        }
      }
      return 0;
    }

    default:
      return 0;
  }
}

QgsGeometry* QgsGeometry::convertToPolygon( bool destMultipart )
{
  switch ( type() )
  {
    case QGis::Point:
    {
      if ( !isMultipart() )
        return 0;

      QgsMultiPoint multiPoint = asMultiPoint();
      if ( multiPoint.count() < 3 )
        return 0;

      if ( multiPoint.last() != multiPoint.first() )
        multiPoint << multiPoint.first();

      QgsPolygon polygon = QgsPolygon() << multiPoint;
      if ( destMultipart )
        return fromMultiPolygon( QgsMultiPolygon() << polygon );
      else
        return fromPolygon( polygon );
    }

    case QGis::Line:
    {
      // input geometry is multiline
      if ( isMultipart() )
      {
        QgsMultiPolyline multiLine = asMultiPolyline();
        QgsMultiPolygon multiPolygon;
        for ( QgsMultiPolyline::iterator multiLineIt = multiLine.begin(); multiLineIt != multiLine.end(); ++multiLineIt )
        {
          // do not create polygon for a 1 segment line
          if (( *multiLineIt ).count() < 3 )
            return 0;
          if (( *multiLineIt ).count() == 3 && ( *multiLineIt ).first() == ( *multiLineIt ).last() )
            return 0;

          // add closing node
          if (( *multiLineIt ).first() != ( *multiLineIt ).last() )
            *multiLineIt << ( *multiLineIt ).first();
          multiPolygon << ( QgsPolygon() << *multiLineIt );
        }
        // check that polygons were inserted
        if ( !multiPolygon.isEmpty() )
        {
          if ( destMultipart )
          {
            return fromMultiPolygon( multiPolygon );
          }
          else if ( multiPolygon.count() == 1 )
          {
            // destination is singlepart => make a single part if possible
            return fromPolygon( multiPolygon[0] );
          }
        }
      }
      // input geometry is single line
      else
      {
        QgsPolyline line = asPolyline();

        // do not create polygon for a 1 segment line
        if ( line.count() < 3 )
          return 0;
        if ( line.count() == 3 && line.first() == line.last() )
          return 0;

        // add closing node
        if ( line.first() != line.last() )
          line << line.first();

        // destination is multipart
        if ( destMultipart )
        {
          return fromMultiPolygon( QgsMultiPolygon() << ( QgsPolygon() << line ) );
        }
        else
        {
          return fromPolygon( QgsPolygon() << line );
        }
      }
      return 0;
    }

    case QGis::Polygon:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single polygon
        QgsPolygon polygon = asPolygon();
        if ( !polygon.isEmpty() )
          return fromMultiPolygon( QgsMultiPolygon() << polygon );
      }
      else
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        if ( multiPolygon.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolygon( multiPolygon[0] );
        }
      }
      return 0;
    }

    default:
      return 0;
  }
}

QgsGeometry *QgsGeometry::unaryUnion( const QList<QgsGeometry*> &geometryList )
{
  QList<GEOSGeometry*> geoms;
  foreach ( QgsGeometry* g, geometryList )
  {
    geoms.append( GEOSGeom_clone( g->asGeos() ) );
  }
  GEOSGeometry *geomUnion = _makeUnion( geoms );
  QgsGeometry *ret = new QgsGeometry();
  ret->fromGeos( geomUnion );
  return ret;
}
