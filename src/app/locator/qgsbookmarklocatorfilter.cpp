/***************************************************************************
                        qgsbookmarklocatorfilters.cpp
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

#include "qgsbookmarklocatorfilter.h"
#include "qgisapp.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"


QgsBookmarkLocatorFilter::QgsBookmarkLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsBookmarkLocatorFilter *QgsBookmarkLocatorFilter::clone() const
{
  return new QgsBookmarkLocatorFilter();
}

void QgsBookmarkLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{
  const QMap<QString, QModelIndex> bookmarkMap = QgisApp::instance()->getBookmarkIndexMap();

  QMapIterator<QString, QModelIndex> i( bookmarkMap );

  while ( i.hasNext() )
  {
    i.next();

    if ( feedback->isCanceled() )
      return;

    const QString name = i.key();
    const QModelIndex index = i.value();
    QgsLocatorResult result;
    result.filter = this;
    result.displayString = name;
    result.userData = index;
    result.icon = QgsApplication::getThemeIcon( QStringLiteral( "/mItemBookmark.svg" ) );

    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

void QgsBookmarkLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  const QModelIndex index = qvariant_cast<QModelIndex>( result.userData );
  QgisApp::instance()->zoomToBookmarkIndex( index );
}
