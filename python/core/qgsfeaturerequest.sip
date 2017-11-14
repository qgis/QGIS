/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturerequest.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFeatureRequest
{
%Docstring
 This class wraps a request for features to a vector layer (or directly its vector data provider).
 The request may apply a filter to fetch only a particular subset of features. Currently supported filters:
 - no filter - all features are returned
 - feature id - only feature that matches given feature id is returned
 - feature ids - only features that match any of the given feature ids are returned
 - filter expression - only features that match the given filter expression are returned

 Additionally a spatial rectangle can be set in combination:
 Only features that intersect given rectangle should be fetched. For the sake of speed,
 the intersection is often done only using feature's bounding box. There is a flag
 ExactIntersect that makes sure that only intersecting features will be returned.

 For efficiency, it is also possible to tell provider that some data is not required:
 - NoGeometry flag
 - SubsetOfAttributes flag
 - SimplifyMethod for geometries to fetch

 The options may be chained, e.g.:
   QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1)).setFlags(QgsFeatureRequest.ExactIntersect)

 Examples:
 - fetch all features:
     QgsFeatureRequest()
 - fetch all features, only one attribute
     QgsFeatureRequest().setSubsetOfAttributes(QStringList("myfield"), provider->fieldMap())
 - fetch all features, without geometries
     QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
 - fetch only features from particular extent
     QgsFeatureRequest().setFilterRect(QgsRectangle(0,0,1,1))
 - fetch only one feature
     QgsFeatureRequest().setFilterFid(45)
%End

%TypeHeaderCode
#include "qgsfeaturerequest.h"
%End
  public:
    enum Flag
    {
      NoFlags,
      NoGeometry,
      SubsetOfAttributes,
      ExactIntersect
    };
    typedef QFlags<QgsFeatureRequest::Flag> Flags;


    enum FilterType
    {
      FilterNone,
      FilterFid,
      FilterExpression,
      FilterFids
    };

    enum InvalidGeometryCheck
    {
      GeometryNoCheck,
      GeometrySkipInvalid,
      GeometryAbortOnInvalid,
    };

    class OrderByClause
{
%Docstring
 The OrderByClause class represents an order by clause for a QgsFeatureRequest.

 It can be a simple field or an expression. Multiple order by clauses can be added to
 a QgsFeatureRequest to fine tune the behavior if a single field or expression is not
 enough to completely specify the required behavior.

 If expression compilation is activated in the settings and the expression can be
 translated for the provider in question, it will be evaluated on provider side.
 If one of these two premises does not apply, the ordering will take place locally
 which results in increased memory and CPU usage.

 If the ordering is done on strings, the order depends on the system's locale if the
 local fallback implementation is used. The order depends on the server system's locale
 and implementation if ordering is done on the server.

 In case the fallback code needs to be used, a limit set on the request will be respected
 for the features returned by the iterator but internally all features will be requested
 from the provider.

.. versionadded:: 2.14
%End

%TypeHeaderCode
#include "qgsfeaturerequest.h"
%End
      public:

        OrderByClause( const QString &expression, bool ascending = true );
%Docstring
 Creates a new OrderByClause for a QgsFeatureRequest

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
                   If the order is ascending, by default nulls are last
                   If the order is descending, by default nulls are first
%End

        OrderByClause( const QString &expression, bool ascending, bool nullsfirst );
%Docstring
 Creates a new OrderByClause for a QgsFeatureRequest

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
 \param nullsfirst If true, NULLS are at the beginning, if false, NULLS are at the end
%End

        OrderByClause( const QgsExpression &expression, bool ascending = true );
%Docstring
 Creates a new OrderByClause for a QgsFeatureRequest

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
                   If the order is ascending, by default nulls are last
                   If the order is descending, by default nulls are first
%End

        OrderByClause( const QgsExpression &expression, bool ascending, bool nullsfirst );
%Docstring
 Creates a new OrderByClause for a QgsFeatureRequest

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
 \param nullsfirst If true, NULLS are at the beginning, if false, NULLS are at the end
%End

        QgsExpression expression() const;
%Docstring
 The expression
 :return: the expression
 :rtype: QgsExpression
%End

        bool prepare( QgsExpressionContext *context );
%Docstring
 Prepare the expression with the given context.

.. seealso:: QgsExpression.prepare

.. versionadded:: 3.0
 :rtype: bool
%End

        bool ascending() const;
%Docstring
 Order ascending
 :return: If ascending order is requested
 :rtype: bool
%End

