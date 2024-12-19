/***************************************************************************
                         qgslocatorfilter.cpp
                         --------------------
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

#include <QThread>

#include "qgslocatorfilter.h"
#include "qgsstringutils.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"


QgsLocatorFilter::QgsLocatorFilter( QObject *parent )
  : QObject( parent )
{
}

QgsLocatorFilter::Flags QgsLocatorFilter::flags() const
{
  return QgsLocatorFilter::Flags();
}

void QgsLocatorFilter::triggerResultFromAction( const QgsLocatorResult &result, const int actionId )
{
  Q_UNUSED( result )
  Q_UNUSED( actionId )
}

bool QgsLocatorFilter::stringMatches( const QString &candidate, const QString &search )
{
  return !search.isEmpty() && candidate.contains( search, Qt::CaseInsensitive );
}

double QgsLocatorFilter::fuzzyScore( const QString &candidate, const QString &search )
{
  return QgsStringUtils::fuzzyScore( candidate, search );
}

bool QgsLocatorFilter::enabled() const
{
  return mEnabled;
}

void QgsLocatorFilter::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

bool QgsLocatorFilter::hasConfigWidget() const
{
  return false;
}

void QgsLocatorFilter::openConfigWidget( QWidget *parent )
{
  Q_UNUSED( parent )
}

bool QgsLocatorFilter::useWithoutPrefix() const
{
  return mUseWithoutPrefix;
}

void QgsLocatorFilter::setUseWithoutPrefix( bool useWithoutPrefix )
{
  mUseWithoutPrefix = useWithoutPrefix;
}

QString QgsLocatorFilter::activePrefix() const
{
  // do not change this to isEmpty!
  // if any issue with an in-built locator filter
  // do not forget to add it in QgsLocator::CORE_FILTERS
  if ( mActivePrefifx.isNull() )
    return prefix();
  else
    return mActivePrefifx;
}

void QgsLocatorFilter::setActivePrefix( const QString &activePrefix )
{
  mActivePrefifx = activePrefix;
}

void QgsLocatorFilter::logMessage( const QString &message, Qgis::MessageLevel level )
{
  QgsMessageLog::logMessage( QString( "%1: %2" ).arg( name(), message ), QStringLiteral( "Locator bar" ), level );
}


QVariant QgsLocatorResult::userData() const
{
  return mUserData;
}

void QgsLocatorResult::setUserData( const QVariant &userData )
{
  mUserData = userData;
}
