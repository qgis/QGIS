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
#include "qgssnapindicator.h"

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator>( canvas ) )
{
  clean();
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddRectangle::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddRectangle::stopCapturing );
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

void QgsMapToolAddRectangle::deactivate( )
{
  if ( !mParentTool || !mRectangle.isValid() )
  {
    return;
  }

  mParentTool->clearCurve( );
  mParentTool->addCurve( mRectangle.toLineString( !QgsWkbTypes::hasZ( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() )->wkbType() ) ) );
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

  mRectangle = QgsQuadrilateral();

  QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( vLayer )
    mLayerType = vLayer->geometryType();
}
