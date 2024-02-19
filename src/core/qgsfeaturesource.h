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
#include "qgis_sip.h"
#include "qgsfeaturerequest.h"

class QgsFeatureIterator;
class QgsCoordinateReferenceSystem;
class QgsFields;
class QgsFeedback;

/**
 * \class QgsFeatureSource
 * \ingroup core
 * \brief An interface for objects which provide features via a getFeatures method.
 *
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
     * Returns a friendly display name for the source. The returned value can be an empty string.
     */
    virtual QString sourceName() const = 0;

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
    virtual Qgis::WkbType wkbType() const = 0;

#ifdef SIP_RUN

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->featureCount();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End
#endif

    /**
     * Returns the number of features contained in the source, or -1
     * if the feature count is unknown.
     */
    virtual long long featureCount() const = 0;

    /**
     * Determines if there are any features available in the source.
     *
     * \since QGIS 3.2
     */
    virtual Qgis::FeatureAvailability hasFeatures() const;

    /**
     * Returns the set of unique values contained within the specified \a fieldIndex from this source.
     * If specified, the \a limit option can be used to limit the number of returned values.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     * \see minimumValue()
     * \see maximumValue()
     */
    virtual QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const;

    /**
     * Returns the minimum value for an attribute column or an invalid variant in case of error.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     * \see maximumValue()
     * \see uniqueValues()
     */
    virtual QVariant minimumValue( int fieldIndex ) const;

    /**
     * Returns the maximum value for an attribute column or an invalid variant in case of error.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     * \see minimumValue()
     * \see uniqueValues()
     */
    virtual QVariant maximumValue( int fieldIndex ) const;

    /**
     * Returns the extent of all geometries from the source.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     */
    virtual QgsRectangle sourceExtent() const;

    /**
     * Returns the 3D extent of all geometries from the source.
     * The base class implementation uses a non-optimised approach of looping through
     * all features in the source.
     * \since QGIS 3.36
     */
    virtual QgsBox3D sourceExtent3D() const;

    /**
     * Returns a list of all feature IDs for features present in the source.
     */
    virtual QgsFeatureIds allFeatureIds() const;

    /**
     * Materializes a \a request (query) made against this feature source, by running
     * it over the source and returning a new memory based vector layer containing
     * the result. All settings from feature \a request will be honored.
     *
     * If a subset of attributes has been set for the request, then only
     * those selected fields will be present in the output layer.
     *
     * The CRS for the output layer will match the input layer, unless
     * QgsFeatureRequest::setDestinationCrs() has been called with a valid QgsCoordinateReferenceSystem.
     * In this case the output layer will match the QgsFeatureRequest::destinationCrs() CRS.
     *
     * The returned layer WKB type will match wkbType(), unless the QgsFeatureRequest::NoGeometry flag is set
     * on the \a request. In that case the returned layer will not be a spatial layer.
     *
     * An optional \a feedback argument can be used to cancel the materialization
     * before it has fully completed.
     *
     * The returned value is a new instance and the caller takes responsibility
     * for its ownership.
     *
     */
    QgsVectorLayer *materialize( const QgsFeatureRequest &request,
                                 QgsFeedback *feedback = nullptr ) SIP_FACTORY;

    /**
     * Returns an enum value representing the presence of a valid spatial index on the source,
     * if it can be determined.
     *
     * If QgsFeatureSource::SpatialIndexUnknown is returned then the presence of an index cannot
     * be determined.
     *
     * \since QGIS 3.10.1
     */
    virtual Qgis::SpatialIndexPresence hasSpatialIndex() const;
};

Q_DECLARE_METATYPE( QgsFeatureSource * )

#endif // QGSFEATURESOURCE_H
