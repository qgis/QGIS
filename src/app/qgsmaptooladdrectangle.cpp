/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdrectangle.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
{
  clean();
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddRectangle::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddRectangle::stopCapturing );
}

void QgsMapToolAddRectangle::setAzimuth( const double azimuth )
{
  mAzimuth = azimuth;
}

void QgsMapToolAddRectangle::setDistance1( const double distance1 )
{
  mDistance1 = distance1;
}

void QgsMapToolAddRectangle::setDistance2( const double distance2 )
{
  mDistance2 = distance2;
}

void QgsMapToolAddRectangle::setSide( const int side )
{
  mSide = side;
}

QgsMapToolAddRectangle::~QgsMapToolAddRectangle()
{
  clean();
}

void QgsMapToolAddRectangle::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    clean();
    if ( mParentTool )
      mParentTool->keyPressEvent( e );
  }
}

void QgsMapToolAddRectangle::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

QgsLineString *QgsMapToolAddRectangle::rectangleToLinestring( const bool isOriented ) const
{
  std::unique_ptr<QgsLineString> ext( new QgsLineString() );
  if ( mRectangle.toRectangle().isEmpty() )
  {
    return ext.release();
  }

  QgsPoint x0( mRectangle.xMinimum(), mRectangle.yMinimum() );

  QgsPoint x1, x2, x3;
  if ( isOriented )
  {
    const double perpendicular = 90.0 * mSide;
    x1 = x0.project( mDistance1, mAzimuth );
    x3 = x0.project( mDistance2, mAzimuth + perpendicular );
    x2 = x1.project( mDistance2, mAzimuth + perpendicular );
  }
  else
  {
    x1 = QgsPoint( mRectangle.xMinimum(), mRectangle.yMaximum() );
    x2 = QgsPoint( mRectangle.xMaximum(), mRectangle.yMaximum() );
    x3 = QgsPoint( mRectangle.xMaximum(), mRectangle.yMinimum() );
  }

  ext->addVertex( x0 );
  ext->addVertex( x1 );
  ext->addVertex( x2 );
  ext->addVertex( x3 );
  ext->addVertex( x0 );

  // keep z value from the first snapped point
  for ( const QgsPoint point : qgis::as_const( mPoints ) )
  {
    if ( QgsWkbTypes::hasZ( point.wkbType() ) )
    {
      if ( point.z() != defaultZValue() )
      {
        ext->dropZValue();
        ext->addZValue( point.z() );
        break;
      }
      else
      {
        ext->dropZValue();
        ext->addZValue( defaultZValue() );
      }
    }
  }

  return ext.release();
}

QgsPolygon *QgsMapToolAddRectangle::rectangleToPolygon( const bool isOriented ) const
{
  std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
  if ( mRectangle.toRectangle().isEmpty() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( rectangleToLinestring( isOriented ) );

  return polygon.release();
}

void QgsMapToolAddRectangle::deactivate( const bool isOriented )
{
  if ( !mParentTool || mRectangle.toRectangle().isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve( );
  mParentTool->addCurve( rectangleToLinestring( isOriented ) );
  clean();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddRectangle::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddRectangle::clean()
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }

  mPoints.clear();

  if ( mParentTool )
  {
    mParentTool->deleteTempRubberBand();
  }

  mRectangle = QgsBox3d();

  QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( vLayer )
    mLayerType = vLayer->geometryType();
}
