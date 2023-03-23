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
#include "qgsgeometry.h"

#include <QString>
#include <QMap>
#include <memory>

class QgsMapSettings;

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

    /**
     * Copy constructor.
     */
    QgsLayerTreeFilterSettings( const QgsLayerTreeFilterSettings &other );

    QgsLayerTreeFilterSettings &operator=( const QgsLayerTreeFilterSettings &other );

    /**
     * Sets the map \a settings used to filter the legend content.
     *
     * \see mapSettings()
     */
    void setMapSettings( const QgsMapSettings &settings );

    /**
     * Returns the map settings used to filter the legend content.
     *
     * \see setMapSettings()
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
     * Returns the layers which should be shown in the legend.
     */
    QList<QgsMapLayer *> layers() const;

  private:

    QMap<QString, QString> mLayerFilterExpressions;

    std::unique_ptr<QgsMapSettings> mMapSettings;

    QgsGeometry mFilterPolygon;

    Qgis::LayerTreeFilterFlags mFlags;

};

#endif // QGSLAYERTREEFILTERSETTINGS_H
