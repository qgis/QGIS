/***************************************************************************
  qgsbloomsettings.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbloomsettings.h"

#include "qgsreadwritecontext.h"

#include <QDomDocument>
#include <QString>

using namespace Qt::StringLiterals;

QgsBloomSettings::QgsBloomSettings( const QgsBloomSettings &other )
  : mEnabled( other.mEnabled )
  , mIntensity( other.mIntensity )
  , mRadius( other.mRadius )
{}

QgsBloomSettings &QgsBloomSettings::operator=( QgsBloomSettings const &rhs )
{
  if ( &rhs == this )
    return *this;

  mEnabled = rhs.mEnabled;
  mIntensity = rhs.mIntensity;
  mRadius = rhs.mRadius;
  return *this;
}

void QgsBloomSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mEnabled = static_cast< bool >( element.attribute( u"enabled"_s, u"0"_s ).toInt() );
  mIntensity = element.attribute( u"intensity"_s, u"0.05"_s ).toDouble();
  mRadius = element.attribute( u"radius"_s, u"0.005"_s ).toDouble();
}

void QgsBloomSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  element.setAttribute( u"intensity"_s, mIntensity );
  element.setAttribute( u"radius"_s, mRadius );
}
