/***************************************************************************
                      qgsvectordataproviderfeaturepool.h
                     --------------------------------------
Date                 : 3.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORDATAPROVIDERFEATUREPOOL_H
#define QGSVECTORDATAPROVIDERFEATUREPOOL_H

#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * A feature pool based on a vector data provider.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsVectorDataProviderFeaturePool : public QgsFeaturePool
{
  public:
    QgsVectorDataProviderFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform, bool selectedOnly = false );

    void addFeature( QgsFeature &feature );
    void updateFeature( QgsFeature &feature );
    void deleteFeature( QgsFeatureId fid );

  private:
    bool mSelectedOnly = false;
};

#endif // QGSVECTORDATAPROVIDERFEATUREPOOL_H
