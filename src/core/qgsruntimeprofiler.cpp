/***************************************************************************
    qgsruntimeprofiler.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsruntimeprofiler.h"
#include "qgslogger.h"

void QgsRuntimeProfiler::beginGroup( const QString &name )
{
  mGroupStack.push( name );
  if ( !name.isEmpty() )
  {
    mGroupPrefix += name;
    mGroupPrefix += QLatin1Char( '/' );
  }
}

void QgsRuntimeProfiler::endGroup()
{
  if ( mGroupStack.isEmpty() )
  {
    qWarning( "QgsSettings::endGroup: No matching beginGroup()" );
    return;
  }

  QString group = mGroupStack.pop();
  int len = group.size();
  if ( len > 0 )
    mGroupPrefix.truncate( mGroupPrefix.size() - ( len + 1 ) );
}

void QgsRuntimeProfiler::start( const QString &name )
{
  mProfileTime.restart();
  mCurrentName = name;
}

void QgsRuntimeProfiler::end()
{
  QString name = mCurrentName;
  name.prepend( mGroupPrefix );
  double timing = mProfileTime.elapsed() / 1000.0;
  mProfileTimes.append( QPair<QString, double>( name, timing ) );
  QgsDebugMsg( QStringLiteral( "PROFILE: %1 - %2" ).arg( name ).arg( timing ) );
}

void QgsRuntimeProfiler::clear()
{
  mProfileTimes.clear();
}

double QgsRuntimeProfiler::totalTime()
{
  double total = 0;
  QList<QPair<QString, double> >::const_iterator it = mProfileTimes.constBegin();
  for ( ; it != mProfileTimes.constEnd(); ++it )
  {
    total += ( *it ).second;
  }
  return total;
}
