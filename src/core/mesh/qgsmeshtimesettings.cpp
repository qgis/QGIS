/***************************************************************************
                         qgsmeshtimesettings.cpp
--                         ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshtimesettings.h"

QgsMeshTimeSettings::QgsMeshTimeSettings() = default;

QgsMeshTimeSettings::QgsMeshTimeSettings( double relativeTimeOffsetHours, const QString &relativeTimeFormat )
  : mUseAbsoluteTime( false )
  , mRelativeTimeOffsetHours( relativeTimeOffsetHours )
  , mRelativeTimeFormat( relativeTimeFormat )
{}

QgsMeshTimeSettings::QgsMeshTimeSettings( const QDateTime &absoluteTimeReferenceTime, const QString &absoluteTimeFormat )
  : mUseAbsoluteTime( true )
  , mAbsoluteTimeReferenceTime( absoluteTimeReferenceTime )
  , mAbsoluteTimeFormat( absoluteTimeFormat )
{}

QDomElement QgsMeshTimeSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-time-settings" ) );
  elem.setAttribute( QStringLiteral( "use-absolute-time" ), mUseAbsoluteTime ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "relative-time-offset-hours" ), mRelativeTimeOffsetHours );
  elem.setAttribute( QStringLiteral( "relative-time-format" ), mRelativeTimeFormat );
  elem.setAttribute( QStringLiteral( "absolute-time-reference-time" ), mAbsoluteTimeReferenceTime.toString() );
  elem.setAttribute( QStringLiteral( "absolute-time-format" ), mAbsoluteTimeFormat );
  return elem;
}

void QgsMeshTimeSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mUseAbsoluteTime = elem.attribute( QStringLiteral( "use-absolute-time" ) ).toInt(); //bool
  mRelativeTimeOffsetHours = elem.attribute( QStringLiteral( "relative-time-offset-hours" ) ).toDouble();
  mRelativeTimeFormat = elem.attribute( QStringLiteral( "relative-time-format" ) );
  mAbsoluteTimeReferenceTime = QDateTime::fromString( elem.attribute( QStringLiteral( "absolute-time-reference-time" ) ) );
  mAbsoluteTimeFormat = elem.attribute( QStringLiteral( "absolute-time-format" ) );
}

bool QgsMeshTimeSettings::useAbsoluteTime() const
{
  return mUseAbsoluteTime;
}

void QgsMeshTimeSettings::setUseAbsoluteTime( bool useAbsoluteTime )
{
  mUseAbsoluteTime = useAbsoluteTime;
}


double QgsMeshTimeSettings::relativeTimeOffsetHours() const
{
  return mRelativeTimeOffsetHours;
}

void QgsMeshTimeSettings::setRelativeTimeOffsetHours( double relativeTimeOffsetHours )
{
  mRelativeTimeOffsetHours = relativeTimeOffsetHours;
}

double QgsMeshTimeSettings::datasetPlaybackInterval() const
{
  return mDatasetPlaybackIntervalSec;
}

void QgsMeshTimeSettings::setDatasetPlaybackInterval( double seconds )
{
  mDatasetPlaybackIntervalSec = seconds;
}

QString QgsMeshTimeSettings::relativeTimeFormat() const
{
  return mRelativeTimeFormat;
}

void QgsMeshTimeSettings::setRelativeTimeFormat( const QString &relativeTimeFormat )
{
  mRelativeTimeFormat = relativeTimeFormat;
}

QDateTime QgsMeshTimeSettings::absoluteTimeReferenceTime() const
{
  return mAbsoluteTimeReferenceTime;
}

void QgsMeshTimeSettings::setAbsoluteTimeReferenceTime( const QDateTime &absoluteTimeReferenceTime )
{
  mAbsoluteTimeReferenceTime = absoluteTimeReferenceTime;
}

QString QgsMeshTimeSettings::absoluteTimeFormat() const
{
  return mAbsoluteTimeFormat;
}

void QgsMeshTimeSettings::setAbsoluteTimeFormat( const QString &absoluteTimeFormat )
{
  mAbsoluteTimeFormat = absoluteTimeFormat;
}
