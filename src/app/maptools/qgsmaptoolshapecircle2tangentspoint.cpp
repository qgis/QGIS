/***************************************************************************
    qgsmaptoolshapecircle2tangentspoint.h  -  map tool for adding circle
    from 2 tangents and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapecircle2tangentspoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgssnappingutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsstatusbar.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgsdoublespinbox.h"
#include "qgsgeometryutils.h"
#include <memory>
#include "qgsmapmouseevent.h"
#include "qgsmessagebar.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeCircle2TangentsPointMetadata::TOOL_ID = QStringLiteral( "circle-from-2-tangents-1-point" );

QString QgsMapToolShapeCircle2TangentsPointMetadata::id() const
{
  return QgsMapToolShapeCircle2TangentsPointMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircle2TangentsPointMetadata::name() const
{
  return QObject::tr( "Circle from 2 tangents and a point" );
}

QIcon QgsMapToolShapeCircle2TangentsPointMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircle2TangentsPoint.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircle2TangentsPointMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircle2TangentsPointMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircle2TangentsPoint( parentTool );
}

QgsMapToolShapeCircle2TangentsPoint::~QgsMapToolShapeCircle2TangentsPoint()
{
  deleteRadiusSpinBox();
}

bool QgsMapToolShapeCircle2TangentsPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  EdgesOnlyFilter filter;
  const QgsPointLocator::Match match = mParentTool->canvas()->snappingUtils()->snapToMap( mParentTool->mapPoint( *e ), &filter );

  QgsPointXY p1, p2;

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 2 * 2 )
    {
      if ( match.isValid() )
      {
        match.edgePoints( p1, p2 );
        mPoints.append( QgsPoint( p1 ) );
        mPoints.append( QgsPoint( p2 ) );
      }
    }
    if ( mPoints.size() == 4 )
    {
      bool isIntersect = false;
      QgsPoint ptInter;
      QgsGeometryUtils::segmentIntersection( mPoints.at( 0 ), mPoints.at( 1 ),
                                             mPoints.at( 2 ), mPoints.at( 3 ), ptInter, isIntersect );
      if ( !isIntersect )
      {
        QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "Segments are parallels" ),
            Qgis::MessageLevel::Critical );
        clean();
      }
      else
        createRadiusSpinBox();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    addCircleToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeCircle2TangentsPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint mapPoint( e->mapPoint() );

  EdgesOnlyFilter filter;
  const QgsPointLocator::Match match = mParentTool->canvas()->snappingUtils()->snapToMap( mapPoint, &filter );

  if ( mPoints.size() < 2 * 2 )
  {
    if ( !mTempRubberBand )
    {
      QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }
    else
      mTempRubberBand->hide();

    if ( match.isValid() )
    {
      QgsPointXY p1, p2;
      match.edgePoints( p1, p2 );
      std::unique_ptr<QgsLineString> line( new QgsLineString() );

      line->addVertex( QgsPoint( p1 ) );
      line->addVertex( QgsPoint( p2 ) );

      mTempRubberBand->setGeometry( line.release() );
      mTempRubberBand->show();
    }
  }

  if ( mPoints.size() == 4 && !mCenters.isEmpty() )
  {
    QgsPoint center = QgsPoint( mCenters.at( 0 ) );
    const double currentDist = mapPoint.distanceSquared( center );
    for ( int i = 1; i < mCenters.size(); ++i )
    {
      const double testDist = mapPoint.distanceSquared( mCenters.at( i ).x(), mCenters.at( i ).y() );
      if ( testDist < currentDist )
        center = QgsPoint( mCenters.at( i ) );
    }

    mCircle = QgsCircle( center, mRadius );
    mTempRubberBand->setGeometry( mCircle.toCircularString( true ) );
  }
}

void QgsMapToolShapeCircle2TangentsPoint::getPossibleCenter( )
{

  mCenters.clear();

  if ( mPoints.size() == 4 )
  {
    std::unique_ptr<QgsLineString> l1( new QgsLineString() );
    l1->addVertex( mPoints.at( 0 ) );
    l1->addVertex( mPoints.at( 1 ) );

    std::unique_ptr<QgsLineString> l2( new QgsLineString() );
    l2->addVertex( mPoints.at( 2 ) );
    l2->addVertex( mPoints.at( 3 ) );

    /* use magic default values (8, QgsGeometry::JoinStyleBevel, 5), is useless for segments */
    const QgsGeometry line1 = QgsGeometry( l1.release() );
    const QgsGeometry line1m = line1.offsetCurve( - mRadius, 8, Qgis::JoinStyle::Bevel, 5 );
    const QgsGeometry line1p = line1.offsetCurve( + mRadius, 8, Qgis::JoinStyle::Bevel, 5 );

    const QgsGeometry line2 = QgsGeometry( l2.release() );
    const QgsGeometry line2m = line2.offsetCurve( - mRadius, 8, Qgis::JoinStyle::Bevel, 5 );
    const QgsGeometry line2p = line2.offsetCurve( + mRadius, 8, Qgis::JoinStyle::Bevel, 5 );

    bool isIntersect = false;
    QgsPoint inter;
    QgsGeometryUtils::segmentIntersection( QgsPoint( line1m.asPolyline().at( 0 ) ), QgsPoint( line1m.asPolyline().at( 1 ) ),
                                           QgsPoint( line2m.asPolyline().at( 0 ) ), QgsPoint( line2m.asPolyline().at( 1 ) ), inter, isIntersect );
    mCenters.append( QgsPoint( inter ) );
    QgsGeometryUtils::segmentIntersection( QgsPoint( line1m.asPolyline().at( 0 ) ), QgsPoint( line1m.asPolyline().at( 1 ) ),
                                           QgsPoint( line2p.asPolyline().at( 0 ) ), QgsPoint( line2p.asPolyline().at( 1 ) ), inter, isIntersect );
    mCenters.append( QgsPoint( inter ) );
    QgsGeometryUtils::segmentIntersection( QgsPoint( line1p.asPolyline().at( 0 ) ), QgsPoint( line1p.asPolyline().at( 1 ) ),
                                           QgsPoint( line2m.asPolyline().at( 0 ) ), QgsPoint( line2m.asPolyline().at( 1 ) ), inter, isIntersect );
    mCenters.append( QgsPoint( inter ) );
    QgsGeometryUtils::segmentIntersection( QgsPoint( line1p.asPolyline().at( 0 ) ), QgsPoint( line1p.asPolyline().at( 1 ) ),
                                           QgsPoint( line2p.asPolyline().at( 0 ) ), QgsPoint( line2p.asPolyline().at( 1 ) ), inter, isIntersect );
    mCenters.append( QgsPoint( inter ) );
  }
}

