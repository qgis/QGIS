/***************************************************************************
    qgsmaptoolcircularstringradius.h  -  map tool for adding circular strings
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

#include "qgsmaptoolcircularstringradius.h"
#include "qgisapp.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspointv2.h"
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <cmath>

QgsMapToolCircularStringRadius::QgsMapToolCircularStringRadius( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode )
    : QgsMapToolAddCircularString( parentTool, canvas, mode )
    , mTemporaryEndPoint( QgsPointV2() )
    , mRadius( 0.0 )
    , mRadiusSpinBox( nullptr )
{

}

QgsMapToolCircularStringRadius::~QgsMapToolCircularStringRadius()
{

}

void QgsMapToolCircularStringRadius::deactivate()
{
  deleteRadiusSpinBox();
  QgsMapToolAddCircularString::deactivate();
}

void QgsMapToolCircularStringRadius::cadCanvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsPointV2 mapPoint( e->mapPoint() );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.isEmpty() )
    {
      mPoints.append( mapPoint );
    }
    else
    {
      if ( mPoints.size() % 2 )
      {
        mTemporaryEndPoint = mapPoint;

        //initial radius is distance( tempPoint - mPoints.last ) / 2.0
        double minRadius = sqrt( QgsGeometryUtils::sqrDistance2D( mPoints.last(), mTemporaryEndPoint ) ) / 2.0;
        mRadius = minRadius + minRadius / 10.0;

        QgsPointV2 result;
        if ( QgsGeometryUtils::segmentMidPoint( mPoints.last(), mTemporaryEndPoint, result, mRadius, QgsPointV2( mapPoint.x(), mapPoint.y() ) ) )
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
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircularStringRadius::cadCanvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mPoints.isEmpty() )
  {
    recalculateTempRubberBand( e->mapPoint() );
    updateCenterPointRubberBand( mTemporaryEndPoint );
  }
}

void QgsMapToolCircularStringRadius::recalculateRubberBand()
{
  if ( mPoints.size() >= 3 )
  {
    QgsCircularStringV2* cString = new QgsCircularStringV2();
    int rubberBandSize = mPoints.size() - ( mPoints.size() + 1 ) % 2;
    cString->setPoints( mPoints.mid( 0, rubberBandSize ) );
    delete mRubberBand;
    mRubberBand = createGeometryRubberBand(( mode() == CapturePolygon ) ? QGis::Polygon : QGis::Line );
    mRubberBand->setGeometry( cString );
    mRubberBand->show();
  }
}

void QgsMapToolCircularStringRadius::recalculateTempRubberBand( const QgsPoint& mousePosition )
{
  QgsPointSequenceV2 rubberBandPoints;
  if ( !( mPoints.size() % 2 ) )
  {
    //recalculate midpoint on circle segment
    QgsPointV2 midPoint;
    if ( !QgsGeometryUtils::segmentMidPoint( mPoints.at( mPoints.size() - 2 ), mTemporaryEndPoint, midPoint, mRadius,
         QgsPointV2( mousePosition ) ) )
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
    rubberBandPoints.append( QgsPointV2( mousePosition ) );
  }
  QgsCircularStringV2* cString = new QgsCircularStringV2();
  cString->setPoints( rubberBandPoints );
  delete mTempRubberBand;
  mTempRubberBand = createGeometryRubberBand(( mode() == CapturePolygon ) ? QGis::Polygon : QGis::Line, true );
  mTempRubberBand->setGeometry( cString );
  mTempRubberBand->show();
}

void QgsMapToolCircularStringRadius::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setDecimals( 2 );
  mRadiusSpinBox->setPrefix( tr( "Radius: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  QObject::connect( mRadiusSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateRadiusFromSpinBox( double ) ) );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
}

void QgsMapToolCircularStringRadius::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    QgisApp::instance()->statusBar()->removeWidget( mRadiusSpinBox );
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolCircularStringRadius::updateRadiusFromSpinBox( double radius )
{
  mRadius = radius;
  recalculateTempRubberBand( toMapCoordinates( mCanvas->mouseLastXY() ).toQPointF() );
}
