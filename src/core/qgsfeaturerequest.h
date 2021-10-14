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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QFlags>
#include <QList>
#include <memory>

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsrectangle.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgssimplifymethod.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatereferencesystem.h"


/**
 * \ingroup core
 * \brief This class wraps a request for features to a vector layer (or directly its vector data provider).
 *
 * The request may apply an attribute/ID filter to fetch only a particular subset of features. Currently supported filters:
 *
 * - no filter - all features are returned
 * - feature id - only feature that matches given feature id is returned
 * - feature ids - only features that match any of the given feature ids are returned
 * - filter expression - only features that match the given filter expression are returned
 *
 * Additionally a spatial filter can be set in combination with the attribute/ID filter. Supported
 * spatial filters are:
 *
 * - Qgis::SpatialFilterType::BoundingBox: Only features that intersect a given rectangle will be fetched. For the sake of speed, the intersection is often done only using feature's bounding box. There is a flag ExactIntersect that makes sure that only intersecting features will be returned.
 * - Qgis::SpatialFilterType::DistanceWithin: Only features within a specified distance of a reference geometry will be fetched.
 *
 * For efficiency, it is also possible to tell provider that some data is not required:
 *
 * - NoGeometry flag
 * - SubsetOfAttributes flag
 * - SimplifyMethod for geometries to fetch
 *
 * The options may be chained, e.g.:
 *
 * \code{.py}
 *   QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1)).setFlags(QgsFeatureRequest.ExactIntersect)
 * \endcode
 *
 * Examples:
 *
 * \code{.py}
 *   # fetch all features:
 *   QgsFeatureRequest()
 *   # fetch all features, only one attribute
 *   QgsFeatureRequest().setSubsetOfAttributes(['myfield'], layer.fields())
 *   # fetch all features, without geometries
 *   QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
 *   # fetch only features from particular extent
 *   QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1))
 *   # fetch only features from particular extent, where the 'type' attribute is equal to 'road':
 *   QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1)).setFilterExpression('"type"=\'road\'')
 *   # fetch only one feature
 *   QgsFeatureRequest().setFilterFid(45)
 *   # fetch features within 50 map units of a linestring geometry
 *   QgsFeatureRequest().setDistanceWithin(QgsGeometry.fromWkt('LineString(0 0, 10 0, 12 1)'), 50)
 * \endcode
 *
 */
class CORE_EXPORT QgsFeatureRequest
{
  public:
    enum Flag
    {
      NoFlags            = 0,
      NoGeometry         = 1,  //!< Geometry is not required. It may still be returned if e.g. required for a filter condition.
      SubsetOfAttributes = 2,  //!< Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
      ExactIntersect     = 4,   //!< Use exact geometry intersection (slower) instead of bounding boxes
      IgnoreStaticNodesDuringExpressionCompilation = 8, //!< If a feature request uses a filter expression which can be partially precalculated due to static nodes in the expression, setting this flag will prevent these precalculated values from being utilized during compilation of the filter for the backend provider. This flag significantly slows down feature requests and should be used for debugging purposes only. (Since QGIS 3.18)
      EmbeddedSymbols    = 16,  //!< Retrieve any embedded feature symbology (since QGIS 3.20)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Types of filters.
     */
    enum FilterType
    {
      FilterNone,       //!< No filter is applied
      FilterFid,        //!< Filter using feature ID
      FilterExpression, //!< Filter using expression
      FilterFids        //!< Filter using feature IDs
    };

    //! Handling of features with invalid geometries
    enum InvalidGeometryCheck
    {
      GeometryNoCheck = 0, //!< No invalid geometry checking
      GeometrySkipInvalid = 1, //!< Skip any features with invalid geometry. This requires a slow geometry validity check for every feature.
      GeometryAbortOnInvalid = 2, //!< Close iterator on encountering any features with invalid geometry. This requires a slow geometry validity check for every feature.
    };

