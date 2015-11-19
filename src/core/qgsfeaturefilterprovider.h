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

class QString;
class QgsVectorLayer;
class QgsFeatureRequest;


/** \ingroup core
 * Interface used by class that will filter the features of a layer.
 * The only method `filterFeatures` fill the `QgsFeatureRequest` to get only the
 * wanted features.
 **/
class CORE_EXPORT QgsFeatureFilterProvider
{
  public:

    /** Constructor */
    QgsFeatureFilterProvider() {};

    /** Destructor */
    virtual ~QgsFeatureFilterProvider() {};

    /** Add some filter to the feature request to don't have the unauthorized (unauthorised) features
     * @param layer the layer to filter
     * @param featureRequest the feature request to update
     */
    virtual void filterFeatures( const QgsVectorLayer* layer, QgsFeatureRequest& featureRequest ) const = 0;

    /** Create a clone of the feature filter provider
     * @return a new clone
     */
    virtual QgsFeatureFilterProvider* clone() const = 0;
};

#endif
