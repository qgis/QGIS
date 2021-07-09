/***************************************************************************
                             qgsproviderutils.cpp
                             ----------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderutils.h"
#include "qgsprovidersublayerdetails.h"

bool QgsProviderUtils::sublayerDetailsAreIncomplete( const QList<QgsProviderSublayerDetails> &details )
{
  for ( const QgsProviderSublayerDetails &sublayer : details )
  {
    switch ( sublayer.type() )
    {
      case QgsMapLayerType::VectorLayer:
        if ( sublayer.wkbType() == QgsWkbTypes::Unknown )
          return true;
        break;

      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
        break;
    }
  }

  return false;
}
