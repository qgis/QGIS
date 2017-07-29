/***************************************************************************
    qgsmaptooladdregularshape.cpp  -  map tool for adding regular shape
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

#include "qgsmaptooladdregularshape.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"

QgsMapToolAddRegularShape::QgsMapToolAddRegularShape( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mRegularShape( nullptr )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRegularShape::setParentTool );
  }
}

QgsMapToolAddRegularShape::QgsMapToolAddRegularShape( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mRegularShape( nullptr )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRegularShape::setParentTool );
  }
}

QgsMapToolAddRegularShape::~QgsMapToolAddRegularShape()
{
  delete mTempRubberBand;
  delete mRegularShape;
  mPoints.clear();
}

void QgsMapToolAddRegularShape::setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }
  mPoints.clear();
  QgsMapToolCapture *tool = dynamic_cast<QgsMapToolCapture *>( oldTool );
  QgsMapToolAddRegularShape *csTool = dynamic_cast<QgsMapToolAddRegularShape *>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
}

void QgsMapToolAddRegularShape::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    mPoints.clear();
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
    if ( mParentTool )
      mParentTool->keyPressEvent( e );
  }
}

void QgsMapToolAddRegularShape::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddRegularShape::deactivate()
{
  if ( !mParentTool || mRegularShape->isEmpty() )
  {
    return;
  }
  mParentTool->addCurve( mRegularShape );
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  mPoints.clear();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddRegularShape::activate()
{
  mPoints.clear();
  if ( mParentTool )
  {
    mParentTool->deleteTempRubberBand();
  }
  QgsMapToolCapture::activate();
}
