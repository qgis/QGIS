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
#include "moc_qgsmaptooladvanceddigitizing.cpp"
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

QgsMapToolAdvancedDigitizing::~QgsMapToolAdvancedDigitizing() = default;

void QgsMapToolAdvancedDigitizing::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( isAdvancedDigitizingAllowed() && mCadDockWidget->cadEnabled() )
  {
    mCadDockWidget->applyConstraints( e ); // updates event's map point
    mCadDockWidget->processCanvasPressEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
    }
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
    mCadDockWidget->processCanvasReleaseEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
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
    mCadDockWidget->applyConstraints( e ); // updates event's map point
    mCadDockWidget->processCanvasMoveEvent( e );
    if ( !e->isAccepted() )
    {
      return; // The dock widget has taken the event
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
    mSnapToGridCanvasItem->setPoint( e->mapPoint() );
  }

  if ( mSnapIndicator )
  {
    mSnapIndicator->setMatch( e->mapPointMatch() );
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

  if ( mSnapIndicator )
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

QgsMapLayer *QgsMapToolAdvancedDigitizing::layer() const
{
  return canvas()->currentLayer();
}

bool QgsMapToolAdvancedDigitizing::useSnappingIndicator() const
{
  return static_cast<bool>( mSnapIndicator.get() );
}

void QgsMapToolAdvancedDigitizing::setUseSnappingIndicator( bool enabled )
{
  if ( enabled && !mSnapIndicator )
  {
    mSnapIndicator = std::make_unique<QgsSnapIndicator>( mCanvas );
  }
  else if ( !enabled && mSnapIndicator )
  {
    mSnapIndicator.reset();
  }
}

void QgsMapToolAdvancedDigitizing::cadPointChanged( const QgsPointXY &point )
{
  Q_UNUSED( point )
  QMouseEvent *ev = new QMouseEvent( QEvent::MouseMove, mCanvas->mouseLastXY(), Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  qApp->postEvent( mCanvas->viewport(), ev ); // event queue will delete the event when processed
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
    if ( !layer || !layer->isSpatial() )
    {
      mCadDockWidget->clear();
      mCadDockWidget->disable();
      mSnapToGridCanvasItem->setEnabled( false );
    }
    else
    {
      mCadDockWidget->enable();
      mSnapToGridCanvasItem->setEnabled( mSnapToLayerGridEnabled );
    }
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
