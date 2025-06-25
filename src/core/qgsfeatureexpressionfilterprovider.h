/***************************************************************************
                         qgsfeatureexpressionfilterprovider.h
                         ------------------
  begin                : 2025-07-26
  copyright            : (C) 2025 by Mathieu Pellerin
  email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATUREEXPRESSIONFILTERPROVIDER_H
#define QGSFEATUREEXPRESSIONFILTERPROVIDER_H

#include "qgsfeaturefilterprovider.h"
#include "qgis_core.h"

#include <QMap>

class QgsExpression;

/**
 * \ingroup core
 * \class QgsFeatureExpressionFilterProvider
 * \brief A feature filter provider allowing to set filter expressions on a per-layer basis.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsFeatureExpressionFilterProvider : public QgsFeatureFilterProvider
{
  public:
    //! Constructor
    QgsFeatureExpressionFilterProvider() = default;

    /**
     * Filter the features of the layer
     * \param layer the layer to control
     * \param filterFeatures the request to fill
     * \deprecated QGIS 4.0. Use the layer ID variant.
     */
    Q_DECL_DEPRECATED void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override SIP_DEPRECATED;

    /**
     * Filter the features of the layer
     * \param layerId the layer ID to control
     * \param filterFeatures the request to fill
     */
    void filterFeatures( const QString &layerId, QgsFeatureRequest &filterFeatures ) const override;

    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;

    /**
     * Returns a clone of the object
     * \returns A clone
     */
    QgsFeatureExpressionFilterProvider *clone() const override SIP_FACTORY;

    /**
     * Set a filter for the given layer.
     * \param layer the layer to filter
     * \param expression the filter expression
     * \deprecated QGIS 4.0. Use the layer ID variant.
     */
    Q_DECL_DEPRECATED void setFilter( const QgsVectorLayer *layer, const QgsExpression &expression ) SIP_DEPRECATED;

    /**
     * Set a filter for the given layer.
     * \param layerId the layer to filter
     * \param expression the filter expression
     */
    void setFilter( const QString &layerId, const QgsExpression &expression );

  private:
    QMap<QString, QString> mFilters;
};

#endif
