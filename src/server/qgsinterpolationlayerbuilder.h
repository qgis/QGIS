/***************************************************************************
                              qgsinterpolationlayerbuilder.h
                              ------------------------------
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

#ifndef QGSINTERPOLATIONLAYERBUILDER_H
#define QGSINTERPOLATIONLAYERBUILDER_H

#include "qgsmslayerbuilder.h"

class QgsVectorLayer;

/**A class that produces a rasterlayer from a vector layer with spatial interpolation*/
class QgsInterpolationLayerBuilder: public QgsMSLayerBuilder
{
  public:
    QgsInterpolationLayerBuilder( QgsVectorLayer* vl );
    ~QgsInterpolationLayerBuilder();

    /**Creates a maplayer from xml tag
       @param elem xml element containing description of datasource
       @param filesToRemove list to append files that should be removed after the request
       @param layersToRemove list to append layers that should be removed after the request
       @param allowCaching flag if layers are allowed to be fetched from layer cache or not
     @return the created layer or 0 in case of error*/
    QgsMapLayer* createMapLayer( const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true ) const override;

  private:
    QgsInterpolationLayerBuilder(); //forbidden

    QgsVectorLayer* mVectorLayer;
};

#endif