        void setAscending( bool ascending );
%Docstring
 Set if ascending order is requested
%End

        bool nullsFirst() const;
%Docstring
 Set if NULLS should be returned first
 :return: if NULLS should be returned first
 :rtype: bool
%End

        void setNullsFirst( bool nullsFirst );
%Docstring
 Set if NULLS should be returned first
%End

        QString dump() const;
%Docstring
 Dumps the content to an SQL equivalent
 :rtype: str
%End


    };


    class OrderBy
{
%Docstring
 Represents a list of OrderByClauses, with the most important first and the least
 important last.

.. versionadded:: 2.14
%End

%TypeHeaderCode
#include "qgsfeaturerequest.h"
%End
      public:

        OrderBy();
%Docstring
 Create a new empty order by
%End

        OrderBy( const QList<QgsFeatureRequest::OrderByClause> &other );
%Docstring
 Create a new order by from a list of clauses
%End

        QList<QgsFeatureRequest::OrderByClause> list() const;
%Docstring
 Get a copy as a list of OrderByClauses

 This is only required in Python where the inheritance
 is not properly propagated and this makes it usable.
 :rtype: list of QgsFeatureRequest.OrderByClause
%End

        void save( QDomElement &elem ) const;
%Docstring
 Serialize to XML
%End

        void load( const QDomElement &elem );
%Docstring
 Deserialize from XML
%End

        QSet<QString> usedAttributes() const;
%Docstring
 Returns a set of used attributes
 :rtype: set of str
%End

        QString dump() const;
%Docstring
 Dumps the content to an SQL equivalent syntax
 :rtype: str
%End
    };

    static const QString ALL_ATTRIBUTES;
%Docstring
 A special attribute that if set matches all attributes
%End

    QgsFeatureRequest();
%Docstring
construct a default request: for all features get attributes and geometries
%End
    explicit QgsFeatureRequest( QgsFeatureId fid );
%Docstring
construct a request with feature ID filter
%End
    explicit QgsFeatureRequest( const QgsFeatureIds &fids );
%Docstring
construct a request with feature ID filter
%End

    explicit QgsFeatureRequest( const QgsRectangle &rectangle );
%Docstring
 Construct a request with ``rectangle`` bounding box filter.

 When a destination CRS is set using setDestinationCrs(), ``rectangle``
 is expected to be in the same CRS as the destinationCrs(). Otherwise, ``rectangle``
 should use the same CRS as the source layer/provider.
%End

    explicit QgsFeatureRequest( const QgsExpression &expr, const QgsExpressionContext &context = QgsExpressionContext() );
%Docstring
construct a request with a filter expression
%End
    QgsFeatureRequest( const QgsFeatureRequest &rh );
%Docstring
copy constructor
%End

    FilterType filterType() const;
%Docstring
 Return the filter type which is currently set on this request

 :return: Filter type
 :rtype: FilterType
%End

    QgsFeatureRequest &setFilterRect( const QgsRectangle &rectangle );
%Docstring
 Sets the ``rectangle`` from which features will be taken. An empty rectangle removes the filter.

 When a destination CRS is set using setDestinationCrs(), ``rectangle``
 is expected to be in the same CRS as the destinationCrs(). Otherwise, ``rectangle``
 should use the same CRS as the source layer/provider.

.. seealso:: filterRect()
 :rtype: QgsFeatureRequest
%End

    const QgsRectangle &filterRect() const;
%Docstring
 Returns the rectangle from which features will be taken. If the returned
 rectangle is null, then no filter rectangle is set.

 When a destination CRS is set using setDestinationCrs(), the rectangle
 will be in the same CRS as the destinationCrs(). Otherwise, the rectangle
 will use the same CRS as the source layer/provider.

.. seealso:: setFilterRect()
 :rtype: QgsRectangle
%End

    QgsFeatureRequest &setFilterFid( QgsFeatureId fid );
%Docstring
Set feature ID that should be fetched.
 :rtype: QgsFeatureRequest
%End
    QgsFeatureId filterFid() const;
%Docstring
Get the feature ID that should be fetched.
 :rtype: QgsFeatureId
%End

    QgsFeatureRequest &setFilterFids( const QgsFeatureIds &fids );
%Docstring
Set feature IDs that should be fetched.
 :rtype: QgsFeatureRequest
%End
    const QgsFeatureIds &filterFids() const;
%Docstring
Get feature IDs that should be fetched.
 :rtype: QgsFeatureIds
