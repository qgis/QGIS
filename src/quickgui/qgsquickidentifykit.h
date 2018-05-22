/***************************************************************************
  qgsquickidentifykit.h
 ---------------------
  Date                 : 30.8.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKIDENTIFYKIT_H
#define QGSQUICKIDENTIFYKIT_H

#include <QObject>
#include <QPair>

#include "qgsfeature.h"
#include "qgsmapsettings.h"
#include "qgspoint.h"
#include "qgsrendercontext.h"

#include "qgis_quick.h"
#include "qgsquickfeature.h"

class QgsMapLayer;
class QgsQuickMapSettings;
class QgsVectorLayer;

/**
 * \ingroup quick
 * Convenient set of tools to get a list of QgsFeatures in a defined radius from a point.
 * Also possible to get a feature with the closest distance to the point or feature(s) from
 * specified QgsVectorLayer.
 *
 * \note QML Type: IdentifyKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickIdentifyKit : public QObject
{
    Q_OBJECT

    /**
      * Map settings. Set directly when creating QML object.
      */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
      * Search radius for the identify functions from the point. Default is 8.
      */
    Q_PROPERTY( double searchRadiusMm READ searchRadiusMm WRITE setSearchRadiusMm NOTIFY searchRadiusMmChanged )

    /**
      * Maximum number of feature returned from by the identify functions in QgsFeatureList. Default is 100.
      */
    Q_PROPERTY( long featuresLimit MEMBER mFeaturesLimit NOTIFY featuresLimitChanged )

  public:
    //! Constructor of new identify kit.
    explicit QgsQuickIdentifyKit( QObject *parent = 0 );

    //! \copydoc QgsQuickIdentifyKit::mapSettings
    QgsQuickMapSettings *mapSettings() const;

    //! \copydoc QgsQuickIdentifyKit::mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    double searchRadiusMm() const;

    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    void setSearchRadiusMm( double searchRadiusMm );

    /**
      * Gets the closest feature to the point. If given layer is defined, identifies only features from it,
      * otherwise searches among identifiable layers.
      * If a layer param is undefined, identify feature from any identifiable layer.
      * \param point QPointF position
      * \param layer QgsVectorLayer used for identifying if is defined, otherwise identifiable layer.
      */
    Q_INVOKABLE QgsQuickFeature identifyOne( const QPointF &point, QgsVectorLayer *layer = nullptr );

    /**
      * Gets all features interseting the point. If layer is defined, identifies only features from given layer,
      * otherwise searches among identifiable layers.
      * \param point QPointF used for identifying.
      * \param layer QgsVectorLayer used for identifying if is defined, otherwise identifiable layer.
      */
    Q_INVOKABLE QgsQuickFeatureList identify( const QPointF &point, QgsVectorLayer *layer = nullptr );

  signals:
    //! \copydoc QgsQuickIdentifyKit::mapSettings
    void mapSettingsChanged();
    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    void searchRadiusMmChanged();
    //! \copydoc QgsQuickIdentifyKit::featuresLimit
    void featuresLimitChanged();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned

    double searchRadiusMU( const QgsRenderContext &context ) const;
    double searchRadiusMU() const;

    QgsRectangle toLayerCoordinates( QgsMapLayer *layer, const QgsRectangle &rect ) const;
    QgsFeatureList identifyVectorLayer( QgsVectorLayer *layer, const QgsPointXY &point ) const;

    double mSearchRadiusMm = 8;
    int mFeaturesLimit = 100;
};

#endif // QGSQUICKIDENTIFYKIT_H
