/***************************************************************************
  qgsquickfeaturelayerpair.h
 ---------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKFEATURELAYERPAIR_H
#define QGSQUICKFEATURELAYERPAIR_H

#include <QObject>

#include "qgsfeature.h"

#include "qgis_quick.h"

class QgsVectorLayer;

/**
 * \ingroup quick
 * Pair of QgsFeature and QgsVectorLayer
 *
 * Vector layer is commonly used to gather geometry type or CRS
 * for the feature.
 *
 * Note that the feature may or may not be part of the layer's features
 *
 * \note QML Type: QgsQuickFeatureLayerPair
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickFeatureLayerPair
{
    Q_GADGET

    /**
     * Vector layer to which the feature belongs. May be NULLPTR if pair is not valid
     *
     * This is a readonly property.
     */
    Q_PROPERTY( QgsVectorLayer *layer READ layer )

    /**
     * Feature that belongs to layer.
     *
     * This is a readonly property.
     */
    Q_PROPERTY( QgsFeature feature READ feature )

    /**
     * Whether
     *  - layer is not NULLPTR
     *  - feature is valid
     *  - feature has geometry and the geometry is the same as geometry expected by layer
     *
     * This is a readonly property.
     */
    Q_PROPERTY( bool valid READ isValid )

  public:
    //! Constructs invalid feature-layer pair.
    QgsQuickFeatureLayerPair();

    /**
     * Constructor of a new feature-layer pair
     * \param feature QgsFeature associated.
     * \param layer Vector layer which the feature belongs to
     */
    QgsQuickFeatureLayerPair( const QgsFeature &feature, QgsVectorLayer *layer );

    //! \copydoc QgsQuickFeatureLayerPair::layer
    QgsVectorLayer *layer() const;

    //! \copydoc QgsQuickFeatureLayerPair::feature
    QgsFeature feature() const;

    //! \copydoc QgsQuickFeatureLayerPair::feature
    QgsFeature &featureRef();

    //! \copydoc QgsQuickFeatureLayerPair::valid
    bool isValid() const;

    bool operator==( const QgsQuickFeatureLayerPair &other ) const;
    bool operator!=( const QgsQuickFeatureLayerPair &other ) const;

  private:
    bool hasValidGeometry() const;

    QgsVectorLayer *mLayer = nullptr; // not owned
    QgsFeature mFeature;
};

typedef QList<QgsQuickFeatureLayerPair> QgsQuickFeatureLayerPairs;

Q_DECLARE_METATYPE( QgsQuickFeatureLayerPair )

#endif // QGSQUICKFEATURELAYERPAIR_H
