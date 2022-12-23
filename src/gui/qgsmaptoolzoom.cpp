/***************************************************************************
    qgsmaptoolzoom.cpp  -  map tool for zooming
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


#include <QRect>
#include <QColor>
#include <QCursor>
#include <QPixmap>

#include "qgsmaptoolzoom.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgslogger.h"
#include "qgsmapmouseevent.h"
#include "qgsapplication.h"



QgsMapToolZoom::QgsMapToolZoom( QgsMapCanvas *canvas, bool zoomOut )
  : QgsMapTool( canvas )
  , mZoomOut( zoomOut )
  , mNativeZoomOut( zoomOut )
  , mDragging( false )
  , mZoomOutCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) )
  , mZoomInCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) )

{
  mToolName = tr( "Zoom" );
  setZoomMode( mNativeZoomOut, true );
}

QgsMapToolZoom::~QgsMapToolZoom()
{
  delete mRubberBand;
}

QgsMapTool::Flags QgsMapToolZoom::flags() const
{
  return QgsMapTool::Transient | QgsMapTool::ShowContextMenu;
}

void QgsMapToolZoom::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  setZoomMode( e->modifiers().testFlag( Qt::AltModifier ) ^ mNativeZoomOut );

  if ( !mDragging )
  {
    mDragging = true;
    delete mRubberBand;
    mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
    QColor color( Qt::blue );
    color.setAlpha( 63 );
    mRubberBand->setColor( color );
    mZoomRect.setTopLeft( e->pos() );
  }

  mZoomRect.setBottomRight( e->pos() );
  if ( mRubberBand )
  {
    mRubberBand->setToCanvasRectangle( mZoomRect );
    mRubberBand->show();
  }
}


void QgsMapToolZoom::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  mZoomRect.setTopLeft( e->pos() );
  mZoomRect.setBottomRight( e->pos() );
}


void QgsMapToolZoom::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( mCanceled )
  {
    mCanceled = false;
    mDragging = false;
    return;
  }

  setZoomMode( e->modifiers().testFlag( Qt::AltModifier ) ^ mNativeZoomOut );

  // We are not really dragging in this case. This is sometimes caused by
  // a pen based computer reporting a press, move, and release, all the
  // one point.
  const bool tooShort = ( mZoomRect.topLeft() - mZoomRect.bottomRight() ).manhattanLength() < mMinPixelZoom;
  if ( !mDragging || tooShort )
  {
    mDragging = false;
    delete mRubberBand;
    mRubberBand = nullptr;

    // change to zoom in/out by the default multiple
    mCanvas->zoomWithCenter( e->x(), e->y(), !mZoomOut );
  }
  else
  {
    mDragging = false;
    delete mRubberBand;
    mRubberBand = nullptr;

    // store the rectangle
    mZoomRect.setRight( e->pos().x() );
    mZoomRect.setBottom( e->pos().y() );

    //account for bottom right -> top left dragging
    mZoomRect = mZoomRect.normalized();

    // set center and zoom
    const QSize &zoomRectSize = mZoomRect.size();
    const QgsMapSettings &mapSettings = mCanvas->mapSettings();
    const QSize &canvasSize = mapSettings.outputSize();
    const double sfx = static_cast<double>( zoomRectSize.width() ) / canvasSize.width();
    const double sfy = static_cast<double>( zoomRectSize.height() ) / canvasSize.height();
    const double sf = std::max( sfx, sfy );

    const QgsMapToPixel *m2p = mCanvas->getCoordinateTransform();
    const QgsPointXY c = m2p->toMapCoordinates( mZoomRect.center() );

    mCanvas->zoomByFactor( mZoomOut ? 1.0 / sf : sf, &c );

    mCanvas->refresh();
  }
}

void QgsMapToolZoom::deactivate()
{
  delete mRubberBand;
  mRubberBand = nullptr;

  QgsMapTool::deactivate();
}

void QgsMapToolZoom::setZoomMode( bool zoomOut, bool force )
{
  if ( !force && zoomOut == mZoomOut )
    return;

  mZoomOut = zoomOut;
  setCursor( mZoomOut ? mZoomOutCursor : mZoomInCursor );
}

void QgsMapToolZoom::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Alt )
  {
    setZoomMode( !mNativeZoomOut );
  }
}

void QgsMapToolZoom::keyReleaseEvent( QKeyEvent *e )
{
  // key press events are not caught wile the mouse is pressed
  // this is detected in map canvas move event

  if ( e->key() == Qt::Key_Alt )
  {
    setZoomMode( mNativeZoomOut );
  }

  if ( e->key() == Qt::Key_Escape && mDragging )
  {
    mCanceled = true;
    delete mRubberBand;
    mRubberBand = nullptr;
  }
}
