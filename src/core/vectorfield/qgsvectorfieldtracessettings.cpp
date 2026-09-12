/***************************************************************************
    qgsvectorfieldtracessettings.cpp
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

#include "qgsvectorfieldtracessettings.h"

#include <QString>

using namespace Qt::StringLiterals;

void QgsVectorFieldTracesSettings::readXml( const QDomElement &elem )
{
  mMaximumTailLength = elem.attribute( u"maximum-tail-length"_s ).toInt();
  mMaximumTailLengthUnit = static_cast<Qgis::RenderUnit>( elem.attribute( u"maximum-tail-length-unit"_s ).toInt() );
  mParticlesCount = elem.attribute( u"particles-count"_s ).toInt();
}

QDomElement QgsVectorFieldTracesSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-traces-settings"_s );
  elem.setAttribute( u"maximum-tail-length"_s, mMaximumTailLength );
  elem.setAttribute( u"maximum-tail-length-unit"_s, static_cast< int >( mMaximumTailLengthUnit ) );
  elem.setAttribute( u"particles-count"_s, mParticlesCount );

  return elem;
}

Qgis::RenderUnit QgsVectorFieldTracesSettings::maximumTailLengthUnit() const
{
  return mMaximumTailLengthUnit;
}

void QgsVectorFieldTracesSettings::setMaximumTailLengthUnit( Qgis::RenderUnit maximumTailLengthUnit )
{
  mMaximumTailLengthUnit = maximumTailLengthUnit;
}

double QgsVectorFieldTracesSettings::maximumTailLength() const
{
  return mMaximumTailLength;
}

void QgsVectorFieldTracesSettings::setMaximumTailLength( double maximumTailLength )
{
  mMaximumTailLength = maximumTailLength;
}

int QgsVectorFieldTracesSettings::particlesCount() const
{
  return mParticlesCount;
}

void QgsVectorFieldTracesSettings::setParticlesCount( int value )
{
  mParticlesCount = value;
}