    /**
     * \ingroup core
     * \brief The OrderByClause class represents an order by clause for a QgsFeatureRequest.
     *
     * It can be a simple field or an expression. Multiple order by clauses can be added to
     * a QgsFeatureRequest to fine tune the behavior if a single field or expression is not
     * enough to completely specify the required behavior.
     *
     * If expression compilation is activated in the settings and the expression can be
     * translated for the provider in question, it will be evaluated on provider side.
     * If one of these two premises does not apply, the ordering will take place locally
     * which results in increased memory and CPU usage.
     *
     * If the ordering is done on strings, the order depends on the system's locale if the
     * local fallback implementation is used. The order depends on the server system's locale
     * and implementation if ordering is done on the server.
     *
     * In case the fallback code needs to be used, a limit set on the request will be respected
     * for the features returned by the iterator but internally all features will be requested
     * from the provider.
     *
     * \since QGIS 2.14
     */
    class CORE_EXPORT OrderByClause
    {
      public:

        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * \param expression The expression to use for ordering
         * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         *                   If the order is ascending, by default nulls are last
         *                   If the order is descending, by default nulls are first
         */
        OrderByClause( const QString &expression, bool ascending = true );

        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * \param expression The expression to use for ordering
         * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         * \param nullsfirst If TRUE, NULLS are at the beginning, if FALSE, NULLS are at the end
         */
        OrderByClause( const QString &expression, bool ascending, bool nullsfirst );

        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * \param expression The expression to use for ordering
         * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         *                   If the order is ascending, by default nulls are last
         *                   If the order is descending, by default nulls are first
         */
        OrderByClause( const QgsExpression &expression, bool ascending = true );

        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * \param expression The expression to use for ordering
         * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         * \param nullsfirst If TRUE, NULLS are at the beginning, if FALSE, NULLS are at the end
         */
        OrderByClause( const QgsExpression &expression, bool ascending, bool nullsfirst );

        /**
         * The expression
         * \returns the expression
         */
        QgsExpression expression() const;

        /**
         * Prepare the expression with the given context.
         *
         * \see QgsExpression::prepare
         *
         * \since QGIS 3.0
         */
        bool prepare( QgsExpressionContext *context );

        /**
         * Order ascending
         * \returns If ascending order is requested
         */
        bool ascending() const;

        /**
         * Set if ascending order is requested
         */
        void setAscending( bool ascending );

        /**
         * Set if NULLS should be returned first
         * \returns if NULLS should be returned first
         */
        bool nullsFirst() const;

        /**
         * Set if NULLS should be returned first
         */
        void setNullsFirst( bool nullsFirst );

        /**
         * Dumps the content to an SQL equivalent
         */
        QString dump() const;

        // friend inline int qHash(const OrderByClause &a) { return qHash(a.mExpression.expression()) ^ qHash(a.mAscending) ^ qHash( a.mNullsFirst); }

        bool operator==( const OrderByClause &v ) const
        {
          return mExpression == v.mExpression &&
                 mAscending == v.mAscending &&
                 mNullsFirst == v.mNullsFirst;
        }

        bool operator!=( const OrderByClause &v ) const
        {
          return !( v == *this );
        }

      private:
        QgsExpression mExpression;
        bool mAscending;
        bool mNullsFirst;
    };


    /**
     * \ingroup core
     * \brief Represents a list of OrderByClauses, with the most important first and the least
     * important last.
     *
     * \since QGIS 2.14
     */
    class OrderBy : public QList<QgsFeatureRequest::OrderByClause>
    {
      public:

        /**
         * Create a new empty order by
         */
        CORE_EXPORT OrderBy();

        /**
         * Create a new order by from a list of clauses
         */
        CORE_EXPORT OrderBy( const QList<QgsFeatureRequest::OrderByClause> &other );

        /**
         * Gets a copy as a list of OrderByClauses
         *
         * This is only required in Python where the inheritance
         * is not properly propagated and this makes it usable.
         */
        QList<QgsFeatureRequest::OrderByClause> CORE_EXPORT list() const;

        /**
         * Serialize to XML
         */
        void CORE_EXPORT save( QDomElement &elem ) const;

