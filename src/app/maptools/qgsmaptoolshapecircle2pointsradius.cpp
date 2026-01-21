/***************************************************************************
    qgsmaptoolshapecircle2pointsradius.cpp  -  map tool for adding circle
    from 2 points and a radius
    ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Loïc Bartoletti
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapecircle2pointsradius.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdoublespinbox.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsmessagebar.h"

#include <QString>

#include "moc_qgsmaptoolshapecircle2pointsradius.cpp"

using namespace Qt::StringLiterals;

QString QgsMapToolShapeCircle2PointsRadiusMetadata::id() const
{
  return QgsMapToolShapeCircle2PointsRadiusMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircle2PointsRadiusMetadata::name() const
{
  return QObject::tr( "Circle from 2 points and a radius" );
}

QIcon QgsMapToolShapeCircle2PointsRadiusMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"/mActionCircle2PointsRadius.svg"_s );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircle2PointsRadiusMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircle2PointsRadiusMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircle2PointsRadius( parentTool );
}

QgsMapToolShapeCircle2PointsRadius::QgsMapToolShapeCircle2PointsRadius( QgsMapToolCapture *parentTool )
  : QgsMapToolShapeCircleAbstract( QgsMapToolShapeCircle2PointsRadiusMetadata::TOOL_ID, parentTool )
{
}

QgsMapToolShapeCircle2PointsRadius::~QgsMapToolShapeCircle2PointsRadius()
{
  clearCenterRubberBands();
  deleteSegmentRubberBand();
  deleteRadiusSpinBox();
}

bool QgsMapToolShapeCircle2PointsRadius::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    // State 0: No points yet - add first point and show spinbox
    if ( mPoints.isEmpty() )
    {
      mPoints.append( point );

      // Create the spinbox after first point is set
      createRadiusSpinBox();

      if ( !mTempRubberBand )
      {
        Qgis::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line;
        mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
        mTempRubberBand->show();
      }
    }
    // State 1: First point set - add second point
    else if ( mPoints.size() == 1 )
    {
      const double distance = mPoints.at( 0 ).distance( point );
      const double minRadius = distance / 2.0;

      if ( mRadius < minRadius )
      {
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "Error" ),
          tr( "Creating a circle of radius %1 requires a radius of at least %2 (distance between points is %3)." )
            .arg( mRadius, 0, 'f', 2 )
            .arg( minRadius, 0, 'f', 2 )
            .arg( distance, 0, 'f', 2 ),
          Qgis::MessageLevel::Warning
        );
        return false;
      }

      mPoints.append( point );
      updateCenters( mPoints.at( 0 ), mPoints.at( 1 ) );

      // If only one center possible (diameter case), finish directly
      if ( mCenters.size() == 1 )
      {
        mCircle = QgsCircle( mCenters.at( 0 ), mRadius );
        addCircleToParentTool();
        return true;
      }
    }
    // State 2: Two points set, multiple centers - select center
    else if ( mPoints.size() == 2 && mCenters.size() == 2 )
    {
      // Find the nearest center to the click position
      const double dist1 = point.distanceSquared( mCenters.at( 0 ) );
      const double dist2 = point.distanceSquared( mCenters.at( 1 ) );

      QgsPoint selectedCenter = ( dist1 <= dist2 ) ? mCenters.at( 0 ) : mCenters.at( 1 );
      mCircle = QgsCircle( selectedCenter, mRadius );
      addCircleToParentTool();
      return true;
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    // If we have 2 points and a valid circle, finish with the current preview
    if ( mPoints.size() == 2 && !mCircle.isEmpty() )
    {
      addCircleToParentTool();
      return true;
    }
    // Otherwise cancel
    clean();
    return true;
  }

  return false;
}

void QgsMapToolShapeCircle2PointsRadius::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint mapPoint = mParentTool->mapPoint( *e );

  // State 0: No points yet - nothing to preview
  if ( mPoints.isEmpty() )
  {
    return;
  }

  // State 1: First point set - preview circle and show both centers
  if ( mPoints.size() == 1 )
  {
    // Always show the segment between point 1 and current mouse position
    updateSegmentRubberBand( mPoints.at( 0 ), mapPoint );

    const double distance = mPoints.at( 0 ).distance( mapPoint );
    const double minRadius = distance / 2.0;

    // Check if radius is sufficient
    if ( mRadius < minRadius )
    {
      // Clear circle preview and centers when radius is insufficient
      clearCenterRubberBands();
      mCenters.clear();
      if ( mTempRubberBand )
      {
        mTempRubberBand->hide();
      }
      return;
    }

    updateCenters( mPoints.at( 0 ), mapPoint );

    if ( mCenters.isEmpty() )
      return;

    // Select the center closest to mouse for preview
    QgsPoint selectedCenter = mCenters.at( 0 );
    if ( mCenters.size() == 2 )
    {
      const double dist1 = mapPoint.distanceSquared( mCenters.at( 0 ) );
      const double dist2 = mapPoint.distanceSquared( mCenters.at( 1 ) );
      if ( dist2 < dist1 )
        selectedCenter = mCenters.at( 1 );
    }

    mCircle = QgsCircle( selectedCenter, mRadius );
    const QgsGeometry newGeometry( mCircle.toCircularString( true ) );
    if ( !newGeometry.isEmpty() && mTempRubberBand )
    {
      mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
      mTempRubberBand->show();
      setTransientGeometry( newGeometry );
    }
  }
  // State 2: Two points set - select center with mouse
  else if ( mPoints.size() == 2 && mCenters.size() == 2 )
  {
    // Select the center closest to mouse for preview
    const double dist1 = mapPoint.distanceSquared( mCenters.at( 0 ) );
    const double dist2 = mapPoint.distanceSquared( mCenters.at( 1 ) );
    QgsPoint selectedCenter = ( dist1 <= dist2 ) ? mCenters.at( 0 ) : mCenters.at( 1 );

    mCircle = QgsCircle( selectedCenter, mRadius );
    const QgsGeometry newGeometry( mCircle.toCircularString( true ) );
    if ( !newGeometry.isEmpty() && mTempRubberBand )
    {
      mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
      setTransientGeometry( newGeometry );
    }
  }
}

void QgsMapToolShapeCircle2PointsRadius::updateCenters( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  clearCenterRubberBands();
  mCenters.clear();

  QVector<QgsCircle> circles = QgsCircle::from2PointsRadius( pt1, pt2, mRadius );

  for ( const QgsCircle &circle : circles )
  {
    mCenters.append( circle.center() );

    // Create a rubber band to show this center
    std::unique_ptr<QgsGeometryRubberBand> rb( mParentTool->createGeometryRubberBand( Qgis::GeometryType::Point, true ) );
    rb->setGeometry( new QgsPoint( circle.center() ) );
    rb->show();
    mCenterRubberBands.append( rb.release() );
  }
}

void QgsMapToolShapeCircle2PointsRadius::clearCenterRubberBands()
{
  qDeleteAll( mCenterRubberBands );
  mCenterRubberBands.clear();
}

void QgsMapToolShapeCircle2PointsRadius::updateSegmentRubberBand( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  if ( !mSegmentRubberBand )
  {
    mSegmentRubberBand = mParentTool->createGeometryRubberBand( Qgis::GeometryType::Line, true );
  }

  auto line = std::make_unique<QgsLineString>();
  line->addVertex( pt1 );
  line->addVertex( pt2 );
  mSegmentRubberBand->setGeometry( line.release() );
  mSegmentRubberBand->show();
}

void QgsMapToolShapeCircle2PointsRadius::deleteSegmentRubberBand()
{
  if ( mSegmentRubberBand )
  {
    delete mSegmentRubberBand;
    mSegmentRubberBand = nullptr;
  }
}

void QgsMapToolShapeCircle2PointsRadius::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QgsDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setMinimum( 0.0001 );
  mRadiusSpinBox->setDecimals( 6 );
  mRadiusSpinBox->setPrefix( tr( "Radius: " ) );
  if ( mRadius > 0 )
  {
    mRadiusSpinBox->setValue( mRadius );
  }
  else
  {
    // Default radius: half of the smallest canvas dimension, rounded to lower multiple of 10
    QgsMapCanvas *canvas = mParentTool->canvas();
    const QgsRectangle extent = canvas->extent();
    double defaultRadius = std::min( extent.width(), extent.height() ) / 2.0;

    if ( defaultRadius >= 10.0 )
    {
      // Round down to nearest multiple of 10
      defaultRadius = std::floor( defaultRadius / 10.0 ) * 10.0;
    }
    else
    {
      // Between 0 and 10: round to nearest integer, minimum 1
      defaultRadius = std::max( 1.0, std::round( defaultRadius ) );
    }

    mRadiusSpinBox->setValue( defaultRadius );
    mRadius = defaultRadius;
  }
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
  QObject::connect( mRadiusSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolShapeCircle2PointsRadius::radiusSpinBoxChanged );
}

void QgsMapToolShapeCircle2PointsRadius::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolShapeCircle2PointsRadius::radiusSpinBoxChanged( double radius )
{
  mRadius = radius;

  // If we have two points, update the centers with the new radius
  if ( mPoints.size() == 2 )
  {
    updateCenters( mPoints.at( 0 ), mPoints.at( 1 ) );

    // Update the circle preview if we have centers
    if ( !mCenters.isEmpty() && mTempRubberBand )
    {
      mCircle = QgsCircle( mCenters.at( 0 ), mRadius );
      const QgsGeometry newGeometry( mCircle.toCircularString( true ) );
      if ( !newGeometry.isEmpty() )
      {
        mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
        setTransientGeometry( newGeometry );
      }
    }
  }
}

void QgsMapToolShapeCircle2PointsRadius::clean()
{
  clearCenterRubberBands();
  mCenters.clear();
  deleteSegmentRubberBand();
  deleteRadiusSpinBox();
  mRadius = 0.0; // Reset to recalculate default based on current canvas extent
  QgsMapToolShapeCircleAbstract::clean();
}

void QgsMapToolShapeCircle2PointsRadius::undo()
{
  // State 2 (2 points): go back to State 1 (1 point)
  if ( mPoints.size() == 2 )
  {
    mPoints.removeLast();
    clearCenterRubberBands();
    mCenters.clear();
    mCircle = QgsCircle();

    // Hide the circle preview but keep the temp rubber band
    if ( mTempRubberBand )
    {
      mTempRubberBand->hide();
    }

    // Delete the segment rubber band - it will be recreated on next mouse move
    // to show the segment from point 1 to mouse position
    deleteSegmentRubberBand();
  }
  // State 1 (1 point): go back to State 0 (clean)
  else if ( mPoints.size() == 1 )
  {
    clean();
  }
}
