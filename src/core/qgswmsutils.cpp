/***************************************************************************
                          qgswmsutils.cpp
                          ---------------
    begin                : September 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsutils.h"

#include "qgsproviderregistry.h"

bool QgsWmsUtils::isWmsLayer( QgsMapLayer *layer )
{
  if ( !layer || layer->providerType() != "wms"_L1 )
    return false;

  // Discard WMTS layers
  const QString url = layer->publicSource();
  if ( url.contains( "SERVICE=WMTS"_L1, Qt::CaseInsensitive ) || url.contains( "/WMTSCapabilities.xml"_L1, Qt::CaseInsensitive ) )
  {
    return false;
  }

  // Discard XYZ layers
  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( u"wms"_s, layer->source() );
  if ( parts.value( u"type"_s ).toString() == "xyz"_L1 )
  {
    return false;
  }

  return true;
}

QString QgsWmsUtils::wmsVersion( QgsRasterLayer *layer )
{
  if ( !layer || !layer->dataProvider() || !QgsWmsUtils::isWmsLayer( layer ) )
    return QString();

  return layer->dataProvider()->metadata().value( u"WmsVersion"_s, QString() ).toString();
}