        /**
         * Deserialize from XML
         */
        void CORE_EXPORT load( const QDomElement &elem );

        /**
         * Returns a set of used attributes
         * \note The returned attributes names are NOT guaranteed to be valid.
         */
        QSet<QString> CORE_EXPORT usedAttributes() const;

        /**
         * Returns a set of used, validated attribute indices
         * \since QGIS 3.8
         */
        QSet<int> CORE_EXPORT usedAttributeIndices( const QgsFields &fields ) const;

        /**
         * Dumps the content to an SQL equivalent syntax
         */
        QString CORE_EXPORT dump() const;
    };

    /**
     * A special attribute that if set matches all attributes
     */
    static const QString ALL_ATTRIBUTES;

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( QgsFeatureId fid );
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( const QgsFeatureIds &fids );

    /**
     * Construct a request with \a rectangle bounding box filter.
     *
     * When a destination CRS is set using setDestinationCrs(), \a rectangle
     * is expected to be in the same CRS as the destinationCrs(). Otherwise, \a rectangle
     * should use the same CRS as the source layer/provider.
     */
    explicit QgsFeatureRequest( const QgsRectangle &rectangle );

    //! construct a request with a filter expression
    explicit QgsFeatureRequest( const QgsExpression &expr, const QgsExpressionContext &context = QgsExpressionContext() );
    //! copy constructor
    QgsFeatureRequest( const QgsFeatureRequest &rh );
    //! Assignment operator
    QgsFeatureRequest &operator=( const QgsFeatureRequest &rh );

    ~QgsFeatureRequest();

    /**
     * Returns the attribute/ID filter type which is currently set on this request.
     *
     * This type will automatically be set to the appropriate value whenever setFilterFid(),
     * setFilterFids(), setFilterExpression() or disableFilter() are called.
     *
     * \note A feature request may have both an attribute/ID filter AND a spatial filter
     * set. See spatialFilterType() to retrieve the spatial filter.
     *
     * \see spatialFilterType()
     */
    FilterType filterType() const { return mFilter; }

    /**
     * Returns the spatial filter type which is currently set on this request.
     *
     * This type will automatically be set to the appropriate value whenever setFilterRect(),
     * or setDistanceWithin() are called.
     *
     * \note A feature request may have both an attribute/ID filter AND a spatial filter
     * set. See filterType() to retrieve the attribute/ID filter.
     *
     * \see filterType()
     * \since QGIS 3.22
     */
    Qgis::SpatialFilterType spatialFilterType() const { return mSpatialFilter; }

    /**
     * Sets the \a rectangle from which features will be taken. An empty rectangle removes the filter.
     *
     * When a destination CRS is set using setDestinationCrs(), \a rectangle
     * is expected to be in the same CRS as the destinationCrs(). Otherwise, \a rectangle
     * should use the same CRS as the source layer/provider.
     *
     * Calling this method will automatically set spatialFilterType() to Qgis::SpatialFilterType::BoundingBox.
     * If \a rectangle is a null rectangle then spatialFilterType() will be reset to Qgis::SpatialFilterType::NoFilter.
     *
     * \see filterRect()
     */
    QgsFeatureRequest &setFilterRect( const QgsRectangle &rectangle );

    /**
     * Returns the rectangle from which features will be taken. If the returned
     * rectangle is null, then no filter rectangle is set.
     *
     * If spatialFilterType() is Qgis::SpatialFilterType::BoundingBox then only
     * features from within this bounding box will be fetched. If spatialFilterType()
     * is Qgis::SpatialFilterType::DistanceWithin then the returned rectangle
     * represents the bounding box of the referenceGeometry() extended by distanceWithin().
     *
     * When a destination CRS is set using setDestinationCrs(), the rectangle
     * will be in the same CRS as the destinationCrs(). Otherwise, the rectangle
     * will use the same CRS as the source layer/provider.
     *
     * \see setFilterRect()
     */
    QgsRectangle filterRect() const;

