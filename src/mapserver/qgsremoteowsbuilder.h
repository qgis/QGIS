/***************************************************************************
                              qgsremoteowsbuilder.h    
                              ---------------------
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

#ifndef QGSREMOTEOWSBUILDER_H
#define QGSREMOTEOWSBUILDER_H

#include "qgsmslayerbuilder.h"
#include <map>

class QgsRasterLayer;
class QgsVectorLayer;

/**Creates QGIS maplayers from <RemoteOWS> sld tags*/
class QgsRemoteOWSBuilder: public QgsMSLayerBuilder
{
 public:
  QgsRemoteOWSBuilder(const std::map<QString, QString>& parameterMap);
  ~QgsRemoteOWSBuilder();
  
  QgsMapLayer* createMapLayer(const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;

 private:
  QgsRemoteOWSBuilder(); //forbidden
  /**Creates a wms layer from a complete wms url (using http get). Returns 0 in case of error*/
  QgsRasterLayer* wmsLayerFromUrl(const QString& url, const QString& layerName, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;
  /**Creates a temporary file such that the gdal library can read from wcs*/
  QgsRasterLayer* wcsLayerFromUrl(const QString& url, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;
  /**Creates sos layer by analizing server url and LayerSensorObservationConstraints*/
  QgsVectorLayer* sosLayer(const QDomElement& remoteOWSElem, const QString& url, const QString& layerName, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;

  std::map<QString, QString> mParameterMap;
};

#endif 
