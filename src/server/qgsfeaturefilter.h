/***************************************************************************
                              qgsfeaturefilter.h
                              ------------------
  begin                : 26-10-2017
  copyright            : (C) 2017 by Patrick Valsecchi
  email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATUREFILTER_H
#define QGSFEATUREFILTER_H

#include "qgsfeaturefilterprovider.h"
#include "qgis_server.h"

#include <QMap>

class QgsExpression;

/**
 * \ingroup server
 * \class QgsFeatureFilter
 * \brief A feature filter provider allowing to set filter expressions on a per-layer basis.
 * \deprecated QGIS 3.4. Use QgsFeatureExpressionFilterProvider
 */
class SERVER_EXPORT QgsFeatureFilter : public QgsFeatureFilterProvider
{
  public:
    //! Constructor
    QgsFeatureFilter() = default;

    bool isFilterThreadSafe() const override { return false; }

    void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override;
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;
    QgsFeatureFilterProvider *clone() const override SIP_FACTORY;

    /**
     * Set a filter for the given layer.
     * \param layer the layer to filter
     * \param expression the filter expression
     */
    void setFilter( const QgsVectorLayer *layer, const QgsExpression &expression );

  private:
    QMap<QString, QString> mFilters;
};

#endif
