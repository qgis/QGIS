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
 * specified QgsVectorLayer
 *
 * \note QML Type: IdentifyKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickIdentifyKit : public QObject
{
    Q_OBJECT
    //! map settings. Set directly when creating QML object
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
      * Search radius for the identify functions from the point. Default is 8
      */
    Q_PROPERTY( double searchRadiusMm READ searchRadiusMm WRITE setSearchRadiusMm NOTIFY searchRadiusMmChanged )

    /**
      * Maximum number of feature returned from by the identify functions in QgsFeatureList. Default is 100
      */
    Q_PROPERTY( long featuresLimit MEMBER mFeaturesLimit NOTIFY featuresLimitChanged )

  public:
    //! create new identify kit
    explicit QgsQuickIdentifyKit( QObject *parent = 0 );

    //! Getter for map settings
    QgsQuickMapSettings *mapSettings() const;

    //! Set map settings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! Return search radius for looking for features.
    double searchRadiusMm() const;
    //! Set search radius
    void setSearchRadiusMm( double searchRadiusMm );

    /**
      * Get the closest feature to the point from the layer in case it is identifiable layer
      */
    Q_INVOKABLE QgsFeature identifyOne( QgsVectorLayer *layer, const QPointF &point );

    /**
      * Get the closest feature to the point from any identifiable layer
      */
    Q_INVOKABLE QgsQuickFeature identifyOne( const QPointF &point );

    /**
      * Get all features interseting the point from the layer in case it is identifiable layer
      */
    Q_INVOKABLE QgsFeatureList identify( QgsVectorLayer *layer, const QPointF &point );

    /**
      * Get all features interseting the point from any identifiable layer
      */
    Q_INVOKABLE QList<QgsQuickFeature> identify( const QPointF &point );


  signals:
    //! map settings changed
    void mapSettingsChanged();
    //! search radius changed
    void searchRadiusMmChanged();
    //! features limit changed
    void featuresLimitChanged();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr;

    double searchRadiusMU( const QgsRenderContext &context ) const;
    double searchRadiusMU() const;

    QgsRectangle toLayerCoordinates( QgsMapLayer *layer, const QgsRectangle &rect ) const;
    QgsFeatureList identifyVectorLayer( QgsVectorLayer *layer, const QgsPointXY &point ) const;

    double mSearchRadiusMm;
    int mFeaturesLimit;
};

#endif // QGSQUICKIDENTIFYKIT_H
