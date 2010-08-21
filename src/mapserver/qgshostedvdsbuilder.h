/***************************************************************************
                              qgshostedvdsbuilder.h    
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

#ifndef QGSHOSTEDVDSBUILDER_H
#define QGSHOSTEDVDSBUILDER_H

#include "qgsmslayerbuilder.h"

class QgsHostedVDSBuilder: public QgsMSLayerBuilder
{
 public:
  QgsHostedVDSBuilder();
  ~QgsHostedVDSBuilder();
  QgsMapLayer* createMapLayer(const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching = true) const;
};

#endif
