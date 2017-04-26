/***************************************************************************
                         qgsfeaturesink.h
                         ----------------
    begin                : April 2017
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

#ifndef QGSFEATURESINK_H
#define QGSFEATURESINK_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

/**
 * \class QgsFeatureSink
 * \ingroup core
 * An interface for objects which accept features via addFeature(s) methods.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeatureSink
{
  public:

    virtual ~QgsFeatureSink() = default;

    /**
     * Adds a single \a feature to the sink.
     * \see addFeatures()
     * \returns true in case of success and false in case of failure
     */
    virtual bool addFeature( QgsFeature &feature );

    /**
     * Adds a list of \a features to the sink.
     * \see addFeature()
     * \returns true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &features ) = 0;

    /**
     * Adds all features from the specified \a iterator to the sink.
     * \returns true if all features were added successfully, or false if any feature could not be added
     */
    virtual bool addFeatures( QgsFeatureIterator &iterator );

};

#endif // QGSFEATURESINK_H
