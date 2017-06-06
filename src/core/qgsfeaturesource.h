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
#include "qgsfeaturerequest.h"

class QgsFeatureIterator;
class QgsCoordinateReferenceSystem;
class QgsFields;

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

    /**
     * Returns the coordinate reference system for features in the source.
     */
    virtual QgsCoordinateReferenceSystem sourceCrs() const = 0;

    /**
     * Returns the fields associated with features in the source.
     */
    virtual QgsFields fields() const = 0;

    /**
     * Returns the geometry type for features returned by this source.
     */
    virtual QgsWkbTypes::Type wkbType() const = 0;

#ifdef SIP_RUN

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->featureCount();
    % End
#endif

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    virtual long featureCount() const = 0;

    /**
     * Returns the set of unique values contained within the specified \a fieldIndex from this source.
     * If specified, the \a limit option can be used to limit the number of returned values.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     */
    virtual QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const;

    /**
     * Returns the extent of all geometries from the source.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     */
    virtual QgsRectangle sourceExtent() const;

};

Q_DECLARE_METATYPE( QgsFeatureSource * )

#endif // QGSFEATURESOURCE_H
