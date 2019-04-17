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

#include "qgsfeature.h"
#include "qgsrectangle.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgssimplifymethod.h"



/**
 * \ingroup core
 * This class wraps a request for features to a vector layer (or directly its vector data provider).
 * The request may apply a filter to fetch only a particular subset of features. Currently supported filters:
 * - no filter - all features are returned
 * - feature id - only feature that matches given feature id is returned
 * - feature ids - only features that match any of the given feature ids are returned
 * - filter expression - only features that match the given filter expression are returned
 *
 * Additionally a spatial rectangle can be set in combination:
 * Only features that intersect given rectangle should be fetched. For the sake of speed,
 * the intersection is often done only using feature's bounding box. There is a flag
 * ExactIntersect that makes sure that only intersecting features will be returned.
 *
 * For efficiency, it is also possible to tell provider that some data is not required:
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
 *   # fetch only one feature
 *   QgsFeatureRequest().setFilterFid(45)
 * \endcode
 */
class CORE_EXPORT QgsFeatureRequest
{
  public:
    enum Flag
    {
      NoFlags            = 0,
      NoGeometry         = 1,  //!< Geometry is not required. It may still be returned if e.g. required for a filter condition.
      SubsetOfAttributes = 2,  //!< Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
      ExactIntersect     = 4   //!< Use exact geometry intersection (slower) instead of bounding boxes
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
     * The OrderByClause class represents an order by clause for a QgsFeatureRequest.
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

      private:
        QgsExpression mExpression;
        bool mAscending;
        bool mNullsFirst;
    };


    /**
     * \ingroup core
     * Represents a list of OrderByClauses, with the most important first and the least
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

    /**
     * Returns the filter type which is currently set on this request
     *
     * \returns Filter type
     */
    FilterType filterType() const { return mFilter; }

    /**
     * Sets the \a rectangle from which features will be taken. An empty rectangle removes the filter.
     *
     * When a destination CRS is set using setDestinationCrs(), \a rectangle
     * is expected to be in the same CRS as the destinationCrs(). Otherwise, \a rectangle
     * should use the same CRS as the source layer/provider.
     *
     * \see filterRect()
     */
    QgsFeatureRequest &setFilterRect( const QgsRectangle &rectangle );

    /**
     * Returns the rectangle from which features will be taken. If the returned
     * rectangle is null, then no filter rectangle is set.
     *
     * When a destination CRS is set using setDestinationCrs(), the rectangle
     * will be in the same CRS as the destinationCrs(). Otherwise, the rectangle
     * will use the same CRS as the source layer/provider.
     *
     * \see setFilterRect()
     */
    const QgsRectangle &filterRect() const { return mFilterRect; }

    //! Sets feature ID that should be fetched.
    QgsFeatureRequest &setFilterFid( QgsFeatureId fid );
    //! Gets the feature ID that should be fetched.
    QgsFeatureId filterFid() const { return mFilterFid; }

    //! Sets feature IDs that should be fetched.
    QgsFeatureRequest &setFilterFids( const QgsFeatureIds &fids );
    //! Gets feature IDs that should be fetched.
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
     * Set the filter expression. {\see QgsExpression}
     * \param expression expression string
     * \see filterExpression
     * \see setExpressionContext
     */
    QgsFeatureRequest &setFilterExpression( const QString &expression );

    /**
     * Returns the filter expression if set.
     * \see setFilterExpression
     * \see expressionContext
     */
    QgsExpression *filterExpression() const { return mFilterExpression.get(); }

    /**
     * Modifies the existing filter expression to add an additional expression filter. The
     * filter expressions are combined using AND, so only features matching both
     * the existing expression and the additional expression will be returned.
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
     * Disables filter conditions.
     * The spatial filter (filterRect) will be kept in place.
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
    QgsFeatureRequest &setLimit( long limit );

    /**
     * Returns the maximum number of features to request, or -1 if no limit set.
     * \see setLimit
     * \since QGIS 2.14
     */
    long limit() const { return mLimit; }

    //! Sets flags that affect how features will be fetched
    QgsFeatureRequest &setFlags( QgsFeatureRequest::Flags flags );
    const Flags &flags() const { return mFlags; }

    /**
     * Set a subset of attributes that will be fetched.
     *
     * An empty attributes list indicates that no attributes will be fetched.
     * To revert a call to setSubsetOfAttributes and fetch all available attributes,
     * the SubsetOfAttributes flag should be removed from the request.
     */
    QgsFeatureRequest &setSubsetOfAttributes( const QgsAttributeList &attrs );

    /**
     * Set that no attributes will be fetched.
     * To revert a call to setNoAttributes and fetch all or some available attributes,
     * the SubsetOfAttributes flag should be removed from the request.
     * \since QGIS 3.4
     */
    QgsFeatureRequest &setNoAttributes();

    /**
     * Returns the subset of attributes which at least need to be fetched
     * \returns A list of attributes to be fetched
     */
    QgsAttributeList subsetOfAttributes() const { return mAttrs; }

    //! Sets a subset of attributes by names that will be fetched
    QgsFeatureRequest &setSubsetOfAttributes( const QStringList &attrNames, const QgsFields &fields );

    //! Sets a subset of attributes by names that will be fetched
    QgsFeatureRequest &setSubsetOfAttributes( const QSet<QString> &attrNames, const QgsFields &fields );

    /**
     * Set a simplification method for geometries that will be fetched
     * \since QGIS 2.2
     */
    QgsFeatureRequest &setSimplifyMethod( const QgsSimplifyMethod &simplifyMethod );

    /**
     * Gets simplification method for geometries that will be fetched
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
     * set on the request is expected to be in the same CRS as the destination
     * CRS.
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
     * \since QGIS 3.4
     */
    int timeout() const;

    /**
     * Sets the \a timeout (in milliseconds) for the maximum time we should wait during feature requests before a
     * feature is returned. A negative value (which is set by default) will wait forever.
     *
     * \note Only works if the provider supports this option.
     *
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
     * \since QGIS 3.4
     */
    QgsFeatureRequest &setRequestMayBeNested( bool requestMayBeNested );

  protected:
    FilterType mFilter = FilterNone;
    QgsRectangle mFilterRect;
    QgsFeatureId mFilterFid = -1;
    QgsFeatureIds mFilterFids;
    std::unique_ptr< QgsExpression > mFilterExpression;
    QgsExpressionContext mExpressionContext;
    Flags mFlags;
    QgsAttributeList mAttrs;
    QgsSimplifyMethod mSimplifyMethod;
    long mLimit = -1;
    OrderBy mOrderBy;
    InvalidGeometryCheck mInvalidGeometryFilter = GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    std::function< void( const QgsFeature & ) > mTransformErrorCallback;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
    int mTimeout = -1;
    int mRequestMayBeNested = false;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRequest::Flags )


class QgsFeatureIterator;
class QgsAbstractFeatureIterator;

/**
 * \ingroup core
 * Base class that can be used for any class that is capable of returning features
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsAbstractFeatureSource
{
  public:
    virtual ~QgsAbstractFeatureSource();

    /**
     * Gets an iterator for features matching the specified request
     * \param request The request
     * \returns A feature iterator
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) = 0;

  protected:
    void iteratorOpened( QgsAbstractFeatureIterator *it );
    void iteratorClosed( QgsAbstractFeatureIterator *it );

    QSet< QgsAbstractFeatureIterator * > mActiveIterators;

    template<typename> friend class QgsAbstractFeatureIteratorFromSource;
};

#endif // QGSFEATUREREQUEST_H
