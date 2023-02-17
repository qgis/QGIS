/***************************************************************************
    qgsfeatureiterator.h
    ---------------------
    begin                : Juli 2012
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
#ifndef QGSFEATUREITERATOR_H
#define QGSFEATUREITERATOR_H

#include "qgis_core.h"
#include "qgsfeaturerequest.h"
#include "qgsindexedfeature.h"

class QgsFeedback;

/**
 * \ingroup core
 * \brief Internal feature iterator to be implemented within data providers
 */
class CORE_EXPORT QgsAbstractFeatureIterator
{
  public:

    //! Status of expression compilation for filter expression requests
    enum CompileStatus
    {
      NoCompilation, //!< Expression could not be compiled or not attempt was made to compile expression
      PartiallyCompiled, //!< Expression was partially compiled, but extra checks need to be applied to features
      Compiled, //!< Expression was fully compiled and delegated to data provider source
    };

    //! base class constructor - stores the iteration parameters
    QgsAbstractFeatureIterator( const QgsFeatureRequest &request );

    //! destructor makes sure that the iterator is closed properly
    virtual ~QgsAbstractFeatureIterator() = default;

    //! fetch next feature, return TRUE on success
    virtual bool nextFeature( QgsFeature &f );

    //! reset the iterator to the starting position
    virtual bool rewind() = 0;
    //! end of iterating: free the resources / lock
    virtual bool close() = 0;

    /**
     * Attach an object that can be queried regularly by the iterator to check
     * if it must stopped. This is mostly useful for iterators where a single
     * nextFeature()/fetchFeature() iteration might be very long. A typical use case is the
     * WFS provider. When nextFeature()/fetchFeature() is reasonably fast, it is not necessary
     * to implement this method. The default implementation does nothing.
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    virtual void setInterruptionChecker( QgsFeedback *interruptionChecker ) SIP_SKIP;

    /**
     * Returns the status of expression compilation for filter expression requests.
     * \since QGIS 2.16
     */
    CompileStatus compileStatus() const { return mCompileStatus; }

    /**
     * Returns if this iterator is valid.
     * An invalid feature iterator is not able to provide a reliable source for data.
     * If an iterator is invalid, either give up or try to send the request again (preferably
     * after a timeout to give the system some time to stay responsive).
     *
     * If you want to check if the iterator successfully completed, better use QgsFeatureIterator::isClosed().
     *
     * \since QGIS 3.0
     */
    virtual bool isValid() const
    {
      return mValid;
    }

    /**
     * Indicator if there was an error when sending the compiled query to the server.
     * This indicates that there is something wrong with the expression compiler.
     *
     * \since QGIS 3.2
     */
    bool compileFailed() const;

    /**
     * Possible results from the updateRequestToSourceCrs() method.
     *
     * \since QGIS 3.22
     */
    enum class RequestToSourceCrsResult : int
    {
      Success, //!< Request was successfully updated to the source CRS, or no changes were required
      DistanceWithinMustBeCheckedManually, //!< The distance within request cannot be losslessly updated to the source CRS, and callers will need to take appropriate steps to handle the distance within requirement manually during feature iteration
    };

  protected:

    /**
     * If you write a feature iterator for your provider, this is the method you
     * need to implement!!
     *
     * \param f The feature to write to
     * \returns  TRUE if a feature was written to f
     */
    virtual bool fetchFeature( QgsFeature &f ) = 0;

    /**
     * By default, the iterator will fetch all features and check if the feature
     * matches the expression.
     * If you have a more sophisticated metodology (SQL request for the features...)
     * and you check for the expression in your fetchFeature method, you can just
     * redirect this call to fetchFeature so the default check will be omitted.
     *
     * \param f The feature to write to
     * \returns  TRUE if a feature was written to f
     */
    virtual bool nextFeatureFilterExpression( QgsFeature &f );

    /**
     * By default, the iterator will fetch all features and check if the id
     * is in the request.
     * If you have a more sophisticated metodology (SQL request for the features...)
     * and you are sure, that any feature you return from fetchFeature will match
     * if the request was FilterFids you can just redirect this call to fetchFeature
     * so the default check will be omitted.
     *
     * \param f The feature to write to
     * \returns  TRUE if a feature was written to f
     */
    virtual bool nextFeatureFilterFids( QgsFeature &f );

