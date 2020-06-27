/***************************************************************************
    qgsmaptooladdregularpolygon.cpp  -  map tool for adding regular polygon
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
#include "qgssnapindicator.h"

QgsMapToolAddRegularPolygon::QgsMapToolAddRegularPolygon( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Add regular polygon" );

  clean();
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddRegularPolygon::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddRegularPolygon::stopCapturing );
}

QgsMapToolAddRegularPolygon::~QgsMapToolAddRegularPolygon()
{
  clean();
}

void QgsMapToolAddRegularPolygon::createNumberSidesSpinBox()
{
  deleteNumberSidesSpinBox();
  mNumberSidesSpinBox = qgis::make_unique<QgsSpinBox>();
  mNumberSidesSpinBox->setMaximum( 99999999 );
  mNumberSidesSpinBox->setMinimum( 3 );
  mNumberSidesSpinBox->setPrefix( tr( "Number of sides: " ) );
  mNumberSidesSpinBox->setValue( mNumberSides );
  QgisApp::instance()->addUserInputWidget( mNumberSidesSpinBox.get() );
}

void QgsMapToolAddRegularPolygon::deleteNumberSidesSpinBox()
{
  if ( mNumberSidesSpinBox )
  {
    mNumberSidesSpinBox.reset( nullptr );
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
  mParentTool->clearCurve( );

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> ls( mRegularPolygon.toLineString() );
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

void QgsMapToolAddRegularPolygon::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddRegularPolygon::clean()
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

  if ( mNumberSidesSpinBox )
  {
    deleteNumberSidesSpinBox();
  }

  mRegularPolygon = QgsRegularPolygon();


  QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( vLayer )
    mLayerType = vLayer->geometryType();
}

void QgsMapToolAddRegularPolygon::release( QgsMapMouseEvent *e )
{
  deactivate();
  if ( mParentTool )
  {
    mParentTool->canvasReleaseEvent( e );
  }
  activate();
}
