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

QgsMapToolAddEllipse::QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mEllipse( QgsEllipse() )
{
  clean();
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddEllipse::setParentTool );
  }
}

QgsMapToolAddEllipse::QgsMapToolAddEllipse( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mEllipse( QgsEllipse() )
{
  clean();
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddEllipse::setParentTool );
  }
}

QgsMapToolAddEllipse::~QgsMapToolAddEllipse()
{
  clean();
}

void QgsMapToolAddEllipse::setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  QgsMapToolCapture *tool = dynamic_cast<QgsMapToolCapture *>( oldTool );
  QgsMapToolAddEllipse *csTool = dynamic_cast<QgsMapToolAddEllipse *>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
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
  mParentTool->addCurve( mEllipse.toLineString() );

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
}
