/***************************************************************************
    qgsmaptooladdabstract.cpp  -  abstract class for map tools of the 'add' kind
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

#include "qgsmaptooladdabstract.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgssnapindicator.h"

QgsMapToolAddAbstract::QgsMapToolAddAbstract( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mSnapIndicator( std::make_unique< QgsSnapIndicator>( canvas ) )
{
  QgsMapToolAddAbstract::clean();

  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddAbstract::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddAbstract::stopCapturing );
}

QgsMapToolAddAbstract::~QgsMapToolAddAbstract()
{
  QgsMapToolAddAbstract::clean();
}

void QgsMapToolAddAbstract::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolAddAbstract::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddAbstract::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddAbstract::clean()
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

  QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( vLayer )
    mLayerType = vLayer->geometryType();
}

void QgsMapToolAddAbstract::release( QgsMapMouseEvent *e )
{
  deactivate();
  if ( mParentTool )
  {
    mParentTool->canvasReleaseEvent( e );
  }
  activate();
}