    /**
     * Sets a reference \a geometry and a maximum \a distance from this geometry to retrieve
     * features within.
     *
     * When a destination CRS is set using setDestinationCrs(), \a geometry
     * is expected to be in the same CRS as the destinationCrs() and \a distance is in
     * the spatial units of the destinationCrs(). Otherwise, \a geometry
     * should use the same CRS as the source layer/provider and \a distance
     * should use the spatial units as this same CRS.
     *
     * Calling this method will automatically set spatialFilterType() to Qgis::SpatialFilterType::DistanceWithin.
     *
     * \see filterRect()
     * \since QGIS 3.22
     */
    QgsFeatureRequest &setDistanceWithin( const QgsGeometry &geometry, double distance );

    /**
     * Returns the reference geometry used for spatial filtering of features.
     *
     * When spatialFilterType() is Qgis::SpatialFilterType::DistanceWithin then only
     * features within distanceWithin() units of the reference geometry will be
     * fetched.
     *
     * When a destination CRS is set using setDestinationCrs(), the geometry
     * will be in the same CRS as the destinationCrs(). Otherwise, the geometry
     * will use the same CRS as the source layer/provider.
     *
     * \see setDistanceWithin()
     * \since QGIS 3.22
     */
    QgsGeometry referenceGeometry() const { return mReferenceGeometry; }

    /**
     * Returns the reference geometry engine used for spatial filtering of features.
     *
     * This is to avoid re-creating the engine.
     *
     * \see referenceGeometry()
     * \since QGIS 3.22
     */
    std::shared_ptr< QgsGeometryEngine > referenceGeometryEngine() const SIP_SKIP { return mReferenceGeometryEngine; }

    /**
     * Returns the maximum distance from the referenceGeometry() of fetched
     * features, if spatialFilterType() is Qgis::SpatialFilterType::DistanceWithin.
     *
     * When a destination CRS is set using setDestinationCrs(), the distance
     * will be in the spatial units of destinationCrs(). Otherwise, the distance
     * will use the same units as the CRS of the source layer/provider.
     *
     * \see setDistanceWithin()
     * \since QGIS 3.22
     */
    double distanceWithin() const { return mDistanceWithin; }

    /**
     * Sets the feature ID that should be fetched.
     *
     * Calling this method will automatically set filterType() to QgsFeatureRequest::FilterFid.
     *
     * \see filterFid()
     * \see setFilterFids()
     */
    QgsFeatureRequest &setFilterFid( QgsFeatureId fid );

    /**
     * Returns the feature ID that should be fetched.
     *
     * \see setFilterFid()
     * \see filterFids()
     */
    QgsFeatureId filterFid() const { return mFilterFid; }

    /**
     * Sets the feature IDs that should be fetched.
     *
     * Calling this method will automatically set filterType() to QgsFeatureRequest::FilterFids.
     *
     * \see filterFids()
     * \see setFilterFid()
     */
    QgsFeatureRequest &setFilterFids( const QgsFeatureIds &fids );

    /**
     * Returns the feature IDs that should be fetched.
     *
     * \see setFilterFids()
     * \see filterFid()
     */
    const QgsFeatureIds &filterFids() const { return mFilterFids; }

    /**
     * Sets invalid geometry checking behavior.
     * \note Invalid geometry checking is not performed when retrieving features
     * directly from a QgsVectorDataProvider.
     * \see invalidGeometryCheck()
     * \since QGIS 3.0
     */
    QgsFeatureRequest &setInvalidGeometryCheck( InvalidGeometryCheck check );

    /**
     * Returns the invalid geometry checking behavior.
     * \see setInvalidGeometryCheck()
     * \since QGIS 3.0
     */
    InvalidGeometryCheck invalidGeometryCheck() const { return mInvalidGeometryFilter; }

    /**
     * Sets a callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid or GeometrySkipInvalid. This function will be
     * called using the feature with invalid geometry as a parameter.
     * \see invalidGeometryCallback()
     * \since QGIS 3.0
     */
#ifndef SIP_RUN
    QgsFeatureRequest &setInvalidGeometryCallback( const std::function< void( const QgsFeature & )> &callback );
#else
    QgsFeatureRequest &setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setInvalidGeometryCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    sipRes = sipCpp;

