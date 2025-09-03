/***************************************************************************
                              qgsfeaturefilterprovider.h
                              --------------------------
  begin                : 22-05-2015
  copyright            : (C) 2008 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATUREFILTERPROVIDER_H
#define QGSFEATUREFILTERPROVIDER_H

#include <QtGlobal>
#include <QStringList>
#include "qgis_sip.h"

#include "qgis_core.h"

class QString;
class QgsVectorLayer;
class QgsFeatureRequest;

/**
 * \ingroup core
 * \class QgsFeatureFilterProvider
 * \brief Abstract interface for use by classes that filter the features or attributes of a layer.
 *
 * A QgsFeatureFilterProvider provides a method for modifying a QgsFeatureRequest in place to apply
 * additional filters to the request, since QGIS 3.18 a method to filter allowed attributes is also available.
 *
 */

class CORE_EXPORT QgsFeatureFilterProvider
{
  public:

#ifndef SIP_RUN

    QgsFeatureFilterProvider() = default;

    virtual ~QgsFeatureFilterProvider() = default;

#endif

    /**
     * Returns TRUE if the filterFeature function is thread safe, which will lead to reliance on layer ID instead
     * of the raw layer pointer.
     * \since QGIS 4.0
     * \deprecated QGIS 4.0
     */
    Q_DECL_DEPRECATED virtual bool isFilterThreadSafe() const SIP_DEPRECATED { return true; }

    /**
     * Add additional filters to the feature request to further restrict the features returned by the request.
     * Derived classes must implement this method.
     * \param layer the layer to filter
     * \param featureRequest the feature request to update
     * \deprecated QGIS 4.0. Use the layer ID variant.
     */
    Q_DECL_DEPRECATED virtual void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &featureRequest ) const SIP_DEPRECATED
    {
      Q_UNUSED( layer )
      Q_UNUSED( featureRequest )
    };

    /**
     * Add additional filters to the feature request to further restrict the features returned by the request.
     * Derived classes must implement this method.
     * \param layerId the layer ID to filter
     * \param featureRequest the feature request to update
     * \since QGIS 4.0
     */
    virtual void filterFeatures( const QString &layerId, QgsFeatureRequest &featureRequest ) const
    {
      Q_UNUSED( layerId )
      Q_UNUSED( featureRequest )
    };

    /**
     * Returns the list of visible attribute names from a list of \a attributes names for the given \a layer
     * \since QGIS 3.18
     */
    virtual QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const = 0;

    /**
     * Create a clone of the feature filter provider
     * \returns a new clone
     */
    virtual QgsFeatureFilterProvider *clone() const = 0 SIP_FACTORY;
};

#endif
