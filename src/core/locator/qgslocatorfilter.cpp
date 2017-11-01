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


#include "qgslocatorfilter.h"
#include "qgsstringutils.h"

QgsLocatorFilter::QgsLocatorFilter( QObject *parent )
  : QObject( parent )
{

}

bool QgsLocatorFilter::stringMatches( const QString &candidate, const QString &search )
{
  return candidate.contains( search, Qt::CaseInsensitive );
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
  Q_UNUSED( parent );
}

bool QgsLocatorFilter::useWithoutPrefix() const
{
  return mUseWithoutPrefix;
}

void QgsLocatorFilter::setUseWithoutPrefix( bool useWithoutPrefix )
{
  mUseWithoutPrefix = useWithoutPrefix;
}
