/***************************************************************************
    qgsmaptoolshapecircularstringradius.h  -  map tool for adding circular strings
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapecircularstringradius.h"
#include "qgisapp.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsstatusbar.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsdoublespinbox.h"
#include <cmath>
#include "qgsapplication.h"

const QString QgsMapToolShapeCircularStringRadiusMetadata::TOOL_ID = QStringLiteral( "circular-string-by-radius" );

QString QgsMapToolShapeCircularStringRadiusMetadata::id() const
{
  return QgsMapToolShapeCircularStringRadiusMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircularStringRadiusMetadata::name() const
{
  return QObject::tr( "Circular string by radius" );
}

QIcon QgsMapToolShapeCircularStringRadiusMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircularStringRadius.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircularStringRadiusMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Curve;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircularStringRadiusMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircularStringRadius( parentTool );
}


void QgsMapToolShapeCircularStringRadius::deactivate()
{
  deleteRadiusSpinBox();
  clean();
}

bool QgsMapToolShapeCircularStringRadius::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  mCaptureMode = mode;

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.isEmpty() )
    {
      mPoints.append( point );
    }
    else
    {
      if ( mPoints.size() % 2 )
      {
        mTemporaryEndPoint = point;

        //initial radius is distance( tempPoint - mPoints.last ) / 2.0
        const double minRadius = std::sqrt( QgsGeometryUtils::sqrDistance2D( mPoints.last(), mTemporaryEndPoint ) ) / 2.0;
        mRadius = minRadius + minRadius / 10.0;

        QgsPoint result;
        if ( QgsGeometryUtils::segmentMidPoint( mPoints.last(), mTemporaryEndPoint, result, mRadius, QgsPoint( point.x(), point.y() ) ) )
        {
          mPoints.append( result );
          createRadiusSpinBox();
          if ( mRadiusSpinBox )
          {
            mRadiusSpinBox->setMinimum( minRadius );
          }
        }
      }
      else
      {
        mPoints.append( mTemporaryEndPoint );
        deleteRadiusSpinBox();
      }
      recalculateRubberBand();
      recalculateTempRubberBand( e->mapPoint() );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( !( mPoints.size() % 2 ) )
      mPoints.removeLast();
    addCurveToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeCircularStringRadius::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  mCaptureMode = mode;

  if ( !mPoints.isEmpty() )
  {
    recalculateTempRubberBand( e->mapPoint() );
    updateCenterPointRubberBand( mTemporaryEndPoint );
  }
}

void QgsMapToolShapeCircularStringRadius::recalculateRubberBand()
{
  if ( mPoints.size() >= 3 )
  {
    QgsCircularString *cString = new QgsCircularString();
    const int rubberBandSize = mPoints.size() - ( mPoints.size() + 1 ) % 2;
    cString->setPoints( mPoints.mid( 0, rubberBandSize ) );
    delete mRubberBand;
    QgsWkbTypes::GeometryType type = mCaptureMode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
    mRubberBand = mParentTool->createGeometryRubberBand( type );
    mRubberBand->setGeometry( cString );
    mRubberBand->show();
  }
}

void QgsMapToolShapeCircularStringRadius::recalculateTempRubberBand( const QgsPointXY &mousePosition )
{
  QgsPointSequence rubberBandPoints;
  if ( !( mPoints.size() % 2 ) )
  {
    //recalculate midpoint on circle segment
    QgsPoint midPoint;
    if ( !QgsGeometryUtils::segmentMidPoint( mPoints.at( mPoints.size() - 2 ), mTemporaryEndPoint, midPoint, mRadius,
         QgsPoint( mousePosition ) ) )
    {
      return;
    }
    mPoints.replace( mPoints.size() - 1, midPoint );
    rubberBandPoints.append( mPoints.at( mPoints.size() - 2 ) );
    rubberBandPoints.append( mPoints.last() );
    rubberBandPoints.append( mTemporaryEndPoint );
  }
  else
  {
    rubberBandPoints.append( mPoints.last() );
    rubberBandPoints.append( mParentTool->mapPoint( mousePosition ) );
  }
  QgsCircularString *cString = new QgsCircularString();
  cString->setPoints( rubberBandPoints );
  delete mTempRubberBand;
  QgsWkbTypes::GeometryType type = mCaptureMode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
  mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
  mTempRubberBand->setGeometry( cString );
  mTempRubberBand->show();
}

void QgsMapToolShapeCircularStringRadius::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QgsDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setDecimals( 2 );
  mRadiusSpinBox->setPrefix( tr( "Radius: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  connect( mRadiusSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolShapeCircularStringRadius::updateRadiusFromSpinBox );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
}

void QgsMapToolShapeCircularStringRadius::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    QgisApp::instance()->statusBarIface()->removeWidget( mRadiusSpinBox );
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolShapeCircularStringRadius::updateRadiusFromSpinBox( double radius )
{
  mRadius = radius;
  recalculateTempRubberBand( mParentTool->toMapCoordinates( mParentTool->canvas()->mouseLastXY() ).toQPointF() );
}
