/***************************************************************************
                              qgsmslayerbuilder.h
                              -------------------
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

#ifndef QGSMSLAYERBUILDER_H
#define QGSMSLAYERBUILDER_H

class QgsMapLayer;
class QgsRasterLayer;
class QDomElement;
class QTemporaryFile;

#include <QList>

/**Abstract base class for layer builders.
 Provides the possibility to create QGIS maplayers
from xml tag*/
class QgsMSLayerBuilder
{
  public:
    QgsMSLayerBuilder();
    virtual ~QgsMSLayerBuilder();

    /**Creates a maplayer from xml tag
       @param elem xml element containing description of datasource
       @param layerName sld name of the maplayer
       @param filesToRemove list to append files that should be removed after the request
       @param layersToRemove list to append layers that should be removed after the request
       @param allowCaching flag if layers are allowed to be fetched from layer cache or not
     @return the created layer or 0 in case of error*/
    virtual QgsMapLayer* createMapLayer( const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true ) const = 0;
  protected:
    /**Tries to create a suitable layer name from a URL. */
    virtual QString layerNameFromUri( const QString& uri ) const;
    /**Helper function that creates a new temporary file with random name under /tmp/qgis_wms_serv/
    and returns the path of the file (Unix). On Windows, it is created in the current working directory \
    and returns the filename only*/
    QString createTempFile() const;
    /**Resets the former symbology of a raster layer. This is important for single band layers (e.g. dems)
     coming from the cash*/
    void clearRasterSymbology( QgsRasterLayer* rl ) const;
};

#endif