%End

    QgsFeatureRequest &setInvalidGeometryCheck( InvalidGeometryCheck check );
%Docstring
 Sets invalid geometry checking behavior.
.. note::

   Invalid geometry checking is not performed when retrieving features
 directly from a QgsVectorDataProvider.
.. seealso:: invalidGeometryCheck()
.. versionadded:: 3.0
 :rtype: QgsFeatureRequest
%End

    InvalidGeometryCheck invalidGeometryCheck() const;
%Docstring
 Returns the invalid geometry checking behavior.
.. seealso:: setInvalidGeometryCheck()
.. versionadded:: 3.0
 :rtype: InvalidGeometryCheck
%End

    QgsFeatureRequest &setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
 Sets a callback function to use when encountering an invalid geometry and
 invalidGeometryCheck() is set to GeometryAbortOnInvalid or GeometrySkipInvalid. This function will be
 called using the feature with invalid geometry as a parameter.
.. versionadded:: 3.0
.. seealso:: invalidGeometryCallback()
 :rtype: QgsFeatureRequest
%End
%MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setInvalidGeometryCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    sipRes = sipCpp;

    Py_END_ALLOW_THREADS
%End


    QgsFeatureRequest &setFilterExpression( const QString &expression );
%Docstring
.. seealso:: QgsExpression}
 \param expression expression string
.. seealso:: filterExpression
.. seealso:: setExpressionContext
 :rtype: QgsFeatureRequest
%End

    QgsExpression *filterExpression() const;
%Docstring
 Returns the filter expression if set.
.. seealso:: setFilterExpression
.. seealso:: expressionContext
 :rtype: QgsExpression
%End

    QgsFeatureRequest &combineFilterExpression( const QString &expression );
%Docstring
 Modifies the existing filter expression to add an additional expression filter. The
 filter expressions are combined using AND, so only features matching both
 the existing expression and the additional expression will be returned.
.. versionadded:: 2.14
 :rtype: QgsFeatureRequest
%End

    QgsExpressionContext *expressionContext();
%Docstring
 Returns the expression context used to evaluate filter expressions.
.. versionadded:: 2.12
.. seealso:: setExpressionContext
.. seealso:: filterExpression
 :rtype: QgsExpressionContext
%End

    QgsFeatureRequest &setExpressionContext( const QgsExpressionContext &context );
%Docstring
 Sets the expression context used to evaluate filter expressions.
.. versionadded:: 2.12
.. seealso:: expressionContext
.. seealso:: setFilterExpression
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &disableFilter();
%Docstring
 Disables filter conditions.
 The spatial filter (filterRect) will be kept in place.

 :return: The object the method is called on for chaining

.. versionadded:: 2.12
 :rtype: QgsFeatureRequest
%End


    QgsFeatureRequest &addOrderBy( const QString &expression, bool ascending = true );
%Docstring
 Adds a new OrderByClause, appending it as the least important one.

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
                   If the order is ascending, by default nulls are last
                   If the order is descending, by default nulls are first

.. versionadded:: 2.14
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &addOrderBy( const QString &expression, bool ascending, bool nullsfirst );
%Docstring
 Adds a new OrderByClause, appending it as the least important one.

 \param expression The expression to use for ordering
 \param ascending  If the order should be ascending (1,2,3) or descending (3,2,1)
 \param nullsfirst If true, NULLS are at the beginning, if false, NULLS are at the end

.. versionadded:: 2.14
 :rtype: QgsFeatureRequest
%End

    OrderBy orderBy() const;
%Docstring
 Return a list of order by clauses specified for this feature request.

.. versionadded:: 2.14
 :rtype: OrderBy
%End

    QgsFeatureRequest &setOrderBy( const OrderBy &orderBy );
%Docstring
 Set a list of order by clauses.

.. versionadded:: 2.14
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &setLimit( long limit );
%Docstring
 Set the maximum number of features to request.
 \param limit maximum number of features, or -1 to request all features.
.. seealso:: limit()
.. versionadded:: 2.14
 :rtype: QgsFeatureRequest
%End

    long limit() const;
%Docstring
 Returns the maximum number of features to request, or -1 if no limit set.
.. seealso:: setLimit
.. versionadded:: 2.14
 :rtype: long
%End

    QgsFeatureRequest &setFlags( QgsFeatureRequest::Flags flags );
%Docstring
Set flags that affect how features will be fetched
 :rtype: QgsFeatureRequest
%End
    const Flags &flags() const;
%Docstring
 :rtype: Flags
