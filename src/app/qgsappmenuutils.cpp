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
#include <QMenuBar>
#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsAppMenuUtils::normalizedMenuName( const QString &name )
{
  const thread_local QRegularExpression sNonAlphaChars( u"[^a-zA-Z]"_s );
  return name.normalized( QString::NormalizationForm_KD ).remove( sNonAlphaChars );
}

void QgsAppMenuUtils::insertActionAlphabeticallyToMenu( QMenu *menu, QAction *action, bool insertAfterSubMenus )
{
  if ( menu->isEmpty() )
  {
    menu->addAction( action );
  }
  else
  {
    // insert into alphabetical position
    QAction *before = nullptr;
    QString normalizedActionText = action->text();
    normalizedActionText.remove( QChar( '&' ) );

    const QList< QAction * > menuActions = menu->actions();
    for ( QAction *action : menuActions )
    {
      if ( insertAfterSubMenus && action->menu() )
      {
        continue;
      }

      QString otherActionText = action->text();
      otherActionText.remove( QChar( '&' ) );
      if ( QString::localeAwareCompare( otherActionText, normalizedActionText ) > 0 )
      {
        before = action;
        break;
      }
    }
    if ( before )
    {
      menu->insertAction( before, action );
    }
    else
    {
      menu->addAction( action );
    }
  }
}

void QgsAppMenuUtils::insertSubmenuAlphabeticallyToMenu( QMenu *menu, QMenu *subMenu, bool insertMenusOnTop )
{
  if ( menu->isEmpty() )
  {
    menu->addMenu( subMenu );
  }
  else
  {
    // insert into alphabetical position
    QAction *before = nullptr;
    QString normalizedMenuName = subMenu->title();
    normalizedMenuName.remove( QChar( '&' ) );

    const QList< QAction * > menuActions = menu->actions();
    for ( QAction *action : menuActions )
    {
      if ( QMenu *otherSubMenu = action->menu() )
      {
        QString otherSubMenuTitle = otherSubMenu->title();
        otherSubMenuTitle.remove( QChar( '&' ) );
        if ( QString::localeAwareCompare( otherSubMenuTitle, normalizedMenuName ) > 0 )
        {
          before = action;
          break;
        }
      }
      else if ( insertMenusOnTop )
      {
        before = action;
        break;
      }
    }
    if ( before )
    {
      menu->insertMenu( before, subMenu );
    }
    else
    {
      menu->addMenu( subMenu );
    }
  }
}

QMenu *QgsAppMenuUtils::getMenu( QMenuBar *menuBar, const QString &menuName )
{
  if ( menuName.isEmpty() )
    return nullptr;

  QString cleanedMenuName = menuName.toLower();
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString targetName = cleanedMenuName;
  targetName.remove( QChar( '&' ) );

  const QList<QAction *> actions = menuBar->actions();
  for ( QAction *action : actions )
  {
    if ( QMenu *otherSubMenu = action->menu() )
    {
      QString candidateName = otherSubMenu->title().toLower();
      candidateName.remove( QChar( '&' ) );
      if ( candidateName == targetName )
      {
        return otherSubMenu;
      }
    }
  }

  // It doesn't exist, so create it
  QMenu *menu = new QMenu( cleanedMenuName, menuBar->parentWidget() );
  menu->setObjectName( normalizedMenuName( cleanedMenuName ) );
  menuBar->addMenu( menu );
  return menu;
}

QMenu *QgsAppMenuUtils::getSubMenu( QMenu *parentMenu, const QString &menuName )
{
  if ( menuName.isEmpty() )
    return parentMenu;

  QString cleanedMenuName = menuName.toLower();
#ifdef Q_OS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  cleanedMenuName.remove( QChar( '&' ) );
#endif
  QString targetName = cleanedMenuName;
  targetName.remove( QChar( '&' ) );

  QAction *before = nullptr;
  QList<QAction *> actions = parentMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString candidateName = actions.at( i )->text().toLower();
    candidateName.remove( QChar( '&' ) );

    int comp = targetName.localeAwareCompare( candidateName );
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
