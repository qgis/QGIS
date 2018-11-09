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
#include "qgsquickfeaturelayerpair.h"

class QgsMapLayer;
class QgsQuickMapSettings;
class QgsVectorLayer;

/**
 * \ingroup quick
 *
 * Convenient set of tools to identify features
 *
 * - get a list of features in a defined radius from a point.
 * - get a feature with the closest distance to the point
 *
 * \note QML Type: IdentifyKit
 *
 * \since QGIS 3.4
 */
class QUICK_EXPORT QgsQuickIdentifyKit : public QObject
{
    Q_OBJECT

    /**
     * Map settings. Set directly when creating QML object.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
     * Search radius for the identify functions
     *
     * Default is 8.
     */
    Q_PROPERTY( double searchRadiusMm READ searchRadiusMm WRITE setSearchRadiusMm NOTIFY searchRadiusMmChanged )

    /**
     * Maximum number of features returned from the QgsQuickIdentifyKit::identify()
     *
     * Default is 100.
     */
    Q_PROPERTY( int featuresLimit READ featuresLimit WRITE setFeaturesLimit NOTIFY featuresLimitChanged )

    /**
     * Defines behavior of the identify tool (See description of IdentifyMode enum).
     *
     * Default is TopDownAll.
     */
    Q_PROPERTY( IdentifyMode identifyMode MEMBER mIdentifyMode NOTIFY identifyModeChanged )

  public:

    /**
     * IdentifyMode enums used to define identify tool behavior on identifiable layers.
     */
    enum IdentifyMode
    {

      /**
       * Identification is performed from top to bottom down layers returning all
       * identified features;
       */
      TopDownAll = 0,

      /**
       * Identification is performed from top to bottom down layers and stops on the first layer
       * returning non-empty list of identified features. Identification on rest layers is skipped.
       */
      TopDownStopAtFirst
    };
    Q_ENUM( IdentifyMode )

    //! Constructor of new identify kit.
    explicit QgsQuickIdentifyKit( QObject *parent = nullptr );

    //! \copydoc QgsQuickIdentifyKit::mapSettings
    QgsQuickMapSettings *mapSettings() const;

    //! \copydoc QgsQuickIdentifyKit::mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    double searchRadiusMm() const;

    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    void setSearchRadiusMm( double searchRadiusMm );

    //! \copydoc QgsQuickIdentifyKit::featuresLimit
    int featuresLimit() const;

    //! \copydoc QgsQuickIdentifyKit::featuresLimit
    void setFeaturesLimit( int limit );

    /**
      * Gets the closest feature to the point within the search radius
      *
      * If layer is nullptr, identifies the closest feature from either
      * all identifiable layers (IdentifyMode::TopDownAll) or the first layer from top to bottom layers
      * with non-empty identified feature list (IdentifyMode::TopDownStopAtFirst)
      * If layer is not nullptr, identifies the closest feature from given layer regardless identify mode.
      *
      * To modify search radius, use QgsQuickIdentifyKit::searchRadiusMm
      *
      * \param point position to search a feature from
      * \param layer if defined, search for a feature only from this layer
      */
    Q_INVOKABLE QgsQuickFeatureLayerPair identifyOne( const QPointF &point, QgsVectorLayer *layer = nullptr );

    /**
      * Gets all features in the search radius
      *
      * If layer is nullptr, identifies features from either
      * all identifiable layers (IdentifyMode::TopDownAll) or the first layer from top to bottom layers
      * with non-empty identified feature list (IdentifyMode::TopDownStopAtFirst)
      * If layer is not nullptr, identifies only features from given layer regardless identify mode.
      *
      * To limit number of results, use QgsQuickIdentifyKit::featuresLimit
      * To modify search radius, use QgsQuickIdentifyKit::searchRadiusMm
      *
      * \param point position to search features ob
      * \param layer if defined, search for features only from this layer
      */
    Q_INVOKABLE QgsQuickFeatureLayerPairs identify( const QPointF &point, QgsVectorLayer *layer = nullptr );

  signals:
    //! \copydoc QgsQuickIdentifyKit::mapSettings
    void mapSettingsChanged();
    //! \copydoc QgsQuickIdentifyKit::searchRadiusMm
    void searchRadiusMmChanged();
    //! \copydoc QgsQuickIdentifyKit::featuresLimit
    void featuresLimitChanged();
    //! \copydoc QgsQuickIdentifyKit::identifyMode
    void identifyModeChanged();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned

    double searchRadiusMU( const QgsRenderContext &context ) const;
    double searchRadiusMU() const;

    QgsRectangle toLayerCoordinates( QgsMapLayer *layer, const QgsRectangle &rect ) const;
    QgsFeatureList identifyVectorLayer( QgsVectorLayer *layer, const QgsPointXY &point ) const;

    double mSearchRadiusMm = 8;
    int mFeaturesLimit = 100;
    IdentifyMode mIdentifyMode = IdentifyMode::TopDownAll;
};

#endif // QGSQUICKIDENTIFYKIT_H
