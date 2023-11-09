/***************************************************************************
                             qgslayoutviewtool.cpp
                             ---------------------
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

#include "qgslayoutviewtool.h"
#include "qgslayoutview.h"
#include "qgslayoutviewmouseevent.h"

QgsLayoutViewTool::QgsLayoutViewTool( QgsLayoutView *view, const QString &name )
  : QObject( view )
  , mView( view )
  , mToolName( name )
{
  connect( mView, &QgsLayoutView::willBeDeleted, this, [ = ]
  {
    mView = nullptr;
  } );
}

bool QgsLayoutViewTool::isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const
{
  const int diffX = endViewPoint.x() - startViewPoint.x();
  const int diffY = endViewPoint.y() - startViewPoint.y();
  return std::abs( diffX ) >= 2 || std::abs( diffY ) >= 2;
}

QgsLayoutView *QgsLayoutViewTool::view() const
{
  return mView;
}

QgsLayout *QgsLayoutViewTool::layout() const
{
  return mView->currentLayout();
}

QList<QgsLayoutItem *> QgsLayoutViewTool::ignoredSnapItems() const
{
  return QList<QgsLayoutItem *>();
}

QgsLayoutViewTool::~QgsLayoutViewTool()
{
  if ( mView )
    mView->unsetTool( this );
}

QgsLayoutViewTool::Flags QgsLayoutViewTool::flags() const
{
  return mFlags;
}

void QgsLayoutViewTool::setFlags( QgsLayoutViewTool::Flags flags )
{
  mFlags = flags;
}

void QgsLayoutViewTool::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::layoutDoubleClickEvent( QgsLayoutViewMouseEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::wheelEvent( QWheelEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::keyPressEvent( QKeyEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::keyReleaseEvent( QKeyEvent *event )
{
  event->ignore();
}

void QgsLayoutViewTool::setAction( QAction *action )
{
  mAction = action;
}

QAction *QgsLayoutViewTool::action()
{
  return mAction;

}

void QgsLayoutViewTool::setCursor( const QCursor &cursor )
{
  mCursor = cursor;
}

void QgsLayoutViewTool::activate()
{
  // make action and/or button active
  if ( mAction )
    mAction->setChecked( true );

  mView->viewport()->setCursor( mCursor );
  emit activated();
}

void QgsLayoutViewTool::deactivate()
{
  if ( mAction )
    mAction->setChecked( false );

  emit deactivated();
}