    Py_END_ALLOW_THREADS
    % End
#endif

    /**
     * Returns the callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid or GeometrySkipInvalid.
     * \note not available in Python bindings
     * \see setInvalidGeometryCallback()
     * \since QGIS 3.0
     */
    std::function< void( const QgsFeature & ) > invalidGeometryCallback() const { return mInvalidGeometryCallback; } SIP_SKIP

    /**
     * Set the filter \a expression. {\see QgsExpression}
     * \param expression expression string
     *
     * Calling this method will automatically set filterType() to QgsFeatureRequest::FilterExpression.
     *
     * \see filterExpression()
     * \see setExpressionContext()
     */
    QgsFeatureRequest &setFilterExpression( const QString &expression );

    /**
     * Returns the filter expression (if set).
     * \see setFilterExpression()
     * \see expressionContext()
     */
    QgsExpression *filterExpression() const { return mFilterExpression.get(); }

    /**
     * Modifies the existing filter expression to add an additional expression filter. The
     * filter expressions are combined using AND, so only features matching both
     * the existing expression and the additional expression will be returned.
     *
     * Calling this method will automatically set filterType() to QgsFeatureRequest::FilterExpression.
     *
     * \since QGIS 2.14
     */
    QgsFeatureRequest &combineFilterExpression( const QString &expression );

    /**
     * Returns the expression context used to evaluate filter expressions.
     * \see setExpressionContext
     * \see filterExpression
     * \since QGIS 2.12
     */
    QgsExpressionContext *expressionContext() { return &mExpressionContext; }

    /**
     * Sets the expression context used to evaluate filter expressions.
     * \see expressionContext
     * \see setFilterExpression
     * \since QGIS 2.12
     */
    QgsFeatureRequest &setExpressionContext( const QgsExpressionContext &context );

    /**
     * Disables any attribute/ID filtering.
     *
     * Calling this method will automatically set filterType() to QgsFeatureRequest::FilterNone.
     *
     * \note Spatial filters will be left in place.
     *
     * \returns The object the method is called on for chaining
     *
     * \since QGIS 2.12
     */
    QgsFeatureRequest &disableFilter() { mFilter = FilterNone; mFilterExpression.reset(); return *this; }

    /**
     * Adds a new OrderByClause, appending it as the least important one.
     *
     * \param expression The expression to use for ordering
     * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
     *                   If the order is ascending, by default nulls are last
     *                   If the order is descending, by default nulls are first
     *
     * \since QGIS 2.14
     */

    QgsFeatureRequest &addOrderBy( const QString &expression, bool ascending = true );

    /**
     * Adds a new OrderByClause, appending it as the least important one.
     *
     * \param expression The expression to use for ordering
     * \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
     * \param nullsfirst If TRUE, NULLS are at the beginning, if FALSE, NULLS are at the end
     *
     * \since QGIS 2.14
     */
    QgsFeatureRequest &addOrderBy( const QString &expression, bool ascending, bool nullsfirst );

    /**
     * Returns a list of order by clauses specified for this feature request.
     *
     * \since QGIS 2.14
     */
    OrderBy orderBy() const;

    /**
     * Set a list of order by clauses.
     *
     * \since QGIS 2.14
     */
    QgsFeatureRequest &setOrderBy( const OrderBy &orderBy );

    /**
     * Set the maximum number of features to request.
     * \param limit maximum number of features, or -1 to request all features.
     * \see limit()
     * \since QGIS 2.14
     */
    QgsFeatureRequest &setLimit( long long limit );

    /**
     * Returns the maximum number of features to request, or -1 if no limit set.
     * \see setLimit
     * \since QGIS 2.14
     */
#ifndef SIP_RUN
    long long limit() const { return mLimit; }
#else
    long long limit() const;
#endif