%End

    QgsFeatureRequest &setSubsetOfAttributes( const QgsAttributeList &attrs );
%Docstring
 Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
 To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
 :rtype: QgsFeatureRequest
%End

    QgsAttributeList subsetOfAttributes() const;
%Docstring
 Return the subset of attributes which at least need to be fetched
 :return: A list of attributes to be fetched
 :rtype: QgsAttributeList
%End

    QgsFeatureRequest &setSubsetOfAttributes( const QStringList &attrNames, const QgsFields &fields );
%Docstring
Set a subset of attributes by names that will be fetched
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &setSubsetOfAttributes( const QSet<QString> &attrNames, const QgsFields &fields );
%Docstring
Set a subset of attributes by names that will be fetched
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &setSimplifyMethod( const QgsSimplifyMethod &simplifyMethod );
%Docstring
 Set a simplification method for geometries that will be fetched
.. versionadded:: 2.2
 :rtype: QgsFeatureRequest
%End

    const QgsSimplifyMethod &simplifyMethod() const;
%Docstring
 Get simplification method for geometries that will be fetched
.. versionadded:: 2.2
 :rtype: QgsSimplifyMethod
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
 Returns the destination coordinate reference system for feature's geometries,
 or an invalid QgsCoordinateReferenceSystem if no reprojection will be done
 and all features will be left with their original geometry.
.. seealso:: setDestinationCrs()
.. versionadded:: 3.0
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsFeatureRequest &setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the destination ``crs`` for feature's geometries. If set, all
 geometries will be reprojected from their original coordinate reference
 system to this desired reference system. If ``crs`` is an invalid
 QgsCoordinateReferenceSystem then no reprojection will be done
 and all features will be left with their original geometry.

 When a ``crs`` is set using setDestinationCrs(), then any filterRect()
 set on the request is expected to be in the same CRS as the destination
 CRS.

 The feature geometry transformation to the destination CRS is performed
 after all filter expressions are tested and any virtual fields are
 calculated. Accordingly, any geometric expressions used in
 filterExpression() will be performed in the original
 source CRS. This ensures consistent results are returned regardless of the
 destination CRS. Similarly, virtual field values will be calculated using the
 original geometry in the source CRS, so these values are not affected by
 any destination CRS transform present in the feature request.

.. seealso:: destinationCrs()
.. versionadded:: 3.0
 :rtype: QgsFeatureRequest
%End

    QgsFeatureRequest &setTransformErrorCallback( SIP_PYCALLABLE / AllowNone / );
%Docstring
 Sets a callback function to use when encountering a transform error when iterating
 features and a destinationCrs() is set. This function will be
 called using the feature which encountered the transform error as a parameter.
.. versionadded:: 3.0
.. seealso:: transformErrorCallback()
.. seealso:: setDestinationCrs()
 :rtype: QgsFeatureRequest
%End
%MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setTransformErrorCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    sipRes = sipCpp;

    Py_END_ALLOW_THREADS
%End



    bool acceptFeature( const QgsFeature &feature );
%Docstring
 Check if a feature is accepted by this requests filter

 \param feature  The feature which will be tested

 :return: true, if the filter accepts the feature

.. versionadded:: 2.1
 :rtype: bool
%End

    int connectionTimeout() const;
%Docstring
 The timeout for how long we should wait for a connection if none is available from the pool
 at this moment. A negative value (which is set by default) will wait forever.

.. note::

   Only works if the provider supports this option.

.. versionadded:: 3.0
 :rtype: int
%End

    void setConnectionTimeout( int connectionTimeout );
%Docstring
 The timeout for how long we should wait for a connection if none is available from the pool
 at this moment. A negative value (which is set by default) will wait forever.

.. note::

   Only works if the provider supports this option.

.. versionadded:: 3.0
%End

  protected:
};

QFlags<QgsFeatureRequest::Flag> operator|(QgsFeatureRequest::Flag f1, QFlags<QgsFeatureRequest::Flag> f2);




class QgsAbstractFeatureSource
{
%Docstring
 Base class that can be used for any class that is capable of returning features
.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsfeaturerequest.h"
%End
  public:
    virtual ~QgsAbstractFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) = 0;
%Docstring
 Get an iterator for features matching the specified request
 \param request The request
 :return: A feature iterator
 :rtype: QgsFeatureIterator
%End

  protected:
    void iteratorOpened( QgsAbstractFeatureIterator *it );
    void iteratorClosed( QgsAbstractFeatureIterator *it );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeaturerequest.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