void QgsMapToolShapeCircle2TangentsPoint::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QgsDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setMinimum( 0 );
  mRadiusSpinBox->setDecimals( 2 );
  mRadiusSpinBox->setPrefix( tr( "Radius of the circle: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
  QObject::connect( mRadiusSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolShapeCircle2TangentsPoint::radiusSpinBoxChanged );
}

void QgsMapToolShapeCircle2TangentsPoint::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolShapeCircle2TangentsPoint::radiusSpinBoxChanged( double radius )
{
  mRadius = radius;
  getPossibleCenter( );

  qDeleteAll( mRubberBands );
  mRubberBands.clear();
  if ( mPoints.size() == 4 )
  {
    const std::unique_ptr<QgsMultiPolygon> rb( new QgsMultiPolygon() );
    for ( int i = 0; i < mCenters.size(); ++i )
    {
      std::unique_ptr<QgsGeometryRubberBand> tempRB( mParentTool->createGeometryRubberBand( QgsWkbTypes::PointGeometry, true ) );
      std::unique_ptr<QgsPoint> tempCenter( new QgsPoint( mCenters.at( i ) ) );
      tempRB->setGeometry( tempCenter.release() );
      tempRB->show();
      mRubberBands.append( tempRB.release() );
    }
  }
}


void QgsMapToolShapeCircle2TangentsPoint::clean()
{
  qDeleteAll( mRubberBands );
  mRubberBands.clear();

  deleteRadiusSpinBox();
  mCenters.clear();

  QgsMapToolShapeCircleAbstract::clean();
}
