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


QDomElement QgsMeshTimeSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-time-settings" ) );
  elem.setAttribute( QStringLiteral( "relative-time-format" ), mRelativeTimeFormat );
  elem.setAttribute( QStringLiteral( "absolute-time-format" ), mAbsoluteTimeFormat );
  return elem;
}

void QgsMeshTimeSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mRelativeTimeFormat = elem.attribute( QStringLiteral( "relative-time-format" ) );
  mAbsoluteTimeFormat = elem.attribute( QStringLiteral( "absolute-time-format" ) );
}

QString QgsMeshTimeSettings::relativeTimeFormat() const
{
  return mRelativeTimeFormat;
}

void QgsMeshTimeSettings::setRelativeTimeFormat( const QString &relativeTimeFormat )
{
  mRelativeTimeFormat = relativeTimeFormat;
}

QString QgsMeshTimeSettings::absoluteTimeFormat() const
{
  return mAbsoluteTimeFormat;
}

void QgsMeshTimeSettings::setAbsoluteTimeFormat( const QString &absoluteTimeFormat )
{
  mAbsoluteTimeFormat = absoluteTimeFormat;
}
