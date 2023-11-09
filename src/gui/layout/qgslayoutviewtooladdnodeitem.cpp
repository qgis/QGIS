/***************************************************************************
                             qgslayoutviewtooladdnodeitem.cpp
                             ----------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutviewtooladdnodeitem.h"
#include "qgsapplication.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslogger.h"
#include "qgslayoutviewrubberband.h"
#include "qgsgui.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutnewitempropertiesdialog.h"
#include "qgssettings.h"
#include "qgslayoututils.h"
#include "qgslayoutitemnodeitem.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>

QgsLayoutViewToolAddNodeItem::QgsLayoutViewToolAddNodeItem( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Add item" ) )
{
  setFlags( QgsLayoutViewTool::FlagSnaps );
  setCursor( Qt::CrossCursor );
}

void QgsLayoutViewToolAddNodeItem::setItemMetadataId( int metadataId )
{
  mItemMetadataId = metadataId;
}

void QgsLayoutViewToolAddNodeItem::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    if ( !mRubberBand )
    {
      mPolygon.clear();
      mRubberBand.reset( QgsGui::layoutItemGuiRegistry()->createNodeItemRubberBand( mItemMetadataId, view() ) );
      if ( mRubberBand )
        layout()->addItem( mRubberBand.get() );
    }

    if ( mRubberBand )
    {
      //add a new node
      addNode( event->snappedPoint() );
    }
  }
  else if ( event->button() == Qt::RightButton && mRubberBand )
  {
    // finish up

    // last (temporary) point is removed
    mPolygon.remove( mPolygon.count() - 1 );

    QgsLayoutItem *item = QgsGui::layoutItemGuiRegistry()->createItem( mItemMetadataId, layout() );
    if ( !item )
      return;

    if ( QgsLayoutNodesItem *nodesItem = qobject_cast< QgsLayoutNodesItem * >( item ) )
      nodesItem->setNodes( mPolygon );

    layout()->addLayoutItem( item );
    layout()->setSelectedItem( item );
    emit createdItem();
  }
  else
  {
    event->ignore();
    mRubberBand.reset();
  }

}

void QgsLayoutViewToolAddNodeItem::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mRubberBand )
  {
    moveTemporaryNode( event->snappedPoint(), event->modifiers() );
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolAddNodeItem::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mRubberBand )
  {
    event->ignore();
    return;
  }
}

void QgsLayoutViewToolAddNodeItem::keyPressEvent( QKeyEvent *event )
{
  if ( !mRubberBand || event->isAutoRepeat() )
  {
    event->ignore();
    return;
  }

  if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
  {
    if ( mPolygon.size() > 2 )
    {
      //remove last added vertex
      mPolygon.pop_back();
      setRubberBandNodes();
    }
    else
    {
      // all deleted, cancel
      mRubberBand.reset();
    }
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    mRubberBand.reset();
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolAddNodeItem::deactivate()
{
  if ( mRubberBand )
  {
    // canceled mid operation
    mRubberBand.reset();
  }
  QgsLayoutViewTool::deactivate();
}

void QgsLayoutViewToolAddNodeItem::addNode( QPointF scenePoint )
{
  mPolygon.append( scenePoint );

  if ( mPolygon.size() == 1 )
    mPolygon.append( scenePoint );

  setRubberBandNodes();
}

void QgsLayoutViewToolAddNodeItem::moveTemporaryNode( QPointF scenePoint, Qt::KeyboardModifiers modifiers )
{
  if ( mPolygon.isEmpty() )
    return;

  if ( mPolygon.size() > 1 && ( modifiers & Qt::ShiftModifier ) )
  {
    const QPointF start = mPolygon.at( mPolygon.size() - 2 );
    QLineF newLine = QLineF( start, scenePoint );

    //movement is constrained to 45 degree angles
    const double angle = QgsLayoutUtils::snappedAngle( newLine.angle() );
    newLine.setAngle( angle );
    scenePoint = newLine.p2();
  }

  mPolygon.replace( mPolygon.size() - 1, scenePoint );
  setRubberBandNodes();
}

void QgsLayoutViewToolAddNodeItem::setRubberBandNodes()
{
  if ( QGraphicsPolygonItem *polygonItem = dynamic_cast< QGraphicsPolygonItem *>( mRubberBand.get() ) )
  {
    polygonItem->setPolygon( mPolygon );
  }
  else if ( QGraphicsPathItem *polylineItem = dynamic_cast< QGraphicsPathItem *>( mRubberBand.get() ) )
  {
    // rebuild a new qpainter path
    QPainterPath path;
    path.addPolygon( mPolygon );
    polylineItem->setPath( path );
  }
}

int QgsLayoutViewToolAddNodeItem::itemMetadataId() const
{
  return mItemMetadataId;
}
