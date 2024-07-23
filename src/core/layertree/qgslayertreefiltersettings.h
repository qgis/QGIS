/***************************************************************************
  qgslayertreefiltersettings.h
  --------------------------------------
  Date                 : March 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEFILTERSETTINGS_H
#define QGSLAYERTREEFILTERSETTINGS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsexpressioncontext.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"

#include <QString>
#include <QMap>
#include <memory>

class QgsMapSettings;
class QgsLayerTree;
class QgsReferencedGeometry;

/**
 * \ingroup core
 * \brief Contains settings relating to filtering the contents of QgsLayerTreeModel and views.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsLayerTreeFilterSettings
{
  public:

    /**
     * Constructor for QgsLayerTreeFilterSettings, using the specified map \a settings.
     */
    explicit QgsLayerTreeFilterSettings( const QgsMapSettings &settings );

    ~QgsLayerTreeFilterSettings();

    QgsLayerTreeFilterSettings( const QgsLayerTreeFilterSettings &other );

    QgsLayerTreeFilterSettings &operator=( const QgsLayerTreeFilterSettings &other );

    /**
     * Returns the map settings used to filter the legend content.
     */
    QgsMapSettings &mapSettings();

    /**
     * Returns the map of layer IDs to legend filter expression.
     *
     * \see layerFilterExpression()
     * \see setLayerFilterExpressions()
     */
    QMap<QString, QString> layerFilterExpressions() const;

    /**
     * Sets the map of layer IDs to legend filter expression.
     *
     * \see layerFilterExpressions()
     */
    void setLayerFilterExpressions( const QMap<QString, QString> &expressions );

    /**
     * Sets layer filter expressions using a layer \a tree.
     */
    void setLayerFilterExpressionsFromLayerTree( QgsLayerTree *tree );

    /**
     * Returns the filter expression to use for the layer with the specified
     * \a layerId, or an empty string if no expression is set for the layer.
     *
     * \see layerFilterExpressions()
     * \see setLayerFilterExpressions()
     */
    QString layerFilterExpression( const QString &layerId ) const;

    /**
     * Returns the optional filter polygon, used when testing for symbols to show in
     * the legend.
     *
     * The CRS of the polygon will match the destination CRS of mapSettings().
     *
     * If not set then the filter visibility extent will use the extent of mapSettings().
     *
     * \see setFilterPolygon()
     */
    QgsGeometry filterPolygon() const;

    /**
     * Sets the optional filter \a polygon, used when testing for symbols to show in
     * the legend.
     *
     * The CRS of the polygon must match the destination CRS of mapSettings().
     *
     * If not set then the filter visibility extent will use the extent of mapSettings().
     *
     * \see filterPolygon()
     */
    void setFilterPolygon( const QgsGeometry &polygon );

    /**
     * Returns the filter flags.
     *
     * \see setFlags()
     */
    Qgis::LayerTreeFilterFlags flags() const;

    /**
     * Sets the filter \a flags.
     *
     * \see flags()
     */
    void setFlags( Qgis::LayerTreeFilterFlags flags );

    /**
     * Adds a visible extent \a polygon for a map \a layer.
     *
     * If \a layer is already included in the layers contained within mapSettings() (or previously added by
     * calling this method) then this \a polygon extent will be unioned with the existing extent.
     *
     * The \a layer will be appended to the list of layers to use during the legend hit test. (See layers()).
     */
    void addVisibleExtentForLayer( QgsMapLayer *layer, const QgsReferencedGeometry &polygon );

    /**
     * Returns the combined visible extent for a \a layer.
     *
     * The combined visible extent includes:
     *
     * - the mapSettings() extent (respecting filterPolygon() if set) IF the layer is contained in mapSettings()
     * - all additional extents added by calls to addVisibleExtentForLayer()
     *
     * The returned geometry will always be in the layer's CRS.
     */
    QgsGeometry combinedVisibleExtentForLayer( const QgsMapLayer *layer );

    /**
     * Returns the layers which should be shown in the legend.
     *
     * This includes all layers from the mapSettings() and any additional layers added by calls to
     * addVisibleExtentForLayer().
     *
     * \see addVisibleExtentForLayer()
     */
    QList<QgsMapLayer *> layers() const;

  private:

    QMap<QString, QString> mLayerFilterExpressions;

    std::unique_ptr<QgsMapSettings> mMapSettings;

    QgsGeometry mFilterPolygon;

    Qgis::LayerTreeFilterFlags mFlags;

    QgsWeakMapLayerPointerList mLayers;

    // geometry must be in layer CRS
    QMap<QString, QVector< QgsGeometry > > mLayerExtents;

};

#endif // QGSLAYERTREEFILTERSETTINGS_H
