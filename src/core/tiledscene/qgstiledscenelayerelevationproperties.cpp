/***************************************************************************
                         qgstiledscenelayerelevationproperties.cpp
                         ---------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgstiledscenelayerelevationproperties.h"
#include "moc_qgstiledscenelayerelevationproperties.cpp"
#include "qgstiledscenelayer.h"

QgsTiledSceneLayerElevationProperties::QgsTiledSceneLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
}

bool QgsTiledSceneLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsTiledSceneLayerElevationProperties::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  writeCommonProperties( element, document, context );

  parentElement.appendChild( element );
  return element;
}

bool QgsTiledSceneLayerElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement elevationElement = element.firstChildElement( QStringLiteral( "elevation" ) ).toElement();
  readCommonProperties( elevationElement, context );

  return true;
}

QgsTiledSceneLayerElevationProperties *QgsTiledSceneLayerElevationProperties::clone() const
{
  std::unique_ptr< QgsTiledSceneLayerElevationProperties > res = std::make_unique< QgsTiledSceneLayerElevationProperties >( nullptr );
  res->copyCommonProperties( this );

  return res.release();
}

QString QgsTiledSceneLayerElevationProperties::htmlSummary() const
{
  QStringList properties;
  properties << tr( "Scale: %1" ).arg( mZScale );
  properties << tr( "Offset: %1" ).arg( mZOffset );
  return QStringLiteral( "<ul><li>%1</li></ul>" ).arg( properties.join( QLatin1String( "</li><li>" ) ) );
}

QgsDoubleRange QgsTiledSceneLayerElevationProperties::calculateZRange( QgsMapLayer *layer ) const
{
  if ( QgsTiledSceneLayer *tiledSceneLayer = qobject_cast< QgsTiledSceneLayer * >( layer ) )
  {
    if ( QgsTiledSceneDataProvider *dp = tiledSceneLayer->dataProvider() )
    {
      const QgsDoubleRange providerRange = dp->zRange();
      if ( providerRange.isInfinite() || providerRange.isEmpty() )
        return QgsDoubleRange();

      return QgsDoubleRange( dp->zRange().lower() * mZScale + mZOffset, dp->zRange().upper() * mZScale + mZOffset );
    }
  }

  return QgsDoubleRange();
}

QList<double> QgsTiledSceneLayerElevationProperties::significantZValues( QgsMapLayer *layer ) const
{
  const QgsDoubleRange range = calculateZRange( layer );
  if ( !range.isInfinite() && range.lower() != range.upper() )
    return {range.lower(), range.upper() };
  else if ( !range.isInfinite() )
    return {range.lower() };
  else
    return {};
}
