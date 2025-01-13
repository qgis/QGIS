/***************************************************************************
                             qgsmodelviewtoollink.cpp
                             ------------------------------------
    Date                 : January 2024
    Copyright            : (C) 2024 Valentin Buira
    Email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtoollink.h"
#include "moc_qgsmodelviewtoollink.cpp"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include <QScrollBar>

QgsModelViewToolLink::QgsModelViewToolLink( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Link Tool" ) )
{
  setCursor( Qt::PointingHandCursor );
  mBezierRubberBand.reset( new QgsModelViewBezierRubberBand( view ) );

  mBezierRubberBand->setBrush( QBrush( QColor( 0, 0, 0, 63 ) ) );
  mBezierRubberBand->setPen( QPen( QBrush( QColor( 0, 0, 0, 100 ) ), 0, Qt::SolidLine ) );
}

void QgsModelViewToolLink::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  qDebug() << "QgsModelViewToolLink::modelMoveEvent";

  
  mBezierRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate 
  QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
    {
      socket->modelHoverEnterEvent( event );
      // snap 
      if ( mFrom != socket){
        QPointF rubberEndPos = socket->mapToScene(socket->getPosition());
        mBezierRubberBand->update( rubberEndPos, Qt::KeyboardModifiers() );
      }
      qDebug() << "should trigger socket->modelHoverEnterEvent( event );";
    }
  }
}

void QgsModelViewToolLink::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    view()->setTool( mPreviousViewTool );
    mBezierRubberBand->finish( event->modelPoint() );


    // // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate events
    // // to multiple items
    // QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );
    // qDebug() << "Click on an item";
    // for ( QGraphicsItem *item : items )
    // {
    //   if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) ){
    //     // Start link tool"
    //     // qDebug() << "Start link tool";
    //     // mLinkTool->setFromSocket(socket);
    //     // view()->setTool( mLinkTool.get() );
    //   }
    // }


  }
}

bool QgsModelViewToolLink::allowItemInteraction()
{
  return false;
}

void QgsModelViewToolLink::activate()
{
  qDebug() << "activate";
  mPreviousViewTool = view()->tool();

  QPointF rubberStartPos = mFrom->mapToScene(mFrom->getPosition());
  mBezierRubberBand->start( rubberStartPos, Qt::KeyboardModifiers() );

  QgsModelViewTool::activate();
}

void QgsModelViewToolLink::deactivate()
{
  mBezierRubberBand->finish( );
  QgsModelViewTool::deactivate();
}