    /**
     * Sets \a flags that affect how features will be fetched.
     *
     * \see flags()
     */
    QgsFeatureRequest &setFlags( QgsFeatureRequest::Flags flags );

    /**
     * Returns the flags which affect how features are fetched.
     *
     * \see setFlags()
     */
    Flags flags() const { return mFlags; }

    /**
     * Set a subset of attributes that will be fetched.
     *
     * An empty attributes list indicates that no attributes will be fetched.
     * To revert a call to setSubsetOfAttributes and fetch all available attributes,
     * the SubsetOfAttributes flag should be removed from the request.
     *
     * \note This is intended as hint to data providers for optimising feature retrieval. Depending
     * on the provider, it may be trivial for the provider to always return all attributes instead of
     * the requested subset, or actually result in slower retrieval when the attributes are filtered out.
     * In these cases the provider may ignore this hint and return all attributes regardless of the
     * requested attributes.
     *
     * \see subsetOfAttributes()
     * \see setNoAttributes()
     */
    QgsFeatureRequest &setSubsetOfAttributes( const QgsAttributeList &attrs );

    /**
     * Set that no attributes will be fetched.
     *
     * To revert a call to setNoAttributes and fetch all or some available attributes,
     * the SubsetOfAttributes flag should be removed from the request.
     *
     * \note This is intended as hint to data providers for optimising feature retrieval. Depending
     * on the provider, it may be trivial for the provider to always return all attributes instead of
     * removing them. In these cases the provider may ignore this hint and return all attributes
     * regardless of whether this method has been called.
     *
     * \see setSubsetOfAttributes()
     *
     * \since QGIS 3.4
     */
    QgsFeatureRequest &setNoAttributes();

    /**
     * Returns the subset of attributes which at least need to be fetched.
     * \returns A list of attributes to be fetched
     *
     * \note This is intended as hint to data providers for optimising feature retrieval. Depending
     * on the provider, it may be trivial for the provider to always return all attributes instead of
     * the requested subset, or actually result in slower retrieval when the attributes are filtered out.
     * In these cases the provider may ignore this hint and return all attributes regardless of the
     * requested attributes.
     *
     * \see setSubsetOfAttributes()
     * \see setNoAttributes()
     */
    QgsAttributeList subsetOfAttributes() const { return mAttrs; }

    /**
     * Sets a subset of attributes by names that will be fetched.
     *
     * \note This is intended as hint to data providers for optimising feature retrieval. Depending
     * on the provider, it may be trivial for the provider to always return all attributes instead of
     * the requested subset, or actually result in slower retrieval when the attributes are filtered out.
     * In these cases the provider may ignore this hint and return all attributes regardless of the
     * requested attributes.
     *
     * \see subsetOfAttributes()
     */
    QgsFeatureRequest &setSubsetOfAttributes( const QStringList &attrNames, const QgsFields &fields );

    /**
     * Sets a subset of attributes by names that will be fetched.
     *
     * \note This is intended as hint to data providers for optimising feature retrieval. Depending
     * on the provider, it may be trivial for the provider to always return all attributes instead of
     * the requested subset, or actually result in slower retrieval when the attributes are filtered out.
     * In these cases the provider may ignore this hint and return all attributes regardless of the
     * requested attributes.
     *
     * \see subsetOfAttributes()
     */
    QgsFeatureRequest &setSubsetOfAttributes( const QSet<QString> &attrNames, const QgsFields &fields );

    /**
     * Set a simplification method for geometries that will be fetched.
     *
     * \see simplifyMethod()
     * \since QGIS 2.2
     */
    QgsFeatureRequest &setSimplifyMethod( const QgsSimplifyMethod &simplifyMethod );

