/***************************************************************************
  qgsambientocclusionsettings.cpp
  --------------------------------------
  Date                 : June 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsambientocclusionsettings.h"

#include "qgsreadwritecontext.h"

#include <QDomDocument>

QgsAmbientOcclusionSettings::QgsAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &other )
  : mEnabled( other.mEnabled )
  , mIntensity( other.mIntensity )
  , mRadius( other.mRadius )
  , mThreshold( other.mThreshold )
{
}

QgsAmbientOcclusionSettings &QgsAmbientOcclusionSettings::operator=( QgsAmbientOcclusionSettings const &rhs )
{
  if ( &rhs == this )
    return *this;

  mEnabled = rhs.mEnabled;
  mIntensity = rhs.mIntensity;
  mRadius = rhs.mRadius;
  mThreshold = rhs.mThreshold;
  return *this;
}

void QgsAmbientOcclusionSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mEnabled = element.attribute( u"enabled"_s, u"0"_s ).toInt();
  mIntensity = element.attribute( u"intensity"_s, u"0.5"_s ).toFloat();
  mRadius = element.attribute( u"radius"_s, u"25"_s ).toFloat();
  mThreshold = element.attribute( u"threshold"_s, u"0.5"_s ).toFloat();

  Q_UNUSED( context );
}

void QgsAmbientOcclusionSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( u"enabled"_s, mEnabled );
  element.setAttribute( u"intensity"_s, mIntensity );
  element.setAttribute( u"radius"_s, mRadius );
  element.setAttribute( u"threshold"_s, mThreshold );

  Q_UNUSED( context );
}
