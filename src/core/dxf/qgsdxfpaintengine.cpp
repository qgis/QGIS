/***************************************************************************
                         qgsdxpaintengine.cpp
                         --------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfpaintengine.h"
#include "qgsdxfexport.h"
#include "qgsdxfpaintdevice.h"
#include "qgslogger.h"

QgsDxfPaintEngine::QgsDxfPaintEngine( const QgsDxfPaintDevice *dxfDevice, QgsDxfExport *dxf )
  : QPaintEngine( QPaintEngine::AllFeatures /*QPaintEngine::PainterPaths | QPaintEngine::PaintOutsidePaintEvent*/ )
  , mPaintDevice( dxfDevice )
  , mDxf( dxf )
{
}

bool QgsDxfPaintEngine::begin( QPaintDevice *pdev )
{
  Q_UNUSED( pdev )
  return true;
}

bool QgsDxfPaintEngine::end()
{
  return true;
}

QPaintEngine::Type QgsDxfPaintEngine::type() const
{
  return QPaintEngine::User;
}

void QgsDxfPaintEngine::drawPixmap( const QRectF &r, const QPixmap &pm, const QRectF &sr )
{
  Q_UNUSED( r )
  Q_UNUSED( pm )
  Q_UNUSED( sr )
}

void QgsDxfPaintEngine::updateState( const QPaintEngineState &state )
{
  if ( state.state() & QPaintEngine::DirtyTransform )
    mTransform = state.transform();

  if ( state.state() & QPaintEngine::DirtyPen )
    mPen = state.pen();

  if ( state.state() & QPaintEngine::DirtyBrush )
    mBrush = state.brush();

  if ( state.state() & QPaintEngine::DirtyOpacity )
  {
    mOpacity = state.opacity();
  }
}

void QgsDxfPaintEngine::setRing( QgsPointSequence &polyline, const QPointF *points, int pointCount )
{
  polyline.clear();
  for ( int i = 0; i < pointCount; ++i )
    polyline.append( toDxfCoordinates( points[i] ) );
}

void QgsDxfPaintEngine::drawPolygon( const QPointF *points, int pointCount, PolygonDrawMode mode )
{
  Q_UNUSED( mode )
  if ( !mDxf || !mPaintDevice )
    return;

  QgsRingSequence polygon;
  polygon << QgsPointSequence();
  setRing( polygon.last(), points, pointCount );

  if ( mode == QPaintEngine::PolylineMode )
  {
    if ( mPen.style() != Qt::NoPen && mPen.brush().style() != Qt::NoBrush )
      mDxf->writePolyline( polygon.at( 0 ), mLayer, QStringLiteral( "CONTINUOUS" ), penColor(), currentWidth() );
  }
  else
  {
    if ( mBrush.style() != Qt::NoBrush )
      mDxf->writePolygon( polygon, mLayer, QStringLiteral( "SOLID" ), brushColor() );
  }
}

void QgsDxfPaintEngine::drawPath( const QPainterPath &path )
{
  const int pathLength = path.elementCount();
  for ( int i = 0; i < pathLength; ++i )
  {
    const QPainterPath::Element &pathElem = path.elementAt( i );
    if ( pathElem.type == QPainterPath::MoveToElement )
    {
      moveTo( pathElem.x, pathElem.y );
    }
    else if ( pathElem.type == QPainterPath::LineToElement )
    {
      lineTo( pathElem.x, pathElem.y );
    }
    else if ( pathElem.type == QPainterPath::CurveToElement )
    {
      curveTo( pathElem.x, pathElem.y );
    }
    else if ( pathElem.type == QPainterPath::CurveToDataElement )
    {
      mCurrentCurve.append( QPointF( pathElem.x, pathElem.y ) );
    }
  }
  endCurve();
  endPolygon();

  if ( !mPolygon.isEmpty() && mBrush.style() != Qt::NoBrush )
    mDxf->writePolygon( mPolygon, mLayer, QStringLiteral( "SOLID" ), brushColor() );

  mPolygon.clear();
}

void QgsDxfPaintEngine::moveTo( double dx, double dy )
{
  endCurve();
  endPolygon();
  mCurrentPolygon.append( QPointF( dx, dy ) );
}

void QgsDxfPaintEngine::lineTo( double dx, double dy )
{
  endCurve();
  mCurrentPolygon.append( QPointF( dx, dy ) );
}

void QgsDxfPaintEngine::curveTo( double dx, double dy )
{
  endCurve();
  if ( !mCurrentPolygon.isEmpty() )
    mCurrentCurve.append( mCurrentPolygon.last() );

  mCurrentCurve.append( QPointF( dx, dy ) );
}

