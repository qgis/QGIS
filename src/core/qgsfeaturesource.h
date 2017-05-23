/***************************************************************************
                         qgsfeaturesource.h
                         ----------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURESOURCE_H
#define QGSFEATURESOURCE_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

/**
 * \class QgsFeatureSource
 * \ingroup core
 * An interface for objects which provide features via a getFeatures method.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeatureSource
{
  public:

    virtual ~QgsFeatureSource() = default;

    /**
     * Returns an iterator for the features in the source.
     * An optional \a request can be used to optimise the returned
     * iterator, eg by restricting the returned attributes or geometry.
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const = 0;

};

#endif // QGSFEATURESOURCE_H
