/***************************************************************************
    qgsvectorfieldstreamlinesettings.cpp
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldstreamlinesettings.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsVectorFieldStreamlineSettings::SeedingStartPointsMethod QgsVectorFieldStreamlineSettings::seedingMethod() const
{
  return mSeedingMethod;
}

void QgsVectorFieldStreamlineSettings::setSeedingMethod( const SeedingStartPointsMethod &seedingMethod )
{
  mSeedingMethod = seedingMethod;
}

double QgsVectorFieldStreamlineSettings::seedingDensity() const
{
  return mSeedingDensity;
}

void QgsVectorFieldStreamlineSettings::setSeedingDensity( double seedingDensity )
{
  mSeedingDensity = seedingDensity;
}

QDomElement QgsVectorFieldStreamlineSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-streamline-settings"_s );

  elem.setAttribute( u"seeding-method"_s, static_cast< int >( mSeedingMethod ) );
  elem.setAttribute( u"seeding-density"_s, mSeedingDensity );

  return elem;
}

void QgsVectorFieldStreamlineSettings::readXml( const QDomElement &elem )
{
  mSeedingMethod = static_cast<QgsVectorFieldStreamlineSettings::SeedingStartPointsMethod>( elem.attribute( u"seeding-method"_s ).toInt() );
  mSeedingDensity = elem.attribute( u"seeding-density"_s ).toDouble();
}
