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
    : QgsMapToolAddCircularString( parentTool, canvas, mode ),
    mTemporaryEndPointX( 0.0 ),
    mTemporaryEndPointY( 0.0 ),
    mRadiusMode( false ),
    mRadius( 0.0 ),
    mRadiusSpinBox( nullptr )
{

}

QgsMapToolCircularStringRadius::~QgsMapToolCircularStringRadius()
{

}

void QgsMapToolCircularStringRadius::cadCanvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsPointV2 mapPoint( e->mapPoint().x(), e->mapPoint().y() );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.isEmpty() )
    {
      mPoints.append( mapPoint );
      return;
    }

    if ( mPoints.size() % 2 == 1 )
    {
      if ( !mRadiusMode )
      {
        mTemporaryEndPointX = mapPoint.x();
        mTemporaryEndPointY = mapPoint.y();

        //initial radius is distance( tempPoint - mPoints.last ) / 2.0
        double minRadius = sqrt( QgsGeometryUtils::sqrDistance2D( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) ) ) / 2.0;
        mRadius = minRadius + minRadius / 10.0;
        createRadiusSpinBox();
        if ( mRadiusSpinBox )
        {
          mRadiusSpinBox->setMinimum( minRadius );
        }
      }
      else
      {
        QgsPointV2 result;
        if ( QgsGeometryUtils::segmentMidPoint( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ), result, mRadius, QgsPointV2( mapPoint.x(), mapPoint.y() ) ) )
        {
          mPoints.append( result );
          mPoints.append( QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) );
        }
        deleteRadiusSpinBox();
      }
      mRadiusMode = !mRadiusMode;
      recalculateCircularString();
    }
    else
    {
      //can we get there?
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircularStringRadius::cadCanvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mPoints.isEmpty() && mRadiusMode )
  {
    mLastMouseMapPos.setX( e->mapPoint().x() );
    mLastMouseMapPos.setY( e->mapPoint().y() );
    recalculateCircularString();
  }
  if ( !mPoints.isEmpty() && !mRadiusMode )
  {
    delete mTempRubberBand;
    mTempRubberBand = createGeometryRubberBand(( mode() == CapturePolygon ) ? QGis::Polygon : QGis::Line, true );
    QgsCircularStringV2* cString = new QgsCircularStringV2();
    QList<QgsPointV2> rubberBandPoints;
    rubberBandPoints.append( mPoints.last() );
    rubberBandPoints.append( QgsPointV2( e->mapPoint() ) );
    cString->setPoints( rubberBandPoints );
    mTempRubberBand->setGeometry( cString );
    mTempRubberBand->show();
  }
}

void QgsMapToolCircularStringRadius::recalculateCircularString()
{
  QgsCircularStringV2* cString2 = new QgsCircularStringV2();
  cString2->setPoints( mPoints );
  delete mRubberBand;
  mRubberBand = createGeometryRubberBand(( mode() == CapturePolygon ) ? QGis::Polygon : QGis::Line );
  mRubberBand->setGeometry( cString2 );
  mRubberBand->show();

  //new midpoint on circle segment
  QgsPointV2 midPoint;
  if ( !QgsGeometryUtils::segmentMidPoint( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ), midPoint, mRadius,
       mLastMouseMapPos ) )
  {
    return;
  }

  QList<QgsPointV2> rubberBandPoints = mPoints;
  rubberBandPoints.append( midPoint );
  rubberBandPoints.append( QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) );
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
  if ( !mRadiusSpinBox )
  {
    return;
  }
  QgisApp::instance()->statusBar()->removeWidget( mRadiusSpinBox );
  delete mRadiusSpinBox;
  mRadiusSpinBox = nullptr;
}

void QgsMapToolCircularStringRadius::updateRadiusFromSpinBox( double radius )
{
  mRadius = radius;
  recalculateCircularString();
}
