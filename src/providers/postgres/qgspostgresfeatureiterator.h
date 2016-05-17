/***************************************************************************
    qgspostgresfeatureiterator.h
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
#ifndef QGSPOSTGRESFEATUREITERATOR_H
#define QGSPOSTGRESFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QQueue>

#include "qgspostgresprovider.h"

class QgsPostgresProvider;
class QgsPostgresResult;
class QgsPostgresTransaction;


class QgsPostgresFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsPostgresFeatureSource( const QgsPostgresProvider* p );
    ~QgsPostgresFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:

    QString mConnInfo;

    QString mGeometryColumn;
    QString mSqlWhereClause;
    QgsFields mFields;
    QgsPostgresGeometryColumnType mSpatialColType;
    QString mRequestedSrid;
    QString mDetectedSrid;
    bool mForce2d;
    QGis::WkbType mRequestedGeomType; //! geometry type requested in the uri
    QGis::WkbType mDetectedGeomType;  //! geometry type detected in the database
    QgsPostgresPrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    // TODO: loadFields()

    QSharedPointer<QgsPostgresSharedData> mShared;

    /* The transaction connection (if any) gets refed/unrefed when creating/
     * destroying the QgsPostgresFeatureSource, to ensure that the transaction
     * connection remains valid during the life time of the feature source
     * even if the QgsPostgresTransaction object which initially created the
     * connection has since been destroyed. */
    QgsPostgresConn* mTransactionConnection;

    friend class QgsPostgresFeatureIterator;
    friend class QgsPostgresExpressionCompiler;
};


class QgsPostgresConn;

class QgsPostgresFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsPostgresFeatureSource>
{
  public:
    QgsPostgresFeatureIterator( QgsPostgresFeatureSource* source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsPostgresFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! fetch next feature filter expression
    bool nextFeatureFilterExpression( QgsFeature& f ) override;

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod ) override;

    QgsPostgresConn* mConn;


    QString whereClauseRect();
    bool getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature );
    void getFeatureAttribute( int idx, QgsPostgresResult& queryResult, int row, int& col, QgsFeature& feature );
    bool declareCursor( const QString& whereClause, long limit = -1, bool closeOnFail = true , const QString& orderBy = QString() );

    QString mCursorName;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from PostgreSQL
     */
    QQueue<QgsFeature> mFeatureQueue;

    //! Maximal size of the feature queue
    int mFeatureQueueSize;

    //! Number of retrieved features
    int mFetched;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

    bool mIsTransactionConnection;

    static const int sFeatureQueueSize;

  private:
    //! returns whether the iterator supports simplify geometries on provider side
    virtual bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const override;

    virtual bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    inline void lock();
    inline void unlock();

    bool mExpressionCompiled;
    bool mOrderByCompiled;
    bool mLastFetch;
    bool mFilterRequiresGeometry;
};

#endif // QGSPOSTGRESFEATUREITERATOR_H
