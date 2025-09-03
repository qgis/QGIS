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

    Q_DECL_DEPRECATED void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override SIP_DEPRECATED;
    void filterFeatures( const QString &layerId, QgsFeatureRequest &filterFeatures ) const override;
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;
    QgsFeatureExpressionFilterProvider *clone() const override SIP_FACTORY;

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
