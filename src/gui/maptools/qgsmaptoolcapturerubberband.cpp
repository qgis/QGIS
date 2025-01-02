/***************************************************************************
    qgsmaptoolcapturerubberband.cpp  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2022
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcapturerubberband.h"
#include "qgsgeometryrubberband.h"


///@cond PRIVATE


QgsMapToolCaptureRubberBand::QgsMapToolCaptureRubberBand( QgsMapCanvas *mapCanvas, Qgis::GeometryType geomType )
  : QgsGeometryRubberBand( mapCanvas, geomType )
{
  setVertexDrawingEnabled( false );
}

QgsCurve *QgsMapToolCaptureRubberBand::curve()
{
  if ( mPoints.empty() )
    return nullptr;

  switch ( mStringType )
  {
    case Qgis::WkbType::LineString:
      return new QgsLineString( mPoints );
      break;
    case Qgis::WkbType::CircularString:
      if ( mPoints.count() != 3 )
        return nullptr;
      return new QgsCircularString(
        mPoints[0],
        mPoints[1],
        mPoints[2]
      );
      break;
    default:
      return nullptr;
  }
}

bool QgsMapToolCaptureRubberBand::curveIsComplete() const
{
  return ( mStringType == Qgis::WkbType::LineString && mPoints.count() > 1 ) || ( mStringType == Qgis::WkbType::CircularString && mPoints.count() > 2 );
}

void QgsMapToolCaptureRubberBand::reset( Qgis::GeometryType geomType, Qgis::WkbType stringType, const QgsPoint &firstPolygonPoint )
{
  if ( !( geomType == Qgis::GeometryType::Line || geomType == Qgis::GeometryType::Polygon ) )
    return;

  mPoints.clear();
  mFirstPolygonPoint = firstPolygonPoint;
  setStringType( stringType );
  setRubberBandGeometryType( geomType );
}

void QgsMapToolCaptureRubberBand::setRubberBandGeometryType( Qgis::GeometryType geomType )
{
  QgsGeometryRubberBand::setGeometryType( geomType );
  updateCurve();
}

void QgsMapToolCaptureRubberBand::addPoint( const QgsPoint &point, bool doUpdate )
{
  if ( mPoints.count() == 0 )
    mPoints.append( point );

  mPoints.append( point );

  if ( doUpdate )
    updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( const QgsPoint &point )
{
  if ( mPoints.count() > 0 )
    mPoints.last() = point;

  updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( int index, const QgsPoint &point )
{
  if ( mPoints.count() > 0 && mPoints.size() > index )
    mPoints[index] = point;

  updateCurve();
}

int QgsMapToolCaptureRubberBand::pointsCount()
{
  return mPoints.size();
}

Qgis::WkbType QgsMapToolCaptureRubberBand::stringType() const
{
  return mStringType;
}

void QgsMapToolCaptureRubberBand::setStringType( Qgis::WkbType type )
{
  if ( ( type != Qgis::WkbType::CircularString && type != Qgis::WkbType::LineString ) || type == mStringType )
    return;

  mStringType = type;
  if ( type == Qgis::WkbType::LineString && mPoints.count() == 3 )
  {
    mPoints.removeAt( 1 );
  }

  setVertexDrawingEnabled( type == Qgis::WkbType::CircularString );
  updateCurve();
}

QgsPoint QgsMapToolCaptureRubberBand::lastPoint() const
{
  if ( mPoints.empty() )
    return QgsPoint();

  return mPoints.last();
}

QgsPoint QgsMapToolCaptureRubberBand::pointFromEnd( int posFromEnd ) const
{
  if ( posFromEnd < mPoints.size() )
    return mPoints.at( mPoints.size() - 1 - posFromEnd );
  else
    return QgsPoint();
}

void QgsMapToolCaptureRubberBand::removeLastPoint()
{
  if ( mPoints.count() > 1 )
    mPoints.removeLast();

  updateCurve();
}

void QgsMapToolCaptureRubberBand::setGeometry( QgsAbstractGeometry *geom )
{
  QgsGeometryRubberBand::setGeometry( geom );
}

void QgsMapToolCaptureRubberBand::updateCurve()
{
  std::unique_ptr<QgsCurve> curve;
  switch ( mStringType )
  {
    case Qgis::WkbType::LineString:
      curve.reset( createLinearString() );
      break;
    case Qgis::WkbType::CircularString:
      curve.reset( createCircularString() );
      break;
    default:
      return;
      break;
  }

  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    std::unique_ptr<QgsCurvePolygon> geom( new QgsCurvePolygon );
    geom->setExteriorRing( curve.release() );
    setGeometry( geom.release() );
  }
  else
  {
    setGeometry( curve.release() );
  }
}

QgsCurve *QgsMapToolCaptureRubberBand::createLinearString()
{
  std::unique_ptr<QgsLineString> curve( new QgsLineString );
  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    QgsPointSequence points = mPoints;
    points.prepend( mFirstPolygonPoint );
    curve->setPoints( points );
  }
  else
    curve->setPoints( mPoints );

  return curve.release();
}

QgsCurve *QgsMapToolCaptureRubberBand::createCircularString()
{
  std::unique_ptr<QgsCircularString> curve( new QgsCircularString );
  curve->setPoints( mPoints );
  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    // add a linear string to close the polygon
    std::unique_ptr<QgsCompoundCurve> polygonCurve( new QgsCompoundCurve );
    polygonCurve->addVertex( mFirstPolygonPoint );
    if ( !mPoints.empty() )
      polygonCurve->addVertex( mPoints.first() );
    polygonCurve->addCurve( curve.release() );
    return polygonCurve.release();
  }
  else
    return curve.release();
}

///@endcond PRIVATE
