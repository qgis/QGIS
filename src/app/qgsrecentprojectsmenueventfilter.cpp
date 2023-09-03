/***************************************************************************
               qgsrecentprojectsmenueventfilter.cpp
               ----------------------------------------------------
    begin                : August 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrecentprojectsmenueventfilter.h"

#include "qgswelcomepage.h"
#include "qgsprojectlistitemdelegate.h"

#include <QMenu>
#include <QAction>
#include <QEvent>
#include <QMouseEvent>


QgsRecentProjectsMenuEventFilter::QgsRecentProjectsMenuEventFilter( QgsWelcomePage *welcomePage, QObject *parent )
  : QObject( parent ), mWelcomePage( welcomePage )
{
}

bool QgsRecentProjectsMenuEventFilter::eventFilter( QObject *obj, QEvent *event )
{

  if ( event->type() != QEvent::MouseButtonPress )
    return QObject::eventFilter( obj, event );

  QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>( event );
  if ( !mouseEvent )
    return QObject::eventFilter( obj, event );

  if ( mouseEvent->button() != Qt::RightButton )
    return QObject::eventFilter( obj, event );

  QMenu *menu = qobject_cast<QMenu *>( obj );
  if ( !menu )
    return QObject::eventFilter( obj, event );

  QAction *action = menu->actionAt( mouseEvent->pos() );
  if ( !action )
    return QObject::eventFilter( obj, event );

  bool ok = false;
  const int actionIndex = action->data().toInt( &ok );
  if ( !ok )
    return QObject::eventFilter( obj, event );

  const QModelIndex modelIndex = mWelcomePage->recentProjectsModel()->index( actionIndex, 0 );
  const bool pinned = mWelcomePage->recentProjectsModel()->data( modelIndex, QgsProjectListItemDelegate::PinRole ).toBool();

  QMenu subMenu;
  if ( pinned )
  {
    QAction *unpin = subMenu.addAction( tr( "Unpin" ) );
    connect( unpin, &QAction::triggered, this, [this, actionIndex] { mWelcomePage->unpinProject( actionIndex ); } );
  }
  else
  {
    QAction *remove = subMenu.addAction( tr( "Remove" ) );
    QAction *pin = subMenu.addAction( tr( "Pin" ) );
    connect( remove, &QAction::triggered, this, [this, actionIndex] { mWelcomePage->removeProject( actionIndex ); } );
    connect( pin, &QAction::triggered, this, [this, actionIndex] { mWelcomePage->pinProject( actionIndex ); } );
  }
  subMenu.exec( menu->mapToGlobal( mouseEvent->pos() ) );
  return true;
}
