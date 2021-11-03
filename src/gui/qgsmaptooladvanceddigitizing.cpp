/***************************************************************************
    qgsmaptooladvanceddigitizing.cpp  - map tool with event in map coordinates
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapmouseevent.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmapcanvas.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsvectorlayer.h"
#include "qgsgeometryoptions.h"
#include "qgssnaptogridcanvasitem.h"

QgsMapToolAdvancedDigitizing::QgsMapToolAdvancedDigitizing( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolEdit( canvas )
  , mCadDockWidget( cadDockWidget )
{
  Q_ASSERT( cadDockWidget );
  connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolAdvancedDigitizing::onCurrentLayerChanged );
}

void QgsMapToolAdvancedDigitizing::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->applyConstraints( e );  // updates event's map point

    if ( mCadDockWidget->constructionMode() )
      return;  // decided to eat the event and not pass it to the map tool (construction mode)
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
  }

  cadCanvasPressEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    if ( e->button() == Qt::RightButton )
    {
      mCadDockWidget->clear();
    }
    else
    {
      mCadDockWidget->applyConstraints( e );  // updates event's map point

      if ( mCadDockWidget->alignToSegment( e ) )
      {
        // Parallel or perpendicular mode and snapped to segment: do not pass the event to map tool
        return;
      }

      mCadDockWidget->addPoint( e->mapPoint() );

      mCadDockWidget->releaseLocks( false );

      if ( mCadDockWidget->constructionMode() )
        return;  // decided to eat the event and not pass it to the map tool (construction mode)
    }
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToGridCanvasItem && mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
  }

  cadCanvasReleaseEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->applyConstraints( e );     // updates event's map point

    // perpendicular/parallel constraint
    // do a soft lock when snapping to a segment
    mCadDockWidget->alignToSegment( e, QgsAdvancedDigitizingDockWidget::CadConstraint::SoftLock );
    mCadDockWidget->updateCadPaintItem();
  }
  else if ( isAutoSnapEnabled() )
  {
    e->snapPoint();
  }

  QgsVectorLayer *layer = currentVectorLayer();
  if ( mSnapToGridCanvasItem && mSnapToLayerGridEnabled && layer )
  {
    e->snapToGrid( layer->geometryOptions()->geometryPrecision(), layer->crs() );
    mSnapToGridCanvasItem->setPoint( e->mapPoint() );
  }

  cadCanvasMoveEvent( e );
}

void QgsMapToolAdvancedDigitizing::activate()
{
  QgsMapToolEdit::activate();
  connect( mCadDockWidget, &QgsAdvancedDigitizingDockWidget::pointChangedV2, this, &QgsMapToolAdvancedDigitizing::cadPointChanged );
  mCadDockWidget->enable();
  mSnapToGridCanvasItem = new QgsSnapToGridCanvasItem( mCanvas );
  QgsVectorLayer *layer = currentVectorLayer();
  if ( layer )
  {
    mSnapToGridCanvasItem->setCrs( currentVectorLayer()->crs() );
    mSnapToGridCanvasItem->setPrecision( currentVectorLayer()->geometryOptions()->geometryPrecision() );
  }
  mSnapToGridCanvasItem->setEnabled( mSnapToLayerGridEnabled );
}

void QgsMapToolAdvancedDigitizing::deactivate()
{
  QgsMapToolEdit::deactivate();
  disconnect( mCadDockWidget, &QgsAdvancedDigitizingDockWidget::pointChangedV2, this, &QgsMapToolAdvancedDigitizing::cadPointChanged );
  mCadDockWidget->disable();
  delete mSnapToGridCanvasItem;
  mSnapToGridCanvasItem = nullptr;
}

QgsMapLayer *QgsMapToolAdvancedDigitizing::layer() const
{
  return canvas()->currentLayer();
}

void QgsMapToolAdvancedDigitizing::cadPointChanged( const QgsPointXY &point )
{
  Q_UNUSED( point )
  QMouseEvent *ev = new QMouseEvent( QEvent::MouseMove, mCanvas->mouseLastXY(), Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  qApp->postEvent( mCanvas->viewport(), ev );  // event queue will delete the event when processed
}

void QgsMapToolAdvancedDigitizing::onCurrentLayerChanged()
{
  if ( mSnapToGridCanvasItem )
  {
    QgsVectorLayer *layer = currentVectorLayer();
    if ( layer && mSnapToLayerGridEnabled )
    {
      mSnapToGridCanvasItem->setPrecision( layer->geometryOptions()->geometryPrecision() );
      mSnapToGridCanvasItem->setCrs( layer->crs() );
    }

    if ( !layer )
      mSnapToGridCanvasItem->setEnabled( false );
    else
      mSnapToGridCanvasItem->setEnabled( mSnapToLayerGridEnabled );
  }
}

bool QgsMapToolAdvancedDigitizing::snapToLayerGridEnabled() const
{
  return mSnapToLayerGridEnabled;
}

void QgsMapToolAdvancedDigitizing::setSnapToLayerGridEnabled( bool snapToGridEnabled )
{
  mSnapToLayerGridEnabled = snapToGridEnabled;

  if ( mSnapToGridCanvasItem )
  {
    mSnapToGridCanvasItem->setEnabled( snapToGridEnabled );
  }
}
