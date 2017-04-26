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
     */
    virtual bool addFeature( QgsFeature &feature ) = 0;

    /**
     * Adds a list of \a features to the sink.
     * \see addFeature()
     */
    virtual bool addFeatures( QgsFeatureList &features ) = 0;

};

#endif // QGSFEATURESINK_H
