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

#include "qgsfeaturerequest.h"
#include "qgslogger.h"
#include "qgsindexedfeature.h"


/** \ingroup core
 * Interface that can be optionaly attached to an iterator so its
 * nextFeature() implementaton can check if it must stop as soon as possible.
 * @note Added in QGIS 2.16
 * @note not available in Python bindings
 */
class CORE_EXPORT QgsInterruptionChecker
{
  public:
    //! return true if the iterator must stop as soon as possible
    virtual bool mustStop() const = 0;
};

/** \ingroup core
 * Internal feature iterator to be implemented within data providers
 */
class CORE_EXPORT QgsAbstractFeatureIterator
{
  public:

    //! Status of expression compilation for filter expression requests
    enum CompileStatus
    {
      NoCompilation, /*!< Expression could not be compiled or not attempt was made to compile expression */
      PartiallyCompiled, /*!< Expression was partially compiled, but extra checks need to be applied to features*/
      Compiled, /*!< Expression was fully compiled and delegated to data provider source*/
    };

    //! base class constructor - stores the iteration parameters
    QgsAbstractFeatureIterator( const QgsFeatureRequest& request );

    //! destructor makes sure that the iterator is closed properly
    virtual ~QgsAbstractFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& f );

    //! reset the iterator to the starting position
    virtual bool rewind() = 0;
    //! end of iterating: free the resources / lock
    virtual bool close() = 0;

    /** Attach an object that can be queried regularly by the iterator to check
     * if it must stopped. This is mostly useful for iterators where a single
     * nextFeature()/fetchFeature() iteration might be very long. A typical use case is the
     * WFS provider. When nextFeature()/fetchFeature() is reasonably fast, it is not necessary
     * to implement this method. The default implementation does nothing.
     * @note added in QGIS 2.16
     * @note not available in Python bindings
     */
    virtual void setInterruptionChecker( QgsInterruptionChecker* interruptionChecker );

    /** Returns the status of expression compilation for filter expression requests.
     * @note added in QGIS 2.16
     */
    CompileStatus compileStatus() const { return mCompileStatus; }

  protected:
    /**
     * If you write a feature iterator for your provider, this is the method you
     * need to implement!!
     *
     * @param f The feature to write to
     * @return  true if a feature was written to f
     */
    virtual bool fetchFeature( QgsFeature& f ) = 0;

    /**
     * By default, the iterator will fetch all features and check if the feature
     * matches the expression.
     * If you have a more sophisticated metodology (SQL request for the features...)
     * and you check for the expression in your fetchFeature method, you can just
     * redirect this call to fetchFeature so the default check will be omitted.
     *
     * @param f The feature to write to
     * @return  true if a feature was written to f
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
     * @param f The feature to write to
     * @return  true if a feature was written to f
     */
    virtual bool nextFeatureFilterFids( QgsFeature & f );

    /** A copy of the feature request. */
    QgsFeatureRequest mRequest;

    /** Set to true, as soon as the iterator is closed. */
    bool mClosed;

    /**
     * A feature iterator may be closed already but still be serving features from the cache.
     * This is done when we serve features which have been pre-fetched and the order by has
     * been locally sorted.
     * In such a scenario, all resources have been released (mClosed is true) but the deads
     * are still alive.
     */
    bool mZombie;

    //! reference counting (to allow seamless copying of QgsFeatureIterator instances)
    //! TODO QGIS3: make this private
    int refs;
    void ref(); //!< add reference
    void deref(); //!< remove reference, delete if refs == 0
    friend class QgsFeatureIterator;

    //! Number of features already fetched by iterator
    long mFetchedCount;

    //! Status of compilation of filter expression
    CompileStatus mCompileStatus;

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod );

  private:
    bool mUseCachedFeatures;
    QList<QgsIndexedFeature> mCachedFeatures;
    QList<QgsIndexedFeature>::ConstIterator mFeatureIterator;

    //! returns whether the iterator supports simplify geometries on provider side
    virtual bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const;

    /**
     * Should be overwritten by providers which implement an own order by strategy
     * If the own order by strategy is successful, return true, if not, return false
     * and a local order by will be triggered instead.
     * By default returns false
     *
     * @note added in QGIS 2.14
     */
    virtual bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys );

    /**
     * Setup the orderby. Internally calls prepareOrderBy and if false is returned will
     * cache all features and order them with local expression evaluation.
     *
     * @note added in QGIS 2.14
     */
    void setupOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys );
};



/** \ingroup core
 * Helper template that cares of two things: 1. automatic deletion of source if owned by iterator, 2. notification of open/closed iterator.
 * \note not available in Python bindings
*/
template<typename T>
class QgsAbstractFeatureIteratorFromSource : public QgsAbstractFeatureIterator
{
  public:
    QgsAbstractFeatureIteratorFromSource( T* source, bool ownSource, const QgsFeatureRequest& request )
        : QgsAbstractFeatureIterator( request )
        , mSource( source )
        , mOwnSource( ownSource )
    {
      mSource->iteratorOpened( this );
    }

    ~QgsAbstractFeatureIteratorFromSource()
    {
      if ( mOwnSource )
        delete mSource;
    }

  protected:
    //! to be called by from subclass in close()
    void iteratorClosed() { mSource->iteratorClosed( this ); }

    T* mSource;
    bool mOwnSource;
};

/**
 * \ingroup core
 * Wrapper for iterator of features from vector data provider or vector layer
 */
class CORE_EXPORT QgsFeatureIterator
{
  public:
    //! construct invalid iterator
    QgsFeatureIterator();
    //! construct a valid iterator
    QgsFeatureIterator( QgsAbstractFeatureIterator* iter );
    //! copy constructor copies the iterator, increases ref.count
    QgsFeatureIterator( const QgsFeatureIterator& fi );
    //! destructor deletes the iterator if it has no more references
    ~QgsFeatureIterator();

    QgsFeatureIterator& operator=( const QgsFeatureIterator& other );

    bool nextFeature( QgsFeature& f );
    bool rewind();
    bool close();

    //! find out whether the iterator is still valid or closed already
    bool isClosed() const;

    /** Attach an object that can be queried regularly by the iterator to check
     * if it must stopped. This is mostly useful for iterators where a single
     * nextFeature()/fetchFeature() iteration might be very long. A typical use case is the
     * WFS provider.
     * @note added in QGIS 2.16
     * @note not available in Python bindings
     */
    void setInterruptionChecker( QgsInterruptionChecker* interruptionChecker );

    /** Returns the status of expression compilation for filter expression requests.
     * @note added in QGIS 2.16
     */
    QgsAbstractFeatureIterator::CompileStatus compileStatus() const { return mIter->compileStatus(); }

    friend bool operator== ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 );
    friend bool operator!= ( const QgsFeatureIterator &fi1, const QgsFeatureIterator &fi2 );

  protected:
    QgsAbstractFeatureIterator* mIter;
};

////////

inline QgsFeatureIterator::QgsFeatureIterator()
    : mIter( nullptr )
{
}

inline QgsFeatureIterator::QgsFeatureIterator( QgsAbstractFeatureIterator* iter )
    : mIter( iter )
{
  if ( iter )
    iter->ref();
}

inline QgsFeatureIterator::QgsFeatureIterator( const QgsFeatureIterator& fi )
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

inline bool QgsFeatureIterator::nextFeature( QgsFeature& f )
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

inline void QgsFeatureIterator::setInterruptionChecker( QgsInterruptionChecker* interruptionChecker )
{
  if ( mIter )
    mIter->setInterruptionChecker( interruptionChecker );
}

#endif // QGSFEATUREITERATOR_H
