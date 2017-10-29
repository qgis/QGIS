/***************************************************************************
    qgsmaptoolcircle2tangentspoint.h  -  map tool for adding circle
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

#include "qgsmaptoolcircle2tangentspoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgssnappingutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsstatusbar.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgsspinbox.h"
#include <memory>
#include <QMouseEvent>

QgsMapToolCircle2TangentsPoint::QgsMapToolCircle2TangentsPoint( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircle( parentTool, canvas, mode )
{
}

QgsMapToolCircle2TangentsPoint::~QgsMapToolCircle2TangentsPoint()
{
  deleteRadiusSpinBox();
}

void QgsMapToolCircle2TangentsPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{

  QgsPoint mapPoint( e->mapPoint() );
  EdgesOnlyFilter filter;
  QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );

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
      QgsPointXY ptInter = intersect( QgsPointXY( mPoints.at( 0 ) ), QgsPointXY( mPoints.at( 1 ) ),
                                      QgsPointXY( mPoints.at( 2 ) ), QgsPointXY( mPoints.at( 3 ) ) );
      if ( ptInter == QgsPointXY() )
      {
        QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "Segments are parallels" ),
            QgsMessageBar::CRITICAL, QgisApp::instance()->messageTimeout() );
        deactivate();
      }
      else
        createRadiusSpinBox();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.clear();
    if ( mTempRubberBand )
    {
      delete mTempRubberBand;
      mTempRubberBand = nullptr;
    }

    qDeleteAll( mRubberBands );
    mRubberBands.clear();

    deactivate();
    deleteRadiusSpinBox();
    mCenters.clear();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircle2TangentsPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );
  EdgesOnlyFilter filter;
  QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );

  if ( mPoints.size() < 2 * 2 )
  {
    if ( !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
      mTempRubberBand->setFillColor( QColor( 0, 0, 255 ) );
      mTempRubberBand->setStrokeColor( QColor( 0, 0, 255 ) );
      mTempRubberBand->setStrokeWidth( 2 );
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

  if ( mPoints.size() == 4 )
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

QgsPointXY QgsMapToolCircle2TangentsPoint::intersect( QgsPointXY seg1_pt1, QgsPointXY seg1_pt2, QgsPointXY seg2_pt1, QgsPointXY seg2_pt2 )
{
  /*
   * Public domain function by Darel Rex Finley, 2006
   * http://alienryderflex.com/intersect/
   */
  QgsPointXY ptInter;

  double Ax = seg1_pt1.x();
  double Ay = seg1_pt1.y();
  double Bx = seg1_pt2.x();
  double By = seg1_pt2.y();

  double Cx = seg2_pt1.x();
  double Cy = seg2_pt1.y();
  double Dx = seg2_pt2.x();
  double Dy = seg2_pt2.y();

  if ( ( ( Ax == Bx ) && ( Ay == By ) ) || ( ( Cx == Dx ) && ( Cy == Dy ) ) )
    return ptInter;

  // (1) Translate the system so that point A is on the origin.
  Bx -= Ax;
  By -= Ay;
  Cx -= Ax;
  Cy -= Ay;
  Dx -= Ax;
  Dy -= Ay;

  // Discover the length of segment A-B
  double distAB = sqrt( Bx * Bx + By * By );

  // (2) Rotate the system so that point B is on the positive X axis.
  double theCos = Bx / distAB;
  double theSin = By / distAB;
  double newX = Cx * theCos + Cy * theSin;
  Cy = Cy * theCos - Cx * theSin;
  Cx = newX;
  newX = Dx * theCos + Dy * theSin;
  Dy = Dy * theCos - Dx * theSin;
  Dx = newX;

  // Fail if the lines are parallel.
  if ( Cy == Dy )
    return ptInter;

  // (3) Discover the position of the intersection point along line A-B.
  double ABpos = Dx + ( Cx - Dx ) * Dy / ( Dy - Cy );

  // (4) Apply the discovered position to line A-B
  // in the original coordinate system.
  ptInter.setX( Ax + ABpos * theCos );
  ptInter.setY( Ay + ABpos * theSin );

  // Success
  return ptInter;
}

void QgsMapToolCircle2TangentsPoint::getPossibleCenter( )
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
    QgsGeometry line1 = QgsGeometry( l1.release() );
    QgsGeometry line1m = line1.offsetCurve( - mRadius, 8, QgsGeometry::JoinStyleBevel, 5 );
    QgsGeometry line1p = line1.offsetCurve( + mRadius, 8, QgsGeometry::JoinStyleBevel, 5 );

    QgsGeometry line2 = QgsGeometry( l2.release() );
    QgsGeometry line2m = line2.offsetCurve( - mRadius, 8, QgsGeometry::JoinStyleBevel, 5 );
    QgsGeometry line2p = line2.offsetCurve( + mRadius, 8, QgsGeometry::JoinStyleBevel, 5 );

    QgsPointXY p1 = intersect( line1m.asPolyline().at( 0 ), line1m.asPolyline().at( 1 ),
                               line2m.asPolyline().at( 0 ), line2m.asPolyline().at( 1 ) );
    QgsPointXY p2 = intersect( line1m.asPolyline().at( 0 ), line1m.asPolyline().at( 1 ),
                               line2p.asPolyline().at( 0 ), line2p.asPolyline().at( 1 ) );
    QgsPointXY p3 = intersect( line1p.asPolyline().at( 0 ), line1p.asPolyline().at( 1 ),
                               line2m.asPolyline().at( 0 ), line2m.asPolyline().at( 1 ) );
    QgsPointXY p4 = intersect( line1p.asPolyline().at( 0 ), line1p.asPolyline().at( 1 ),
                               line2p.asPolyline().at( 0 ), line2p.asPolyline().at( 1 ) );

    mCenters.append( p1 );
    mCenters.append( p2 );
    mCenters.append( p3 );
    mCenters.append( p4 );

  }
}

void QgsMapToolCircle2TangentsPoint::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QgsSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setMinimum( 0 );
  mRadiusSpinBox->setPrefix( tr( "Radius of the circle: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
  QObject::connect( mRadiusSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( radiusSpinBoxChanged( int ) ) );
}

void QgsMapToolCircle2TangentsPoint::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolCircle2TangentsPoint::radiusSpinBoxChanged( int radius )
{
  mRadius = radius;
  getPossibleCenter( );

  qDeleteAll( mRubberBands );
  mRubberBands.clear();
  if ( mPoints.size() == 4 )
  {
    std::unique_ptr<QgsMultiPolygon> rb( new QgsMultiPolygon() );
    for ( int i = 0; i < mCenters.size(); ++i )
    {
      std::unique_ptr<QgsGeometryRubberBand> tempRB( createGeometryRubberBand( QgsWkbTypes::PointGeometry, true ) );
      std::unique_ptr<QgsPoint> tempCenter( new QgsPoint( mCenters.at( i ) ) );
      tempRB->setGeometry( tempCenter.release() );
      tempRB->show();
      mRubberBands.append( tempRB.release() );
    }
  }
}
