/***************************************************************************
                             qgsmodelgraphicsview.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelgraphicsview.h"
#include "qgssettings.h"

#include <QDragEnterEvent>
#include <QScrollBar>

///@cond NOT_STABLE

QgsModelGraphicsView::QgsModelGraphicsView( QWidget *parent )
  : QGraphicsView( parent )
{
  setAcceptDrops( true );
  setDragMode( QGraphicsView::ScrollHandDrag );
}

void QgsModelGraphicsView::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasText() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
    event->acceptProposedAction();
  else
    event->ignore();
}

void QgsModelGraphicsView::dropEvent( QDropEvent *event )
{
  const QPointF dropPoint = mapToScene( event->pos() );
  if ( event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
  {
    QByteArray data = event->mimeData()->data( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) );
    QDataStream stream( &data, QIODevice::ReadOnly );
    QString algorithmId;
    stream >> algorithmId;

    QTimer::singleShot( 0, this, [this, dropPoint, algorithmId ]
    {
      emit algorithmDropped( algorithmId, dropPoint );
    } );
    event->accept();
  }
  else if ( event->mimeData()->hasText() )
  {
    const QString itemId = event->mimeData()->text();
    QTimer::singleShot( 0, this, [this, dropPoint, itemId ]
    {
      emit inputDropped( itemId, dropPoint );
    } );
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QgsModelGraphicsView::dragMoveEvent( QDragMoveEvent *event )
{
  if ( event->mimeData()->hasText() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
    event->acceptProposedAction();
  else
    event->ignore();
}

void QgsModelGraphicsView::wheelEvent( QWheelEvent *event )
{
  setTransformationAnchor( QGraphicsView::AnchorUnderMouse );

  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( !zoomIn ? 1 / zoomFactor : zoomFactor );

  scale( scaleFactor, scaleFactor );
}

void QgsModelGraphicsView::enterEvent( QEvent *event )
{
  QGraphicsView::enterEvent( event );
  viewport()->setCursor( Qt::ArrowCursor );
}

void QgsModelGraphicsView::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::MidButton )
    mPreviousMousePos = event->pos();
  else
    QGraphicsView::mousePressEvent( event );
}

void QgsModelGraphicsView::mouseMoveEvent( QMouseEvent *event )
{
  if ( event->buttons() == Qt::MidButton )
  {
    const QPoint offset = mPreviousMousePos - event->pos();
    mPreviousMousePos = event->pos();

    verticalScrollBar()->setValue( verticalScrollBar()->value() + offset.y() );
    horizontalScrollBar()->setValue( horizontalScrollBar()->value() + offset.x() );
  }
  else
  {
    QGraphicsView::mouseMoveEvent( event );
  }
}


///@endcond


