/***************************************************************************
    qgsmaptoolshapecircularstringabstract.h  -  map tool for adding circular strings
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapecircularstringabstract.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsmaptoolcapture.h"

QgsMapToolShapeCircularStringAbstract::QgsMapToolShapeCircularStringAbstract( const QString &id, QgsMapToolCapture *parentTool )
  : QgsMapToolShapeAbstract( id, parentTool )
  , mShowCenterPointRubberBand( false )
{}

QgsMapToolShapeCircularStringAbstract::~QgsMapToolShapeCircularStringAbstract()
{
  delete mRubberBand;
  delete mTempRubberBand;
  removeCenterPointRubberBand();
}


void QgsMapToolShapeCircularStringAbstract::keyPressEvent( QKeyEvent *e )
{
  if ( e && !e->isAutoRepeat() && e->key() == Qt::Key_R )
  {
    mShowCenterPointRubberBand = true;
    createCenterPointRubberBand();
    e->accept();
  }
  else if ( e )
  {
    e->ignore();
  }
}

void QgsMapToolShapeCircularStringAbstract::undo()
{
  if ( mPoints.size() == 1 )
  {
    clean();
  }
  if ( mPoints.size() > 1 )
  {
    mPoints.removeLast();
    std::unique_ptr<QgsCircularString> geomRubberBand( new QgsCircularString() );
    std::unique_ptr<QgsLineString> geomTempRubberBand( new QgsLineString() );
    const int lastPositionCompleteCircularString = mPoints.size() - 1 - ( mPoints.size() + 1 ) % 2 ;

    geomTempRubberBand->setPoints( mPoints.mid( lastPositionCompleteCircularString ) );
    mTempRubberBand->setGeometry( geomTempRubberBand.release() );

    if ( mRubberBand )
    {
      geomRubberBand->setPoints( mPoints.mid( 0, lastPositionCompleteCircularString + 1 ) );
      mRubberBand->setGeometry( geomRubberBand.release() );
    }

    const QgsVertexId idx( 0, 0, ( mPoints.size() + 1 ) % 2 );
    if ( mTempRubberBand )
    {
      mTempRubberBand->moveVertex( idx, mPoints.last() );
      updateCenterPointRubberBand( mPoints.last() );
    }
  }
}

void QgsMapToolShapeCircularStringAbstract::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && !e->isAutoRepeat() && e->key() == Qt::Key_R )
  {
    removeCenterPointRubberBand();
    mShowCenterPointRubberBand = false;
    e->accept();
  }
  else if ( e )
  {
    e->ignore();
  }
}

void QgsMapToolShapeCircularStringAbstract::activate( QgsMapToolCapture::CaptureMode mode, const QgsPoint &lastCapturedMapPoint )
{
  if ( mPoints.isEmpty() && !lastCapturedMapPoint.isEmpty() )
  {
    mPoints.append( lastCapturedMapPoint );
    if ( !mTempRubberBand )
    {
      QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }
    QgsCircularString *c = new QgsCircularString();
    QgsPointSequence rubberBandPoints = mPoints;
    rubberBandPoints.append( lastCapturedMapPoint );
    c->setPoints( rubberBandPoints );
    mTempRubberBand->setGeometry( c );
  }
}

void QgsMapToolShapeCircularStringAbstract::createCenterPointRubberBand()
{
  if ( !mShowCenterPointRubberBand || mPoints.size() < 2 || mPoints.size() % 2 != 0 )
  {
    return;
  }

  mCenterPointRubberBand = mParentTool->createGeometryRubberBand( QgsWkbTypes::PolygonGeometry );
  mCenterPointRubberBand->show();

  if ( mTempRubberBand )
  {
    const QgsAbstractGeometry *rubberBandGeom = mTempRubberBand->geometry();
    if ( rubberBandGeom )
    {
      const QgsVertexId idx( 0, 0, 2 );
      const QgsPoint pt = rubberBandGeom->vertexAt( idx );
      updateCenterPointRubberBand( pt );
    }
  }
}

void QgsMapToolShapeCircularStringAbstract::updateCenterPointRubberBand( const QgsPoint &pt )
{
  if ( !mShowCenterPointRubberBand || !mCenterPointRubberBand || mPoints.size() < 2 )
  {
    return;
  }

  if ( ( mPoints.size() ) % 2 != 0 )
  {
    return;
  }

  //create circular string
  QgsCircularString *cs = new QgsCircularString();
  QgsPointSequence csPoints;
  csPoints.append( mPoints.at( mPoints.size() - 2 ) );
  csPoints.append( mPoints.at( mPoints.size() - 1 ) );
  csPoints.append( pt );
  cs->setPoints( csPoints );

  QgsPoint center;
  double radius;
  QgsGeometryUtils::circleCenterRadius( csPoints.at( 0 ), csPoints.at( 1 ), csPoints.at( 2 ), radius, center.rx(), center.ry() );

  QgsLineString *segment1 = new QgsLineString();
  segment1->addVertex( center );
  segment1->addVertex( csPoints.at( 0 ) );

  QgsLineString *segment2 = new QgsLineString();
  segment2->addVertex( csPoints.at( 2 ) );
  segment2->addVertex( center );

  QgsCompoundCurve *cc = new QgsCompoundCurve();
  cc->addCurve( segment1 );
  cc->addCurve( cs );
  cc->addCurve( segment2 );

  QgsCurvePolygon *cp = new QgsCurvePolygon();
  cp->setExteriorRing( cc );
  mCenterPointRubberBand->setGeometry( cp );
  mCenterPointRubberBand->show();
}

void QgsMapToolShapeCircularStringAbstract::removeCenterPointRubberBand()
{
  delete mCenterPointRubberBand;
  mCenterPointRubberBand = nullptr;
}

void QgsMapToolShapeCircularStringAbstract::addCurveToParentTool()
{
  QgsCircularString *c = new QgsCircularString();
  c->setPoints( mPoints );
  mParentTool->addCurve( c );
}

void QgsMapToolShapeCircularStringAbstract::clean()
{
  mPoints.clear();
  delete mRubberBand;
  mRubberBand = nullptr;
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  removeCenterPointRubberBand();
}