void QgsDxfPaintEngine::endPolygon()
{
  if ( mCurrentPolygon.size() > 1 )
  {
    if ( mPen.style() != Qt::NoPen )
      drawPolygon( mCurrentPolygon.constData(), mCurrentPolygon.size(), QPaintEngine::PolylineMode );

    mPolygon << QgsPointSequence();
    setRing( mPolygon.last(), mCurrentPolygon.constData(), mCurrentPolygon.size() );
  }
  mCurrentPolygon.clear();
}

void QgsDxfPaintEngine::endCurve()
{
  if ( mCurrentCurve.empty() )
    return;

  if ( mCurrentPolygon.empty() )
  {
    mCurrentCurve.clear();
    return;
  }

  if ( mCurrentCurve.size() >= 3 )
  {
    double t = 0.05;
    for ( int i = 1; i <= 20; ++i ) //approximate curve with 20 segments
    {
      mCurrentPolygon.append( bezierPoint( mCurrentCurve, t ) );
      t += 0.05;
    }
  }
  else if ( mCurrentCurve.size() == 2 )
  {
    mCurrentPolygon.append( mCurrentCurve.at( 1 ) );
  }
  mCurrentCurve.clear();
}

void QgsDxfPaintEngine::drawLines( const QLineF *lines, int lineCount )
{
  if ( !mDxf || !mPaintDevice || !lines || mPen.style() == Qt::NoPen )
    return;

  for ( int i = 0; i < lineCount; ++i )
  {
    mDxf->writeLine( toDxfCoordinates( lines[i].p1() ),
                     toDxfCoordinates( lines[i].p2() ),
                     mLayer, QStringLiteral( "CONTINUOUS" ), penColor(), currentWidth() );
  }
}

QgsPoint QgsDxfPaintEngine::toDxfCoordinates( QPointF pt ) const
{
  if ( !mPaintDevice || !mDxf )
    return QgsPoint( pt.x(), pt.y() );

  const QPointF dxfPt = mPaintDevice->dxfCoordinates( mTransform.map( pt ) ) + mShift;
  return QgsPoint( dxfPt.x(), dxfPt.y() );
}


double QgsDxfPaintEngine::currentWidth() const
{
  if ( !mPaintDevice )
    return 1;

  return mPen.widthF() * mPaintDevice->widthScaleFactor();
}

QPointF QgsDxfPaintEngine::bezierPoint( const QList<QPointF> &controlPolygon, double t )
{
  double x = 0;
  double y = 0;
  const int cPolySize = controlPolygon.size();
  double bPoly  = 0;

  QList<QPointF>::const_iterator it = controlPolygon.constBegin();
  int i = 0;
  for ( ; it != controlPolygon.constEnd(); ++it )
  {
    bPoly = bernsteinPoly( cPolySize - 1, i, t );
    x += ( it->x() * bPoly );
    y += ( it->y() * bPoly );
    ++i;
  }

  return QPointF( x, y );
}

double QgsDxfPaintEngine::bernsteinPoly( int n, int i, double t )
{
  if ( i < 0 )
    return 0;

  return lower( n, i ) * power( t, i ) * power( ( 1 - t ), ( n - i ) );
}

int QgsDxfPaintEngine::lower( int n, int i )
{
  if ( i >= 0 && i <= n )
  {
    return faculty( n ) / ( faculty( i ) * faculty( n - i ) );
  }
  else
  {
    return 0;
  }
}

double QgsDxfPaintEngine::power( double a, int b )
{
  if ( b == 0 )
    return 1;

  const double tmp = a;
  for ( int i = 2; i <= std::abs( b ); i++ )
    a *= tmp;

  if ( b > 0 )
    return a;
  else
    return 1.0 / a;
}

int QgsDxfPaintEngine::faculty( int n )
{
  if ( n < 0 )//Is faculty also defined for negative integers?
    return 0;

  int i;
  int result = n;

  if ( n == 0 || n == 1 )
    return 1;  //faculty of 0 is 1!

  for ( i = n - 1; i >= 2; i-- )
    result *= i;

  return result;
}

QColor QgsDxfPaintEngine::penColor() const
{
  if ( qgsDoubleNear( mOpacity, 1.0 ) )
  {
    return mPen.color();
  }
  QColor c = mPen.color();
  c.setAlphaF( c.alphaF() * mOpacity );
  return c;
}

QColor QgsDxfPaintEngine::brushColor() const
{
  if ( qgsDoubleNear( mOpacity, 1.0 ) )
  {
    return mBrush.color();
  }
  QColor c = mBrush.color();
  c.setAlphaF( c.alphaF() * mOpacity );
  return c;
}
