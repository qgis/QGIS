/***************************************************************************
                              qgssentdatasourcebuilder.h
                              --------------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENTDATASOURCEBUILDER_H
#define QGSSENTDATASOURCEBUILDER_H

#include "qgsmslayerbuilder.h"

class QgsVectorLayer;
class QgsRasterLayer;

/**Builds maplayer from <RemoteRDS> and <RemoteVDS> tags*/
class QgsSentDataSourceBuilder: public QgsMSLayerBuilder
{
  public:
    QgsSentDataSourceBuilder();
    ~QgsSentDataSourceBuilder();

    /**Creates a maplayer from xml tag
       @param elem xml element containing description of datasource
       @param filesToRemove list to append files that should be removed after the request
       @param layersToRemove list to append layers that should be removed after the request
       @param allowCaching flag if layers are allowed to be fetched from layer cache or not
     @return the created layer or 0 in case of error*/
    QgsMapLayer* createMapLayer( const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true ) const override;

  private:
    /**Creates a vector layer from a <SentVDS> tag. Returns a 0 pointer in case of error*/
    QgsVectorLayer* vectorLayerFromSentVDS( const QDomElement& sentVDSElem, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove ) const;

    /**Creates a raster layer from a <SentRDS> tag. Returns a 0 pointer in case of error*/
    QgsRasterLayer* rasterLayerFromSentRDS( const QDomElement& sentRDSElem, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove ) const;
};

#endif
