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

#include "qgsapplication.h"
#include "qgsfocuskeeper.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgsprojectstorageguiregistry.h"
#include "qgsprojectstorageregistry.h"
#include "qgswelcomescreen.h"

#include <QAction>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>

#include "moc_qgsrecentprojectsmenueventfilter.cpp"

QgsRecentProjectsMenuEventFilter::QgsRecentProjectsMenuEventFilter( QgsWelcomeScreen *welcomeScreen, QObject *parent )
  : QObject( parent ), mWelcomeScreen( welcomeScreen )
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

  const QModelIndex modelIndex = mWelcomeScreen->recentProjectsModel()->index( actionIndex, 0 );

  QMenu subMenu;
  const bool pinned = mWelcomeScreen->recentProjectsModel()->data( modelIndex, static_cast<int>( QgsRecentProjectItemsModel::CustomRole::PinnedRole ) ).toBool();
  if ( pinned )
  {
    QAction *unpin = subMenu.addAction( tr( "Unpin from List" ) );
    connect( unpin, &QAction::triggered, this, [this, actionIndex] { mWelcomeScreen->recentProjectsModel()->unpinProject( actionIndex ); } );
  }
  else
  {
    QAction *pin = subMenu.addAction( tr( "Pin to List" ) );
    connect( pin, &QAction::triggered, this, [this, actionIndex] { mWelcomeScreen->recentProjectsModel()->pinProject( actionIndex ); } );
  }

  const QString path = mWelcomeScreen->recentProjectsModel()->data( modelIndex, static_cast<int>( QgsRecentProjectItemsModel::CustomRole::NativePathRole ) ).toString();
  if ( !path.isEmpty() )
  {
    QAction *openFolderAction = subMenu.addAction( tr( "Open Directory…" ) );
    connect( openFolderAction, &QAction::triggered, this, [this, actionIndex] { mWelcomeScreen->recentProjectsModel()->openProject( actionIndex ); } );
  }

  QAction *remove = subMenu.addAction( tr( "Remove from List" ) );
  connect( remove, &QAction::triggered, this, [this, actionIndex] { mWelcomeScreen->recentProjectsModel()->removeProject( actionIndex ); } );
  subMenu.exec( menu->mapToGlobal( mouseEvent->pos() ) );
  return true;
}
