/***************************************************************************
    qgsmaptooladdcircle.cpp  -  map tool for adding circle
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

#include "qgsmaptooladdcircle.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"

QgsMapToolAddCircle::QgsMapToolAddCircle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mCircle( QgsCircle() )
{
  clean();
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddCircle::setParentTool );
  }
}

QgsMapToolAddCircle::QgsMapToolAddCircle( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mCircle( QgsCircle() )
{
  clean();
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddCircle::setParentTool );
  }
}

QgsMapToolAddCircle::~QgsMapToolAddCircle()
{
  clean();
}

void QgsMapToolAddCircle::setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  QgsMapToolCapture *tool = dynamic_cast<QgsMapToolCapture *>( oldTool );
  QgsMapToolAddCircle *csTool = dynamic_cast<QgsMapToolAddCircle *>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
}

void QgsMapToolAddCircle::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolAddCircle::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddCircle::deactivate()
{
  if ( !mParentTool || mCircle.isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve();
  mParentTool->addCurve( mCircle.toCircularString() );
  clean();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddCircle::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddCircle::clean()
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
  mCircle = QgsCircle();
}
