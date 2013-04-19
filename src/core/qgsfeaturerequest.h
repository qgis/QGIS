/***************************************************************************
    qgsfeaturerequest.h
    ---------------------
    begin                : Mai 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREREQUEST_H
#define QGSFEATUREREQUEST_H

#include <QFlags>

#include "qgsfeature.h"
#include "qgsrectangle.h"

#include <QList>
typedef QList<int> QgsAttributeList;

/**
 * This class wraps a request for features to a vector layer (or directly its vector data provider).
 * The request may apply a filter to fetch only a particular subset of features. Currently supported filters:
 * - no filter - all features are returned
 * - feature id - only feature that matches given feature id is returned
 * - rectangle - only features that intersect given rectangle should be fetched. For the sake of speed,
 *               the intersection is often done only using feature's bounding box. There is a flag
 *               ExactIntersect that makes sure that only intersecting features will be returned.
 *
 * For efficiency, it is also possible to tell provider that some data is not required:
 * - NoGeometry flag
 * - SubsetOfAttributes flag
 *
 * The options may be chained, e.g.:
 *   QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1)).setFlags(QgsFeatureRequest::ExactIntersect)
 *
 * Examples:
 * - fetch all features:
 *     QgsFeatureRequest()
 * - fetch all features, only one attribute
 *     QgsFeatureRequest().setSubsetOfAttributes(QStringList("myfield"), provider->fieldMap())
 * - fetch all features, without geometries
 *     QgsFeatureRequest().setFlags(QgsFeatureRequest::NoGeometry)
 * - fetch only features from particular extent
 *     QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1))
 * - fetch only one feature
 *     QgsFeatureRequest().setFilterFid(45)
 *
 */
class CORE_EXPORT QgsFeatureRequest
{
  public:
    enum Flag
    {
      NoFlags            = 0,
      NoGeometry         = 1,  //!< Do not fetch geometry
      SubsetOfAttributes = 2,  //!< Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
      ExactIntersect     = 4   //!< Use exact geometry intersection (slower) instead of bounding boxes
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    enum FilterType
    {
      FilterNone,   //!< No filter is applied
      FilterRect,   //!< Filter using a rectangle
      FilterFid     //!< Filter using feature ID
    };

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( QgsFeatureId fid );
    //! construct a request with rectangle filter
    explicit QgsFeatureRequest( const QgsRectangle& rect );

    QgsFeatureRequest( const QgsFeatureRequest& rh );

    FilterType filterType() const { return mFilter; }

    //! Set rectangle from which features will be taken. Empty rectangle removes the filter.
    //!
    QgsFeatureRequest& setFilterRect( const QgsRectangle& rect );
    const QgsRectangle& filterRect() const { return mFilterRect; }

    //! Set feature ID that should be fetched.
    QgsFeatureRequest& setFilterFid( QgsFeatureId fid );
    const QgsFeatureId& filterFid() const { return mFilterFid; }

    //! Set flags that affect how features will be fetched
    QgsFeatureRequest& setFlags( Flags flags );
    const Flags& flags() const { return mFlags; }

    //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
    //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
    QgsFeatureRequest& setSubsetOfAttributes( const QgsAttributeList& attrs );
    const QgsAttributeList& subsetOfAttributes() const { return mAttrs; }

    //! Set a subset of attributes by names that will be fetched
    QgsFeatureRequest& setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields );

    // TODO: in future
    // void setFilterExpression(const QString& expression); // using QgsExpression
    // void setFilterNativeExpression(con QString& expr);   // using provider's SQL (if supported)
    // void setLimit(int limit);

  protected:
    FilterType mFilter;
    QgsRectangle mFilterRect;
    QgsFeatureId mFilterFid;
    Flags mFlags;
    QgsAttributeList mAttrs;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRequest::Flags )


#endif // QGSFEATUREREQUEST_H
