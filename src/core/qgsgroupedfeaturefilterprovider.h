/***************************************************************************
                       qgsgroupedfeaturefilterprovider.h
                       -------------------------------
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

#ifndef QGSGROUPEDFEATUREFILTERPROVIDER_H
#define QGSGROUPEDFEATUREFILTERPROVIDER_H

#include "qgsfeaturefilterprovider.h"
#include "qgis_core.h"

#include <QList>

/**
 * \ingroup core
 * \class QgsGroupedFeatureFilterProvider
 * \brief A filter filter provider grouping several filter providers.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsGroupedFeatureFilterProvider : public QgsFeatureFilterProvider
{
  public:
    //! Constructor
    QgsGroupedFeatureFilterProvider() = default;

    Q_DECL_DEPRECATED bool isFilterThreadSafe() const override SIP_DEPRECATED;

    Q_DECL_DEPRECATED void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override SIP_DEPRECATED;
    void filterFeatures( const QString &layerId, QgsFeatureRequest &filterFeatures ) const override;
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;
    QgsGroupedFeatureFilterProvider *clone() const override SIP_FACTORY;

    /**
     * Add another filter provider to the group
     * \param provider The provider to add
     * \return itself
     */
    QgsGroupedFeatureFilterProvider &addProvider( const QgsFeatureFilterProvider *provider );


  private:
    QList<const QgsFeatureFilterProvider *> mProviders;
};

#endif
