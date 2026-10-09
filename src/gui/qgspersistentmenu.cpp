/***************************************************************************
    QgsPersistentMenu.cpp  -  menu to be used in toolbar
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Till Frankenbach
    email                : till.frankenbach@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspersistentmenu.h"

#include <QAction>
#include <QMouseEvent>
#include <QSignalBlocker>

#include "moc_qgspersistentmenu.cpp"

QgsPersistentMenu::QgsPersistentMenu( const QString &title, QWidget *parent )
  : QMenu( title, parent )
{}

QgsPersistentMenu::~QgsPersistentMenu()
{}

void QgsPersistentMenu::mouseReleaseEvent( QMouseEvent *e )
{
  QAction *action = activeAction();
  if ( action && action->isEnabled() )
  {
    action->trigger();
    e->accept();
  }
  else
    QMenu::mouseReleaseEvent( e );
}

void QgsPersistentMenu::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space )
  {
    QAction *action = activeAction();
    if ( action && action->isEnabled() )
    {
      action->trigger();
      e->accept();
      return;
    }
  }
  else
    QMenu::keyPressEvent( e );
}
