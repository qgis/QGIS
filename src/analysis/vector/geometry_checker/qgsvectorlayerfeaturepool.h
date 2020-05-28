/***************************************************************************
                      qgsvectorlayerfeaturepool.h
                     --------------------------------------
Date                 : 18.9.2018
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

#ifndef QGSVECTORLAYERFEATUREPOOL_H
#define QGSVECTORLAYERFEATUREPOOL_H

#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * A feature pool based on a vector layer.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsVectorLayerFeaturePool : public QObject, public QgsFeaturePool
{
    Q_OBJECT

  public:

    /**
     * Creates a new feature pool for \a layer.
     */
    QgsVectorLayerFeaturePool( QgsVectorLayer *layer );

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) override;
    void updateFeature( QgsFeature &feature ) override;
    void deleteFeature( QgsFeatureId fid ) override;

  private slots:
    void onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );
    void onFeatureDeleted( QgsFeatureId fid );
};

#endif // QGSVECTORLAYERFEATUREPOOL_H
