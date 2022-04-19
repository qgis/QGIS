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

QDomElement QgsPointCloudLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  writeCommonProperties( element, document, context );
  parentElement.appendChild( element );
  return element;
}

bool QgsPointCloudLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  readCommonProperties( elevationElement, context );
  return true;
}

QgsPointCloudLayerElevationProperties *QgsPointCloudLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsPointCloudLayerElevationProperties > res = std::make_unique< QgsPointCloudLayerElevationProperties >( nullptr );
  res->copyCommonProperties( this );
  return res.release();
}

QString QgsPointCloudLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  properties << tr( "Scale: %1" ).arg( mZScale );
  properties << tr( "Offset: %1" ).arg( mZOffset );
  return QStringLiteral( "<ul><li>%1</li></ul>" ).arg( properties.join( QStringLiteral( "</li><li>" ) ) );
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
