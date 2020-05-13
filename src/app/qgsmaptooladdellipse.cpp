/***************************************************************************
    qgsmaptooladdellipse.cpp  -  map tool for adding ellipse
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

#include "qgsmaptooladdellipse.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgssnapindicator.h"

QgsMapToolAddEllipse::QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Add ellipse" );

  clean();
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddEllipse::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddEllipse::stopCapturing );
}

QgsMapToolAddEllipse::~QgsMapToolAddEllipse()
{
  clean();
}

void QgsMapToolAddEllipse::keyPressEvent( QKeyEvent *e )
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

  if ( e && e->key() == Qt::Key_Backspace )
  {
    if ( mPoints.size() == 1 )
    {

      if ( mTempRubberBand )
      {
        delete mTempRubberBand;
        mTempRubberBand = nullptr;
      }

      mPoints.clear();
    }
    else if ( mPoints.size() > 1 )
    {
      mPoints.removeLast();

    }
    if ( mParentTool )
      mParentTool->keyPressEvent( e );
  }
}

void QgsMapToolAddEllipse::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddEllipse::deactivate()
{
  if ( !mParentTool || mEllipse.isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve();

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> ls( mEllipse.toLineString( segments() ) );
  for ( const QgsPoint point : qgis::as_const( mPoints ) )
  {
    if ( QgsWkbTypes::hasZ( point.wkbType() ) &&
         point.z() != defaultZValue() )
    {
      ls->dropZValue();
      ls->addZValue( point.z() );
      break;
    }
  }

  mParentTool->addCurve( ls.release() );
  clean();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddEllipse::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddEllipse::clean()
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

  mEllipse = QgsEllipse();

  QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( vLayer )
    mLayerType = vLayer->geometryType();
}

void QgsMapToolAddEllipse::release( QgsMapMouseEvent *e )
{
  deactivate();
  if ( mParentTool )
  {
    mParentTool->canvasReleaseEvent( e );
  }
  activate();
}
