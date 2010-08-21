/***************************************************************************
                              qgsremotedatasourcebuilder.h
                              ----------------------------
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

#ifndef QGSRDSBUILDER_H
#define QGSRDSBUILDER_H

#include "qgsmslayerbuilder.h"
class QgsRasterLayer;
class QgsVectorLayer;

/**A class that creates map layer from <RemoteRDS> or <RemoteVDS> tags*/
class QgsRemoteDataSourceBuilder: public QgsMSLayerBuilder
{
 public:
  QgsRemoteDataSourceBuilder();
  ~QgsRemoteDataSourceBuilder();
  QgsMapLayer* createMapLayer(const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;

 private:
  /**Creates a raster layer from a <RemoteRDS>. This function loads the data into a temporary file and creates a rasterlayer from it. Returns a 0 pointer in case of error*/
  QgsRasterLayer* rasterLayerFromRemoteRDS(const QDomElement& remoteRDSElem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;
  /**Saves the vector data into a temporary file and creates a vector layer. Returns a 0 pointer in case of error*/
  QgsVectorLayer* vectorLayerFromRemoteVDS(const QDomElement& remoteVDSElem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;

  /**Loads data from http or ftp
   @return 0 in case of success*/
  int loadData(const QString& url, QByteArray& data) const;
};

#endif
