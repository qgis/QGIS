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
#include "qgis.h"
#include "qgsapplication.h"
#include <QSet>

void QgsRuntimeProfiler::beginGroup( const QString &name )
{
  start( name );
}

void QgsRuntimeProfiler::endGroup()
{
  end();
}

QStringList QgsRuntimeProfiler::childGroups( const QString &parent ) const
{
  QStringList res;
  const int parentDepth = parent.split( '/' ).length();
  for ( auto it = mProfileTimes.constBegin(); it != mProfileTimes.constEnd(); ++it )
  {
    if ( !parent.isEmpty() && !it->first.startsWith( parent + '/' ) )
      continue;

    if ( it->first.isEmpty() )
      continue;

    const QStringList groups = it->first.split( '/' );
    if ( parent.isEmpty() )
    {
      if ( !res.contains( groups.at( 0 ) ) )
        res << groups.at( 0 );
    }
    else
    {
      if ( !res.contains( groups.at( parentDepth ) ) )
        res << groups.at( parentDepth );
    }
  }
  return res;
}

void QgsRuntimeProfiler::start( const QString &name )
{
  mProfileTime.push( QElapsedTimer() );
  mProfileTime.top().restart();
  mCurrentName.push( name );
}

void QgsRuntimeProfiler::end()
{
  QString name;
  for ( const QString &group : qgis::as_const( mCurrentName ) )
  {
    name += name.isEmpty() || name.right( 1 ) == '/' ? group : '/' + group;
  }
  mCurrentName.pop();

  double timing = mProfileTime.top().elapsed() / 1000.0;
  mProfileTime.pop();

  mProfileTimes << qMakePair( name, timing );
  QgsDebugMsgLevel( QStringLiteral( "PROFILE: %1 - %2" ).arg( name ).arg( timing ), 2 );
}

double QgsRuntimeProfiler::profileTime( const QString &name ) const
{
  for ( auto it = mProfileTimes.constBegin(); it != mProfileTimes.constEnd(); ++it )
  {
    if ( it->first == name )
      return it->second;
  }
  return -1;
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
