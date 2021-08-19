/***************************************************************************
                             qgsmodelviewtool.cpp
                             ---------------------
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

#include "qgsmodelviewtool.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelviewmouseevent.h"

QgsModelViewTool::QgsModelViewTool( QgsModelGraphicsView *view, const QString &name )
  : QObject( view )
  , mView( view )
  , mToolName( name )
{
  connect( mView, &QgsModelGraphicsView::willBeDeleted, this, [ = ]
  {
    mView = nullptr;
  } );
}

bool QgsModelViewTool::isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const
{
  const int diffX = endViewPoint.x() - startViewPoint.x();
  const int diffY = endViewPoint.y() - startViewPoint.y();
  return std::abs( diffX ) >= 2 || std::abs( diffY ) >= 2;
}

QgsModelGraphicsView *QgsModelViewTool::view() const
{
  return mView;
}

QgsModelGraphicsScene *QgsModelViewTool::scene() const
{
  return qobject_cast< QgsModelGraphicsScene * >( mView->scene() );
}

QgsModelViewTool::~QgsModelViewTool()
{
  if ( mView )
    mView->unsetTool( this );
}

QgsModelViewTool::Flags QgsModelViewTool::flags() const
{
  return mFlags;
}

void QgsModelViewTool::setFlags( QgsModelViewTool::Flags flags )
{
  mFlags = flags;
}

void QgsModelViewTool::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::modelDoubleClickEvent( QgsModelViewMouseEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::modelPressEvent( QgsModelViewMouseEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::wheelEvent( QWheelEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::keyPressEvent( QKeyEvent *event )
{
  event->ignore();
}

void QgsModelViewTool::keyReleaseEvent( QKeyEvent *event )
{
  event->ignore();
}

bool QgsModelViewTool::allowItemInteraction()
{
  return false;
}

void QgsModelViewTool::setAction( QAction *action )
{
  mAction = action;
}

QAction *QgsModelViewTool::action()
{
  return mAction;
}

void QgsModelViewTool::setCursor( const QCursor &cursor )
{
  mCursor = cursor;
}

void QgsModelViewTool::activate()
{
  // make action and/or button active
  if ( mAction )
    mAction->setChecked( true );

  mView->viewport()->setCursor( mCursor );
  emit activated();
}

void QgsModelViewTool::deactivate()
{
  if ( mAction )
    mAction->setChecked( false );

  emit deactivated();
}
