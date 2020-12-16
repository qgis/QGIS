/***************************************************************************
                         qgspointcloudlayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudlayer.h"

QgsPointCloudLayerElevationProperties::QgsPointCloudLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
}

bool QgsPointCloudLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsPointCloudLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  element.setAttribute( QStringLiteral( "zoffset" ), qgsDoubleToString( mZOffset ) );
  element.setAttribute( QStringLiteral( "zscale" ), qgsDoubleToString( mZScale ) );
  parentElement.appendChild( element );
  return element;
}

bool QgsPointCloudLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  mZOffset = elevationElement.attribute( QStringLiteral( "zoffset" ), QStringLiteral( "0" ) ).toDouble();
  mZScale = elevationElement.attribute( QStringLiteral( "zscale" ), QStringLiteral( "1" ) ).toDouble();
  return true;
}

bool QgsPointCloudLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual point cloud z range
  return true;
}

QgsDoubleRange QgsPointCloudLayerElevationProperties::calculateZRange( QgsMapLayer *layer ) const
{
  if ( QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layer ) )
  {
    if ( pcLayer->dataProvider() )
    {
      // try to fetch z range from provider metadata
      const QVariant zMin = pcLayer->dataProvider()->metadataStatistic( QStringLiteral( "Z" ), QgsStatisticalSummary::Min );
      const QVariant zMax = pcLayer->dataProvider()->metadataStatistic( QStringLiteral( "Z" ), QgsStatisticalSummary::Max );
      if ( zMin.isValid() && zMax.isValid() )
      {
        return QgsDoubleRange( zMin.toDouble() * mZScale + mZOffset, zMax.toDouble() * mZScale + mZOffset );
      }
    }
  }

  return QgsDoubleRange();
}
