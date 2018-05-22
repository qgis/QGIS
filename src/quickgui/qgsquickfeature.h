/***************************************************************************
  qgsquickfeature.h
 ---------------------
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

#ifndef QGSQUICKFEATURE_H
#define QGSQUICKFEATURE_H

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
 * Note that the feature may or may not be part of the vector layer's
 * associated features
 *
 * \note QML Type: QgsQuickFeature
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickFeature
{
    Q_GADGET

    /**
     * Vector layer to which the feature belongs.
     *
     * This is a readonly property.
     */
    Q_PROPERTY( QgsVectorLayer *layer READ layer )

    /**
     * Feature instance itself.
     *
     * This is a readonly property.
     */
    Q_PROPERTY( QgsFeature feature READ feature )

    /**
     * Whether the feature is valid and vector layer assigned.
     *
     * This is a readonly property.
     */
    Q_PROPERTY( bool valid READ valid )

  public:
    //! Constructor of a new feature.
    QgsQuickFeature();

    /**
     * Constructor of a new feature.
     * \param feature QgsFeature associated.
     * \param layer Vector layer which the feature belongs to, if not defined, the feature is not valid.
     */
    QgsQuickFeature( const QgsFeature &feature,
                     QgsVectorLayer *layer );

    //! \copydoc QgsQuickFeature::layer
    QgsVectorLayer *layer() const;

    //! \copydoc QgsQuickFeature::feature
    QgsFeature feature() const;

    //! \copydoc QgsQuickFeature::valid
    bool valid() const;

    //! \copydoc QgsQuickFeature::feature
    void setFeature( const QgsFeature &feature );

    //! \copydoc QgsQuickFeature::layer
    void setLayer( QgsVectorLayer *layer );

  private:
    QgsVectorLayer *mLayer = nullptr; // not owned
    QgsFeature mFeature;
};

typedef QList<QgsQuickFeature> QgsQuickFeatureList;

Q_DECLARE_METATYPE( QgsQuickFeature )

#endif // QGSQUICKFEATURE_H
