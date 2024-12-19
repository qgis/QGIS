/***************************************************************************
                        qgsactionlocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
   copyright            : (C) 2017 by Nyall Dawson
   email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactionlocatorfilter.h"

#include <QAction>
#include <QMenu>
#include <QRegularExpression>



QgsActionLocatorFilter::QgsActionLocatorFilter( const QList<QWidget *> &parentObjectsForActions, QObject *parent )
  : QgsLocatorFilter( parent )
  , mActionParents( parentObjectsForActions )
{
  setUseWithoutPrefix( false );
}

QgsActionLocatorFilter *QgsActionLocatorFilter::clone() const
{
  return new QgsActionLocatorFilter( mActionParents );
}

void QgsActionLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback * )
{
  // collect results in main thread, since this method is inexpensive and
  // accessing the gui actions is not thread safe

  QList<QAction *> found;

  for ( QWidget *object : std::as_const( mActionParents ) )
  {
    searchActions( string, object, found );
  }
}

void QgsActionLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QAction *action = qobject_cast< QAction * >( qvariant_cast<QObject *>( result.userData() ) );
  if ( action )
    action->trigger();
}

void QgsActionLocatorFilter::searchActions( const QString &string, QWidget *parent, QList<QAction *> &found )
{
  const QList< QWidget *> children = parent->findChildren<QWidget *>();
  for ( QWidget *widget : children )
  {
    searchActions( string, widget, found );
  }

  const thread_local QRegularExpression extractFromTooltip( QStringLiteral( "<b>(.*)</b>" ) );
  const thread_local QRegularExpression newLineToSpace( QStringLiteral( "[\\s\\n\\r]+" ) );

  const auto constActions = parent->actions();
  for ( QAction *action : constActions )
  {
    if ( action->menu() )
    {
      searchActions( string, action->menu(), found );
      continue;
    }

    if ( !action->isEnabled() || !action->isVisible() || action->text().isEmpty() )
      continue;
    if ( found.contains( action ) )
      continue;

    QString searchText = action->text();
    searchText.replace( '&', QString() );

    QString tooltip = action->toolTip();
    tooltip.replace( newLineToSpace, QStringLiteral( " " ) );
    QRegularExpressionMatch match = extractFromTooltip.match( tooltip );
    if ( match.hasMatch() )
    {
      tooltip = match.captured( 1 );
    }
    tooltip.replace( QLatin1String( "..." ), QString() );
    tooltip.replace( QString( QChar( 0x2026 ) ), QString() );
    searchText.replace( QLatin1String( "..." ), QString() );
    searchText.replace( QString( QChar( 0x2026 ) ), QString() );
    bool uniqueTooltip = searchText.trimmed().compare( tooltip.trimmed(), Qt::CaseInsensitive ) != 0;
    if ( action->isChecked() )
    {
      searchText += QStringLiteral( " [%1]" ).arg( tr( "Active" ) );
    }
    if ( uniqueTooltip )
    {
      searchText += QStringLiteral( " (%1)" ).arg( tooltip.trimmed() );
    }

    QgsLocatorResult result;
    result.displayString = searchText;
    result.setUserData( QVariant::fromValue( action ) );
    result.icon = action->icon();
    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
    {
      found << action;
      emit resultFetched( result );
    }
  }
}
