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
  QDomElement elem = doc.createElement( u"mesh-time-settings"_s );
  elem.setAttribute( u"relative-time-format"_s, mRelativeTimeFormat );
  elem.setAttribute( u"absolute-time-format"_s, mAbsoluteTimeFormat );
  return elem;
}

void QgsMeshTimeSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mRelativeTimeFormat = elem.attribute( u"relative-time-format"_s );
  mAbsoluteTimeFormat = elem.attribute( u"absolute-time-format"_s );
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
