/***************************************************************************
    qgsmaptooladdregularpolygon.h  -  map tool for adding regular polygon
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

#include "qgsmaptooladdregularpolygon.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsstatusbar.h"

QgsMapToolAddRegularPolygon::QgsMapToolAddRegularPolygon( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mRegularPolygon( QgsRegularPolygon() )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRegularPolygon::setParentTool );
  }
}

QgsMapToolAddRegularPolygon::QgsMapToolAddRegularPolygon( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mRegularPolygon( QgsRegularPolygon() )
{
  if ( mCanvas )
  {
    connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsMapToolAddRegularPolygon::setParentTool );
  }
}

QgsMapToolAddRegularPolygon::~QgsMapToolAddRegularPolygon()
{
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  mPoints.clear();
}

void QgsMapToolAddRegularPolygon::setParentTool( QgsMapTool *newTool, QgsMapTool *oldTool )
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }
  mPoints.clear();
  QgsMapToolCapture *tool = dynamic_cast<QgsMapToolCapture *>( oldTool );
  QgsMapToolAddRegularPolygon *csTool = dynamic_cast<QgsMapToolAddRegularPolygon *>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
}

void QgsMapToolAddRegularPolygon::createNumberSidesSpinBox()
{
  deleteNumberSidesSpinBox();
  mNumberSidesSpinBox = new QSpinBox();
  mNumberSidesSpinBox->setMaximum( 99999999 );
  mNumberSidesSpinBox->setMinimum( 3 );
  mNumberSidesSpinBox->setPrefix( tr( "Number of sides: " ) );
  mNumberSidesSpinBox->setValue( mNumberSides );
  QgisApp::instance()->addUserInputWidget( mNumberSidesSpinBox );
  mNumberSidesSpinBox->setFocus( Qt::TabFocusReason );
}

void QgsMapToolAddRegularPolygon::deleteNumberSidesSpinBox()
{
  if ( mNumberSidesSpinBox )
  {
    QgisApp::instance()->statusBarIface()->removeWidget( mNumberSidesSpinBox );
    delete mNumberSidesSpinBox;
    mNumberSidesSpinBox = nullptr;
  }
}

void QgsMapToolAddRegularPolygon::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    if ( mNumberSidesSpinBox )
    {
      deleteNumberSidesSpinBox();
    }
    mPoints.clear();
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
    if ( mParentTool )
      mParentTool->keyPressEvent( e );
  }
}

void QgsMapToolAddRegularPolygon::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

void QgsMapToolAddRegularPolygon::deactivate()
{
  if ( !mParentTool || mRegularPolygon.isEmpty() )
  {
    return;
  }
  mParentTool->addCurve( mRegularPolygon.toLineString() );
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  mPoints.clear();
  mRegularPolygon = QgsRegularPolygon();
  if ( mNumberSidesSpinBox )
  {
    deleteNumberSidesSpinBox();
  }

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddRegularPolygon::activate()
{
  mPoints.clear();
  if ( mParentTool )
  {
    mParentTool->deleteTempRubberBand();
  }
  QgsMapToolCapture::activate();
}
