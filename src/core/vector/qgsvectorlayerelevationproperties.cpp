/***************************************************************************
                         qgsvectorlayerelevationproperties.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerelevationproperties.h"
#include "qgsrasterlayer.h"

QgsVectorLayerElevationProperties::QgsVectorLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
}

bool QgsVectorLayerElevationProperties::hasElevation() const
{
  // layer is considered as having non-default elevation settings if we aren't clamping to terrain
  return mClamping != Qgis::AltitudeClamping::Terrain;
}

QDomElement QgsVectorLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  element.setAttribute( QStringLiteral( "zoffset" ), qgsDoubleToString( mZOffset ) );
  element.setAttribute( QStringLiteral( "zscale" ), qgsDoubleToString( mZScale ) );

  element.setAttribute( QStringLiteral( "extrusionEnabled" ), mEnableExtrusion ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "extrusion" ), qgsDoubleToString( mExtrusionHeight ) );
  element.setAttribute( QStringLiteral( "clamping" ), qgsEnumValueToKey( mClamping ) );
  element.setAttribute( QStringLiteral( "binding" ), qgsEnumValueToKey( mBinding ) );
  parentElement.appendChild( element );
  return element;
}

bool QgsVectorLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mZOffset = elevationElement.attribute( QStringLiteral( "zoffset" ), QStringLiteral( "0" ) ).toDouble();
  mZScale = elevationElement.attribute( QStringLiteral( "zscale" ), QStringLiteral( "1" ) ).toDouble();

  mClamping = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "clamping" ) ), Qgis::AltitudeClamping::Terrain );
  mBinding = qgsEnumKeyToValue( elevationElement.attribute( QStringLiteral( "binding" ) ), Qgis::AltitudeBinding::Centroid );
  mEnableExtrusion = elevationElement.attribute( QStringLiteral( "extrusionEnabled" ), QStringLiteral( "0" ) ).toInt();
  mExtrusionHeight = elevationElement.attribute( QStringLiteral( "extrusion" ), QStringLiteral( "0" ) ).toDouble();

  return true;
}

bool QgsVectorLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual layer z range
  return true;
}

QgsDoubleRange QgsVectorLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO -- determine actual z range from layer statistics
  return QgsDoubleRange();
}
