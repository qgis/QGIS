/***************************************************************************
                       qgsfeaturefilterprovidergroup.h
                       -------------------------------
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

#ifndef QGSFEATUREFILTERPROVIDERGROUP_H
#define QGSFEATUREFILTERPROVIDERGROUP_H

#include "qgsfeaturefilterprovider.h"
#include "qgis_server.h"

#include <QList>

/**
 * \ingroup server
 * \class QgsFeatureFilterProviderGroup
 * \brief A filter filter provider grouping several filter providers.
 * \deprecated QGIS 3.4. Use QgsGroupedFeatureFilterProvider
 */
class SERVER_EXPORT QgsFeatureFilterProviderGroup : public QgsFeatureFilterProvider
{
  public:
    //! Constructor
    QgsFeatureFilterProviderGroup() = default;

    bool isFilterThreadSafe() const override { return false; }

    void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override;
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;
    QgsFeatureFilterProviderGroup *clone() const override SIP_FACTORY;

    /**
     * Add another filter provider to the group
     * \param provider The provider to add
     * \return itself
     */
    QgsFeatureFilterProviderGroup &addProvider( const QgsFeatureFilterProvider *provider );


  private:
    QList<const QgsFeatureFilterProvider *> mProviders;
};

#endif