    /**
     * Returns the simplification method for geometries that will be fetched.
     *
     * \see setSimplifyMethod()
     * \since QGIS 2.2
     */
    const QgsSimplifyMethod &simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Returns the destination coordinate reference system for feature's geometries,
     * or an invalid QgsCoordinateReferenceSystem if no reprojection will be done
     * and all features will be left with their original geometry.
     * \see setDestinationCrs()
     * \see transformContext()
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Returns the transform context, for use when a destinationCrs() has been set
     * and reprojection is required
     * \see setDestinationCrs()
     * \see destinationCrs()
     * \since QGIS 3.0
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the destination \a crs for feature's geometries. If set, all
     * geometries will be reprojected from their original coordinate reference
     * system to this desired reference system. If \a crs is an invalid
     * QgsCoordinateReferenceSystem then no reprojection will be done
     * and all features will be left with their original geometry.
     *
     * When a \a crs is set using setDestinationCrs(), then any filterRect()
     * or referenceGeometry() set on the request is expected to be in the
     * same CRS as the destination CRS.
     *
     * The feature geometry transformation to the destination CRS is performed
     * after all filter expressions are tested and any virtual fields are
     * calculated. Accordingly, any geometric expressions used in
     * filterExpression() will be performed in the original
     * source CRS. This ensures consistent results are returned regardless of the
     * destination CRS. Similarly, virtual field values will be calculated using the
     * original geometry in the source CRS, so these values are not affected by
     * any destination CRS transform present in the feature request.
     *
     * \see destinationCrs()
     * \since QGIS 3.0
     */
    QgsFeatureRequest &setDestinationCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    /**
     * Sets a callback function to use when encountering a transform error when iterating
     * features and a destinationCrs() is set. This function will be
     * called using the feature which encountered the transform error as a parameter.
     * \see transformErrorCallback()
     * \see setDestinationCrs()
     * \since QGIS 3.0
     */
#ifndef SIP_RUN
    QgsFeatureRequest &setTransformErrorCallback( const std::function< void( const QgsFeature & )> &callback );
#else
    QgsFeatureRequest &setTransformErrorCallback( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setTransformErrorCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    sipRes = sipCpp;

    Py_END_ALLOW_THREADS
    % End
#endif

    /**
     * Returns the callback function to use when encountering a transform error when iterating
     * features and a destinationCrs() is set.
     * \note not available in Python bindings
     * \see setTransformErrorCallback()
     * \see destinationCrs()
     * \since QGIS 3.0
     */
    std::function< void( const QgsFeature & ) > transformErrorCallback() const { return mTransformErrorCallback; } SIP_SKIP


    /**
     * Check if a feature is accepted by this requests filter
     *
     * \param feature  The feature which will be tested
     *
     * \returns TRUE, if the filter accepts the feature
     *
     * \since QGIS 2.1
     */
    bool acceptFeature( const QgsFeature &feature );

    /**
     * Returns the timeout (in milliseconds) for how long we should wait for a connection if none is available from the pool
     * at this moment. A negative value (which is set by default) will wait forever.
     *
     * \note Only works if the provider supports this option.
     *
     * \deprecated Use timeout() instead.
     * \since QGIS 3.0
     */
    Q_DECL_DEPRECATED int connectionTimeout() const SIP_DEPRECATED;

    /**
     * Sets the timeout (in milliseconds) for how long we should wait for a connection if none is available from the pool
     * at this moment. A negative value (which is set by default) will wait forever.
     *
     * \note Only works if the provider supports this option.
     *
     * \deprecated Use setTimeout() instead.
     * \since QGIS 3.0
     */
    Q_DECL_DEPRECATED QgsFeatureRequest &setConnectionTimeout( int connectionTimeout ) SIP_DEPRECATED;

    /**
     * Returns the timeout (in milliseconds) for the maximum time we should wait during feature requests before a
     * feature is returned. A negative value (which is set by default) will wait forever.
     *
     * \note Only works if the provider supports this option.
     *
     * \see setTimeout()
     * \since QGIS 3.4
     */
    int timeout() const;

    /**
     * Sets the \a timeout (in milliseconds) for the maximum time we should wait during feature requests before a
     * feature is returned. A negative value (which is set by default) will wait forever.
     *
     * \note Only works if the provider supports this option.
     *
     * \see timeout()
     * \since QGIS 3.4
     */
    QgsFeatureRequest &setTimeout( int timeout );

    /**
     * In case this request may be run nested within another already running
     * iteration on the same connection, set this to TRUE.
     *
     * If this flag is TRUE, this request will be able to make use of "spare"
     * connections to avoid deadlocks.
     *
     * For example, this should be set on requests that are issued from an
     * expression function.
     *
     * \see setRequestMayBeNested()
     * \since QGIS 3.4
     */
    bool requestMayBeNested() const;

    /**
     * In case this request may be run nested within another already running
     * iteration on the same connection, set this to TRUE.
     *
     * If this flag is TRUE, this request will be able to make use of "spare"
     * connections to avoid deadlocks.
     *
     * For example, this should be set on requests that are issued from an
     * expression function.
     *
     * \see requestMayBeNested()
     * \since QGIS 3.4
     */
    QgsFeatureRequest &setRequestMayBeNested( bool requestMayBeNested );

    /**
     * Attach a \a feedback object that can be queried regularly by the iterator to check
     * if it should be canceled.
     *
     * Ownership of \a feedback is NOT transferred, and the caller must take care that it exists
     * for the lifetime of the feature request and feature iterators.
     *
     * \see feedback()
     *
     * \since QGIS 3.20
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the feedback object that can be queried regularly by the iterator to check
     * if it should be canceled, if set.
     *
     * \see setFeedback()
     *
     * \since QGIS 3.20
     */
    QgsFeedback *feedback() const;

  protected:

    /**
     * Attribute/ID filter type.
     */
    FilterType mFilter = FilterNone;

    /**
     * Spatial filter type.
     *
     * \since QGIS 3.22
     */
    Qgis::SpatialFilterType mSpatialFilter = Qgis::SpatialFilterType::NoFilter;

    /**
     * Bounding box for spatial filtering.
     */
    QgsRectangle mFilterRect;

    /**
     * Reference geometry for Qgis::RequestSpatialFilter::DistanceWithin filtering.
     */
    QgsGeometry mReferenceGeometry;

    /**
     * Prepared geometry engine for mReferenceGeometry.
     */
    std::shared_ptr< QgsGeometryEngine > mReferenceGeometryEngine;

    /**
     * Maximum distance from reference geometry.
     */
    double mDistanceWithin = 0;

    QgsFeatureId mFilterFid = -1;
    QgsFeatureIds mFilterFids;
    std::unique_ptr< QgsExpression > mFilterExpression;
    QgsExpressionContext mExpressionContext;
    Flags mFlags = Flags();
    QgsAttributeList mAttrs;
    QgsSimplifyMethod mSimplifyMethod;
    long long mLimit = -1;
    OrderBy mOrderBy;
    InvalidGeometryCheck mInvalidGeometryFilter = GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    std::function< void( const QgsFeature & ) > mTransformErrorCallback;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
    int mTimeout = -1;
    int mRequestMayBeNested = false;
    QgsFeedback *mFeedback = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRequest::Flags )


class QgsFeatureIterator;
class QgsAbstractFeatureIterator;

/**
 * \ingroup core
 * \brief Base class that can be used for any class that is capable of returning features
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsAbstractFeatureSource
{
  public:
    virtual ~QgsAbstractFeatureSource();


    // IMPORTANT -- do NOT remove the /TransferBack/ annotation here -- while it looks completely wrong, it's
    // required for Python data providers to work correctly! Argh!

    /**
     * Gets an iterator for features matching the specified request
     * \param request The request
     * \returns A feature iterator
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) = 0 SIP_TRANSFERBACK;

    // IMPORTANT -- do NOT remove the /TransferBack/ annotation here -- while it looks completely wrong, it's
    // required for Python data providers to work correctly! Argh!

  protected:
    void iteratorOpened( QgsAbstractFeatureIterator *it );
    void iteratorClosed( QgsAbstractFeatureIterator *it );

    QSet< QgsAbstractFeatureIterator * > mActiveIterators;

    template<typename> friend class QgsAbstractFeatureIteratorFromSource;
};

#endif // QGSFEATUREREQUEST_H
