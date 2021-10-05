/***************************************************************************
    qgsmaptool.cpp  -  base class for map canvas tools
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsmaptool.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrendercontext.h"
#include "qgssettings.h"
#include "qgsmapmouseevent.h"

#include <QAction>
#include <QAbstractButton>

QgsMapTool::QgsMapTool( QgsMapCanvas *canvas )
  : QObject( canvas )
  , mCanvas( canvas )
  , mCursor( Qt::CrossCursor )
{
}


QgsMapTool::~QgsMapTool()
{
  if ( mCanvas )
    mCanvas->unsetMapTool( this );
}

QgsPointXY QgsMapTool::toMapCoordinates( QPoint point )
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates( point );
}

QgsPoint QgsMapTool::toMapCoordinates( const QgsMapLayer *layer, const QgsPoint &point )
{
  return mCanvas->mapSettings().layerToMapCoordinates( layer, point );
}

QgsPointXY QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, QPoint point )
{
  const QgsPointXY pt = toMapCoordinates( point );
  return toLayerCoordinates( layer, pt );
}

QgsPointXY QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsPointXY &point )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, point );
}

QgsPoint QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsPoint &point )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, point );
}

QgsPointXY QgsMapTool::toMapCoordinates( const QgsMapLayer *layer, const QgsPointXY &point )
{
  return mCanvas->mapSettings().layerToMapCoordinates( layer, point );
}

QgsRectangle QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsRectangle &rect )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, rect );
}

QPoint QgsMapTool::toCanvasCoordinates( const QgsPointXY &point ) const
{
  qreal x = point.x(), y = point.y();
  mCanvas->getCoordinateTransform()->transformInPlace( x, y );
  return QPoint( std::round( x ), std::round( y ) );
}

QgsMapLayer *QgsMapTool::layer( const QString &id )
{
  return mCanvas->layer( id );
}

void QgsMapTool::setToolName( const QString &name )
{
  mToolName = name;
}

void QgsMapTool::activate()
{
  // make action and/or button active
  if ( mAction )
    mAction->setChecked( true );
  if ( mButton )
    mButton->setChecked( true );

  // set cursor (map tools usually set it in constructor)
  mCanvas->setCursor( mCursor );
  QgsDebugMsgLevel( QStringLiteral( "Cursor has been set" ), 4 );

  emit activated();
}


void QgsMapTool::deactivate()
{
  if ( mAction )
    mAction->setChecked( false );
  if ( mButton )
    mButton->setChecked( false );

  emit deactivated();
}

void QgsMapTool::clean()
{

}

void QgsMapTool::setAction( QAction *action )
{
  if ( mAction )
    disconnect( mAction, &QObject::destroyed, this, &QgsMapTool::actionDestroyed );
  mAction = action;
  if ( mAction )
    connect( mAction, &QObject::destroyed, this, &QgsMapTool::actionDestroyed );
}

void QgsMapTool::actionDestroyed()
{
  if ( mAction == sender() )
    mAction = nullptr;
}

QAction *QgsMapTool::action()
{
  return mAction;
}

bool QgsMapTool::isActive() const
{
  return mCanvas && mCanvas->mapTool() == this;
}

void QgsMapTool::setButton( QAbstractButton *button )
{
  mButton = button;
}

QAbstractButton *QgsMapTool::button()
{
  return mButton;
}

void QgsMapTool::setCursor( const QCursor &cursor )
{
  mCursor = cursor;
  if ( isActive() )
    mCanvas->setCursor( mCursor );
}


void QgsMapTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::wheelEvent( QWheelEvent *e )
{
  e->ignore();
}

void QgsMapTool::keyPressEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::keyReleaseEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
}

bool QgsMapTool::gestureEvent( QGestureEvent *e )
{
  Q_UNUSED( e )
  return true;
}

bool QgsMapTool::canvasToolTipEvent( QHelpEvent *e )
{
  Q_UNUSED( e )
  return false;
}

QgsMapCanvas *QgsMapTool::canvas() const
{
  return mCanvas;
}

double QgsMapTool::searchRadiusMM()
{
  const QgsSettings settings;
  const double radius = settings.value( QStringLiteral( "Map/searchRadiusMM" ), Qgis::DEFAULT_SEARCH_RADIUS_MM ).toDouble();

  if ( radius > 0 )
  {
    return radius;
  }
  return Qgis::DEFAULT_SEARCH_RADIUS_MM;
}

double QgsMapTool::searchRadiusMU( const QgsRenderContext &context )
{
  return searchRadiusMM() * context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
}

double QgsMapTool::searchRadiusMU( QgsMapCanvas *canvas )
{
  if ( !canvas )
  {
    return 0;
  }
  const QgsMapSettings mapSettings = canvas->mapSettings();
  const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  return searchRadiusMU( context );
}


void QgsMapTool::populateContextMenu( QMenu * )
{

}


bool QgsMapTool::populateContextMenuWithEvent( QMenu *, QgsMapMouseEvent * )
{
  return false;
}
