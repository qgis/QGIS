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
#include <QList>

#include "qgsfeature.h"
#include "qgsrectangle.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgssimplifymethod.h"

typedef QList<int> QgsAttributeList;

/** \ingroup core
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
      FilterRect,       //!< Obsolete, will be ignored. If a filterRect is set it will be used anyway. Filter using a rectangle, no need to set NoGeometry. Instead check for request.filterRect().isNull()
      FilterFid,        //!< Filter using feature ID
      FilterExpression, //!< Filter using expression
      FilterFids        //!< Filter using feature IDs
    };

    /** \ingroup core
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
     * @note added in QGIS 2.14
     */
    class CORE_EXPORT OrderByClause
    {
      public:
        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * @param expression The expression to use for ordering
         * @param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         *                   If the order is ascending, by default nulls are last
         *                   If the order is descending, by default nulls are first
         */
        OrderByClause( const QString &expression, bool ascending = true );
        /**
         * Creates a new OrderByClause for a QgsFeatureRequest
         *
         * @param expression The expression to use for ordering
         * @param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
         * @param nullsfirst If true, NULLS are at the beginning, if false, NULLS are at the end
         */
        OrderByClause( const QString &expression, bool ascending, bool nullsfirst );

        /**
         * The expression
         * @return the expression
         */
        QgsExpression expression() const;

        /**
         * Order ascending
         * @return If ascending order is requested
         */
        bool ascending() const;

        /**
         * Set if ascending order is requested
         */
        void setAscending( bool ascending );

        /**
         * Set if NULLS should be returned first
         * @return if NULLS should be returned first
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

    /** \ingroup core
     * Represents a list of OrderByClauses, with the most important first and the least
     * important last.
     *
     * @note added in QGIS 2.14
     */
    class OrderBy : public QList<OrderByClause>
    {
      public:
        /**
         * Create a new empty order by
         */
        CORE_EXPORT OrderBy()
            : QList<OrderByClause>()
        {}

        /**
         * Create a new order by from a list of clauses
         */
        CORE_EXPORT OrderBy( const QList<OrderByClause>& other );

        /**
         * Get a copy as a list of OrderByClauses
         *
         * This is only required in python where the inheritance
         * is not properly propagated and this makes it usable.
         */
        QList<OrderByClause> CORE_EXPORT list() const;

        /**
         * Serialize to XML
         */
        void CORE_EXPORT save( QDomElement& elem ) const;

        /**
         * Deserialize from XML
         */
        void CORE_EXPORT load( const QDomElement& elem );

        /**
         * Returns a set of used attributes
         */
        QSet<QString> CORE_EXPORT usedAttributes() const;

        /**
         * Dumps the content to an SQL equivalent syntax
         */
        QString CORE_EXPORT dump() const;
    };

    /**
     * A special attribute that if set matches all attributes
     */
    static const QString AllAttributes;

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( QgsFeatureId fid );
    //! construct a request with rectangle filter
    explicit QgsFeatureRequest( const QgsRectangle& rect );
    //! construct a request with a filter expression
    explicit QgsFeatureRequest( const QgsExpression& expr, const QgsExpressionContext& context = QgsExpressionContext() );
    //! copy constructor
    QgsFeatureRequest( const QgsFeatureRequest& rh );
    //! Assignment operator
    QgsFeatureRequest& operator=( const QgsFeatureRequest& rh );

    ~QgsFeatureRequest();

    /**
     * Return the filter type which is currently set on this request
     *
     * @return Filter type
     */
    FilterType filterType() const { if ( mFilter == FilterNone && !mFilterRect.isNull() ) return FilterRect; else return mFilter; }

    /**
     * Set rectangle from which features will be taken. Empty rectangle removes the filter.
     */
    QgsFeatureRequest& setFilterRect( const QgsRectangle& rect );

    /**
     * Get the rectangle from which features will be taken.
     */
    const QgsRectangle& filterRect() const { return mFilterRect; }

    //! Set feature ID that should be fetched.
    QgsFeatureRequest& setFilterFid( QgsFeatureId fid );
    //! Get the feature ID that should be fetched.
    QgsFeatureId filterFid() const { return mFilterFid; }

    //! Set feature IDs that should be fetched.
    QgsFeatureRequest& setFilterFids( const QgsFeatureIds& fids );
    //! Get feature IDs that should be fetched.
    const QgsFeatureIds& filterFids() const { return mFilterFids; }

    /** Set the filter expression. {@see QgsExpression}
     * @param expression expression string
     * @see filterExpression
     * @see setExpressionContext
     */
    QgsFeatureRequest& setFilterExpression( const QString& expression );

    /** Returns the filter expression if set.
     * @see setFilterExpression
     * @see expressionContext
     */
    QgsExpression* filterExpression() const { return mFilterExpression; }

    /** Modifies the existing filter expression to add an additional expression filter. The
     * filter expressions are combined using AND, so only features matching both
     * the existing expression and the additional expression will be returned.
     * @note added in QGIS 2.14
     */
    QgsFeatureRequest& combineFilterExpression( const QString& expression );

    /** Returns the expression context used to evaluate filter expressions.
     * @note added in QGIS 2.12
     * @see setExpressionContext
     * @see filterExpression
     */
    QgsExpressionContext* expressionContext() { return &mExpressionContext; }

    /** Sets the expression context used to evaluate filter expressions.
     * @note added in QGIS 2.12
     * @see expressionContext
     * @see setFilterExpression
     */
    QgsFeatureRequest& setExpressionContext( const QgsExpressionContext& context );

    /**
     * Disables filter conditions.
     * The spatial filter (filterRect) will be kept in place.
     *
     * @return The object the method is called on for chaining
     *
     * @note Added in 2.12
     */
    QgsFeatureRequest& disableFilter() { mFilter = FilterNone; return *this; }

    /**
     * Adds a new OrderByClause, appending it as the least important one.
     *
     * @param expression The expression to use for ordering
     * @param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
     *                   If the order is ascending, by default nulls are last
     *                   If the order is descending, by default nulls are first
     *
     * @note added in QGIS 2.14
     */

    QgsFeatureRequest& addOrderBy( const QString &expression, bool ascending = true );
    /**
     * Adds a new OrderByClause, appending it as the least important one.
     *
     * @param expression The expression to use for ordering
     * @param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
     * @param nullsfirst If true, NULLS are at the beginning, if false, NULLS are at the end
     *
     * @note added in QGIS 2.14
     */
    QgsFeatureRequest& addOrderBy( const QString &expression, bool ascending, bool nullsfirst );

    /**
     * Return a list of order by clauses specified for this feature request.
     *
     * @note added in 2.14
     */
    OrderBy orderBy() const;

    /**
     * Set a list of order by clauses.
     *
     * @note added in 2.14
     */
    QgsFeatureRequest& setOrderBy( const OrderBy& orderBy );

    /** Set the maximum number of features to request.
     * @param limit maximum number of features, or -1 to request all features.
     * @see limit()
     * @note added in QGIS 2.14
     */
    QgsFeatureRequest& setLimit( long limit );

    /** Returns the maximum number of features to request, or -1 if no limit set.
     * @see setLimit
     * @note added in QGIS 2.14
     */
    long limit() const { return mLimit; }

    //! Set flags that affect how features will be fetched
    QgsFeatureRequest& setFlags( const QgsFeatureRequest::Flags& flags );
    const Flags& flags() const { return mFlags; }

    //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
    //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
    QgsFeatureRequest& setSubsetOfAttributes( const QgsAttributeList& attrs );
    /**
     * Return the subset of attributes which at least need to be fetched
     * @return A list of attributes to be fetched
     */
    QgsAttributeList subsetOfAttributes() const { return mAttrs; }

    //! Set a subset of attributes by names that will be fetched
    QgsFeatureRequest& setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields );

    //! Set a simplification method for geometries that will be fetched
    //! @note added in 2.2
    QgsFeatureRequest& setSimplifyMethod( const QgsSimplifyMethod& simplifyMethod );
    //! Get simplification method for geometries that will be fetched
    //! @note added in 2.2
    const QgsSimplifyMethod& simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Check if a feature is accepted by this requests filter
     *
     * @param feature  The feature which will be tested
     *
     * @return true, if the filter accepts the feature
     *
     * @note added in 2.1
     */
    bool acceptFeature( const QgsFeature& feature );

    // TODO: in future
    // void setFilterNativeExpression(con QString& expr);   // using provider's SQL (if supported)

  protected:
    FilterType mFilter;
    QgsRectangle mFilterRect;
    QgsFeatureId mFilterFid;
    QgsFeatureIds mFilterFids;
    QgsExpression* mFilterExpression;
    QgsExpressionContext mExpressionContext;
    Flags mFlags;
    QgsAttributeList mAttrs;
    QgsSimplifyMethod mSimplifyMethod;
    long mLimit;
    OrderBy mOrderBy;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRequest::Flags )


class QgsFeatureIterator;
class QgsAbstractFeatureIterator;

/** \ingroup core
 * Base class that can be used for any class that is capable of returning features
 * @note added in 2.4
 */
class CORE_EXPORT QgsAbstractFeatureSource
{
  public:
    virtual ~QgsAbstractFeatureSource();

    /**
     * Get an iterator for features matching the specified request
     * @param request The request
     * @return A feature iterator
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) = 0;

  protected:
    void iteratorOpened( QgsAbstractFeatureIterator* it );
    void iteratorClosed( QgsAbstractFeatureIterator* it );

    QSet< QgsAbstractFeatureIterator* > mActiveIterators;

    template<typename> friend class QgsAbstractFeatureIteratorFromSource;
};

#endif // QGSFEATUREREQUEST_H
