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
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mRectangle( QgsRectangle() )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRectangle::setParentTool );
  }
}

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mRectangle( QgsRectangle() )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRectangle::setParentTool );
  }
}

QgsMapToolAddRectangle::~QgsMapToolAddRectangle()
{
  delete mTempRubberBand;
  mPoints.clear();
}

void QgsMapToolAddRectangle::setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }
  mPoints.clear();
  QgsMapToolCapture *tool = dynamic_cast<QgsMapToolCapture *>( oldTool );
  QgsMapToolAddRectangle *csTool = dynamic_cast<QgsMapToolAddRectangle *>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
}

void QgsMapToolAddRectangle::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolAddRectangle::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddRectangle::deactivate()
{
  if ( !mParentTool || mRectangle.isEmpty() )
  {
    return;
  }
  std::unique_ptr<QgsPolygonV2> rubber( new QgsPolygonV2() );
  rubber->fromWkt( mRectangle.asPolygon() );

  mParentTool->addCurve( rubber.release()->exteriorRing()->clone() );
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  mPoints.clear();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddRectangle::activate()
{
  if ( mParentTool )
  {
    mPoints.clear();
    mParentTool->deleteTempRubberBand();
  }
  QgsMapToolCapture::activate();
}
