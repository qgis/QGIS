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
#include "qgsmodelviewtool.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelviewtooltemporarykeypan.h"
#include "qgsmodelviewtooltemporarymousepan.h"
#include "qgsmodelviewtooltemporarykeyzoom.h"
#include "qgsmodelcomponentgraphicitem.h"

#include <QDragEnterEvent>
#include <QScrollBar>

///@cond NOT_STABLE

#define MIN_VIEW_SCALE 0.05
#define MAX_VIEW_SCALE 1000.0

QgsModelGraphicsView::QgsModelGraphicsView( QWidget *parent )
  : QGraphicsView( parent )
{
  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
  setAcceptDrops( true );

  mSpacePanTool = new QgsModelViewToolTemporaryKeyPan( this );
  mMidMouseButtonPanTool = new QgsModelViewToolTemporaryMousePan( this );
  mSpaceZoomTool = new QgsModelViewToolTemporaryKeyZoom( this );

}

QgsModelGraphicsView::~QgsModelGraphicsView()
{
  emit willBeDeleted();
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
#if 0
  if ( !currentLayout() )
    return;
#endif

  if ( mTool )
  {
    mTool->wheelEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    event->accept();
    wheelZoom( event );
  }
}

void QgsModelGraphicsView::wheelZoom( QWheelEvent *event )
{
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
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
  QPointF scenePoint = mapToScene( event->pos() );

  //adjust view center
  QgsPointXY oldCenter( visibleRect.center() );
  QgsPointXY newCenter( scenePoint.x() + ( ( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                        scenePoint.y() + ( ( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
  centerOn( newCenter.x(), newCenter.y() );

  //zoom layout
  if ( zoomIn )
  {
    scaleSafe( zoomFactor );
  }
  else
  {
    scaleSafe( 1 / zoomFactor );
  }
}

void QgsModelGraphicsView::scaleSafe( double scale )
{
  double currentScale = transform().m11();
  scale *= currentScale;
  scale = qBound( MIN_VIEW_SCALE, scale, MAX_VIEW_SCALE );
  setTransform( QTransform::fromScale( scale, scale ) );
}

void QgsModelGraphicsView::mousePressEvent( QMouseEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif

  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->button() == Qt::MidButton )
    {
      // Pan layout with middle mouse button
      setTool( mMidMouseButtonPanTool );
      event->accept();
    }
    else
    {
      QGraphicsView::mousePressEvent( event );
    }
  }
}

void QgsModelGraphicsView::mouseReleaseEvent( QMouseEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif

  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsModelGraphicsView::mouseMoveEvent( QMouseEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif

  mMouseCurrentXY = event->pos();

  QPointF cursorPos = mapToScene( mMouseCurrentXY );
  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, false ) );
#if 0
    if ( mTool->flags() & QgsModelViewTool::FlagSnaps )
    {

      me->snapPoint( mHorizontalSnapLine, mVerticalSnapLine, mTool->ignoredSnapItems() );
    }
    if ( mTool->flags() & QgsModelViewTool::FlagSnaps )
    {
      //draw snapping point indicator
      if ( me->isSnapped() )
      {
        cursorPos = me->snappedPoint();
        if ( mSnapMarker )
        {
          mSnapMarker->setPos( me->snappedPoint() );
          mSnapMarker->setVisible( true );
        }
      }
      else if ( mSnapMarker )
      {
        mSnapMarker->setVisible( false );
      }
    }
#endif
    mTool->modelMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsModelGraphicsView::mouseDoubleClickEvent( QMouseEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif
  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsModelGraphicsView::keyPressEvent( QKeyEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif

  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }

  if ( mTool && event->isAccepted() )
    return;

  if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() )
  {
    if ( !( event->modifiers() & Qt::ControlModifier ) )
    {
      // Pan layout with space bar
      setTool( mSpacePanTool );
    }
    else
    {
      //ctrl+space pressed, so switch to temporary keyboard based zoom tool
      setTool( mSpaceZoomTool );
    }
    event->accept();
  }
#if 0
  else if ( event->key() == Qt::Key_Left
            || event->key() == Qt::Key_Right
            || event->key() == Qt::Key_Up
            || event->key() == Qt::Key_Down )
  {
    QgsLayout *l = currentLayout();
    const QList<QgsLayoutItem *> layoutItemList = l->selectedLayoutItems();

    QPointF delta = deltaForKeyEvent( event );

    l->undoStack()->beginMacro( tr( "Move Item" ) );
    for ( QgsLayoutItem *item : layoutItemList )
    {
      l->undoStack()->beginCommand( item, tr( "Move Item" ), QgsLayoutItem::UndoIncrementalMove );
      item->attemptMoveBy( delta.x(), delta.y() );
      l->undoStack()->endCommand();
    }
    l->undoStack()->endMacro();
    event->accept();
  }
#endif
}

void QgsModelGraphicsView::keyReleaseEvent( QKeyEvent *event )
{
#if 0
  if ( !currentLayout() )
    return;
#endif

  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

QgsModelViewTool *QgsModelGraphicsView::tool()
{
  return mTool;
}

void QgsModelGraphicsView::setTool( QgsModelViewTool *tool )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    mTool->deactivate();
    disconnect( mTool, &QgsModelViewTool::itemFocused, this, &QgsModelGraphicsView::itemFocused );
  }

  // activate new tool before setting it - gives tools a chance
  // to respond to whatever the current tool is
  tool->activate();
  mTool = tool;
  connect( mTool, &QgsModelViewTool::itemFocused, this, &QgsModelGraphicsView::itemFocused );
  emit toolSet( mTool );
}

void QgsModelGraphicsView::unsetTool( QgsModelViewTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolSet( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}


///@endcond


