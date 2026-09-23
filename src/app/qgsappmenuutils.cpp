/***************************************************************************
                             qgsappmenuutils.cpp
                             ------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappmenuutils.h"

#include <QMenu>
#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsAppMenuUtils::normalizedMenuName( const QString &name )
{
  const thread_local QRegularExpression sNonAlphaChars( u"[^a-zA-Z]"_s );
  return name.normalized( QString::NormalizationForm_KD ).remove( sNonAlphaChars );
}

QMenu *QgsAppMenuUtils::getSubMenu( QMenu *parentMenu, const QString &menuName )
{
  if ( menuName.isEmpty() )
    return parentMenu;

  QString cleanedMenuName = menuName;
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString dst = cleanedMenuName;
  dst.remove( QChar( '&' ) );

  QAction *before = nullptr;
  QList<QAction *> actions = parentMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( cleanedMenuName, parentMenu->parentWidget() );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  if ( before )
    parentMenu->insertMenu( before, menu );
  else
    parentMenu->addMenu( menu );

  return menu;
}