    /**
     * Transforms \a feature's geometry according to the specified coordinate \a transform.
     * If \a feature has no geometry or \a transform is invalid then calling this method
     * has no effect and will be shortcut.
     * Iterators should call this method before returning features to ensure that any
     * QgsFeatureRequest::destinationCrs() set on the request is respected.
     * \since QGIS 3.0
     */
    void geometryToDestinationCrs( QgsFeature &feature, const QgsCoordinateTransform &transform ) const;


    /**
     * Returns a rectangle representing the original request's QgsFeatureRequest::filterRect().
     * If \a transform is a valid coordinate transform, the return rectangle will represent
     * the requested filterRect() transformed to the source's coordinate reference system.
     * Iterators should call this method and use the returned rectangle for filtering
     * features to ensure that any QgsFeatureRequest::destinationCrs() set on the request is respected.
     * Will throw a QgsCsException if the rect cannot be transformed from the destination CRS.
     * \since QGIS 3.0
     */
    QgsRectangle filterRectToSourceCrs( const QgsCoordinateTransform &transform ) const SIP_THROW( QgsCsException );

    /**
     * Update a QgsFeatureRequest so that spatial filters are
     * transformed to the source's coordinate reference system.
     * Iterators should call this method against the request used for filtering
     * features to ensure that any QgsFeatureRequest::destinationCrs() set on the request is respected.
     *
     * \returns result of operation. See QgsAbstractFeatureIterator::RequestToSourceCrsResult for interpretation.
     *
     * \throws QgsCsException if the rect cannot be transformed from the destination CRS.
     *
     * \since QGIS 3.22
     */
    RequestToSourceCrsResult updateRequestToSourceCrs( QgsFeatureRequest &request, const QgsCoordinateTransform &transform ) const SIP_THROW( QgsCsException );

    //! A copy of the feature request.
    QgsFeatureRequest mRequest;

    //! Sets to TRUE, as soon as the iterator is closed.
    bool mClosed = false;

    /**
     * A feature iterator may be closed already but still be serving features from the cache.
     * This is done when we serve features which have been pre-fetched and the order by has
     * been locally sorted.
     * In such a scenario, all resources have been released (mClosed is TRUE) but the deads
     * are still alive.
     */
    bool mZombie = false;

    // TODO QGIS 4: make this private

    /**
     * reference counting (to allow seamless copying of QgsFeatureIterator instances)
     */
    int refs = 0;
    //! Add reference
    void ref();
    //! Remove reference, delete if refs == 0
    void deref();
    friend class QgsFeatureIterator;

    //! Number of features already fetched by iterator
    long long mFetchedCount = 0;

    //! Status of compilation of filter expression
    CompileStatus mCompileStatus = NoCompilation;

    bool mCompileFailed = false;

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod );

    /**
     * An invalid state of a feature iterator indicates that there was a problem with
     * even getting it up and running.
     * This should be set to FALSE by subclasses if they have problems connecting to
     * the provider.
     * Do NOT set this to FALSE when the feature iterator closes or has no features but
     * we are sure, that it's just an empty dataset.
     */
    bool mValid = true;

  private:
    bool mUseCachedFeatures = false;
    QList<QgsIndexedFeature> mCachedFeatures;
    QList<QgsIndexedFeature>::ConstIterator mFeatureIterator;

    //! returns whether the iterator supports simplify geometries on provider side
    virtual bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const;

    /**
     * Should be overwritten by providers which implement an own order by strategy
     * If the own order by strategy is successful, return TRUE, if not, return FALSE
     * and a local order by will be triggered instead.
     * By default returns FALSE
     *
     * \since QGIS 2.14
     */
    virtual bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys );

    /**
     * Setup the orderby. Internally calls prepareOrderBy and if FALSE is returned will
     * cache all features and order them with local expression evaluation.
     *
     * \since QGIS 2.14
     */
    void setupOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys );
};


/**
 * \ingroup core
 * \brief Helper template that cares of two things: 1. automatic deletion of source if owned by iterator, 2. notification of open/closed iterator.
 * \note not available in Python bindings (although present in SIP file)
*/
template<typename T>
class QgsAbstractFeatureIteratorFromSource : public QgsAbstractFeatureIterator
{
  public:
    QgsAbstractFeatureIteratorFromSource( T *source, bool ownSource, const QgsFeatureRequest &request )
      : QgsAbstractFeatureIterator( request )
      , mSource( source )
      , mOwnSource( ownSource )
    {
      mSource->iteratorOpened( this );
    }

    ~QgsAbstractFeatureIteratorFromSource() override
    {
      if ( mOwnSource )
        delete mSource;
    }

