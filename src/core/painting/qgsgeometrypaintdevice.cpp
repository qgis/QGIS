/***************************************************************************
  qgsgeometrypaintdevice.cpp
  --------------------------------------
  Date                 : May 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrypaintdevice.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsgeos.h"
#include "qgsmultipolygon.h"
#include "qgsmultilinestring.h"
#include "qgspainting.h"

//
// QgsGeometryPaintEngine
//

QgsGeometryPaintEngine::QgsGeometryPaintEngine( bool usePathStroker )
  : QPaintEngine( QPaintEngine::AllFeatures ) // we lie and say we support all paint features, as we don't want Qt trying to be helpful and rasterizing shapes
  , mUsePathStroker( usePathStroker )
{
}

void QgsGeometryPaintEngine::setStrokedPathSegments( int segments )
{
  mStrokedPathsSegments = segments;
}

void QgsGeometryPaintEngine::setSimplificationTolerance( double tolerance )
{
  mSimplifyTolerance = tolerance;
}

bool QgsGeometryPaintEngine::begin( QPaintDevice * )
{
  return true;
}

bool QgsGeometryPaintEngine::end()
{
  return true;
}

QPaintEngine::Type QgsGeometryPaintEngine::type() const
{
  return QPaintEngine::User;
}

void QgsGeometryPaintEngine::updateState( const QPaintEngineState &state )
{
  if ( mUsePathStroker && state.state().testFlag( QPaintEngine::DirtyFlag::DirtyPen ) )
  {
    mPen = state.pen();
  }
}

void QgsGeometryPaintEngine::drawImage( const QRectF &, const QImage &, const QRectF &, Qt::ImageConversionFlags )
{
  // ignore, we don't need to support raster drawing
  QgsDebugError( QStringLiteral( "QgsGeometryPaintEngine does not support drawImage method" ) );
}

void QgsGeometryPaintEngine::drawPixmap( const QRectF &, const QPixmap &, const QRectF & )
{
  // ignore, we don't need to support raster drawing
  QgsDebugError( QStringLiteral( "QgsGeometryPaintEngine does not support drawPixmap method" ) );
}

void QgsGeometryPaintEngine::drawTiledPixmap( const QRectF &, const QPixmap &, const QPointF & )
{
  // ignore, we don't need to support raster drawing
  QgsDebugError( QStringLiteral( "QgsGeometryPaintEngine does not support drawTiledPixmap method" ) );
}

template <typename T>
void drawLinesImp( const QTransform &transform, QgsGeometryCollection &geometry, const T *lines, int lineCount )
{
  geometry.reserve( geometry.numGeometries() + lineCount );
  if ( transform.isIdentity() )
  {
    for ( int i = 0; i < lineCount; ++i, ++lines )
    {
      geometry.addGeometry( new QgsLineString(
                              QVector<double> { static_cast< double >( lines->x1() ), static_cast< double >( lines->x2() ) },
                              QVector<double> { static_cast< double >( lines->y1() ), static_cast< double >( lines->y2() ) } )
                          );
    }
  }
  else
  {
    for ( int i = 0; i < lineCount; ++i, ++lines )
    {
      double x1 = lines->x1();
      double x2 = lines->x2();
      double y1 = lines->y1();
      double y2 = lines->y2();

      double tx1, tx2, ty1, ty2;
      transform.map( x1, y1, &tx1, &ty1 );
      transform.map( x2, y2, &tx2, &ty2 );

      geometry.addGeometry( new QgsLineString(
                              QVector<double> { tx1, tx2 },
                              QVector<double> { ty1, ty2 } )
                          );
    }
  }
}

void QgsGeometryPaintEngine::drawLines( const QLineF *lines, int lineCount )
{
  if ( mUsePathStroker )
  {
    // if stroking we have no choice but to go via the QPainterPath route
    QPaintEngine::drawLines( lines, lineCount );
  }
  else
  {
    const QTransform transform = painter()->combinedTransform();
    drawLinesImp( transform, mGeometry, lines, lineCount );
  }
}

void QgsGeometryPaintEngine::drawLines( const QLine *lines, int lineCount )
{
  if ( mUsePathStroker )
  {
    // if stroking we have no choice but to go via the QPainterPath route
    QPaintEngine::drawLines( lines, lineCount );
  }
  else
  {
    const QTransform transform = painter()->combinedTransform();
    drawLinesImp( transform, mGeometry, lines, lineCount );
  }
}

template <typename T>
void drawPointsImp( const QTransform &transform, QgsGeometryCollection &geometry, const T *points, int pointCount )
{
  geometry.reserve( geometry.numGeometries() + pointCount );
  if ( transform.isIdentity() )
  {
    for ( int i = 0; i < pointCount; ++i, ++points )
    {
      geometry.addGeometry( new QgsPoint( static_cast< double >( points->x() ),
                                          static_cast< double >( points->y() ) ) );
    }
  }
  else
  {
    for ( int i = 0; i < pointCount; ++i, ++points )
    {
      double x = points->x();
      double y = points->y();

      double tx, ty;
      transform.map( x, y, &tx, &ty );

      geometry.addGeometry( new QgsPoint( tx, ty ) );
    }
  }
}

void QgsGeometryPaintEngine::drawPoints( const QPointF *points, int pointCount )
{
  const QTransform transform = painter()->combinedTransform();
  drawPointsImp( transform, mGeometry, points, pointCount );
}

void QgsGeometryPaintEngine::drawPoints( const QPoint *points, int pointCount )
{
  const QTransform transform = painter()->combinedTransform();
  drawPointsImp( transform, mGeometry, points, pointCount );
}

template <typename T>
void drawRectsImp( const QTransform &transform, QgsGeometryCollection &geometry, const T *rects, int rectCount )
{
  geometry.reserve( geometry.numGeometries() + rectCount );
  if ( transform.isIdentity() )
  {
    for ( int i = 0; i < rectCount; ++i, ++rects )
    {
      QgsLineString *exterior = new QgsLineString(
        QVector<double> { static_cast< double >( rects->left() ),
                          static_cast< double >( rects->right() ),
                          static_cast< double >( rects->right() ),
                          static_cast< double >( rects->left() ),
                          static_cast< double>( rects->left() )
                        },
        QVector<double> { static_cast< double >( rects->bottom() ),
                          static_cast< double >( rects->bottom() ),
                          static_cast< double >( rects->top() ),
                          static_cast< double >( rects->top() ),
                          static_cast< double >( rects->bottom() )
                        } );
      geometry.addGeometry( new QgsPolygon( exterior ) );
    }
  }
  else
  {
    for ( int i = 0; i < rectCount; ++i, ++rects )
    {
      const double left = rects->left();
      const double right = rects->right();
      const double top = rects->top();
      const double bottom = rects->bottom();

      double bottomLeftX, bottomLeftY, bottomRightX, bottomRightY, topLeftX, topLeftY, topRightX, topRightY;
      transform.map( left, bottom, &bottomLeftX, &bottomLeftY );
      transform.map( right, bottom, &bottomRightX, &bottomRightY );
      transform.map( left, top, &topLeftX, &topLeftY );
      transform.map( right, top, &topRightX, &topRightY );

      QgsLineString *exterior = new QgsLineString(
        QVector<double> { bottomLeftX, bottomRightX, topRightX, topLeftX, bottomLeftX  },
        QVector<double> { bottomLeftY, bottomRightY, topRightY, topLeftY, bottomLeftY } );
      geometry.addGeometry( new QgsPolygon( exterior ) );
    }
  }
}

void QgsGeometryPaintEngine::drawRects( const QRectF *rects, int rectCount )
{
  const QTransform transform = painter()->combinedTransform();
  drawRectsImp( transform, mGeometry, rects, rectCount );
}

void QgsGeometryPaintEngine::drawRects( const QRect *rects, int rectCount )
{
  const QTransform transform = painter()->combinedTransform();
  drawRectsImp( transform, mGeometry, rects, rectCount );
}

template <typename T>
void drawPolygonImp( const QTransform &transform, QgsGeometryCollection &geometry, const T *points, int pointCount, QPaintEngine::PolygonDrawMode mode, double simplifyTolerance )
{
  QVector< double > x;
  QVector< double > y;
  x.resize( pointCount );
  y.resize( pointCount );
  double *xData = x.data();
  double *yData = y.data();

  if ( transform.isIdentity() )
  {
    for ( int i = 0; i < pointCount; ++i, ++points )
    {
      *xData++ = points->x();
      *yData++ = points->y();
    }
  }
  else
  {
    for ( int i = 0; i < pointCount; ++i, ++points )
    {
      const double x = points->x();
      const double y = points->y();
      double tx, ty;
      transform.map( x, y, &tx, &ty );

      *xData++ = tx;
      *yData++ = ty;
    }
  }

  switch ( mode )
  {
    case QPaintEngine::PolylineMode:
      if ( simplifyTolerance > 0 )
        geometry.addGeometry( QgsLineString( x, y ).simplifyByDistance( simplifyTolerance ) );
      else
        geometry.addGeometry( new QgsLineString( x, y ) );
      break;

    case QPaintEngine::OddEvenMode:
    case QPaintEngine::WindingMode:
    case QPaintEngine::ConvexMode:
      if ( simplifyTolerance > 0 )
        geometry.addGeometry( new QgsPolygon( QgsLineString( x, y ).simplifyByDistance( simplifyTolerance ) ) );
      else
        geometry.addGeometry( new QgsPolygon( new QgsLineString( x, y ) ) );
      break;
  }
}

void QgsGeometryPaintEngine::drawPolygon( const QPoint *points, int pointCount, QPaintEngine::PolygonDrawMode mode )
{
  if ( mUsePathStroker && mode == PolygonDrawMode::PolylineMode )
  {
    // if stroking we have no choice but to go via the QPainterPath route
    if ( pointCount > 0 )
    {
      QPainterPath path;
      path.moveTo( *points++ );
      for ( int i = 1; i < pointCount; ++i )
      {
        path.lineTo( *points++ );
      }
      drawPath( path );
    }
  }
  else
  {
    const QTransform transform = painter()->combinedTransform();
    drawPolygonImp( transform, mGeometry, points, pointCount, mode, mSimplifyTolerance );
  }
}

void QgsGeometryPaintEngine::drawPolygon( const QPointF *points, int pointCount, QPaintEngine::PolygonDrawMode mode )
{
  if ( mUsePathStroker )
  {
    // if stroking we have no choice but to go via the QPainterPath route
    if ( pointCount > 0 )
    {
      QPainterPath path;
      path.moveTo( *points++ );
      for ( int i = 1; i < pointCount; ++i )
      {
        path.lineTo( *points++ );
      }
      drawPath( path );
    }
  }
  else
  {
    const QTransform transform = painter()->combinedTransform();
    drawPolygonImp( transform, mGeometry, points, pointCount, mode, mSimplifyTolerance );
  }
}

void QgsGeometryPaintEngine::addStrokedLine( const QgsLineString *line, double penWidth, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit, const QTransform *matrix )
{
  std::unique_ptr< QgsAbstractGeometry > buffered;
  if ( mSimplifyTolerance > 0 )
  {
    // For performance, we apply a lower level of simplification to the line BEFORE doing the buffer.
    // This avoids making the call to GEOS buffer function too expensive, as we'd otherwise be doing it
    // on the unsimplified line and then immediately discarding most of the detail when we simplify
    // the resultant buffer.
    // The 0.75 factor here is just a guess! This could likely be made smarter, eg by considering the pen width?
    const double preBufferedSimplificationFactor = mSimplifyTolerance * 0.75;
    std::unique_ptr< QgsLineString > simplified( line->simplifyByDistance( preBufferedSimplificationFactor ) );
    QgsGeos geos( simplified.get() );
    buffered.reset( geos.buffer( penWidth / 2, mStrokedPathsSegments, endCapStyle, joinStyle, miterLimit ) );
  }
  else
  {
    QgsGeos geos( line );
    buffered.reset( geos.buffer( penWidth / 2, mStrokedPathsSegments, endCapStyle, joinStyle, miterLimit ) );
  }

  if ( !buffered )
    return;

  if ( matrix )
    buffered->transform( *matrix );

  if ( QgsGeometryCollection *bufferedCollection = qgsgeometry_cast< QgsGeometryCollection * >( buffered.get() ) )
  {
    if ( mSimplifyTolerance > 0 )
    {
      for ( auto it = bufferedCollection->const_parts_begin(); it != bufferedCollection->const_parts_end(); ++it )
      {
        mGeometry.addGeometry( ( *it )->simplifyByDistance( mSimplifyTolerance ) );
      }
    }
    else
    {
      mGeometry.addGeometries( bufferedCollection->takeGeometries() );
    }
  }
  else if ( buffered )
  {
    if ( mSimplifyTolerance > 0 )
    {
      mGeometry.addGeometry( buffered->simplifyByDistance( mSimplifyTolerance ) );
    }
    else
    {
      mGeometry.addGeometry( buffered.release() );
    }
  }
}

Qgis::EndCapStyle QgsGeometryPaintEngine::penStyleToCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::FlatCap:
      return Qgis::EndCapStyle::Flat;
    case Qt::SquareCap:
      return Qgis::EndCapStyle::Square;
    case Qt::RoundCap:
      return Qgis::EndCapStyle::Round;
    case Qt::MPenCapStyle:
      // undocumented?
      break;
  }

  return Qgis::EndCapStyle::Round;
}

Qgis::JoinStyle QgsGeometryPaintEngine::penStyleToJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::MiterJoin:
    case Qt::SvgMiterJoin:
      return Qgis::JoinStyle::Miter;
    case Qt::BevelJoin:
      return Qgis::JoinStyle::Bevel;
    case Qt::RoundJoin:
      return Qgis::JoinStyle::Round;
    case Qt::MPenJoinStyle:
      // undocumented?
      break;
  }
  return Qgis::JoinStyle::Round;
}

// based on QPainterPath::toSubpathPolygons()
void QgsGeometryPaintEngine::addSubpathGeometries( const QPainterPath &path, const QTransform &matrix )
{
  if ( path.isEmpty() )
    return;

  const bool transformIsIdentity = matrix.isIdentity();

  const Qgis::EndCapStyle endCapStyle = penStyleToCapStyle( mPen.capStyle() );
  const Qgis::JoinStyle joinStyle = penStyleToJoinStyle( mPen.joinStyle() );
  const double penWidth = mPen.widthF() <= 0 ? 1 : mPen.widthF();
  const double miterLimit = mPen.miterLimit();

  QVector< double > currentX;
  QVector< double > currentY;
  const int count = path.elementCount();

  // polygon parts get queued and post-processed before adding them to the collection
  std::vector< std::unique_ptr< QgsPolygon > > queuedPolygons;

  for ( int i = 0; i < count; ++i )
  {
    const QPainterPath::Element &e = path.elementAt( i );
    switch ( e.type )
    {
      case QPainterPath::MoveToElement:
      {
        if ( currentX.size() > 1 )
        {
          std::unique_ptr< QgsLineString > line = std::make_unique< QgsLineString >( currentX, currentY );
          if ( mUsePathStroker )
          {
            addStrokedLine( line.get(), penWidth, endCapStyle, joinStyle, miterLimit, transformIsIdentity ? nullptr : &matrix );
          }
          else if ( line->isClosed() )
          {
            if ( !transformIsIdentity )
              line->transform( matrix );

            if ( mSimplifyTolerance > 0 )
            {
              queuedPolygons.emplace_back( std::make_unique< QgsPolygon >( line->simplifyByDistance( mSimplifyTolerance ) ) );
              line.reset();
            }
            else
            {
              queuedPolygons.emplace_back( std::make_unique< QgsPolygon >( line.release() ) );
            }
          }
          else
          {
            if ( !transformIsIdentity )
              line->transform( matrix );
            if ( mSimplifyTolerance > 0 )
            {
              mGeometry.addGeometry( line->simplifyByDistance( mSimplifyTolerance ) );
              line.reset();
            }
            else
            {
              mGeometry.addGeometry( line.release() );
            }
          }
        }
        currentX.resize( 0 );
        currentY.resize( 0 );

        currentX.reserve( 16 );
        currentY.reserve( 16 );
        currentX << e.x;
        currentY << e.y;
        break;
      }

      case QPainterPath::LineToElement:
      {
        currentX << e.x;
        currentY << e.y;
        break;
      }

      case QPainterPath::CurveToElement:
      {
        Q_ASSERT( path.elementAt( i + 1 ).type == QPainterPath::CurveToDataElement );
        Q_ASSERT( path.elementAt( i + 2 ).type == QPainterPath::CurveToDataElement );

        const double x1 = path.elementAt( i - 1 ).x;
        const double y1 = path.elementAt( i - 1 ).y;

        const double x3 = path.elementAt( i + 1 ).x;
        const double y3 = path.elementAt( i + 1 ).y;

        const double x4 = path.elementAt( i + 2 ).x;
        const double y4 = path.elementAt( i + 2 ).y;

        // TODO -- we could likely reduce the number of segmented points here!
        std::unique_ptr< QgsLineString> bezier( QgsLineString::fromBezierCurve(
            QgsPoint( x1, y1 ),
            QgsPoint( e.x, e.y ),
            QgsPoint( x3, y3 ),
            QgsPoint( x4, y4 ) ) );

        currentX << bezier->xVector();
        currentY << bezier->yVector();

        i += 2;
        break;
      }
      case QPainterPath::CurveToDataElement:
        Q_ASSERT( !"addSubpathGeometries(), bad element type" );
        break;
    }
  }

  if ( currentX.size() > 1 )
  {
    std::unique_ptr< QgsLineString > line = std::make_unique< QgsLineString >( currentX, currentY );
    if ( mUsePathStroker )
    {
      addStrokedLine( line.get(), penWidth, endCapStyle, joinStyle, miterLimit, transformIsIdentity ? nullptr : &matrix );
    }
    else if ( line->isClosed() )
    {
      if ( !transformIsIdentity )
        line->transform( matrix );
      if ( mSimplifyTolerance > 0 )
      {
        queuedPolygons.emplace_back( std::make_unique< QgsPolygon >( line->simplifyByDistance( mSimplifyTolerance ) ) );
        line.reset();
      }
      else
      {
        queuedPolygons.emplace_back( std::make_unique< QgsPolygon >( line.release() ) );
      }
    }
    else
    {
      if ( !transformIsIdentity )
        line->transform( matrix );
      if ( mSimplifyTolerance > 0 )
      {
        mGeometry.addGeometry( line->simplifyByDistance( mSimplifyTolerance ) );
        line.reset();
      }
      else
      {
        mGeometry.addGeometry( line.release() );
      }
    }
  }

  if ( queuedPolygons.empty() )
    return;

  mGeometry.reserve( static_cast< int >( mGeometry.numGeometries() + queuedPolygons.size() ) );

  QgsMultiPolygon tempMultiPolygon;
  tempMultiPolygon.reserve( static_cast< int >( queuedPolygons.size() ) );
  for ( auto &part : queuedPolygons )
  {
    tempMultiPolygon.addGeometry( part.release() );
  }

  // ensure holes are holes, not overlapping polygons
  QgsGeos geosCollection( &tempMultiPolygon );
  std::unique_ptr< QgsAbstractGeometry > g = geosCollection.makeValid( Qgis::MakeValidMethod::Linework );
  if ( !g )
    return;

  for ( auto it = g->const_parts_begin(); it != g->const_parts_end(); ++it )
  {
    mGeometry.addGeometry( ( *it )->clone() );
  }
}

void QgsGeometryPaintEngine::drawPath( const QPainterPath &path )
{
  const QTransform transform = painter()->combinedTransform();
  addSubpathGeometries( path, transform );
}


//
// QgsGeometryPaintDevice
//

QgsGeometryPaintDevice::QgsGeometryPaintDevice( bool usePathStroker )
{
  mPaintEngine = std::make_unique<QgsGeometryPaintEngine>( usePathStroker );
}

void QgsGeometryPaintDevice::setStrokedPathSegments( int segments )
{
  if ( mPaintEngine )
    mPaintEngine->setStrokedPathSegments( segments );
}

void QgsGeometryPaintDevice::setSimplificationTolerance( double tolerance )
{
  if ( mPaintEngine )
    mPaintEngine->setSimplificationTolerance( tolerance );
}

QPaintEngine *QgsGeometryPaintDevice::paintEngine() const
{
  return mPaintEngine.get();
}

int QgsGeometryPaintDevice::metric( PaintDeviceMetric m ) const
{
  // copy/paste from qpicture.cpp
  int val;

  switch ( m )
  {
    case PdmWidth:
      val = static_cast< int >( mPaintEngine->geometry().boundingBox().width() );
      break;
    case PdmHeight:
      val = static_cast< int >( mPaintEngine->geometry().boundingBox().height() );
      break;
    case PdmWidthMM:
      val = static_cast< int >( 25.4 / QgsPainting::qtDefaultDpiX() * mPaintEngine->geometry().boundingBox().width() );
      break;
    case PdmHeightMM:
      val = static_cast< int >( 25.4 / QgsPainting::qtDefaultDpiY() * mPaintEngine->geometry().boundingBox().height() );
      break;
    case PdmDpiX:
    case PdmPhysicalDpiX:
      val = QgsPainting::qtDefaultDpiX();
      break;
    case PdmDpiY:
    case PdmPhysicalDpiY:
      val = QgsPainting::qtDefaultDpiY();
      break;
    case PdmNumColors:
      val = 16777216;
      break;
    case PdmDepth:
      val = 24;
      break;
    case PdmDevicePixelRatio:
      val = 1;
      break;
    case PdmDevicePixelRatioScaled:
      val = static_cast< int >( 1 * QPaintDevice::devicePixelRatioFScale() );
      break;
    default:
      val = 0;
      qWarning( "QPicture::metric: Invalid metric command" );
  }
  return val;
}

const QgsAbstractGeometry &QgsGeometryPaintDevice::geometry() const
{
  return mPaintEngine->geometry();
}

QgsGeometry QgsGeometryPaintDevice::painterPathToGeometry( const QPainterPath &path )
{
  QgsGeometryPaintDevice device;
  QPainter painter( &device );
  painter.drawPath( path );
  painter.end();
  return QgsGeometry( device.geometry().clone() );
}

