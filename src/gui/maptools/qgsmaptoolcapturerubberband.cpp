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
#include "qgsnurbscurve.h"
#include "qgsrubberband.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

///@cond PRIVATE


QgsMapToolCaptureRubberBand::QgsMapToolCaptureRubberBand( QgsMapCanvas *mapCanvas, Qgis::GeometryType geomType )
  : QgsGeometryRubberBand( mapCanvas, geomType )
{
  setVertexDrawingEnabled( false );

  // Create control polygon rubberband for NURBS visualization
  mControlPolygonRubberBand = new QgsRubberBand( mapCanvas, Qgis::GeometryType::Line );
  mControlPolygonRubberBand->setColor( QColor( 100, 100, 100, 150 ) );
  mControlPolygonRubberBand->setWidth( 1 );
  mControlPolygonRubberBand->setLineStyle( Qt::DashLine );
  mControlPolygonRubberBand->setVisible( false );
}

QgsMapToolCaptureRubberBand::~QgsMapToolCaptureRubberBand()
{
  delete mControlPolygonRubberBand;
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
      if ( mPoints.size() != 3 )
        return nullptr;
      return new QgsCircularString(
        mPoints[0],
        mPoints[1],
        mPoints[2]
      );
      break;
    case Qgis::WkbType::NurbsCurve:
      if ( mPoints.size() < 4 )
        return nullptr;
      return createNurbsCurve();
      break;
    default:
      return nullptr;
  }
}

bool QgsMapToolCaptureRubberBand::curveIsComplete() const
{
  return ( mStringType == Qgis::WkbType::LineString && mPoints.size() > 1 )
         || ( mStringType == Qgis::WkbType::CircularString && mPoints.size() > 2 );
}

void QgsMapToolCaptureRubberBand::reset( Qgis::GeometryType geomType, Qgis::WkbType stringType, const QgsPoint &firstPolygonPoint )
{
  if ( !( geomType == Qgis::GeometryType::Line || geomType == Qgis::GeometryType::Polygon ) )
    return;

  mPoints.clear();
  mWeights.clear();
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
  if ( mPoints.size() == 0 )
  {
    mPoints.append( point );
    mWeights.append( 1.0 );
  }

  mPoints.append( point );
  mWeights.append( 1.0 );

  if ( doUpdate )
    updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( const QgsPoint &point )
{
  if ( mPoints.size() > 0 )
    mPoints.last() = point;

  updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( int index, const QgsPoint &point )
{
  if ( mPoints.size() > 0 && mPoints.size() > index )
    mPoints[index] = point;

  updateCurve();
}

int QgsMapToolCaptureRubberBand::pointsCount() const
{
  return mPoints.size();
}

Qgis::WkbType QgsMapToolCaptureRubberBand::stringType() const
{
  return mStringType;
}

void QgsMapToolCaptureRubberBand::setStringType( Qgis::WkbType type )
{
  if ( ( type != Qgis::WkbType::CircularString && type != Qgis::WkbType::LineString && type != Qgis::WkbType::NurbsCurve ) || type == mStringType )
    return;

  mStringType = type;
  if ( type == Qgis::WkbType::LineString && mPoints.size() == 3 )
  {
    mPoints.removeAt( 1 );
  }

  setVertexDrawingEnabled( type == Qgis::WkbType::CircularString || type == Qgis::WkbType::NurbsCurve );
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
  if ( mPoints.size() > 1 )
  {
    mPoints.removeLast();
    if ( !mWeights.isEmpty() )
      mWeights.removeLast();
  }

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
    case Qgis::WkbType::NurbsCurve:
      curve.reset( createNurbsCurve() );
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

  // Update control polygon for NURBS visualization
  updateControlPolygon();
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

QgsCurve *QgsMapToolCaptureRubberBand::createNurbsCurve()
{
  // Use control points from mPoints
  QgsPointSequence controlPoints = mPoints;
  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    controlPoints.prepend( mFirstPolygonPoint );
    // For closed curves, the last control point must equal the first
    controlPoints.append( mFirstPolygonPoint );
  }

  // Get degree from settings
  int degree = QgsSettingsRegistryCore::settingsDigitizingNurbsDegree->value();
  const int n = controlPoints.size();

  // Adapt degree if not enough control points
  if ( n < degree + 1 )
  {
    // Try lower degrees, minimum is 1 (linear)
    degree = std::max( 1, n - 1 );
    if ( n < 2 )
    {
      return new QgsLineString( controlPoints );
    }
  }

  // Generate uniform clamped knot vector
  // Size = n + degree + 1
  const int knotCount = n + degree + 1;
  QVector<double> knots( knotCount );

  // First (degree + 1) knots are 0
  for ( int i = 0; i <= degree; ++i )
    knots[i] = 0.0;

  // Last (degree + 1) knots are 1
  for ( int i = knotCount - degree - 1; i < knotCount; ++i )
    knots[i] = 1.0;

  // Middle knots are uniformly spaced
  const int numMiddleKnots = n - degree - 1;
  for ( int i = 0; i < numMiddleKnots; ++i )
  {
    knots[degree + 1 + i] = static_cast<double>( i + 1 ) / ( numMiddleKnots + 1 );
  }

  // Use stored weights, or default to 1.0
  QVector<double> weights;
  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    // For polygon, prepend weight 1.0 for mFirstPolygonPoint
    weights.append( 1.0 );
    weights.append( mWeights );
    // Closing point weight should match the first point weight
    weights.append( 1.0 );
  }
  else
  {
    weights = mWeights;
  }
  // Ensure we have the right number of weights
  while ( weights.size() < n )
    weights.append( 1.0 );
  weights.resize( n );

  auto curve = std::make_unique<QgsNurbsCurve>( controlPoints, degree, knots, weights );

  if ( geometryType() == Qgis::GeometryType::Polygon )
  {
    // For polygons, we need to close the curve
    std::unique_ptr<QgsCompoundCurve> polygonCurve( new QgsCompoundCurve );
    polygonCurve->addCurve( curve.release() );
    return polygonCurve.release();
  }
  else
    return curve.release();
}

void QgsMapToolCaptureRubberBand::updateControlPolygon()
{
  if ( !mControlPolygonRubberBand )
    return;

  // Only show control polygon for NURBS curves
  if ( mStringType != Qgis::WkbType::NurbsCurve || mPoints.size() < 2 )
  {
    mControlPolygonRubberBand->reset( Qgis::GeometryType::Line );
    mControlPolygonRubberBand->setVisible( false );
    return;
  }

  mControlPolygonRubberBand->reset( Qgis::GeometryType::Line );

  // Add control points to form the control polygon
  QgsPointSequence controlPoints = mPoints;
  if ( geometryType() == Qgis::GeometryType::Polygon && !mFirstPolygonPoint.isEmpty() )
  {
    controlPoints.prepend( mFirstPolygonPoint );
  }

  for ( const QgsPoint &pt : std::as_const( controlPoints ) )
  {
    mControlPolygonRubberBand->addPoint( QgsPointXY( pt ) );
  }

  mControlPolygonRubberBand->setVisible( true );
}

double QgsMapToolCaptureRubberBand::weight( int index ) const
{
  if ( index < 0 || index >= mWeights.size() )
    return 1.0;
  return mWeights[index];
}

bool QgsMapToolCaptureRubberBand::setWeight( int index, double weight )
{
  if ( index < 0 || index >= mWeights.size() )
    return false;
  if ( weight <= 0.0 )
    return false;

  mWeights[index] = weight;
  updateCurve();
  return true;
}

///@endcond PRIVATE