  protected:
    //! to be called by from subclass in close()
    void iteratorClosed() { mSource->iteratorClosed( this ); }

    T *mSource = nullptr;
    bool mOwnSource;
};


/**
 * \ingroup core
 * \brief Wrapper for iterator of features from vector data provider or vector layer
 */
class CORE_EXPORT QgsFeatureIterator
{
  public:

#ifdef SIP_RUN
    QgsFeatureIterator *__iter__();
    % MethodCode
    sipRes = sipCpp;
    % End

    SIP_PYOBJECT __next__() SIP_TYPEHINT( QgsFeature );
    % MethodCode
    std::unique_ptr< QgsFeature > f = std::make_unique< QgsFeature >();
    bool result = false;
    Py_BEGIN_ALLOW_THREADS
    result = ( sipCpp->nextFeature( *f ) );
    Py_END_ALLOW_THREADS
    if ( result )
      sipRes = sipConvertFromType( f.release(), sipType_QgsFeature, Py_None );
    else
    {
      PyErr_SetString( PyExc_StopIteration, "" );
    }
    % End
#endif

    //! Construct invalid iterator
    QgsFeatureIterator() = default;
    //! Construct a valid iterator
    QgsFeatureIterator( QgsAbstractFeatureIterator *iter SIP_TRANSFER );
    //! Copy constructor copies the iterator, increases ref.count
    QgsFeatureIterator( const QgsFeatureIterator &fi );
    //! Destructor deletes the iterator if it has no more references
    ~QgsFeatureIterator();

    QgsFeatureIterator &operator=( const QgsFeatureIterator &other );

    bool nextFeature( QgsFeature &f );
    bool rewind();
    bool close();

    /**
     * Will return if this iterator is valid.
     * An invalid iterator was probably introduced by a failed attempt to acquire a connection
     * or is a default constructed iterator.
     *
     * \see isClosed to check if the iterator successfully completed and returned all the features.
     *
     * \since QGIS 3.0
     */
    bool isValid() const;

    //! find out whether the iterator is still valid or closed already
    bool isClosed() const;

    /**
     * Attach an object that can be queried regularly by the iterator to check
     * if it must stopped. This is mostly useful for iterators where a single
     * nextFeature()/fetchFeature() iteration might be very long. A typical use case is the
     * WFS provider.
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    void setInterruptionChecker( QgsFeedback *interruptionChecker ) SIP_SKIP;

    /**
     * Returns the status of expression compilation for filter expression requests.
     * \since QGIS 2.16
     */
    QgsAbstractFeatureIterator::CompileStatus compileStatus() const { return mIter->compileStatus(); }

    /**
     * Indicator if there was an error when sending the compiled query to the server.
     * This indicates that there is something wrong with the expression compiler.
     *
     * \since QGIS 3.2
     */
    bool compileFailed() const { return mIter->compileFailed(); }

    friend bool operator== ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 ) SIP_SKIP;
    friend bool operator!= ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 ) SIP_SKIP;

  protected:
    QgsAbstractFeatureIterator *mIter = nullptr;


};

#ifndef SIP_RUN

inline QgsFeatureIterator::QgsFeatureIterator( QgsAbstractFeatureIterator *iter )
  : mIter( iter )
{
  if ( iter )
    iter->ref();
}

inline QgsFeatureIterator::QgsFeatureIterator( const QgsFeatureIterator &fi )
  : mIter( fi.mIter )
{
  if ( mIter )
    mIter->ref();
}

inline QgsFeatureIterator::~QgsFeatureIterator()
{
  if ( mIter )
    mIter->deref();
}

inline bool QgsFeatureIterator::nextFeature( QgsFeature &f )
{
  return mIter ? mIter->nextFeature( f ) : false;
}

inline bool QgsFeatureIterator::rewind()
{
  if ( mIter )
    mIter->mFetchedCount = 0;

  return mIter ? mIter->rewind() : false;
}

inline bool QgsFeatureIterator::close()
{
  if ( mIter )
    mIter->mFetchedCount = 0;

  return mIter ? mIter->close() : false;
}

inline bool QgsFeatureIterator::isClosed() const
{
  return mIter ? mIter->mClosed && !mIter->mZombie : true;
}

inline bool operator== ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 )
{
  return fi1.mIter == fi2.mIter;
}

inline bool operator!= ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 )
{
  return !( fi1 == fi2 );
}

inline void QgsFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  if ( mIter )
    mIter->setInterruptionChecker( interruptionChecker );
}

#endif

#endif // QGSFEATUREITERATOR_H
